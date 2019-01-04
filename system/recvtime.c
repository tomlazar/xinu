/**
 * @file recvtime.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <conf.h>
#include <stddef.h>
#include <thread.h>
#include <clock.h>

/**
 * @ingroup threads
 *
 * wait to receive a message or timeout and return result
 * @param  maxwait ticks to wait before timeout
 * @return msg if becomes available, TIMEOUT if no message
 */
message recvtime(int maxwait)
{
    register struct thrent *thrptr;
    irqmask im;
    message msg;

    if (maxwait < 0)
    {
        return SYSERR;
    }
    im = disable();
    thrptr = &thrtab[thrcurrent];
    if (FALSE == thrptr->hasmsg)
    {
#if RTCLOCK
        if (SYSERR == insertd(thrcurrent, sleepq, maxwait))
        {
            restore(im);
            return SYSERR;
        }

		thrtab_acquire(thrcurrent);

        thrtab[thrcurrent].state = THRTMOUT;

		thrtab_release(thrcurrent);

        resched();
#else
        restore(im);
        return SYSERR;
#endif
    }

	thrtab_acquire(thrcurrent);

    if (thrptr->hasmsg)
    {
        msg = thrptr->msg;      /* retrieve message              */
        thrptr->hasmsg = FALSE; /* reset message flag            */
    }
    else
    {
        msg = TIMEOUT;
    }

	thrtab_release(thrcurrent);

    restore(im);
    return msg;
}
