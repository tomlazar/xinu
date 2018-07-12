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
#include "platforms/arm-rpi3/bcm2837_mbox.h"
#include <ether.h>
#include <stdlib.h>
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
extern void bzero(void *, size_t);

volatile uint32_t mailbuffer[MBOX_BUFLEN];

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

	kprintf("\r\n***********************************************************\r\n");
	kprintf("******************** Hello Xinu World! ********************\r\n");
	kprintf("***********************************************************\r\n");

	/* Store the MAC Address into the ether struct's devAddress member,
	 * obtained using the BCM2837B0's mailbox. */
	//struct ether *ethptr = (struct ether *)malloc(sizeof(struct ether));
	struct ether *ethptr;
	ethptr = &ethertab[0];

	uint8_t macaddr[6] = {0};		// Temporary storage of MAC address
	init_mailbuffer(mailbuffer);

	//print_parameter(mailbuffer, "MAC address", MBX_TAG_GET_MAC_ADDRESS, 2);

	#ifdef NETHER
	/* Fill the mailbox buffer with the MAC address */
        get_mac_mailbox(mailbuffer);

	int i;
	/* Print the buffer */
        for (i = 0; i < 2; ++i) {
		uint32_t value = mailbuffer[MBOX_HEADER_LENGTH + TAG_HEADER_LENGTH + i];

		/* Store the low MAC address bits into a temporary array */
		if(i == 0){
		        macaddr[0] = (value >> 0)  & 0xff;
		        macaddr[1] = (value >> 8)  & 0xff;
		        macaddr[2] = (value >> 16) & 0xff;
		        macaddr[3] = (value >> 24) & 0xff;
		}

		/* Store the high MAC address bits into a temporary array */
		if(i == 1){
		        macaddr[4] = (value >> 0)  & 0xff;
			macaddr[5] = (value >> 8)  & 0xff;
		}
	}

	for(i = 0; i < 6; i++){
		kprintf("macaddr[%d]: 0x%X\r\n", i, macaddr[i]);
	}

	kprintf("\r\n\nPrinting ethptr->devAddress...\r\n");
        /* Place the MAC (obtained from VC mailbox) into the Ethernet Control Block. */
	for(i = 0; i < 6; i ++){
		ethptr->devAddress[i] = macaddr[i];
		kprintf("0x%X ", ethptr->devAddress[i]);
	}

	#endif
	//print_parameter(mailbuffer,"board serial", MBX_TAG_GET_BOARD_SERIAL, 2);

	// kprintf("\r\nLAN7800_GET_MAC_ADDRESS: %08X\r\n", lan7800_get_mac_address(&usb_devices[3], macaddr));
	kprintf("\r\n\n===========================================================\r\n");

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

	/*
#if NETHER
	netInit();
#endif
*/

#if GPIO
	gpioLEDOn(GPIO_LED_CISCOWHT);
#endif

	return OK;
}
