/*	2017 Embedded Xinu Team
 *	core.h
 *
 *	Contains Core Start addresses, semaphores, and CPUID function.
*/
#ifndef _CORE_H_
#define _CORE_H_

#ifdef _XINU_PLATFORM_ARM_RPI_3_
#define CORE_MBOX_BASE      0x4000008C
#define CORE_MBOX_OFFSET    0x10

/* Numeric core definitions for readability */
#define CORE_ZERO  0
#define CORE_ONE   1
#define CORE_TWO   2
#define CORE_THREE 3

extern unsigned int getmode(void);
extern unsigned int getcpuid(void);
extern unsigned int core_init_sp[];
extern void unparkcore(int, void *, void *);
extern void pld(void *);
extern void pldw(void *);
#endif	/* _XINU_PLATFORM_ARM_RPI_3_ */
#endif	/* _CORE_H_ */
