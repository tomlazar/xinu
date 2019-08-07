/**
 * @file kprintf.c
 */

/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <kernel.h>
#include <stdarg.h>
#include <mutex.h>

/**
 * @ingroup uartgeneric
 *
 * kernel printf: Multicore-safe (by mutex locks), formatted, synchronous output to SERIAL0.
 *
 * @param format
 *      The format string.  Not all standard format specifiers are supported by
 *      this implementation.  See _doprnt() for a description of supported
 *      conversion specifications.
 * @param ...
 *      Arguments matching those in the format string.
 *
 * @return
 *      The number of characters written.
 */
syscall kprintf(const char *format, ...)
{
    int retval;
    va_list ap;
    
    va_start(ap, format);

    mutex_acquire(serial_lock);
    retval = kvprintf(format, ap);
    mutex_release(serial_lock);

    va_end(ap);

    return retval;
}
