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
#include <thread.h>

extern void led_init(void);
extern void led_off(void);
extern void stop_mmu(void);
extern void invalidate_tlbs(void);
extern syscall kexec(const void *, uint);

const ulong blink_kernel[] = {
	0xe59f000c,	/* ldr	r0, [pc. #12]	*/
	0xe3a01001,	/* mov	r1, #1		*/
	0xe1a01801,	/* lsl	r1, r1, #16	*/
	0xe5801000,	/* str	r1, [r0]	*/
	0xeafffffe,	/* b	10		*/
	0x3f20001c
};

static thread test_thread(void);
static thread test_thread1(void);
static thread test_thread2(tid_typ);

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
#if 0
	stop_mmu();
	invalidate_tlbs();
	led_init();
	led_off();

	kexec((const void *)blink_kernel, sizeof(blink_kernel));	
#endif
#if 1
	tid_typ tid1 = create(test_thread1, INITSTK, INITPRIO, "TEST01", 0);
	tid_typ tid2 = create(test_thread2, INITSTK, INITPRIO, "TEST02", 1, tid1);
//	tid_typ tid3 = create(test_thread, INITSTK, 100, "TEST03", 0);

	ready_multi(tid1, 1);	
	ready_multi(tid2, 2);
#endif
	return 0;
}

static thread test_thread()
{
	disable();
	uint cpuid = getcpuid();
//	udelay(500);
	kprintf("\rtest_thread on core %u\r\n", cpuid);
	return OK;
}

static thread test_thread1()
{
	disable();
	message msg = recvtime(1000);
	if (TIMEOUT == msg)
	{
		kprintf("msg timed out\r\n");
		return OK;
	}
	kprintf("received: %d\r\n", msg);
	return OK;
}

static thread test_thread2(tid_typ tid)
{
	disable();
	send(tid, 69);
	return OK;
}
