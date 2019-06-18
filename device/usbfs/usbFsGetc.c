/**
 * @file usbKbdGetc.c
 */
/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */

#include <usbfs.h>

/**
 * Read a single character from a USB keyboard.
 *
 * @param devptr
 *      Pointer to the device table entry for a USB keyboard.
 *
 * @return
 *      On success, returns the character read as an <code>unsigned char</code>
 *      cast to an <code>int</code>.  If keyboard has not been initialized or is
 *      in non-blocking mode and no data is available, returns ::SYSERR.
 */
devcall usbFsGetc(device *devptr)
{
    uchar ch;
    int retval;

    USBFS_TRACE("usbFsGetc has been called on address");

    retval = usbFsRead(devptr, &ch, 1);
    if (retval == 1)
    {
        return ch;
    }
    else
    {
        return SYSERR;
    }
}
