# ya-sprintf
A fast replacement for the sprintf family of functions (printf, fprintf, sprintf, etc).

This started with the code from  stb_sprintf (v1.08) which is a public domain snprintf() implementation (http://github.com/nothings/stb)
which itself was originally written by Jeff Roberts / RAD Game Tools, 2015/10/20.
This version is dual licensed (MIT and Public Domain)

"ya-sprintf" stands for Yet Another sprintf.

It provides an almost full C99 printf family implementation but excludes wide characters/strings.
ya_sprintf also provides a number of extensions, the most significant is the ability to print 128 bit integers (__int128)
and 128 bit floating point numbers (__float128) for compilers which support thse types (most modern compilers do for 64 bit targets).
Its thread safe (with no need for any locking), does no dynamic memory allocation and is much faster than the built in "printf's" in MinGW/TDM-GCC.
It also fixes all the bugs I'm aware of in the MinGW/TDM-GCC implementations, these bugs and the slow speed are the main reason this exists. 
In comparison to stb_sprintf (v1.08) it fixes all the bugs I found, implements more of the C99 formats and adds long doubles, 128 bit ints and 128 bit floats.
Finally, it provides a consistant format string specification across different targets which aids portability between targets and compilers.

It assumes a compiler that supports at least C99, it assumes chars are 8 bit ascii, int's are 32 bits, long can be 32/64 bits and long long int is 64 bits. Pointers can be 32 or 64 bits.
Floating point numbers are assumed to be  IEEE 754 format, with double as 64 bits, long double as 80 bits (which may be stored in up to 128 bits depending on the api in use) and __float128's being 128 bits all of which are used.
Integers are asumed to be stored in 2's complement representation.
These assumptions are true for almost all processors manufactured in the last 10+ years (Intel X32 & X64, ARM, PowerPC, etc).

Also included is a "double double" library that uses two floating point numbers to provide higher accuracy and implementations of strtof(), strtod() and strtof128().
