
/**
 * @file     ctxsw.s
 * @provides ctxsw
 *
 * COSC 3250 / COEN 4820 Assignment 4
 */
/* Embedded XINU, Copyright (C) 2013.  All rights reserved. */

#include <arm.h>

.text
        .align 4
        .globl  ctxsw

/**
 * @fn void ctxsw(&oldregs, &newregs)
 *
 * Switch context (values in registers) to another process, saving the
 * current processes information. This function will not return as normally
 * thought; as it will load in the stack pointer for a different process and
 * jump to that location and begin executing code.
 *
 * @param  &oldregs address of outgoing register save area
 * @param  &newregs address of incoming register save area
 * @return special case -- see above
 */

ctxsw:

    .func ctxsw

    /* Store registers in respective places in stack.  Link register gets saved into location for stack pointer*/
    /* so when process switches back, program will continue execution in correct location*/

    mrs	   r12, cpsr
    push   {r0-r14}

    str    sp, [r0]
    ldr    sp, [r1]

    /* Load registers from respective places in stack.  Using x1 and x2, so these need to be pushed onto stack.*/
    /* Program counter must be loaded last, since program will jump.*/
    /* In this case, returning is same as loading the link register into the program counter*/
   
    pop	   {r0-r12}
    msr    cpsr_c, r12

    /* Load link register into program counter*/
    /* Not returning back to function we came from, but going to new function in different process, */
    /* since we changed value of link register*/
    pop    {lr, pc}

    .endfunc
