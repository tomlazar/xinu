/**
 * @file lan7800.h
 * 
 * @authors
 * 		Rade Latinovich
 *		Patrick J. McGee
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
usb_status_t lan7800_modify_reg(struct usb_device *udev, uint32_t index, uint32_t mask, uint32_t set);
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
usb_status_t  lan7800_mdio_read (struct usb_device *udev, int phy_id, int idx);
void lan7800_mdio_write(struct usb_device *udev, int phy_id, int idx, int regval);

/* LAN7800 function declarations (defined in lan7800.c) */
usb_status_t lan7800_read_raw_otp(struct usb_device *udev, uint32_t offset,
				uint32_t length, uint8_t *data);
int lan7800_read_otp_mac(unsigned char *enetaddr,
		struct usb_device *udev);
usb_status_t lan7800_eeprom_confirm_not_busy(struct usb_device *udev);
usb_status_t lan7800_wait_eeprom(struct usb_device *udev);
usb_status_t lan7800_read_raw_eeprom(struct usb_device *dev, uint32_t offset,
		uint32_t length, uint8_t *data);
usb_status_t lan7800_read_eeprom(struct usb_device *dev, uint32_t offset,
		uint32_t length, uint8_t *data);
usb_status_t lan7800_set_rx_max_frame_length(struct usb_device *dev, int size);

#define NETIF_F_RXCSUM			4
#define NETIF_F_HW_VLAN_CTAG_RX		2
#define NETIF_F_HW_VLAN_CTAG_FILTER	1
usb_status_t lan7800_set_features(struct usb_device *dev, uint32_t features);
usb_status_t lan7800_init(struct usb_device *dev, uint8_t* macaddress);

usb_status_t lan7800_mdio_wait_for_bit(struct usb_device *udev,
					const uint32_t reg,
					const uint32_t mask,
					const bool set);

#define lan7800_wait_for_bit	lan7800_mdio_wait_for_bit

/***************************************************************************
 * The following regsiter set is primarily based on that of Linux's open-source 78xx driver:
 * https://github.com/torvalds/linux/blob/8fa3b6f9392bf6d90cb7b908e07bd90166639f0a/drivers/net/usb/lan78xx.h
 * and U-Boot:
 * https://github.com/trini/u-boot/blob/890e79f2b1c26c5ba1a86d179706348aec7feef7/drivers/usb/eth/lan7x.h
 ***************************************************************************/

/* TX command word A */
#define TX_CMD_A_IGE_			0x20000000
#define TX_CMD_A_ICE_			0x10000000

/* ??? LSO = Last segment... */
#define TX_CMD_A_LSO			0x08000000

#define TX_CMD_A_IPE_			0x04000000
#define TX_CMD_A_TPE_			0x02000000
#define TX_CMD_A_IVTG_			0x01000000
#define TX_CMD_A_RVTG_			0x00800000

/* TX word A buffer size. */
#define LAN7800_TX_CMD_A_BUF_SIZE	0x000FFFFF

/* TX command word B */
#define TX_CMD_B_MSS_SHIFT_		16
#define TX_CMD_B_MSS_MASK_		0x3FFF0000
#define TX_CMD_B_MSS_MIN_		(unsigned short)8
#define TX_CMD_B_VTAG_MASK_		0x0000FFFF
#define TX_CMD_B_VTAG_PRI_MASK_		0x0000E000
#define TX_CMD_B_VTAG_CFI_MASK_		0x00001000
#define TX_CMD_B_VTAG_VID_MASK_		0x00000FFF


/* TX/RX Overhead
 * Used in sum to allocate for Tx transfer buffer in etherOpen() */
#define LAN7800_TX_OVERHEAD			8
#define LAN7800_RX_OVERHEAD                     4

/* According to U-Boot's LAN78xx driver. */
#define LAN7800_HS_USB_PKT_SIZE			512

/* Read/write regsiters. */
#define LAN7800_VENDOR_REQUEST_WRITE		0xA0
#define LAN7800_VENDOR_REQUEST_READ		0xA1
#define LAN7800_VENDER_REQUEST_GET_STATS	0xA2

/* MAC TX/RX */
#define LAN7800_MAC_RX				0x104
#define LAN7800_MAC_TX				0x108

/* High and low RX register offsets */
#define LAN7800_ADDRH				0x118
#define LAN7800_ADDRL				0x11C

