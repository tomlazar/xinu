/**
 * @file usbFsInit.c
 */
/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */

#include <usb_core_driver.h>
#include <usbfs.h>

/** Table of USB keyboard control blocks  */
struct usbfs usbfstab[NUSBFS];

/** USB device driver structure for the usbkbd driver  */
static struct usb_device_driver usbfs_driver = {
    .name          = "USB Mass Storage Device driver (MSC boot protocol)",
    .bind_device   = usbFsBindDevice,
    .unbind_device = usbFsUnbindDevice,
};

/**
 * Initializes the specified USB keyboard.
 *
 * This actually only prepares the corresponding keyboard structure for use and
 * does not depend on a physical keyboard being attached.  The physical keyboard
 * is recognized only when it is attached, and any read requests to the device
 * will block until that point.
 */
devcall usbFsInit(device *devptr)
{
    usb_status_t status;
    struct usbfs *fs;

    fs = &usbfstab[devptr->minor];

    USBFS_TRACE("Initializing usb filesystem");

    /* Already initialized?  */
    if (fs->initialized)
    {
        goto err;
    }

    /* Initialize input queue.  */
    fs->isema = semcreate(0);
    if (SYSERR == fs->isema)
    {
        goto err;
    }

    /* Allocate USB transfer request for keyboard data.  */
    fs->intr = usb_alloc_xfer_request(512);
    if (NULL == fs->intr)
    {
        goto err_semfree;
    }

    fs->outr = usb_alloc_xfer_request(512);
    if (NULL == fs->outr) 
    {
        goto err_semfree;
    }

    fs->initialized = TRUE;

    /* Register the keyboard USB device driver with the USB subsystem.
     * (no-op if already registered).  */
    status = usb_register_device_driver(&usbfs_driver);
    if (status != USB_STATUS_SUCCESS)
    {
        goto err_free_req;
    }

    return OK;

err_free_req:
    fs->initialized = FALSE;
    usb_free_xfer_request(fs->intr);
err_semfree:
    semfree(fs->isema);
err:
    return SYSERR;
}
