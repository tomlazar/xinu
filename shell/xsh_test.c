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
#include <random.h>

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
shellcmd xsh_test(int nargs, char *args[])
{
#if 0
	uint i;
	for (i = 0; i < 32; i++)
	{
		printf("%d inuse: %c; address: %d\n", i, usb_devices[i].inuse == 1 ? 'Y' : 'N', usb_devices[i].address);
	}
#endif
	/* Attempt to change value */
	printf("\n====\tSTART READ/WRITE TEST:\t====\n");
	usb_status_t status;
	uint32_t val = 0;

	status = lan7800_read_reg(&usb_devices[3], LAN7800_ADDRL, &val);
	printf("RX_ADDRL: 0x%08X\nstatus: %s\n", val, usb_status_string(status));

	status = lan7800_write_reg(&usb_devices[3], LAN7800_ADDRL, 0xDEADBEEF);
	printf("\nwrite status: %s\n", usb_status_string(status));

	status = lan7800_read_reg(&usb_devices[3], LAN7800_ADDRL, &val);
	printf("\nRX_ADDRL: 0x%08X\nstatus: %s\n", val, usb_status_string(status));

#if 0
	struct ether *ethptr;

	/* Bind the device */
        ethptr = &ethertab[0];

	/* Make sure it's supported */
	if(ethptr->csr != NULL){
		printf("\nDEVICE NOT SUPPORTED.\n\n");
		return USB_STATUS_DEVICE_UNSUPPORTED;
	}

	/* Attempt to open the device */
	etherOpen(&usb_devices[3]);
#endif

	uint8_t r[6];

	int j;
	for(j = 0; j < 6; j++){
		r[j] = random() % (255 - 0) + 0;
	}

	uint8_t macaddr[] = {r[0], r[1], r[2], r[3], r[4], r[5]};

	printf("\n\n====\tSTART MAC TEST:\t====\n");

	printf("\nMAC TO SET:\t");
	int i;
	for(i = 0; i < 6; i ++){
	         printf("%02X", macaddr[i]);
	         if(i != 5)
                 	printf(":");
  	}  
	

	/* Attempt to set the MAC address */
	status = lan7800_set_mac_address(&usb_devices[3], macaddr);
	printf("\nMAC SET STATUS: %s\n", usb_status_string(status));	

	/* Read the MAC address */
	status = lan7800_get_mac_address(&usb_devices[3], macaddr);
	printf("\nMAC GET STATUS: %s\n", usb_status_string(status));
	printf("\nGET MAC:\t");

	/* [for debugging] Print MAC bits 0-5 which are set in lan7800_set_mac_address() */
	for(i = 0; i < 6; i ++){
		printf("%02X", macaddr[i]);
		if(i != 5)
			printf(":");
	}

	printf("\n\n");

	status = lan7800_read_reg(&usb_devices[3], LAN7800_ADDRL, &val);
	printf("LAN7800_ADDRL: 0x%08X\nstatus: %s\n", val, usb_status_string(status));
	status = lan7800_read_reg(&usb_devices[3], LAN7800_ADDRH, &val);
	printf("\nLAN7800 ADDRH: 0x%08X\nstatus: %s\n", val, usb_status_string(status));

	
	printf("\n\n====\tSTART ETHERTAB DUMP TEST:\t====\n");
	
	struct ether *ethptr;
	ethptr = &ethertab[0];

	printf("!ethptr->dev:			%c\n", (!ethptr->dev) ? 'Y' : 'N');
	printf("ethptr->state:			0x%08X\n", ethptr->state);
	printf("ethptr->mtu:			0x%08X\n", ethptr->mtu);
	printf("ethptr->addressLength:	        0x%08X\n", ethptr->addressLength);

	return 0;
}
