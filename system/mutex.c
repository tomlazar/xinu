#include <mutex.h>
#include <clock.h>

struct muxent muxtab[NMUTEX];

extern void _mutex_acquire(unsigned int *);
extern void _mutex_release(unsigned int *);
extern int _atomic_mutex_check(unsigned int *);
static mutex_t mutex_alloc(void);

/* Create and return an unlocked mutex */
mutex_t mutex_create()
{
	mutex_t mux;
	
	mux = mutex_alloc();
	if (SYSERR == mux)
	{
		return SYSERR;
	}
	
	muxtab[mux].lock = MUTEX_UNLOCKED;
	return mux;
}

syscall mutex_free(mutex_t mux)
{
	if (isbadmux(mux))
		return SYSERR;

	muxtab[mux].state = MUTEX_FREE;
	
	return OK;	
}

syscall mutex_acquire(mutex_t mux)
{
	if (isbadmux(mux))
		return SYSERR;
	
	if (MUTEX_FREE == muxtab[mux].state)
		return SYSERR;
	
	_mutex_acquire(&(muxtab[mux].lock));
	return OK;

}

syscall mutex_release(mutex_t mux)
{
	if (isbadmux(mux))
		return SYSERR;
	if (MUTEX_FREE == muxtab[mux].state)
		return SYSERR;

	_mutex_release(&(muxtab[mux].lock));
	return OK;
}

static mutex_t mutex_alloc(void)
{
	int i;
	static unsigned int nextmux = 0;
	
	for (i = 0; i < NMUTEX; i++)
	{
		nextmux = (nextmux + 1) % NMUTEX;
		//kprintf("\r\ni = %d ... Checking state of nextmux... %d", i, muxtab[nextmux].state);
		if (0 == _atomic_mutex_check((unsigned int *)&(muxtab[nextmux].state)))
		{
			muxtab[nextmux].state = MUTEX_USED;
			return nextmux;
		}
	}
	return SYSERR;
};
