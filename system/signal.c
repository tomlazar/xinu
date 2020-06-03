/**
 * @file signal.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>

/**
 * @ingroup semaphores
 *
 * Signal a semaphore, releasing up to one waiting thread.
 *
 * signal() may reschedule the currently running thread.  As a result, signal()
 * should not be called from non-reentrant interrupt handlers unless ::resdefer
 * is set to a positive value at the start of the interrupt handler.
 *
 * @param sem
 *      Semaphore to signal.
 *
 * @return
 *      ::OK on success, ::SYSERR on failure.  This function can only fail if @p
 *      sem did not specify a valid semaphore.
 */
syscall signal(semaphore sem)
{
    register struct sement *semptr;
	int count;
    irqmask im;

    im = disable();
    if (isbadsem(sem))
    {
        restore(im);
        return SYSERR;
    }

	semtab_acquire(sem);
    semptr = &semtab[sem];
	count = semptr->count++;
	semtab_release(sem);

    if (count < 0)
    {
        ready(dequeue(semptr->queue), RESCHED_NO);
		resched();
    }

    restore(im);
    return OK;
}
