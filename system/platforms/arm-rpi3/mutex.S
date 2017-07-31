/*
 * file		mutex.S
 * provides	mutex_acquire mutex_release
 *
 * Written by:	Rade Latinovich
				Patrick McGee
 */

#include "mutex.h"

.globl mutex_acquire
.globl mutex_release

mutex_acquire:
	.func	mutex_acquire

	push	{lr}

loop:
	pld		[r0]			/* Preload Data into cache */

	mov		r1, #0
	/* nop sled to wait for data the load into cache */
	cmp		r1, #250
	bge		continue
	nop
	add		r1, #1	


continue:
	ldrex	r1, [r0]

	cmp		r1, #LOCKED
	beq		loop

	mov		r1, #LOCKED
	strex	r2, r1, [r0]
	cmp		r2, #0x0
	bne		loop
	dmb

	pop		{lr}
	bx		lr

	.endfunc

mutex_release:
	.func	mutex_release

	mov		r1, #UNLOCKED
	dmb						/* Required before accessing protected resource */
	str		r1, [r0]		/* Unlock mutex */
	bx		lr

	.endfunc
