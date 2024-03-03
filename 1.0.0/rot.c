#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

///////////////////////////////////////////////////////////
// BOILERPLATE BEGINS HERE
///////////////////////////////////////////////////////////

const char *VERSION = "1.0.0";

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

// Modes of operation for this program
#define LIST		(1 << 2)
#define ROT47		(1 << 3)
#define ROTN		(1 << 4)

// ANSI escape codes, to use for color output, cause
// I quite like colors in my terminal :)
#define RED		"\033[30;101m"
#define BLUE		"\033[34m"
#define GREEN		"\033[32m"
#define RESET		"\033[0m"

// Macro shorthands, for inserting as a string variable
// into a printf-like statment
// e.g. printf("%s%s%s", red_if_color(status), "whatever", reset_if_color(status))
#define green_if_color(x)	(test((x), ANSI) ? GREEN: "")
#define reset_if_color(x)	(test((x), ANSI) ? RESET: "")

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
#define _NoteArg(x, y, z)	fprintf(stderr, "%s%s%s: %s -- \"%s\"\n", green_if_color(status), x, reset_if_color(status), y, z)

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

void rotate(FILE *fObj, FILE *outObj, flag_t *status, int shift)
{
	if(fObj == nullptr) return;
	FILE *out = (outObj == nullptr) ? stdout : outObj;

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

		fprintf(out, "%s\n", buffer);
	}

	return;
}

void showVersion(void) { printf("ROTate (%s): a customizable substitution cipher by anson.\n", VERSION); }
void showUsage(void)
{
	printf("ROTate (%s): a customizable substitution cipher by anson.\n", VERSION);
	puts("usage:\n\trot -h / --help");
	puts("\trot -v / --version");
	puts("\trot [-al] [-n <number>] infile [outfile]");
	puts("\tcommand-to-stdout | rot [-al] [-n <number>] [outfile]\n");
	puts("options:\n\t-l, --list\tPrints a list of rotated strings from infile");
	puts("\t-n, --num <arg> Specifies the amount that each character is shifted");
	puts("\t-a, --ansi\tPrints color via ANSI escape codes, where applicable");
	puts("\t-h, --help\tDisplays this information");
	puts("\t-v, --version\tDisplays version information");
}

int main(int argc, char *argv[])
{
	// This program needs some sort of input, whether it be from pipe or a file
	_Assert(!isatty(STDIN_FILENO) || argc > 1, "too few arguments. try \"--help\"");

	char inFile[80] = {0}, outFile[80] = {0};
	char *programName = argv[0];
	int c, shiftAmount = 0;

	// Iterate through all arguments sent, character by character
	while(--argc > 0 && (*++argv)[0] != '\0')
	{
		if((*argv)[0] != '-' && !isdigit((*argv)[0]))
		{
			if(outFile[0] != '\0' && !isdigit((*argv)[0]))
			{
				_NoteArg(programName, "discarded program input", *argv);
				continue;
			}

			if(inFile[0] == '\0') 	    strncpy(inFile, *argv, sizeof(inFile));
			else if(outFile[0] == '\0') strncpy(outFile, *argv, sizeof(outFile));
		}

		if((*argv)[0] == '-')
		{
			// If there's another dash, then it's a long option.
			// Move the pointer up 2 places and compare the word itself.
			if((*argv)[1] == '-')
			{
				// Using continue statements here so that the user
				// can use both single character and long options
				// simultaniously, and the loop can test both.
				if(strcmp((*argv) + 2, "help")    == 0) { showUsage(); 		 exit(EXIT_SUCCESS); }
				if(strcmp((*argv) + 2, "version") == 0) { showVersion(); 	 exit(EXIT_SUCCESS); }
				if(strcmp((*argv) + 2, "ansi")    == 0) { setbit(status, ANSI);		   continue; }
				if(strcmp((*argv) + 2, "list")    == 0) { setbit(status, LIST);		   continue; }
				if(strcmp((*argv) + 2, "num")     == 0)
				{
					if(*(argv + 1) == nullptr) _Error("option requires argument");
					else shiftAmount = atoi(*(argv + 1));
					continue;
				}
			}
			while((c = *++argv[0]))
			{
				// Single character option testing here.
				switch(c)
				{
					case 'h': showUsage(); 		exit(EXIT_SUCCESS);
					case 'v': showVersion(); 	exit(EXIT_SUCCESS);
					case 'a': setbit(status, ANSI); break;
					case 'l': setbit(status, LIST); break;
					case 'n':
						if(*(argv + 1) == nullptr) _Error("option requires argument");
						else shiftAmount = atoi(*(argv + 1));
						break;
					// This error flag can either be set by a
					// completely unrelated character inputted,
					// or you managed to put -option instead of
					// --option.
					default : _Error("unknown option. try \"--help\""); exit(EXIT_FAILURE);
				}
			}

			continue;
		}
	}

	FILE *inObj = nullptr, *outObj = nullptr;
	inObj = (!isatty(STDIN_FILENO) && inFile[0] != '\0') ? stdin : fopen(inFile, "r");

	if(inObj == nullptr) _Error(strerror(errno));
	if(outFile[0] != '\0') outObj = fopen(outFile, "a");

	puts(inFile);
	puts(outFile);
	printf("shiftAmount = %d\n", shiftAmount);

	puts((inObj == stdin) ? "input from pipe" : "input from file");
	puts((outObj != nullptr) ? "Writing to file" : "Writing to stdout");

	// Actual rotation
	//rotate(fObj, &status, 13);
	return 0;
}
