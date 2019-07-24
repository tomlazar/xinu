#include <dma_buf.h>
#include <compiler.h>
#include <mutex.h>

// XXX: This code is really bad...
// only being used as a proof-of-concept..
// definitely will have to implement this differently.

#define SECTION_SIZE	0x00100000

static int freespace_idx = 0;
static int remaining_space = SECTION_SIZE;
static mutex_t dma_buf_mutex;

uint8_t dma_buf_space[SECTION_SIZE] __aligned(SECTION_SIZE);

void *dma_buf_alloc(uint size)
{
	void *retval;
	retval = (void *)(dma_buf_space + freespace_idx);

	mutex_acquire(dma_buf_mutex);	
	freespace_idx += size;
	remaining_space -= size;
	mutex_release(dma_buf_mutex);

	return retval;
}

syscall dma_buf_free(void *ptr, uint size)
{
	mutex_acquire(dma_buf_mutex);
	freespace_idx -= size;
	remaining_space += size;
	mutex_release(dma_buf_mutex);

	return OK;
}

syscall dma_buf_init()
{
	freespace_idx = 0;
	remaining_space = SECTION_SIZE;
	dma_buf_mutex = mutex_create();

	return OK;
}

static void *_geti(struct two_dim_array *this, int i)
{
	return (void *)(this->base + (i * this->sizei));
}

static void *_geto(struct two_dim_array *this, int i, int j)
{
	return (void *)(this->base + (i * this->sizei) + j);
}

static void _seti(struct two_dim_array *this, int i, int val)
{
}

static void _seto(struct two_dim_array *this, int i, int j, int val)
{
}

syscall two_dim_array_init(struct two_dim_array *p, void *base, int sizei, int sizej)
{
	p->base = base;
	p->sizei = sizei;
	p->sizej = sizej;
	p->geti = _geti;
	p->geto = _geto;
	p->seti = _seti;
	p->seto = _seto;
}
