/**
 * @file bcm2837_mbox.c
 *
 * Definitions for Mailbox functions of the BCM2837B0 (Pi 3 B+).
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

/**
 * @ingroup bcm2837
 *
 * Add a tag to the mailbox buffer. This step must be done before a mailbox request is built. 
 * @param buffer	Mailbox buffer
 * @param tag		Tag to pass
 * @param buflen	Buffer length
 * @param len		Length of data to be sent
 * @param data		Data to be sent
 */
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
	/* End of tags, unless overwritten later */
	start[SLOT_TAG_DATA+bufwords] = 0;
}

/**
 * @ingroup bcm2837
 *
 * Ready the buffer by initializing proper lengths, slots. 
 * @param mailbuffer	Mailbox buffer
 */
void build_mailbox_request(volatile uint32_t* mailbuffer) {
	uint32_t tag_length = mailbuffer[MBOX_HEADER_LENGTH + SLOT_TAG_BUFLEN];
	uint32_t end = (MBOX_HEADER_LENGTH*4) + (TAG_HEADER_LENGTH*4) + tag_length;
	uint32_t overall_length = end + 4;
	mailbuffer[SLOT_OVERALL_LENGTH] = overall_length;
	mailbuffer[SLOT_RR] = RR_REQUEST;
}

/**
 * @ingroup bcm2837
 *
 * Function for getting the MAC address using the corresponding MAC mailbox tag. 
 * @param mailbuffer	Mailbox buffer written with MAC address upon conclusion. 
 */
void get_mac_mailbox(volatile uint32_t* mailbuffer){

	/* Load the tag, build the buffer */
	add_mailbox_tag(mailbuffer, MBX_TAG_GET_MAC_ADDRESS, 8, 0, 0);
	build_mailbox_request(mailbuffer);

	/* Write the mailbox register */
	bcm2837_mailbox_write(8, (uint32_t)mailbuffer);
	bcm2837_mailbox_read(8);
}
