#include <core.h>
#include <mmu.h>

void led_test(void);

extern void createnullthread(void);

extern void init_led(void);
extern void led_on(void);
extern void led_off(void);
extern void udelay(unsigned int);


void testallcores(void)
{
	unparkcore(1, (void *)createnullthread);
	unparkcore(2, (void *)createnullthread);
	unparkcore(3, (void *)createnullthread);
	led_test();
}

void led_test()
{
	init_led();
	led_off();
	while(1)
	{
		udelay(1000);
		led_on();
		udelay(1000);
		led_off();
	}
}

