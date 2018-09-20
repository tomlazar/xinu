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

extern void test_boundedbuffer(void);
extern void unparkcore(uint, void *, void *);
extern uint getcpuid(void);

static void print_test(void)
{
	uint cpuid = getcpuid();

	while(1)
		kprintf("THIS IS CORE %d SAYING HELLO\r\n", cpuid);
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
//	test_boundedbuffer();
	udelay(500);
	kprintf("\r\n--- PRINT TEST ON MULTIPLE CORES ---\r\n");
	unparkcore(1, (void *) print_test, NULL);
	unparkcore(2, (void *) print_test, NULL);
	unparkcore(3, (void *) print_test, NULL);
	while (1) ;
	return 0;
}
