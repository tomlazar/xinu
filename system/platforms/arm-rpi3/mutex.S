/*
 * file		mutex.S
 * provides	mutex_acquire mutex_release
 *
 * Written by:	Rade Latinovich
		Patrick McGee
		Benjamin Levandowski
 */

#include "mutex.h"

.globl mutex_acquire
.globl mutex_release

mutex_acquire:
	.func	mutex_acquire

	push	{lr}

	mov	r1, #LOCKED

preload:
	pldw	[r0]		// Not necessary, but invalidates other cores cache lines
	nop

	ldrexb	r2, [r0]	// Load lock value with exclusive access
	cmp	r2, #0		// Is lock taken?
	strexbeq r2, r1, [r0] 	// Attempt to store lock taken vlaue
	cmpeq	r2, #0		// Did the store succeed?
	bne	preload		// Try again if lock is take or store failed
	dmb			// data memory barrier

	pop		{lr}
	bx		lr

	.endfunc

mutex_release:
	.func	mutex_release

	mov	r1, #UNLOCKED
	dmb			// data memory barrier
	strb	r1, [r0]	// Free lock
	dsb			// data synchronization barrier

	bx		lr

	.endfunc
