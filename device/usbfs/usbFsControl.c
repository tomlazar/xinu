/**
 * @file usbKbdControl.c
 */
/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */

#include <usbfs.h>
#include <interrupt.h>

/**
 * Control parameters to a USB keyboard.
 * @param devptr  pointer to USB keyboard device
 * @param func  index of function to run (defined in usbkbd.h)
 * @param arg1  first argument to called function
 * @param arg2  second argument to called function
 *
 * @return
 *      SYSERR if control function not recognized; otherwise a
 *      control-function-dependent value.
 */
devcall usbFsControl(device *devptr, int func, long arg1, long arg2)
{
    irqmask im;
    struct usbfs *fs;
    int retval;

    USBFS_TRACE("devptr->minor=%u, func=%d, arg1=%ld, arg2=%ld",
                 (uint)devptr->minor, func, arg1, arg2);

    im = disable();
    fs = &usbfstab[devptr->minor];
    retval = SYSERR;

    if (!fs->initialized)
    {
        restore(im);
        return retval;
    }

    switch (func)
    {
    case USBFS_CTRL_SET_IFLAG:
        retval = fs->iflags & arg1;
        fs->iflags |= arg1;
        break;

    case USBFS_CTRL_CLR_IFLAG:
        retval = fs->iflags & arg1;
        fs->iflags &= ~arg1;
        break;

    case USBFS_CTRL_GET_IFLAG:
        retval = fs->iflags;
        break;
    }
    restore(im);
    return retval;
}
