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

// The "nullptr" keyword also applies, I
// understand if this makes a C developer
// weary.

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

// This is how many shifts we should have for a shift table
static constexpr short shift_count = 10;

// In the previous version, I used ctype.h for two functions.
// I like to use the least amount of header files whenever I can,
// so I will approximate the ctype.h functions below.
// You can easily delete these and substitute for the actual functions
static inline bool my_isgraph(const int what) { return ( what >= ascii_begin && what <= ascii_end ) ? true : false; }
static inline bool my_isdigit(const int what) { return ( (what - '0') <= 9 && (what - '0') >= 1 ) ? true : false; }
static inline bool strempty(const char *what) { return (*what == '\0'); }
static inline const char *boolstr(bool what)  { return (what) ? "true" : "false"; }

// Actual rotation function
// Warning! This function is destructive, the buffer given will be operated on
// Save a previous state of the buffer before passing
void rotate_str(char *in, size_t size, short shift, bool verbose)
{
	// Sanity check
	if(in == nullptr)
	{
		fprintf(stderr, "%s: buffers reported nullptr, exiting...", __func__);
		return;
	}

	// If we get some sort of outrageous shift, either wrap-around or reset
	if(shift >= 26) shift -= 26;
	if(shift >= 52)
	{
		fprintf(stderr, "%s: enormous shift amount recieved, reverting to rot47...\n", __func__);
		shift = 0;
	}

	if(shift == 0)
	{
		// ROT47
		if(verbose) printf("%s: rot47 mode. begin rotation\n", __func__);
		for(size_t i = 0; i < size; i++)
			if(my_isgraph(in[i])) in[i] = ascii_begin + ((in[i] + 14) % ascii_span);
	}
	else if(shift != 0)
	{
		// ROT(N), e.g. ROT13, ROT1, ROT25, etc...
		if(verbose) printf("%s: rot-n mode. begin rotation\n", __func__);
		for(size_t i = 0; i < size; i++)
		{
			if(in[i] >= upper_begin && in[i] <= upper_end)
			{
				in[i] = (in[i] + shift > upper_end) ? (in[i] + shift) - alpha_span : in[i] + shift;
			}
			else if(in[i] >= lower_begin && in[i] <= lower_end)
			{
				in[i] = (in[i] + shift > lower_end) ? (in[i] + shift) - alpha_span : in[i] + shift;
			}
		}
	}

	return;

}

// The verbose flag should also function as run-time diagnostics,
// spitting out its current state and arguments to stdout
void rotate_file(const char *in, const char *out, short shift, bool verbose)
{
	if(verbose) printf("%s: begin function (from %s, to %s, shift %d, verbose %s)\n", __func__, in, out, shift, boolstr(verbose));

	// We can either gather input from stdin or a file, and output to stdout or a file
	FILE *in_obj  = (in  == nullptr) ? stdin  : fopen(in, "r");
	FILE *out_obj = (out == nullptr) ? stdout : fopen(out, "a");
	if(in_obj == nullptr || out_obj == nullptr) { perror(__func__); return; }

	// Input and output buffers
	char buffer[bufsize] = {0};

	// Main loop through file object
	while(fgets(buffer, bufsize, in_obj) != nullptr)
	{
		// Erase trailing newline
		buffer[strcspn(buffer, "\r\n")] = '\0';
		rotate_str(buffer, bufsize, shift, verbose);

		// Output rotated string
		fprintf(out_obj, "%s\n", buffer);
		if(verbose) printf("%s: done.\n", __func__);
	}

	// Clean up
	if(verbose) printf("%s: end function (from %s, to %s, shift %d, verbose %s)\n", __func__, in, out, shift, boolstr(verbose));
	if(in_obj  != stdin)  fclose(in_obj);
	if(out_obj != stdout) fclose(out_obj);

	return;
}

