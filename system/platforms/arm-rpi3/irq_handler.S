//TODO Write the IRQ_HANDLER
.globl irq_handler

irq_handler:
	.func irq_handler
	bx	lr	
	.endfunc
