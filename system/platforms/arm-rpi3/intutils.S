//TODO Create the interupt utils file for arm64

.globl enable
.globl disable
.globl restore

enable:
  .func enable
  msr daifclr, #0b0011
  ret
  .endfunc

disable:
  .func disable
  msr daifset, #0b0011
  ret
  .endfunc

restore:
  .func restore
  ret
  .endfunc
