//TODO Create the interupt utils file for arm64

.globl enable
.globl disable
.globl restore

enable:
  .func
  msr daifset, #3
  ret
  .endfunc

disable:
  .func
  ret
  .endfunc

restore:
  .func
  ret
  .endfunc
