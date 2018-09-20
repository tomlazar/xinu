/**
 * @file signal.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>

#if MULTICORE
#include <mutex.h>
#endif

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
    irqmask im;

    im = disable();
    if (isbadsem(sem))
    {
        restore(im);
        return SYSERR;
    }
    semptr = &semtab[sem];

#if MULTICORE
    mutex_acquire(&(semptr->mutex));
#endif
    
    if ((semptr->count++) < 0)
    {
        ready(dequeue(semptr->queue), RESCHED_NO);
#if MULTICORE
	mutex_release(&(semptr->mutex));
#endif
	resched();
    }
#if MULTICORE
    mutex_release(&(semptr->mutex));
#endif

    restore(im);
    return OK;
}
