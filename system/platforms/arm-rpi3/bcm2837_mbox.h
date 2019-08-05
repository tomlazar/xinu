/* Mailbox driver for the BCM2837B0 (Pi 3 B+).
 *
 * Embedded Xinu Copyright (C) 2009, 2013, 2018. All rights reserved.
 *
 * Authors:	Patrick J. McGee
 * 		Rade Latinovich
 */

#include "bcm2837.h"
#include <stdint.h>

/* Mailbox functions */
void bcm2837_mailbox_write(uint, uint);
uint bcm2837_mailbox_read(uint);
void add_mailbox_tag(volatile uint32_t*, uint32_t, uint32_t, uint32_t, uint32_t*);
void build_mailbox_request(volatile uint32_t*);
void dump_response(volatile uint32_t*, const char*, int);
void print_parameter(volatile uint32_t*, const char*, uint32_t, int);
void get_mac_mailbox(volatile uint32_t*);

/* Mailbox base offset */
#define MAILBOX_REGS_BASE      (PERIPHERALS_BASE + 0xB880)

static volatile uint *const mailbox_regs = (volatile uint*)MAILBOX_REGS_BASE;

/* BCM2837 mailbox register indices  */
#define MAILBOX_READ               0
#define MAILBOX_STATUS             6
#define MAILBOX_WRITE              8

/* BCM2837 mailbox status flags  */
#define MAILBOX_FULL               0x80000000
#define MAILBOX_EMPTY              0x40000000

/* BCM2837 mailbox channels  */
#define MAILBOX_CHANNEL_POWER_MGMT 0
#define MAILBOX_CHANNEL_1	   1

/* The BCM2837 mailboxes are used for passing 28-bit messages.  The low 4 bits
 *  * of the 32-bit value are used to specify the channel over which the message is
 *   * being transferred  */
#define MAILBOX_CHANNEL_MASK       0xf

/* Length of mailbox buffer. */
#define MBOX_BUFLEN 1024

/* States for the mailbox. */
#define RR_REQUEST 0x00000000
#define RR_RESPONSE_OK 0x80000000
#define RR_RESPONSE_ERROR 0x80000001

#define SLOT_OVERALL_LENGTH 0
#define SLOT_RR 1
#define SLOT_TAGSTART 2

#define SLOT_TAG_ID 0
#define SLOT_TAG_BUFLEN 1
#define SLOT_TAG_DATALEN 2
#define SLOT_TAG_DATA 3

#define MBOX_HEADER_LENGTH 2
#define TAG_HEADER_LENGTH 3

#define MBX_DEVICE_SDCARD 0x00000000
#define MBX_DEVICE_UART0 0x00000001
#define MBX_DEVICE_UART1 0x00000002
#define MBX_DEVICE_USBHCD 0x00000003
#define MBX_DEVICE_I2C0 0x00000004
#define MBX_DEVICE_I2C1 0x00000005
#define MBX_DEVICE_I2C2 0x00000006
#define MBX_DEVICE_SPI 0x00000007
#define MBX_DEVICE_CCP2TX 0x00000008

/* Mailbox tags for receiving board information. */
#define MBX_TAG_GET_FIRMWARE 0x00000001 /* in 0, out 4 */
#define MBX_TAG_GET_BOARD_MODEL 0x00010001 /* in 0, out 4 */
#define MBX_TAG_GET_BOARD_REVISION 0x00010002 /* in 0, out 4 */
#define MBX_TAG_GET_MAC_ADDRESS 0x00010003 /* in 0, out 6 */
#define MBX_TAG_GET_BOARD_SERIAL 0x00010004 /* in 0, out 8 */
#define MBX_TAG_GET_ARM_MEMORY 0x00010005 /* in 0, out 8 (4 -> base addr, 4 -> len in bytes) */
#define MBX_TAG_GET_VC_MEMORY 0x00010006 /* in 0, out 8 (4 -> base addr, 4 -> len in bytes) */
#define MBX_TAG_GET_COMMANDLINE 0x00050001 /* in 0, out variable */
#define MBX_TAG_GET_DMA_CHANNELS 0x00060001 /* in 0, out 4 */
#define MBX_TAG_GET_POWER_STATE 0x00020001 /* in 4 -> dev id, out 8 (4 -> device, 4 -> status) */
#define MBX_TAG_GET_TIMING 0x00020002 /* in 0, out 4 */
#define MBX_TAG_GET_FIRMWARE 0x00000001 /* in 0, out 4 */
