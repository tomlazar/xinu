/**
 * @file screenInit.c
 *
 * Initializes communication channels between VC and ARM.
 */
/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */

#include <stddef.h>
#include <framebuffer.h>
#include <stdlib.h>
#include <shell.h> /* for banner */
#include <kernel.h>
#include <dma_buf.h>
#include "../../system/platforms/arm-rpi3/bcm2837_mbox.h"
#include "../../system/platforms/arm-rpi3/bcm2837.h"
#include <platform.h>
#include <stdint.h>
#include <uart.h>

extern void _inval_area(void *);

int rows;
int cols;
int cursor_row;
int cursor_col;
ulong background;
ulong foreground;
ulong linemap[MAPSIZE];
bool minishell;
ulong framebufferAddress;
int pitch;
bool screen_initialized;
volatile unsigned int  __attribute__((aligned(16))) mbox[36];

/* screenInit(): Calls framebufferInit() several times to ensure we successfully initialize, just in case. */
void screenInit() {
	
	// Note: boolean screen_initialized is true if there is no hardware error (e.g. mailbox failure)
	// The boolean does not determine whether the screen is being used, as it it always initialized in start.S
	int i = 0;
	while (framebufferInit() == SYSERR) {
		if ( (i++) == MAXRETRIES) {
			screen_initialized = FALSE;
			return;
		}
	}
	// clear the screen to the background color.
	screenClear(background);
	initlinemap();
	screen_initialized = TRUE;
}

/**
 * @ingroup framebuffer
 *
 * Initializes the framebuffer used by the GPU via the framebuffer mailbox.
 * @return ::OK on success; ::SYSERR on failure. 
 */
int framebufferInit() {

	/* Build the mailbox buffer for the frame buffer
	 * Mailbox 8 (ARM->VC PROPERTY mailbox) is used on the Pi 3 B+ 
	 * because the frame buffer mailbox (1) does not seem to work. 
	 * BCM2837 documentation is not available as of this writing 
	 * This working implementation was obtained from an open source example by GitHub user
	 * bztsrc: https://github.com/bztsrc/raspi3-tutorial/blob/master/09_framebuffer/lfb.c
	 * */
	mbox[0] = 35*4;
	mbox[1] = MBOX_REQUEST;

	mbox[2] = 0x48003;  // set phy wh
	mbox[3] = 8;
	mbox[4] = 8;
	mbox[5] = 1024;     // width
	mbox[6] = 768;      // height

	mbox[7] = 0x48004;  // Set virt wh
	mbox[8] = 8;
	mbox[9] = 8;
	mbox[10] = 1024;    // Virtual width
	mbox[11] = 768;     // Virtual height

	mbox[12] = 0x48009; // Set virt offset
	mbox[13] = 8;
	mbox[14] = 8;
	mbox[15] = 0;       // x offset
	mbox[16] = 0;       // y offset

	mbox[17] = 0x48005; // Set depth
	mbox[18] = 4;
	mbox[19] = 4;
	mbox[20] = 32;      // Depth

	mbox[21] = 0x48006; // Set pixel order
	mbox[22] = 4;
	mbox[23] = 4;
	mbox[24] = 1;       // RGB, not BGR preferably

	mbox[25] = 0x40001; // Get framebuffer, gets alignment on request
	mbox[26] = 8;
	mbox[27] = 8;
	mbox[28] = 0;       // Pointer
	mbox[29] = 0;       // Size

	mbox[30] = 0x40008; // Get pitch
	mbox[31] = 4;
	mbox[34] = MBOX_TAG_LAST;

	bcm2837_mailbox_write(8,((unsigned int)&mbox));
	
	/* Wait for a response to our mailbox message... */
	while(1){
		if(bcm2837_mailbox_read(8) == ((unsigned int)&mbox))
		{
			if(mbox[28] != 0) {
				mbox[28]&=0x3FFFFFFF;
				cols=mbox[5] / CHAR_WIDTH;
				rows=mbox[6] / CHAR_HEIGHT;
				pitch=mbox[33];
				framebufferAddress=mbox[28];
			} else {
				return SYSERR;
			}
		break;
		}
	}

	/* Initialize global variables */
	cursor_row = 0;
	cursor_col = 0;
	background = BLACK;
	foreground = WHITE;
	minishell = FALSE;
	return OK;
}

/** 
 * @ingroup framebuffer
 *
 * Very heavy handed clearing of the screen to a single color. 
 * @param color	
 */
void screenClear(ulong color) {
	ulong *address = (ulong *)(framebufferAddress);
	ulong *maxaddress = (ulong *)(framebufferAddress + (DEFAULT_HEIGHT * pitch) + (DEFAULT_WIDTH * (BIT_DEPTH / 8)));
	while (address != maxaddress) {
		*address = color;
		_inval_area(address);
		address++;
	}
	_inval_area((void *)framebufferAddress);
}

/** 
 * @ingroup framebuffer
 *
 * Clear the minishell window
 * @param color	Color to paint the window upon clear
 */
void minishellClear(ulong color) {
	ulong *address = (ulong *)(framebufferAddress + (pitch * (DEFAULT_HEIGHT - (MINISHELLMINROW * CHAR_HEIGHT))) +  (DEFAULT_WIDTH * (BIT_DEPTH / 8)));
	ulong *maxaddress = (ulong *)(framebufferAddress + (DEFAULT_HEIGHT * pitch) + (DEFAULT_WIDTH * (BIT_DEPTH / 8)));
	while (address != maxaddress) {
		*address = color;
		address++;
	}
	_inval_area((void *)framebufferAddress);
}

/** 
 * @ingroup framebuffer
 *
 * Clear the "linemapping" array used to keep track of pixels we need to remember 
 */
void initlinemap() {
	int i = MAPSIZE;
	while (i != 0) {
		i--;
		linemap[i] = background;
	}
}
