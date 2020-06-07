/**
 * @file resched.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>
#include <clock.h>
#include <queue.h>
#include <memory.h>

extern void ctxsw(void *, void *, uchar);
int resdefer;                   /* >0 if rescheduling deferred */

/**
 * @ingroup threads
 *
 * Reschedule processor to highest priority ready thread.
 * Upon entry, thrcurrent gives current thread id.
 * Threadtab[thrcurrent].pstate gives correct NEXT state
 * for current thread if other than THRREADY.
 * @return OK when the thread is context switched back
 */
int resched(void)
{
	uchar asid;                 /* address space identifier */
	struct thrent *throld;      /* old thread entry */
	struct thrent *thrnew;      /* new thread entry */
	unsigned int cpuid;

	if (resdefer > 0)
	{                           /* if deferred, increase count & return */
		resdefer++;
		return (OK);
	}

	cpuid = getcpuid();

	throld = &thrtab[thrcurrent[cpuid]];
	throld->intmask = disable();

	if (THRCURR == throld->state)
	{
		if (nonempty(readylist[cpuid]) && (throld->prio > firstkey(readylist[cpuid])))
		{
			restore(throld->intmask);
			return OK;
		}
		throld->state = THRREADY;
		insert(thrcurrent[cpuid], readylist[cpuid], throld->prio);
	}

	/* get highest priority thread from ready list */
	thrcurrent[cpuid] = dequeue(readylist[cpuid]);
	thrnew = &thrtab[thrcurrent[cpuid]];
	thrnew->state = THRCURR;

	/* change address space identifier to thread id */
	asid = thrcurrent[cpuid] & 0xff;
	ctxsw(&throld->stkptr, &thrnew->stkptr, asid);

	/* old thread returns here when resumed */
	restore(throld->intmask);
	return OK;
}
