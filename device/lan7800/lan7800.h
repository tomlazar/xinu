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

/* ??? */
#define LAN7800_HS_USB_PKT_SIZE			512

/* Cannot say for sure what this means. Calculation from SMSC driver. */
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
#define LAN7800_MAC_RX				0x104

/* High and low RX register offsets */
#define LAN7800_ADDRH				0x118
#define LAN7800_ADDRL				0x11C

/* Offset of Hardware Configuration Register. */
#define LAN7800_HW_CFG				(0x010)

/* ??? A bunch of other HWC registers. */
#define HW_CFG_CLK125_EN_		(0x02000000)
#define HW_CFG_REFCLK25_EN_		(0x01000000)
#define HW_CFG_LED3_EN_			(0x00800000)
#define HW_CFG_LED2_EN_			(0x00400000)
#define HW_CFG_LED1_EN_			(0x00200000)
#define HW_CFG_LED0_EN_			(0x00100000)
#define HW_CFG_EEE_PHY_LUSU_		(0x00020000)
#define HW_CFG_EEE_TSU_			(0x00010000)
#define HW_CFG_NETDET_STS_		(0x00008000)
#define HW_CFG_NETDET_EN_		(0x00004000)
#define HW_CFG_EEM_			(0x00002000)
#define HW_CFG_RST_PROTECT_		(0x00001000)
#define HW_CFG_CONNECT_BUF_		(0x00000400)
#define HW_CFG_CONNECT_EN_		(0x00000200)
#define HW_CFG_CONNECT_POL_		(0x00000100)
#define HW_CFG_SUSPEND_N_SEL_MASK_	(0x000000C0)
#define HW_CFG_SUSPEND_N_SEL_2		(0x00000000)
#define HW_CFG_SUSPEND_N_SEL_12N	(0x00000040)
#define HW_CFG_SUSPEND_N_SEL_012N	(0x00000080)
#define HW_CFG_SUSPEND_N_SEL_0123N	(0x000000C0)
#define HW_CFG_SUSPEND_N_POL_		(0x00000020)
#define HW_CFG_ETC_			(0x00000008)

/* Multiple Ethernet Frames. */
#define LAN7800_HW_CFG_MEF			(0x00000010)

/* Lite reset flag. */
#define LAN7800_HW_CFG_LRST			(0x00000002)

/* ??? */
#define LAN7800_HW_CFG_SRST			(0x00000001)

/* Offset of Burst Cap Register. HW_CFG_MEF bust be set first. */
#define LAN7800_BURST_CAP			(0x090)
#define BURST_CAP_SIZE_MASK		(0x000000FF)

/* Bulk In ??? */
#define LAN7800_CFG_BIR			(0x00000040)

/* ??? */
#define LAN7800_CFG_BCE			(0x00000020)

/* Mac-layer transmission. */
#define LAN7800_MAC_TX				(0x108)

/* Transmit enable at MAC layer ??? */
#define LAN7800_MAC_TX_TXEN			(0x00000001)

#endif	/* _LAN7800_H_ */