/* Offset of Hardware Configuration Register. */
#define LAN7800_HW_CFG			0x010

/* ??? */
#define LAN7800_HW_CFG_SRST		0x00000001
#define LAN7800_HW_CFG_LRST		0x00000002

#define LAN7800_BURST_CAP		0x090
#define LAN7800_BURST_CAP_SIZE		0x000000FF

/* Loopback mode. */
#define MAC_CR_LOOPBACK			0x00000400

/* Received Ethernet Frame Length */
#define LAN7800_RX_STS_FL		0x00003FFF

/* ??? Not entirely sure, appears to be Received Ethernet Error Summary. */
#define LAN7800_RX_CMD_A_RX_ERR		0xC03F0000

/* The following register definitions were pulled from U-Boot's implementation:
 * https://github.com/Screenly/u-boot/blob/master/drivers/usb/eth/lan78xx.c
 */

/* For initializing Tx. */
#define LAN7800_TX_FLOW			0x0D0

/* For initializing Rx. */
#define LAN7800_RFE_CTL			0x0B0

#define LAN7800_MAC_CR			0x100

/* Adaptive & Dynamic Polling at MAC layer ??? */
#define LAN7800_MAC_CR_ADP		(1 << 13)

#define LAN7800_FCT_TX_CTL		0x0C4

#define LAN7800_ETH_FRAME_LEN		1514
#define LAN7800_MAC_RX_MAX_SIZE(mtu) \
		((mtu) << 16)
#define LAN7800_MAC_RX_MAX_SIZE_DEFAULT LAN7800_MAC_RX_MAX_SIZE(LAN7800_ETH_FRAME_LEN + 4 /* VLAN */ + 4 /* CRC */)

#define LAN7800_MAC_RX_FCS_STRIP	(1 << 4)

#define LAN7800_FCT_RX_CTL              0x0C0

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
#define MII_ACC				0x120
#define MII_ACC_MII_READ		0x0
#define MII_ACC_MII_WRITE		0x2
#define MII_ACC_MII_BUSY		1 << 0

#define MII_DATA			0x124

#define TX_OVERHEAD			8
#define RX_OVERHEAD			10
#define RX_CMD_A_LEN_MASK		0x00003FFF
#define RX_CMD_A_RED			0x00400000
#define TX_CMD_A_LEN_MASK		0x000FFFFF
#define TX_CMD_A_FCS			0x00400000
#define EINVAL				22

/* OTP (One-Time Programmable) */
#define LAN7800_OTP_BASE		0x00001000
#define LAN7800_OTP_PWR_DN	 	LAN7800_OTP_BASE + 4 * 0x00
#define LAN7800_OTP_PWR_DN_PWRDN_N	0x01
#define LAN7800_OTP_ADDR1		LAN7800_OTP_BASE + 4 * 0x01
#define LAN7800_OTP_ADDR1_15_11		0x1F
#define LAN7800_OTP_ADDR2		LAN7800_OTP_BASE + 4 * 0x02
#define LAN7800_OTP_ADDR2_10_3		0xFF
#define LAN7800_OTP_FUNC_CMD		LAN7800_OTP_BASE + 4 * 0x08
#define LAN7800_OTP_FUNC_CMD_READ	0x01
#define LAN7800_OTP_CMD_GO		LAN7800_OTP_BASE + 4 * 0x0A
#define LAN7800_OTP_CMD_GO_GO		0x01
#define LAN7800_OTP_STATUS		LAN7800_OTP_BASE + 4 * 0x0C
#define LAN7800_OTP_STATUS_BUSY		0x01
#define LAN7800_OTP_RD_DATA		0x01
#define LAN7800_OTP_INDICATOR_1		0xF3
#define LAN7800_OTP_INDICATOR_2		0xF7

#define EIO				5

/* EEPROM */
#define LAN7800_EEPROM_INDICATOR	0xA5
#define LAN7800_E2P_CMD			0x040
#define LAN7800_E2P_CMD_EPC_BUSY	0x80000000
#define LAN7800_EEPROM_MAC_OFFSET	0x01
#define LAN7800_E2P_CMD_EPC_TIMEOUT	0x00000400
#define LAN7800_HW_CFG_LED3_EN		0x00800000
#define LAN7800_HW_CFG_LED2_EN		0x00400000
#define LAN7800_HW_CFG_LED1_EN		0x00200000
#define LAN7800_HW_CFG_LED0_EN		0x00100000
#define LAN7800_E2P_CMD_EPC_CMD_READ	0x00000000
#define LAN7800_E2P_CMD_EPC_ADDR_MASK	0x000001FF
#define LAN7800_E2P_DATA		0x44
#define LAN7800_USB_CFG1		0x084
#define LAN7800_USB_CFG1_LTM_ENABLE	0x00000100

