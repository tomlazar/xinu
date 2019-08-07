#include <core.h>
#include <mmu.h>
#include <stddef.h>

void led_test(void);

extern void createnullthread(void);

extern void led_init(void);
extern void led_on(void);
extern void led_off(void);
extern void udelay(unsigned int);


void testallcores(void)
{
    unparkcore(1, (void *)createnullthread, NULL);
    unparkcore(2, (void *)createnullthread, NULL);
    unparkcore(3, (void *)createnullthread, NULL);
    led_test();
}

void led_test()
{
    led_init();
    led_off();
    while(1)
    {
	udelay(1000);
	led_on();
	udelay(1000);
	led_off();
    }
}
