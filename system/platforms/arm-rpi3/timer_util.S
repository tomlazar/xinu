/**
 * @file timer_util.S
 * @provides assembly subroutines necessary
 *           for Timer functions.
 *
 */
/* Embedded Xinu, Copyright (C) 2018. All rights reserved. */

.globl CLEAR_ISTATUS
/**
 * @fn void CLEAR_ISTATUS(void)
 *
 * Clears ISTATUS bit of CNTP_CTL register.
 *
 */
CLEAR_ISTATUS:
	.func CLEAR_ISTATUS
	mrc	p15, 0, r0, c14, c2, 1		/* CNTP_CTL */
	bic	r2, #0b100	// ISTATUS bit
	mcr	p15, 0, r0, c14, c2, 1
	bx	lr
	.endfunc

.globl CLKUPDATE
/**
 * @fn void CLKUPDATE(int count)
 *
 * Update timer to interrupt in count cycles.
 *
 * @param count  amount of timer cycles
 *               until next interrupt is triggered.
 *
 */
CLKUPDATE:
	.func CLKUPDATE
	mrrc	p15, 0, r1, r2, c14		/* CNTPCT (timer count) */
	add		r1, r0
	mcrr	p15, 2, r1, r2, c14		/* CNTP_CVAL (CompareValue) */	

	bx	lr
	.endfunc
