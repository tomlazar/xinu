#ifndef _MMU_H_
#define _MMU_H_

#define MMUTABLEBASE	0x00004000

#ifndef __ASSEMBLER__

/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0201d/ch03s03s05.html */
extern void mmu_initialize(void);
extern void start_mmu(unsigned int);
extern void _clean_cache(void);
extern void stop_mmu(void);
extern void invalidate_tlbs(void);
extern unsigned int mmu_section(unsigned int, unsigned int, unsigned int);
extern void mmu_init(void);

extern void PUT32(unsigned int, unsigned int);
extern unsigned int GET32(unsigned int);

#endif	/* __ASSEMBLER__ */

#endif	/* _MMU_H_ */
