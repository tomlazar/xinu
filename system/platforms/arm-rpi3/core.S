/* 2017 Embedded Xinu Team
 *
 * core.S - Contains assembly functions which start cores 1-3
 * at their accessible mailbox addresses (same as Pi2)
 *
 */

.globl PUT32
PUT32:
    str r1,[r0]
    bx lr

.globl GET32
GET32:
    ldr r0,[r0]
    bx lr

.globl showcpu0
showcpu0:
    mrc p15, 0, r0, c0, c0, 5
    bx lr

.globl start1
start1:
    ldr r0,=start_cpu1
    mov r1,#0x40000000
    str r0,[r1,#0x9C]
    bx lr

start_cpu1:
    mrc p15, 0, r0, c0, c0, 5
    mov r1,#0x40
    str r0,[r1]
    b start_cpu1

.globl start2
start2:
    ldr r0,=start_cpu2
    mov r1,#0x40000000
    str r0,[r1,#0xAC]
    bx lr

start_cpu2:
    mrc p15, 0, r0, c0, c0, 5
    mov r1,#0x40
    str r0,[r1]
    b start_cpu2

.globl start3
start3:
    ldr r0,=start_cpu3
    mov r1,#0x40000000
    str r0,[r1,#0xBC]
    bx lr

start_cpu3:
    mrc p15, 0, r0, c0, c0, 5
    mov r1,#0x40
    str r0,[r1]
    b start_cpu3
