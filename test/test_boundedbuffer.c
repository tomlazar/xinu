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

#define	BUFFER_SIZE	10

extern void mutex_acquire(void *);
extern void mutex_release(void *);



/* these are static so that they are not visible to the outside xinu world 
 * and dont interfere with any other variable names, just in case */
static unsigned int bb_mutex = UNLOCKED;

static int buffer[BUFFER_SIZE];
static int in  = 0;
static int out = 0;


void producer()
{
	while (true)
	{

		// do nothing while buffer is full
		while (((in + 1) % BUFFER_SIZE) == out)
			;

		mutex_acquire(&bb_mutex);
		// do stuff
		mutex_release(&bb_mutex);		
	}
}

void consumer()
{
	while (true)
	{
		
		// do nothing while buffer is empty
		while (in == out)
			;

		mutex_acquire(&bb_mutex);
		// do stuff
		mutex_release(&bb_mutex);
	}
}

void print_bb_status()
{
	int i;

	while (true)
	{
		mutex_acquire(&bb_mutex);
		// print buffer & info
		kprintf("in=%d, out=%d\r\n", in, out);
		for (i = 0; i < BUFFER_SIZE; i++)
			kprintf("%d: %d\r\n", i, buffer[i]);		
		mutex_release(&bb_mutex);
	}
}
