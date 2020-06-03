/**
 * @file: 		test_boundedbuffer.c
 * @provides	test_boundedbuffer
 * 
 * @usage		A producer-consumer problem test for the mutex for multicore rpi3
 * 
 * This code is straight from Dinosaur Book, essentially... slightly modified
 */

/* producer() runs on one core,
 * consumer() runs on another core,
 * and print_bb_status() runs on a third core,
 * effectively each acting as their own separate "process" */

#include <mutex.h>
#include <mmu.h>
#include <clock.h>

#define	BUFFER_SIZE	10

void test_boundedbuffer(void);
extern void unparkcore(unsigned int, void *);
extern void led_test(void);
extern void udelay(unsigned long);
extern void pld(unsigned int *);
extern void dsb(void);
static void producer(void);
static void consumer(void);
static void print_bb_status(void);

/* these are static so that they are not visible to the outside xinu world 
 * and dont interfere with any other variable names, just in case */
static mutex_t bb_mutex = MUTEX_UNLOCKED;

static unsigned int buffer[BUFFER_SIZE];
static unsigned int in  = 0;
static unsigned int out = 0;

void test_boundedbuffer()
{
	kprintf("---------------------------------\r\n");
	kprintf("\tBOUNDED BUFFER TEST\r\n");
	kprintf("---------------------------------\r\n");
	unparkcore(1, (void *) producer);
	unparkcore(2, (void *) consumer);
	unparkcore(3, (void *) print_bb_status);
	led_test();
}

static void producer()
{

	udelay(2500);
	kprintf("producer() starting...\r\n");

	unsigned int next_produced = 1;
	while (1)
	{

		// do nothing while buffer is full
		while (((in + 1) % BUFFER_SIZE) == out)
			pld(&out);

		udelay(1000 + (clkticks % 1000));		// using clock to generate "random" durations of delay	
		
		mutex_acquire(&bb_mutex);
		// do stuff	
		
		buffer[in] = next_produced;
		in = (in + 1) % BUFFER_SIZE;
		next_produced = (next_produced + 1) % BUFFER_SIZE;

		dsb();		
		
		mutex_release(&bb_mutex);		
	}
}

static void consumer()
{
	udelay(2500);
	kprintf("consumer() starting...\r\n");

	unsigned int consumed;
	while (1)
	{

		// do nothing while buffer is empty
		// 
		// preloadData is necessary otherwise cache coherency becomes an issue
		// aka consumer() will read "outdated" value of "in" variable and will get stuck in loop
		while (in == out)
			pld(&in);

		udelay(1000 + (clkticks % 1000));		// using clock to generate "random" durations of delay	

		mutex_acquire(&bb_mutex);
		
		consumed = buffer[out];
		consumed += 1;
		out = (out + 1) % BUFFER_SIZE;

		// for cache coherency issue mentioned above..
		dsb();
		
		// consume item now...

		mutex_release(&bb_mutex);
	}
}

static void print_bb_status()
{

	int i;
	while (1)
	{

		udelay(500);

		mutex_acquire(&bb_mutex);
		// print buffer & info
		kprintf("in=%d, out=%d\r\n", in, out);
		for (i = 0; i < BUFFER_SIZE; i++)
			kprintf("%d: %d\r\n", i, buffer[i]);		
		mutex_release(&bb_mutex);
	}
}
