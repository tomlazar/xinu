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
	unsigned int cpuid;

	cpuid = getcpuid();

    if (maxwait < 0)
    {
        return SYSERR;
    }
    im = disable();
    thrptr = &thrtab[thrcurrent[cpuid]];
    if (FALSE == thrptr->hasmsg)
    {
#if RTCLOCK
        if (SYSERR == insertd(thrcurrent[cpuid], sleepq, maxwait))
        {
            restore(im);
            return SYSERR;
        }

        thrtab[thrcurrent[cpuid]].state = THRTMOUT;

        resched();
#else
        restore(im);
        return SYSERR;
#endif
    }

    if (thrptr->hasmsg)
    {
        msg = thrptr->msg;      /* retrieve message              */
        thrptr->hasmsg = FALSE; /* reset message flag            */
    }
    else
    {
        msg = TIMEOUT;
    }

    restore(im);
    return msg;
}
