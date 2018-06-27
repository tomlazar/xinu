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

extern struct usb_device usb_devices[];
struct ether ethertab[1];

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
	printf("\n====START READ/WRITE TEST:====\n");
	usb_status_t status;
	uint32_t val = 0;

	status = lan7800_read_reg(&usb_devices[3], LAN7800_ADDRL, &val);
	printf("RX_ADDRL: 0x%08X\nstatus: %s\n", val, usb_status_string(status));

	status = lan7800_write_reg(&usb_devices[3], LAN7800_ADDRL, 0xDEADBEEF);
	printf("\nwrite status: %s\n", usb_status_string(status));

	status = lan7800_read_reg(&usb_devices[3], LAN7800_ADDRL, &val);
	printf("\nRX_ADDRL: 0x%08X\nstatus: %s\n", val, usb_status_string(status));
	printf("\n====END READ/WRITE TEST:====\n");

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

	printf("\n\n====START MAC TEST:====\n");
	
	/* Attempt to set the MAC address */
	status = lan7800_set_mac_address(&usb_devices[3], ethptr->devAddress);
	printf("\nMAC SET STATUS: %s\n", usb_status_string(status));	

	/* Read the MAC address */
	status = lan7800_get_mac_address(&usb_devices[3], ethptr->devAddress);
	printf("\nMAC GET STATUS: %s\n", usb_status_string(status));
	printf("\nGET MAC: ");

	/* [for debugging] Print MAC bits 0-5 which are set in lan7800_set_mac_address() */
	int i;
	for(i = 0; i < 6; i ++){
		printf("%d", ethptr->devAddress[i]);
	}

	printf("\n\n");

	return 0;
}
