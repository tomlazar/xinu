/**
 * @file ready.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>
#include <queue.h>

/**
 * @ingroup threads
 *
 * Make a thread eligible for CPU service.
 * @param tid target thread
 * @param resch if RESCHED_YES, reschedules
 * @return OK if thread has been added to readylist, else SYSERR
 */
int ready(tid_typ tid, bool resch)
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

	/* if core affinity is not set,
	 * set affinity to core currently running this code (most likely 0) */
	if (-1 == core_affinity[tid])
	{
		core_affinity[tid] = cpuid;
	}

#if 0
	/* do not put in ready list if calling ready from a different cpu */
	if (cpuid != core_affinity[tid])
	{
		thrtab_release(tid);
		return SYSERR;
	}
#endif
	thrtab_release(tid);

    insert(tid, readylist[core_affinity[tid]], thrptr->prio);

    if (resch == RESCHED_YES)
    {
        resched();
    }

    return OK;
}

#ifdef _XINU_PLATFORM_ARM_RPI_3_
int ready_multi(tid_typ tid, unsigned int core)
{
	register struct thrent *thrptr;

//	kprintf("\r[ready_multi] readying tid %d on core %d\r\n", tid, core);

	udelay(25);

	if (isbadtid(tid))
	{
		return SYSERR;
	}

	thrtab_acquire(tid);

	thrptr = &thrtab[tid];
	thrptr->state = THRREADY;

	if (-1 == core_affinity[tid])
	{
		core_affinity[tid] = core;
	}

	thrtab_release(tid);

	insert(tid, readylist[core_affinity[tid]], thrptr->prio);

	return OK;
}
#endif
