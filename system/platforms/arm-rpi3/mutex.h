#ifndef _MUTEX_H_
#define _MUTEX_H_

#define MUTEX_LOCKED	1
#define MUTEX_UNLOCKED	0

#define MUTEX_FREE		0x01
#define MUTEX_USED		0x02

#define isbadmux(x)		( (x < 0) || (x > NMUTEX) )

#ifndef __ASSEMBLER__

#include <kernel.h>

#ifndef NMUTEX
/* 1 for each thread, 1 for each semaphore, plus 50 more for extra */
#define NMUTEX	NTHREAD + NSEM + 50
#endif	/* NMUTEX */

typedef unsigned int mutex_t;

struct muxent
{
	unsigned char state;
	unsigned int lock;	
};

extern struct muxent muxtab[];

mutex_t mutex_create(void);
syscall mutex_free(mutex_t);
syscall mutex_acquire(mutex_t);
syscall mutex_release(mutex_t);
#endif	/* __ASSEMBLER__ */

#endif	/* _MUTEX_H_ */
