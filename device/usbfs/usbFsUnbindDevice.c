/**
 * @file usbUnbindDevice.c
 */
/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */

#include <usb_core_driver.h>
#include <usbfs.h>

/**
 * Function called by the USB subsystem when a keyboard bound with
 * usbKbdBindDevice() has been detached.
 */
void usbFsUnbindDevice(struct usb_device *dev)
{
    struct usbfs *fs;

    USBFS_TRACE("USB storage device disconnected (%s %s: address %u)",
                 dev->manufacturer, dev->product, dev->address);
    fs = dev->driver_private;
    fs->attached = FALSE;
}

