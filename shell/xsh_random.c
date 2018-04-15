#include <stdio.h>
#include <stdlib.h>
#include <shell.h>
#include <string.h>
#ifdef _XINU_PLATFORM_ARM_RPI_3_
#include <random.h>
#endif

static void usage(const char *command);

#define DEFAULT_MIN	0
#define DEFAULT_MAX	999999

shellcmd xsh_random(int nargs, char *args[])
{
#ifdef _XINU_PLATFORM_ARM_RPI_3_
	unsigned int min, max, rand;

	if (2 == nargs && 0 == strcmp(args[1], "--help"))
	{
		usage(args[0]);
		return SHELL_OK;
	}

	if (nargs > 3)
	{
		fprintf(stderr, "ERROR: Too many arguments.\n");
		usage(args[0]);
		return SHELL_ERROR;
	}

	if (nargs == 3)
	{
		min = atoi(args[1]);
		max = atoi(args[2]);
		if (min > max)
		{
			fprintf(stderr, "ERROR: Minimum is greater than maximum.\n");
			usage(args[0]);
			return SHELL_ERROR;
		}
	}
	else if (nargs == 2)
	{
		min = DEFAULT_MIN;
		max = atoi(args[1]);
	}
	else
	{
		min = DEFAULT_MIN;
		max = DEFAULT_MAX;
	}

	rand = random() % (max - min) + min;
	printf("%u\n", rand);
	return SHELL_OK;
#else
	fprintf(stderr, "ERROR: Command not compatible with this platform\n");
	return SHELL_ERROR;
#endif	/* _XINU_PLATFORM_ARM_RPI_3_ */
}

static void usage(const char *command)
{
	printf(
"Usage: %s <MIN> <MAX>\n\n"
"Description:\n"
"\tGenerate a random number.\n"
"Options:\n"
"\t<MIN>	An optional argument, specifies the minimum number to be generated.\n"
"\t<MAX>	Required argument, specifies the maximum number to be generated.\n"
	, command);
}
