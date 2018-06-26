/**
 * @file lan7800.h
 * 
 * @authors
 * 		Rade Latinovich
 * 		Patrick J. McGee
 *
 * This header file provides definitions of the registers of the LAN7800 USB
 * Ethernet Adapter. Many of these definitions were borrowed primarily from
 * Microchip's LAN78XX Linux Driver
 */

#ifndef _LAN7800_H_
#define _LAN7800_H_

#include "usb_util.h"

#define LAN7800_VENDOR_ID	0x424
#define LAN7800_PRODUCT_ID	0x7800

usb_status_t lan7800_write_reg(struct usb_device *udev, uint32_t index, uint32_t data);
usb_status_t lan7800_read_reg(struct usb_device *udev, uint32_t index, uint32_t *data);

usb_status_t lan7800_modify_reg(struct usb_device *udev, uint32_t index,
								uint32_t mask, uint32_t set);
usb_status_t lan7800_set_reg_bits(struct usb_device *udev, uint32_t index, uint32_t set);

usb_status_t lan7800_wait_device_attached(ushort minor);

usb_status_t lan7800_set_mac_address(struct usb_device *udev, const uint8_t *macaddr);
usb_status_t lan7800_get_mac_address(struct usb_device *udev, uint8_t *macaddr);

struct usb_xfer_request;

void lan7800_rx_complete(struct usb_xfer_request *req);
void lan7800_tx_complete(struct usb_xfer_request *req);

static inline void
__lan7800_dump_reg(struct usb_device *udev, uint32_t index, const char *name)
{
	uint32_t val = 0;
	lan7800_read_reg(udev, index, &val);
	kprintf("LAN7800: %s = 0x%08X\n", name, val);
}

#define lan7800_dump_reg(udev, index) __lan7800_dump_reg(udev, index, #index)

/***************************************************************************/

#define LAN7800_VENDOR_REQUEST_WRITE_REGISTER	0xA0
#define LAN7800_VENDOR_REQUEST_READ_REGISTER	0xA1
#define LAN7800_VENDER_REQUEST_GET_STATS		0xA2

#endif	/* _LAN7800_H_ */
