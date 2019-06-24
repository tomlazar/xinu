/**
 * @file testcases.c
 * @provides testcases
 *
 *
 * Modified by:	
 *
 * TA-BOT:MAILTO 
 *
 */
/* Embedded XINU, Copyright (C) 2007.  All rights reserved. */

#include <xinu.h>

/**
 * Prints the contents of the specified queue
 */
void
ostest_printQueue(qid_typ q)
{
	short head, tail, next, extent;
	kprintf("Queue(0x%08X) =\r\n", q);

	head = queuehead(q);
	tail = queuetail(q);
	next = head;

	extent = NQENT;

	while ((next < NQENT) && (next != tail) && (extent))
	{
		kprintf("   [%3d|%3d:%3d]\r\n", next, queuetab[next].prev, queuetab[next].next);
		next = queuetab[next].next;
		extent--;
	}
	if (!extent)
	{
		kprintf("   ...Loop detected in Queue!\r\n");
	}
	else
	{
		kprintf("   [%3d|%3d:%3d]\r\n", tail, queuetab[tail].prev, queuetab[tail].next);
	}
}

/**
 * A test process for priority testing
 */
int
testprocE(void)
{
	/* print nothing, do nothing */
	return 0;
}

/**
 * Print the pid of a process
 * @param times the number of repititions
 */
int
printTestPid(int times)
{
	int i = 0;
	uint cpuid = getcpuid();

	for (i = 0; i < times; i++)
	{
		kprintf("This is process %d \r\n", currpid[cpuid]);

		resched();
	}
}

/**
 * Create two simple processes and run them.
 * If process B gets to run, it was not starved,
 * and aging has been implemented successfully.
 */
void
testStarvation(void)
{
	ready(create((void *) printTestPid, INITSTK, PRIORITY_MED , "PROCESS-A", 1, 10), 0, 0);
	ready(create((void *) printTestPid, INITSTK, PRIORITY_LOW , "PROCESS-B", 1,  5), 0, 0);
}

/**
 * testprocF and G are used for testing preemption
 */
void
testprocF(void)
{
	uint cpuid = getcpuid();

	enable();
	kprintf("Process %d never yields.\r\n", currpid[cpuid]);
	while (TRUE)
		;
}

void
testprocG(pid_typ pid)
{
	uint cpuid = getcpuid();

	enable();
	irqmask ps;

	ps = disable();
	kprintf("Process %d preempted process %d.\r\n", currpid[cpuid], pid);

	/* Kill all processes before system hangs up */
	kill(pid);
	kill(currpid[cpuid]);

	restore(ps);
}


/**
 * testcases - called after initialization completes to test things.
 */
