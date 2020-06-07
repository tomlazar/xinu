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
#include <mmu.h>
#include <thread.h>

void testproc(int i)
{
	int j;
	uint cpuid = getcpuid();

	enable();

	for (j = 0; j < i; j++)
	{
		kprintf("Hello from TID %d\r\n", thrcurrent[cpuid]);
	}
}

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
shellcmd xsh_test(int nargs, char *args[])
{
	kprintf("\r\n===TEST BEGIN===\r\n");

	message msg;
	int done1 = 0;
	int done2 = 0;
	tid_typ tid1, tid2;
	tid1 = create(testproc, INITSTK, INITPRIO, "TESTPROC-1", 1, 100);
	tid2 = create(testproc, INITSTK, INITPRIO, "TESTPROC-2", 1, 100);	

	kprintf("Readying tid %d and %d on core 0\r\n", tid1, tid2);

	ready(tid1, RESCHED_NO, 0);
	ready(tid2, RESCHED_NO, 0);

	while (!done1 || !done2)
	{
		msg = receive();
		if (msg == (message)tid1) { done1 = 1; }
		if (msg == (message)tid2) { done2 = 1; }
	}

	kprintf("\r\n===TEST END===\r\n");
	return 0;
}
