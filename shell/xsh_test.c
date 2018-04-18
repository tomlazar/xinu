/**
 * @file     xsh_test.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <shell.h>
#include <string.h>
#include <ctype.h>
#ifdef _XINU_PLATFORM_ARM_RPI_3_
#include <core.h>
#include <des.h>
#include <clock.h>

#define MSG_SIZE 48

extern unsigned int getcpuid(void);

char msg[MSG_SIZE] = "The quickred fox jumped over the lazy brown dog";
char cipher[MSG_SIZE];	

#if 0
static char most_common(char *str, int size)
{
	int i, max, index;
	int array[256] = {0};
	for (i = 0; i < size; i++)
	{
		++array[str[i]];
	}

	max = array[0];
	index = 0;
	for (i = 0; i < 256; i++)
	{
		if (array[i] >= max)
		{
			index = i;
			max = array[i];
		}
	}

	return (char) index;
}
#endif

static bool is_ascii_string(char *s, size_t length)
{
	int i;
	for (i = 0; i < length - 1; i++)
	{
		if (s[i] >= 0x20 && s[i] <= 0x7E)
		{ continue; }
		else if (s[i] == 0x0A || s[i] == 0x0D || s[i] == 0x09)
		{ continue; }
		else
			return FALSE;
	}
	return TRUE;
}

static void print_key(uint64_t key)
{
	int i;
    for(i = 0; i < 64; i++)
    {
        if( ((key << i) & FIRSTBIT) == (uint64_t)0)
            printf("0");
        else
            printf("1");
    }
	printf("\n");
}

static int calculate_parity_bit(uint8_t b)
{
	int i, parity_bit;
	parity_bit = 0;
	for (i = 1; i < 8; i++)	// start at 2nd bit to reserve space for the parity bit
	{
		if ((b >> i) & 1)
			parity_bit = parity_bit == 0 ? 1 : 0;
	}
	return parity_bit;
}

static uint64_t key_guess(int s)
{
	int i, r;
	uint64_t g = 0;
	uint8_t *split = (uint8_t *) &g;
	
	i = 0;
	r = s;
	while (r > 0 || i < 8)
	{
		split[i] = (r % 128) << 1;
		split[i] |= calculate_parity_bit(split[i]);
		
		r = r / 128;
		i++;
	}

	return g;
}

bool done[4] = {0};
uint64_t guesses[4] = {0};

/**
 * start == index to start at
 * end   == index to end at
 */
struct args
{
	int start;
	int end;
};

void brutus(void *arguments)
{
	uint cpuid = getcpuid();

	struct args *a = (struct args *) arguments;
	
	char d[MSG_SIZE];
	int i, start, end;
	uint64_t guess;

	start = a->start;
	end = a->end;
	free(a);	
	
	for (i = start; i < end; i++)
	{
		guess = key_guess(i);
		des_decrypt(cipher, MSG_SIZE, d, guess);
		if (is_ascii_string(d, MSG_SIZE) == TRUE)
		{
			// FOUND THE RIGHT KEY
			done[cpuid] = TRUE;
			guesses[cpuid] = guess;
			kprintf("%s\r\n", d);
			while (TRUE) { } // infinite loop.. nothing else to do			
		}		
	}
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
/* This commented out section is to demonstrate argument passing for unparkcore */
#if 0
	struct test_args *a = (struct test_args *) malloc(sizeof(struct test_args));
	int i;
	i = atoi(args[1]);
	a->a = atoi(args[2]);
	a->b = atoi(args[3]);
	printf("Unparking core %d\n", i);
	unparkcore(atoi(args[1]), (void *) test, (void *) a);
#endif

//	uint64_t g = key_guess(i);
//	print_key(g);

	int i, found_on_core;
	uint64_t correct_guess;

	char d[MSG_SIZE];

	uint64_t key = 0;
	uint64_t guess;
//	genkey(&key);
	key = (0x05 << 16) | (0x7D << 8) | (0x2D);
	print_key(key);
	printf("parity: %s\n", key_parity_verify(key) == 1 ? "GOOD" : "BAD");

	des_encrypt(msg, MSG_SIZE, cipher, key);

#if 0
	/* 2^16 == 65536 */
	for (i = 0; i < 65536; i++)
	{
		guess = key_guess(i);
		printf("GUESS KEY %d:\n", i);
		des_decrypt(cipher, MSG_SIZE, d, guess);
		if (strncmp(d, msg, MSG_SIZE) == 0)
		{
			// successful crack
			printf("FOUND KEY!!!\n");
			print_key(guess);
			return SHELL_OK;
		}
	}
#endif
	
	/* allocate memory for argument structs */
	/* they are free'd once the core is unparked */
	struct args *core1args = (struct args *) malloc(sizeof(struct args));
	struct args *core2args = (struct args *) malloc(sizeof(struct args));
	struct args *core3args = (struct args *) malloc(sizeof(struct args));
	if (core1args == NULL || core2args == NULL || core3args == NULL)
	{
		fprintf(stderr, "ERROR: malloc\n");
		return SHELL_OK;
	}

	/* set the arguments for each core */
	core1args->start = 16384;
	core1args->end   = 32768;
	core2args->start = 32768;
	core2args->end   = 49152;
	core3args->start = 49152;
	core3args->end   = 65536;
	
	/* unpark the cores */
	unparkcore(1, brutus, (void *) core1args);
	unparkcore(2, brutus, (void *) core2args);
	unparkcore(3, brutus, (void *) core3args);

	for (i = 0; i < 16384; i++)
	{
		// BRUTE FORCE SECTION
		guess = key_guess(i);
		des_decrypt(cipher, MSG_SIZE, d, guess);
		if (is_ascii_string(d, MSG_SIZE) == TRUE)
		{
			// FOUND KEY
			done[0] = TRUE;
			guesses[0] = guess;
			printf("%s\n", d);
			break;
		}
		

		// CORE MONITORING SECTION
		if (done[0] == TRUE)
		{
			found_on_core = 0;
			correct_guess = guesses[0];
			break;
		}
		else if (done[1] == TRUE)
		{
			found_on_core = 1;
			correct_guess = guesses[1];
			break;
		}
		else if (done[2] == TRUE)
		{
			found_on_core = 2;
			correct_guess = guesses[2];
			break;
		}
		else if (done[3] == TRUE)
		{
			found_on_core = 3;
			correct_guess = guesses[3];
			break;
		}
		else { }
	}

	printf("KEY: ");
	print_key(correct_guess);
	printf("FOUND ON CORE: %d\n", found_on_core);

	return SHELL_OK;

/*
	printf("encrypting message: %s\n\n", msg);
	des_encrypt(msg, MSG_SIZE, cipher, key);
	printf("encrypted message (in hex):\n");
	for (i = 0; i < MSG_SIZE; i++)
	{
		printf("0x%02X ", cipher[i]);
	}
	printf("\n\n");

	des_decrypt(cipher, MSG_SIZE, d, key);
	printf("decrypted message: %s\n", d);

	return SHELL_OK;
*/
}
