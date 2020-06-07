/**
 * @file kill.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>
#include <queue.h>
#include <memory.h>
#include <safemem.h>
#include <core.h>

extern void xdone(void);

/**
 * @ingroup threads
 *
 * Kill a thread and remove it from the system
 * @param tid target thread
 * @return OK on success, SYSERR otherwise
 */
syscall kill(tid_typ tid)
{
	register struct thrent *thrptr;     /* thread control block */
	irqmask im;
	unsigned int cpuid;

	cpuid = getcpuid();

	im = disable();
	if (isbadtid(tid) || (NULLTHREAD == tid) || (NULLTHREAD1 == tid) ||
			(NULLTHREAD2 == tid) || (NULLTHREAD3 == tid))
	{
		restore(im);
		return SYSERR;
	}

	thrptr = &thrtab[tid];

	/* cannot kill process that is running on a different core */
	if (thrptr->core_affinity != cpuid)
	{
		restore(im);
		return SYSERR;
	}

	if (--thrcount <= 1)
	{
		xdone();
	}

#ifdef UHEAP_SIZE
	/* reclaim used memory regions */
	memRegionReclaim(tid);
#endif                          /* UHEAP_SIZE */

	send(thrptr->parent, tid);

	stkfree(thrptr->stkbase, thrptr->stklen);

	thrtab->core_affinity = cpuid;

	switch (thrptr->state)
	{
		case THRSLEEP:
			unsleep(tid);
			thrptr->state = THRFREE;
			break;
		case THRCURR:
			thrptr->state = THRFREE;        /* suicide */
			resched();

		case THRWAIT:
			semtab[thrptr->sem].count++;

		case THRREADY:
			getitem(tid);           /* removes from queue */

		default:
			thrptr->state = THRFREE;
	}

	restore(im);
	return OK;
}
