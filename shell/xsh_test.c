/**
 * @file     xsh_test.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <shell.h>
#include <string.h>
#include <ctype.h>
#include <clock.h>
#include <testsuite.h>
#include <device.h>
#include <uart.h>

/**
 * @ingroup shell
 *
 * Shell command (test) provides a mechanism for testing Xinu features.  The
 * action and output varies depending on the feature currently being tested.
 * This is not meant to serve as a permanent shell command for a particular
 * action.
 * @param nargs number of arguments
 * @param args  array of arguments
 * @return non-zero value on error
 */

struct thrent *ppcb = NULL;
int testmain(int argc, char **argv)
{
	uint cpuid = getcpuid();
	int i = 0;
	kprintf("\r\n********=======********\r\n");
	
	struct thrent *thr = &thrtab[thrcurrent[cpuid]];

	for (i = 0; i < 10; i++)
	{
		kprintf("NAME: %s\tTID: %d\tCore: %d\r\n", thr->name, thrcurrent[cpuid], cpuid);

		/* Uncomment the resched() line for cooperative scheduling. */
		//resched();
	}
	return 0;
}

void printpcb(int pid)
{
	struct thrent *ppcb = NULL;

	/* Using the process ID, access it in the PCB table. */
	ppcb = &thrtab[pid];

	/* Printing PCB */
	kprintf("Process name		  : %s \r\n", ppcb->name);

	switch (ppcb->state)
	{
		case THRFREE:
			kprintf("State of the process	  : FREE \r\n");
			break;
		case THRCURR:
			kprintf("State of the process 	  : CURRENT \r\n");
			break;
		case THRSUSP:
			kprintf("State of the process	  : SUSPENDED \r\n");
			break;
		case THRREADY:
			kprintf("State of the process	  : READY \r\n");
			break;
		default:
			kprintf("ERROR: Process state not correctly set!\r\n");
			break;
	}

	/* Print PCB contents and registers */
	kprintf("Base of run time stack    : 0x%08X \r\n", ppcb->stkbase);
	kprintf("Stack length of process   : %8u \r\n", ppcb->stklen);
}

shellcmd xsh_test(int nargs, char *args[])
{
	int c, pid;
	int pids[12];

	kprintf("0) Test creation of one process\r\n");
	kprintf("1) Test passing of many args\r\n");
	kprintf("2) Create three processes and run them\r\n");
	kprintf("3) Create three processes and run them on other cores\r\n");
	kprintf("4) Create three processes with different priorities and run them (RESCHED_YES)\r\n");
	kprintf("5) Create three processes with different priorities and run them (RESCHED_NO)\r\n");

	kprintf("===TEST BEGIN===\r\n");

	// TODO: Test your operating system!

	device *devptr;
	devptr = (device *)&devtab[SERIAL0];
	c = kgetc(devptr);
	switch (c)
	{
		case '0':
			// Process creation testcase
			pid = create((void *)testmain, INITSTK, 2, "MAIN1", 0, NULL);
			printpcb(pid);
			break;

		case '1':
			// Many arguments testcase
			//pid = create((void *)testbigargs, INITSTK, 2, "MAIN1", 8,
			//             0x11111111, 0x22222222, 0x33333333, 0x44444444,
			//             0x55555555, 0x66666666, 0x77777777, 0x88888888);
			printpcb(pid);
			// TODO: print out stack with extra args
			// TODO: ready(pid, RESCHED_YES, 0);
			break;

		case '2':
			// Create three copies of a process, and let them play.
			ready(create((void *)testmain, INITSTK, 2, "MAIN1", 0, NULL), RESCHED_NO , 0);
			ready(create((void *)testmain, INITSTK, 2, "MAIN2", 0, NULL), RESCHED_NO , 0);
			ready(create((void *)testmain, INITSTK, 2, "MAIN3", 0, NULL), RESCHED_YES, 0);
			break;

		case '3':
			// Create 3 processes and ready them on cores 1, 2, 3
			ready(create((void *)testmain, INITSTK, 2, "MAIN1", 0, NULL), RESCHED_NO, 1);
			ready(create((void *)testmain, INITSTK, 2, "MAIN2", 0, NULL), RESCHED_NO, 2);
			//break;
			ready(create((void *)testmain, INITSTK, 2, "MAIN3", 0, NULL), RESCHED_NO, 3);
			break;
		case '4':
			//create 3 processes on the same core with different priorities; resched on
			ready(create((void *)testmain, INITSTK, 1, "PRIORITY1", 0, NULL), RESCHED_YES, 0);
			ready(create((void *)testmain, INITSTK, 2, "PRIORITY2", 0, NULL), RESCHED_YES, 0);
			ready(create((void *)testmain, INITSTK, 3, "PRIORITY3", 0, NULL), RESCHED_YES, 0);

			ready(create((void *)testmain, INITSTK, 1, "PRIORITY1", 0, NULL), RESCHED_YES, 1);
			ready(create((void *)testmain, INITSTK, 2, "PRIORITY2", 0, NULL), RESCHED_YES, 1);
			ready(create((void *)testmain, INITSTK, 3, "PRIORITY3", 0, NULL), RESCHED_YES, 1);

			ready(create((void *)testmain, INITSTK, 1, "PRIORITY1", 0, NULL), RESCHED_YES, 2);
			ready(create((void *)testmain, INITSTK, 2, "PRIORITY2", 0, NULL), RESCHED_YES, 2);
			ready(create((void *)testmain, INITSTK, 3, "PRIORITY3", 0, NULL), RESCHED_YES, 2);
			
			ready(create((void *)testmain, INITSTK, 1, "PRIORITY1", 0, NULL), RESCHED_YES, 3);
			ready(create((void *)testmain, INITSTK, 2, "PRIORITY2", 0, NULL), RESCHED_YES, 3);
			ready(create((void *)testmain, INITSTK, 3, "PRIORITY3", 0, NULL), RESCHED_YES, 3);
			
			break;		
		case '5':
			//create 3 processes on the same core with different priorities, resched off
			ready(create((void *)testmain, INITSTK, 1, "PRIORITY1", 0, NULL), RESCHED_NO, 0);
			ready(create((void *)testmain, INITSTK, 2, "PRIORITY2", 0, NULL), RESCHED_NO, 0);
			ready(create((void *)testmain, INITSTK, 3, "PRIORITY3", 0, NULL), RESCHED_NO, 0);
			
			ready(create((void *)testmain, INITSTK, 1, "PRIORITY1", 0, NULL), RESCHED_NO, 1);
			ready(create((void *)testmain, INITSTK, 2, "PRIORITY2", 0, NULL), RESCHED_NO, 1);
			ready(create((void *)testmain, INITSTK, 3, "PRIORITY3", 0, NULL), RESCHED_NO, 1);
			
			ready(create((void *)testmain, INITSTK, 1, "PRIORITY1", 0, NULL), RESCHED_NO, 2);
			ready(create((void *)testmain, INITSTK, 2, "PRIORITY2", 0, NULL), RESCHED_NO, 2);
			ready(create((void *)testmain, INITSTK, 3, "PRIORITY3", 0, NULL), RESCHED_NO, 2);
			
			ready(create((void *)testmain, INITSTK, 1, "PRIORITY1", 0, NULL), RESCHED_NO, 3);
			ready(create((void *)testmain, INITSTK, 2, "PRIORITY2", 0, NULL), RESCHED_NO, 3);
			ready(create((void *)testmain, INITSTK, 3, "PRIORITY3", 0, NULL), RESCHED_NO, 3);
			break;
		default:
			break;
	}

	kprintf("\r\n===TEST END===\r\n");
	return;
}
