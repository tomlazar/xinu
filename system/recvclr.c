/**
 * @file recvclr.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>

/**
 * @ingroup threads
 *
 * Clear messages, return waiting message (if any)
 * @return msg if available, NOMSG if no message
 */
message recvclr(void)
{
    register struct thrent *thrptr;
    irqmask im;
    message msg;

    im = disable();

	thrtab_acquire(thrcurrent);

    thrptr = &thrtab[thrcurrent];
    if (thrptr->hasmsg)
    {
        msg = thrptr->msg;
    }                           /* retrieve message       */
    else
    {
        msg = NOMSG;
    }
    thrptr->hasmsg = FALSE;     /* reset message flag   */

	thrtab_release(thrcurrent);

    restore(im);
    return msg;
}
