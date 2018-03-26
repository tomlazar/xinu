/**
 * @file kprintf.c
 */

/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <kernel.h>
#include <stdarg.h>

#ifdef _XINU_PLATFORM_ARM_RPI_3_
#	include <mutex.h>
	mutex_t serial_lock = UNLOCKED;
#endif	/* _XINU_PLATFORM_ARM_RPI_3_ */

/**
 * @ingroup uartgeneric
 *
 * kernel printf: formatted, synchronous output to SERIAL0.
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

#ifdef _XINU_PLATFORM_ARM_RPI_3_
	mutex_acquire(&serial_lock);
	retval = kvprintf(format, ap);
	mutex_release(&serial_lock);
#else
	retval = kvprintf(format, ap);
#endif	

	va_end(ap);
    
	return retval;

}
