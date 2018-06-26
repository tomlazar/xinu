/**
 * @file	lan7800.c
 *
 * @authors
 * 			Rade Latinovich
 * 			Patrick J. McGee
 *
 * This file provides various functions needed by the LAN7800 USB Ethernet Driver.
 */
/* Embedded Xinu, Copyright (C) 2018. All rights reserved. */

#include "lan7800.h"
#include <usb_core_driver.h>

/**
 * @ingroup etherspecific
 *
 * Write to a register on the LAN7800 USB Ethernet Adapter.
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
							LAN7800_VENDOR_REQUEST_WRITE_REGISTER,
							USB_BMREQUESTTYPE_DIR_OUT |
								USB_BMREQUESTTYPE_TYPE_VENDOR |
								USB_BMREQUESTTYPE_REVIPIENT_DEVICE,
							0, index, &data, sizeof(uint32_t));
}

/**
 * @ingroup etherspecific
 *
 * Read from a register on the LAN7800 USB Ethernet Adapter.
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
lan7800_read_reg(struct usb_device *dev, uint32_t index, uint32_t *data)
{
	return usb_control_msg(udev, NULL,
							LAN7800_VENDOR_REQUEST_READ_REGISTER,
							USB_BMREQUESTTYPE_DIR_IN |
								USB_BMREQUESTTYPE_TYPE_VENDOR |
								USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
							0, index, data, sizeof(uint32_t));
}
