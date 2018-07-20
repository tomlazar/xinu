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
 *
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
 *
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
	kprintf("\r\nSETTING MAC ADDRESS.\r\n");
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
