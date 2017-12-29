#include <stddef.h>
#include <thread.h>
#include <core.h>
#include <mmu.h>

extern void CoreSetup(void) __attribute__((naked));
typedef void (*fn)(void);

void printcpsr(void);

/* array for holding the address of the starting point for each core */
void *corestart[4];

void unparkcore(int num, void *procaddr) {
	/* parameter checking */
	if (num > 0 && num < 4)
	{
		corestart[num] = (void *) procaddr;
		*(volatile fn *)(CORE_MBOX_BASE + CORE_MBOX_OFFSET * num) = CoreSetup;
	}
}

void createnullthread(void)
{
	uint cpuid;
	uint ra;
	uint i;

	start_mmu(MMUTABLEBASE);

	cpuid = getcpuid();

//	kprintf("Core %d: Beginning of createnullthread\r\n", cpuid);
//	kprintf("Core %d: Corestart is now 0x%08X\r\n", cpuid, corestart);
//	ready(create((void *)idle_thread1, INITSTK, 5, "null thread", 0, NULL), 1);
//	ready(create((void *)idle_thread2, INITSTK, 5, "null thread", 0, NULL), 1);
//	ready(create((void *)idle_thread3, INITSTK, 5, "null thread", 0, NULL), 1);

	// only prints 25 times for the sake of readability
	i = 0;
	while(i < 25)
	{
		udelay(2 * cpuid);	// delay for reducing number of print statements
		kprintf("This is Core %d\r\n", cpuid);
		i++;
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
