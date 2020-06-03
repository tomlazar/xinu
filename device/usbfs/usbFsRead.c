/**
 * @file usbKbdRead.c
 */
/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */

#include <usbfs.h>
#include <interrupt.h>

/**
 * Read data from a USB keyboard.
 *
 * @param devptr
 *      Pointer to the device table entry for a USB keyboard.
 * @param buf
 *      Pointer to a buffer into which to place the read data.
 * @param len
 *      Maximum number of bytes of data to read.
 *
 * @return
 *      On success, returns the number of bytes read, which normally is @p len,
 *      but may be less than @p len if the keyboard has been set to non-blocking
 *      mode.  Returns ::SYSERR on other error (currently, only if usbKbdInit()
 *      has not yet been called).
 */
devcall usbFsRead(device *devptr, void *buf, uint len)
{
    irqmask im;
    struct usbfs *fs;
    uint count;

    /* Disable interrupts and get a pointer to the keyboard structure.  */
    im = disable();
    fs = &usbfstab[devptr->minor];

    /* Make sure usbFsInit() has run.  */
    if (!fs->initialized)
    {
        restore(im);
        return SYSERR;
    }

    /* Attempt to read each byte requested.  */
    USBFS_TRACE("Attempting to read %u bytes from filesystem", len);
    for (count = 0; count < len; count++)
    {
        /* If the filesystem is in non-blocking mode, ensure there is a byte
         * available in the input buffer from the interrupt handler.  If not,
         * return early with a short count.  */
        if ((fs->iflags & USBFS_IFLAG_NOBLOCK) && fs->icount == 0)
        {
	    USBFS_TRACE("No bytes available in the input buffer from interrupt handler");
            break;
        }

        /* Wait for there to be at least one byte in the input buffer from the
         * interrupt handler, then remove it.  */
	USBFS_TRACE("Waiting for at least one bytes in the input buffer");
        wait(fs->isema);
        ((uchar*)buf)[count] = fs->in[fs->istart];
        fs->icount--;
        fs->istart = (fs->istart + 1) % USBFS_IBLEN;
	USBFS_TRACE("Received a byte");
    }

    /* Restore interrupts and return the number of bytes read.  */
    restore(im);
    USBFS_TRACE("Return count=%u", count);
    return count;
}
