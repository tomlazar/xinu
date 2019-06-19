//.equ	MMU_TABLE_BASE,	0x00004000

/*
.globl mmu_initialize
mmu_initialize:
	// Code adapted from Bare-metal Boot Code for ARMv8-A Processors v1.0
	ldr	r0, =MMU_TABLE_BASE
    ldr	r4, =0x00100000	// Increment value
	ldr	r2, =0x00015C06	// Memory is shareable (p. 4861 of ARM ARM v8)
	ldr	r3, =0x3F000000 // Peripherials start here

memLoop:
	str	r2, [r0], #4
	add	r2, r4
	cmp	r3, r2
	bhi	memLoop

	ldr	r2, =0x3F000C02	// Peripherials are non-cacheable device memory
	ldr	r3, =0xFFF00000 // Stop here
perLoop:
	str	r2, [r0], #4
	add	r2, r4
	cmp	r3, r2
	bhi	perLoop

	ldr r0, =MMU_TABLE_BASE	// Re-load table base to start MMU
*/

.globl _flush_cache
_flush_cache:
	mov	r2, #0			;@ Flush entire cache by transferring a zero
	//mcr	p15, 0, r2, c8, c7, 0	;@ Invalidate unified tlb
	//mcr	p15, 0, r2, c7, c1, 0	;@ Invalidate all i caches inner shareable to PoU
	
	//mcr	p15, 0, r2, c7, c5, 0	;@ Flush instruction cache
	mcr	p15, 0, r2, c7, c6, 0	;@ Flush data cache
	dsb
	
	bx	lr

.globl start_mmu
start_mmu:
	mov	r2, #0
	mcr	p15, 0, r2, c7, c1, 0	;@ invalidate all i caches inner shareable to PoU
	mcr	p15, 0, r2, c7, c5, 0	;@ invalidate all i caches to PoU
	mcr	p15, 0, r2, c7, c6, 1	;@ invalidate d cache line by VA to PoC
	mcr	p15, 0, r2, c7, c6, 2	;@ invalidate d cache line by set/way
	mcr	p15, 0, r2, c8, c7, 0	;@ invalidate tlb	/* invalidate unified TLB	*/
	dsb
	
	mvn	r2, #0
	mcr	p15, 0, r2, c3, c0, 0	;@ domain

	orr	r0, #0x6A
	
	orr	r0, #0x6A

	mcr	p15, 0, r0, c2, c0, 0	;@ tlb base
	mcr	p15, 0, r0, c2, c0, 1	;@ tlb base

	/* SCTLR configuration*/
	mrc	p15, 0, r2, c1, c0, 0
//	orr	r2, r2, r1
	/* gets rid of the need for a 2nd argument when using start_mmu() */
	/* makes it simpler to use */
	mov r1, #0x1
	orr	r1, r1, #0x1000
	orr	r1, r1, #0x4
	orr	r2, r2, r1
	mcr	p15, 0, r2, c1, c0, 0

	bx	lr


.globl stop_mmu
stop_mmu:
	mrc	p15, 0, r2, c1, c0, 0
	bic	r2, #0x1000
	bic	r2, #0x0004
	bic	r2, #0x0001
	mcr	p15, 0, r2, c1, c0, 0

	bx	lr

.globl invalidate_tlbs
invalidate_tlbs:
	mov	r2, #0
	mcr	p15, 0, r2, c8, c7, 0
	dsb
	bx	lr

.globl PUT32
PUT32:
	str	r1, [r0]
	bx	lr

.globl GET32
GET32:
	ldr	r0, [r0]
	bx	lr
