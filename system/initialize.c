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

#define RR_REQUEST 0x00000000
#define RR_RESPONSE_OK 0x80000000
#define RR_RESPONSE_ERROR 0x80000001

#define SLOT_OVERALL_LENGTH 0
#define SLOT_RR 1
#define SLOT_TAGSTART 2

#define SLOT_TAG_ID 0
#define SLOT_TAG_BUFLEN 1
#define SLOT_TAG_DATALEN 2
#define SLOT_TAG_DATA 3

#define MBOX_HEADER_LENGTH 2
#define TAG_HEADER_LENGTH 3

#define MBX_DEVICE_SDCARD 0x00000000
#define MBX_DEVICE_UART0 0x00000001
#define MBX_DEVICE_UART1 0x00000002
#define MBX_DEVICE_USBHCD 0x00000003
#define MBX_DEVICE_I2C0 0x00000004
#define MBX_DEVICE_I2C1 0x00000005
#define MBX_DEVICE_I2C2 0x00000006
#define MBX_DEVICE_SPI 0x00000007
#define MBX_DEVICE_CCP2TX 0x00000008

#define MBX_TAG_GET_FIRMWARE 0x00000001 /* in 0, out 4 */
#define MBX_TAG_GET_BOARD_MODEL 0x00010001 /* in 0, out 4 */
#define MBX_TAG_GET_BOARD_REVISION 0x00010002 /* in 0, out 4 */
#define MBX_TAG_GET_MAC_ADDRESS 0x00010003 /* in 0, out 6 */
#define MBX_TAG_GET_BOARD_SERIAL 0x00010004 /* in 0, out 8 */
#define MBX_TAG_GET_ARM_MEMORY 0x00010005 /* in 0, out 8 (4 -> base addr, 4 -> len in bytes) */
#define MBX_TAG_GET_VC_MEMORY 0x00010006 /* in 0, out 8 (4 -> base addr, 4 -> len in bytes) */
#define MBX_TAG_GET_COMMANDLINE 0x00050001 /* in 0, out variable */
#define MBX_TAG_GET_DMA_CHANNELS 0x00060001 /* in 0, out 4 */

#define MBX_TAG_GET_POWER_STATE 0x00020001 /* in 4 -> dev id, out 8 (4 -> device, 4 -> status) */
#define MBX_TAG_GET_TIMING 0x00020002 /* in 0, out 4 */
#define MBX_TAG_GET_FIRMWARE 0x00000001 /* in 0, out 4 */

extern void bcm2837_mailbox_write(uint, uint);
extern void bcm2837_mailbox_read(uint);
extern void bzero(void *, size_t);

volatile uint32_t mailbuffer[1024];

void add_mailbox_tag(volatile uint32_t* buffer, uint32_t tag, uint32_t buflen, uint32_t len, uint32_t* data) {
	volatile uint32_t* start = buffer + SLOT_TAGSTART;
	start[SLOT_TAG_ID] = tag;
	start[SLOT_TAG_BUFLEN] = buflen;
	start[SLOT_TAG_DATALEN] = len & 0x7FFFFFFF;

	uint32_t bufwords = buflen >> 2;

	if (0 == data) {
		for (int i = 0; i < bufwords; ++i) {
			start[SLOT_TAG_DATA + i] = 0;
		}
	} else {
		for (int i = 0; i < bufwords; ++i) {
			start[SLOT_TAG_DATA + i] = data[i];
		}
	}

	start[SLOT_TAG_DATA+bufwords] = 0; // end of tags, unless overwritten later
}

void build_mailbox_request(volatile uint32_t* buffer) {
	uint32_t tag_length = buffer[MBOX_HEADER_LENGTH + SLOT_TAG_BUFLEN];
	uint32_t end = (MBOX_HEADER_LENGTH*4) + (TAG_HEADER_LENGTH*4) + tag_length;
	uint32_t overall_length = end + 4;
	buffer[SLOT_OVERALL_LENGTH] = overall_length;
	buffer[SLOT_RR] = RR_REQUEST;
}

void dump_response(const char* name, int nwords) {
	kprintf("%s: ", name);
	for (int i = 0; i < nwords; ++i) {
		uint32_t value = mailbuffer[MBOX_HEADER_LENGTH + TAG_HEADER_LENGTH + i];
		kprintf("0x%08X ", value);
	}
	kprintf("\r\n");
}

void print_parameter(const char* name, uint32_t tag, int nwords) {
	add_mailbox_tag(mailbuffer, tag, nwords * 4, 0, 0);
	build_mailbox_request(mailbuffer);

	//  raspi_mini_uart_send_string("before:");
	//  //  raspi_mini_uart_send_newline();
	//  //  dump_mailbox_to_uart();
	///
	bcm2837_mailbox_write(8, (uint32_t)mailbuffer);
	bcm2837_mailbox_read(8);

	/* Valid response in data structure */
	if(mailbuffer[1] != 0x80000000) {
		kprintf("MAILBOX ERROR\r\n");
	} else {
		dump_response(name, nwords);
	}
}

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

	extern struct usb_device usb_devices[];	
	uint8_t macaddr[6] = {0};
	bzero(mailbuffer, 1024);
	print_parameter("board serial", MBX_TAG_GET_BOARD_SERIAL, 2);
	print_parameter("MAC address ", MBX_TAG_GET_MAC_ADDRESS, 2);
	lan7800_get_mac_address(&usb_devices[3], macaddr);
	for (int i = 0; i < 6; i++)
		kprintf("0x%02X ", macaddr[i]);
	kprintf("\r\n");

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

	/* //XXX
#if NETHER
netInit();
#endif
*/

#if GPIO
	gpioLEDOn(GPIO_LED_CISCOWHT);
#endif

	return OK;
}
