#include <core.h>
void testcore1(void);
void testcore2(void);
void testcore3(void);
void testcore4(void);
void testallcores(void);

int semaphore = 0;

extern unsigned int serial_lock;
extern void mutex_acquire(void *);
extern void mutex_release(void *);

void testallcores(void)
{
	unparkcore(1, (void *)testcore1);
	unparkcore(2, (void *)testcore2);
	unparkcore(3, (void *)testcore3);
}

void testcore1(void)
{
	while(1)
		kprintf("dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\r\n");
}

void testcore2(void)
{
	while(1)
		kprintf("22222222222222222222222222222222\r\n");
}

void testcore3(void)
{
	while(1)
		kprintf("33333333333333333333333333333333\r\n");
}

void testcore4(void)
{
	while(1)
		kprintf("44444444444444444444444444444444\r\n");
}
