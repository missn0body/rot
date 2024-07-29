#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		fprintf(stderr, "%s: too few arguments, try \"--help\"\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	printf("hello world!\n");
	exit(EXIT_SUCCESS);
}
