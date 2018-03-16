#ifndef _MMU_H_
#define _MMU_H_

#define MMUTABLEBASE	0x00004000

#ifndef __ASSEMBLER__
extern void start_mmu(unsigned int);
extern void stop_mmu(void);
extern void invalidate_tlbs(void);
extern int mmu_section(unsigned int, unsigned int, unsigned int);
extern void mmu_init(void);

extern void PUT32(unsigned int, unsigned int);
extern unsigned int GET32(unsigned int);
#endif	/* __ASSEMBLER__ */

#endif	/* _MMU_H_ */
