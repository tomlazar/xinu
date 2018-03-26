/*	2017 Embedded Xinu Team
 *	core.h
 *
 *	Contains Core Start addresses, semaphores, and CPUID function.
*/
#ifndef _CORE_H_
#define _CORE_H_

#define CORE_MBOX_BASE      0x4000008C
#define CORE_MBOX_OFFSET    0x10

extern unsigned int core_init_sp[];

#endif	/* _CORE_H_ */
