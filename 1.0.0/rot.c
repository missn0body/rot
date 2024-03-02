#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

///////////////////////////////////////////////////////////
// BOILERPLATE BEGINS HERE
///////////////////////////////////////////////////////////

// We're not using "char", just in case there's some
// dystopian future where they changed the definition
// of "char". Might be just bias, but I put more trust
// in stdint.h's typedefs.
typedef uint8_t flag_t;

// Shorthands for setting bits in a flag typedef
#define setbit(x, y)	((x) |= (y))
#define unsetbit(x, y)	((x) &= (~(y)))

// Shorthand for testing a bit in a flag typedef
// e.g. if(test(status, ANSI)) { /*do ANSI stuff*/ }
#define test(x, y)	(((x) & (y)) != 0)

// Set the last bit on, which will be used when
// checking for ANSI mode
#define ANSI            (1 << 7)

#define ROT47		(1 << 3)
#define ROTN		(1 << 4)

// ANSI escape codes, to use for color output, cause
// I quite like colors in my terminal :)
#define RED		"\033[30;101m"
#define BLUE		"\033[34m"
#define GREEN		"\033[32m"
#define RESET		"\033[0m"

// This static object will hold all of the
// active modes within the program, with 8
// booleans (bits) to be raised or lowered
static flag_t status = 0;

// This function is not meant to be called directly, unless you
// want to provide a file and line number directly instead of
// nifty GCC extensions.
[[ __noreturn__ ]] void _error(char *file, int line, const char *mes, flag_t *status)
{
	if(test(*status, ANSI)) fprintf(stderr, "%s(%s:%d)%s %s\n", RED, file, line, RESET, mes);
	else			fprintf(stderr, "(%s:%d) %s\n", file, line, mes);
        exit(EXIT_FAILURE);
}

// Rather I prefer you call this, unless you can't.
#define _Error(x)		_error(__FILE__, __LINE__, x, &status)

// My own assert function, because assert.h was too much for me
#define _Assert(x, y)		if(!(x)) { _Error(y); }

///////////////////////////////////////////////////////////
// BOILERPLATE ENDS HERE
///////////////////////////////////////////////////////////

constexpr int OURBUF 	  = 256;
constexpr int ASCII_BEGIN = 33;
constexpr int ASCII_END   = 126;
constexpr int ASCII_SPAN  = (ASCII_END - ASCII_BEGIN) + 1;

constexpr int UPPER_BEGIN = 65;
constexpr int UPPER_END   = 90;
constexpr int LOWER_BEGIN = 97;
constexpr int LOWER_END   = 122;
constexpr int ALPHA_SPAN  = (UPPER_END - UPPER_BEGIN) + 1;

void rotate(FILE *fObj, flag_t *status, int shift)
{
	if(fObj == nullptr) return;

	char buffer[OURBUF];
	while(fgets(buffer, sizeof buffer, fObj) != NULL)
	{
		buffer[strcspn(buffer, "\r\n")] = '\0';
		if(test(*status, ROT47) || shift == 0)
		{
			// ROT47
			for(size_t i = 0; buffer[i] != '\0'; i++)
				if(buffer[i] >= ASCII_BEGIN && buffer[i] <= ASCII_END)
					buffer[i] = ASCII_BEGIN + ((buffer[i] + 14) % ASCII_SPAN);
		}
		else if(test(*status, ROTN) || shift != 0)
		{
			// ROT(N), e.g. ROT13, ROT1, ROT25, etc...
			for(size_t i = 0; buffer[i] != '\0'; i++)
			{
				if(buffer[i] >= UPPER_BEGIN && buffer[i] <= UPPER_END)
				{
					buffer[i] = (buffer[i] + shift > UPPER_END) ? (buffer[i] + shift) - ALPHA_SPAN : buffer[i] + shift;
				}
				else if(buffer[i] >= LOWER_BEGIN && buffer[i] <= LOWER_END)
				{
					buffer[i] = (buffer[i] + shift > LOWER_END) ? (buffer[i] + shift) - ALPHA_SPAN : buffer[i] + shift;
				}
			}
		}

		printf("%s\n", buffer);
	}

	return;
}

int main(int argc, char *argv[])
{
	// This program needs some sort of input, whether it be from pipe or a file
	_Assert(!isatty(STDIN_FILENO) || argc > 1, "too few arguments. try \"--help\"");
	// If STDIN is a terminal, then there must be args at this point
	FILE *fObj = (isatty(STDIN_FILENO) || argc > 2) ? fopen(argv[1], "r") : stdin;
	_Assert(fObj != nullptr, strerror(errno));

	// Actual rotation
	rotate(fObj, &status, 13);
	return 0;
}
