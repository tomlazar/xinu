/**
 * @file queue.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <thread.h>
#include <queue.h>

struct queent quetab[NQENT];    /**< global thread queue table       */

/**
 * @ingroup threads
 *
 * Insert a thread at the tail of a queue
 * @param  tid  thread ID to enqueue
 * @param  q    target queue
 * @return thread id of enqueued thread
 */
tid_typ enqueue(tid_typ tid, qid_typ q)
{
    int prev, tail;

    if (isbadqid(q) || isbadtid(tid))
    {
        return SYSERR;
    }

	quetab_acquire();

    tail = quetail(q);
    prev = quetab[tail].prev;

    quetab[tid].next = tail;
    quetab[tid].prev = prev;
    quetab[prev].next = tid;
    quetab[tail].prev = tid;

	quetab_release();

    return tid;
}

/**
 * @ingroup threads
 *
 * Remove and return the first thread on a list
 * @param  q  target queue
 * @return thread id of removed thread, or EMPTY
 */
tid_typ dequeue(qid_typ q)
{
    int tid;


    if (isbadqid(q))
    {
        return SYSERR;
    }
    if (isempty(q))
    {
        return EMPTY;
    }

    tid = getfirst(q);

	quetab_acquire();
    if (!isbadtid(tid))
    {
        quetab[tid].prev = EMPTY;
        quetab[tid].next = EMPTY;
    }
	quetab_release();
    return tid;
}

void quetab_acquire()
{
#ifdef _XINU_PLATFORM_ARM_RPI_3_
	mutex_acquire(&quetab_mutex);
#endif
}

void quetab_release()
{
#ifdef _XINU_PLATFORM_ARM_RPI_3_
	mutex_release(&quetab_mutex);
#endif
}
