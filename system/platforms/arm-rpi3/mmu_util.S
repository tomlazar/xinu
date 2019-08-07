/* mmu_util.S
 * Contains assembly routines for cache maintenance on the ARM Cortex A53.
 * Authors: Rade Latinovich and Patrick J. McGee
 * Embedded Xinu, Copyright 2019
 */
.globl _getcacheinfo
_getcacheinfo:
	mrc	p15, 1, r0, c0, c0, 0	;@ Read CCSIDR into r0
	bx	lr

.globl _getcachemaint
_getcachemaint:
	mrc	p15, 0, r0, c0, c1, 7	;@ Read ID_MMFR3 (32 bit) into r0
	bx	lr

/* Brief explanation about DMA and Cortex-A53:
 * https://stackoverflow.com/questions/42081435/flush-invalidate-range-by-virtual-address-armv8-cache/
 */

/* DCCMVAC (clean/flush specific value)
 * Input: 32-bit virtual address
 * Armv8 architecture reference manual page 5720 */
.globl _flush_area
_flush_area:
	//mcr     p15, 0, r0, c7, c10, 1         ;@ Clean line by VA to PoC
	mcr     p15, 0, r0, c7, c11, 1         ;@ Clean line by VA to PoU
	bx	lr

/* DCIMVAC (invalidate specific value)
 * Input: 32-bit virtual address */
.globl _inval_area
_inval_area:
	mcr     p15, 0, r0, c7, c6, 1         ;@ Invalidate line by VA to PoC
	bx	lr

.globl _inval_cache
_inval_cache:

	//mcr     p15, 0, r0, c8, c6, 0       ;@ Invalidate entire data TLB
	mov	r0, #0x0		      ;@ Start at zero for first cache section (way)
isec1:	
	mcr     p15, 0, r0, c7, c6, 2         ;@ Invalidate the line by set/way
	add	r0, r0, #0x10                 ;@ Increment by 64 bytes (segment is 16 words, 16*4=64 bytes)
	cmp	r0, #0x1FC0		      ;@ Reached end of set (bits [12:6])
	bne     sec1                          ;@ If not branch back to set1

	movt	r0, #0x4000		      ;@ Use movw and movt to load the full 32-bit constant
	movw	r0, #0x0000		      ;@ Start at 0x40000000 for second cache section (way)
isec2:
	mcr     p15, 0, r0, c7, c6, 2         ;@ Invalidate the line by set/way
	add	r0, r0, #0x10                 ;@ Increment to next line

	and	r1, r0, #0xBFFFFFFF	      ;@ Clear top way bit because we're checking the lower 16 set bits, for 1FC0
	cmp	r1, #0x1FC0		      ;@ Reached end of set (bits [12:6])
	bne     sec2                          ;@ If not branch back to sec2

	movt	r0, #0x8000
	movw	r0, #0x0000		      ;@ Start at 0x80000000 for third way
isec3:
	mcr     p15, 0, r0, c7, c6, 2         ;@ Invalidate the line by set/way
	add	r0, r0, #0x10                 ;@ Increment to next line
	
	and	r1, r0, #0x1FFFFFFF
	cmp	r1, #0x1FC0		      ;@ Reached end of set (bits [12:6])
	bne     sec3                          ;@ If not branch back to sec3

	movt	r0, #0xC000
	movw	r0, #0x0000		      ;@ Start at 0xC0000000 for fourth way
isec4:
	mcr     p15, 0, r0, c7, c6, 2         ;@ Invalidate the line by set/way
	add	r0, r0, #0x10                 ;@ Increment to next line
	
	and	r1, r0, #0x3FFFFFFF
	cmp	r1, #0x1FC0		      ;@ Reached end of set (bits [12:6])
	bne     sec4                          ;@ If not branch back to sec4

	bx	lr

.globl _flush_cache
_flush_cache:

	/* From Cortex A53 page 330:
		The ARMv8-A architecture does not support an operation to invalidate the
entire data cache. If this function is required in software, it must be
constructed by iterating over the cache geometry and executing a series of
individual invalidate by set/way instructions.
The Cortex-A53 processor automatically invalidates caches on reset
unless suppressed with the DBGL1RSTDISABLE or L2RSTDISABLE
pins. It is therefore not necessary for software to invalidate the caches on
startup.
	*/

	mcr     p15, 0, r0, c8, c6, 0         ;@ Invalidate entire data TLB
	mov	r0, #0x0		      ;@ Start at zero for first cache section (way)
sec1:	
	mcr     p15, 0, r0, c7, c10, 2        ;@ Clean and flush the line by set/way (cache line index held by r0)
	//mcr     p15, 0, r0, c7, c6, 2         ;@ Invalidate the line by set/way
	add	r0, r0, #0x10                 ;@ Increment by 64 bytes (segment is 16 words, 16*4=64 bytes)
	cmp	r0, #0x1FC0		      ;@ Reached end of set (bits [12:6])
	bne     sec1                          ;@ If not branch back to set1

	movt	r0, #0x4000		      ;@ Use movw and movt to load the full 32-bit constant
	movw	r0, #0x0000		      ;@ Start at 0x40000000 for second cache section (way)
sec2:
	mcr     p15, 0, r0, c7, c10, 2        ;@ Clean and flush the line by set/way
	//mcr     p15, 0, r0, c7, c6, 2         ;@ Invalidate the line by set/way
	add	r0, r0, #0x10                 ;@ Increment to next line

	and	r1, r0, #0xBFFFFFFF	      ;@ Clear top way bit because we're checking the lower 16 set bits, for 1FC0
	cmp	r1, #0x1FC0		      ;@ Reached end of set (bits [12:6])
	bne     sec2                          ;@ If not branch back to sec2

	movt	r0, #0x8000
	movw	r0, #0x0000		      ;@ Start at 0x80000000 for third way
sec3:
	mcr     p15, 0, r0, c7, c10, 2        ;@ Clean and flush the line by set/way
	//mcr     p15, 0, r0, c7, c6, 2         ;@ Invalidate the line by set/way
	add	r0, r0, #0x10                 ;@ Increment to next line
	
	and	r1, r0, #0x1FFFFFFF
	cmp	r1, #0x1FC0		      ;@ Reached end of set (bits [12:6])
	bne     sec3                          ;@ If not branch back to sec3

	movt	r0, #0xC000
	movw	r0, #0x0000		      ;@ Start at 0xC0000000 for fourth way
sec4:
	mcr     p15, 0, r0, c7, c10, 2        ;@ Clean and flush the line by set/way
	//mcr     p15, 0, r0, c7, c6, 2         ;@ Invalidate the line by set/way
	add	r0, r0, #0x10                 ;@ Increment to next line
	
	and	r1, r0, #0x3FFFFFFF
	cmp	r1, #0x1FC0		      ;@ Reached end of set (bits [12:6])
	bne     sec4                          ;@ If not branch back to sec4

	bx	lr

.globl start_mmu
start_mmu:
	mov	r2, #0
	mcr	p15, 0, r2, c7, c1, 0	;@ invalidate all instruction caches inner shareable to PoU (point of unification)
	mcr	p15, 0, r2, c7, c5, 0	;@ invalidate all instruction caches to PoU
	mcr	p15, 0, r2, c7, c6, 1	;@ invalidate d cache line by VA (virtually addressed cache) to PoC (point of coherence
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
