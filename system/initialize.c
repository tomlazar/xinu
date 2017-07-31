/**
 * @file initialize.c
 * The system begins intializing after the C environment has been
 * established.  After intialization, the null thread remains always in
 * a ready (THRREADY) or running (THRCURR) state.
 */
/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <xinu.h>
#include <platform.h>
#include <stdint.h>

#ifdef _XINU_PLATFORM_ARM_RPI_3_
#include <bcm2837.h>
#include <rpi_gpio.h>
#include <core.h>
#endif /* _XINU_PLATFORM_ARM_RPI_3_ */

#ifdef WITH_USB
#  include <usb_subsystem.h>
#endif

/* Function prototypes */
extern thread main(void);       /* main is the first thread created    */
static int sysinit(void);       /* intializes system structures        */

/* Declarations of major kernel variables */
struct thrent thrtab[NTHREAD];  /* Thread table                   */
struct sement semtab[NSEM];     /* Semaphore table                */
struct monent montab[NMON];     /* Monitor table                  */
qid_typ readylist;              /* List of READY threads          */
struct memblock memlist;        /* List of free memory blocks     */
struct bfpentry bfptab[NPOOL];  /* List of memory buffer pools    */

/* Active system status */
int thrcount;                   /* Number of live user threads         */
tid_typ thrcurrent;             /* Id of currently running thread      */

/* Params set by startup.S */
void *memheap;                  /* Bottom of heap (top of O/S stack)   */
ulong cpuid;                    /* Processor id                        */

/* Raspberry Pi 3 array for initial stack pointer addresses for each core */
#ifdef _XINU_PLATFORM_ARM_RPI_3_
unsigned int core_init_sp[4];
int token = 0;
#endif

struct platform platform;       /* Platform specific configuration     */

void init_led(void)
{
	volatile struct rpi_gpio_regs *regptr = (volatile struct rpi_gpio *)(GPIO_REGS_BASE);
	regptr->gpfsel[1] &= ~(7 << 18);
	regptr->gpfsel[1] |=  (1 << 18);
}

void led_on(void)
{
	volatile struct rpi_gpio_regs *regptr = (volatile struct rpi_gpio *)(GPIO_REGS_BASE);
	regptr->gpset[0] = 1 << 16;
}

void led_off(void)
{
	volatile struct rpi_gpio_regs *regptr = (volatile struct rpi_gpio *)(GPIO_REGS_BASE);
	regptr->gpclr[0] = 1 << 16;
}

/* For MMU configuration */
/* dwelch MMU code */
#define MMUTABLEBASE	0x00004000

extern void PUT32(unsigned int, unsigned int);
extern unsigned int GET32(unsigned int);

extern void start_mmu(unsigned int, unsigned int);
extern void stop_mmu(void);
extern void invalidate_tlbs(void);

extern unsigned int exclusive_text(unsigned int);

unsigned int mmu_section(unsigned int vadd, unsigned int padd, unsigned int flags)
{
	unsigned int ra, rb, rc;

	ra = vadd >> 20;
	rb = MMUTABLEBASE | (ra << 2);
	rc = (padd & 0xFFF00000) | 0xC00 | flags | 2;
	PUT32(rb, rc);
	
	return 0;
}

extern unsigned serial_lock;

/*
 * Intializes the system and becomes the null thread.
 * This is where the system begins after the C environment has been
 * established.  Interrupts are initially DISABLED, and must eventually
 * be enabled explicitly.  This routine turns itself into the null thread
 * after initialization.  Because the null thread must always remain ready
 * to run, it cannot execute code that might cause it to be suspended, wait
 * for a semaphore, or put to sleep, or exit.  In particular, it must not
 * do I/O unless it uses kprintf for synchronous output.
 */
