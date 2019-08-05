/**
 * @file atomic.S
 *
 * Atomic assembly subroutines.
 *
 * Embedded Xinu, Copyright (C) 2018. All rights reserved.
 *
 */

.globl _atomic_increment
/**
 * @fn int _atomic_increment(int *var)
 *
 * Atomically increments specified integer.
 * (Equivalent to ++var in C).
 *
 * @param *var pointer to variable
 * @return value the variable held after
 *         being incremented.
 */
_atomic_increment:
	.func _atomic_increment
	pldw	[r0]
	ldrex	r1, [r0]	// load
	add		r1, #1		// increment
	strex	r2, r1, [r0] // store
	cmp		r2, #0		// if store fails, try again
	bne		_atomic_increment
	
	mov	r0, r1			// return incremented value
	dmb

	bx	lr
	.endfunc


.globl _atomic_decrement
/**
 * @fn int _atomic_decrement(int *var)
 *
 * Atomically decrements specified integer.
 * (Equivalent to --var in C).
 *
 * @param *var pointer to variable
 * @return value the variable held after
 *         being decremented.
 */
_atomic_decrement:
    .func _atomic_decrement
    pldw    [r0]
    ldrex   r1, [r0]    // load
    sub     r1, #1      // decrement
    strex   r2, r1, [r0] // store
    cmp     r2, #0      // if store fails, try again
    bne     _atomic_decrement

    mov r0, r1          // return decremented value
    dmb

    bx  lr
    .endfunc


.globl _atomic_increment_mod
/**
 * @fn int _atomic_increment_mod(int *var, int mod)
 *
 * Atomically increment and then mod with specified value.
 * Equivalent to var = (var + 1) % mod.
 *
 * @param *var  pointer to variable
 * @param mod value to wrap-around if
 *              var exceeds it.
 * @return incremented var value
 */
_atomic_increment_mod:
    .func _atomic_increment_mod
    pldw    [r0]
    ldrex   r2, [r0]        // load
    add     r2, #1          // increment
    cmp     r2, r1          // if >= to limit, wrap to zero
    movge   r2, #0
    strex   r3, r2, [r0]    // store
    cmp     r3, #0          // if it fails, retry
    bne     _atomic_increment_mod

    mov     r0, r2
    dmb

    bx  lr
    .endfunc

