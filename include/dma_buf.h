/**
 * @file dma_buf.h
 * Definitions for dma buffer allocation and maintenance.
 *
 */
/* Embedded Xinu, Copyright (C) 2019. All rights reserved. */

#ifndef _DMA_BUF_H_
#define _DMA_BUF_H_

#include <stddef.h>
#include <stdint.h>

extern uint8_t dma_buf_space[];

/* DMA buffer function prototypes */
syscall dma_buf_init(void);
void *dma_buf_alloc(uint);
syscall dma_buf_free(void *, uint);

#endif	/* _DMA_BUF_H_ */
