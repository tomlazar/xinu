#include <stddef.h>
#include <thread.h>
#include <core.h>

extern int getmode(void);
extern void setmode(void);
extern void unparkcore(int, void *);
extern void Core1Setup (void) __attribute__((naked));
extern void Core2Setup (void) __attribute__((naked));
extern void Core3Setup (void) __attribute__((naked));
typedef void (*fn)(void);
void idle_thread1(void);
void idle_thread2(void);
void idle_thread3(void);

void *corestart;
int numcore;

void printcpsr(void);

extern void start_mmu(unsigned int, unsigned int);
extern void stop_mmu(void);
extern void invalidate_tlbs(void);
extern unsigned int mmu_section(unsigned int, unsigned int, unsigned int);

#define MMUTABLEBASE	0x00004000

void unparkcore(int num, void *procaddr) {
	corestart = procaddr;
//	kprintf("Proc addr passed into unparkcore is 0x%08X\r\n", procaddr);
	numcore = num;
	if (num == 1)
		*(volatile fn *)(CORE_MBOX_BASE + CORE_MBOX_OFFSET * num) = Core1Setup;
	if (num == 2)
	    	*(volatile fn *)(CORE_MBOX_BASE + CORE_MBOX_OFFSET * num) = Core2Setup;
        if (num == 3)
	    	*(volatile fn *)(CORE_MBOX_BASE + CORE_MBOX_OFFSET * num) = Core3Setup;
}


void createnullthread(void)
{
	uint ra;
	
	/* dwelch mmu code for initializing mmu */
	/* same thing as done on core 0 in nulluser */
	/* have to do on all cores because each core has separate MMU */
	/*	
	for (ra = 0; ; ra += 0x00100000)
	{
		mmu_section(ra, ra, 0x0 | 8 | 4);
		if (ra == 0x3EF0000)
			break;	
	}
	for ( ; ; ra += 0x00100000)
	{
		mmu_section(ra, ra, 0x0);
		if (ra == 0xFFF00000)
			break;
	}
	*/
	start_mmu(MMUTABLEBASE, 0x1 | 0x1000 | 0x4);

	kprintf("Beginning of createnullthread\r\n");
	kprintf("Corestart is now 0x%08X\r\n", corestart);
	printcpsr();
	ready(create((void *)idle_thread1, INITSTK, 5, "null thread", 0, NULL), 1);
	ready(create((void *)idle_thread2, INITSTK, 5, "null thread", 0, NULL), 1);
	ready(create((void *)idle_thread3, INITSTK, 5, "null thread", 0, NULL), 1);
	while(1)
		;
}

void idle_thread1(void){
//	while(1)
	kprintf("fffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
}

void idle_thread2(void){
//	while(1)
	kprintf("ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo");
}

void idle_thread3(void){
//	while(1)
	kprintf("000000000000000000000000000000000000000000000000000000000");
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
