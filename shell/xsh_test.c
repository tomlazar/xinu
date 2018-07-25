/**
 * @file     xsh_test.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <shell.h>
#include <string.h>
#include <ctype.h>
#include <clock.h>
#include <usb_core_driver.h>
#include <usb_util.h>
#include "../device/lan7800/lan7800.h"
#include <ether.h>
#include <testsuite.h>

#define MAX_PAYLOAD  1516
#define ETH_TYPE_ARP 0x0806
#define hs2net(x) (unsigned) ((((x)>>8) &0xff) | (((x) & 0xff)<<8))
#define failif(cond, failmsg) \
	if ( cond ) { testFail(verbose, failmsg); printf("\t%s:%d\r\n", __FILE__, __LINE__); passed = FALSE; } \
	else { testPass(verbose, ""); }

extern struct usb_device usb_devices[];

/**
 * @ingroup shell
 *
 * Shell command (test) provides a mechanism for testing Xinu features.  The
 * action and output varies depending on the feature currently being tested.
 * This is not meant to serve as a permanent shell command for a particular
 * action.
 * @param nargs number of arguments
 * @param args  array of arguments
 * @return non-zero value on error
 */

/* Simple ethernet packet structure */
struct etherGram
{
	char dst[ETH_ADDR_LEN];     /* Destination MAC */
	char src[ETH_ADDR_LEN];     /* Source MAC */
	ushort type_len;            /* EthernetII type/Ethernet length */
	char payload[1];            /* Payload data */
};   

static int ethtest(void);
shellcmd xsh_test(int nargs, char *args[])
{
	bool passed = TRUE;

	passed &= ethtest();

	if(passed)
		testPass(TRUE, "");

	else
		testFail(TRUE, "");

	return 0;
}

static int ethtest()
{
	bool verbose = TRUE;
	bool passed = TRUE;
	bool subpass;
	uint memsize;
	int i, value, len;
	struct etherGram *inpkt;
	struct etherGram *outpkt;
	char *payload;
	char mymac[ETH_ADDR_LEN];
	char str[80];
	struct ether *peth = &ethertab[0];
	int dev = peth->dev->num;

	/* memget */
	memsize = sizeof(struct etherGram) + MAX_PAYLOAD - 1;
	inpkt = memget(memsize);
	outpkt = memget(memsize);
	payload = &(outpkt->payload[0]);

	control(dev, ETH_CTRL_GET_MAC, (long)mymac, 0);
	memcpy(outpkt->dst, mymac, ETH_ADDR_LEN);
	memcpy(outpkt->src, mymac, ETH_ADDR_LEN);
	outpkt->type_len = hs2net(ETH_TYPE_ARP);

	/* generate payload content */
	for (i = 0; i < MAX_PAYLOAD; i++)
	{
		/* Cycle through 0x20 to 0x7d (range of 0x5e) */
		value = (i % 0x5e) + 0x20;
		payload[i] = value;
	}

	/* place ether in loopback mode */
	control(dev, ETH_CTRL_SET_LOOPBK, TRUE, 0);

	/* flush any packets already received */
	while (peth->icount > 0)
	{
		read(dev, inpkt, memsize);
	}

	/* oversized packet (paylod 1502 bytes + 14 byte header)
	 * should result in s SYSERR */
	sprintf(str, "%s 1516 OVERSIZED byte packet (write)", peth->dev->name);
	testPrint(verbose, str);
	len = write(dev, outpkt, 1516);
	failif((SYSERR != len), "");

	sprintf(str, "%s 1514 MAX byte packet (write)", peth->dev->name);
	testPrint(verbose, str);
	len = write(dev, outpkt, 1514);
	failif((len < 1514), "");

	sprintf(str, "%s 700 NORMAL byte packet (write)", peth->dev->name);
	testPrint(verbose, str);
	len = write(dev, outpkt, 700);
	failif((len < 700), "");

	/* Too small of a packet, should end in SYSERR. */
	sprintf(str, "%s 12 MICRO byte packet (write)", peth->dev->name);
	testPrint(verbose, str);
	len = write(dev, outpkt, 12);
	failif((SYSERR != len), "");

	sprintf(str, "%s 512 random-sized packets (write)", peth->dev->name);
	testPrint(verbose, str);
	subpass = TRUE;

	for(i = 0; i < 512; i++)
	{
		len = 32 + (rand() % 1200);
		value = write(dev, outpkt, len);

		if(value < len){
			kprintf("\r\nsubpass = false, value < len... \r\n");
			subpass = FALSE;
		}

		bzero(inpkt, memsize);
		value = read(dev, inpkt, len);
		if (0 != memcmp(outpkt, inpkt, 30))
			subpass = FALSE;
	}
	failif((TRUE != subpass), "");

	/*
	   sprintf(str, "%s  700 byte packet (read)", peth->dev->name);
	   testPrint(verbose, str);
	   bzero(inpkt, memsize);
	   printf("\r\nbefore attempt to read...\r\n");
	   len = read(dev, inpkt, 700);
	   printf("Before fail check...\r\n");
	   failif((0 != memcmp(outpkt, inpkt, 700)), "");
	   printf("After fail statement...\r\n");
	   return 0;
	   */
	/* ether out of loopback mode */
	control(dev, ETH_CTRL_SET_LOOPBK, FALSE, 0);

	memfree(outpkt, memsize);
	memfree(inpkt, memsize);

	return passed;
}

void testPass(bool verbose, const char *msg)
{
	if (TRUE == verbose)
	{
		kprintf("\033[60G[\033[1;32mPASS\033[0;39m] %s\n", msg);
	}
}

void testFail(bool verbose, const char *msg)
{
	if (TRUE == verbose)
	{
		kprintf("\033[60G[\033[1;31mFAIL\033[0;39m] %s\n", msg);
	}
}

void testSkip(bool verbose, const char *msg)
{
	if (TRUE == verbose)
	{
		kprintf("\033[60G[\033[1;33mSKIP\033[0;39m] %s\n", msg);
	}
}

void testPrint(bool verbose, const char *msg)
{
	if (TRUE == verbose)
	{
		kprintf("  %s", msg);
	}
}
