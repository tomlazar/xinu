/**
 * @file	lan7800.c
 *
 * @authors
 * 			Rade Latinovich
 * 			Patrick J. McGee
 *
 * This file provides various functions needed by the Microchip LAN7800 USB Ethernet Driver,
 * equipped on the Raspberry Pi 3 Model B+. These functions are based on those of the
 * Raspberry Pi 1 driver, SMSC 9512, in @file smsc9512.c
 *
 * Embedded Xinu, Copyright (C) 2018. All rights reserved.
 */


/* **********************************************************************
 * **********************************************************************
 * LAN7800 Device Driver Notes:
 * 
 * Seemingly relevant chapters:
 * 5,6,7,8,9,10,11,12,14
 * 
 * Section 5.1:
 * "The USB functionality consists of five major parts.
 *  The USB PHY, UDC (USB Device Controller), URX (USB Bulk Out Receiver),
 *  UTX (USB Bulk In Transmitter), and CTL (USB Control Block)."
 *
 * Section 6.1: (DOC p. 50)
 * ------------
 *  - 12 KB RX FIFO -> buffers frames received from the RFE
 *  				[RFE = Receive Filtering Engine]
 *  - UTX (USB Bulk-In transfer) extracts the frames from the
 *  	FCT (FIFO Controller) to form USB Bulk In packets
 *  - "Host software will reassemble the ethernet frames from USB packets."
 *  - 
 *
 *
 * **********************************************************************
 * **********************************************************************/

#include "lan7800.h"
#include <usb_core_driver.h>
#include <stdio.h>
#include <clock.h>
#include <string.h>

/**
 * @ingroup etherspecific
 *
 * Write to a register on the Microchip LAN7800 USB Ethernet Adapter.
 *
 * @param udev
 * 		USB device for the adapter
 * @param index
 * 		Index of the register to write
 * @param data
 * 		Value to write to the register
 *
 * @return
 * 		::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 * 		code.
 */
	usb_status_t
lan7800_write_reg(struct usb_device *udev, uint32_t index, uint32_t data)
{
	return usb_control_msg(udev, NULL,
			LAN7800_VENDOR_REQUEST_WRITE,
			USB_BMREQUESTTYPE_DIR_OUT |
			USB_BMREQUESTTYPE_TYPE_VENDOR |
			USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
			0, index, &data, sizeof(uint32_t));
}

/**
 * @ingroup etherspecific
 *
 * Read from a register on the Microchip LAN7800 USB Ethernet Adapter.
 *
 * @param udev
 * 		USB device for the adapter
 * @param index
 * 		Index of the register to read
 * @param data
 * 		Pointer into which to write the register's value
 *
 * @return
 * 		::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 * 		code.
 *
 */
	usb_status_t
lan7800_read_reg(struct usb_device *udev, uint32_t index, uint32_t *data)
{
	return usb_control_msg(udev, NULL,
			LAN7800_VENDOR_REQUEST_READ,
			USB_BMREQUESTTYPE_DIR_IN |
			USB_BMREQUESTTYPE_TYPE_VENDOR |
			USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
			0, index, data, sizeof(uint32_t));
}

/**
 * @ingroup etherspecific
 *
 * Modify the value contained in a register on the Microchip LAN7800 USB Ethernet
 * Adapter.
 *
 * @param udev
 * 		USB device for the adapter
 * @param index
 * 		Index of the register to modify
 * @param mask
 * 		Mask that contains 1 for the bits where the old value in the register
 * 		will be kept rather than cleared (unless those bits also appear in @p
 * 		set, in which case they will still be set).
 * @param set
 * 		Mask of bits to set in the register.
 *
 * @return
 * 		::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 * 		code.
 */
	usb_status_t
lan7800_modify_reg(struct usb_device *udev, uint32_t index,
		uint32_t mask, uint32_t set)
{
	usb_status_t status;
	uint32_t val;

	status = lan7800_read_reg(udev, index, &val);
	if (status != USB_STATUS_SUCCESS)
	{
		return status;
	}
	val &= mask;
	val |= set;
	return lan7800_write_reg(udev, index, val);
}

