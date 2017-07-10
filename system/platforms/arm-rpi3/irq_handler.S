//TODO Write the IRQ_HANDLER
.globl irq_handler

irq_handler:
	.func irq_handler
	sub sp, sp, #32
	stp	x0, x1, [sp]
	stp x2, x3, [sp]

	bl	led_on	
	bl	dispatch

	ldp x0, x1, [sp]
	ldp x2, x3, [sp]

	add sp, sp, #32
	eret
	.endfunc
