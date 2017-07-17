.globl getmode

	.func	getmode
getmode:
	mrs	r0, cpsr
	bx	lr
	.endfunc
