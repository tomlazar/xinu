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
#include "../device/lan7800/lan7800.h"
#endif

#include "platforms/arm-rpi3/mmu.h"

/* Function prototypes */
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

	/* XXX Test for determining cache structure */
	uint encoding = _getcacheinfo();
	kprintf("\r\nCCSIDR:\t\t\t\t%032b\r\n", encoding);
	// CCSIDR: 01110000000011111110000000011010
	// According to Cortex A53 doc:
	// [31] Write through		0
	// [30] Write back		1
	// [29] Read allocation		1
	// [28] Write allocation	1
	// [27:13] NumSets		127 == 0x7F
	// [12:3] Associativity		3 (4-way set assoc. cache) (see sec 2.1.6 of a53 doc)
	// 				Meaning, each set contains 4 cache lines.
	// [2:0] Words per line         16 words per cache line (according to arm cortex a53)
	// 				16 words * 32 bits each = 512 bits per cache line
	// 				Cache representation: From 0 to 126 sets,
	// 				Each set contains 4 * 512 bits of data cache = 2048 bits

	encoding = _getcachemaint();
	kprintf("\r\nCache maintenance register:\t%032b\r\n\n", encoding);
	//00000010000100000010001000010001
	// [27:24] Cached memory size		0010 == 0x2
	// [7:4] Cache maintenance by set/way: 	0001
	// [0:3] Cache maintenance by MVA:	0001

	/* Enable interrupts  */
	enable();	

	/* Spawn the main thread  */
	ready(create(main, INITSTK, INITPRIO, "MAIN", 0), RESCHED_YES, CORE_ZERO);

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

	/* Initialize serial lock */
	serial_lock = mutex_create();
	kprintf("\r\nSERIAL_LOCK: %d\r\n", serial_lock);

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

void dump_cache_tags(void){

	uint start = 0x40;
	uint end = 0x400;	// inc by 40 to 400
	uint tag0 = 0;
	uint tag1 = 0;

	for(uint i=start; i<=end; i+= 0x40){

		tag0 = _dump_dr0(i);
		tag1 = _dump_dr1(i);
	
		if(i == 0x40)
			kprintf("DR0\t\t\t\tDR1\t\t\t\t\r\n---\t\t\t\t---\t\t\t\t\r\n");
		
		kprintf("%d. %032b\t\t\t\t%032b\r\n\n", tag0, tag1);
	}

}
