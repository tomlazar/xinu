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
	unsigned int cpuid;

	cpuid = getcpuid();

	if (isbadtid(tid))
	{
		return SYSERR;
	}

	thrtab_acquire(tid);

	thrptr = &thrtab[tid];
	thrptr->state = THRREADY;
	thrptr->core_affinity = core;

	/* if core affinity is not set,
	 * set affinity to core currently running this code (most likely 0) */
	if (-1 == thrptr->core_affinity)
	{
		thrptr->core_affinity = cpuid;
	}

	thrtab_release(tid);

	insert(tid, readylist[thrptr->core_affinity], thrptr->prio);

	if (resch == RESCHED_YES)
	{
		resched();
	}

	return OK;
}
