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

#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size)			\
		ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

#define ALLOC_ALIGN_BUFFER(type, name, size, align)		\
		ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, 1)

#define ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, pad)		\
		char __##name[ROUND(PAD_SIZE((size) * sizeof(type), pad), align)  \
				      + (align - 1)];

#define PAD_SIZE(s, pad) (PAD_COUNT(s, pad) * pad)

/* 
 * @ingroup etherspecific
 *
 * Implementation of etherRead() for the MicroChip LAN7800; see the documentation for
 * this function in ether.h.  
 * @param devptr	Pointer to ethernet device to read from. 
 * @param buf		Pointer to packet buffer
 * @param len		Amount of bytes to be read
 * @return		Length of packet received
 */
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
        return SYSERR;
    }

    /* Wait for received packet to be available in the ethptr->in circular
     * queue.  */
    wait(ethptr->isema);

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
    memcpy(buf, pkt->buf, len);

    /* Return the packet buffer to the pool, then return the length of the
     * packet received.  */
    buffree(pkt);
    restore(im);
    return len;
}
