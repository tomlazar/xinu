/**
 * @file ready.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>
#include <queue.h>
#include <clock.h>

/**
 * @ingroup threads
 *
 * Make a thread eligible for CPU service.
 * @param tid target thread
 * @param resch if RESCHED_YES, reschedules
 * @return OK if thread has been added to readylist, else SYSERR
 */
int ready(tid_typ tid, bool resch, uint core)
{
	register struct thrent *thrptr;

	if (isbadtid(tid)){
		return SYSERR;
	}

	thrtab_acquire(tid);

	thrptr = &thrtab[tid];
	thrptr->state = THRREADY;

	/* if core affinity is not set,
	 * set affinity to core currently running this code (most likely 0) */
	unsigned int cpuid;
	cpuid = getcpuid();
	if (-1 == thrptr->core_affinity)
	{
		thrptr->core_affinity = core;
	}

	thrtab_release(tid);

	if (SYSERR == insert(tid, readylist[thrptr->core_affinity], thrptr->prio)){
		return SYSERR;
	}

	if ((resch == RESCHED_YES) && (thrptr->core_affinity == cpuid)){
		resched();
	}

	return OK;
}
