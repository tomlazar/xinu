/**
 * @file	lan7800.c
 *
 * @authors
 * 		Rade Latinovich
 * 		Patrick J. McGee
 *
 * This file provides various functions needed by the Microchip LAN7800 USB Ethernet Driver,
 * equipped on the Raspberry Pi 3 Model B+.
 *
 * These functions are based on those of the LAN78XX drivers found in:
 * 	- The Linux Kernel
 * 	  (https://github.com/torvalds/linux/blob/master/drivers/net/usb/lan78xx.c)
 *	- U-Boot
 *	  (https://github.com/u-boot/u-boot/blob/af15946aa081dbcd0bec7d507a2b2db4e6b6cda5/drivers/usb/eth/lan78xx.c)
 *
 * Embedded Xinu, Copyright (C) 2018. All rights reserved.
 */

#include "lan7800.h"
#include <usb_core_driver.h>
#include <stdio.h>
#include <clock.h>
#include <string.h>

/**
 * Write to a register on the Microchip LAN7800 USB Ethernet Adapter.
 * @param udev
 * 		USB device for the adapter
 * @param index
 * 		Index of the register to write
 * @param data
 * 		Value to write to the register
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
 * Read from a register on the Microchip LAN7800 USB Ethernet Adapter.
 * @param udev
 * 		USB device for the adapter
 * @param index
 * 		Index of the register to read
 * @param data
 * 		Pointer into which to write the register's value
 * @return
 * 		::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 * 		code.
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
 * Modify the value contained in a register on the Microchip LAN7800 USB Ethernet
 * Adapter.
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
 * Set bits in a register on the Microchip LAN7800 USB Ethernet Adapter.
 * @param udev
 * 		USB Device for the adapter
 * @param index
 * 		Index of the register to modify
 * @param set
 * 		Bits to set in the register. At positions where the is a 0, the old value
 * 		in the register will be written.
 * @return
 * 		::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 * 		code.
 */
usb_status_t lan7800_set_reg_bits(struct usb_device *udev, uint32_t index, uint32_t set)
{
	return lan7800_modify_reg(udev, index, 0xFFFFFFFF, set);
}

/**
 * Change the MAC address of the Microchip LAN7800 USB Ethernet Adapter
 * on the actual hardware by writing to its registers.
 * @param udev
 *          	USB device for the adapter
 * @param macaddr
 *           	New MAC address to set (6 bytes long)
 * @return
 *      	::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *              code.  On failure the existing MAC address may have been partially
 *              modified.
 */     
usb_status_t lan7800_set_mac_address(struct usb_device *udev, const uint8_t *macaddr)
{
	usb_status_t status;
	uint32_t addrl, addrh;

	addrl = macaddr[0] | macaddr[1] << 8 | macaddr[2] << 16 | macaddr[3] << 24;
	addrh = macaddr[4] | macaddr[5] << 8;

	status = lan7800_write_reg(udev, LAN7800_ADDRL, addrl);
	if (status != USB_STATUS_SUCCESS)
	{
		kprintf("\r\nERROR: Failed to write low registers of MAC address\r\n");
		return status;
	}

	/* Try to write the MAC until there are nonzero values read back from the mailbox */

	return lan7800_write_reg(udev, LAN7800_ADDRH, addrh);
}

/**
 * Reads the MAC address of the MICROCHIP LAN7800 USB Ethernet Adapter.
 * @param udev
 *      	USB device for the adapter
 * @param macaddr
 *      	Pointer into which to write the MAC address (6 bytes long)
 * @return
 *       	::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *       	code.
 */
usb_status_t lan7800_get_mac_address(struct usb_device *udev, uint8_t *macaddr)
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
/**
 * Wait for a bit value to change on the Microchip LAN7800 USB Ethernet Adapter.
 * @param udev
 *      	USB device for the adapter
 * @param reg
 *      	Register to change
 * @param mask
 * 		Mask value for register
 * @param set
 * 		Value of register bit to apply
 * @return
 *       	::USB_STATUS_SUCCESS after value is applied
 */
