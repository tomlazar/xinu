/**
 * @file start.S
 *
 * Initialization code for Embedded Xinu on the Raspberry Pi.
 */

#define IO_BASE 0x3f000000
#define GP_BASE 0x3f200000
#define GPFSEL1 0x3f200004
#define GPSET0  0x3f20001C

/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */


#include <arm.h>     /* For ARM_MODE_SYS */

#define NULLSTK 8192

.section .init
	.globl _start

	/* _start:  Entry point of the Xinu kernel.  This will be the very first
	 * byte of the kernel image and on the Raspberry Pi will be loaded by
	 * the GPU at address 0x8000.  */
	.func _start
_start:
	/* Save the pointer to the atags (ARM boot tags).  The bootloader should
	 * pass this in r2, but override 0 with 0x100 to deal with old
	 * bootloaders that assume Linux's default behavior.  (This is the case
	 * with, for example, the bootloader on the BCM2835.)  */
	ldr 	x3, =atags_ptr
	cmp 	x2, #0
	b.ne 	_sne
	mov 	x2, #0x100
_sne:
	str 	x2, [x3]

	/* Continue execution at reset_handler.  */
	b 	reset_handler
	.endfunc

/* ARM exception vector table.  This is copied to address 0.  See A2.6
 * "Exceptions" of the ARM Architecture Reference Manual.  */
//_vectors:
//	ldr pc, reset_addr    /* Reset handler */
//	ldr pc, undef_addr	  /* Undefined instruction handler */
//	ldr pc, swi_addr	  /* Software interrupt handler */
//	ldr pc, prefetch_addr /* Prefetch abort handler */
//	ldr pc, abort_addr	  /* Data abort handler */
//	ldr pc, reserved_addr /* Reserved */
//	ldr pc, irq_addr	  /* IRQ (Interrupt request) handler */
//	ldr pc, fiq_addr	  /* FIQ (Fast interrupt request) handler */

reset_addr:     .word reset_handler
undef_addr:     .word reset_handler
swi_addr:       .word reset_handler
prefetch_addr:  .word reset_handler
abort_addr:     .word reset_handler
reserved_addr:  .word reset_handler
irq_addr:       .word irq_handler
fiq_addr:       .word reset_handler

_endvectors:

.section .text

	/* reset_handler: Reset handler routine executed to start up the kernel,
	 * when the ARM processor is reset, or (currently) when an unhandled
	 * exception occurs.  */
	.func reset_handler
reset_handler:

	/* Enter SYS mode with all interrupts disabled so the ARM processor is
	 * in a known state. */
//	cpsid if, #ARM_MODE_SYS

	/* Copy the ARM exception table from .init section to address 0,
	 * including the absolute address table.  Here we assume it is exactly
	 * 16 words.  */
/*	mov 	r0, #0
	ldr 	r1, =_vectors
	ldmia 	r1!, {r2-r9}
	stmia 	r0!, {r2-r9}
	ldmia 	r1!, {r2-r9}
	stmia 	r0!, {r2-r9}
*/
	/* Enable the following features by modifying the ARM processor's
	 * Control Register:
	 *
	 * - unaligned memory accesses   (bit 22)
	 *
	 * Note: Xinu shouldn't do any unaligned memory accesses on purpose, but
	 * we found that gcc can still generate unaligned memory accesses for
	 * tasks such as copying character strings.  (An alternative would be to
	 * pass -mno-unaligned-access to gcc to prevent it from doing so.)  */
/*	mrc p15, 0, r0, c1, c0, 0
	orr r0, #1 << 22
	mcr p15, 0, r0, c1, c0, 0
*/
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

	/* Initialize the Raspberry Pi graphics subsystem.  */
	/* bl screenInit */

	/* Branch to the platform-independent C startup code, which takes no
	 * parameters and never returns.  */

/*
  mov   x0, 4
  movk  x0, 0x3f20, lsl 16
  mov   x1, 4
  movk  x1, 0x3f20, lsl 16
  ldr   w1, [x1]
  and   w1, w1, -1835009
  str   w1, [x0]
  mov   x0, 4
  movk  x0, 0x3f20, lsl 16
  mov   x1, 4
  movk  x1, 0x3f20, lsl 16
  ldr   w1, [x1]
  orr   w1, w1, 262144
  str   w1, [x0]
  mov   x0, 28
  movk  x0, 0x3f20, lsl 16
  mov   w1, 65536
  str   w1, [x0]
*/
  mrs x7, mpidr_el1
  and x7, x7, #3
  cbz x7, __start_master
0:  wfe
  b   0b


__start_master:
	//bl  platforminit
	//bl	init_led
	//bl	led_on
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
