/**
 * @file mmu.c
 *
 * Define functions for enabling and configuring the MMU.
 *
 * @authors Rade Latinovich
 * 	    Patrick J. McGee
 */
/* Embedded Xinu, Copyright (C) 2009. All rights reserved */

#include <mmu.h>
#include <mutex.h>
#include <dma_buf.h>

/**
 * @ingroup bcm2837
 *
 * Mark a section of memory as cacheable, given flags
 * @param vadd		Virtual address
 * @param padd		Physical address
 * @param flags		Flag to mark the section
 * @return Zero
 */
/* code from Github user dwelch67
 * https://github.com/dwelch67/raspberrypi/tree/master/mmu */
unsigned int mmu_section(unsigned int vadd, unsigned int padd, unsigned int flags)
{
	unsigned int ra, rb, rc;

	ra = vadd >> 20;
	rb = MMUTABLEBASE | (ra << 2);
	rc = (padd & 0xFFF00000) | 0xC00 | flags | 2;
	PUT32(rb, rc);

	return 0;	
}

/**
 * @ingroup bcm2837
 *
 * mmu_init() configures virtual address == physical address
 * also configures memory to be cacheable, except for peripheral portion 
 */
void mmu_init()
{
	unsigned int ra;
	for (ra = 0; ; ra += 0x00100000)
	{
		mmu_section(ra, ra, 0x15C06);
		//mmu_section(ra, ra, 0x0 | 0x8);
		if (ra >= 0x3F000000)
			break;
	}

	/* Peripherals not marked (use 0x0000) */
	for ( ; ; ra += 0x00100000)
	{
		mmu_section(ra, ra, 0x0000);
		if (ra == 0x40000000)
			break;
	}

	// make dma buffer area non-cacheable
	mmu_section(dma_buf_space, dma_buf_space, 0x0);

	start_mmu(MMUTABLEBASE);
}
