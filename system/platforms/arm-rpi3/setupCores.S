#include <arm.h>

.globl CoreSetup
CoreSetup:
	mrs		r0, cpsr
//	bic		r0, r0, #ARM_MODE_SYS
	orr		r0, r0, #ARM_MODE_SYS
	msr		spsr_cxsf, r0
	add		r0, pc, #4
	msr		ELR_hyp, r0
	eret

//	cps		#ARM_MODE_SYS

	mrc		p15, 0, r0, c0, c0, 5		/* MPIDR */
	and		r0, r0, #7

	mov		r1, r0
	mov		r2, #4
	mul		r1, r1, r2
	
	ldr		r2, =core_init_sp
	ldr		sp, [r2, r1]

	ldr		r2, =corestart
	ldr		pc, [r2, r1]	


