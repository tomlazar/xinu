/**
 * @file	etherInit.c
 *
 * @authors
 * 			Rade Latinovich
 * 			Patrick J. McGee
 *
 * Initialization for the LAN7800 USB Ethernet Adapter
 */
/* Embedded Xinu, Copyright (C) 2018. All rights reserved. */

#include "lan7800.h"
#include <clock.h>
#include <ether.h>
#include <memory.h>
#include <platform.h>
#include <semaphore.h>
#include <stdlib.h>
#include <usb_core_driver.h>
#include "../../system/platforms/arm-rpi3/bcm2837_mbox.h"
#include <string.h>
#include <kernel.h>
#include "../system/platforms/arm-rpi3/bcm2837.h"

bool lan7800_isattached = 0;

/* Global table of Ethernet devices. */
struct ether ethertab[NETHER];

/**
 * Semaphores that indicate whether each given Ethernet device is connected to the system
 * yet (in our case, attached to USB).
 */
static semaphore lan7800_attached[NETHER];

/* Global variable which contains the MAC address after
 * getEthAddr() is called. This is later copied to the
 * devAddress member of the ether struct using memcpy(). */
uchar addr[ETH_ADDR_LEN] = {0};

/**
 * Try to bind the LAN7800 driver to a specific USB device.
 */
static usb_status_t
lan7800_bind_device(struct usb_device *udev)
{
	struct ether *ethptr;

	/* Check if this is actually a LAN7800 by checking the USB device's
	 * standard device descriptor, which the USB core already read into memory.
	 * Also check to make sure the expected endpoints for sending/receiving
	 * packets are present and that the device is operating at high speed.  */
	if (udev->descriptor.idVendor != LAN7800_VENDOR_ID ||
			udev->descriptor.idProduct != LAN7800_PRODUCT_ID ||
			udev->interfaces[0]->bNumEndpoints < 2 ||
			(udev->endpoints[0][0]->bmAttributes & 0x3) != USB_TRANSFER_TYPE_BULK ||
			(udev->endpoints[0][1]->bmAttributes & 0x3) != USB_TRANSFER_TYPE_BULK ||
			(udev->endpoints[0][0]->bEndpointAddress >> 7) != USB_DIRECTION_IN ||
			(udev->endpoints[0][1]->bEndpointAddress >> 7) != USB_DIRECTION_OUT ||
			udev->speed != USB_SPEED_HIGH)
	{
		return USB_STATUS_DEVICE_UNSUPPORTED;
	}

	/* Make sure this driver isn't already bound to a LAN7800.
	 *      * TODO: Support multiple devices of this type concurrently.  */
	ethptr = &ethertab[0];
	STATIC_ASSERT(NETHER == 1);
	if (ethptr->csr != NULL)
	{
		return USB_STATUS_DEVICE_UNSUPPORTED;
	}

	/* The rest of this function is responsible for making the LAN7800
	 * ready to use, but not actually enabling Rx and Tx (which is done in
	 * etherOpen()).  This primarily involves writing to the registers on the
	 * LAN7800.  But these are not memory mapped registers, as this is a
	 * USB Ethernet Adapter that is attached on the USB!  Instead, registers are
	 * read and written using USB control transfers.  It's somewhat of a pain,
	 * and also unlike memory accesses it is possible for USB control transfers
	 * to fail.  However, here we perform lazy error checking where we just do
	 * all the needed reads and writes, then check at the end if an error
	 * occurred. */
	udev->last_error = USB_STATUS_SUCCESS;

	/* Check for error and return.  */
	if (udev->last_error != USB_STATUS_SUCCESS)
	{
		return udev->last_error;
	}

	ethptr->csr = udev;
	udev->driver_private = ethptr;

	signal(lan7800_attached[ethptr - ethertab]);
	
	return USB_STATUS_SUCCESS;
}

/**
 * Unbinds the LAN7800 driver from the LAN7800 device that has been detached.
 */
static void
lan7800_unbind_device(struct usb_device *udev)
{

	struct ether *ethptr = udev->driver_private;

	/* Reset attached semaphore to 0.  */
	wait(lan7800_attached[ethptr - ethertab]);

	/* Close the device.  */
	etherClose(ethptr->dev);

}

