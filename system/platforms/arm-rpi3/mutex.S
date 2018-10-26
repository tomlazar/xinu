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
	pldw		[r0]			/* Preload Data into cache to increase chance of success 	*/

//	mov		r1, #0
	/* nop sled to wait for data the load into cache */
//	cmp		r1, #250
//	bge		continue
//	nop
//	add		r1, #1	


continue:
	ldrexb	r1, [r0]		/* Load the value of the lock from memory					*/

	cmp		r1, #LOCKED		/* IF already locked ,										*/
	beq		loop			/* THEN try again...										*/

	mov		r1, #LOCKED		/* ELSE unlocked, try to lock..								*/
	strexb	r2, r1, [r0]	
	cmp		r2, #0x0		/* IF lock acquire failed this time around,					*/
	bne		loop			/* THEN try again back up at the top						*/
	dmb						/* Data Memory Barrier opcode waits for memory accesses		*/
							/*    to complete before returning. 						*/
	pop		{lr}
	bx		lr

	.endfunc

mutex_release:
	.func	mutex_release

	mov		r1, #UNLOCKED
	dmb						/* Required before accessing protected resource */
	strb		r1, [r0]		/* Unlock mutex */
	bx		lr

	.endfunc
