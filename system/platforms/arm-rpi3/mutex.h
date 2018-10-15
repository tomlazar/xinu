#ifndef _MUTEX_H_
#define _MUTEX_H_

#define LOCKED		1
#define UNLOCKED	0

#ifndef __ASSEMBLER__
typedef unsigned int mutex_t;
extern void mutex_acquire(mutex_t *);
extern void mutex_release(mutex_t *);
#endif	/* __ASSEMBLER__ */

#endif	/* _MUTEX_H_ */
