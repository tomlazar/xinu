/**
 * @file send.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>

/**
 * @ingroup threads
 *
 * Send a message to another thread
 * @param tid thread id of recipient
 * @param msg contents of message
 * @return OK on success, SYSERR on failure
 */
syscall send(tid_typ tid, message msg)
{
    register struct thrent *thrptr;
    irqmask im;
    uint cpuid;
    cpuid = getcpuid();

    im = disable();
    if (isbadtid(tid))
    {
        restore(im);
        return SYSERR;
    }
    thrptr = &thrtab[tid];
    if ((THRFREE == thrptr->state) || thrptr->hasmsg)
    {
        restore(im);
        return SYSERR;
    }

	thrtab_acquire(tid);

    thrptr->msg = msg;          /* deposit message                */
    thrptr->hasmsg = TRUE;      /* raise message flag             */

	thrtab_release(tid);

    /* if receiver waits, start it */
    if (THRRECV == thrptr->state)
    {
        ready(tid, RESCHED_YES, cpuid);
    }
    else if (THRTMOUT == thrptr->state)
    {
        unsleep(tid);
        ready(tid, RESCHED_YES, cpuid);
    }
    restore(im);
    return OK;
}
