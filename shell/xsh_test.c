/**
 * @file     xsh_test.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _XINU_PLATFORM_ARM_RPI_3_
#include <core.h>
extern unsigned int getcpuid(void);
extern void udelay(uint);
static void test(void *ar)
{
	int i = (int) ar;
	udelay(250);
	kprintf("Core %d, arg: %d\r\n", getcpuid(), i);
	while (1) {}	
}
#endif
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
	int i, a;
	i = atoi(args[1]);
	a = 69;
	printf("Unparking core %d\n", i);
	unparkcore(atoi(args[1]), (void *) test, (void *) a);
	return 0;
}
