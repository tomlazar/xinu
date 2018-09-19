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
#include <clock.h>		/* for udelay() */

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

/* lan7800_mdio methods */
int  lan7800_mdio_read (struct usb_device *udev, int phy_id, int idx);
void lan7800_mdio_write(struct usb_device *udev, int phy_id, int idx, int regval);

static inline int lan7800_mdio_wait_for_bit(struct usb_device *udev,
					const uint32_t reg,
					const uint32_t mask,
					const bool set)
{
	uint32_t val;

	while(1)
	{
		lan7800_read_reg(udev, reg, &val);

		if (!set)
			val = ~val;

		if ((val & mask) == mask)
			return 0;

		udelay(1);
	}
}


/***************************************************************************
 * According to Linux's open-source 78xx driver:
 * https://github.com/torvalds/linux/blob/8fa3b6f9392bf6d90cb7b908e07bd90166639f0a/drivers/net/usb/lan78xx.h
 * and also U-Boot:
 * https://github.com/trini/u-boot/blob/890e79f2b1c26c5ba1a86d179706348aec7feef7/drivers/usb/eth/lan7x.h
 ***************************************************************************/

/* TX command word A */
#define TX_CMD_A_IGE_			(0x20000000)
#define TX_CMD_A_ICE_			(0x10000000)

/* ??? LSO = Last segment o... */
#define TX_CMD_A_LSO			(0x08000000)

#define TX_CMD_A_IPE_			(0x04000000)
#define TX_CMD_A_TPE_			(0x02000000)
#define TX_CMD_A_IVTG_			(0x01000000)
#define TX_CMD_A_RVTG_			(0x00800000)

/* ??? FCS = first command segment? */
#define TX_CMD_A_FCS			(0x00400000)

/* TX word A buffer size. */
#define LAN7800_TX_CMD_A_BUF_SIZE	(0x000FFFFF)

/* TX command word B */
#define TX_CMD_B_MSS_SHIFT_		(16)
#define TX_CMD_B_MSS_MASK_		(0x3FFF0000)
#define TX_CMD_B_MSS_MIN_		((unsigned short)8)
#define TX_CMD_B_VTAG_MASK_		(0x0000FFFF)
#define TX_CMD_B_VTAG_PRI_MASK_		(0x0000E000)
#define TX_CMD_B_VTAG_CFI_MASK_		(0x00001000)
#define TX_CMD_B_VTAG_VID_MASK_		(0x00000FFF)


/* TX/RX Overhead
 * Used in sum to allocate for Tx transfer buffer in etherOpen() */
#define LAN7800_TX_OVERHEAD			8
#define LAN7800_RX_OVERHEAD                     4

/* Max Receive requests, allocated in etherOpen() */ 
#define LAN7800_MAX_RX_REQUESTS 		1

/* Max Transmit requests, allocated in etherOpen() */
#define LAN7800_MAX_TX_REQUESTS 		1

/* According to U-Boot's LAN78xx driver. */
#define LAN7800_HS_USB_PKT_SIZE			512

/* Read/write regsiters. */
#define LAN7800_VENDOR_REQUEST_WRITE		0xA0
#define LAN7800_VENDOR_REQUEST_READ		0xA1
#define LAN7800_VENDER_REQUEST_GET_STATS	0xA2

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

/* LED definitions. */
#define LAN7800_HW_CFG_LED1_EN		1<<21 /* Muxed with EEDO */
#define LAN7800_HW_CFG_LED0_EN		1<<20 /* Muxed with EECLK */

/* Multiple Ethernet Frames. */
#define LAN7800_HW_CFG_MEF		(0x00000010)

/* Lite reset flag. */
#define LAN7800_HW_CFG_LRST		(0x00000002)

/* ??? */
#define LAN7800_HW_CFG_SRST		(0x00000001)

/* Offset of Burst Cap Register. HW_CFG_MEF bust be set first. */
#define LAN7800_BURST_CAP		(0x090)
#define BURST_CAP_SIZE_MASK		(0x000000FF)

/* Loopback mode. */
#define MAC_CR_LOOPBACK			(0x00000400)

/* Received Ethernet Frame Length */
#define LAN7800_RX_STS_FL		(0x00003FFF)

/* ??? Not entirely sure, appears to be Received Ethernet Error Summary. */
#define LAN7800_RX_CMD_A_RX_ERR		(0xC03F0000)

/* The following register definitions were pulled from U-Boot's implementation:
 * https://github.com/Screenly/u-boot/blob/master/drivers/usb/eth/lan78xx.c
 */
#define LAN7800_BULK_IN_DLY		0x094
#define LAN7800_DEFAULT_BULK_IN_DLY	0x0800
#define LAN7800_INT_STS			0x0C

#define LAN7800_FCT_RX_FIFO_END		0x0C8
#define LAN7800_MAX_RX_FIFO_SIZE	(12 * 1024)

#define LAN7800_FCT_TX_FIFO_END         0x0CC
#define LAN7800_MAX_TX_FIFO_SIZE        (12 * 1024)

/* For initializing Tx. */
#define LAN7800_TX_FLOW			0x0D0

/* For initializing Rx. */
#define LAN7800_RFE_CTL			0x0B0
#define LAN7800_RFE_CTL_BCAST_EN	(1 << 10)
#define LAN7800_RFE_CTL_DA_PERFECT	(1 << 1)

/* MAC functions. */
#define LAN7800_MAC_CR_AUTO_DUPLEX	(1 << 12)
#define MAC_CR_AUTO_SPEED		(1 << 11)

/* Adaptive & Dynamic Polling at MAC layer ??? */
#define MAC_CR_ADP			(1 << 13)

#define LAN7800_MAC_TX				0x108
#define MAC_TX_TXEN			(1 << 0)

#define LAN7800_FCT_TX_CTL		0x0C4
#define LAN7800_FCT_TX_CTL_EN		(1 << 31)

#define MAC_RX                          0x104
#define MAC_RX_RXEN			(1 << 0)

#define LAN7800_ETH_FRAME_LEN		1514
#define LAN7X_MAC_RX_MAX_SIZE_DEFAULT \
		LAN7X_MAC_RX_MAX_SIZE(LAN7800_ETH_FRAME_LEN + 4 /* VLAN */ + 4 /* CRC */)

#define LAN7800_MAC_RX_FCS_STRIP	(1 << 4)

#define LAN7800_FCT_RX_CTL              0x0C0
#define LAN7800_FCT_TX_CTL_EN           (1 << 31)

/* MAF is where U-Boot writes the hardware address. */
#define LAN78XX_MAF_BASE		0x400
#define LAN78XX_MAF_HIX			0x00
#define LAN78XX_MAF_LOX			0x04
#define LAN78XX_MAF_HI_BEGIN		(LAN78XX_MAF_BASE + LAN78XX_MAF_HIX)
#define LAN78XX_MAF_LO_BEGIN		(LAN78XX_MAF_BASE + LAN78XX_MAF_LOX)
#define LAN78XX_MAF_HI(index)		(LAN78XX_MAF_BASE + (8 * (index)) + \
							LAN78XX_MAF_HIX)
#define LAN78XX_MAF_LO(index)		(LAN78XX_MAF_BASE + (8 * (index)) + \
							LAN78XX_MAF_LOX)
#define LAN78XX_MAF_HI_VALID		BIT(31)


/* MII_ACC */
#define MII_ACC				(0x120)
#define MII_ACC_MII_READ		(0x0)
#define MII_ACC_MII_WRITE		(0x2)
#define MII_ACC_MII_BUSY		(1 << 0)

#define MII_DATA			(0x124)

#endif	/* _LAN7800_H_ */