/**
 * @ingroup etherspecific
 *
 * Set bits in a register on the Microchip LAN7800 USB Ethernet Adapter.
 *
 * @param udev
 * 		USB Device for the adapter
 * @param index
 * 		Index of the register to modify
 * @param set
 * 		Bits to set in the register. At positions where the is a 0, the old value
 * 		in the register will be written.
 *
 * @return
 * 		::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 * 		code.
 *
 */
	usb_status_t
lan7800_set_reg_bits(struct usb_device *udev, uint32_t index, uint32_t set)
{
	return lan7800_modify_reg(udev, index, 0xFFFFFFFF, set);
}

/**
 * @ingroup etherspecific
 * 
 * Change the MAC address of the Microchip LAN7800 USB Ethernet Adapter
 * on the actual hardware by writing to its registers.
 * 
 * @param udev
 *          	USB device for the adapter
 * @param macaddr
 *           	New MAC address to set (6 bytes long)
 * @return
 *      	::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *              code.  On failure the existing MAC address may have been partially
 *              modified.
 */     
	usb_status_t
lan7800_set_mac_address(struct usb_device *udev, const uint8_t *macaddr)
{
	usb_status_t status;
	uint32_t addrl, addrh;

	addrl = macaddr[0] | macaddr[1] << 8 | macaddr[2] << 16 | macaddr[3] << 24;
	addrh = macaddr[4] | macaddr[5] << 8;

	status = lan7800_write_reg(udev, LAN7800_ADDRL, addrl);
	if (status != USB_STATUS_SUCCESS)
	{
		kprintf("\r\nFailed to write low registers of MAC.\r\n");
		return status;
	}

	return lan7800_write_reg(udev, LAN7800_ADDRH, addrh);
}

/**
 * @ingroup etherspecific
 *
 * Reads the MAC address of the MICROCHIP LAN7800 USB Ethernet Adapter.
 *
 * @param udev
 *      	USB device for the adapter
 * @param macaddr
 *      	Pointer into which to write the MAC address (6 bytes long)
 * @return
 *       	::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *       	code.
 */
	usb_status_t
lan7800_get_mac_address(struct usb_device *udev, uint8_t *macaddr)
{
	usb_status_t status;
	uint32_t addrl, addrh;

	status = lan7800_read_reg(udev, LAN7800_ADDRL, &addrl);
	if (status != USB_STATUS_SUCCESS)
	{
		return status;
	}
	status = lan7800_read_reg(udev, LAN7800_ADDRH, &addrh);
	if (status != USB_STATUS_SUCCESS)
	{
		return status;
	}

	macaddr[0] = (addrl >> 0)  & 0xff;
	macaddr[1] = (addrl >> 8)  & 0xff;
	macaddr[2] = (addrl >> 16) & 0xff;
	macaddr[3] = (addrl >> 24) & 0xff;
	macaddr[4] = (addrh >> 0)  & 0xff;
	macaddr[5] = (addrh >> 8)  & 0xff;

	return USB_STATUS_SUCCESS;

}

int lan7800_mdio_wait_for_bit(struct usb_device *udev, const uint32_t reg,
			      const uint32_t mask, const bool set)
{
	uint32_t val = 0;

	while(1){
		lan7800_read_reg(udev, reg, &val);

		if(!set)
			val = ~val;
		if((val & mask) == mask)
			return 0;

		udelay(1);
	}
}

/**
 * @ingroup etherspecific
 *
 * Waits for phy to not be busy
 *
 * @param udev
 * 		USB device for the adapter
 *
 * @return
 * 		returns 0 on success; only returns when it is successful,
 * 		so it only returns a 0.
 */
static int lan7800_phy_wait_not_busy(struct usb_device *udev)
{
	return lan7800_mdio_wait_for_bit(udev, MII_ACC, MII_ACC_MII_BUSY, 0);
}

/* Read over the MII (bus between PHY layer and MAC layer on LAN7800). */
int lan7800_mdio_read(struct usb_device *udev, int phy_id, int idx)
{
	uint32_t val, addr;

	/* confirm MII is not busy */
	if (lan7800_phy_wait_not_busy(udev))
	{
		/* Should not reach this point... */
		return -1;
	}

	addr = (phy_id << 11) | (idx << 6) | MII_ACC_MII_READ | MII_ACC_MII_BUSY;
	lan7800_write_reg(udev, MII_ACC, addr);

	if (lan7800_phy_wait_not_busy(udev))
	{
		/* Should not reach this point... */
		return -1;
	}

	lan7800_read_reg(udev, MII_DATA, &val);

	return val & 0xFFFF;
}

