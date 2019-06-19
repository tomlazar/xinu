/**
 * @file spinlock.h
 * @provides isbadlock
 *
 */
/* Embedded Xinu, Copyright (C) 2008. All rights reserved. */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

/* Spinlock state definitions */
#define SPINLOCK_FREE		0x01
#define SPINLOCK_USED		0x02

/* Spinlock lock state definitions */
#define SPINLOCK_LOCKED		0x01
#define SPINLOCK_UNLOCKED	0x00

/* Number of spinlocks in the system */
#define NLOCK	100

#ifndef __ASSEMBLER__

#include <kernel.h>

/* type definition of a "spinlock" */
typedef unsigned int spinlock_t;

/* spinlock table entry */
struct lockent
{
	unsigned char state;
	unsigned int  lock;	
	int           core;
};

extern struct lockent locktab[];

/* isbadlock - check validity of requested spinlock id and state */
#define isbadlock(x)	((x < 0) || (x > NLOCK) || \
						(SPINLOCK_FREE == locktab[x].state))


/* Spinlock function prototypes */
spinlock_t lock_create(void);
syscall    lock_free(spinlock_t);
syscall    lock_acquire(spinlock_t);
syscall    lock_release(spinlock_t);

/* Spinlocks used in serial driver */
extern spinlock_t serial_lock;

#endif	/* __ASSEMBLER__ */

#endif	/* _SPINLOCK_H_ */
