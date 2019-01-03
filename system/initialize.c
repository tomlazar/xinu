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

#ifdef WITH_USB
#include <usb_subsystem.h>
#include <usb_core_driver.h>
#include "../device/lan7800/lan7800.h"
#endif

/* Function prototypes */
extern thread main(void);       /* main is the first thread created    */
static int sysinit(void);       /* intializes system structures        */

/* Declarations of major kernel variables */
struct thrent thrtab[NTHREAD];  /* Thread table                   */
struct sement semtab[NSEM];     /* Semaphore table                */
struct monent montab[NMON];     /* Monitor table                  */
#ifndef _XINU_PLATFORM_ARM_RPI_3_
qid_typ readylist;              /* List of READY threads          */
#endif
struct memblock memlist;        /* List of free memory blocks     */
struct bfpentry bfptab[NPOOL];  /* List of memory buffer pools    */

/* Declarations of major multicore variables */
#ifdef _XINU_PLATFORM_ARM_RPI_3_
mutex_t quetab_mutex;
mutex_t thrtab_mutex[NTHREAD];
mutex_t semtab_mutex[NSEM];
tid_typ thrcurrent_[4];
qid_typ readylist_[4];

static void core_nulluser(void);
extern void unparkcore(unsigned int, void *, void *);
#endif

/* Active system status */
int thrcount;                   /* Number of live user threads         */
#ifndef _XINU_PLATFORM_ARM_RPI_3_
tid_typ thrcurrent;             /* Id of currently running thread      */
#endif

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

#ifdef _XINU_PLATFORM_ARM_RPI_3_
	unparkcore(1, (void *) core_nulluser, NULL);
	unparkcore(2, (void *) core_nulluser, NULL);
	unparkcore(3, (void *) core_nulluser, NULL);
#endif

	kprintf("\r\n***********************************************************\r\n");
	kprintf("******************** Hello Xinu World! ********************\r\n");
	kprintf("***********************************************************\r\n");

	/* Enable interrupts  */
	enable();	

	/* Spawn the main thread  */
	ready(create(main, INITSTK, INITPRIO, "MAIN", 0), RESCHED_YES);

	/* null thread has nothing else to do but cannot exit  */
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
#ifdef _XINU_PLATFORM_ARM_RPI_3_
	thrcount = 4;
#endif

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
#ifdef _XINU_PLATFORM_ARM_RPI_3_
	thrptr->stklen /= 4; 	/* there are 4 stacks for pi 3 */
#endif
	thrptr->stkptr = 0;
	thrptr->memlist.next = NULL;
	thrptr->memlist.length = 0;
	thrcurrent = NULLTHREAD;

#ifdef _XINU_PLATFORM_ARM_RPI_3_
#if 1
	ulong stkoffset = ((ulong)memheap - (ulong)&_end) / 4;
	/* Core 1 NULLTHREAD */
	thrptr = &thrtab[NULLTHREAD1];
	thrptr->state = THRCURR;
	thrptr->prio = 0;
	strlcpy(thrptr->name, "prnull01", TNMLEN);
	thrptr->stkbase = (void *)(&_end + stkoffset);
	thrptr->stklen = stkoffset;
	thrptr->stkptr = 0;
	thrptr->memlist.next = NULL;
	thrptr->memlist.length = 0;
	thrcurrent_[1] = NULLTHREAD1;

	/* Core 2 NULLTHREAD */
	thrptr = &thrtab[NULLTHREAD2];
	thrptr->state = THRCURR;
	thrptr->prio = 0;
	strlcpy(thrptr->name, "prnull02", TNMLEN);
	thrptr->stkbase = (void *)(&_end + stkoffset + stkoffset);
	thrptr->stklen = stkoffset;
	thrptr->stkptr = 0;
	thrptr->memlist.next = NULL;
	thrptr->memlist.length = 0;
	thrcurrent_[2] = NULLTHREAD2;

	/* Core 3 NULLTHREAD */
	thrptr = &thrtab[NULLTHREAD3];
	thrptr->state = THRCURR;
	thrptr->prio = 0;
	strlcpy(thrptr->name, "prnull03", TNMLEN);
	thrptr->stkbase = (void *)(&_end + stkoffset + stkoffset + stkoffset);
	thrptr->stklen = stkoffset;
	thrptr->stkptr = 0;
	thrptr->memlist.next = NULL;
	thrptr->memlist.length = 0;
	thrcurrent_[3] = NULLTHREAD3;	
#endif
#endif

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
#ifdef _XINU_PLATFORM_ARM_RPI_3_
	readylist_[0] = queinit();
	readylist_[1] = queinit();
	readylist_[2] = queinit();
	readylist_[3] = queinit();
#else
	readylist = queinit();
#endif

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
	disable();
	while (TRUE) 
	{
		if (nonempty(readylist))
			resched();
	}
}
