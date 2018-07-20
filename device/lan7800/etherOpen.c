/**
 * @file lan7800_open.c
 *
 * @authors
 * 	Patrick J. McGee
 * 	Rade Latinovich
 *
 * Code for opening a Microchip LAN7800 USB Ethernet Adapter device.
 */
/* Embedded Xinu, Copyright (C) 2018.  All rights reserved. */

#include <bufpool.h>
#include <ether.h>
#include <stdlib.h>
#include <string.h>
#include <usb_core_driver.h>
#include "lan7800.h"

/* Implementation of etherOpen() for the lan7800; see the documentation for
 * this function in ether.h.  */
devcall etherOpen(device *devptr)
{
    struct ether *ethptr;
    struct usb_device *udev;
    irqmask im;
    uint i;
    int retval = SYSERR;

    im = disable();

    /* Wait for USB device to actually be attached.  */
    if (lan7800_wait_device_attached(devptr->minor) != USB_STATUS_SUCCESS)
    {
	    kprintf("\r\nfailed to wait...\r\n");
	    goto out_restore;
    }

    /* Fail if device is not down.  */
    ethptr = &ethertab[devptr->minor];
    if (ethptr->state != ETH_STATE_DOWN)
    {
	    kprintf("device is not down, cannot open.\r\n");
        goto out_restore;
    }

    /* Create buffer pool for Tx transfers.  */
    ethptr->outPool = bfpalloc(sizeof(struct usb_xfer_request) + ETH_MAX_PKT_LEN +
                                   LAN7800_TX_OVERHEAD,
                               LAN7800_MAX_TX_REQUESTS);
    if (ethptr->outPool == SYSERR)
    {
	    kprintf("outpool syserr\r\n");
        goto out_restore;
    }

    /* Create buffer pool for Rx packets (not the actual USB transfers, which
     * are allocated separately).  */
    ethptr->inPool = bfpalloc(sizeof(struct ethPktBuffer) + ETH_MAX_PKT_LEN,
                              ETH_IBLEN);
    if (ethptr->inPool == SYSERR)
    {
	    kprintf("\r\ninpool syserr\r\n");
        goto out_free_out_pool;
    }

    /* We're abusing the csr field to store a pointer to the USB device
     * structure.  At least it's somewhat equivalent, since it's what we need to
     * actually communicate with the device hardware.  */
    udev = ethptr->csr;

    /* Set MAC address */
    if (lan7800_set_mac_address(udev, ethptr->devAddress) != USB_STATUS_SUCCESS)
    {
	    kprintf("Failed to set MAC\r\n");
    	goto out_free_in_pool;
    }
    kprintf("made it here...001, max tx requests= %d\r\n", LAN7800_MAX_TX_REQUESTS);
    
    /* Initialize the Tx requests.  */
    struct usb_xfer_request *reqs[LAN7800_MAX_TX_REQUESTS];
    for (i = 0; i < LAN7800_MAX_TX_REQUESTS; i++)
    {
        struct usb_xfer_request *req;
        req = bufget(ethptr->outPool);
        usb_init_xfer_request(req);
        req->dev = udev;
        /* Assign Tx endpoint, checked in lan7800_bind_device() */
        req->endpoint_desc = udev->endpoints[0][1];
        req->sendbuf = (uint8_t*)req + sizeof(struct usb_xfer_request);
        req->completion_cb_func = lan7800_tx_complete;
        req->private = ethptr;
        reqs[i] = req;
    }
    for (i = 0; i < LAN7800_MAX_TX_REQUESTS; i++)
    {
        buffree(reqs[i]);
    }

    kprintf("made it here...002, max rx requests= %d\r\n", LAN7800_MAX_RX_REQUESTS);
    /* Allocate and submit the Rx requests.  TODO: these aren't freed anywhere.
     * */
    for (i = 0; i < LAN7800_MAX_RX_REQUESTS; i++)
    {
	    kprintf("loop %d ", i);
        struct usb_xfer_request *req;
        req = usb_alloc_xfer_request(LAN7800_DEFAULT_HS_BURST_CAP_SIZE);
        if (req == NULL)
        {
		kprintf("NULL REQ\r\n");
            goto out_free_in_pool;
        }
        req->dev = udev;
        /* Assign Rx endpoint, checked in lan7800_bind_device() */
        req->endpoint_desc = udev->endpoints[0][0];
        req->completion_cb_func = lan7800_rx_complete;
        req->private = ethptr;
        usb_submit_xfer_request(req);
    }


    kprintf("made it here...003\r\n");
    /* Enable transmit and receive on the actual hardware.  After doing this and
     * restoring interrupts, the Rx transfers can complete at any time due to
     * incoming packets.  */
    udev->last_error = USB_STATUS_SUCCESS;

    kprintf("made it here...004\r\n");
    /* MAC Layer */
    lan7800_set_reg_bits(udev, LAN7800_MAC_CR, LAN7800_MAC_CR_TXEN | LAN7800_MAC_CR_RXEN);

    /* Physical (PHY) layer
     * The following line uses the TX control registers that I believe are correct,
     * as used in torvalds/linux for its lan78xx device driver. */
    lan7800_write_reg(udev, LAN7800_FCT_TX_CTL, LAN7800_FCT_TX_CTL_EN);
    
    if (udev->last_error != USB_STATUS_SUCCESS)
    {
	    kprintf("ERROR\r\n");
        goto out_free_in_pool;
    }

    kprintf("\r\nETHER IS OPEN!\r\n");
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
