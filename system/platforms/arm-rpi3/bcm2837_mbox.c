/* Definitions for Mailbox functions of the BCM2837B0 (Pi 3 B+).
 *
 * Embedded Xinu Copyright (C) 2009, 2013, 2018. All rights reserved.
 *
 * Authors:     Patrick J. McGee
 *              Rade Latinovich
 */

#include <kernel.h>
#include <stdint.h>
#include <string.h>
#include "bcm2837_mbox.h"

/* Function prototypes */
extern void bcm2837_mailbox_write(uint, uint);
extern void bcm2837_mailbox_read(uint);
void bzero(volatile uint32_t* s, size_t n);

volatile uint32_t mailbuffer[1024]; /* Buffer array for mailbox  */

/* Clear the mailbox buffer entirely. */
void mbox_clear(void){
	bzero(mailbuffer, 1024);
	kprintf("\r\nMailbox cleared.\r\n");
}

/* Add a tag to the mailbox buffer. */
void add_mailbox_tag(volatile uint32_t* buffer, uint32_t tag, uint32_t buflen, uint32_t len, uint32_t* data) {
	volatile uint32_t* start = buffer + SLOT_TAGSTART;
        start[SLOT_TAG_ID] = tag;
	start[SLOT_TAG_BUFLEN] = buflen;
	start[SLOT_TAG_DATALEN] = len & 0x7FFFFFFF;
	uint32_t bufwords = buflen >> 2;

	if (0 == data) {
		for (int i = 0; i < bufwords; ++i) {
			start[SLOT_TAG_DATA + i] = 0;
		}
	} else {
		for (int i = 0; i < bufwords; ++i) {
			start[SLOT_TAG_DATA + i] = data[i];
		}
	}
	start[SLOT_TAG_DATA+bufwords] = 0; // end of tags, unless overwritten later
}

/* Ready the buffer (done after the tag is added) */
void build_mailbox_request(volatile uint32_t* buffer) {
	uint32_t tag_length = buffer[MBOX_HEADER_LENGTH + SLOT_TAG_BUFLEN];
	uint32_t end = (MBOX_HEADER_LENGTH*4) + (TAG_HEADER_LENGTH*4) + tag_length;
	uint32_t overall_length = end + 4;
	buffer[SLOT_OVERALL_LENGTH] = overall_length;
	buffer[SLOT_RR] = RR_REQUEST;
}

/* Print the mailbox buffer. */
void dump_response(const char* name, int nwords) {
	kprintf("%s: ", name);
	for (int i = 0; i < nwords; ++i) {
		uint32_t value = mailbuffer[MBOX_HEADER_LENGTH + TAG_HEADER_LENGTH + i];
		kprintf("0x%08X ", value);
	}
	kprintf("\r\n");
}

/* Print a parameter of the mailbox buffer. */
void print_parameter(const char* name, uint32_t tag, int nwords) {
	add_mailbox_tag(mailbuffer, tag, nwords * 4, 0, 0);
	build_mailbox_request(mailbuffer);
			
	bcm2837_mailbox_write(8, (uint32_t)mailbuffer);		
	bcm2837_mailbox_read(8);

	kprintf("\r\nMB[1]: 0x%08X\r\n\n", mailbuffer[1]);

	/* Valid response in data structure */
	if(mailbuffer[1] != RR_RESPONSE_OK) {
		kprintf("MAILBOX ERROR\r\n");
	} else {
		dump_response(name, nwords);
	}
}