/**
 * Specification of a USB device driver for the LAN7800 device. This is for the USB core subsystem
 * and not related to Xinu's primary device and driver model.
 */
static const struct usb_device_driver lan7800_driver = {
	.name		= "LAN7800 USB Ethernet Adapter Driver",
	.bind_device	= lan7800_bind_device,
	.unbind_device	= lan7800_unbind_device,
};

/* Get static MAC address from Pi 3 B+ chip, based on the XinuPi
 * mailbox technique.
 *
 * @details
 *
 * Get the Pi 3 B+'s MAC address using its ARM->VideoCore (VC) mailbox
 * and assign corresponding values to a global array containing the MAC. 
 * This array is then assigned to the devAddress member of the ether structure.
 */
static void
getEthAddr(uint8_t *addr)
{
	/* Initialize the mailbox buffer */
	uint32_t *mailbuffer;
	mailbuffer = dma_buf_alloc(MBOX_BUFLEN / 4);

	/* Fill the mailbox buffer with the MAC address.
	 * This function is defined in system/platforms/arm-rpi3/bcm2837_mbox.c */
	get_mac_mailbox(mailbuffer);

	ushort i;
	for (i = 0; i < 2; ++i) {

		/* Access the MAC value within the buffer */
		uint32_t value = mailbuffer[MBOX_HEADER_LENGTH + TAG_HEADER_LENGTH + i];

		/* Store the low MAC values */
		if(i == 0){
			addr[0] = (value >> 0)  & 0xff;
			addr[1] = (value >> 8)  & 0xff;
			addr[2] = (value >> 16) & 0xff;
			addr[3] = (value >> 24) & 0xff;
		}

		/* Store the remaining high MAC values */
		if(i == 1){
			addr[4] = (value >> 0)  & 0xff;
			addr[5] = (value >> 8)  & 0xff;
		}
	}
}

/**
 * @ingroup etherspecific
 * Wait until the specified Ethernet device has been attached.
 * This is necessary because USB is a dynamic bus, but Xinu expects static devices.
 * @param minor
 * 		Minor number of the Ethernet device to wait for.
 * @return
 * 		Currently ::USB_STATUS_SUCCESS.
 */
usb_status_t
lan7800_wait_device_attached(ushort minor)
{
	wait(lan7800_attached[minor]);
	signal(lan7800_attached[minor]);
	lan7800_isattached = 1;
	return USB_STATUS_SUCCESS;
}

/* Implementation of etherInit() for lan7800; documentation in ether.h */
/**
 * @details
 * LAN7800-specific notes: This function returns ::OK if the Ethernet
 * Driver was successfully registered with the USB core, otherwise ::SYSERR.
 * This is a work-around to use USB's dynamic device model at the same time as
 * Xinu's static device model, and there is no guarantee that the device
 * actually exists when the function returns.
 */
devcall etherInit(device *devptr)
{
	struct ether *ethptr;
	usb_status_t status;

	/* Initialize static ether struct for this device. */
	ethptr = &ethertab[devptr->minor];
	bzero(ethptr, sizeof(struct ether));
	ethptr->dev = devptr;
	ethptr->state = ETH_STATE_DOWN;
	ethptr->mtu = ETH_MTU;
	ethptr->addressLength = ETH_ADDR_LEN;
	ethptr->isema = semcreate(0);
	if (isbadsem(ethptr->isema))
	{
		goto err;
	}

	/* Create semaphore for device attachment. */
	lan7800_attached[devptr->minor] = semcreate(0);

	if (isbadsem(lan7800_attached[devptr->minor]))
	{
		goto err_free_isema;
	}

	/* Get the MAC address and store it into addr[] */
	getEthAddr(ethptr->devAddress);

	/* Register this device driver with the USB core and return. */
	status = usb_register_device_driver(&lan7800_driver);
	if (status != USB_STATUS_SUCCESS)
	{
		goto err_free_attached_sema;
	}

	return OK;

err_free_attached_sema:
	semfree(lan7800_attached[devptr->minor]);
err_free_isema:
	semfree(ethptr->isema);
err:
	return SYSERR;
}
