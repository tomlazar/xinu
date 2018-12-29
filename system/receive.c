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

    im = disable();


    thrptr = &thrtab[thrcurrent];
    if (FALSE == thrptr->hasmsg)
    {                           /* if no message, wait for one */
		thrtab_acquire(thrcurrent);
        
		thrptr->state = THRRECV;

		thrtab_release(thrcurrent);

        resched();
    }

	thrtab_acquire(thrcurrent);

    msg = thrptr->msg;          /* retrieve message                */
    thrptr->hasmsg = FALSE;     /* reset message flag              */

	thrtab_release(thrcurrent);

    restore(im);
    return msg;
}