usb_status_t lan7800_mdio_wait_for_bit(struct usb_device *udev, const uint32_t reg,
		const uint32_t mask, const bool set)
{
	uint32_t val = 0;
	while(1){
		lan7800_read_reg(udev, reg, &val);

		if(!set)
			val = ~val;
		if((val & mask) == mask)
			return USB_STATUS_SUCCESS;
		udelay(1);
	}
}

/**
 * Waits for PHY to be free.
 * @param udev
 * 		USB device for the adapter
 * @return
 * 		returns ::USB_STATUS_SUCCESS on success.
 */
usb_status_t lan7800_phy_wait_not_busy(struct usb_device *udev)
{
	return lan7800_mdio_wait_for_bit(udev, MII_ACC, MII_ACC_MII_BUSY, USB_STATUS_SUCCESS);
}

/* Before waiting, confirm that EEPROM is not busy. (Helper function) */
usb_status_t lan7800_eeprom_confirm_not_busy(struct usb_device *udev)
{
	return lan7800_wait_for_bit(udev, LAN7800_E2P_CMD, LAN7800_E2P_CMD_EPC_BUSY,
			USB_STATUS_SUCCESS);
}

/* Wait for EEPROM. (Helper function) */
usb_status_t lan7800_wait_eeprom(struct usb_device *udev)
{
	return lan7800_wait_for_bit(udev, LAN7800_E2P_CMD, (LAN7800_E2P_CMD_EPC_BUSY
				| LAN7800_E2P_CMD_EPC_TIMEOUT),
			USB_STATUS_SUCCESS);
}

/**
 * Read raw EEPROM from the Microchip LAN7800 Ethernet device.
 * @param udev
 * 		USB device for the adapter
 * @return
 * 		returns ::USB_STATUS_SUCCESS on success.
 */
usb_status_t lan7800_read_raw_eeprom(struct usb_device *udev, uint32_t offset,
		uint32_t length, uint8_t *data)
{
	uint32_t val;
	uint32_t saved;
	int i, ret = 0;
	int retval;

	lan7800_read_reg(udev, LAN7800_HW_CFG, &val);
	saved = val;

	/* Disable and restore LED function to access EEPROM.
	 * EEPROM is muxed with LED function on the LAN7800. */
	val &= ~(LAN7800_HW_CFG_LED1_EN | LAN7800_HW_CFG_LED0_EN);
	lan7800_write_reg(udev, LAN7800_HW_CFG, val);

	retval = lan7800_eeprom_confirm_not_busy(udev);
	if (retval)
		return retval;

	for (i = 0; i < length; i++) {
		val = LAN7800_E2P_CMD_EPC_BUSY | LAN7800_E2P_CMD_EPC_CMD_READ;
		val |= (offset & LAN7800_E2P_CMD_EPC_ADDR_MASK);
		lan7800_write_reg(udev, LAN7800_E2P_CMD, val);
		if (ret < 0) {
			retval = -EIO;
			goto exit;
		}

		retval = lan7800_wait_eeprom(udev);
		if (retval < 0)
			goto exit;

		lan7800_read_reg(udev, LAN7800_E2P_DATA, &val);
		if (ret < 0) {
			retval = -EIO;
			goto exit;
		}

		data[i] = val & 0xFF;
		offset++;
	}
	retval = USB_STATUS_SUCCESS;
exit:
	lan7800_write_reg(udev, LAN7800_HW_CFG, saved);

	return retval;
}

/**
 * Set max RX frame length for the Microchip LAN7800 Ethernet device.
 * @param udev
 * 		USB device for the adapter
 * @param size
 * 		Size of max RX frame
 * @return
 * 		returns ::USB_STATUS_SUCCESS on success.
 */
