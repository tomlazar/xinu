/**
 * @file platforminit.c
 */
/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <platform.h>
#include <string.h>
#include "bcm2837.h"
#include "../../../device/uart-pl011/pl011.h"




/* Definitions of ARM boot tags.  */
enum {
	ATAG_NONE       = 0x00000000,
	ATAG_CORE       = 0x54410001,
	ATAG_MEM        = 0x54410002,
	ATAG_VIDEOTEXT  = 0x54410003,
	ATAG_RAMDISK    = 0x54410004,
	ATAG_INITRD2    = 0x54410005,
	ATAG_SERIAL     = 0x54410006,
	ATAG_REVISION   = 0x54410007,
	ATAG_VIDEOLFB   = 0x54410008,
	ATAG_CMDLINE    = 0x54410009,
};

/* Below we only define structures for tags we actually use.  */

/* ARM boot tag header.  */
struct atag_header {
	uint size;  /* Size of tag, in words, including the header.  */
	uint tag;   /* One of the ATAG_* values from above.          */
};

/* Description of memory region (ATAG_MEM)  */
struct atag_mem {
	uint size;
	uint start;
};

/* Board serial number (ATAG_SERIAL)  */
struct atag_serialnr {
	uint low;
	uint high;
};

/* Format of ARM boot tag  */
struct atag {
	struct atag_header hdr;
	union {
		struct atag_mem mem;
		struct atag_serialnr serialnr;
	};
};

/** Physical memory address at which the bootloader placed the ARM boot tags.
 * This is set by the code in start.S.  Here, initialize it to a dummy value to
 * prevent it from being placed in .bss.  */
const struct atag *atags_ptr = (void*)-1;

/* End of kernel (used for sanity check)  */
extern void *_end;

/* Extract some information from the ARM boot tag list and place it in the
 * 'platform' structure.  */
	static void
parse_atag_list(void)
{
	const struct atag *atag;
	bool parse_again;
	ulong maxaddr = 0;

	/* We may parse the atags more than once to try to coalesce memory specified
	 * as multiple contiguous chunks.  */
	do
	{
		parse_again = FALSE;
		for (atag = atags_ptr;
				atag->hdr.size > 2 && atag->hdr.tag != ATAG_NONE;
				atag = (const struct atag*)((const uint*)atag + atag->hdr.size))
		{
			switch (atag->hdr.tag)
			{
				case ATAG_MEM:
					if (maxaddr == atag->mem.start && atag->mem.size != 0)
					{
						maxaddr += atag->mem.size;
						parse_again = TRUE;
					}
					break;
				case ATAG_SERIAL:
					platform.serial_low = atag->serialnr.low;
					platform.serial_high = atag->serialnr.high;
					break;
				default:
					break;
			}
		}
	} while (parse_again);

	/* Set platform maximum address if calculated value is not insane.  */
	if (maxaddr >= (ulong)&_end)
	{
		platform.maxaddr = (void*)maxaddr;
	}
}

#define GPFSEL1     (*(volatile unsigned *)(GPIO_REGS_BASE + 0x04))
void pl011init(void)
{
	int i;

	volatile struct pl011_uart_csreg *regptr =
		(struct pl011_uart_csreg *)(PL011_REGS_BASE);

	udelay(1500);

	regptr->cr = 0;

	GPFSEL1 &= ~((7 << 12) | (7 << 15));  /* GPIO14 & 15: alt0  */
	GPFSEL1 |= (4 << 12) | (4 << 15);

	udelay(150);

	while(regptr->fr & PL011_FR_BUSY)
		;
	regptr->lcrh &= ~PL011_LCRH_FEN;

	regptr->ibrd = PL011_BAUD_INT(115200);
	regptr->fbrd = PL011_BAUD_FRAC(115200);

	regptr->lcrh = PL011_LCRH_WLEN_8BIT;

	regptr->cr = PL011_CR_RXE | PL011_CR_TXE | PL011_CR_UARTEN;

	regptr->lcrh |= PL011_LCRH_FEN;
}

/**
 * Initializes platform specific information for the Raspberry Pi hardware.
 * @return OK
 */
int platforminit(void)
{
	strlcpy(platform.family, "BCM2837", PLT_STRMAX);
	strlcpy(platform.name, "Raspberry Pi 3", PLT_STRMAX);
	platform.maxaddr = (void *)0xBFFFFFF; /* Used only if atags are bad */
	platform.clkfreq = 1000000;
	platform.serial_low = 0;   /* Used only if serial # not found in atags */
	platform.serial_high = 0;  /* Used only if serial # not found in atags */
	parse_atag_list();
	bcm2835_power_init(); 
//	pl011init();
//	kprintf("Hello World!\r\n");  
//	led_on();	
	return OK;
}
