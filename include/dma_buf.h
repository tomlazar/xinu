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

/* 2D array abstraction */
struct two_dim_array
{
	void *base;
	int sizei;
	int sizej;
	void *(*geti)(struct two_dim_array *, int i);			// base[i]
	void *(*geto)(struct two_dim_array *, int i, int j);		// base[i][j]
	void (*seti)(struct two_dim_array *, int i, int val);		// base[i]
	void (*seto)(struct two_dim_array*, int i, int j, int val);	// base[i][j]
};

syscall two_dim_array_init(struct two_dim_array *, void *, int, int);

extern uint8_t dma_buf_space[];

/* DMA buffer function prototypes */
syscall dma_buf_init(void);
void *dma_buf_alloc(uint);
syscall dma_buf_free(void *, uint);

#endif	/* _DMA_BUF_H_ */
