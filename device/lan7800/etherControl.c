/**
 * @file etherControl.c
 *
 * Simple contoller to handle requests of the MicroChip LAN7800 device. 
 *
 * Authors: Patrick J. McGee
 * 	    Rade Latinovich
 */
/* Embedded Xinu, Copyright (C) 2008, 2013, 2018.  All rights reserved. */

#include <ether.h>
#include <network.h>
#include <string.h>
#include "lan7800.h"

/* 
 * @ingroup etherspecific
 *
 * Implementation of etherControl() for the LAN7800 device; see the documentation for
 * this function in ether.h.  
 * @param devptr	TODODOC desc. all params!
 * @param req		
 * @param arg1
 * @param arg2
 * @return TODODOC when does it return what
 */
devcall etherControl(device *devptr, int req, long arg1, long arg2)
{
    struct usb_device *udev;
    usb_status_t status;
    struct netaddr *addr;
    struct ether *ethptr;

    ethptr = &ethertab[devptr->minor];
    udev = ethptr->csr;
    if (udev == NULL)
    {
        return SYSERR;
    }

    status = USB_STATUS_SUCCESS;

    switch (req)
    {
    /* Program MAC address into device. */
    case ETH_CTRL_SET_MAC:
        status = lan7800_set_mac_address(udev, (const uchar*)arg1);
        break;

    /* Get MAC address from device. */
    case ETH_CTRL_GET_MAC:
        status = lan7800_get_mac_address(udev, (uchar*)arg1);
        break;

    /* Enable or disable loopback mode.  */
    case ETH_CTRL_SET_LOOPBK:;
	uint32_t buf;
	
	// disable tx and rx
	lan7800_read_reg(udev, LAN7800_MAC_RX, &buf);
	buf &= ~(LAN7800_MAC_RX_RXEN);
	lan7800_write_reg(udev, LAN7800_MAC_RX, buf);

	lan7800_read_reg(udev, LAN7800_MAC_TX, &buf);
	buf &= ~(LAN7800_MAC_TX_TXEN);
	lan7800_write_reg(udev, LAN7800_MAC_TX, buf);

	status = lan7800_modify_reg(udev, LAN7800_MAC_CR, ~MAC_CR_LOOPBACK,
                                     ((bool)arg1 == TRUE) ? MAC_CR_LOOPBACK : 0);
       	// enable tx and rx
	lan7800_read_reg(udev, LAN7800_MAC_RX, &buf);
	buf |= (LAN7800_MAC_RX_RXEN);
	lan7800_write_reg(udev, LAN7800_MAC_RX, buf);

	lan7800_read_reg(udev, LAN7800_MAC_TX, &buf);
	buf |= (LAN7800_MAC_TX_TXEN);
	lan7800_write_reg(udev, LAN7800_MAC_TX, buf);
 
	break;

    /* Get link header length. */
    case NET_GET_LINKHDRLEN:
        return ETH_HDR_LEN;

    /* Get MTU. */
    case NET_GET_MTU:
        return ETH_MTU;

    /* Get hardware address.  */
    case NET_GET_HWADDR:
        addr = (struct netaddr *)arg1;
        addr->type = NETADDR_ETHERNET;
        addr->len = ETH_ADDR_LEN;
        return etherControl(devptr, ETH_CTRL_GET_MAC, (long)addr->addr, 0);

    /* Get broadcast hardware address. */
    case NET_GET_HWBRC:
        addr = (struct netaddr *)arg1;
        addr->type = NETADDR_ETHERNET;
        addr->len = ETH_ADDR_LEN;
        memset(addr->addr, 0xFF, ETH_ADDR_LEN);
        break;

    default:
        return SYSERR;
    }

    if (status != USB_STATUS_SUCCESS)
    {
        return SYSERR;
    }

    return OK;
}
