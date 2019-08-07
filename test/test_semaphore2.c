#include <stddef.h>
#include <thread.h>
#include <semaphore.h>
#include <stdio.h>
#include <testsuite.h>
#include <core.h>

#if NSEM

extern bool test_checkSemCount(semaphore s, short c);
extern bool test_checkProcState(tid_typ tid, uchar state);
extern bool test_checkResult(uchar testResult, uchar expected);

extern void test_semWaiter(semaphore s, int times, uchar *testResult);

#endif

thread test_semaphore2(bool verbose)
{
#if NSEM
    tid_typ atid, btid, ctid, dtid;
    bool passed = TRUE;
    semaphore s;
    uchar testResult = 0;
    char msg[50];

    testPrint(verbose, "Semaphore creation: ");
    s = semcreate(0);
    if (isbadsem(s))
    {
        passed = FALSE;
        sprintf(msg, "%d", s);
        testFail(verbose, msg);
    }
    else if (test_checkSemCount(s, 0))
    {
        testPass(verbose, "");
    }
    else
    {
        passed = FALSE;
    }

    ready(atid =
          create((void *)test_semWaiter, INITSTK, 32,
                 "SEMAPHORE-A", 3, s, 1, &testResult), RESCHED_NO, CORE_ZERO);
    ready(btid =
          create((void *)test_semWaiter, INITSTK, 31,
                 "SEMAPHORE-B", 3, s, 1, &testResult), RESCHED_NO, CORE_ZERO);
    ready(ctid =
          create((void *)test_semWaiter, INITSTK, 32,
                 "SEMAPHORE-C", 3, s, 1, &testResult), RESCHED_NO, CORE_ONE);
    ready(dtid =
          create((void *)test_semWaiter, INITSTK, 31,
                 "SEMAPHORE-D", 3, s, 1, &testResult), RESCHED_NO, CORE_ONE);

    /* Both processes should run and immediately wait.  A and C should wait first
     * because they have higher priority.  */
    yield();
    //ready(create((void *)yield, INITSTK, 99, "YieldCore1", 0), RESCHED_YES, CORE_ONE);
    udelay(9);
    
    testPrint(verbose, "Wait on semaphore: ");
    
    if (test_checkProcState(atid, THRWAIT)
        && test_checkProcState(btid, THRWAIT)
        && test_checkSemCount(s, -4) && test_checkResult(testResult, 0))
    {
        testPass(verbose, "");
    }
    else
    {
        passed = FALSE;
    }

    signal(s);

    /* Process A and C have the same priority, so a signal should release one, then the other. */
    testPrint(verbose, "Signal first semaphore: ");
    if (test_checkProcState(atid, THRFREE) 
		&& test_checkProcState(ctid, THRWAIT)
        && test_checkProcState(btid, THRWAIT)
        && test_checkProcState(dtid, THRWAIT)
        && test_checkSemCount(s, -3) && test_checkResult(testResult, 1))
    {
		testPass(verbose, "A 1");
		signal(s);
		testPrint(verbose, "Signal second semaphore: ");
		if (test_checkProcState(ctid, THRFREE)
			&& test_checkProcState(btid, THRWAIT)
			&& test_checkProcState(dtid, THRWAIT)
			&& test_checkSemCount(s, -2) && test_checkResult(testResult, 2))
		{
			testPass(verbose, "C 2");
			signal(s);
			testPrint(verbose, "Signal third semaphore: ");
			if (test_checkProcState(btid, THRFREE)
				&& test_checkProcState(dtid, THRWAIT)
				&& test_checkSemCount(s, -1) && test_checkResult(testResult, 3))
			{
				testPass(verbose, "B 3");
				signal(s);
				testPrint(verbose, "Signal fourth semaphore: ");
				if (test_checkProcState(dtid, THRFREE)
					&& test_checkSemCount(s, 0) && test_checkResult(testResult, 4))
				{
					testPass(verbose, "D 4"); /* runs A, C, B, D */
				}	
				else
				{
					passed = FALSE;
				}
			}
			else if (test_checkProcState(dtid, THRFREE)
				&& test_checkProcState(btid, THRWAIT)
				&& test_checkSemCount(s, -1) && test_checkResult(testResult, 3))
			{
				testPass(verbose, "D 3");
				signal(s);
				testPrint(verbose, "Signal fourth semaphore: ");
				if (test_checkProcState(btid, THRFREE)
					&& test_checkSemCount(s, 0) && test_checkResult(testResult, 4))
				{
					testPass(verbose, "B 4"); /* runs A, C, D, B */
				}
				else
				{
					passed = FALSE;
				}
			}
			else
			{
				passed = FALSE;
			}
		}
		else if (test_checkProcState(btid, THRFREE)
			&& test_checkProcState(ctid, THRWAIT)
			&& test_checkProcState(dtid, THRWAIT)
			&& test_checkSemCount(s, -2) && test_checkResult(testResult, 2))
		{
			testPass(verbose, "B 2");
			signal(s);
			testPrint(verbose, "Signal third semaphore: ");
			if (test_checkProcState(ctid, THRFREE)
			&& test_checkProcState(dtid, THRWAIT)
			&& test_checkSemCount(s, -1) && test_checkResult(testResult, 3))
			{
				testPass(verbose, "C 3");
			}
			else 
			{
				passed = FALSE;
			}
			signal(s);
			testPrint(verbose, "Signal fourth semaphore: ");
			if (test_checkProcState(dtid, THRFREE)
				&& test_checkSemCount(s, 0) && test_checkResult(testResult, 4))
			{
				testPass(verbose, "D 4"); /* runs A, B, C, D */
			}
			else
			{
				passed = FALSE;
			}
		}
		else
		{
			passed = FALSE;
		}
    } 
    else if (test_checkProcState(ctid, THRFREE) 
		&& test_checkProcState(atid, THRWAIT)
        && test_checkProcState(btid, THRWAIT)
        && test_checkProcState(dtid, THRWAIT)
        && test_checkSemCount(s, -3) && test_checkResult(testResult, 1))
    {
		testPass(verbose, "C 1");
		signal(s);
		testPrint(verbose, "Signal second semaphore: ");
		if (test_checkProcState(atid, THRFREE)
			&& test_checkProcState(btid, THRWAIT)
			&& test_checkProcState(dtid, THRWAIT)
			&& test_checkSemCount(s, -2) && test_checkResult(testResult, 2))
		{
			testPass(verbose, "A 2");
			signal(s);
			testPrint(verbose, "Signal third semaphore: ");
			if (test_checkProcState(btid, THRFREE)
				&& test_checkProcState(dtid, THRWAIT)
				&& test_checkSemCount(s, -1) && test_checkResult(testResult, 3))
			{
				testPass(verbose, "B 3");
				signal(s);
				testPrint(verbose, "Signal fourth semaphore: ");
				if (test_checkProcState(dtid, THRFREE)
					&& test_checkSemCount(s, 0) && test_checkResult(testResult, 4))
				{
					testPass(verbose, "D 4"); /* runs C, A, B, D */
				}	
				else
				{
					passed = FALSE;
				}
			}
			else if (test_checkProcState(dtid, THRFREE)
				&& test_checkProcState(btid, THRWAIT)
				&& test_checkSemCount(s, -1) && test_checkResult(testResult, 3))
			{
				testPass(verbose, "D 3");
				signal(s);
				testPrint(verbose, "Signal fourth semaphore: ");
				if (test_checkProcState(btid, THRFREE)
					&& test_checkSemCount(s, 0) && test_checkResult(testResult, 4))
				{
					testPass(verbose, "B 4"); /* runs C, A, D, B */
				}
				else
				{
					passed = FALSE;
				}
			}
			else
			{
				passed = FALSE;
			}
		}
		else if (test_checkProcState(dtid, THRFREE)
			&& test_checkProcState(atid, THRWAIT)
			&& test_checkProcState(btid, THRWAIT)
			&& test_checkSemCount(s, -2) && test_checkResult(testResult, 2))
		{
			testPass(verbose, "D 2");
			signal(s);
			testPrint(verbose, "Signal third semaphore: ");
			if (test_checkProcState(atid, THRFREE)
			&& test_checkProcState(btid, THRWAIT)
			&& test_checkSemCount(s, -1) && test_checkResult(testResult, 3))
			{
				testPass(verbose, "A 3");
			}
			else 
			{
				passed = FALSE;
			}
			signal(s);
			testPrint(verbose, "Signal fourth semaphore: ");
			if (test_checkProcState(btid, THRFREE)
				&& test_checkSemCount(s, 0) && test_checkResult(testResult, 4))
			{
				testPass(verbose, "B 4"); /* runs C, D, A, B */
			}
			else
			{
				passed = FALSE;
			}
		}
		else
		{
			passed = FALSE;
		}
    }
    else
    {
        passed = FALSE;
    }

    if (TRUE == passed)
    {
        testPass(TRUE, "");
    }
    else
    {
        testFail(TRUE, "");
    }

    /* Processes should be dead, but in case the test failed. */
    kill(atid);
    kill(btid);
	kill(ctid);
	kill(dtid);
    semfree(s);

#else /* NSEM */
    testSkip(TRUE, "");
#endif /* NSEM == 0 */
    return OK;
}