/* Write over the MII (bus between PHY layer and MAC layer on LAN7800). */
void lan7800_mdio_write(struct usb_device *udev, int phy_id, int idx, int regval)
{
	uint32_t addr;

	/* confirm MII is not busy */
	if (lan7800_phy_wait_not_busy(udev))
	{
		return;
	}

	lan7800_write_reg(udev, MII_DATA, regval);

	/* set address, index, and direction (write to PHY) */
	addr = (phy_id << 11) | (idx << 6) | MII_ACC_MII_WRITE | MII_ACC_MII_BUSY;
	lan7800_write_reg(udev, MII_ACC, addr);

	if (lan7800_phy_wait_not_busy(udev))
		return;

}

/* lan7800_read_raw_otp.
 * LAN7800 infrastructure command to read from OTP (One-time programmable) registers
 * on the chip. Used as a helper function within lan7800_read_otp.
 *
 * Taken from the U-Boot implementation developed by MicroChip and Andrew Thomas:
 * https://github.com/u-boot/u-boot/blob/af15946aa081dbcd0bec7d507a2b2db4e6b6cda5/drivers/usb/eth/lan7800.c
 */ 
int lan7800_read_raw_otp(struct usb_device *udev, uint32_t offset,
		uint32_t length, uint8_t *data)
{
	int i;
	uint32_t buf;

	lan7800_read_reg(udev, LAN7800_OTP_PWR_DN, &buf);

	if (buf & LAN7800_OTP_PWR_DN_PWRDN_N) {

		/* clear it and wait to be cleared */
		lan7800_write_reg(udev, LAN7800_OTP_PWR_DN, 0);

		lan7800_wait_for_bit(udev, LAN7800_OTP_PWR_DN,
				LAN7800_OTP_PWR_DN_PWRDN_N,
				0);
	}

	for (i = 0; i < length; i++) {
		lan7800_write_reg(udev, LAN7800_OTP_ADDR1,
				((offset + i) >> 8) &
				LAN7800_OTP_ADDR1_15_11);
		lan7800_write_reg(udev, LAN7800_OTP_ADDR2,
				((offset + i) & LAN7800_OTP_ADDR2_10_3));

		lan7800_write_reg(udev, LAN7800_OTP_FUNC_CMD, LAN7800_OTP_FUNC_CMD_READ);
		lan7800_write_reg(udev, LAN7800_OTP_CMD_GO, LAN7800_OTP_CMD_GO_GO);

		lan7800_wait_for_bit(udev, LAN7800_OTP_STATUS,
				LAN7800_OTP_STATUS_BUSY,
				0);
		lan7800_read_reg(udev, LAN7800_OTP_RD_DATA, &buf);

		data[i] = (uint8_t)(buf & 0xFF);
	}

	return 0;
}

/* lan7800_read_otp
 * Reads from OTP register set. If successful, MAC will be readable from OTP.
 *
 * Taken from the U-Boot implementation developed by MicroChip and Andrew Thomas:
 * https://github.com/u-boot/u-boot/blob/af15946aa081dbcd0bec7d507a2b2db4e6b6cda5/drivers/usb/eth/lan7800.c
 */
int lan7800_read_otp(struct usb_device *udev, uint32_t offset,
		uint32_t length, uint8_t *data)
{
	uint8_t sig;
	lan7800_read_raw_otp(udev, 0, 1, &sig);

	if (sig == LAN7800_OTP_INDICATOR_1)
		offset = offset;
	else if (sig == LAN7800_OTP_INDICATOR_2)
		offset += 0x100;
	else
		return -EINVAL;
	lan7800_read_raw_otp(udev, offset, length, data);

	usb_dev_debug(dev, "LAN7800: MAC address from OTP = %pM\n", data);

	return OK;
}

/* lan7800_read_otp_mac
 * Reads MAC values from EEPROM.
 *
 * Taken from the U-Boot implementation developed by MicroChip and Andrew Thomas:
 * https://github.com/u-boot/u-boot/blob/af15946aa081dbcd0bec7d507a2b2db4e6b6cda5/drivers/usb/eth/lan7800.c
 */ 
