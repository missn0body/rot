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
static constexpr short shift_count = 13;

// In the previous version, I used ctype.h for two functions.
// I like to use the least amount of header files whenever I can,
// so I will approximate the ctype.h functions below.
// You can easily delete these and substitute for the actual functions
static inline bool my_isgraph(const int what) { return ( what >= ascii_begin && what <= ascii_end ) ? true : false; }
static inline bool my_isdigit(const int what) { return ( (what - '0') <= 9 && (what - '0') >= 1 ) ? true : false; }
static inline bool strempty(const char *what) { return (*what == '\0'); }
static inline const char *boolstr(bool what)  { return (what) ? "true" : "false"; }

// Actual rotation function
// Previously, this function would have trouble in a loop, because it destroyed its inputs,
// so you would get exponential instead of incremental shifts.
void rotate_str(char *in, char *out, size_t size, short shift, bool verbose)
{
	// Sanity check
	if(in == nullptr || out == nullptr)
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

	// Copy the input to the output
	// This function assumes that both buffers are of the same size
	strncpy(out, in, size);

	if(shift == 0)
	{
		// ROT47
		if(verbose) printf("%s: rot47 mode. begin rotation\n", __func__);
		for(size_t i = 0; i <= size; i++)
			if(my_isgraph(out[i])) out[i] = ascii_begin + ((out[i] + 14) % ascii_span);
	}
	else if(shift != 0)
	{
		// ROT(N), e.g. ROT13, ROT1, ROT25, etc...
		if(verbose) printf("%s: rot-n (%d) mode. begin rotation\n", __func__, shift);
		for(size_t i = 0; i <= size; i++)
		{
			if(out[i] >= upper_begin && out[i] <= upper_end)
			{
				out[i] = (out[i] + shift > upper_end) ? (out[i] + shift) - alpha_span : out[i] + shift;
			}
			else if(out[i] >= lower_begin && out[i] <= lower_end)
			{
				out[i] = (out[i] + shift > lower_end) ? (out[i] + shift) - alpha_span : out[i] + shift;
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
	FILE *out_obj = (out == nullptr) ? stdout : fopen(out, "w");
	if(in_obj == nullptr || out_obj == nullptr) { perror(__func__); return; }

	char buffer[bufsize] = {0}, outbuf[bufsize] = {0};

	// Main loop through file object
	while(fgets(buffer, bufsize, in_obj) != nullptr)
	{
		// Erase trailing newline
		buffer[strcspn(buffer, "\r\n")] = '\0';
		rotate_str(buffer, outbuf, bufsize, shift, verbose);

		// Output rotated string
		fprintf(out_obj, "%s\n", outbuf);
	}

	// Clean up
	if(verbose) printf("%s: end function (from %s, to %s, shift %d, verbose %s)\n", __func__, in, out, shift, boolstr(verbose));
	if(in_obj  != stdin)  fclose(in_obj);
	if(out_obj != stdout) fclose(out_obj);

	return;
}

// This function will generate a table which iterates through rotations of the input file
// e.g. "abcd"/w 1 -> "bcde", "abcd"/w 2 -> "cdef, etc."
void generate_table(const char *in, const char *out, bool verbose)
{
	if(verbose) printf("%s: begin function (from %s, to %s, verbose %s)\n", __func__, in, out, boolstr(verbose));

	// We can either gather input from stdin or a file, and output to stdout or a file
	FILE *in_obj  = (in  == nullptr) ? stdin  : fopen(in, "r");
	FILE *out_obj = (out == nullptr) ? stdout : fopen(out, "w");
	if(in_obj == nullptr || out_obj == nullptr) { perror(__func__); return; }

	char buffer[bufsize] = {0}, outbuf[bufsize] = {0};

	// Main loop through file object
	while(fgets(buffer, bufsize, in_obj) != nullptr)
	{
		// Erase trailing newline
		buffer[strcspn(buffer, "\r\n")] = '\0';

		fprintf(out_obj, "\"%s\" shifted %d times, from 1 to %d\n\n", buffer, shift_count, shift_count);
		for(short i = 1; i <= shift_count; i++)
		{
			rotate_str(buffer, outbuf, bufsize, i, verbose);
			fprintf(out_obj, "%s: %d\n", outbuf, i);
		}

		fputc('\n', out_obj);
	}

	// Clean up
	if(verbose) printf("%s: end function (from %s, to %s, verbose %s)\n", __func__, in, out, boolstr(verbose));
	if(in_obj  != stdin)  fclose(in_obj);
	if(out_obj != stdout) fclose(out_obj);

	return;
}

// User Interface functions
void version(void)
{
	printf("ROTate (%s): a customizable rotation cipher\n", VERSION);
	return;
}

void usage(void)
{
	printf("ROTate (%s): a customizable rotation cipher\n", VERSION);
	printf("created by anson <thesearethethingswesaw@gmail.com>\n\n");
	printf("usage:\n\trot (-h / --help)\n\trot --version\n");
	printf("\trot -t [-v] input-file [<output-file>]\n");
	printf("\trot [-v] [-n <shift>] input-file [<output-file>]\n");
	printf("\tcommand-to-stdout | rot -t [-v] [<output-file>]\n");
	printf("\tcommand-to-stdout | rot [-v] [-n <shift>] [<output-file>]\n\n");

	printf("options:\n\t%18s\t%s\n","-n, --num",	"specifies the shift amount, e.g. \"-n 3\" means to shift 3 times, ala Caesar cipher");
	printf("\t%18s\t%s\n", "-v, --verbose",		"prints verbose diagnostic information");
	printf("\t%18s\t%s\n", "-t, --table",		"generates a table of incrementing shifts for viewing");
	printf("\t%18s\t%s\n", "<shift>",		"a decimal, non-negative number");
	printf("\t%18s\t%s\n", "input-file",		"a file to pull information from. will not be altered");
	printf("\t%18s\t%s\n", "output-file",		"a file to output to. will erase previous contents\n");

	printf("copyright (c) 2024, see LICENSE for related details\n");
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
	if(!gen_table)
	{
		// Actual rotation
		rotate_file((is_stdin == true) ? nullptr : infile, (is_stdout == true) ? nullptr : outfile, shift_amount, verbose_flag);

	}
	// Generating a shift table
	else if(gen_table)
	{
		generate_table((is_stdin == true) ? nullptr : infile, (is_stdout == true) ? nullptr : outfile, verbose_flag);
	}

	// End!
	exit(EXIT_SUCCESS);
}

