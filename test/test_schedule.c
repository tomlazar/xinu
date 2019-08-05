#include <stddef.h>
#include <thread.h>
#include <stdio.h>
#include <testsuite.h>
#include <core.h>
#include <thread.h>

#define TIMES 5

static void t4(int times, uchar *testArray, int *shared)
{
    int i = 0;

    /* Disable interrupts so we execute in a known order.
     * NOTE: even when doing this, this test is still subject to a race
     * condition where the running thread may be replaced with an equal-priority
     * thread before reaching the call to disable() below.  As a result, it is
     * unpredictable whether thread B or thread D will write to the array first.
     * But regardless of which one does, they should alternate back and forth.
     * */
    disable();
    int cpuid = getcpuid();
    for (i = 0; i < times; i++)
    {
	resched();
	cpuid = getcpuid();
	testArray[*shared] = thrcurrent[cpuid];
	*shared = *shared + 1;
        yield();
    }
}

thread test_schedule(bool verbose)
{
    char str[50];
    uchar testArray[4 * TIMES];
    uchar expectedResults[4 * TIMES];
    int shared = 0;
    tid_typ atid, btid, ctid, dtid, i;
    int btid_remaining, dtid_remaining;
    bool passed = TRUE;

    /* disable interrupts so we execute in known order */
    disable();
    ready(atid = create((void *)t4, INITSTK, 31, "PRIORITY-A",
                        3, TIMES, testArray, &shared), 0, CORE_ZERO);
    ready(btid = create((void *)t4, INITSTK, 32, "PRIORITY-B",
                        3, TIMES, testArray, &shared), 0, CORE_ZERO);
    ready(ctid = create((void *)t4, INITSTK, 34, "PRIORITY-C",
                        3, TIMES, testArray, &shared), 0, CORE_ZERO);
    ready(dtid = create((void *)t4, INITSTK, 32, "PRIORITY-D",
                        3, TIMES, testArray, &shared), 0, CORE_ZERO);

    /* Run the tests by yielding the processor */
    yield();

    /* Generate expected results and compare */

    for (i = 0; i < TIMES; i++)
    {
        expectedResults[i] = ctid;
    }

    /* Since threads may be rescheduled before they can disable interrupts, the
     * exact order than threads B and D, which have equal priority, execute in
     * is unpredictable.  To avoid spurious test failures, we accept them
     * executing in any order.  */
    btid_remaining = TIMES;
    dtid_remaining = TIMES;
    for (i = TIMES; i < 3 * TIMES; i++)
    {
        if (testArray[i] == btid && btid_remaining > 0) {
            expectedResults[i] = btid;
            btid_remaining--;
        } else {
            expectedResults[i] = dtid;
            dtid_remaining--;
        }
    }

    for (i = 3 * TIMES; i < 4 * TIMES; i++)
    {
        expectedResults[i] = atid;
    }

    for (i = 0; i < 4 * TIMES; i++)
    {
	kprintf("\r");
	kprintf("\r\nExpectedResults[%d]: %d (%s), TestResults[%d]: %d", i, expectedResults[i], thrtab[i].name, i, testArray[i]);
        if (expectedResults[i] != testArray[i])
        {
            passed = FALSE;
            sprintf(str,
                    "Expected testArray[%d] == %d, not %d\n",
                    i, expectedResults[i], testArray[i]);
            testFail(verbose, str);
        }
    }

    /* reset processes */
    kill(atid);
    kill(btid);
    kill(ctid);
    kill(dtid);
    shared = 0;

    /* run test on core 1 */
    ready(atid = create((void *)t4, INITSTK, 31, "PRIORITY-A",
                        3, TIMES, testArray, &shared), 0, CORE_ONE);
    ready(btid = create((void *)t4, INITSTK, 32, "PRIORITY-B",
                        3, TIMES, testArray, &shared), 0, CORE_ONE);
    ready(ctid = create((void *)t4, INITSTK, 34, "PRIORITY-C",
                        3, TIMES, testArray, &shared), 0, CORE_ONE);
    ready(dtid = create((void *)t4, INITSTK, 32, "PRIORITY-D",
                        3, TIMES, testArray, &shared), 0, CORE_ONE);

    /* Run the tests by yielding the processor */
    yield();

    /* Generate expected results and compare */

    for (i = 0; i < TIMES; i++)
    {
        expectedResults[i] = ctid;
    }

    /* Since threads may be rescheduled before they can disable interrupts, the
     * exact order than threads B and D, which have equal priority, execute in
     * is unpredictable.  To avoid spurious test failures, we accept them
     * executing in any order.  */
    btid_remaining = TIMES;
    dtid_remaining = TIMES;
    udelay(9);
    for (i = TIMES; i < 3 * TIMES; i++)
    {
        if (testArray[i] == btid && btid_remaining > 0) {
            expectedResults[i] = btid;
            btid_remaining--;
        } else {
            expectedResults[i] = dtid;
            dtid_remaining--;
        }
    }

    for (i = 3 * TIMES; i < 4 * TIMES; i++)
    {
        expectedResults[i] = atid;
    }

    for (i = 0; i < 4 * TIMES; i++)
    {
        if (expectedResults[i] != testArray[i])
        {
            passed = FALSE;
            sprintf(str,
                    "Expected testArray[%d] == %d, not %d\n",
                    i, expectedResults[i], testArray[i]);
            testFail(verbose, str);
        }
    }

    /* reset processes */
    kill(atid);
    kill(btid);
    kill(ctid);
    kill(dtid);
    shared = 0;

    /* run test on core 2 */
    ready(atid = create((void *)t4, INITSTK, 31, "PRIORITY-A",
                        3, TIMES, testArray, &shared), 0, CORE_TWO);
    ready(btid = create((void *)t4, INITSTK, 32, "PRIORITY-B",
                        3, TIMES, testArray, &shared), 0, CORE_TWO);
    ready(ctid = create((void *)t4, INITSTK, 34, "PRIORITY-C",
                        3, TIMES, testArray, &shared), 0, CORE_TWO);
    ready(dtid = create((void *)t4, INITSTK, 32, "PRIORITY-D",
                        3, TIMES, testArray, &shared), 0, CORE_TWO);

    /* Run the tests by yielding the processor */
    yield();

    /* Generate expected results and compare */

    for (i = 0; i < TIMES; i++)
    {
        expectedResults[i] = ctid;
    }

    /* Since threads may be rescheduled before they can disable interrupts, the
     * exact order than threads B and D, which have equal priority, execute in
     * is unpredictable.  To avoid spurious test failures, we accept them
     * executing in any order.  */
    btid_remaining = TIMES;
    dtid_remaining = TIMES;
    udelay(9);
    for (i = TIMES; i < 3 * TIMES; i++)
    {
        if (testArray[i] == btid && btid_remaining > 0) {
            expectedResults[i] = btid;
            btid_remaining--;
        } else {
            expectedResults[i] = dtid;
            dtid_remaining--;
        }
    }

    for (i = 3 * TIMES; i < 4 * TIMES; i++)
    {
        expectedResults[i] = atid;
    }

    for (i = 0; i < 4 * TIMES; i++)
    {
        if (expectedResults[i] != testArray[i])
        {
            passed = FALSE;
            sprintf(str,
                    "Expected testArray[%d] == %d, not %d\n",
                    i, expectedResults[i], testArray[i]);
            testFail(verbose, str);
        }
    }

    /* reset processes */
    kill(atid);
    kill(btid);
    kill(ctid);
    kill(dtid);
    shared = 0;

    /* run test on core 3 */
    ready(atid = create((void *)t4, INITSTK, 31, "PRIORITY-A",
                        3, TIMES, testArray, &shared), 0, CORE_THREE);
    ready(btid = create((void *)t4, INITSTK, 32, "PRIORITY-B",
                        3, TIMES, testArray, &shared), 0, CORE_THREE);
    ready(ctid = create((void *)t4, INITSTK, 34, "PRIORITY-C",
                        3, TIMES, testArray, &shared), 0, CORE_THREE);
    ready(dtid = create((void *)t4, INITSTK, 32, "PRIORITY-D",
                        3, TIMES, testArray, &shared), 0, CORE_THREE);

    /* Run the tests by yielding the processor */
    yield();

    /* Generate expected results and compare */

    for (i = 0; i < TIMES; i++)
    {
        expectedResults[i] = ctid;
    }

    /* Since threads may be rescheduled before they can disable interrupts, the
     * exact order than threads B and D, which have equal priority, execute in
     * is unpredictable.  To avoid spurious test failures, we accept them
     * executing in any order.  */
    btid_remaining = TIMES;
    dtid_remaining = TIMES;
    udelay(9);
    for (i = TIMES; i < 3 * TIMES; i++)
    {
        if (testArray[i] == btid && btid_remaining > 0) {
            expectedResults[i] = btid;
            btid_remaining--;
        } else {
            expectedResults[i] = dtid;
            dtid_remaining--;
        }
    }

    for (i = 3 * TIMES; i < 4 * TIMES; i++)
    {
        expectedResults[i] = atid;
    }

    for (i = 0; i < 4 * TIMES; i++)
    {
        if (expectedResults[i] != testArray[i])
        {
            passed = FALSE;
            sprintf(str,
                    "Expected testArray[%d] == %d, not %d\n",
                    i, expectedResults[i], testArray[i]);
            testFail(verbose, str);
        }
    }

    if (TRUE == passed)
    {
        testPass(TRUE, "");
    }
    else
    {
        testFail(TRUE, "");
    }

    return OK;
}
