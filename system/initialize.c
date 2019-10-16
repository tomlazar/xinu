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
#include <mutex.h>

#ifdef WITH_USB
#include <usb_subsystem.h>
#include <usb_core_driver.h>
#include "../device/smsc9512/smsc9512.h"
#endif

#include "platforms/arm-rpi3/mmu.h"
#include <dma_buf.h>

/* Function prototypes */
extern void testmain();
extern thread main(void);       /* main is the first thread created    */
static int sysinit(void);       /* intializes system structures        */

/* Declarations of major kernel variables */
struct thrent thrtab[NTHREAD];  /* Thread table                   */
struct sement semtab[NSEM];     /* Semaphore table                */
struct monent montab[NMON];     /* Monitor table                  */
qid_typ readylist[NCORES];      /* List of READY threads          */
struct memblock memlist;        /* List of free memory blocks     */
struct bfpentry bfptab[NPOOL];  /* List of memory buffer pools    */

/* Declarations of major multicore variables */
mutex_t quetab_mutex;
mutex_t thrtab_mutex[NTHREAD];
mutex_t semtab_mutex[NSEM];

mutex_t serial_lock;

static void core_nulluser(void);

/* Active system status */
int thrcount;                   /* Number of live user threads         */
tid_typ thrcurrent[NCORES];     /* Id of currently running thread      */

/* Params set by startup.S */
void *memheap;                  /* Bottom of heap (top of O/S stack)   */
ulong cpuid;                    /* Processor id                        */
struct platform platform;       /* Platform specific configuration     */


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
	/* Platform-specific initialization */		
	platforminit();

	/* General initialization  */
	sysinit();

	unparkcore(1, (void *) core_nulluser, NULL);
	unparkcore(2, (void *) core_nulluser, NULL);
	unparkcore(3, (void *) core_nulluser, NULL);

	kprintf("\r\n***********************************************************\r\n");
	kprintf("******************** Hello Xinu World! ********************\r\n");
	kprintf("***********************************************************\r\n");

	/* Enable interrupts  */
	enable();	

	/* Spawn the main thread  */
	ready(create(main, INITSTK, INITPRIO, "MAIN", 0), RESCHED_YES, CORE_ZERO);

	/* null thread has nothing else to do but cannot exit  */
	while (TRUE)
	{
		
	}
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

	/* Initialize serial lock */
	serial_lock = mutex_create();

	/* Initialize system variables */
	/* Count this NULLTHREAD as the first thread in the system. */
	thrcount = NCORES;		/* 1 nullthread per core */

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
	thrptr->stklen = 8192; 	/* NULLSTK */
	thrptr->stkptr = 0;
	thrptr->memlist.next = NULL;
	thrptr->memlist.length = 0;
	thrptr->core_affinity = CORE_ZERO;
	thrcurrent[CORE_ZERO] = NULLTHREAD;

	/* Core 1 NULLTHREAD */
	thrptr = &thrtab[NULLTHREAD1];
	thrptr->state = THRCURR;
	thrptr->prio = 0;
	strlcpy(thrptr->name, "prnull01", TNMLEN);
	thrptr->stkbase = (void *)(&_end + 8192);
	thrptr->stklen = 8192;
	thrptr->stkptr = 0;
	thrptr->memlist.next = NULL;
	thrptr->memlist.length = 0;
	thrptr->core_affinity = CORE_ONE;
	thrcurrent[CORE_ONE] = NULLTHREAD1;

	/* Core 2 NULLTHREAD */
	thrptr = &thrtab[NULLTHREAD2];
	thrptr->state = THRCURR;
	thrptr->prio = 0;
	strlcpy(thrptr->name, "prnull02", TNMLEN);
	thrptr->stkbase = (void *)(&_end + 16384);
	thrptr->stklen = 8192;
	thrptr->stkptr = 0;
	thrptr->memlist.next = NULL;
	thrptr->memlist.length = 0;
	thrptr->core_affinity = CORE_TWO;
	thrcurrent[CORE_TWO] = NULLTHREAD2;

	/* Core 3 NULLTHREAD */
	thrptr = &thrtab[NULLTHREAD3];
	thrptr->state = THRCURR;
	thrptr->prio = 0;
	strlcpy(thrptr->name, "prnull03", TNMLEN);
	thrptr->stkbase = (void *)(&_end + 24576);
	thrptr->stklen = 8192;
	thrptr->stkptr = 0;
	thrptr->memlist.next = NULL;
	thrptr->memlist.length = 0;
	thrptr->core_affinity = CORE_THREE;
	thrcurrent[CORE_THREE] = NULLTHREAD3;	

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

	/* initialize thread ready lists */
	for (i = 0; i < NCORES; i++)
	{
		readylist[i] = queinit();
	}
	
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
	mailboxInit();
#endif

#if NDEVS
	for (i = 0; i < NDEVS; i++)
	{
		devtab[i].init((device*)&devtab[i]);
	}
#endif


#ifdef WITH_USB
	usbinit();
#endif


#if NVRAM
	nvramInit();
#endif

#if NETHER
	netInit();
#endif

#if GPIO
	gpioLEDOn(GPIO_LED_CISCOWHT);
#endif

	return OK;
}

static void core_nulluser(void)
{
	unsigned int cpuid;
	cpuid = getcpuid();

	disable();
	while (TRUE) 
	{
		pldw(&quetab[readylist[cpuid]].next);
		if (nonempty(readylist[cpuid]))
			resched();
	}
}
