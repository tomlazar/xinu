/**
 * @file     lan7800_interrupt.c
 *
 * @authors Patrick J. McGee
 * 	    Rade Latinovich
 *
 * This file provides USB transfer completion callbacks for the
 * Microchip LAN7800 USB Ethernet Adapter. 
 *
 */
/* Embedded Xinu, Copyright (C) 2018.  All rights reserved. */

#include "lan7800.h"
#include <bufpool.h>
#include <ether.h>
#include <string.h>
#include <usb_core_driver.h>
#include <kernel.h>

/**
 * @ingroup etherspecific
 *
 * Callback function executed with interrupts disabled when an asynchronous USB
 * bulk transfer to the Bulk OUT endpoint of the Microchip LAN7800 USB Ethernet
 * Adapter for the purpose of sending an Ethernet packet has successfully
 * completed or has failed.
 *
 * Currently all this function has to do is return the buffer to its pool.  This
 * may wake up a thread in etherWrite() that is waiting for a free buffer.
 *
 * @param req
 *      USB bulk OUT transfer request that has completed.
 */
void lan7800_tx_complete(struct usb_xfer_request *req)
{
	struct ether *ethptr = req->private;

	ethptr->txirq++;
	usb_dev_debug(req->dev, "\r\n\nLAN7800: Tx complete\r\n");
	buffree(req);
}

/**
 * @ingroup etherspecific
 *
 * Callback function executed with interrupts disabled when an asynchronous USB
 * bulk transfer from the Bulk IN endpoint of the Microchip LAN7800 USB Ethernet
 * Adapter for the purpose of receiving one or more Ethernet packets has
 * successfully completed or has failed.
 *
 * This function is responsible for breaking up the raw USB transfer data into
 * the constituent Ethernet packet(s), then pushing them onto the incoming
 * packets queue (which may wake up threads in etherRead() that are waiting for
 * new packets).  It then must re-submit the USB bulk transfer request so that
 * packets can continue to be received.
 *
 * @param req
 *      USB bulk IN transfer request that has completed.
 */
void lan7800_rx_complete(struct usb_xfer_request *req)
{
	struct ether *ethptr = req->private;

	usb_dev_debug(req->dev, "\r\n\nLAN7800: Rx complete\r\n");
	ethptr->rxirq++;
	usb_dev_debug(req->dev, "req->status = %d\r\n", req->status);
	usb_dev_debug(req->dev, "(req->status == USB_STATUS_SUCCESS) ? = %s\r\n",
			(req->status == USB_STATUS_SUCCESS) ? "TRUE" : "FALSE");
	usb_dev_debug(req->dev, "req->recvbuf = 0x%08X\r\n", req->recvbuf);
	usb_dev_debug(req->dev, "req->actual_size = %d\r\n", req->actual_size);
	if (req->status == USB_STATUS_SUCCESS)
	{
		const uint8_t *data, *edata;
		uint32_t frame_length;

		uint32_t rx_cmd_a;

		/* For each Ethernet frame in the received USB data... */
		for (data = req->recvbuf, edata = req->recvbuf + req->actual_size;
				data + RX_OVERHEAD + ETH_HDR_LEN + ETH_CRC_LEN <= edata;
				data += RX_OVERHEAD + ((frame_length + 3) & ~3))
		{
			/* Get the RxA, RxB, RxC status word, which contains information about the next
			 * Ethernet frame.  */
			rx_cmd_a = data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24;

			/* Extract frame_length, which specifies the length of the next
			 * Ethernet frame from the MAC destination address to end of the CRC
			 * following the payload.  (This does not include the Rx status
			 * words, which we instead account for in RX_OVERHEAD.) */
			frame_length = (rx_cmd_a & RX_CMD_A_LEN_MASK);

			if ((rx_cmd_a & RX_CMD_A_RED) ||
					(frame_length + RX_OVERHEAD > edata - data) ||
					(frame_length > ETH_MAX_PKT_LEN + ETH_CRC_LEN) ||
					(frame_length < ETH_HDR_LEN + ETH_CRC_LEN))
			{
				/* The Ethernet adapter set the error flag to indicate a problem
				 * or the Ethernet frame size it provided was invalid. */
				//usb_dev_debug(req->dev, "LAN78XX: Tallying rx error "
						//"(recv_status=0x%08x, frame_length=%u)\n",
						//recv_status, frame_length);
				ethptr->errors++;
			}
			else if (ethptr->icount == ETH_IBLEN)
			{
				/* No space to buffer another received packet.  */
				usb_dev_debug(req->dev, "LAN78XX: Tallying overrun\n");
				ethptr->ovrrun++;
			}
			else
			{
				/* Buffer the received packet.  */
				struct ethPktBuffer *pkt;

				pkt = bufget(ethptr->inPool);
				pkt->buf = pkt->data = (uint8_t*)(pkt + 1);
				pkt->length = frame_length - ETH_CRC_LEN;
				memcpy(pkt->buf, data + RX_OVERHEAD, pkt->length);
				ethptr->in[(ethptr->istart + ethptr->icount) % ETH_IBLEN] = pkt;
				ethptr->icount++;

				usb_dev_debug(req->dev, "LAN78XX: Receiving "
						"packet (length=%u, icount=%u)\n",
						pkt->length, ethptr->icount);

				/* This may wake up a thread in etherRead().  */
				signal(ethptr->isema);
			}
		}
	}
	else
	{
		/* USB transfer failed for some reason.  */
		usb_dev_debug(req->dev, "\r\n\nLAN78XX: USB Rx transfer failed\n");
		ethptr->errors++;
	}
	usb_dev_debug(req->dev, "LAN78XX: Re-submitting USB Rx request\n");
	usb_submit_xfer_request(req);

}


