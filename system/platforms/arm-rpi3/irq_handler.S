//TODO Write the IRQ_HANDLER
.globl irq_handler

irq_handler:
	.func irq_handler
	bl	led_on	
//	bl	dispatch
	ret
	.endfunc
