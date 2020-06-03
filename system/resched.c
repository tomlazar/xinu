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

	thrtab_acquire(thrcurrent[cpuid]);
	throld = &thrtab[thrcurrent[cpuid]];
	throld->intmask = disable();

	if (THRCURR == throld->state)
	{
		quetab_acquire();
		if (nonempty(readylist[cpuid]) && (throld->prio > firstkey(readylist[cpuid])))
		{
			quetab_release();
			thrtab_release(thrcurrent[cpuid]);
			restore(throld->intmask);
			return OK;
		}
		quetab_release();
		throld->state = THRREADY;
		insert(thrcurrent[cpuid], readylist[cpuid], throld->prio);
	}

	thrtab_release(thrcurrent[cpuid]);

	/* get highest priority thread from ready list */
	thrcurrent[cpuid] = dequeue(readylist[cpuid]);
	thrtab_acquire(thrcurrent[cpuid]);
	thrnew = &thrtab[thrcurrent[cpuid]];
	thrnew->state = THRCURR;
	thrtab_release(thrcurrent[cpuid]);

	/* change address space identifier to thread id */
	asid = thrcurrent[cpuid] & 0xff;
	ctxsw(&throld->stkptr, &thrnew->stkptr, asid);

	/* old thread returns here when resumed */
	restore(throld->intmask);
	return OK;
}
