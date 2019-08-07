#include <stdio.h>
#include <stddef.h>
#include <testsuite.h>
#include <interrupt.h>
#include <thread.h>
#include <core.h>

thread spin(void)
{
    volatile int n = 1;

    while (n != 0)
    {
    }

    return OK;
}

thread test_preempt(bool verbose)
{
    /* the failif macro depends on 'passed' and 'verbose' vars */
    bool passed = TRUE;
    tid_typ thrspin;

    uint cpuid;
    cpuid = getcpuid();

    thrtab_acquire(thrcurrent[cpuid]);
    /* This is the first "subtest" of this suite */
    thrspin =
        create(spin, INITSTK, thrtab[thrcurrent[cpuid]].prio, "test_spin", 0);
    thrtab_release(thrcurrent[cpuid]);

    /* Make spin ... spin */
    ready(thrspin, RESCHED_YES, CORE_ZERO);

    /* If this next line runs, we're good */
    kill(thrspin);

    /* test on other cores one at a time */
    ready(thrspin, RESCHED_YES, CORE_ONE);
    kill(thrspin);
    ready(thrspin, RESCHED_YES, CORE_TWO);
    kill(thrspin);
    ready(thrspin, RESCHED_YES, CORE_THREE);
    kill(thrspin);

    /* always print out the overall tests status */
    if (passed)
    {
        testPass(TRUE, "");
    }
    else
    {
        testFail(TRUE, "");
    }

    return OK;
}
