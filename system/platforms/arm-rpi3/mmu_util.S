/* mmu_util.S
 * Contains assembly routines for cache maintenance on the ARM Cortex A53.
 * Authors: Rade Latinovich and Patrick J. McGee
 * Embedded Xinu, Copyright 2019
 */
.globl _getcacheinfo
_getcacheinfo:
	mrc	p15, 1, r0, c0, c0, 0	;@ Read CCSIDR 
	//ldr     r0, [r0]
	bx	lr

.globl _flush_cache
_flush_cache:
	mov	r2, #0			;@ Flush entire cache by transferring a zero
	//mcr	p15, 0, r2, c8, c7, 0	;@ Invalidate unified tlb
	//mcr	p15, 0, r2, c7, c1, 0	;@ Invalidate all i caches inner shareable to PoU
	
	mcr	p15, 0, r2, c7, c10, 1	;@ Flush data cache by VA to PoC
	mcr	p15, 0, r2, c7, c10, 2	;@ Flush data cache set/way
	
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
