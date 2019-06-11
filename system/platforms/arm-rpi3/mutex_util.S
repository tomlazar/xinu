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

	pldw	[r0]

	ldrex	r1, [r0]		/* Load the value of the lock from memory */
	cmp	r1, #MUTEX_LOCKED	/* IF already locked ,*/
	beq	_mutex_acquire		/* THEN try again...*/

	mov	r1, #MUTEX_LOCKED	/* ELSE unlocked, try to lock..	*/
	strex	r2, r1, [r0]	
	cmp	r2, #0          	/* IF lock acquire failed this time around,*/
	bne	_mutex_acquire		/* THEN try again back up at the top */
	dmb		/* Data Memory Barrier opcode waits for memory accesses		*/
			/*    to complete before returning. */
	bx		lr

	.endfunc

_mutex_release:
	.func	_mutex_release

	mov		r1, #MUTEX_UNLOCKED
	dmb				/* Required before accessing protected resource */
	str		r1, [r0]	/* Unlock mutex */
	dsb
	//sev
	bx		lr

	.endfunc
