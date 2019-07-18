/**
 * @file lan7800_open.c
 *
 * @authors
 * 	Patrick J. McGee
 * 	Rade Latinovich
 *
 * Routine for opening a Microchip LAN7800 USB Ethernet Adapter device.
 */
/* Embedded Xinu, Copyright (C) 2018.  All rights reserved. */

#include <bufpool.h>
#include <ether.h>
#include <stdlib.h>
#include <string.h>
#include <usb_core_driver.h>
#include "lan7800.h"

/* 
 * @ingroup etherspecific
 *
 * Implementation of etherOpen() for the lan7800; see the documentation for
 * this function in ether.h.  
 * @param devptr Pointer to ethernet device.
 * @return ::OK on success, ::SYSERR on failure.
 */
devcall etherOpen(device *devptr)
{
	struct ether *ethptr;
	struct usb_device *udev;
	irqmask im;
	int retval = SYSERR;

	im = disable();
	/* Wait for USB device to actually be attached.  */
	if (lan7800_wait_device_attached(devptr->minor) != USB_STATUS_SUCCESS)
	{
		goto out_restore;
	}

	/* Fail if device is not down.  */
	ethptr = &ethertab[devptr->minor];
	if (ethptr->state != ETH_STATE_DOWN)
	{
		goto out_restore;
	}

	/* Create buffer pool for Tx transfers.  */
#define  LAN7800_MAX_TX_REQUESTS 1
	ethptr->outPool = bfpalloc(sizeof(struct usb_xfer_request)
			+ ETH_MAX_PKT_LEN
			+ LAN7800_TX_OVERHEAD,
			LAN7800_MAX_TX_REQUESTS);
	if (ethptr->outPool == SYSERR)
	{
		goto out_restore;
	}

	/* Create buffer pool for Rx packets (not the actual USB transfers, which
	 * are allocated separately).  */
	ethptr->inPool = bfpalloc(sizeof(struct ethPktBuffer) + ETH_MAX_PKT_LEN,
			ETH_IBLEN);
	if (ethptr->inPool == SYSERR)
	{
		goto out_free_out_pool;
	}

	/* We're abusing the csr field to store a pointer to the USB device
	 * structure.  At least it's somewhat equivalent, since it's what we need to
	 * actually communicate with the device hardware.  */
	udev = ethptr->csr;


	/* The rest of this function is responsible for making the LAN78xx
	 * ready to use, but not actually enabling Rx and Tx (which is done in
	 * etherOpen()).  This primarily involves writing to the registers on the
	 * LAN78xx.  But these are not memory mapped registers, as this is a
	 * USB Ethernet Adapter that is attached on the USB!  Instead, registers are
	 * read and written using USB control transfers.  It's somewhat of a pain,
	 * and also unlike memory accesses it is possible for USB control transfers
	 * to fail.  However, here we perform lazy error checking where we just do
	 * all the needed reads and writes, then check at the end if an error
	 * occurred.  */
	udev->last_error = USB_STATUS_SUCCESS;

	retval = lan7800_init(udev, &ethptr->devAddress[0]);
	if (retval < 0) goto out_free_in_pool;

	/* Initialize the Tx requests.  */
	{
		struct usb_xfer_request *reqs[LAN7800_MAX_TX_REQUESTS];
		for (int i = 0; i < LAN7800_MAX_TX_REQUESTS; i++)
		{
			struct usb_xfer_request *req;

			req = bufget(ethptr->outPool);
			usb_init_xfer_request(req);
			req->dev = udev;
			/* Assign Tx endpoint, checked in lan78xx_bind_device() */
			req->endpoint_desc = udev->endpoints[0][1];
			req->sendbuf = (uint8_t*)req + sizeof(struct usb_xfer_request);
			req->completion_cb_func = lan7800_tx_complete;
			req->private = ethptr;
			reqs[i] = req;
		}
		for (int i = 0; i < LAN7800_MAX_TX_REQUESTS; i++)
		{
			buffree(reqs[i]);
		}
	}

	/* Allocate and submit the Rx requests.  TODO: these aren't freed anywhere. */
#define LAN7800_MAX_RX_REQUESTS 1
	for (int i = 0; i < LAN7800_MAX_RX_REQUESTS; i++)
	{
		struct usb_xfer_request *req;

		req = usb_alloc_xfer_request(LAN7800_DEFAULT_BURST_CAP_SIZE);
		if (req == NULL)
		{
			goto out_free_in_pool;
		}
		req->dev = udev;
		/* Assign Rx endpoint, checked in lan78xx_bind_device() */
		req->endpoint_desc = udev->endpoints[0][0];
		req->completion_cb_func = lan7800_rx_complete;
		req->private = ethptr;
		usb_submit_xfer_request(req);
	}

	/* Success!  Set the device to ETH_STATE_UP. */
	ethptr->state = ETH_STATE_UP;
	retval = OK;
	goto out_restore;

out_free_in_pool:
	bfpfree(ethptr->inPool);
out_free_out_pool:
	bfpfree(ethptr->outPool);
out_restore:
	restore(im);
	return retval;
}
