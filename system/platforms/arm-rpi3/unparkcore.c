/**
 * @file unparkcore.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009. All rights reserved */

#include <stddef.h>
#include <thread.h>
#include <core.h>
#include <mmu.h>
#include <clock.h>

extern void CoreSetup(void) __attribute__((naked));
typedef void (*fn)(void);
extern void sev(void);

/* array for holding the address of the starting point for each core */
void *corestart[4];

/* array for holding the initial stack pointer for each core */
/* these values are set in start.S */
unsigned int core_init_sp[4];

void *init_args[4];

/**
 * @ingroup bcm2837
 *
 * Send an event to a processor core, "unparking" it from the waiting state it starts into. This is done by loading a function into its mailbox. Note: this operation need only be completed once upon initialization, then it can context switch. See initialize.c for usage.
 *
 * @param num		Number of core to unpark
 * @param procaddr	Address of the thread to begin
 * @param args		Arguments to pass
 */
void unparkcore(int num, void *procaddr, void *args) {
	udelay(5);
	if (num > 0 && num < 4)
	{
		corestart[num] = (void *) procaddr;
		init_args[num] = args;
		sev();	// send event
		*(volatile fn *)(CORE_MBOX_BASE + CORE_MBOX_OFFSET * num) = CoreSetup;
	}
}

/**
 * @ingroup bcm2837
 *
 * Create a null thread (for testing purposes -> should be taken out)
 */
void createnullthread(void)
{
	uint cpuid;
	cpuid = getcpuid();


	/* enable interrupts */
//XXX	enable();

	while(TRUE) 
	{
		kprintf("CORE %d IS RUNNING\r\n", cpuid);
		udelay(250);
	}
}
