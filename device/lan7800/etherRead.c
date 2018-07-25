/**
 * @file etherRead.c
 *
 * Authors: Patrick J. McGee
 * 	    Rade Latinovich
 *
 */
/* Embedded Xinu, Copyright (C) 2018.  All rights reserved. */

#include <bufpool.h>
#include <ether.h>
#include <interrupt.h>
#include <string.h>

/* Implementation of etherRead() for the MicroChip LAN7800; see the documentation for
 * this function in ether.h.  */
devcall etherRead(device *devptr, void *buf, uint len)
{
    irqmask im;
    struct ether *ethptr;
    struct ethPktBuffer *pkt;

    im = disable();

    /* Make sure device is actually up.  */
    ethptr = &ethertab[devptr->minor];
    if (ethptr->state != ETH_STATE_UP)
    {
        restore(im);
	kprintf("Interrupts restored...error... device not up.\r\n");
        return SYSERR;
    }

    kprintf("[READ] Wait for available packet...\r\n");
    /* Wait for received packet to be available in the ethptr->in circular
     * queue.  */
    wait(ethptr->isema);

    kprintf("Remove received packet from circular queue.\r\n");

    /* Remove the received packet from the circular queue.  */
    pkt = ethptr->in[ethptr->istart];
    ethptr->istart = (ethptr->istart + 1) % ETH_IBLEN;
    ethptr->icount--;

    /* TODO: we'd like to restore interrupts here (before the memcpy()), but
     * this doesn't work yet because smsc9512_rx_complete() expects a buffer to
     * be available if icount < ETH_IBLEN; therefore, since we decremented
     * icount, we can't restore interrupts until we actually release the
     * corresponding buffer.  */

    /* Copy the data from the packet buffer, being careful to copy at most the
     * number of bytes requested. */
    if (pkt->length < len)
    {
        len = pkt->length;
    }
    kprintf("Copy the data from the packet buffer.\r\n");
    memcpy(buf, pkt->buf, len);

    /* Return the packet buffer to the pool, then return the length of the
     * packet received.  */
    buffree(pkt);
    restore(im);
    return len;
}