usb_status_t lan7800_set_rx_max_frame_length(struct usb_device *udev, int size)
{
	uint32_t buf;
	bool rxenabled;

	lan7800_read_reg(udev, LAN7800_MAC_RX, &buf);

	rxenabled = ((buf & LAN7800_MAC_RX_RXEN) != 0);

	if (rxenabled) {
		buf &= ~LAN7800_MAC_RX_RXEN;
		lan7800_write_reg(udev, LAN7800_MAC_RX, buf);
	}

	/* To fit FCS, add 4 */
	buf &= ~LAN7800_MAC_RX_MAX_SIZE_MASK;
	buf |= (((size + 4) << LAN7800_MAC_RX_MAX_SIZE_SHIFT) & LAN7800_MAC_RX_MAX_SIZE_MASK);

	lan7800_write_reg(udev, LAN7800_MAC_RX, buf);

	if (rxenabled) {
		buf |= LAN7800_MAC_RX_RXEN;
		lan7800_write_reg(udev, LAN7800_MAC_RX, buf);
	}

	return USB_STATUS_SUCCESS;
}

/**
 * Enable or disable Rx checksum offload engine for the Microchip LAN7800 Ethernet device.
 * @param udev
 * 		USB device for the adapter
 * @param set
 * 		Bit (whether to enable or disable)
 * @return
 * 		returns ::USB_STATUS_SUCCESS on success.
 */
usb_status_t lan7800_set_features(struct usb_device *udev, uint32_t set)
{
	uint32_t rfe_ctl;
	lan7800_read_reg(udev, LAN7800_RFE_CTL, &rfe_ctl);

	if (set & NETIF_F_RXCSUM) {
		rfe_ctl |= RFE_CTL_TCPUDP_COE | RFE_CTL_IP_COE;
		rfe_ctl |= RFE_CTL_ICMP_COE | RFE_CTL_IGMP_COE;
	}
	else {
		rfe_ctl &= ~(RFE_CTL_TCPUDP_COE | RFE_CTL_IP_COE);
		rfe_ctl &= ~(RFE_CTL_ICMP_COE | RFE_CTL_IGMP_COE);
	}

	if (set & NETIF_F_HW_VLAN_CTAG_RX)
		rfe_ctl |= RFE_CTL_VLAN_STRIP;
	else
		rfe_ctl &= ~RFE_CTL_VLAN_STRIP;

	if (set & NETIF_F_HW_VLAN_CTAG_FILTER)
		rfe_ctl |= RFE_CTL_VLAN_FILTER;
	else
		rfe_ctl &= ~RFE_CTL_VLAN_FILTER;

	lan7800_write_reg(udev, LAN7800_RFE_CTL, rfe_ctl);

	return USB_STATUS_SUCCESS;
}

/**
 * Initialize various functions for the Microchip LAN7800 Ethernet device.
 * @param udev
 * 		USB device for the adapter
 * @param macaddress
 * 		MAC address to be set on the device (obtained from the BCM2837B0 mailbox)
 * @return
 * 		returns ::USB_STATUS_SUCCESS on success.
 */
