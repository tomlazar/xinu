/**
 * @file     xsh_test.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#ifdef _XINU_PLATFORM_ARM_RPI_3_
#include <core.h>
#include <des.h>
extern unsigned int getcpuid(void);
extern void udelay(uint);

struct test_args
{
	int a;
	int b;	
};

static void test(void *ar)
{
	struct test_args *i = (struct test_args *) ar;
	udelay(1);
	kprintf("\r\nCore %d, arg: %d, %d\r\n", getcpuid(), i->a, i->b);
	free(i);
	while (1) {}	
}
#endif
/**
 * @ingroup shell
 *
 * Shell command (test) provides a mechanism for testing Xinu features.  The
 * action and output varies depending on the feature currently being tested.
 * This is not meant to serve as a permanent shell command for a particular
 * action.
 * @param nargs number of arguments
 * @param args  array of arguments
 * @return non-zero value on error
 */
shellcmd xsh_test(int nargs, char *args[])
{
#if 0
	struct test_args *a = (struct test_args *) malloc(sizeof(struct test_args));
	int i;
	i = atoi(args[1]);
	a->a = atoi(args[2]);
	a->b = atoi(args[3]);
	printf("Unparking core %d\n", i);
	unparkcore(atoi(args[1]), (void *) test, (void *) a);
	return 0;
#endif
	int i;

	char msg[16] = "Hello_World1234";
	char cipher[16];	
	char d[16];

	uint64_t key = 0;
	genkey(&key);

    for(int i = 0; i < 64; i++)
    {
        if( ((key << i) & FIRSTBIT) == (uint64_t)0)
            printf("0");
        else
            printf("1");
    }


	printf("\n");
	printf("key_parity_verify(): %d\n\n", key_parity_verify(key));

	printf("encrypting message: %s\n\n", msg);
	des_encrypt(msg, 16, cipher, key);
	printf("encrypted message (in hex):\n");
	for (i = 0; i < 16; i++)
	{
		printf("0x%02X ", cipher[i]);
	}
	printf("\n\n");

	des_decrypt(cipher, 16, d, key);
	printf("decrypted message: %s\n", d);

	return 0;
}
