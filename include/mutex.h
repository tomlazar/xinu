#ifndef _MUTEX_H_
#define _MUTEX_H_

#define MUTEX_FREE	1
#define MUTEX_USED	2

#define MUTEX_LOCKED	0x01
#define MUTEX_UNLOCKED	0x00

#define isbadmux(x)	( (x < 0) || (x > NMUTEX) )

#ifndef __ASSEMBLER__

#include <kernel.h>

/* 1 for each thread, 1 for each semaphore, plus 50 more for extra */
#define NMUTEX		NTHREAD + NSEM + 50

typedef unsigned int mutex_t;

/* Lock for kprintf to protect multi-core printing. */
extern mutex_t serial_lock;

struct muxent
{
	unsigned char state;
	unsigned int lock;	
	int	     core;
};

extern struct muxent muxtab[];

mutex_t mutex_create(void);
syscall mutex_free(mutex_t);
syscall mutex_acquire(mutex_t);
syscall mutex_release(mutex_t);
#endif	/* __ASSEMBLER__ */

#endif	/* _MUTEX_H_ */
