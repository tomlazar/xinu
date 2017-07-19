/*	2017 Embedded Xinu Team
 *      Patrick J. McGee
 *	core.h
 *
 *	Contains Core Start addresses, semaphores, and CPUID function.
*/
#ifndef _CORE_H_
#define _CORE_H_

#define CORE1_START 0x4000009C
#define CORE2_START 0x400000AC
#define CORE3_START 0x400000BC
#define CORE_MASK   3

uint32_t get_core_number(void);
void start_core1(void (*func)(void));
void start_core2(void (*func)(void));
void start_core3(void (*func)(void));

uint32_t load32(uint32_t address);
void store32(uint32_t address, uint32_t value);


#endif	/* _CORE_H_ */