int lan7800_read_otp_mac(unsigned char *enetaddr,
		struct usb_device *udev)
{
	int ret = 0;

	memset(enetaddr, 0, 6);

	lan7800_read_otp(udev,
			LAN7800_EEPROM_MAC_OFFSET,
			LAN7800_ETH_ALEN,
			enetaddr);
	if (!ret && is_valid_ethaddr(enetaddr)) {
		/* eeprom values are valid so use them */
		usb_dev_debug("MAC address read from OTP %pM\n", enetaddr);
		return 0;
	}
	usb_dev_debug("MAC address read from OTP invalid %pM\n", enetaddr);

	memset(enetaddr, 0, 6);
	return -EINVAL;
}

int lan7800_eeprom_confirm_not_busy(struct usb_device *udev)
{
	return lan7800_wait_for_bit(udev, LAN7800_E2P_CMD, LAN7800_E2P_CMD_EPC_BUSY,
			0);
}

int lan7800_wait_eeprom(struct usb_device *udev)
{
	return lan7800_wait_for_bit(udev, LAN7800_E2P_CMD, (LAN7800_E2P_CMD_EPC_BUSY
							  | LAN7800_E2P_CMD_EPC_TIMEOUT),
			0);
}

int lan7800_read_raw_eeprom(struct usb_device *dev, uint32_t offset,
		uint32_t length, uint8_t *data)
{
	uint32_t val;
	uint32_t saved;
	int i, ret = 0;
	int retval;

	/* depends on chip, some EEPROM pins are muxed with LED function.
	 * 	 * disable & restore LED function to access EEPROM.
	 * 	 	 */
	lan7800_read_reg(dev, LAN7800_HW_CFG, &val);
	saved = val;

	val &= ~(LAN7800_HW_CFG_LED1_EN | LAN7800_HW_CFG_LED0_EN);
	lan7800_write_reg(dev, LAN7800_HW_CFG, val);

	retval = lan7800_eeprom_confirm_not_busy(dev);
	if (retval)
		return retval;

	for (i = 0; i < length; i++) {
		val = LAN7800_E2P_CMD_EPC_BUSY | LAN7800_E2P_CMD_EPC_CMD_READ;
		val |= (offset & LAN7800_E2P_CMD_EPC_ADDR_MASK);
		lan7800_write_reg(dev, LAN7800_E2P_CMD, val);
		if (ret < 0) {
			retval = -EIO;
			goto exit;
		}

		retval = lan7800_wait_eeprom(dev);
		if (retval < 0)
			goto exit;

		lan7800_read_reg(dev, LAN7800_E2P_DATA, &val);
		if (ret < 0) {
			retval = -EIO;
			goto exit;
		}

		data[i] = val & 0xFF;
		offset++;
	}
	retval = 0;
exit:
	lan7800_write_reg(dev, LAN7800_HW_CFG, saved);

	return retval;
}

int lan7800_read_eeprom(struct usb_device *dev, uint32_t offset,
		uint32_t length, uint8_t *data)
{
	uint8_t sig;
	int ret = 0;

	lan7800_read_raw_eeprom(dev, 0, 1, &sig);
	if ((ret == 0) && (sig == LAN7800_EEPROM_INDICATOR))
		ret = lan7800_read_raw_eeprom(dev, offset, length, data);
	else
		ret = -EINVAL;

	return ret;
}

void lan7800_init_ltm(struct usb_device *dev)
{
	int ret = 0;
	uint32_t buf;
	uint32_t regs[6] = { 0 };

	lan7800_read_reg(dev, LAN7800_USB_CFG1, &buf);
	if (buf & LAN7800_USB_CFG1_LTM_ENABLE) {
		uint8_t temp[2];
		/* Get values from EEPROM first */
		if (lan7800_read_eeprom(dev, 0x3F, 2, temp) == 0) {
			if (temp[0] == 24) {
				lan7800_read_raw_eeprom(dev,
						temp[1] * 2,
						24,
						(uint8_t *)regs);
				if (ret < 0)
					return;
			}
		}
		else if (lan7800_read_otp(dev, 0x3F, 2, temp) == 0) {
			if (temp[0] == 24) {
				lan7800_read_raw_otp(dev,
						temp[1] * 2,
						24,
						(uint8_t *)regs);
				if (ret < 0)
					return;
			}
		}
	}

	lan7800_write_reg(dev, LAN7800_LTM_BELT_IDLE0, regs[0]);
	lan7800_write_reg(dev, LAN7800_LTM_BELT_IDLE1, regs[1]);
	lan7800_write_reg(dev, LAN7800_LTM_BELT_ACT0, regs[2]);
	lan7800_write_reg(dev, LAN7800_LTM_BELT_ACT1, regs[3]);
	lan7800_write_reg(dev, LAN7800_LTM_INACTIVE0, regs[4]);
	lan7800_write_reg(dev, LAN7800_LTM_INACTIVE1, regs[5]);
}

