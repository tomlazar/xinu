/**
 * @file initialize.c
 * The system begins intializing after the C environment has been
 * established.  After intialization, the null thread remains always in
 * a ready (THRREADY) or running (THRCURR) state.
 */
/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <kernel.h>
#include <backplane.h>
#include <clock.h>
#include <device.h>
#include <gpio.h>
#include <memory.h>
#include <bufpool.h>
#include <mips.h>
#include <thread.h>
#include <tlb.h>
#include <queue.h>
#include <semaphore.h>
#include <monitor.h>
#include <mailbox.h>
#include <network.h>
#include <nvram.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <safemem.h>
#include <platform.h>

#include <rpi_gpio.h>
#include "bcm2837.h"

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

struct platform platform;       /* Platform specific configuration     */

#define GPFSEL1     (*(volatile unsigned *)(GPIO_REGS_BASE + 0x04))
#define GPPUD       (*(volatile unsigned *)(GPIO_REGS_BASE + 0x94))
#define GPPUDCLK0   (*(volatile unsigned *)(GPIO_REGS_BASE + 0x98))
#define GPSET0      (*(volatile unsigned *)(GPIO_REGS_BASE + 0x1C))
#define GPCLR0      (*(volatile unsigned *)(GPIO_REGS_BASE + 0x28))
#define GPLEV0		(*(volatile unsigned *)(GPIO_REGS_BASE + 0x34))

#define PL011_DR    (*(volatile unsigned *)(PL011_BASE + 0x0))  /* Data Register */
#define PL011_FR    (*(volatile unsigned *)(PL011_BASE + 0x18)) /* Flag Register */
#define PL011_IBRD  (*(volatile unsigned *)(PL011_BASE + 0x24)) /* Integer Baud rate
																   divisor */
#define PL011_FBRD  (*(volatile unsigned *)(PL011_BASE + 0x28)) /* Fractional Baud rate
																   divisor */
#define PL011_LCRH  (*(volatile unsigned *)(PL011_BASE + 0x2C)) /* Line Control Register*/
#define PL011_CR    (*(volatile unsigned *)(PL011_BASE + 0x30)) /* Control register */

#define _UART_CLK    48000000
#define _PL011_BAUD_INT(x)   (_UART_CLK / (16 * (x)))
#define _PL011_BAUD_FRAC(x)  (int)((((_UART_CLK / (16.0 * (x)))-_PL011_BAUD_INT(x))*64.0)+0.5)

#define TIMER_BASE	(GPIO_REGS_BASE + 0x03000)
#define TIMER_CS	(*(volatile unsigned *)(TIMER_BASE + 0x0))
#define TIMER_CMP3	(*(volatile unsigned *)(TIMER_BASE + 0x18))

#define IRQ_BASE	(GPIO_REGS_BASE + 0xB200)
#define IRQ_PND_BSC	(*(volatile unsigned *)(IRQ_BASE + 0x0))
#define IRQ_PND_1	(*(volatile unsigned *)(IRQ_BASE + 0x4))
#define IRQ_PND_2	(*(volatile unsigned *)(IRQ_BASE + 0x8))
#define IRQ_ENB_1	(*(volatile unsigned *)(IRQ_BASE + 0x10))
#define IRQ_ENB_2	(*(volatile unsigned *)(IRQ_BASE + 0x14))
#define IRQ_ENB_BSC	(*(volatile unsigned *)(IRQ_BASE + 0x18))
#define IRQ_DIS_1	(*(volatile unsigned *)(IRQ_BASE + 0x1C))
#define IRQ_DIS_2	(*(volatile unsigned *)(IRQ_BASE + 0x20))
#define IRQ_DIS_BAS	(*(volatile unsigned *)(IRQ_BASE + 0x24))

void init_led(void)
{
	GPFSEL1 &= ~(7 << 18); // GPIO Pin 16
	GPFSEL1 |= 1 << 18;    // Set as output
}

void init_button(void)
{
	GPFSEL1 &= ~(7 << 21); // GPIO Pin 17 as input
}

void led_on(void)
{
	GPSET0 = 1 << 16;
}

void led_off(void)
{
	GPCLR0 = 1 << 16;
}

int button_lev(void)
{
	return (GPLEV0 & (1 << 17));
}


/**
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
	uint lev, mode;
	int i;

	init_led();
	init_button();
	
	/* Platform-specific initialization  */
	platforminit();
	
	/* General initialization  */
	sysinit();
	
	kprintf("Hello Xinu W3rld!\r\n");
	print_os_info();

	mode = getmode();
	kprintf("Printing out CPSR:\r\n");

	i = 31;
	while (i >= 0)
	{
		kprintf("%d", (mode >> i) & 1);
		--i;
	}

	/* Enable interrupts */
	enable();

	/* Enable system timer peripheral */
	/* TEMPORARY... this should be done in clkinit, but is used for testing */
	interruptVector[IRQ_TIMER] = 0;
	enable_irq(IRQ_TIMER);
	clkupdate(platform.clkfreq / CLKTICKS_PER_SEC);	
	
	/* Spawn the main thread  */
	//ready(create(main, INITSTK, INITPRIO, "MAIN", 0), RESCHED_YES);

	/* null thread has nothing else to do but cannot exit  */
	while (TRUE)
	{
#ifndef DEBUG
		pause();
#endif                          /* DEBUG */
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
#if 0	
	for (i = 0; i < NSEM; i++)
	{
		semtab[i].state = SFREE;
		semtab[i].queue = queinit();
	}
#endif

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
	//	readylist = queinit();

#if SB_BUS
	backplaneInit(NULL);
#endif                          /* SB_BUS */

#if RTCLOCK
	/* initialize real time clock */
//	clkinit();
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

#ifdef WITH_USB
	usbinit();
#endif

#if NVRAM
	nvramInit();
#endif

#if NNETIF
	netInit();
#endif

#if GPIO
	gpioLEDOn(GPIO_LED_CISCOWHT);
#endif
	return OK;
}
