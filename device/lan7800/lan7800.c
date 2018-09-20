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

#include "lan7800.h"
#include <usb_core_driver.h>
#include <stdio.h>
#include <clock.h>

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

/* lan7800_wait_for_bit
 * Use the system clock to wait for a bit to be read over the LAN7800 chip.
 */
static int lan7800_wait_for_bit(struct usb_device *dev, const char *func,
		const uint32_t reg, const uint32_t mask,
		const bool set, const unsigned int timeout)
{
	u32 val;
	unsigned long start = clkcount();

	while (1) {
		lan7800_read_reg(udev, reg, &val);

		if (!set)
			val = ~val;

		if ((val & mask) == mask)
			return 0;

		/* Timeout if specified timeout is exceeded. */
		if ((platform.clkfreq + start) > timeout_ms)
			break;

		udelay(1);
	}

	debug("%s: Timeout (reg=0x%x mask=%08x wait_set=%i)\n", prefix, reg,
			mask, set);

	return -ETIMEDOUT;
}

/* lan7800_read_raw_otp.
 * LAN7800 infrastructure command to read from OTP (One-time programmable) registers
 * on the chip. Used as a helper function within lan7800_read_otp.
 *
 * Taken from the U-Boot implementation developed by MicroChip and Andrew Thomas:
 * https://github.com/u-boot/u-boot/blob/af15946aa081dbcd0bec7d507a2b2db4e6b6cda5/drivers/usb/eth/lan7800.c
 */ 
static int lan7800_read_raw_otp(struct usb_device *udev, u32 offset,
		u32 length, u8 *data)
{
	int i;
	uint32_t buf;

	lan7800_read_reg(udev, LAN7800_OTP_PWR_DN, &buf);

	if (buf & LAN7800_OTP_PWR_DN_PWRDN_N) {

		/* clear it and wait to be cleared */
		lan7800_write_reg(udev, LAN7800_OTP_PWR_DN, 0);

		lan7800_wait_for_bit(udev, "LAN7800_OTP_PWR_DN_PWRDN_N",
				LAN7800_OTP_PWR_DN,
				LAN7800_OTP_PWR_DN_PWRDN_N,
				false, 1000);
	}

	for (i = 0; i < length; i++) {
		lan7800_write_reg(udev, LAN7800_OTP_ADDR1,
				((offset + i) >> 8) &
				LAN7800_OTP_ADDR1_15_11);
		lan7800_write_reg(udev, LAN7800_OTP_ADDR2,
				((offset + i) & LAN7800_OTP_ADDR2_10_3));

		lan7800_write_reg(udev, LAN7800_OTP_FUNC_CMD, LAN7800_OTP_FUNC_CMD_READ);
		lan7800_write_reg(udev, LAN7800_OTP_CMD_GO, LAN7800_OTP_CMD_GO_GO);

		lan7800_wait_for_bit(udev, "LAN7800_OTP_STATUS_BUSY",
				LAN7800_OTP_STATUS,
				LAN7800_OTP_STATUS_BUSY,
				false, 1000);
		lan7800_read_reg(udev, LAN7800_OTP_RD_DATA, &buf);

		data[i] = (u8)(buf & 0xFF);
	}

	return 0;
}

/* lan7800_read_otp
 * Reads from OTP register set. If successful, MAC will be readable from OTP.
 *
 * Taken from the U-Boot implementation developed by MicroChip and Andrew Thomas:
 * https://github.com/u-boot/u-boot/blob/af15946aa081dbcd0bec7d507a2b2db4e6b6cda5/drivers/usb/eth/lan7800.c
 */
static int lan7800_read_otp(struct usb_device *udev, u32 offset,
		u32 length, u8 *data)
{
	u8 sig;
	lan7800_read_raw_otp(udev, 0, 1, &sig);

	if (sig == LAN7800_OTP_INDICATOR_1)
		offset = offset;
	else if (sig == LAN7800_OTP_INDICATOR_2)
		offset += 0x100;
	else
		return -EINVAL;
	lan7800_read_raw_otp(udev, offset, length, data);

	usb_dev_debug(dev, "LAN7800: MAC address from OTP = %pM\n", data);

	return ret;
}

/* lan7800_read_otp_mac
 * Reads MAC values from EEPROM.
 *
 * Taken from the U-Boot implementation developed by MicroChip and Andrew Thomas:
 * https://github.com/u-boot/u-boot/blob/af15946aa081dbcd0bec7d507a2b2db4e6b6cda5/drivers/usb/eth/lan7800.c
 */ 
static int lan7800_read_otp_mac(unsigned char *enetaddr,
		struct usb_device *udev)
{
	int ret;

	memset(enetaddr, 0, 6);

	ret = lan7800_read_otp(udev,
			EEPROM_MAC_OFFSET,
			ETH_ALEN,
			enetaddr);
	if (!ret && is_valid_ethaddr(enetaddr)) {
		/* eeprom values are valid so use them */
		debug("MAC address read from OTP %pM\n", enetaddr);
		return 0;
	}
	debug("MAC address read from OTP invalid %pM\n", enetaddr);

	memset(enetaddr, 0, 6);
	return -EINVAL;
}

static int lan7800_eeprom_confirm_not_busy(struct usb_device *udev)
{
	return lan7800_wait_for_bit(udev, __func__,
			E2P_CMD, E2P_CMD_EPC_BUSY,
			false, 100);
}

static int lan7800_wait_eeprom(struct usb_device *udev)
{
	return lan7800_wait_for_bit(udev, __func__,
			E2P_CMD,
			(E2P_CMD_EPC_BUSY | E2P_CMD_EPC_TIMEOUT),
			false, 100);
}

static int lan7800_read_eeprom(struct usb_device *udev,
		u32 offset, u32 length, u8 *data)
{
	u32 val;
	int i, ret;

	ret = lan7800_eeprom_confirm_not_busy(udev);
	if (ret)
		return ret;

	for (i = 0; i < length; i++) {
		val = E2P_CMD_EPC_BUSY | E2P_CMD_EPC_CMD_READ |
			(offset & E2P_CMD_EPC_ADDR_MASK);
		lan7800_write_reg(udev, E2P_CMD, val);

		ret = lan7800_wait_eeprom(udev);
		if (ret)
			return ret;

		lan7800_read_reg(udev, E2P_DATA, &val);
		data[i] = val & 0xFF;
		offset++;
	}
	return ret;
}
