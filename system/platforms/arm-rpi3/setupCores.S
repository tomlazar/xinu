#include <arm.h>

.globl CoreSetup
CoreSetup:
	mrs		r0, cpsr
	bic		r0, r0, #ARM_MODE_SYS
	orr		r0, r0, #ARM_MODE_SVC
	msr		spsr_cxsf, r0
	add		r0, pc, #4
	msr		ELR_hyp, r0
	eret

	cps		#ARM_MODE_SYS

	mrc		p15, 0, r0, c0, c0, 5		/* MPIDR */
	and		r0, r0, #7

	mov		r1, r0
	mov		r2, #4
	mul		r1, r1, r2
	
	ldr		r2, =core_init_sp
	ldr		sp, [r2, r1]

	ldr		r2, =corestart
	ldr		pc, [r2, r1]	

.globl Core1Setup
Core1Setup:  
	mrs     r0, cpsr
        bic     r0, r0, #ARM_MODE_SYS
	orr     r0, r0, #ARM_MODE_SVC
        msr     spsr_cxsf, r0
        add     r0, pc, #4
	msr    ELR_hyp, r0 
	eret	

	cps	#ARM_MODE_SYS

	ldr r0, =core_init_sp
	ldr sp, [r0, #4] 
	
//	ldr r0, =checkcore1
//	bl kprintf

	b createnullthread

.globl Core2Setup
Core2Setup:
	mrs     r0, cpsr
        bic     r0, r0, #ARM_MODE_SYS
        orr     r0, r0, #ARM_MODE_SVC
        msr     spsr_cxsf, r0
        add     r0, pc, #4
        msr    ELR_hyp, r0
        eret

        cps     #ARM_MODE_SYS

	ldr r0, =core_init_sp
	ldr sp, [r0, #8]
//	ldr r0, =checkcore2
//	bl kprintf
	b createnullthread

.globl Core3Setup
Core3Setup:
	mrs     r0, cpsr
        bic     r0, r0, #ARM_MODE_SYS
        orr     r0, r0, #ARM_MODE_SVC
        msr     spsr_cxsf, r0
        add     r0, pc, #4
        msr    ELR_hyp, r0
        eret

        cps     #ARM_MODE_SYS

	ldr r0, =core_init_sp
	ldr sp, [r0, #12] 
//	ldr r0, =checkcore3
//	bl kprintf
	b createnullthread


.section .data
checkcore1:
		.asciz	"1111111111111111111\r\n"
checkcore2:
		.asciz	"2222222222222222222\r\n"
checkcore3:
		.asciz	"3333333333333333333\r\n"
check:
		.asciz  "0x%08X\r\n"
