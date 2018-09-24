/**
 * @file semaphore.h
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <queue.h>

#if MULTICORE
#include <mutex.h>
#endif

/* Semaphore state definitions */
#define SFREE 0x01 /**< this semaphore is free */
#define SUSED 0x02 /**< this semaphore is used */

/* type definition of "semaphore" */
typedef unsigned int semaphore;

/**
 * Semaphore table entry
 */
struct sement                   /* semaphore table entry      */
{
    char state;                 /**< the state SFREE or SUSED */
    int count;                  /**< count for this semaphore */
    qid_typ queue;              /**< requires queue.h.        */
#if MULTICORE
    mutex_t sem_mutex;		/**< requires mutex.h.	      */
#endif
};

extern struct sement semtab[];

/* isbadsem - check validity of reqested semaphore id and state */
#define isbadsem(s) ((s >= NSEM) || (SFREE == semtab[s].state))

/* Semaphore function prototypes */
syscall wait(semaphore);
syscall signal(semaphore);
syscall signaln(semaphore, int);
semaphore semcreate(int);
syscall semfree(semaphore);
syscall semcount(semaphore);

#endif                          /* _SEMAPHORE_H */
