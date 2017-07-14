//TODO Create the interupt utils file for arm64

.globl enable
.globl disable
.globl restore

enable:
  .func enable
  bx lr
  .endfunc

disable:
  .func disable
  bx lr
  .endfunc

restore:
  .func restore
  bx lr
  .endfunc
