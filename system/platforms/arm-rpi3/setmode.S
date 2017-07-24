#include <arm.h>

.globl setmode
setmode:
	.func setmode

/*
	mrs     r0, cpsr
        bic     r0, r0, #ARM_MODE_SYS
        orr     r0, r0, #ARM_MODE_SVC
        msr     spsr_cxsf, r0
        add     r0, pc, #4
        msr     ELR_hyp, r0
        eret
*/
	
	cpsid	if, #ARM_MODE_SYS
	bx	lr

	.endfunc
