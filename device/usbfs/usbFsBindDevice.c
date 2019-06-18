/** 
 * @file usbFsBinddevice
 */
/* Embedded Xinu, Copyright (C) 2013. All rights reserved. */

#include <usb_core_driver.h>
#include <usbfs.h>
#include <usbkbd.h>

#define MSC_SUBCLASS_BOOT	 	6
#define MSC_BOOT_PROTOCOL_STORAGE	0x50
#define HID_REQUEST_SET_PROTOCOL        0x0B
#define HID_BOOT_PROTOCOL               0

usb_status_t usbFsBindDevice(struct usb_device *dev) {
    uint i, j;
    const struct usb_interface_descriptor *fs_interface;
    const struct usb_endpoint_descriptor *in_bulk_endpoint;
    const struct usb_endpoint_descriptor *out_bulk_endpoint;
    struct usbfs *fs;

    usb_status_t status;

    USBFS_TRACE("Attempting to bind USB device (%s %s: address %u)", dev->manufacturer, dev->product, dev->address);

    if (USB_CLASS_CODE_INTERFACE_SPECIFIC != dev->descriptor.bDeviceClass) {
        USBFS_TRACE("Device does not have interface specific class: "
	            "can't have any MSC interfaces!");
	return USB_STATUS_DEVICE_UNSUPPORTED;
    }

    USBFS_TRACE("Looking for MSC interface supporting filesystem protocol");
    fs_interface = NULL;

    for (i = 0; i < dev->config_descriptor->bNumInterfaces; i++) {
        struct usb_interface_descriptor *interface = dev->interfaces[i];

	USBFS_TRACE("Examining interface (index=%u, bInterfaceNumber=%u)",
	            i, interface->bInterfaceNumber);
        if (USB_CLASS_CODE_MASS_STORAGE != interface->bInterfaceClass) {
	    USBFS_TRACE("Not a MSC interface");
	    continue;
	}

	if (MSC_SUBCLASS_BOOT != interface->bInterfaceSubClass ||
	    MSC_BOOT_PROTOCOL_STORAGE != interface->bInterfaceProtocol) {
	    USBFS_TRACE("Interface does not support storage boot protocol");
	    continue;
	}
        
	/* Find the IN and OUT endpoints. */
	in_bulk_endpoint = NULL;
	out_bulk_endpoint = NULL;

	for (j = 0; j < interface->bNumEndpoints; j++) {
	    if ((dev->endpoints[i][j]->bmAttributes & 0x3) ==
	        USB_TRANSFER_TYPE_BULK
		&& (dev->endpoints[i][j]->bEndpointAddress >> 7) ==
		USB_DIRECTION_IN)
	    {
	        in_bulk_endpoint = dev->endpoints[i][j];
		continue;
	    }
	    if ((dev->endpoints[i][j]->bmAttributes & 0x3) ==
	        USB_TRANSFER_TYPE_BULK
		&& (dev->endpoints[i][j]->bEndpointAddress >> 7) ==
		USB_DIRECTION_OUT)
	    {
	        out_bulk_endpoint = dev->endpoints[i][j];
		continue;
	    }
	}

        if (NULL == in_bulk_endpoint) {
	    USBFS_TRACE("Interface has no IN bulk endpoint");
	    continue;
	}

	if (NULL == out_bulk_endpoint) {
	    USBFS_TRACE("Interface has no OUT bulk endpoint");
	    continue;
	}

	USBFS_TRACE("Interface is supported");
	fs_interface = interface;
	break;
    }
    if (NULL == fs_interface) {
        USBFS_TRACE("No MSC interface with filesystem boot protocol found");
	return USB_STATUS_DEVICE_UNSUPPORTED;
    }

    /* Map this newly attached USB Mass Storage to usbfs structure. */
    fs = NULL;
    for (i = 0; i < NUSBFS; i++) {
        if (usbfstab[i].initialized && !usbfstab[i].attached) {
	    USBFS_TRACE("Using USB filesystem control block %u", i);
	    fs = &usbfstab[i];
	    break;
	}
    }

    if (NULL == fs) {
        USBFS_TRACE("All initialized filesystem devices already in use");
	return USB_STATUS_DEVICE_UNSUPPORTED;
    }

    /* Put keyboard in boot protocol mode, not report mode. */
    {
        usb_status_t status;

	USBFS_TRACE("Placing filesystem in boot protocol mode");
    //    status = usb_control_msg(dev, NULL, HID_REQUEST_SET_PROTOCOL,
//	                         USB_BMREQUESTTYPE_TYPE_CLASS |
//				     USB_BMREQUESTTYPE_DIR_OUT |
//				     USB_BMREQUESTTYPE_RECIPIENT_INTERFACE,
//				 HID_BOOT_PROTOCOL,
//				 fs_interface->bInterfaceNumber, NULL, 0);
	if (USB_STATUS_SUCCESS != status) {
	    USBFS_TRACE("Failed to place filesystem in boot protocol mode: %s", usb_status_string(status));
	    return USB_STATUS_DEVICE_UNSUPPORTED;
	}
    }

    dev->driver_private = fs;

    fs->intr->dev = dev;
    fs->intr->endpoint_desc = in_bulk_endpoint;
    fs->intr->private = dev;

    USBFS_TRACE("Asking filesystem for data");
    //status = usb_submit_xfer_request(fs->intr);
    if (USB_STATUS_SUCCESS != status) {
        return status;
    }
    fs->attached = TRUE;

    return USB_STATUS_SUCCESS;
}