void nulluser(void)
{
	uint mode, cpuid, ra;
	int i, tret;

	init_led();

	/* dwelch mmu code */
	/* setting up virtual addresses == to physical addresses */
	for (ra = 0; ; ra += 0x00100000)
	{
		mmu_section(ra, ra, 0x0000 | 8 | 4);
		//mmu_section(ra, ra, 0x0);
		if (ra == 0x3EF00000) 
			break;	/* stop before IO peripherals, dont want cache on those... */
	}

	// peripherals
	for ( ; ; ra += 0x00100000)
	{
		mmu_section(ra, ra, 0x0000);
		if (ra == 0xFFF00000) break;
	}

	start_mmu(MMUTABLEBASE, 0x1 | 0x1000 | 0x4);

	
	/* Platform-specific initialization (system/platforms/arm-rpi3/platforminit.c) */
	platforminit();

	/* General initialization  */
	sysinit();
	
    kprintf("\r\n***********************************************************\r\n");
	kprintf("******************** Hello Xinu World! ********************\r\n");
	kprintf("***********************************************************\r\n");
	/* Print memory usage (located in system/main.c) */
	print_os_info();

	/* ldrex/strex test */
	i = 0;
	
//	kprintf("starting ldrex/strex test, i = %d\r\n", i);
//	tret = exclusive_test(&i);
//	kprintf("after test, i = %d, tret = %d\r\n", i, tret);

//	kprintf("serial_lock = %d, &serial_lock = 0x%08X\r\n", serial_lock, &serial_lock);

	// cpsr
	mode = getmode();
	
	kprintf("\r\nPrinting out CPSR:\r\n");
	// print out bits of cpsr
	for (i = 31; i >= 0; i--)
		kprintf("%d", (mode >> i) & 1);

	cpuid = getcpuid();
	kprintf("\r\nPrinting out MPIDR:\r\n");
	for (i = 31; i >= 0; i--)
		kprintf("%d", (cpuid >> i) & 1);

	kprintf("\r\nPrinting out core_init_sp array:\r\n");
	for (i = 0; i < 4; i++)
		kprintf("%d: 0x%08X\r\n", i, core_init_sp[i]);

	kprintf("\r\n");

	/*  Test all cores (located in test/test_semaphore_core.c) */
	testallcores();

	/* Call to test method (located in test/test_processcreation.c) */
	//testmain();

	/* Enable interrupts  */
	enable();

	/* Spawn the main thread  */
	//ready(create(main, INITSTK, INITPRIO, "MAIN", 0), RESCHED_YES);

	/* null thread has nothing else to do but cannot exit  */
	token = 4;
	while (TRUE){}
}

/**
 * Intializes all Xinu data structures and devices.
 * @return OK if everything is initialized successfully
 */
static int sysinit(void)
{
	int i;
	struct thrent *thrptr;      /* thread control block pointer  */
	struct memblock *pmblock;   /* memory block pointer          */

	/* Initialize system variables */
	/* Count this NULLTHREAD as the first thread in the system. */
	thrcount = 1;

	/* Initialize free memory list */
	memheap = roundmb(memheap);
	platform.maxaddr = truncmb(platform.maxaddr);
	memlist.next = pmblock = (struct memblock *)memheap;
	memlist.length = (uint)(platform.maxaddr - memheap);
	pmblock->next = NULL;
	pmblock->length = (uint)(platform.maxaddr - memheap);

	/* Initialize thread table */
	for (i = 0; i < NTHREAD; i++)
	{
		thrtab[i].state = THRFREE;
	}

	/* initialize null thread entry */
	thrptr = &thrtab[NULLTHREAD];
	thrptr->state = THRCURR;
	thrptr->prio = 0;
	strlcpy(thrptr->name, "prnull", TNMLEN);
	thrptr->stkbase = (void *)&_end;
	thrptr->stklen = (ulong)memheap - (ulong)&_end;
	thrptr->stkptr = 0;
	thrptr->memlist.next = NULL;
	thrptr->memlist.length = 0;
	thrcurrent = NULLTHREAD;

	/* Initialize semaphores */
	for (i = 0; i < NSEM; i++)
	{
		semtab[i].state = SFREE;
		semtab[i].queue = queinit();
	}

	/* Initialize monitors */
	for (i = 0; i < NMON; i++)
	{
		montab[i].state = MFREE;
	}

	/* Initialize buffer pools */
	for (i = 0; i < NPOOL; i++)
	{
		bfptab[i].state = BFPFREE;
	}

	/* initialize thread ready list */
	readylist = queinit();

#if SB_BUS
	backplaneInit(NULL);
#endif                          /* SB_BUS */

#if RTCLOCK
	/* initialize real time clock */
	clkinit();
#endif                          /* RTCLOCK */

#ifdef UHEAP_SIZE
	/* Initialize user memory manager */
	{
		void *userheap;             /* pointer to user memory heap   */
		userheap = stkget(UHEAP_SIZE);
		if (SYSERR != (int)userheap)
		{
			userheap = (void *)((uint)userheap - UHEAP_SIZE + sizeof(int));
			memRegionInit(userheap, UHEAP_SIZE);

			/* initialize memory protection */
			safeInit();

			/* initialize kernel page mappings */
			safeKmapInit();
		}
	}
#endif

#if USE_TLB
	/* initialize TLB */
	tlbInit();
	/* register system call handler */
	exceptionVector[EXC_SYS] = syscall_entry;
#endif                          /* USE_TLB */

#if NMAILBOX
	/* intialize mailboxes */
	//	mailboxInit();
#endif

#if NDEVS
	for (i = 0; i < NDEVS; i++)
	{
		devtab[i].init((device*)&devtab[i]);
	}
#endif

#if GPIO
	gpioLEDOn(GPIO_LED_CISCOWHT);
#endif
	return OK;
}
