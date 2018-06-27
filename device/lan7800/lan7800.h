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
	kprintf("LAN7800: %s = 0x%08X\r\n", name, val);
}

#define lan7800_dump_reg(udev, index) __lan7800_dump_reg(udev, index, #index)

/***************************************************************************
 * According to Linux's open-source 78xx driver:
 * https://github.com/torvalds/linux/blob/8fa3b6f9392bf6d90cb7b908e07bd90166639f0a/drivers/net/usb/lan78xx.h
***************************************************************************/

/* TX/RX Overhead
 * Used in sum to allocate for Tx transfer buffer in etherOpen() */
#define LAN7800_TX_OVERHEAD			8
#define LAN7800_RX_OVERHEAD                     4

#define LAN7800_MAX_TX_REQUESTS			1

/* Left as a TODO in smsc... */
#define LAN7800_MAX_RX_REQUESTS                 1

#define LAN7800_HS_USB_PKT_SIZE			512
#define LAN7800_DEFAULT_HS_BURST_CAP_SIZE	(16 * 1024 + 5 * LAN7800_HS_USB_PKT_SIZE)

#define LAN7800_VENDOR_REQUEST_WRITE		0xA0
#define LAN7800_VENDOR_REQUEST_READ		0xA1
#define LAN7800_VENDER_REQUEST_GET_STATS	0xA2

/* MAC offset */
#define LAN7800_MAC_CR				0x100
#define LAN7800_MAC_CR_TXEN			0x00000001
#define LAN7800_MAC_CR_RXEN			0x00000001

/* ??? TODO: Need for ethOpen() completion.
#define LAN7800_TX_CFG
#define LAN8900_TX_CFG_ON
*/

/* MAC TX/RX */
#define LAN8900_MAC_RX				0x104
#define LAN7800_MAC_TX				0x108

/* High and low RX register offsets */
#define LAN7800_ADDRH				0x118
#define LAN7800_ADDRL				0x11C

#endif	/* _LAN7800_H_ */
