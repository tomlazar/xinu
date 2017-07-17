.globl getcpuid

        .func   getcpuid
getcpuid:
        mrc	p15, 0, r0, c0, c0, 5
	bx      lr
        .endfunc

