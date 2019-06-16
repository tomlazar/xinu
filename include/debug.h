/**
 * @file debug.h
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stddef.h>

#define DEBUG_ASCII 0x01
#define DEBUG_HEX   0x02

void hexdump(void *buffer, ulong length, bool text);
void debugbreak(void);
void debugret(void);
void _debug_util(const char *file, const char *func, int line, const char *format, ...);

//#define DEBUG(msg) kprintf("[DEBUG] %s, line %d: %s\r\n", __FILE__, __LINE__, msg) 
#define DEBUG(format, ...) _debug_util(__FILE__, __func__, __LINE__, format, ##__VA_ARGS__)

#endif                          /* _DEBGU_H_ */
