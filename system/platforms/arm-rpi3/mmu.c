#include <mmu.h>

/* code from Github user dwelch67 */
int mmu_section(unsigned int vadd, unsigned int padd, unsigned int flags)
{
	unsigned int ra, rb, rc;

	ra = vadd >> 20;
	rb = MMUTABLEBASE | (ra << 2);
	rc = (padd & 0xFFF00000) | 0xC00 | flags | 2;
	PUT32(rb, rc);

	return 0;	
}

/* mmu_init() configures virtual address == physical address */
/* also configures memory to be cacheable, except for peripheral portion */
void mmu_init()
{
	unsigned int ra;

	for (ra = 0; ; ra += 0x00100000)
	{
		mmu_section(ra, ra, 0x15C06);
		if (ra >= 0x3F000000)
			break;	/* stop before IO peripherals, dont want cache on those... */
	}

	// peripherals
	for ( ; ; ra += 0x00100000)
	{
		mmu_section(ra, ra, 0x0000);
		if (ra == 0xFFF00000)
			break;
	}

	start_mmu(MMUTABLEBASE);
}
