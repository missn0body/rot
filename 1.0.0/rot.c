#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int OURBUF = 256;

void rotate(FILE *fObj, int shift)
{
	if(fObj == NULL) return;

	char buffer[OURBUF];
	while(fgets(buffer, sizeof buffer, fObj) != NULL)
	{
		buffer[strcspn(buffer, "\r\n")] = '\0';
		printf("%s\n", buffer);
	}
}

int main(int argc, char *argv[])
{
	// This program needs some sort of input, whether it be from pipe or a file
	if(isatty(STDIN_FILENO) && argc < 2) { fprintf(stderr, "error: no args\n"); return -1; }
	// If STDIN is a terminal, then there must be args at this point
	FILE *fObj = (isatty(STDIN_FILENO) || argc > 2) ? fopen(argv[1], "r") : stdin;
	if(fObj == NULL) { perror(argv[0]); return -1; }

	// Actual rotation
	rotate(fObj, 47);
	return 0;
}
