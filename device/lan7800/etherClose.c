/**
 * @file etherClose.c
 *
 * @authors
 * 		Rade Latinovich
 * 		Patrick J. McGee
 *
 */
/* Embedded Xinu, Copyright (C) 2018.  All rights reserved. */

#include <ether.h>

/* 
 * @ingroup etherspecific
 *
 * Implementation of etherClose() for the lan7800
 * @param devptr	Pointer to ethernet device to close
 * @return		::SYSERR, the device is never closed
 */
devcall etherClose(device *devptr)
{
    /* TODO: need to handle canceling all the outstanding USB requests, etc. */
    return SYSERR;
}
