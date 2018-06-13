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
#include <usb_std_defs.h>

extern struct usb_device usb_devices[];
extern usb_status_t usb_read_device_descriptor(struct usb_device *, uint16_t);
extern struct usb_device *usb_root_hub;

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
	usb_status_t status1, status2;


	struct usb_device *ud = usb_alloc_device(usb_root_hub);

/*
	struct usb_device_descriptor *desc =
		(struct usb_device_descriptor *) malloc(sizeof(struct usb_device_descriptor));

	status1 = usb_get_descriptor(ud, USB_DEVICE_REQUEST_GET_DESCRIPTOR, 
							USB_BMREQUESTTYPE_DIR_IN |
							USB_BMREQUESTTYPE_TYPE_STANDARD |
							USB_BMREQUESTTYPE_RECIPIENT_DEVICE, 
							USB_DESCRIPTOR_TYPE_DEVICE << 8, 0x0, 
							desc, sizeof(struct usb_device_descriptor) );
	udelay(250);	
	printf("usb_get_descriptor status: %d, %s\n", status1, usb_status_string(status1));
	udelay(250);
*/
	

	status2 = usb_attach_device(ud);
	
	udelay(250);
	printf("\nusb_attach_device  status: %d, %s\n\n", status2, usb_status_string(status2));
	udelay(250);

/*
	status1 = usb_read_device_descriptor(ud, sizeof(ud->descriptor));
	udelay(250);
	printf("\nusb_read_device_descriptor status: %d, %s\n\n", status1, usb_status_string(status1));
	udelay(250);
*/
	
	usb_free_device(ud);

	return 0;
}
