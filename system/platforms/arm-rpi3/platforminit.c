/**
 * @file platforminit.c
 */
/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <platform.h>
#include <string.h>
#include <bcm2837.h>
#include <rpi_gpio.h>
#include "../../../device/uart-pl011/pl011.h"
#include <mmu.h>
#include <random.h>

#include <mutex.h>
#include <queue.h>

/* Definitions of usable ARM boot tags. ATAG list is a list of parameters passed from
 * the bootloader to the kernel. atags_ptr is passed inside start.S as a parameter. */

/*to_save_for_later0xB900001F*/
/* CHANGED SERIAL AND CMDLINE TAGS, SWITCHED ...0006 and ...0009 */


/* Definitions of usable ARM boot tags. ATAG list is a list of parameters passed from
 * the bootloader to the kernel. atags_ptr is passed inside start.S as a parameter. */

/*to_save_for_later0xB900001F*/
/* CHANGED SERIAL AND CMDLINE TAGS, SWITCHED ...0006 and ...0009 */

enum {
	ATAG_NONE       = 0x00000000, //Empty tag used to end list
	ATAG_CORE       = 0x54410001, //First tag used to start list
	ATAG_MEM        = 0x54410002, //Describes a physical area of memory
	ATAG_VIDEOTEXT  = 0x54410003, //Describes a VGA text display
	ATAG_RAMDISK    = 0x54410004, //Describes how the ramdisk will be used in kernel
	ATAG_INITRD2    = 0x54410005, //Describes where the compressed ramdisk image is placed in memory
	ATAG_SERIAL     = 0x54410006, //64 bit board serial number
	ATAG_REVISION   = 0x54410007, //32 bit board revision number
	ATAG_VIDEOLFB   = 0x54410008, //Initial values for vesafb-type framebuffers
	ATAG_CMDLINE    = 0x54410009, //Command line to pass to kernel
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
 *tagata This is set by the code in start.S.  Here, initialize it to a dummy value to
 * prevent it from being placed in .bss.  */
const struct atag *atags_ptr = (void*)-1;

/* End of kernel (used for sanity check)  */
extern void *_end;

/**
 * This code is irrelevant to Embedded Xinu and is only used as a means of testing on the RPI 3 boards
 */
/* Initialize GPIO pin 16 as an output */
void led_init(void)
{
	volatile struct rpi_gpio_regs *regptr;
	regptr = (struct rpi_gpio_regs *)(GPIO_REGS_BASE);
	regptr->gpfsel[1] &= ~(7 << 18);
	regptr->gpfsel[1] |=  (1 << 18);	
}
/* Set GPIO pin 16 to ON */
void led_on(void)
{	
	volatile struct rpi_gpio_regs *regptr = (struct rpi_gpio_regs *)(GPIO_REGS_BASE);
	regptr->gpset[0] = 1 << 16;
}
/* Set GPIO pin 16 to OFF */
void led_off(void)
{	
	volatile struct rpi_gpio_regs *regptr = (struct rpi_gpio_regs *)(GPIO_REGS_BASE);
	regptr->gpclr[0] = 1 << 16;
}

/**
 * Initializes platform specific information for the Raspberry Pi hardware.
 * @return OK
 */
int platforminit(void)
{
	strlcpy(platform.family, "BCM2837", PLT_STRMAX);
	strlcpy(platform.name, "Raspberry Pi 3", PLT_STRMAX);
	platform.maxaddr = (void *)0x3EFFFFFC; /* Used only if atags are bad */
//	platform.clkfreq = 1200000000;
	platform.clkfreq = 1000000;
	platform.serial_low = 0;   /* Used only if serial # not found in atags */
	platform.serial_high = 0;  /* Used only if serial # not found in atags */
	
	bcm2837_power_init(); 
	
	mmu_init();
	
	random_init();

	quetab_mutex = MUTEX_UNLOCKED;
	return OK;
}
