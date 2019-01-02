.globl pld
.globl pldw

pld:
	pld	[r0]
	bx	lr

pldw:
	pldw	[r0]
	bx		lr
