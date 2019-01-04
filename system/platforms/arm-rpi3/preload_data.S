.globl pld
.globl pldw

pld:
	pldw	[r0]
	bx	lr

pldw:
	pldw	[r0]
	bx		lr