#define LAN7800_LTM_BELT_IDLE0		0x0E0
#define LAN7800_LTM_BELT_IDLE1		0x0E4
#define LAN7800_LTM_BELT_ACT0		0x0E8
#define LAN7800_LTM_BELT_ACT1		0x0EC
#define LAN7800_LTM_INACTIVE0		0x0F0
#define LAN7800_LTM_INACTIVE1		0x0F4
#define LAN7800_MAC_RX_MAX_SIZE_MASK	0x3FFF0000
#define LAN7800_MAC_RX_MAX_SIZE_SHIFT	16	

/* RFE */
#define RFE_CTL_TCPUDP_COE		0x00001000
#define RFE_CTL_IP_COE			0x00000800
#define RFE_CTL_ICMP_COE		0x00002000
#define RFE_CTL_IGMP_COE		0x00004000
#define RFE_CTL_TCPUDP_COE		0x00001000
#define NETIF_F_HW_VLAN_CTAG_RX		2
#define RFE_CTL_VLAN_STRIP		0x00000080 
#define NETIF_F_HW_VLAN_CTAG_FILTER	1
#define RFE_CTL_VLAN_FILTER		0x00000020
#define NETIF_F_RXCSUM			4

/* RESET CONTINUE */
#define LAN7800_USB_CFG0		0x080
#define LAN7800_USB_CFG_BIR		0x00000040
#define LAN7800_MAX_TX_FIFO_SIZE      	12 * 1024
#define LAN7800_MAX_RX_FIFO_SIZE        12 * 1024
#define LAN7800_DEFAULT_BURST_CAP_SIZE  LAN7800_MAX_TX_FIFO_SIZE 
#define LAN7800_FS_USB_PKT_SIZE		64
#define LAN7800_HS_USB_PKT_SIZE		512
#define LAN7800_SS_USB_PKT_SIZE		1024

#define LAN7800_BULK_IN_DLY		0x094
#define LAN7800_DEFAULT_BULK_IN_DELAY   0x0800

#define LAN7800_HW_CFG_MEF		0x00000010
#define LAN7800_USB_CFG_BCE		0x00000020

/* FIFO */
#define LAN7800_INT_STS			0x0C
#define LAN7800_INT_STS_CLEAR_ALL	0xFFFFFFFF
#define LAN7800_FLOW			0x10C
#define LAN7800_FCT_FLOW		0x0D0
#define LAN7800_FCT_TX_FIFO_END		0x0CC
#define LAN7800_FCT_RX_FIFO_END		0x0C8

#define LAN7800_RFE_CTL_BCAST_EN	0x00000400
#define LAN7800_RFE_CTL_DA_PERFECT	0x00000002
#define LAN7800_RFE_CTL			0x0B0
#define LAN7800_RFE_CTL_UCAST_EN	0x00000100
#define LAN7800_RFE_CTL_MCAST_EN	0x00000200
#define LAN7800_RFE_CTL_MCAST_HASH	0x00000008

/* PMT */
#define LAN7800_PMT_CTL			0x014
#define LAN7800_PMT_CTL_PHY_RST		0x00000010

/* EEPROM, MAC */
#define LAN7800_EEPROM_INDICATOR	0xA5
#define LAN7800_MAC_CR_AUTO_DUPLEX	0x00001000
#define LAN7800_MAC_CR_AUTO_SPEED	0x00000800
#define LAN7800_FCT_TX_CTL		0x0C4

/* DMA buffer size */
#define LAN7800_ETH_MTU			1500

#define LAN7800_ETH_ALEN		6

#define LAN7800_ETH_VLAN_LEN		4	
#define LAN7800_MAC_RX_RXEN		0x00000001
#define LAN7800_MAC_TX_TXEN		0x00000001

/* FCT */
#define LAN7800_FCT_RX_CTL		0x0C0
#define LAN7800_FCT_RX_CTL_EN		0x80000000
#define LAN7800_FCT_TX_CTL		0x0C4
#define LAN7800_FCT_TX_CTL_EN		0x80000000

#endif	/* _LAN7800_H_ */
