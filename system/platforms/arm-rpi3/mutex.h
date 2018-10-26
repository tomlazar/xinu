#ifndef _MUTEX_H_
#define _MUTEX_H_

#define MUTEX_LOCKED	1
#define MUTEX_UNLOCKED	0

#ifndef __ASSEMBLER__
typedef unsigned char mutex_t;
extern void mutex_acquire(mutex_t *);
extern void mutex_release(mutex_t *);
#endif	/* __ASSEMBLER__ */

#endif	/* _MUTEX_H_ */
