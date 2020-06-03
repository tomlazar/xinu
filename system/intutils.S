.globl enable
.globl disable
.globl restore

enable:
  msr daifclr, #0b0010
  ret
  .endfunc

disable:
  .endfunc

restore:
  .endfunc
