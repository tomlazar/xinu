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

#include <usb_core_driver.h>
#include <usb_std_defs.h>

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

	int i;
	for (i = 0; i < 32; i++)
	{
		if (usb_devices[i].inuse == 1)
		{
			printf("%d: depth     = %d\n", i, usb_devices[i].depth);
			printf("%d: idProduct = 0x%04X\n", i, usb_devices[i].descriptor.idProduct);
			printf("%d: idVendor  = 0x%04X\n", i, usb_devices[i].descriptor.idVendor);
		}
	}

	return 0;

}
