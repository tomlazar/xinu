.globl	getcpuid
.func	getcpuid
getcpuid:
	mrc p15, 0, r0, c0, c0, 5	// ACCESS MPIDR REGISTER AND STORE IN R0
	and	r0, r0, #7				// RETURN ONLY THE FIRST THREE BITS
	bx	lr
	.endfunc
