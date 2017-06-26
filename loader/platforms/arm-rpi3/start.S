/**
 * @file start.S
 *
 * Initialization code for Embedded Xinu on the Raspberry Pi 3.
 */

/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */


#include <arm.h>     /* For ARM_MODE_SYS */

#define NULLSTK 8192

.section .init
	.globl _start

	/* _start:  Entry point of the Xinu kernel.  This will be the very first
	 * byte of the kernel image and on the Raspberry Pi 3 will be loaded by
	 * the GPU at address 0x80000.  */
	.func _start
_start:
	/* Save the pointer to the atags (ARM boot tags).  The bootloader should
	 * pass this in x0, but override 0 with 0x100 to deal with old
	 * bootloaders that assume Linux's default behavior.  (This is the case
	 * with, for example, the bootloader on the BCM2835.)  */
	ldr 	x3, =atags_ptr
	str 	x0, [x3]

	/* Continue execution at el_setter.  */
//	b 	el_setter
	b	reset_handler
	.endfunc

/* ARM exception vector table.  This is copied to VBAR_ELn register. */
_vectors:
	/* Current EL with SP0 */
	b		sync_addr	/* Synchronous */
	.balign	128	
	b		irq_addr	/* IRQ/vIRQ */
	.balign	128
	b		fiq_addr	/* FIQ/vFIQ */
	.balign	128
	b		serr_addr	/* SError/vSError */
	.balign	128
	/* Current EL with SPx */
	b		sync_addr
	.balign	128
	b		irq_addr
	.balign	128
	b		fiq_addr
	.balign	128
	b		serr_addr
	.balign	128
	/* Lower EL using AArch64 */
	b		sync_addr
	.balign	128
	b		irq_addr
	.balign	128
	b		fiq_addr
	.balign	128
	b		serr_addr
	.balign	128
	/* Lower EL using AArch32 */
	b		sync_addr
	.balign	128
	b		irq_addr
	.balign	128
	b		fiq_addr
	.balign	128
	b		serr_addr

sync_addr:		.word reset_handler
irq_addr:       .word irq_handler
fiq_addr:       .word reset_handler
serr_addr:		.word reset_handler

_endvectors:

.section .text

/* el_setter: Sets processor to correct Exception Level (EL1)
 * for kernel operation. EL upon boot is EL3, while EL1 is necessary 
 * for kernel code */
.func el_setter
el_setter:
	/* Disable MMU at EL1 */
	msr	sctlr_el1, xzr

	mov	x0, xzr
	orr	x0, x0, #(1 << 10)	// .RW = 0b1 --> EL1 is AArch64
	orr x0, x0, #(1 << 0)	// .NS = 0b1 --> Non-secure status
	msr	scr_el3, x0

	mov	x0, xzr
	orr	x0, x0, #(7 << 6)	// .A = .I = .F = 0b1 --> SError, IRQ, FIQ enabled
//	orr x0, x0, #(0 << 4)	// .M[4] = 0b0 --> Return to AArch64 state
	orr	x0, x0, #(1 << 2)	// .M[3:1] = 0b010 --> Return to EL1 (SYS Mode equivalent)
	orr	x0, x0, #(1 << 0)	// .M[0] = 0b1 --> Use EL1's dedicated stack
	msr	spsr_el3, x0
	
//	adrp x0, reset_handler			// Program EL1 entrypoint
//	add	x0, x0, :lo12:reset_handler
	ldr	x0, =reset_handler
	msr	elr_el3, x0		

	eret
.endfunc


	/* reset_handler: Reset handler routine executed to start up the kernel,
	 * when the ARM processor is reset, or (currently) when an unhandled
	 * exception occurs.  */
	.func reset_handler
reset_handler:
	bl	init_led
	bl	led_on

	/* disables irq and fiq interupts */
	msr daifclr, 0b0011  

	/* Place vector table address in VBAR_ELn register so 
	 * processor knows where it is */
	ldr	x0, =_vectors
	mov	x1, #11
	lsl	x0, x0, x1
	msr	VBAR_EL1, x0
//	msr VBAR_EL2, x0
//	msr VBAR_EL3, x0

/**
 * This function sets up the expected C environment and then runs the null
 * process.  A major part of the expected C environment is memory
 * preparation.  The standard XINU executable consists of three main
 * segments.  (In this context, a 'segment' is a relocatable section, not
 * necessarily corresponding to virtual memory segments.)  These are:
 *  Text segment (assembled machine instructions for the program code),
 *  Data segment (initialized data), and
 *  BSS ("Block Started by Symbol") segment (uninitialized data).
 * The Stack segment provides space for a stack of activation records
 *  used to implement statically-scoped variables and recursive procedure
 *  calls in most ALGOL-like languages, including C and all its progeny.
 * The BSS is the unitialized data segment produced by the Unix linkers.
 *  Objects in the BSS segment have only a name and a size, but no value.
 * In addition, the running program conceptually also has:
 *  Stack segment (stack of activation records,)
 *  Heap segment (area for dynamic memory allocation).
 *
 * Memory Layout (Not to scale)
 * +----------+
 * |          |
 * |          |
 * |   HEAP   | -> getmem allocates from here
 * |          |
 * |          |
 * +----------+
 * | OS STACK | -> becomes the null process' stack
 * +----------+
 * |   BSS    | -> needed for C environment
 * +----------+
 * |   TEXT   | -> XINU code
 * |          |
 * +----------+
 * | RESERVED | -> interrupt handler and vectors go here
 * +----------+
 */

	/* Clear the .bss section of the kernel.  */
	ldr x0, =_bss
	ldr x1, =_end
  	mov x2, #0
  	b bssloopa
bssloopb:
  	str x2, [x0, 8]
  	str x2, [x0, 16]
  	str x2, [x0, 24]
  	str x2, [x0, 32]
  	add x0, x0, 32

bssloopa:
	cmp x0, x1 	/* check that we still haven't hit the end of bss yet */
	b.lo bssloopb	/* if still below, go backwards and loop */



	/* Put the null thread's stack directly after the kernel image.  */
	add sp, x1, #NULLSTK

	/* The remaining memory available to the ARM will be Xinu's "memheap"
	 * region, which is used for dynamic memory allocation.  Set its
	 * starting address.  */
	ldr x0, =memheap
    mov x1, sp
    str x1, [x0]

	/* Put non-used cores in busy wait */
  	mrs x7, mpidr_el1
  	and x7, x7, #3
  	cbz x7, __start_master
0:  wfe
  	b   0b

	
	/* Branch to the C environment */
__start_master:
	b	nulluser
	.endfunc




//.globl dmb

/**
 * @fn void dmb(void)
 *
 * Executes a data memory barrier operation using the c7 (Cache Operations)
 * register of system control coprocessor CP15.
 *
 * All explicit memory accesses occurring in program order before this operation
 * will be globally observed before any memory accesses occurring in program
 * order after this operation.  This includes both read and write accesses.
 *
 * This differs from a "data synchronization barrier" in that a data
 * synchronization barrier will ensure that all previous explicit memory
F
 * accesses occurring in program order have fully completed before continuing
 * and that no subsequent instructions will be executed until that point, even
 * if they do not access memory.  This is unnecessary for what we need this for.
 *
 * On the BCM2835 (Raspberry Pi), this is needed before and after accessing
 * peripherals, as documented on page 7 of the "BCM2835 ARM Peripherals"
 * document.  As documented, it is only needed when switching between
 * _different_ peripherals.
 */
/*
dmb:
	.func dmb
	mov	r12, #0
	mcr	p15, 0, r12, c7, c10, 5
	mov 	pc, lr
	.endfunc
*/
