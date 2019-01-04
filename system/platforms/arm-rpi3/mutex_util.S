/*
 * file		mutex.S
 * provides	mutex_acquire mutex_release
 *
 * Written by:	Rade Latinovich
				Patrick McGee
 */

#include "mutex.h"

.globl _mutex_acquire
.globl _mutex_release

_mutex_acquire:
	.func	_mutex_acquire

	push	{lr}

loop:
	pldw	[r0]				/* Preload Data into cache to increase chance of success 	*/

//	mov		r1, #0
	/* nop sled to wait for data the load into cache */
//	cmp		r1, #250
//	bge		continue
//	nop
//	add		r1, #1	


	ldrex	r1, [r0]			/* Load the value of the lock from memory					*/

	cmp		r1, #MUTEX_LOCKED	/* IF already locked ,										*/
	beq		wait				/* THEN try again...										*/

	mov		r1, #MUTEX_LOCKED	/* ELSE unlocked, try to lock..								*/
	strex	r2, r1, [r0]	
	cmp		r2, #0x0			/* IF lock acquire failed this time around,					*/
	bne		loop				/* THEN try again back up at the top						*/
	dmb							/* Data Memory Barrier opcode waits for memory accesses		*/
								/*    to complete before returning. 						*/
	pop		{lr}
	bx		lr

wait:
	wfe
	b	loop

	.endfunc

_mutex_release:
	.func	_mutex_release

	mov		r1, #MUTEX_UNLOCKED
	dmb							/* Required before accessing protected resource */
	str		r1, [r0]			/* Unlock mutex */
	dsb
	sev
	bx		lr

	.endfunc
