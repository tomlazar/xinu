#include <arm.h>
#include <mmu.h>

.globl CoreSetup
CoreSetup:
	/* change processor to SYSTEM mode */
	mrs		r0, cpsr
	orr		r0, r0, #ARM_MODE_SYS
	msr		spsr_cxsf, r0
	add		r0, pc, #4
	msr		ELR_hyp, r0
	eret

	/* set vector addresses */
	ldr		r1, =_vectors
	mcr		p15, 0, r1, c12, c0, 0 /* Write to VBAR */
	
	/* get CPUID from MPIDR register and put value in r0 */
	mrc		p15, 0, r0, c0, c0, 5		/* MPIDR */
	and		r0, r0, #7					/* last 3 bits is CPUID */

	/* multiply the CPUID by 4 to get the correct index for the arrays */
	mov		r1, r0
	mov		r2, #4
	mul		r1, r1, r2

	/* core_init_sp array is for the initial stack pointer for each core */	
	ldr		r2, =core_init_sp
	ldr		sp, [r2, r1]

	/* move value to non-volatile registers before calling start_mmu */
	mov		r4, r1

	/* call start_mmu so the programmer does not have to manual call it each time they unpark a core 			*/
	mov		r0, #MMUTABLEBASE		/* MMUTABLEBASE from mmu.h */
	bl		start_mmu

	/* corestart array of addresses for the instruction for each core to execute once setup is done */
	ldr		r0, =init_args
	ldr		r0, [r0, r4]
	ldr		r2, =corestart
	ldr		pc, [r2, r4]	


/* Send EVent routine so we can use it in C code.. (unparkcore.c) */
.globl sev
sev:
	.func sev
	sev
	bx	lr
	.endfunc
