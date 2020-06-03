#include <random.h>

void random_init()
{
	*RNG_STATUS = 0x40000;
	*RNG_INT_MASK |= 1;			// mask interupts
	*RNG_CTRL |= 1;				// enable
	// wait for it to gain entropy
	while(!((*RNG_STATUS)>>24)) asm volatile("nop");
}

unsigned int random()
{
//	return *RNG_DATA % (max - min) + min;
	return (unsigned int) *RNG_DATA;
}