int lan7800_set_rx_max_frame_length(struct usb_device *dev, int size)
{
	uint32_t buf;
	bool rxenabled;

	lan7800_read_reg(dev, LAN7800_MAC_RX, &buf);

	rxenabled = ((buf & LAN7800_MAC_RX_RXEN) != 0);

	if (rxenabled) {
		buf &= ~LAN7800_MAC_RX_RXEN;
		lan7800_write_reg(dev, LAN7800_MAC_RX, buf);
	}

	/* add 4 to size for FCS */
	buf &= ~LAN7800_MAC_RX_MAX_SIZE_MASK;
	buf |= (((size + 4) << LAN7800_MAC_RX_MAX_SIZE_SHIFT) & LAN7800_MAC_RX_MAX_SIZE_MASK);

	lan7800_write_reg(dev, LAN7800_MAC_RX, buf);

	if (rxenabled) {
		buf |= LAN7800_MAC_RX_RXEN;
		lan7800_write_reg(dev, LAN7800_MAC_RX, buf);
	}

	return 0;
}


#define NETIF_F_RXCSUM			4
#define NETIF_F_HW_VLAN_CTAG_RX		2
#define NETIF_F_HW_VLAN_CTAG_FILTER	1

/* Enable or disable Rx checksum offload engine */
int lan7800_set_features(struct usb_device *dev, uint32_t features)
{
	uint32_t rfe_ctl;
	lan7800_read_reg(dev, LAN7800_RFE_CTL, &rfe_ctl);

	if (features & NETIF_F_RXCSUM) {
		rfe_ctl |= RFE_CTL_TCPUDP_COE | RFE_CTL_IP_COE;
		rfe_ctl |= RFE_CTL_ICMP_COE | RFE_CTL_IGMP_COE;
	}
	else {
		rfe_ctl &= ~(RFE_CTL_TCPUDP_COE | RFE_CTL_IP_COE);
		rfe_ctl &= ~(RFE_CTL_ICMP_COE | RFE_CTL_IGMP_COE);
	}

	if (features & NETIF_F_HW_VLAN_CTAG_RX)
		rfe_ctl |= RFE_CTL_VLAN_STRIP;
	else
		rfe_ctl &= ~RFE_CTL_VLAN_STRIP;

	if (features & NETIF_F_HW_VLAN_CTAG_FILTER)
		rfe_ctl |= RFE_CTL_VLAN_FILTER;
	else
		rfe_ctl &= ~RFE_CTL_VLAN_FILTER;

	lan7800_write_reg(dev, LAN7800_RFE_CTL, rfe_ctl);

	return 0;
}

