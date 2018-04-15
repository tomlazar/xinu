#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <bcm2837.h>

#define RNG_CTRL        ((volatile unsigned int*)(PERIPHERALS_BASE+0x00104000))
#define RNG_STATUS      ((volatile unsigned int*)(PERIPHERALS_BASE+0x00104004))
#define RNG_DATA        ((volatile unsigned int*)(PERIPHERALS_BASE+0x00104008))
#define RNG_INT_MASK    ((volatile unsigned int*)(PERIPHERALS_BASE+0x00104010))

extern void random_init(void);
extern unsigned int random(void);

#endif 	/* _RANDOM_H_ */
