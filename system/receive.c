/**
 * @file receive.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>

/**
 * @ingroup threads
 *
 * receive - wait for a message and return it
 * @return message
 */
message receive(void)
{
    register struct thrent *thrptr;
    irqmask im;
    message msg;
	unsigned int cpuid;

    im = disable();

	cpuid = getcpuid();

    thrptr = &thrtab[thrcurrent[cpuid]];
    if (FALSE == thrptr->hasmsg)
    {                           /* if no message, wait for one */
		thrtab_acquire(thrcurrent[cpuid]);
        
		thrptr->state = THRRECV;

		thrtab_release(thrcurrent[cpuid]);

        resched();
    }

	thrtab_acquire(thrcurrent[cpuid]);

    msg = thrptr->msg;          /* retrieve message                */
    thrptr->hasmsg = FALSE;     /* reset message flag              */

	thrtab_release(thrcurrent[cpuid]);

    restore(im);
    return msg;
}
