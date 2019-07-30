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
#include <platform.h>
#include <stdint.h>

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

/* Make a mailbox call. Returns 0 on failure, non-zero on success */
int mbox_call(unsigned char ch)
{
	unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
	/* wait until we can write to the mailbox */
	do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
	/* write the address of our message to the mailbox with channel identifier */
	*MBOX_WRITE = r;
	/* now wait for the response */
	while(1) {
		// LED on here... there is a response...
		/* Is there a response? */
		do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
		/* Is it a response to our message? */
		if(r == *MBOX_READ){
			/* Is it a valid successful response? */
			// LED on... apparently the response is successful
			return mbox[1]==MBOX_RESPONSE;
		}
	}
	return 0;
}

/* screenInit(): Calls framebufferInit() several times to ensure we successfully initialize, just in case. */
void screenInit() {
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

/* Initializes the framebuffer used by the GPU. Returns OK on success; SYSERR on failure. */
int framebufferInit() {

	// LED turns on here...

	//XXXbcm2837_mailbox_write(MAILBOX_CH_PROPERTY, (uint)&frame);

	//XXXulong result = bcm2837_mailbox_read(MAILBOX_CH_PROPERTY);
	
	mbox[0] = 35*4;
	mbox[1] = MBOX_REQUEST;

	mbox[2] = 0x48003;  //set phy wh
	mbox[3] = 8;
	mbox[4] = 8;
	mbox[5] = 1024;         //FrameBufferInfo.width
	mbox[6] = 768;          //FrameBufferInfo.height

	mbox[7] = 0x48004;  //set virt wh
	mbox[8] = 8;
	mbox[9] = 8;
	mbox[10] = 1024;        //FrameBufferInfo.virtual_width
	mbox[11] = 768;         //FrameBufferInfo.virtual_height

	mbox[12] = 0x48009; //set virt offset
	mbox[13] = 8;
	mbox[14] = 8;
	mbox[15] = 0;           //FrameBufferInfo.x_offset
	mbox[16] = 0;           //FrameBufferInfo.y.offset

	mbox[17] = 0x48005; //set depth
	mbox[18] = 4;
	mbox[19] = 4;
	mbox[20] = 32;          //FrameBufferInfo.depth

	mbox[21] = 0x48006; //set pixel order
	mbox[22] = 4;
	mbox[23] = 4;
	mbox[24] = 1;           //RGB, not BGR preferably

	mbox[25] = 0x40001; //get framebuffer, gets alignment on request
	mbox[26] = 8;
	mbox[27] = 8;
	mbox[28] = 4096;        //FrameBufferInfo.pointer
	mbox[29] = 0;           //FrameBufferInfo.size

	mbox[30] = 0x40008; //get pitch
	mbox[31] = 4;
	mbox[34] = MBOX_TAG_LAST;

	// LED turns on here...

	if(mbox_call(MAILBOX_CH_PROPERTY) && mbox[20]==32 && mbox[28]!=0) {
		
		led_init();
		led_on();

		mbox[28]&=0x3FFFFFFF;
		cols=mbox[5];
		rows=mbox[6];
		pitch=mbox[33];
		framebufferAddress=(ulong)((unsigned long)mbox[28]);
	} else {	// If mailbox call ends in error, return
		return SYSERR;
	}
	
	/* Error checking */
	/*if (result) { //if anything but zero
	  return SYSERR;
	  }
	  if (!frame.address) { //if address remains zero
	  return SYSERR;
	  }*/

	/* Initialize global variables */
	//framebufferAddress = frame.address;
	//rows = frame.height_p / CHAR_HEIGHT;
	//cols = frame.width_p / CHAR_WIDTH;
	//pitch = frame.pitch;
	cursor_row = 0;
	cursor_col = 0;
	background = RED;
	foreground = CYAN;
	minishell = FALSE;
	return OK;
}

/* Very heavy handed clearing of the screen to a single color. */
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

/* Clear the minishell window */
void minishellClear(ulong color) {
	ulong *address = (ulong *)(framebufferAddress + (pitch * (DEFAULT_HEIGHT - (MINISHELLMINROW * CHAR_HEIGHT))) +  (DEFAULT_WIDTH * (BIT_DEPTH / 8)));
	ulong *maxaddress = (ulong *)(framebufferAddress + (DEFAULT_HEIGHT * pitch) + (DEFAULT_WIDTH * (BIT_DEPTH / 8)));
	while (address != maxaddress) {
		*address = color;
		address++;
	}
	_inval_area((void *)framebufferAddress);
}

/* Clear the "linemapping" array used to keep track of pixels we need to remember */
void initlinemap() {
	int i = MAPSIZE;
	while (i != 0) {
		i--;
		linemap[i] = background;
	}
}
