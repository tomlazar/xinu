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

// XXX HACK seting NETHER here due to weird stuff with build system
#define NETHER 1

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
	// Dummy return statement
	return USB_STATUS_DEVICE_UNSUPPORTED;
}

/**
 * Unbinds the LAN7800 driver from the LAN7800 device that has been detached.
 */
static void
lan7800_unbind_device(struct usb_device *udev)
{
	
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
 * 	@return
 * 		Currently ::USB_STATUS_SUCCESS.
 *
 */
usb_status_t
lan7800_wait_device_attached(ushort minor)
{
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
	kprintf("IN ETHERINIT(): I am here..\r\n");

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
