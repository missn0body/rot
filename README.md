# rotate

A customizable rotation cipher. Made by anson in 2024.
This program can accept input from a pipe and also by a
file passed in arguments. By default, this program uses
the ROT47 and outputs to standard output, but can be 
configured to use different rotation ciphers, such as 
Caesar and ROT13, as well as output to a file.

Command-line arguments are as follows:
> rot -n (--number) <number> -a (--ansi) -v (--verbose) -h (--help)

The short version of these arguments can be combined together,
and both short and long arguments can be mixed together. If not
accepting input from a pipe or file, this program will not run.

Usage examples are as follows:
> rot [-alv] [-n <number>] infile [outfile]
> command-to-stdout | rot [-alv] [-n <number>] [outfile]

For more information on the ciphers as replicated by these programs,
see the links below:

www.rot47.net
www.rumkin.com/tools/cipher/rot13/
en.wikipedia.org/wiki/ROT13
en.wikipedia.org/wiki/Caesar_cipher

### v.1.0.0 (Initial Release)

(March 2024)
A customizable rotation cipher.
