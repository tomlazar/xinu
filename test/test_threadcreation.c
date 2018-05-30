
/**
 * @file tset_processcreation.c
 */
/* Embedded XINU, Copyright (C) 2007.  All rights reserved. */


#include <thread.h>
#include <arm.h>

extern void main(int, char *);
void testproc(void);
void printtid(int tid);
void testbigargs(int, int, int, int, int, int, int, int, int);
void test_thr_core(void);
void corejob_t1(void);
void corejob_t2(void);
void corejob_t3(void);
void testmain(void);


void testproc(void)
{
	// Test job for the process to handle
	kprintf("\r\nTesting random job...\r\n");
	kprintf("Current running thread: %d\r\n\r\n", thrcurrent);
	kprintf("	               ___          \r\n");
	kprintf("	             _//_||         \r\n");
	kprintf("	           ,'    //'.       \r\n");
	kprintf("	          /          )      \r\n");
	kprintf("	        _/           |      \r\n");
	kprintf("	       (.-,--.       |      \r\n");
	kprintf("	       (o/  o |     /       \r\n");
	kprintf("	       |_|    /  /|/|       \r\n");
	kprintf("	       (__`--'   ._)        \r\n");
	kprintf("	       /  `-.     |         \r\n");
	kprintf("	      (     ,`-.  |         \r\n");
	kprintf("	       `-,--|_  ) |-.       \r\n");
	kprintf("	        _`.__.'  ,-' )      \r\n");
	kprintf("	       || )  _.-'    |      \r\n");
	kprintf("	       i-|.'|     ,--+.     \r\n");
	kprintf("	     .' .'   |,-'/     )    \r\n");
	kprintf("	    / /         /       )   \r\n");
	kprintf("	    7_|         |       |   \r\n");
	kprintf("	    |/          '(.___.;'   \r\n");
	kprintf("	    /            |     |)   \r\n");
	kprintf("	   /             |     | )  \r\n");
	kprintf("	  /              |     |  | \r\n");
	kprintf("	  |              |     |  | \r\n");
	kprintf("	  |____          |     |-;' \r\n");
	kprintf("	   |   """"----""|     | |  \r\n");
	kprintf("	   (           ,-'     |/   \r\n");
	kprintf("	    `.         `-,     |    \r\n");
	kprintf("	     |`-._      / /| |} )   \r\n");
	kprintf("	     |    `-.   `' | ||`-'  \r\n");
	kprintf("	     |      |      `-'|     \r\n");
	kprintf("	     |      |         |     \r\n");
	kprintf("	     |      |         |     \r\n");
	kprintf("	     |      |         |     \r\n");
	kprintf("	     |      |         |     \r\n");
	kprintf("	     |      |         |     \r\n");
	kprintf("	     |      |         |     \r\n");
	kprintf("	     )`-.___|         |     \r\n");
	kprintf("	   .'`-.____)`-.___.-'(     \r\n");
	kprintf("	 .'        .'-._____.-|     \r\n");
	kprintf("	/        .'           |     \r\n");
	kprintf("	`-------/         .   |     \r\n");
	kprintf("	        `--------' '--'     \r\n");
}

void testmain()
{
	int tid;
	kprintf("\r\n------------------------------- This is thread 1 -------------------------------\r\n");
	tid=create((void *) testproc, INITSTK, INITPRIO, "THREAD 1", 0, NULL);
	ready(tid, 0);
	kprintf("Done readying thread: %d\r\n", tid);
	printtid(tid);

	kprintf("\r\n------------------------------- This is thread 2 -------------------------------\r\n");
	tid=create((void *) testbigargs, INITSTK, INITPRIO, "THREAD 2", 9, 1, 2, 3, 4, 5, 6, 7, 8, 9);
	ready(tid, 0);
	kprintf("Done readying thread: %d\r\n", tid);
	printtid(tid);

	kprintf("\r\n------------------------------- This is thread 3 -------------------------------\r\n");
	tid=create((void *) testbigargs, INITSTK, INITPRIO, "THREAD 3", 9, 9, 8, 7, 6, 5, 4, 3, 2, 1);
	ready(tid, 0);
	kprintf("Done readying thread: %d\r\n", tid);
	printtid(tid);

#if 0
	while(1)
	{
		kprintf("Rescheding...\r\n");
		resched(); 
		kprintf("Resched complete.\r\n");
	}
#endif
}

void corejob_t1(void){

	kprintf("***");

}

void corejob_t2(void){

	kprintf("+++");

}

void corejob_t3(void){

	kprintf("---");

}

void test_thr_core(){
	int tidc;
	tidc=create((void *) corejob_t1, INITSTK, 1, "THREAD 3", 9, 9, 8, 7, 6, 5, 4, 3, 2, 1);
	ready(tidc, 0);

	tidc=create((void *) corejob_t2, INITSTK, 2, "THREAD 3", 9, 9, 8, 7, 6, 5, 4, 3, 2, 1);
	ready(tidc, 0);

	tidc=create((void *) corejob_t3, INITSTK, 3, "THREAD 3", 9, 9, 8, 7, 6, 5, 4, 3, 2, 1);
	ready(tidc, 0);



	while(1)
	{
		kprintf("Rescheding...\r\n");
		resched();
		kprintf("Resched complete.\r\n");
	}

}

void testbigargs(int a, int b, int c, int d, int e, int f, int g, int h, int i)
{
	kprintf("Current running thread: %d\r\n", thrcurrent);
	kprintf("Testing bigargs...\r\n");
	kprintf("a = 0x%08X\r\n", a);
	kprintf("b = 0x%08X\r\n", b);
	kprintf("c = 0x%08X\r\n", c);
	kprintf("d = 0x%08X\r\n", d);
	kprintf("e = 0x%08X\r\n", e);
	kprintf("f = 0x%08X\r\n", f);
	kprintf("g = 0x%08X\r\n", g);
	kprintf("h = 0x%08X\r\n", h);
	kprintf("i = 0x%08X\r\n", i);
}
void printtid(int tid)
{
	struct thrent *tthrent = NULL;

	/* Using the Thread ID, access it in the TID table. */
	tthrent = &thrtab[tid];

	/* Printing TID */
	kprintf("TID: %d\r\n", tid);
	kprintf("\tName:\t%s\r\n", tthrent->name);
	kprintf("\tState:\t%d\r\n", tthrent->state);

#if 0
	/* Print TID contents and registers */
	kprintf("Base of run time stack   : 0x%08X \r\n", tthrent->stkbase);
	kprintf("Stack pointer of thread  : 0x%08X \r\n",
			tthrent->stkptr);
	kprintf("Stack length of thread   : %8u\r\n", tthrent->stklen);
#endif
}
