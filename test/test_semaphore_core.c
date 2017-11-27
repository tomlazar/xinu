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

extern void start_mmu(unsigned int, unsigned int);
extern void createnullthread(void);

#define MMUTABLEBASE	0x00004000

void testallcores(void)
{
	unparkcore(1, (void *)createnullthread);
	unparkcore(2, (void *)createnullthread);
	unparkcore(3, (void *)createnullthread);
}

void testcore1(void)
{
	start_mmu(MMUTABLEBASE, 0x1 | 0x1000 | 0x4);
	while(1)
		kprintf("dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\r\n");
}

void testcore2(void)
{
	start_mmu(MMUTABLEBASE, 0x1 | 0x1000 | 0x4);
	while(1)
		kprintf("22222222222222222222222222222222\r\n");
}

void testcore3(void)
{
	start_mmu(MMUTABLEBASE, 0x1 | 0x1000 | 0x4);
	while(1)
		kprintf("33333333333333333333333333333333\r\n");
}

void testcore4(void)
{
	while(1)
		kprintf("44444444444444444444444444444444\r\n");
}