/*

// This function will generate a table which iterates through rotations of the input,
// e.g. "abcd"/w 1 -> "bcde", "abcd"/w 2 -> "cdef, etc."
void generate_table(const char *in, const char *out, bool verbose)
{

}

*/

// User Interface functions
void version(void)
{
	printf("ROTate (%s): a customizable substitution cipher\n", VERSION);
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
	// This program needs some sort of input, whether it be from pipe or a file
	bool frompipe = !isatty(STDIN_FILENO);
	bool verbose_flag = false;
	bool gen_table = false;

	char *program = argv[0];
	char infile[bufsize] = {0}, outfile[bufsize] = {0};

	if(!(frompipe || argc > 1))
	{
		fprintf(stderr, "%s: too few arguments, try \"--help\"\n", program);
		exit(EXIT_FAILURE);
	}

	int c, shift_amount = 0;
	// Iterate through all arguments sent, character by character
	while(--argc > 0 && (*++argv)[0] != '\0')
	{
		if((*argv)[0] != '-' && !my_isdigit((*argv)[0]))
		{
			if(!strempty(outfile) && !my_isdigit((*argv)[0]))
			{
				fprintf(stderr, "%s: discarded program input -- \"%s\"\n", program, *argv);
				continue;
			}

			if	(strempty(infile) && !frompipe) strncpy(infile, *argv, bufsize);
			else if (strempty(outfile))		strncpy(outfile, *argv, bufsize);
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
				if(strcmp((*argv) + 2, "help")    == 0) { usage();   exit(EXIT_SUCCESS); }
				if(strcmp((*argv) + 2, "version") == 0) { version(); exit(EXIT_SUCCESS); }
				if(strcmp((*argv) + 2, "verbose") == 0) { verbose_flag = true; continue; }
				if(strcmp((*argv) + 2, "table")	  == 0) { gen_table = true;    continue; }
				if(strcmp((*argv) + 2, "num")     == 0)
				{
					if(*(argv + 1) == nullptr)
					{
						fprintf(stderr, "%s: option \"%s\" requires argument\n", program, *argv);
						exit(EXIT_FAILURE);
					}
					else shift_amount = atoi(*(argv + 1));
					continue;
				}
			}
			while((c = *++argv[0]))
			{
				// Single character option testing here.
				switch(c)
				{
					case 'h': usage(); exit(EXIT_SUCCESS);
					case 'v': verbose_flag = true; break;
					case 't': gen_table = true;    break;
					case 'n':
						if(*(argv + 1) == nullptr)
						{
							fprintf(stderr, "%s: option \"%c\" requires argument\n", program, c);
							exit(EXIT_FAILURE);
						}
						else shift_amount = atoi(*(argv + 1));
						break;
					// This error flag can either be set by a
					// completely unrelated character inputted,
					// or you managed to put -option instead of
					// --option.
					default : fprintf(stderr, "%s: unknown option. try \"--help\"", program); exit(EXIT_FAILURE);
				}
			}

			continue;
		}
	}

	// Again let's as if we're getting anything from pipe, that takes precedence over
	// input from a file
	bool is_stdin  = frompipe && strempty(infile);
	bool is_stdout = strempty(outfile);

	// Debug prints if it's set
	if(verbose_flag)
	{
		printf("%s: %s\n", program, is_stdin  ? "recieved input from pipe" : "recieved input from file");
		printf("%s: %s\n", program, is_stdout ? "writing to stdout"	   : "writing to file");
	}

	// Only do a single round of rotation, no tables
//	if(!gen_table)
//	{
		// Actual rotation
		rotate_file((is_stdin == true) ? nullptr : infile, (is_stdout == true) ? nullptr : outfile, shift_amount, verbose_flag);

//	}
	// Generating a shift table
//	else
//	{
//		generate_table((is_stdin == true) ? nullptr : infile, (is_stdout == true) ? nullptr : outfile, verbose_flag);
//	}

	// End!
	exit(EXIT_SUCCESS);
}

