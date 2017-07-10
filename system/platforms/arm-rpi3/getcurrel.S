.globl getcurrel
getcurrel:
	.func getcurrel
	mrs	x0, CurrentEL
	ret
	.endfunc