void testcases(void)
{
	uchar c;
	pid_typ pid, pid2;
	irqmask im;

	im = disable();

	kprintf("===TEST BEGIN===\r\n");

	proctab[10].state = PRSUSP;
	proctab[11].state = PRSUSP;
	proctab[12].state = PRSUSP;

	// TODO: Test your operating system!

	c = kgetc();
	switch (c)
	{
		/* Testing prioritize in isolation. */
		case 'a':
			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			
			kprintf("Add [10:PRIORITY_LOW] to queue\r\n");
			
			proctab[10].priority = PRIORITY_LOW;
			ready(10, 0, 0);
			
			/* Print queue */
			ostest_printQueue(readylist[0][PRIORITY_LOW]);

			remove(10);
			break;
		
		case 'b':
			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);

			kprintf("Add [10:PRIORITY_LOW] to queue\r\n");
			proctab[10].priority = PRIORITY_LOW;
			ready(10, 0, 0);

			kprintf("Add [11:PRIORITY_MED] to queue\r\n");
			proctab[11].priority = PRIORITY_MED;
			ready(11, 0, 0);

			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);


			remove(10);
			remove(11);	

			break;

		case 'c':
			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);

			kprintf("Add [10:PRIORITY_MED] to queue\r\n");
			proctab[10].priority = PRIORITY_MED;
			ready(10, 0, 0);

			kprintf("Add [11:PRIORITY_LOW] to queue\r\n");
			proctab[11].priority = PRIORITY_LOW;
			ready(11, 0, 0);

			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);


			remove(10);
			remove(11);	

		case 'd':
			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			kprintf("Add [10:PRIORITY_LOW] to queue\r\n");
			proctab[10].priority = PRIORITY_LOW;
			ready(10, 0, 0);

			kprintf("Add [11:PRIORITY_MED] to queue\r\n");
			proctab[11].priority = PRIORITY_MED;
			ready(11, 0, 0);

			kprintf("Add [12:PRIORITY_HIGH] to queue\r\n");
			proctab[12].priority = PRIORITY_HIGH;
			ready(12, 0, 0);

			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			remove(10);
			remove(11);	
			remove(12);
			
			break;

		case 'e':
			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			kprintf("Add [10:PRIORITY_HIGH] to queue\r\n");
			proctab[10].priority = PRIORITY_HIGH;
			ready(10, 0, 0);

			kprintf("Add [11:PRIORITY_MED] to queue\r\n");
			proctab[11].priority = PRIORITY_MED;
			ready(11, 0, 0);

			kprintf("Add [12:PRIORITY_MED] to queue\r\n");
			proctab[12].priority = PRIORITY_MED;
			ready(12, 0, 0);

			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			remove(10);
			remove(11);	
			remove(12);
			
			break;

		case 'f':
			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			kprintf("Add [10:PRIORITY_LOW] to queue\r\n");
			proctab[10].priority = PRIORITY_LOW;
			ready(10, 0, 0);

			kprintf("Add [11:PRIORITY_LOW] to queue\r\n");
			proctab[11].priority = PRIORITY_LOW;
			ready(11, 0, 0);

			kprintf("Add [12:PRIORITY_LOW] to queue\r\n");
			proctab[12].priority = PRIORITY_LOW;
			ready(12, 0, 0);

			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			remove(10);
			remove(11);	
			remove(12);
			
			break;

		case 'g':
			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			kprintf("Add [10:PRIORITY_MED] to queue\r\n");
			proctab[10].priority = PRIORITY_MED;
			ready(10, 0, 0);

			kprintf("Add [11:PRIORITY_MED] to queue\r\n");
			proctab[11].priority = PRIORITY_MED;
			ready(11, 0, 0);

			kprintf("Add [12:PRIORITY_MED] to queue\r\n");
			proctab[12].priority = PRIORITY_MED;
			ready(12, 0, 0);

			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			remove(10);
			remove(11);	
			remove(12);
			
			break;

		case 'h':
			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			kprintf("Add [10:PRIORITY_HIGH] to queue\r\n");
			proctab[10].priority = PRIORITY_HIGH;
			ready(10, 0, 0);

			kprintf("Add [11:PRIORITY_HIGH] to queue\r\n");
			proctab[11].priority = PRIORITY_HIGH;
			ready(11, 0, 0);

			kprintf("Add [12:PRIORITY_HIGH] to queue\r\n");
			proctab[12].priority = PRIORITY_HIGH;
			ready(12, 0, 0);

			ostest_printQueue(readylist[0][PRIORITY_LOW]);
			ostest_printQueue(readylist[0][PRIORITY_MED]);
			ostest_printQueue(readylist[0][PRIORITY_HIGH]);

			remove(10);
			remove(11);	
			remove(12);
			
			break;

		case 'i':
			break;

		/* Starvation */
		case 'j':
#if AGING
			kprintf("Testing starvation with AGING = TRUE\r\n");
#else
			kprintf("Testing starvation with AGING = FALSE\r\n");
#endif
			testStarvation();
			break;

		/* Preemption */
		case 'k':
			enable();

			/* This process normally would never yield */
			pid = 
				create((void *) testprocF, INITSTK, PRIORITY_HIGH, "Evil Proc", 0);

			/* This process should run if preemption works */
			pid2 = 
				create((void *) testprocG, INITSTK, PRIORITY_HIGH, "Pwning Proc",
					1, pid);

			ready(pid, RESCHED_NO, 0);
			ready(pid2, RESCHED_YES, 0);

			break;

		default:
			kprintf("Unknown test %c.\r\n", c);
			break;
	}

	while (numproc > 4)
	{
		resched();
	}

	kprintf("\r\n===TEST END===\r\n");

	proctab[10].state = PRFREE;
	proctab[11].state = PRFREE;
	proctab[12].state = PRFREE;

	kprintf("\r\n\r\nAll user processes have completed.\r\n\r\n");
	while (1)
		;

	return;
}
