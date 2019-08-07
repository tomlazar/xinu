/**
 * @file mailbox.c
 *
 * Provides communication channels between VC and ARM.
 */
/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */

#include <stddef.h>
#include <framebuffer.h>
#include "../../system/platforms/arm-rpi3/bcm2837_mbox.h"

/**
 * @ingroup framebuffer 
 * 
 * Read from mailbox one on channel one (GPU mailbox) 
 * Note: Data: first 28 bits. Channel: last 4 bits.   
 */
ulong mailboxRead() {
	ulong result = 0;
	while ((result & 0xF) != MAILBOX_CHANNEL) {
		while (readMMIO(MAILBOX_BASE, MAILBOX_STATUS) & MAILBOX_EMPTY)
			; //wait for data to arrive
		result = readMMIO(MAILBOX_BASE, MAILBOX_READ);
	}
	return result & 0xFFFFFFF0; //account for offset
}

/**
 * @ingroup framebuffer
 *
 * Write to GPU mailbox. 
 * @param data Data to write to the mailbox
 */
void mailboxWrite(ulong data) {
	ulong toWrite = MAILBOX_CHANNEL | (data & 0xFFFFFFF0);
	while (readMMIO(MAILBOX_BASE, MAILBOX_STATUS) & MAILBOX_FULL)
		; //wait for space
	writeMMIO(MAILBOX_BASE, MAILBOX_WRITE, toWrite);
}

/**
 * @ingroup framebuffer
 * 
 * To omit illegible lines of code, a helper function that reads from
 * memory mapped IO registers. 
 * @param base	Register base
 * @param reg	MMIO register to read
 */
ulong readMMIO(ulong base, ulong reg)
{
	ulong n;
	pre_peripheral_read_mb();
	n = *(volatile ulong *)(MMIO_BASE + base + reg);
	post_peripheral_read_mb();
	return n;
}

/**
 * @ingroup framebuffer
 * 
 * The opposite of above. Write to MMIO. 
 * @param base	Register base
 * @param reg	MMIO register to write to
 * @param val	Value to be written
 */
void writeMMIO(ulong base, ulong reg, ulong val)
{
	pre_peripheral_write_mb();
	*(volatile ulong *)(MMIO_BASE + base + reg) = val;
	//post_peripheral_write_mb();
}

/**
 * @ingroup framebuffer
 *
 * Converts ARM physical addresses to ARM bus addresses.
 * Separate function for legibility's sake.
 * Rev 1 board GPU required bus addresses. Now deprecated. 
 * @param address	Physical address to convert
 */
ulong physToBus(void *address) {
	return ((ulong)address) + 0xC0000000;
}
