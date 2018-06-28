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
#include <kernel.h>

/* Global table of Ethernet devices. */
struct ether ethertab[NETHER];

/**
 * Semaphores that indicate whether each given Ethernet device is connected to the system
 * yet (in our case, attached to USB).
 */
static semaphore lan7800_attached[NETHER];

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
	kprintf("\r\n\n\nENTER DEVICE UNSUPPORTED 1:\r\n\nDUMP REAL VS. EXPECTED VALUES:\r\n==============================");
	kprintf("\r\nudev->descriptor.idVendor: 0x%08X", udev->descriptor.idVendor);
	kprintf("\r\nudev->descriptor.idProduct: 0x%08X", udev->descriptor.idProduct);
	kprintf("\r\nudev->interfaces[0]->bNumEndpoints (should be >= 2 ?): %d", udev->interfaces[0]->bNumEndpoints);
	kprintf("\r\nudev->endpoints[0][0]->bmAttributes & 0x3: 0x%08X, expected: 0x%08X", udev->endpoints[0][0]->bmAttributes & 0x3,
										USB_TRANSFER_TYPE_BULK);
	kprintf("\r\nudev->endpoints[0][1]->bmAttributes & 0x3: 0x%08X, expected: 0x%08X", udev->endpoints[0][1]->bmAttributes & 0x3,
										USB_TRANSFER_TYPE_BULK);
	kprintf("\r\nudev->endpoints[0][0]->bEndpointAddress >> 7: 0x%08X, expected: 0x%08X", udev->endpoints[0][0]->bEndpointAddress >> 7,
										USB_DIRECTION_IN);
	kprintf("\r\nudev->endpoints[0][1]->bEndpointAddress >> 7: 0x%08X, expected: 0x%08X", udev->endpoints[0][1]->bEndpointAddress >> 7,
										USB_DIRECTION_OUT);
	kprintf("\r\nudev->speed: 0x%08X, expected: 0x%08X", udev->speed, USB_SPEED_HIGH);

	return USB_STATUS_DEVICE_UNSUPPORTED;
    }
    /* Make sure this driver isn't already bound to a SMSC LAN9512.
     *      * TODO: Support multiple devices of this type concurrently.  */
    ethptr = &ethertab[0];
    STATIC_ASSERT(NETHER == 1);
    if (ethptr->csr != NULL)
    {
	kprintf("\r\nDEVICE UNSUPPORTED 2\r\n");
	return USB_STATUS_DEVICE_UNSUPPORTED;
    }

/* The rest of this function is responsible for making the SMSC LAN9512
 * ready to use, but not actually enabling Rx and Tx (which is done in
 * etherOpen()).  This primarily involves writing to the registers on the
 * SMSC LAN9512.  But these are not memory mapped registers, as this is a
 * USB Ethernet Adapter that is attached on the USB!  Instead, registers are
 * read and written using USB control transfers.  It's somewhat of a pain,
 * and also unlike memory accesses it is possible for USB control transfers
 * to fail.  However, here we perform lazy error checking where we just do
 * all the needed reads and writes, then check at the end if an error
 * occurred. */

    udev->last_error = USB_STATUS_SUCCESS;

    /* Resetting the SMSC LAN9512 via its registers should not be necessary
     * because the USB code already performed a reset on the USB port it's
     * attached to.  */

    /* Set MAC address.  */
    lan7800_set_mac_address(udev, ethptr->devAddress);

    /* Allow multiple Ethernet frames to be received in a single USB transfer.
     * Also set a couple flags of unknown function.  */
    lan7800_set_reg_bits(udev, LAN7800_HW_CFG, LAN7800_HW_CFG_MEF | LAN7800_CFG_BIR | LAN7800_CFG_BCE);

    /* Set the maximum USB (not networking!) packets per USB Rx transfer.
     * Required when HW_CFG_MEF was set.  */
    lan7800_write_reg(udev, LAN7800_BURST_CAP,
                       LAN7800_DEFAULT_HS_BURST_CAP_SIZE / LAN7800_HS_USB_PKT_SIZE);

    /* Check for error and return.  */
    if (udev->last_error != USB_STATUS_SUCCESS)
    {
	kprintf("\r\nLAST ERROR\r\n");
    	return udev->last_error;
    }
    
    ethptr->csr = udev;
    udev->driver_private = ethptr;
    signal(lan7800_attached[ethptr - ethertab]);

    kprintf("\r\nBEFORE BIND SUCCESS.\r\n");
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
	.name			= "LAN7800 USB Ethernet Adapter Driver",
	.bind_device	= lan7800_bind_device,
	.unbind_device	= lan7800_unbind_device,
};

static void
randomEthAddr(uchar addr[ETH_ADDR_LEN])
{
	uint i;
	srand(clkcount());
	for (i = 0; i < ETH_ADDR_LEN; i++)
	{
		addr[i] = rand();
	}
	/* Clear multicast bit and set locally assigned bit */
	addr[0] &= 0xFE;
	addr[0] |= 0x02;
}

/**
 * @ingroup etherspecific
 *
 * Wait until the specified Ethernet device has been attached.
 *
 * This is necessary because USB is a dynamic bus, but Xinu expects static devices.
 *
 * @param minor
 * 		Minor number of the Ethernet device to wait for.
 *
 * @return
 * 		Currently ::USB_STATUS_SUCCESS.
 *
 */
usb_status_t
lan7800_wait_device_attached(ushort minor)
{
	kprintf("\r\nWAIT FOR DEV ATTACH\r\n");
	wait(lan7800_attached[minor]);
	signal(lan7800_attached[minor]);
	return USB_STATUS_SUCCESS;
}

/* Implementation of etherInit() for lan7800; documentation in ether.h */
/**
 * @details
 *
 * LAN7800-specific notes: This function returns ::OK if the Ethernet
 * Driver was successfully registered with the USB core, otherwise ::SYSERR.
 * This is a work-around to use USB's dynamic device model at the same time as
 * Xinu's static device model, and there is no guarantee that the device
 * actually exists when the function returns.
 */
devcall etherInit(device *devptr)
{
	kprintf("\r\n<<<BEGIN ETHER INIT>>>\r\n");
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

	lan7800_attached[devptr->minor] = semcreate(0);

	if (isbadsem(lan7800_attached[devptr->minor]))
	{
		kprintf("\r\nBAD SEM\r\n");
		goto err_free_isema;
	}

    /* THE LAN7800 on the Raspberry Pi 3B+ does not have an EEPROM attached,
     * The EEPROM is normally used to store MAC address of the adapter,
     * along with some other information. As a result, software needs to set
     * the MAC address to a value of its choosing (random number...). */
     randomEthAddr(ethptr->devAddress);

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