int lan7800_reset(struct usb_device *dev, uint8_t* macaddress)
{
	uint32_t buf;

	lan7800_read_reg(dev, LAN7800_HW_CFG, &buf);
	buf |= LAN7800_HW_CFG_LRST;
	lan7800_write_reg(dev, LAN7800_HW_CFG, buf);

	lan7800_wait_for_bit(dev, LAN7800_HW_CFG, LAN7800_HW_CFG_LRST, 0);
  
	lan7800_set_mac_address(dev, macaddress);

	/* Respond to the IN token with a NAK */
  	lan7800_read_reg(dev, LAN7800_USB_CFG0, &buf);
  	buf |= LAN7800_USB_CFG_BIR;
  	lan7800_write_reg(dev, LAN7800_USB_CFG0, buf);

	/* Init LTM */
  	lan7800_init_ltm(dev);
  	buf = LAN7800_DEFAULT_BURST_CAP_SIZE / LAN7800_FS_USB_PKT_SIZE;
  	lan7800_write_reg(dev, LAN7800_BURST_CAP, buf);
  	lan7800_write_reg(dev, LAN7800_BULK_IN_DLY, LAN7800_DEFAULT_BULK_IN_DELAY);

  	lan7800_read_reg(dev, LAN7800_HW_CFG, &buf);
	buf |= LAN7800_HW_CFG_MEF;
	buf |= LAN7800_HW_CFG_LED0_EN;
	buf |= LAN7800_HW_CFG_LED1_EN;
	lan7800_write_reg(dev, LAN7800_HW_CFG, buf);
  
	lan7800_read_reg(dev, LAN7800_USB_CFG0, &buf);
	buf |= LAN7800_USB_CFG_BCE;
	lan7800_write_reg(dev, LAN7800_USB_CFG0, buf);

	/* set FIFO sizes */
	buf = (LAN7800_MAX_RX_FIFO_SIZE - 512) / 512;
	lan7800_write_reg(dev, LAN7800_FCT_RX_FIFO_END, buf);

	buf = (LAN7800_MAX_TX_FIFO_SIZE - 512) / 512;
	lan7800_write_reg(dev, LAN7800_FCT_TX_FIFO_END, buf);

  	lan7800_write_reg(dev, LAN7800_INT_STS, LAN7800_INT_STS_CLEAR_ALL);
  	lan7800_write_reg(dev, LAN7800_FLOW, 0);
  	lan7800_write_reg(dev, LAN7800_FCT_FLOW, 0);

	/* Don't need rfe_ctl_lock during initialisation */
	lan7800_read_reg(dev, LAN7800_RFE_CTL, &buf);
	buf |= (LAN7800_RFE_CTL_BCAST_EN | LAN7800_RFE_CTL_UCAST_EN | LAN7800_RFE_CTL_MCAST_EN);
	lan7800_write_reg(dev, LAN7800_RFE_CTL, buf);

	/* Enable or disable checksum offload engines */
	lan7800_set_features(dev, 0);

	lan7800_read_reg(dev, LAN7800_RFE_CTL, &buf);
	buf &= ~(LAN7800_RFE_CTL_DA_PERFECT | LAN7800_RFE_CTL_MCAST_HASH);
	lan7800_write_reg(dev, LAN7800_RFE_CTL, buf);

	/* reset PHY */
	lan7800_read_reg(dev, LAN7800_PMT_CTL, &buf);
	buf |= LAN7800_PMT_CTL_PHY_RST;
	lan7800_write_reg(dev, LAN7800_PMT_CTL, buf);

	/* ??? Could also wait for PMT_CTL_READY. */
	lan7800_wait_for_bit(dev, LAN7800_PMT_CTL, LAN7800_PMT_CTL_PHY_RST, 0);

	lan7800_read_reg(dev, LAN7800_MAC_CR, &buf);

	uint8_t sig;
	lan7800_read_raw_eeprom(dev, 0, 1, &sig);
	if (sig != LAN7800_EEPROM_INDICATOR) {
		usb_dev_debug(dev, "No External EEPROM. Setting MAC Speed\n");
		buf |= LAN7800_MAC_CR_AUTO_DUPLEX | LAN7800_MAC_CR_AUTO_SPEED;
	}

	lan7800_write_reg(dev, LAN7800_MAC_CR, buf);

	lan7800_read_reg(dev, LAN7800_MAC_TX, &buf);
	buf |= LAN7800_MAC_TX_TXEN;
	lan7800_write_reg(dev, LAN7800_MAC_TX, buf);

	lan7800_read_reg(dev, LAN7800_FCT_TX_CTL, &buf);
	buf |= LAN7800_FCT_TX_CTL_EN;
	lan7800_write_reg(dev, LAN7800_FCT_TX_CTL, buf);

  	lan7800_set_rx_max_frame_length(dev, LAN7800_ETH_MTU + LAN7800_ETH_VLAN_LEN);

	lan7800_read_reg(dev, LAN7800_MAC_RX, &buf);
	buf |= LAN7800_MAC_RX_RXEN;
	lan7800_write_reg(dev, LAN7800_MAC_RX, buf);

	lan7800_read_reg(dev, LAN7800_FCT_RX_CTL, &buf);
	buf |= LAN7800_FCT_RX_CTL_EN;
	lan7800_write_reg(dev, LAN7800_FCT_RX_CTL, buf);

	return 0;
}

