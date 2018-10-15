#include <stddef.h>
#include <thread.h>
#include <core.h>
#include <mmu.h>
#include <clock.h>

extern void CoreSetup(void) __attribute__((naked));
typedef void (*fn)(void);
extern void sev(void);

void printcpsr(void);

/* array for holding the address of the starting point for each core */
void *corestart[4];

/* array for holding the initial stack pointer for each core */
/* these values are set in start.S */
unsigned int core_init_sp[4];

void *init_args[4];

void unparkcore(int num, void *procaddr, void *args) {
	if (num > 0 && num < 4)
	{
		corestart[num] = (void *) procaddr;
		init_args[num] = args;
		sev();	// send event
				// this takes the core out of its sleeping state and allows it to
				// start running code
		*(volatile fn *)(CORE_MBOX_BASE + CORE_MBOX_OFFSET * num) = CoreSetup;
	}
}

void createnullthread(void)
{
	uint cpuid;
	cpuid = getcpuid();


	/* enable interrupts */
//	enable();

	while(TRUE) 
	{
		kprintf("CORE %d IS RUNNING\r\n", cpuid);
		udelay(250);
	}
}

void printcpsr(void){
	uint mode;
	int i;
	mode = getmode();

	kprintf("Printing out CPSR:\r\n");
	// print out bits of cpsr
	for (i = 31; i >= 0; i--)
		kprintf("%d", (mode >> i) & 1);

}