usb_status_t lan7800_init(struct usb_device *udev, uint8_t* macaddress)
{
	uint32_t buf;

	/* Set the MAC address on the device. */
	lan7800_set_mac_address(udev, macaddress);

	/* Respond to the IN token with a NAK */
	lan7800_read_reg(udev, LAN7800_USB_CFG0, &buf);
	buf |= LAN7800_USB_CFG_BIR;
	lan7800_write_reg(udev, LAN7800_USB_CFG0, buf);

	/* Set burst cap. */
	buf = LAN7800_DEFAULT_BURST_CAP_SIZE / LAN7800_FS_USB_PKT_SIZE;
	lan7800_write_reg(udev, LAN7800_BURST_CAP, buf);
	lan7800_write_reg(udev, LAN7800_BULK_IN_DLY, LAN7800_DEFAULT_BULK_IN_DELAY);

	/* Enable LED over HW CFG. */
	lan7800_read_reg(udev, LAN7800_HW_CFG, &buf);
	buf |= LAN7800_HW_CFG_MEF; /* Multiple ethernet frames */
	buf |= LAN7800_HW_CFG_LED0_EN;
	buf |= LAN7800_HW_CFG_LED1_EN;
	lan7800_write_reg(udev, LAN7800_HW_CFG, buf);

	lan7800_read_reg(udev, LAN7800_USB_CFG0, &buf);
	buf |= LAN7800_USB_CFG_BCE;
	lan7800_write_reg(udev, LAN7800_USB_CFG0, buf);

	/* Set FIFO sizes (similar to SMSC9512) */
	buf = (LAN7800_MAX_RX_FIFO_SIZE - 512) / 512;
	lan7800_write_reg(udev, LAN7800_FCT_RX_FIFO_END, buf);

	buf = (LAN7800_MAX_TX_FIFO_SIZE - 512) / 512;
	lan7800_write_reg(udev, LAN7800_FCT_TX_FIFO_END, buf);

	lan7800_write_reg(udev, LAN7800_INT_STS, LAN7800_INT_STS_CLEAR_ALL);
	lan7800_write_reg(udev, LAN7800_FLOW, 0);
	lan7800_write_reg(udev, LAN7800_FCT_FLOW, 0);

	lan7800_read_reg(udev, LAN7800_RFE_CTL, &buf);
	buf |= (LAN7800_RFE_CTL_BCAST_EN | LAN7800_RFE_CTL_UCAST_EN | LAN7800_RFE_CTL_MCAST_EN);
	lan7800_write_reg(udev, LAN7800_RFE_CTL, buf);

	/* Disable checksum offload engines */
	lan7800_set_features(udev, 0);

	/* Do not filter packets. This is done in the network functions stack. */
	lan7800_read_reg(udev, LAN7800_RFE_CTL, &buf);
	buf &= ~(LAN7800_RFE_CTL_DA_PERFECT | LAN7800_RFE_CTL_MCAST_HASH);
	lan7800_write_reg(udev, LAN7800_RFE_CTL, buf);

	lan7800_read_reg(udev, LAN7800_MAC_CR, &buf);

	/* Set MAC speed */
	uint8_t sig;
	lan7800_read_raw_eeprom(udev, 0, 1, &sig);
	if (sig != LAN7800_EEPROM_INDICATOR) {
		usb_dev_debug(udev, "No External EEPROM. Setting MAC Speed\n");
		buf |= LAN7800_MAC_CR_AUTO_DUPLEX | LAN7800_MAC_CR_AUTO_SPEED;
	}

	buf &= ~(LAN7800_MAC_CR_AUTO_DUPLEX);
	lan7800_write_reg(udev, LAN7800_MAC_CR, buf);

	/* Full duplex mode. */
	lan7800_read_reg(udev, LAN7800_MAC_CR, &buf);
	buf |= (1 << 3);
	lan7800_write_reg(udev, LAN7800_MAC_CR, buf);

	/* Set TX, RX registers for MAC, FCT (similar to SMSC9512) */
	lan7800_read_reg(udev, LAN7800_MAC_TX, &buf);
	buf |= LAN7800_MAC_TX_TXEN;
	lan7800_write_reg(udev, LAN7800_MAC_TX, buf);

	lan7800_read_reg(udev, LAN7800_FCT_TX_CTL, &buf);
	buf |= LAN7800_FCT_TX_CTL_EN;
	lan7800_write_reg(udev, LAN7800_FCT_TX_CTL, buf);

	lan7800_set_rx_max_frame_length(udev, LAN7800_ETH_MTU + LAN7800_ETH_VLAN_LEN);

	lan7800_read_reg(udev, LAN7800_MAC_RX, &buf);
	buf |= LAN7800_MAC_RX_RXEN;
	lan7800_write_reg(udev, LAN7800_MAC_RX, buf);

	lan7800_read_reg(udev, LAN7800_FCT_RX_CTL, &buf);
	buf |= LAN7800_FCT_RX_CTL_EN;
	lan7800_write_reg(udev, LAN7800_FCT_RX_CTL, buf);

	return USB_STATUS_SUCCESS;
}

