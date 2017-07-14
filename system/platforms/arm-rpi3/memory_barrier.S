//TODO create the memory barrier for arm64

.globl dmb

dmb:
  .func dmb

  bx lr
  .endfunc
