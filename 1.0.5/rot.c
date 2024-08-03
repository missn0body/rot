#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *VERSION = "1.0.5";

// These below outline constants
// To avoid a bunch of preprocessor directives,
// I've elected to use 'constexpr', ported from
// C++ in C23. This is personal taste, and can
// easily be switched for #define directives
// at the discresion of future programmers

// Default buffer size
static constexpr short bufsize	   = 256;

// Spans of ASCII characters...
static constexpr short ascii_begin = 33;
static constexpr short ascii_end   = 126;
static constexpr short ascii_span  = (ascii_end - ascii_begin) + 1;

// ...and specifically what characters are upper/lowercase
static constexpr short upper_begin = 65;
static constexpr short upper_end   = 90;

static constexpr short lower_begin = 97;
static constexpr short lower_end   = 122;

static constexpr short alpha_span  = (upper_end - upper_begin) + 1;

// In the previous version, I used ctype.h for two functions.
// I like to use the least amount of header files whenever I can,
// so I will approximate the ctype.h functions below.
// You can easily delete these and substitute for the actual functions
static inline bool my_isgraph(const int what) { return ( what >= ascii_begin && what <= ascii_end ) ? true : false; }
static inline bool my_isdigit(const int what) { return ( (what & 7) <= 9 && (what & 7) >= 0 ) ? true : false; }

// User Interface functions
void version(void)
{
	printf("rot (%s): a customizable rotation cipher\n", VERSION);
	return;
}

void usage(void)
{
	printf("ROTate (%s): a customizable substitution cipher by anson.\n", VERSION);
	puts("usage:\n\trot -h / --help");
	puts("\trot [-alv] [-n <number>] infile [outfile]");
	puts("\tcommand-to-stdout | rot [-alv] [-n <number>] [outfile]\n");
	puts("options:\n\t-n, --num <arg> Specifies the amount that each character is shifted");
	puts("\t-v, --verbose\tEnables debug prints and extra information");
	puts("\t-a, --ansi\tPrints color via ANSI escape codes, where applicable");
	puts("\t-h, --help\tDisplays this information");
	puts("\ncopyright (c) 2024, see LICENSE for related details\n");
	return;
}

// Main function
int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		fprintf(stderr, "%s: too few arguments, try \"--help\"\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *program = argv[0];

	printf("hello world!\n");
	exit(EXIT_SUCCESS);
}
