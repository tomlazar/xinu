/**
 * @file: 		test_boundedbuffer.c
 * @provides	consumer
 * 				producer
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

extern void mutex_acquire(void *);
extern void mutex_release(void *);

extern void unparkcore(unsigned int, void *);
extern void led_test(void);

extern void udelay(unsigned long);

static void producer(void);
static void consumer(void);
static void print_bb_status(void);

/* these are static so that they are not visible to the outside xinu world 
 * and dont interfere with any other variable names, just in case */
static unsigned int bb_mutex = UNLOCKED;

static int buffer[BUFFER_SIZE];
static int in  = 0;
static int out = 0;


void test_boundedbuffer()
{
	unparkcore(1, (void *) producer);
	unparkcore(2, (void *) consumer);
	unparkcore(3, (void *) print_bb_status);
	led_test();
}

static void producer()
{
	// necessary since this is the first thing running on the core
	start_mmu(MMUTABLEBASE, 0x1 | 0x1000 | 0x4);

	udelay(5000);
	kprintf("producer() starting...\r\n");

	unsigned int next_produced = 0;
	while (1)
	{

		// do nothing while buffer is full
		while (((in + 1) % BUFFER_SIZE) == out)
			;

		udelay(1000 + (clkticks % 1000));		// using clock to generate "random" durations of delay	
		
		mutex_acquire(&bb_mutex);
		// do stuff	
		
		buffer[in] = next_produced;
		in = (in + 1) % BUFFER_SIZE;
		next_produced = (next_produced + 1) % BUFFER_SIZE;
		
		mutex_release(&bb_mutex);		
	}
}

static void consumer()
{
	start_mmu(MMUTABLEBASE, 0x1 | 0x1000 | 0x4);

	udelay(5000);
	kprintf("consumer() starting...\r\n");

	unsigned int consumed;
	while (1)
	{

		kprintf("consumer(): before while\r\n");		
		// do nothing while buffer is empty
		while (in == out)
			;
		kprintf("consumer(): after while\r\n");

		udelay(1000 + (clkticks % 1000));		// using clock to generate "random" durations of delay	

		kprintf("consumer(): before mutex_acquire\r\n");		
		mutex_acquire(&bb_mutex);
		// do stuff
		kprintf("consumer(): mutex acquired\r\n");
		consumed = buffer[out];
		out = (out + 1) % BUFFER_SIZE;
		//consume the item now...

		mutex_release(&bb_mutex);
	}
}

static void print_bb_status()
{
	start_mmu(MMUTABLEBASE, 0x1 | 0x1000 | 0x4);	

	int i;
	while (1)
	{

		udelay(1000);

		mutex_acquire(&bb_mutex);
		// print buffer & info
		kprintf("in=%d, out=%d\r\n", in, out);
		for (i = 0; i < BUFFER_SIZE; i++)
			kprintf("%d: %d\r\n", i, buffer[i]);		
		mutex_release(&bb_mutex);
	}
}

