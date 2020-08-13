/*	ya_sprintf.h
	============
Replacement for the "printf" family of functions (printf, fprintf,sprintf etc).
Version 2.0 30/7/2020 by Peter Miller.

This started with the code from  stb_sprintf (v1.08) which is a public domain snprintf() implementation (http://github.com/nothings/stb)
which itself was originally written by Jeff Roberts / RAD Game Tools, 2015/10/20.
This version is dual licensed (MIT and Public Domain) - see the end of this file for details.

"ya_sprintf" stands for Yet Another sprintf.

It provides an almost full C99 printf family implementation but excludes wide characters/strings.
ya_sprintf also provides a number of extensions, the most significant is the ability to print 128 bit integers (__int128)
and 128 bit floating point numbers (__float128) for compilers which support thse types (most modern compilers do for 64 bit targets).
Its thread safe (with no need for any locking), does no dynamic memory allocation and is much faster than the built in "printf's" in MinGW/TDM-GCC.
It also fixes all the bugs I'm aware of in the MinGW/TDM-GCC implementations, these bugs and the slow speed are the main reason this exists. 
In comparison to stb_sprintf (v1.08) it fixes all the bugs I found, implements more of the C99 formats and adds long doubles, 128 bit ints and 128 bit floats.
Finally, it provides a consistant format string specification across different targets which aids portability between targets and compilers.

It assumes a compiler that supports at least C99, it assumes chars are 8 bit ascii, int's are 32 bits, long can be 32/64 bits and long long int is 64 bits. Pointers can be 32 or 64 bits.
Floating point numbers are assumed to be  IEEE 754 format, with double as 64 bits, long double as 80 bits (which may be stored in up to 128 bits depending on the api in use)
 and __float128's being 128 bits all of which are used. Integers are asumed to be stored in 2's complement representation.
These assumptions are true for almost all processors manufactured in the last 10+ years (Intel X32 & X64, ARM, PowerPC, etc).

Basic use:
  #define YA_SP_SPRINTF_IMPLEMENTATION
  #define YA_SP_SPRINTF_DEFAULT  // with this defined printf etc will call the functions in this file 
  #include "ya_sprintf.h"
   ..
  printf("Hello world\n 1+2=%d\n",1+2); // use the library 


You also need to compile it with double-double.c which contains a library of maths functions that use two floating point variables to gain extra precision 
(effectively twice the mantissa resolution of a single variable).
This library supports 3 sets of functions using doubles, long doubles and __float128's
This library can also be used standalone where extra precision is required in calculations.

Also supplied are:
atof.c - a MIT licensed implementation of strtod() for floats, doubles, long doubles and __float128's . This is used in the test program, but can be used standalone
main.c	- a test program that checks all functions of ya_sprintf.h

To compile the test program under Linux try:
 gcc -Wall -Ofast -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope -fstack-protector-all -g3  main.c atof.c double-double.c -lasan -lquadmath -lm  -o test
 
 then ./test to run
 
 Under Windows with TDM-GCC 9.2.0 
  gcc -Wall -Ofast main.c atof.c double-double.c -lquadmath -o test
   
  then test.exe to run
   
*/
/* 
The format string is:

  Zero or more flags (in any order), which modify the meaning of the conversion specification.
  Duplicate flags are not allowed so for example %005d would be considered as the 0 flag followed by a field width of 05 (decimal). 

  An optional minimum field width. If the converted value has fewer bytes than the field width, it will be padded with <space> characters by default on the left;
	 it will be padded on the right if the left-adjustment flag ( '-' ), described below, is given to the field width.
	  The field width takes the form of an <asterisk> ( '*' ), described below, or a decimal integer.

  An optional precision that gives the minimum number of digits to appear for the d, i, o, u, x, and X conversion specifiers;
	 the number of digits to appear after the decimal point for the a, A, e, E, f, and F conversion specifiers;
	 the maximum number of significant digits for the g and G conversion specifiers; 
	 or the maximum number of bytes to be printed from a string in the s 
	 
	 The precision takes the form of a <period> ( '.' ) followed either by an <asterisk> ( '*' ), described below, 
	 or an optional decimal digit string, where a null digit string is treated as zero. 
	 The precision when printing floating point numbers is clamped at 350 (this allows the maximium buffer size to be fixed).
	 If a precision appears with any other conversion specifier it is ignored.

  An optional length modifier that specifies the size of the argument.

  A conversion specifier character that indicates the type of conversion to be applied.

The field width, or precision, or both, may be indicated by an <asterisk> ( '*' ).
 In this case an argument of type int supplies the field width or precision.
 The arguments specifying field width, or precision, or both must appear in that order before the argument to be converted.
 A negative field width is taken as a '-' flag followed by a positive field width. 
 A negative precision is taken as if the precision were omitted.

The flag characters and their meanings are:

 ' 	The integer portion of the result of a decimal conversion ( %i, %d, %u, %f, %F, %g, or %G ) will be formatted with thousands' grouping character (default is a comma). 
 	As an example "%'d" on 12345 would print 12,345
	For other conversions it is ignored. Note this is a POSIX.1-2017 extension to C99.
 -  The result of the conversion is left-justified within the field. 
	The conversion is right-justified if this flag is not specified.
 +	The result of a signed conversion will always begin with a sign ( '+' or '-' ). 
	The conversion will begin with a sign only when a negative value is converted if this flag is not specified.
 <space>
    If the first character of a signed conversion is not a sign or if a signed conversion results in no characters,
	 a <space> will be prefixed to the result. 
	 If the <space> and '+' flags both appear, the <space> flag is ignored.
 #  Specifies that the value is to be converted to an alternative form. 
	For o conversion, it increases the precision, if and only if necessary, to force the first digit of the result to be a zero 
	(if the value and precision are both 0, a single 0 is printed). 
	For x or X conversion specifiers, a non-zero result has 0x (or 0X) prefixed to it. 
	For b or B conversion specifiers, a non-zero result has 0b (or 0B) prefixed to it. 
	For a, A, e, E, f, F, g, and G conversion specifiers, the result always contain a decimal point character, even if no digits follow the decimal point. 
	Without this flag, a decimal point appears in the result of these conversions only if a digit follows it. 
	For g and G conversion specifiers, trailing zeros are not be removed from the result as they normally are. 
	For other conversion specifiers, this flag is ignored.
 0  For d, i, o, u, x, X, b, B, a, A, e, E, f, F, g, and G conversion specifiers, leading zeros (following any indication of sign or base) are used to pad to the field width
	 rather than performing space padding, except when converting an infinity or NaN. 
	 If the '0' and '-' flags both appear, the '0' flag is ignored. 
	 For d, i, o, u, x, X, b and B conversion specifiers, if a precision is specified, the '0' flag is ignored. 
	 If the '0' and <apostrophe> flags both appear, the grouping characters are inserted after zero padding (Note this is different to POSIX.1-2017 which specifies <before> zero padding).
	 For other conversions the 0 flag is ignored.
 The following are not defined by C99 or POSIX.1-2017. 
 $  For integers and floats, you can use a "$" specifier and the number
	will be converted to float and then divided to get kilo, mega, giga or
	tera , etc and then printed, so "%$d" 1000 is "1.0 k", "%$.2d" 2536000 is "2.53 M", etc.
	For byte values (where k is 1024 rather than 1000) , use two $:s, like "%$$d" to turn 2536000 to "2.42 Mi".
	If you prefer JEDEC suffixes to SI ones, use three $:s: "%$$$d" -> "2.42 M".
	To remove the space between the number and the suffix, add "_" specifier: "%_$d" -> "2.53M".
	The complete list of suffixes is KMGTPEZY.
	Note that numbers < 1000 will have no suffix added.

The length modifiers and their meanings are:

 hh	a following d, i, o, u, x, X, b or B conversion specifier applies to a signed char or unsigned char argument 
	(the argument will have been promoted according to the integer promotions, but its value is converted to signed char or unsigned char before printing);
	 or that a following n conversion specifier applies to a pointer to a signed char argument.
 h  a following d, i, o, u, x, X, b or B conversion specifier applies to a short or unsigned short argument
	(the argument will have been promoted according to the integer promotions, but its value is converted to short or unsigned short before printing);
	 or that a following n conversion specifier applies to a pointer to a short argument.
 l  a following d, i, o, u, x, X, b or B conversion specifier applies to a long or unsigned long argument;
	that a following n conversion specifier applies to a pointer to a long argument
	or has no effect on a following a, A, e, E, f, F, g, or G conversion specifier.
 ll a following d, i, o, u, x, X, b or B conversion specifier applies to a long long or unsigned long long argument;
	or that a following n conversion specifier applies to a pointer to a long long argument.
 j 	a following d, i, o, u, x, X, b or B conversion specifier applies to an intmax_t or uintmax_t argument;
	or that a following n conversion specifier applies to a pointer to an intmax_t argument.
 z  a following d, i, o, u, x, X, b or B conversion specifier applies to a size_t or the corresponding signed integer type argument;
	or that a following n conversion specifier applies to a pointer to a signed integer type corresponding to a size_t argument.
 t  a following d, i, o, u, x, X, b or B conversion specifier applies to a ptrdiff_t or the corresponding unsigned type argument;
	or that a following n conversion specifier applies to a pointer to a ptrdiff_t argument.
 L  a following a, A, e, E, f, F, g, or G conversion specifier applies to a long double argument.
 
 The following are not defined by C99 or POSIX.1-2017. quadmath_snprintf() does support %Q but thats all it supports and then it only prints 1 argument at a time. 
 Q   a following a, A, e, E, f, F, g, or G conversion specifier applies to a __float128 argument;
	that a following d, i, o, u, x, X, b or B conversion specifier applies to an __int128 or unsigned __int128 argument;
	or that a following n conversion specifier applies to a pointer to an __int128 argument.
 I	 a following d, i, o, u, x, X, b or B conversion specifier applies to a void *
 I32  a following d, i, o, u, x, X, b or B conversion specifier applies to int32_t or uint32_t
	 or that a following n conversion specifier applies to a pointer to an int32_t argument.
 I64  a following d, i, o, u, x, X, b or B conversion specifier applies to int64_t or uint64_t
	 or that a following n conversion specifier applies to a pointer to an int64_t argument.
 I128 a following d, i, o, u, x, X, b or B conversion specifier applies to __int128 or __uint128 
     the following a, A, e, E, f, F, g, or G conversion specifier applies to a __float128;
     or that a following n conversion specifier applies to a pointer to an __int128 argument.


If a length modifier appears with any conversion specifier other than as specified above, it is ignored.

The conversion specifiers and their meanings are:

 d, i
    The int argument is converted to a signed decimal in the style "[-]dddd". 
	The precision specifies the minimum number of digits to appear; if the value being converted can be represented in fewer digits, it is expanded with leading zeros. 
	The default precision is 1. 
	The result of converting zero with an explicit precision of zero is no characters.
 o  The unsigned argument is converted to unsigned octal format in the style "dddd".
	The precision specifies the minimum number of digits to appear; if the value being converted can be represented in fewer digits, it is expanded with leading zeros.
	The default precision is 1. The result of converting zero with an explicit precision of zero is no characters.
 u  The unsigned argument is converted to unsigned decimal format in the style "dddd".
	The precision specifies the minimum number of digits to appear; if the value being converted can be represented in fewer digits, it is expanded with leading zeros.
	The default precision is 1. The result of converting zero with an explicit precision of zero is no characters.
 x  The unsigned argument is converted to unsigned hexadecimal format in the style "dddd"; the letters "abcdef" are used.
	The precision specifies the minimum number of digits to appear; if the value being converted can be represented in fewer digits, it is expanded with leading zeros.
	The default precision is 1. The result of converting zero with an explicit precision of zero is no characters.
 X  Equivalent to the x conversion specifier, except that letters "ABCDEF" are used instead of "abcdef" .
 b	The unsigned argument to binary in the format "dddd" where each digit is either 0 or 1
	The precision specifies the minimum number of digits to appear; if the value being converted can be represented in fewer digits, it is expanded with leading zeros.
	The default precision is 1. The result of converting zero with an explicit precision of zero is no characters.
 B  Equivalent to the b conversion specifier.
	Note the b and B conversion specifiers are not defined by C99 or or POSIX.1-2017.
 f, F
    The double argument is converted to decimal notation in the style "[-]ddd.ddd", where the number of digits after the decimal point is equal to the precision specification.
	If the precision is missing, it is taken as 6; if the precision is explicitly zero and no '#' flag is present, no decimal point will appear.
	If a decimal point appears, at least one digit appears before it. 
	The low-order digit is rounded towards even.
    A double argument representing an infinity is converted into "[-]inf".
	A double argument representing a NaN is converted into "[-]nan" if YA_SP_SIGNED_NANS is defined before "ya_sprintf.h" is included or "nan" otherwise
	The F conversion specifier produces "INF", or "NAN" instead of "inf", or "nan", respectively.
 e, E
    The double argument is converted in the style "[-]d.ddde±dd", where there is one digit before the decimal point (which is non-zero if the argument is non-zero)
	 and the number of digits after it is equal to the precision; if the precision is missing, it is taken as 6;
	 if the precision is zero and no '#' flag is present, no decimal point appears.
	The low-order digit is rounded towards even.
	The E conversion specifier produces a number with 'E' instead of 'e' introducing the exponent.
	The exponent always contain at least two digits. If the value is zero, the exponent is zero.
    A double argument representing an infinity or NaN is converted in the same way as for an f or F conversion specifier.
 g, G
    The double argument representing a floating-point number is converted in the style f or e (or in the style F or E in the case of a G conversion specifier), depending on the value converted and the precision.
	Let P equal the precision if non-zero, 6 if the precision is omitted, or 1 if the precision is zero. Then, if a conversion with style E would have an exponent of X:

        If P > X>=-4, the conversion is with style f (or F ) and precision P -( X+1).

        Otherwise, the conversion is with style e (or E ) and precision P -1.

    Finally, unless the '#' flag is used, any trailing zeros is removed from the fractional portion of the result and the decimal-point character is removed if there is no fractional portion remaining.
    A double argument representing an infinity or NaN is converted in the same way as for an f or F conversion specifier.
 a, A
    A double argument representing a floating-point number is converted in the style "[-]0xh.hhhhp±d", where there is one hexadecimal digit 
	(which is non-zero if the argument is a normalized floating-point number and is otherwise unspecified) before the decimal-point character and the number of hexadecimal digits after it is equal to the precision;
	if the precision is missing then the precision is sufficient for an exact representation of the value;
	if the precision is zero and the '#' flag is not specified, no decimal-point character is present.
	The letters "abcdef" is used for a conversion and the letters "ABCDEF" for A conversion. 
	The A conversion specifier produces a number with 'X' and 'P' instead of 'x' and 'p'.
	The exponent always contains at least one digit, and only as many more digits as necessary to represent the decimal exponent of 2. If the value is zero, the exponent is zero.
    A double argument representing an infinity or NaN is converted in the same way as for an f or F conversion specifier.
 c  The int argument is converted to an unsigned char, and the resulting byte is written.
 s  The argument is a pointer to an array of char. Bytes from the array is written up to (but not including) any terminating null byte.
    If the precision is specified, no more than that many bytes is written.
	If the precision is not specified or is greater than the size of the array, the application must ensure that the array contains a null byte.
 p  The argument is a pointer to void. The value of the pointer is converted to a hex number.
	If YA_SP_LINUX_STYLE is defined before including ya_sprintf the hex number will be preceded with 0x
 n  The argument is a pointer to an integer into which is written the number of bytes written to the output so far by this call to one of the fprintf() functions. No argument is converted.
 %  Print a '%' character; no argument is converted. The complete conversion specification is %%.

If a conversion specification does not match one of the above forms, the format is just treated as a string and "printed" as is. 
If any argument is not the correct type for the corresponding conversion specification, the behaviour is undefined (and the application may crash).

In no case will a non-existent or small field width cause truncation of a field; if the result of a conversion is wider than the field width, the field is expanded to contain the conversion result.
Characters generated by fprintf() and printf() are printed as if fputc() had been called.

For the a and A conversion specifiers, the value is rounded to a hexadecimal floating number with the given precision.
If YA_SP_LINUX_STYLE is defined before including ya_sprintf the rounding is towards even.

For the e, E, f, F, g, and G conversion specifiers, if the number of significant decimal digits is at most 19, then the result will be correctly rounded (towards even).
This means that float and doubles can be printed exactly (the value output from va_sprintf if fed to fast_strtod() will give a bitwise identical value to the original double),
 but long doubles or __float128's cannot always be printed exactly if this would need more than 20 digits.
%a/%A will always print an exact representation of float, double, long double and __float128.

*/

/*
LICENSE:  See end of file for license information.
*/

#ifndef YA_SP_SPRINTF_H_INCLUDE
#define YA_SP_SPRINTF_H_INCLUDE

/*

API:
====
 int ya_s_sprintf( char * buf, char const * fmt, ... )
 int ya_s_snprintf( char * buf, int count, char const * fmt, ... )
  Convert an arg list into a buffer.  ya_s_snprintf always returns
  a zero-terminated string (unlike regular snprintf).

 int ya_s_vsprintf( char * buf, char const * fmt, va_list va )
 int ya_s_vsnprintf( char * buf, int count, char const * fmt, va_list va )
  Convert a va_list arg list into a buffer.  ya_s_vsnprintf always returns
  a zero-terminated string (unlike regular snprintf).

 int ya_s_vsprintfcb( YA_S_SPRINTFCB * callback, void * user, char * buf, char const * fmt, va_list va )
    typedef char * YA_S_SPRINTFCB( char const * buf, void * user, int len );
  Convert into a buffer, calling back every YA_SP_SPRINTF_MIN chars.
  Your callback can then copy the chars out, print them or whatever.
  This function is actually the workhorse for everything else.
  The buffer you pass in must hold at least YA_SP_SPRINTF_MIN characters.
  you return the next buffer to use or 0 to stop converting
    
The following 2 functions write to streams (files):    
 int ya_s_vfprintfFILE *stream, const char *format, va_list va)
 int ya_s_fprintf(FILE *stream, const char *format, ... )

The next 2 functions write to stdout:
 int ya_s_vprintf(const char *format, va_list va) 
 int ya_s_printf(const char *format, ...)    

 void ya_s_set_separators( char comma, char period )
  Set the comma and period (decimal point) characters to use.

*/

/* compile time options are done by defining (#define) specific names before # including ya_sprintf.h
#define YA_SP_SPRINTF_STATIC // make the definitions of the exported functions static
#define YA_SP_SPRINTF_MIN XXX // XXX is the number of characters per callback , default 512 (see ya_s_vsprintfcb() above)
#define STB_SPRINTF_IMPLEMENTATION // for backwards compatibility with stb_sprintf()
#define YA_SP_SPRINTF_DECORATE PREFIX // define the names of the exported functions as PREFIXname , default ya_s_. If this is not defined then vsprintf, vsnprintf, sprintf, snprintf, vfprintf, vprintf, fprintf & printf are defined via macros to equal the ya_s_ versions.
#define YA_SP_SPRINTF_IMPLEMENTATION // actually include code from header file (see "use" at the start of this file for examples)
#define YA_SP_SPRINTF_NOFLOAT // no floating point support (saves space if you don't need it)
#define YA_SP_SPRINTF_LD // support long doubles (compiler must support a long double data type that has more mantissa bits than a plain double).
#define YA_SP_SPRINTF_Q // support int128 and float128 output (needs YA_SP_SPRINTF_LD defined as well and compiler must support these types )
#define YA_SP_NO_DIGITPAIR // selects an alternative way to convert numbers to ascii characters. This may or may not be faster. Its likley this option will be removed in future releases.
#define YA_SP_LINUX_STYLE // make subtle changes to the output to match gcc 9.3.0 under Ubuntu . By default matches TDM-GCC 9.2.0 under windows 10 with #define __USE_MINGW_ANSI_STDIO 1
#define YA_SP_SIGNED_NANS // if defined print NAN's as signed numbers. Default is that a NAN is considered unsigned.
*/


#ifdef YA_SP_SPRINTF_STATIC
#define YA_S__PUBLICDEC static
#define YA_S__PUBLICDEF static 
#else
#ifdef __cplusplus
#define YA_S__PUBLICDEC extern "C"
#define YA_S__PUBLICDEF extern "C" 
#else
#define YA_S__PUBLICDEC extern
#define YA_S__PUBLICDEF 
#endif
#endif

#include <stdarg.h> // for va_list()
#include <stddef.h> // size_t, ptrdiff_t

#ifndef YA_SP_SPRINTF_MIN
#define YA_SP_SPRINTF_MIN 512 // how many characters per callback
#endif
typedef char *YA_S_SPRINTFCB(const char *buf, void *user, int len);

#ifdef STB_SPRINTF_IMPLEMENTATION  /* provide some backwards compatability with stb_sprintf for common situation where header file is just included into program */
 #define YA_SP_SPRINTF_DECORATE(name) stbsp_##name  /* make function names the same as they were */
 #define YA_SP_SPRINTF_IMPLEMENTATION /* this is the implementation */
#endif



#ifndef YA_SP_SPRINTF_DECORATE
#define YA_SP_SPRINTF_DECORATE(name) ya_s_##name // define this before including if you want to change the names
 #ifdef YA_SP_SPRINTF_DEFAULT  /* define all printf() family to use routines in this file by using macros [needed for c++ under gcc at least where printf is native c++ and here its defined as C linkage] */
  #define vsprintf ya_s_vsprintf
  #define vsnprintf ya_s_vsnprintf
  #define sprintf ya_s_sprintf
  #define snprintf ya_s_snprintf
  #define vfprintf ya_s_vfprintf
  #define vprintf ya_s_vprintf
  #define fprintf ya_s_fprintf
  #define printf ya_s_printf
 #endif 
#endif

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va);
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vsnprintf)(char *buf, int count, char const *fmt, va_list va);
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...);
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...);

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vsprintfcb)(YA_S_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va);
YA_S__PUBLICDEF void YA_SP_SPRINTF_DECORATE(set_separators)(char comma, char period);

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vfprintf)(FILE *stream, const char *format, va_list va);

// as above but to stdout
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vprintf)(const char *format, va_list va);

// fprintf() : write to stream (file)
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(fprintf)(FILE *stream, const char *format, ...);

// printf(): like fprintf() but to stdout
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(printf) (const char *format, ...);

#endif // YA_SP_SPRINTF_H_INCLUDE

#ifdef YA_SP_SPRINTF_IMPLEMENTATION

#include <stdlib.h> // for va_arg()
#include <string.h> // strlen() , memcpy(), memset() 
#include <stdint.h>  /* for int64_t etc */
#include "double-double.h"
#include "table10.h"

/* the line below defines GCC_OPTIMIZE_AWARE to 1 when we can use # pragma GCC optimize ("-O2") */
#define YA_SP_GCC_OPTIMIZE_AWARE (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__clang__)
// define the level of gcc optimisations used as we cannot use Ofast as with gcc 9.3.0 on ubuntu this gives incorrect results around NAN's even in main test program
#if YA_SP_GCC_OPTIMIZE_AWARE
#pragma GCC push_options
#pragma GCC optimize ("-O3") /* cannot use Ofast */
#endif


#ifndef YA_SP_SPRINTF_NOFLOAT
// internal float utility functions
#if defined(YA_SP_SPRINTF_Q ) && !defined(YA_SP_SPRINTF_LD)
#define YA_SP_SPRINTF_LD /* need _LD if _Q is defined */
#endif

#ifdef YA_SP_SPRINTF_LD
static int32_t ya_s__real_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, long double value, uint32_t frac_digits);
#else 
static int32_t ya_s__real_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, double value, uint32_t frac_digits);
#endif
#define YA_S__SPECIAL 0x7000

#define YA_SP_STATIC_ASSERT(condition) extern int static_assert_##__FILE__##__LINE__[!!(condition)-1]  /* check at compile time and works in a global or function context see https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */
#ifdef YA_SP_SPRINTF_LD
YA_SP_STATIC_ASSERT( sizeof(long double) > sizeof(double)); // to use USE_LD long double needs more precision than double.
#endif
#ifdef YA_SP_SPRINTF_Q /* 128 bit support needs __float128 and __int128 's */
#ifdef __SIZEOF_INT128__  /* if 128 bit integers are supported */
typedef __uint128_t ya_s__uint128_t; // same format as used above
typedef __int128_t ya_s__int128_t;
typedef __float128 ya_s__f128_t;
#else
#error "need __int128 and __float128 to support Q (define YA_S__Q)"
#endif
#endif
#endif


static char ya_s__period = '.';
static char ya_s__comma = ',';
#ifndef YA_SP_NO_DIGITPAIR
static struct
{
   short temp; // force next field to be 2-byte aligned
   char pair[201];
} ya_s__digitpair =
{
  0,
   "00010203040506070809101112131415161718192021222324"
   "25262728293031323334353637383940414243444546474849"
   "50515253545556575859606162636465666768697071727374"
   "75767778798081828384858687888990919293949596979899"
};
#endif
YA_S__PUBLICDEF void YA_SP_SPRINTF_DECORATE(set_separators)(char pcomma, char pperiod)
{
   ya_s__period = pperiod;
   ya_s__comma = pcomma;
}

/* following constants must increase as powers of 2 */
#define YA_S__LEFTJUST 1
#define YA_S__LEADINGPLUS 2
#define YA_S__LEADINGSPACE 4
#define YA_S__LEADING_0X 8  /* set when # found in format specifier */
#define YA_S__LEADINGZERO 16
#define YA_S__INTMAX 32
#define YA_S__TRIPLET_COMMA 64
#define YA_S__NEGATIVE 128
#define YA_S__METRIC_SUFFIX 256
#define YA_S__HALFWIDTH 512
#define YA_S__METRIC_NOSPACE 1024
#define YA_S__METRIC_1024 2048
#define YA_S__METRIC_JEDEC 4096
#define YA_S__QUARTWIDTH 8192
#define YA_S__L 16384 /* %Lg etc for long double - needs YA_SP_SPRINTF_LD defined to work */
#define YA_S__Q 32768 /* %Qg etc for __float128 - needs YA_SP_SPRINTF_Q defined to work */

static void ya_s__lead_sign(uint32_t fl, char *sign)
{
   sign[0] = 0;
   if (fl & YA_S__NEGATIVE) {
      sign[0] = 1;
      sign[1] = '-';
   } else if (fl & YA_S__LEADINGPLUS) { // if both space and + present in flags then space is ignored, so test for + first.
      sign[0] = 1;
      sign[1] = '+';      
   } else if (fl & YA_S__LEADINGSPACE) {
      sign[0] = 1;
      sign[1] = ' ';
   }
}

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vsprintfcb)(YA_S_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va)
{
   static char hex[] = "0123456789abcdefxp";
   static char hexu[] = "0123456789ABCDEFXP";
   char *bf;
   char const *f;
   int tlen = 0;
   if(fmt==NULL) return 0; // PMi - just in case ! 
   bf = buf;
   f = fmt;
   for (;;) {
      int32_t fw, pr, tz;
      uint32_t fl;

      // macros for the callback buffer stuff
      #define ya_s__chk_cb_bufL(bytes)                        \
         {                                                     \
            int len = (int)(bf - buf);                         \
            if ((len + (bytes)) >= YA_SP_SPRINTF_MIN) {          \
               tlen += len;                                    \
               if (0 == (bf = buf = callback(buf, user, len))) \
                  goto done;                                   \
            }                                                  \
         }
      #define ya_s__chk_cb_buf(bytes)    \
         {                                \
            if (callback) {               \
               ya_s__chk_cb_bufL(bytes); \
            }                             \
         }
      #define ya_s__flush_cb()                      \
         {                                           \
            ya_s__chk_cb_bufL(YA_SP_SPRINTF_MIN - 1); \
         } // flush if there is even one byte in the buffer
      // PMI - added if(lg<0) and V? checks in macro below to ensure it always returns a positive number   
      #define ya_s__cb_buf_clamp(cl, v)                \
         cl = (v)>0?(v):0;                              \
         if (callback) {                                \
            int lg = YA_SP_SPRINTF_MIN - (int)(bf - buf); \
            if(lg<0) lg=0;                              \
            if (cl > lg)                                \
               cl = lg;                                 \
         }

      // fast copy everything up to the next % (or end of string)
      for (;;) {
			// simple loop to check for % or 0 and copy to output buffer if neither
            if (*f == '%')
               goto scandd;
            if (*f == 0)
               goto endfmt;
            ya_s__chk_cb_buf(1);
            *bf++ = *f++;         
      }
   scandd:

      ++f;

      // ok, we have a percent, read the modifiers first
      fw = 0;
      pr = -1;
      fl = 0;
      tz = 0;

      // flags
      for (;;) {
         switch (f[0]) {
         // if we have left justify
         case '-':
         	if(fl & YA_S__LEFTJUST) goto flags_done; // duplicate flags are not allowed
            fl |= YA_S__LEFTJUST;
            ++f;
            continue;
         // if we have leading plus
         case '+':
         	if(fl & YA_S__LEADINGPLUS) goto flags_done; // duplicate flags are not allowed
            fl |= YA_S__LEADINGPLUS;
            ++f;
            continue;
         // if we have leading space
         case ' ':
         	if(fl & YA_S__LEADINGSPACE) goto flags_done; // duplicate flags are not allowed
            fl |= YA_S__LEADINGSPACE;
            ++f;
            continue;
         // if we have leading 0x
         case '#':
         	if(fl & YA_S__LEADING_0X) goto flags_done; // duplicate flags are not allowed
            fl |= YA_S__LEADING_0X;
            ++f;
            continue;
         // if we have thousand commas
         case '\'':
         	if(fl & YA_S__TRIPLET_COMMA) goto flags_done; // duplicate flags are not allowed
            fl |= YA_S__TRIPLET_COMMA;
            ++f;
            continue;
         // if we have kilo marker (none->kilo->kibi->jedec)
         case '$':
            if (fl & YA_S__METRIC_SUFFIX) {
               if (fl & YA_S__METRIC_1024) {
               	  if(fl & YA_S__METRIC_JEDEC) goto flags_done; // too many $ signs
                  fl |= YA_S__METRIC_JEDEC;
               } else {
                  fl |= YA_S__METRIC_1024;
               }
            } else {
               fl |= YA_S__METRIC_SUFFIX;
            }
            ++f;
            continue;
         // if we don't want space between metric suffix and number
         case '_':
         	if(fl & YA_S__METRIC_NOSPACE) goto flags_done; // duplicate flags are not allowed
            fl |= YA_S__METRIC_NOSPACE;
            ++f;
            continue;
         // if we have leading zero
         case '0':
         	if(fl & YA_S__LEADINGZERO) goto flags_done; // duplicate flags are not allowed
            fl |= YA_S__LEADINGZERO;
            ++f;
            continue;
         default: goto flags_done;
         }
      }
   flags_done:
      // deal with invalid combinations
      if( (fl & YA_S__LEADINGZERO) && (fl & YA_S__LEFTJUST ))
          fl &= ~YA_S__LEADINGZERO; // cannot have 0 and -
      // get the field width
      if (f[0] == '*') {
         fw = va_arg(va, uint32_t);
         if(fw<0)
         	{fl |= YA_S__LEFTJUST; // -neg field width is taken as negative flag followed by a positive field width [C99 standard]
         	 fw= -fw;
         	}
         ++f;
      } else {
         while ((f[0] >= '0') && (f[0] <= '9')) {
            fw = fw * 10 + f[0] - '0';
            f++;
         }
      }
      // get the precision
      if (f[0] == '.') {
         ++f;
         if (f[0] == '*') {
            pr = va_arg(va, uint32_t);
            if(pr<0) pr= -1;// C99 standard: if a negative precision given behave as if no precision is specified
            ++f;
         } else {
            pr = 0;
            while ((f[0] >= '0') && (f[0] <= '9')) {
               pr = pr * 10 + f[0] - '0';
               f++;
            }
         }
        // deal with invalid combinations
        if((fl & YA_S__LEADINGZERO) && pr != -1 ) 
          fl &= ~YA_S__LEADINGZERO; // cannot have 0 flag when precision specified
      }

      // handle integer and double size overrides
      switch (f[0]) {
      // are we halfwidth?
      case 'h':
         fl |= YA_S__HALFWIDTH;
         ++f;
         if (f[0] == 'h')
         	{fl |= YA_S__QUARTWIDTH;
         	 fl &= ~ YA_S__HALFWIDTH; // not 1/2 width
             ++f;  // QUARTERWIDTH
        	}
         break;
      // are we 64-bit (unix style)
      case 'l':
         fl |= ((sizeof(long) == 8) ? YA_S__INTMAX : 0);
         ++f;
         if (f[0] == 'l') {
            fl |= YA_S__INTMAX;
            ++f;
         }
         break;
      // are we 64-bit on intmax? (c99)
      case 'j':
         fl |= (sizeof(intmax_t) == 8) ? YA_S__INTMAX : 0;// PMi was sizeof(size_t)
         ++f;
         break;
      // are we 64-bit on size_t or ptrdiff_t? (c99)
      case 'z':
         fl |= (sizeof(size_t) == 8) ? YA_S__INTMAX : 0; // PMi was sizeof(ptrdiff_t)
         ++f;
         break;
      case 't':
         fl |= (sizeof(ptrdiff_t) == 8) ? YA_S__INTMAX : 0;
         ++f;
         break;
      // are we 64-bit (msft style)
      case 'I':
         if ((f[1] == '6') && (f[2] == '4')) {
            fl |= YA_S__INTMAX;
            f += 3;
         } else if ((f[1] == '3') && (f[2] == '2')) {
            f += 3;
#ifdef YA_SP_SPRINTF_Q             
         } else if ((f[1] == '1') && (f[2] == '2')&& (f[3] == '8') ) {
         	fl|= YA_S__Q; // I128 = Quad double (__float128,__int128)
            f += 4;
#endif             
         }  
		 else {
            fl |= ((sizeof(void *) == 8) ? YA_S__INTMAX : 0);
            ++f;
         }
         break;
#ifdef YA_SP_SPRINTF_LD       
	  case 'L': fl|= YA_S__L; // long double 
	  		++f;
			break;
#endif	
#ifdef YA_SP_SPRINTF_Q       
	  case 'Q': fl|= YA_S__Q; // Quad double (__float128,__int128)
	  		++f;
			break;
#endif			  	      
      default: break;
      }
     char lead[8];
     char tail[10];
     lead[0]=0;// make sure lead and tail are correctly initialised, 1st element is count of elements actually used.
     tail[0]=0;
      // handle each replacement
      switch (f[0]) {
#ifdef YA_SP_SPRINTF_LD   
		 #define YA_S__NUMSZ 20000 // clipped to 350 digits after decimal point and could have 4932 before plus commas... 
#else      	
         #define YA_S__NUMSZ 1024 // clipped to 350 digits after decimal point and could have 308 before plus commas...
#endif         
         char num[YA_S__NUMSZ];

         char *s;
         char const *h;
         uint32_t l, n, cs;
         uint64_t n64;
#ifndef YA_SP_SPRINTF_NOFLOAT
         double fv;
#ifdef YA_SP_SPRINTF_LD       
	  	 long double fvL; // long double
#endif	
#ifdef YA_SP_SPRINTF_Q       
	      __float128 fv128;  // Quad double (__float128)
	      ya_s__uint128_t u128; // 128 bit integer
#endif	         
#endif
         int32_t dp;
         char const *sn;

      case 's':
         // get the string
         s = va_arg(va, char *);
         if (s == NULL)  // PMi was 0 
#ifdef YA_SP_LINUX_STYLE
			{if(pr!= -1 && pr<strlen("(null)") )
				s="" ; // print nothing for null strings when a precision is specified thats too small to fully print "(null)"
			 else
				s = "(null)"; // PMi was "null" changed to match built in sprintf()	
			}
#else         
            s = "(null)"; // PMi was "null" changed to match built in sprintf()
#endif            
         // get the length
		l=strlen(s); // assume this is faster than code below, its certainly clearer.         
         // clamp to precision
         if (l > (uint32_t)pr)
            l = pr;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         // copy the string in
         goto scopy;

      case 'c': // char
         // get the character
         s = num + YA_S__NUMSZ - 1;
         *s = (char)va_arg(va, int);
         l = 1;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         goto scopy;

      case 'n': // weird write-bytes specifier
      {
        /* PMi need to write to the correct size pointer   */
        if(fl & YA_S__QUARTWIDTH) // %hhn
        	{signed char *d=va_arg(va, signed char *);
         	 *d = tlen + (int)(bf - buf);
     		}
     	 else if(fl & YA_S__HALFWIDTH) // %hn
        	{short *d=va_arg(va, short *);
         	 *d = tlen + (int)(bf - buf);
     		}
     	 else if(fl & YA_S__INTMAX) // %lln
        	{int64_t *d=va_arg(va, int64_t *);
         	 *d = tlen + (int)(bf - buf);
     		}	
#ifdef YA_SP_SPRINTF_Q  
     	 else if(fl & YA_S__Q) // %Qn
        	{ya_s__int128_t *d=va_arg(va, ya_s__int128_t *);
         	 *d = tlen + (int)(bf - buf);
     		}	
#endif     		
		 else		 
		 	{	// *n (or %ln) 		 	
         	 int *d = va_arg(va, int *);
         	 *d = tlen + (int)(bf - buf);
         	}
      } break;

#ifdef YA_SP_SPRINTF_NOFLOAT
      case 'A':              // float
      case 'a':              // hex float
      case 'G':              // float
      case 'g':              // float
      case 'E':              // float
      case 'e':              // float
      case 'f':              // float
      case 'F':				 // float
         va_arg(va, double); // eat it
         s = (char *)"No float";
         l = 8;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         goto scopy;
#else
      case 'A': // A hex float
      case 'a': // a hex float
         h = (f[0] == 'A') ? hexu : hex;
#ifdef YA_SP_SPRINTF_LD       
	     if(fl & YA_S__L) // long double 
	     	{
			 fvL=va_arg(va, long double);		 
         	 if (isnan(fvL) || isinf(fvL)) 
		   		{
				 fl &= ~YA_S__LEADINGZERO;// special (nan,inf) don't have leading zeros
				 if(isupper(f[0]))
					{	
#ifdef YA_SP_SIGNED_NANS
                     if(isnan(fvL))
                       {
                        if(signbit(fvL)) sn="-NAN";
				 	    else		      sn= "NAN";
				       }
#else					
				 	 if(isnan(fvL)) 	sn="NAN";
#endif				 	 
				 	 else if(signbit(fvL))	sn="-INF";
				 	 else		sn="INF";	
					}
				 else
					{
#ifdef YA_SP_SIGNED_NANS
                     if(isnan(fvL))
                       {
                        if(signbit(fvL)) sn="-nan";
				 	    else		      sn= "nan";
				 	   }
#else								
				 	 if(isnan(fvL)) 	sn="nan";
#endif				 	 
				 	 else if(signbit(fvL))	sn="-inf";
				 	 else 				sn="inf";
					}							
            	 s = (char *)sn;
            	 l=strlen(s);
            	 cs = 0;
            	 pr = 0;
            	 goto scopy;
           		} 			 
           	 // the code below uses long doubles even for doubles when 	YA_SP_SPRINTF_LD defined
			}
	  	 else
#endif	
#ifdef YA_SP_SPRINTF_Q       
	  	   if(fl & YA_S__Q) //__float128  
	  	   	{
			 fv128=va_arg(va, __float128);
         	 if (isnanq(fv128) || isinfq(fv128)) 
		   		{
				 fl &= ~YA_S__LEADINGZERO;// special (nan,inf) don't have leading zeros
				 if(isupper(f[0]))
					{	
#ifdef YA_SP_SIGNED_NANS
                     if(isnanq(fv128))
                      {
                       if(signbitq(fv128)) sn="-NAN";
				 	   else		      sn= "NAN";
				      }
#else							
				 	 if(isnanq(fv128)) 	sn="NAN";
#endif				 	 
				 	 else if(signbitq(fv128))	sn="-INF";
				 	 else 				sn="INF";
					}
				 else
					{
#ifdef YA_SP_SIGNED_NANS
                     if(isnanq(fv128))
                      {
                       if(signbitq(fv128)) sn="-nan";
				 	   else		      sn= "nan";
				      }
#else								
				 	 if(isnanq(fv128)) 	sn="nan";
#endif				 	 
				 	 else if(signbitq(fv128))	sn="-inf";
				 	 else 				sn="inf";
					}							
            	 s = (char *)sn;
            	 l=strlen(s);
            	 cs = 0;
            	 pr = 0;
            	 goto scopy;
           		} 			 
			  // we need to process float128 differently to others as mantissa is > 64 bits (we will use ya_s__uya_s__int128_t for mantissa)
			 int origpr=pr;
		 	 if(pr== -1) pr=28; // default - full resolution of 112 bit float128 mantissa
		 	 if(signbitq(fv128) )
		 		{fl |= YA_S__NEGATIVE;
			 	 fv128= -fv128;	
				}
		 	 s = num + 64;
         	 ya_s__lead_sign(fl, lead);
		 	 ya_s__f128_t dmant=frexpq(fv128,&dp); // get mantissa and exponent, 0.5<=dmant<1
		 	 //  quadmath_snprintf(%a) prints 1.xxx whereas ya_s_snprintf(%a) by default prints 8.xxx [which is better as it gives us more resolution for a given number of digits ]
		 	 // also does not let exponent go below -16382
		 	 ya_s__uint128_t n128=rintq(ldexpq(dmant,113)); // convert mantissa to a uint128 by multiplying by 2^113
		 	 if(fv128!=0)
			 	dp-=1; // have 1.x before decimal point		
		 	 if(dp< -16382)
		 		{n128>>=(-16382-dp);
		 		 // n128 >>= 1;
			 	 dp= -16382;
				}		  
	        if (pr < 28)
	            {
				 n128 += (((ya_s__uint128_t)8) << (108 - pr*4)) ; // rounding
	             n128 &= ~ (((ya_s__uint128_t)8) << (108 - pr*4)) ; // get rid of bit we added for rounding as it messes up trailing zero detection below
	             if(n128 & ((ya_s__uint128_t)16)<<112 ) // we use & 15 (0x0f) below to extract 1st digit so 16 is 1 bit up
	             	{// rounding caused overflow, fix this
	             	 n128>>=1; // divide by 2 (we did not actually overflow n128, just the range we are using) and adjust exponent to compensate
	             	 dp++;
	             	}
	        	}	
  	
			// add leading chars
	         lead[1 + lead[0]] = '0';
	         lead[2 + lead[0]] = (f[0] == 'A') ?'X':'x';
	         lead[0] += 2;
	         *s++ = h[(n128 >> 112) & 15];//  we want 1. for 1st digit as thats what quadmath_snprintf () generates
	         n128 ^= n128 & ((ya_s__uint128_t)15)<<112 ; // delete digit just printed
	         n128 <<= 4;
	         if (origpr>=0 || (origpr<0 && n128 !=0) || (fl & YA_S__LEADING_0X) ) // PMi only print period if non-zero digits after dp, or user requested it (eg %.5A) or %#A (LEADING_0X flag)
	            *s++ = ya_s__period;
	         sn = s;
	
	         // print the bits
	         if(pr<0) n=0; // PMi
	         else n = pr;
	         if (n > 31)
	            n = 31;          
	         if (pr > (int32_t)n)
	            tz = pr - n;
	         pr = 0;
	         while (n-- && (origpr!= -1 || n128!=0)) 
			 	{ // PMi only print if rest are non-zero or user requested it (eg %.5A)
	             *s++ = h[(n128 >> 112) & 15];
	             n128 ^= n128 & ((ya_s__uint128_t)15)<<112 ; // delete digit just printed
	             n128 <<= 4;
	         	}
			 goto a_pr_axp; // print exponent like long double
			}			 
	  	 else
#endif         
         	{fv = va_arg(va, double); // warning - this may be part of a prior else (so only 1 argument is "eaten")
         	 if (isnan(fv) || isinf(fv)) 
		   		{
				 fl &= ~YA_S__LEADINGZERO;// special (nan,inf) don't have leading zeros
				 if(isupper(f[0]))
					{
#ifdef YA_SP_SIGNED_NANS
                     if(isnan(fv))
                       {
                        if(signbit(fv)) sn="-NAN";
				 	    else		      sn= "NAN";
				       }
#else						
				 	 if(isnan(fv)) 	sn="NAN";
#endif				 	 
				 	 else if(signbit(fv))	sn="-INF";
				 	 else 			sn="INF";
					}
				 else
					{
#ifdef YA_SP_SIGNED_NANS
                     if(isnan(fv))
                       {
                        if(signbit(fv)) sn="-nan";
				 	    else		      sn= "nan";
				       }
#else						
				 	 if(isnan(fv)) 	sn="nan";
#endif				 	 
				 	 else if(signbit(fv))	sn="-inf";
				 	 else 			sn="inf";
					}							
            	 s = (char *)sn;
            	 l=strlen(s);
            	 cs = 0;
            	 pr = 0;
            	 goto scopy;
           		}
			}
#ifdef YA_SP_SPRINTF_LD    
		 int origpr=pr;
		 if(fl & YA_S__L) // long double 
		 	{ if(pr== -1) pr=15; // default - full resolution 64 bit mantissa is present [ total of 16 hex digits as 1 before DP ]
		 	}
		 else // just normal double
		 	{ if(pr==-1) pr=13 ; // all thats needed for full resolution with a double (52 bit mantissa + hidden bit )	
		 	  fvL=fv; // using long doubles now
		 	}
		 if(signbit(fvL) && !isnan(fvL))
		 	{fl |= YA_S__NEGATIVE;
			 fvL= -fvL;	
			}
		 s = num + 64;
         ya_s__lead_sign(fl, lead);
		 long double dmant=frexpl(fvL,&dp); // get mantissa and exponent, 0.5<=dmant<1
	 	 n64=(uint64_t)rintl(ldexpl(dmant,64)); // convert mantissa to a uint64 by multiplying by 2^64
		 if(fvL!=0)
		 		dp-=4; // have 1 hex digit before decimal point
		 // printf("\n n64=0x%llx, dmant=%Lg ldexpl(dmant,64)=%Lg lrintl(ldexpl(dmant,64))=%Lg dp=%d\n",n64,dmant,ldexpl(dmant,64),rintl(ldexpl(dmant,64)),dp);
#else		 		       
         int origpr=pr;
         if (pr == -1)
            pr = 13; // default is 13 - C standard pp 279 requires default to be full resolution
         // get the individual parts of the double
		 if(signbit(fv) && !isnan(fv))
		 	{fl |= YA_S__NEGATIVE;
			 fv= -fv;	
			}
		 s = num + 64;
         ya_s__lead_sign(fl, lead);
		 double dmant=frexp(fv,&dp); // get mantissa and exponent, 0.5<=dmant<1
	 	 n64=(uint64_t)rint(ldexp(dmant,64)); // convert mantissa to a uint64 by multiplying by 2^64
		 if(fv!=0)
		 		dp-=4; // have 1 hex digit before decimal point	 	 
#endif			  
#ifdef YA_SP_LINUX_STYLE
		// %a is printed as 1p+0 rather than 8p-3 also min exponent is clamped to -1022
		if((fl & YA_S__L)==0 && n64!=0)  // ! long double [ ie a double]  & n64!=0
			{n64>>=3;
			 dp+=3;
             if(dp< -1022)
               {n64>>=(-1022-dp);
                dp=-1022;
               }		 	
			}
		 else if(fl & YA_S__L)	// long double
		 	{
		 	 if(dp<-16385) // denorm
			 	{n64>>=(-16385-dp);
			 	 dp=-16385;
			 	}
		 	}
         if (pr < 15)
            {uint64_t orig_n64=n64;
			// ieee round to even
			uint64_t rnd=(((uint64_t)8) << (56-pr*4) ); // bit to add for rounding
			if(((n64+rnd)& (rnd|(rnd-1))) == 0 && (n64&(rnd<<1)) ==0) 
				{ // do nothing as already even
				}
			else	            
			 	{n64 += rnd; // rounding
                 n64 &= ~ rnd; // PMi : get rid of bit we added for rounding as it messes up trailing zero detection below
            	}
             if(n64<orig_n64)
             	{// rounding caused overflow, fix this
             	 n64=(((uint64_t)1) << 60);
             	 dp++;
             	}
        	}				
		
#else       
         if (pr < 15)
            {uint64_t orig_n64=n64;
             uint64_t rnd=(((uint64_t)8) << (56-pr*4) ); // bit to add for rounding
			 n64 += rnd; // rounding
             n64 &= ~ rnd; // PMi : get rid of bit we added for rounding as it messes up trailing zero detection below
             if(n64<orig_n64)
             	{// rounding caused overflow, fix this
             	 n64=(((uint64_t)8) << 60);
             	 dp++;
             	}
        	}	
 #endif 	
// add leading chars
         lead[1 + lead[0]] = '0';
         lead[2 + lead[0]] = (f[0] == 'A') ?'X':'x';
         lead[0] += 2;
         *s++ = h[(n64 >> 60) & 15];
         n64 <<= 4;
         if (origpr>=0 || (origpr<0 && n64 !=0) || (fl & YA_S__LEADING_0X) ) // PMi only print period if non-zero digits after dp, or user requested it (eg %.5A) or %#A (LEADING_0X flag)
            *s++ = ya_s__period;
         sn = s;

         // print the bits
         if(pr<0) n=0; // PMi
         else n = pr;
#ifdef YA_SP_SPRINTF_LD 
         if (n > 15)
            n = 15;
#else // original code         
         if (n > 13)
            n = 13;
#endif            
         if (pr > (int32_t)n)
            tz = pr - n;
         pr = 0;
         while (n-- && (origpr!= -1 || n64!=0)) { // PMi only print if rest are non-zero or user requested it (eg %.5A)
            *s++ = h[(n64 >> 60) & 15];
            n64 <<= 4;
         }
#ifdef  YA_SP_SPRINTF_Q 
a_pr_axp:
#endif	
         // print the expo
         tail[1] = h[17];
         if (dp < 0) {
            tail[2] = '-';
            dp = -dp;
         } else
            tail[2] = '+';
         n = dp>=10000? 7: ((dp >= 1000) ? 6 : ((dp >= 100) ? 5 : ((dp >= 10) ? 4 : 3)));
         tail[0] = (char)n;
         for (;;) {
            tail[n] = '0' + dp % 10;
            if (n <= 3)
               break;
            --n;
            dp /= 10;
         }

         dp = (int)(s - sn);
         l = (int)(s - (num + 64));
         s = num + 64;
         cs = 1 + (3 << 24);
         goto scopy;
       
      case 'G': // float
      case 'g': // float
         h = (f[0] == 'G') ? hexu : hex;
#ifdef YA_SP_SPRINTF_LD       
	     if(fl & YA_S__L) // long double 
	     	{
			 fvL=va_arg(va, long double);
			 fv=(double)fvL; // simple way to allow original code to work until something better is added
			}
	  	 else
#endif	
#ifdef YA_SP_SPRINTF_Q       
	  	   if(fl & YA_S__Q) //__float128  
	  	   	{
			 fv128=va_arg(va, __float128);
			 fv=(double)fv128; // simple way to allow original code to work until something better is added
			 fvL=(long double)fv128;
			}			 
	  	 else
#endif     
			{       
             fv = va_arg(va, double); // might be trailing part of an else ..
#ifdef YA_SP_SPRINTF_LD              
             fvL=fv; // process as long double
#endif             
        	}
         if (pr == -1)
            pr = 6; // default is 6
         else if (pr == 0)
            pr = 1; 
         if(pr>350) pr=350; // limit number of digits after dp to something sensible [so buffer size is limited]            
         // read the double into a string
#ifdef YA_SP_SPRINTF_LD 
		 if (ya_s__real_to_str(&sn, &l, num, &dp, fvL, (pr - 1) | 0x80000000))
#else         
         if (ya_s__real_to_str(&sn, &l, num, &dp, fv, (pr - 1) | 0x80000000))
#endif         
            fl |= YA_S__NEGATIVE;
		 if(fl & YA_S__LEADING_0X)
		 	{// # modifier applied 0=>0.00000 (5 zeros after dp)
         	 // should we use %e of %f ?
         	 // printf(" #g(%g) found l=%d dp=%d pr=%d n=%d\n",fv,l,dp,pr,n); 
         	 if ((dp <= -4) || (dp > pr) ) 
			    {
				 if (pr)
               		--pr; // when using %e, there is one digit before the decimal
            	 goto doexpfromg;
         		}
         	 else
			  	{ // use %f 
			  	  pr=pr-dp;
			  	  goto dofloatfromg;
				}
         	}
		 	 
         // clamp the precision and delete extra zeros after clamp
         n = pr;
         if (l > (uint32_t)pr)
              l = pr;

         while ((l > 1) && (pr) && (sn[l - 1] == '0')) 
		  	{
             --pr;
             --l;
         	}


         // should we use %e
         if ((dp <= -4) || (dp > (int32_t)n)) {
            if (pr > (int32_t)l)
               pr = l - 1;
            else if (pr)
               --pr; // when using %e, there is one digit before the decimal
            goto doexpfromg;
         }
         // this is the insane action to get the pr to match %g semantics for %f
         if (dp > 0) {
            pr = (dp < (int32_t)l) ? l - dp : 0;
         } else {
            pr = -dp + ((pr > (int32_t)l) ? (int32_t) l : pr);
         }
   	
         goto dofloatfromg;

      case 'E': // float
      case 'e': // float
         h = (f[0] == 'E') ? hexu : hex;
#ifdef YA_SP_SPRINTF_LD       
	     if(fl & YA_S__L) // long double 
	     	{
			 fvL=va_arg(va, long double);
			 fv=(double)fvL; // simple way to allow original code to work until something better is added
			}
	  	 else
#endif	
#ifdef YA_SP_SPRINTF_Q       
	  	   if(fl & YA_S__Q) //__float128  
	  	   	{
			 fv128=va_arg(va, __float128);
			 fv=(double)fv128; // simple way to allow original code to work until something better is added
			 fvL=(long double)fv128;
			}			 
	  	 else
#endif    
			{        
             fv = va_arg(va, double); // might be part of trailing else...
#ifdef YA_SP_SPRINTF_LD 
			 fvL=fv;
#endif
			}
         if (pr == -1)
            pr = 6; // default is 6
         if(pr>350) pr=350; // limit number of digits after dp to something sensible [so buffer size is limited]            
         // read the double into a string
#ifdef YA_SP_SPRINTF_LD      
		 if (ya_s__real_to_str(&sn, &l, num, &dp, fvL, pr | 0x80000000))  
#else  
         if (ya_s__real_to_str(&sn, &l, num, &dp, fv, pr | 0x80000000))
#endif         
            fl |= YA_S__NEGATIVE;
      doexpfromg:
         tail[0] = 0;
         ya_s__lead_sign(fl, lead);
         if (dp == YA_S__SPECIAL) {
			fl &= ~YA_S__LEADINGZERO;// special (nan,inf) don't have leading zeros
			if(isupper(f[0]))
				{
				 if(*sn=='n') sn="NAN";
				 else if(*sn=='i') sn="INF";
				}
            s = (char *)sn;
            cs = 0;
            pr = 0;
            goto scopy;
         }
         s = num + 64;
         // handle leading chars
         *s++ = sn[0];

         if (pr || (fl & YA_S__LEADING_0X))
            *s++ = ya_s__period;

         // handle after decimal
         if ((l - 1) > (uint32_t)pr)
            l = pr + 1;
         for (n = 1; n < l; n++)
            *s++ = sn[n];
         // trailing zeros
         tz = pr - (l - 1);
         pr = 0;
         // dump expo
         tail[1] = h[0xe];
         dp -= 1;
         if (dp < 0) {
            tail[2] = '-';
            dp = -dp;
         } else
            tail[2] = '+';
#ifdef YA_SP_SPRINTF_LD 
         n = dp>=1000?6:((dp >= 100) ? 5 : 4);
#else
         n = (dp >= 100) ? 5 : 4;
#endif
         tail[0] = (char)n;
         for (;;) {
            tail[n] = '0' + dp % 10;
            if (n <= 3)
               break;
            --n;
            dp /= 10;
         }
         cs = 1 + (3 << 24); // how many tens
         goto flt_lead;
         
	  case 'F': // float
      case 'f': // float
#ifdef YA_SP_SPRINTF_LD       
	     if(fl & YA_S__L) // long double 
	     	{
			 fvL=va_arg(va, long double);
			 fv=(double)fvL; // simple way to allow original code to work until something better is added
			}
	  	 else
#endif	
#ifdef YA_SP_SPRINTF_Q       
	  	   if(fl & YA_S__Q) //__float128  
	  	   	{
			 fv128=va_arg(va, __float128);
			 fv=(double)fv128; // simple way to allow original code to work until something better is added
			 fvL=(long double)fv128;
			}			 
	  	 else
#endif      
			{   
             fv = va_arg(va, double); // might be part of a trailing else ...
#ifdef YA_SP_SPRINTF_LD   
			 fvL=fv;
#endif             
         	}
      doafloat:
#ifdef YA_SP_SPRINTF_LD   
         // do kilos
         if (fl & YA_S__METRIC_SUFFIX) {
            long double divisor;
            divisor = 1000.0;
            if (fl & YA_S__METRIC_1024)
               divisor = 1024.0;
            while (fl < 0x8000000) {
               if ((fvL < divisor) && (fvL > -divisor))
                  break;
               fvL /= divisor;
               fl += 0x1000000;
            }
         }
#else      	
         // do kilos
         if (fl & YA_S__METRIC_SUFFIX) {
            double divisor;
            divisor = 1000.0f;
            if (fl & YA_S__METRIC_1024)
               divisor = 1024.0;
            while (fl < 0x8000000) {
               if ((fv < divisor) && (fv > -divisor))
                  break;
               fv /= divisor;
               fl += 0x1000000;
            }
         }
#endif         
         if (pr == -1)
            pr = 6; // default is 6
         if(pr>350) pr=350; // limit number of digits after dp to something sensible [so buffer size is limited]
         // read the double into a string
#ifdef YA_SP_SPRINTF_LD   
		 if (ya_s__real_to_str(&sn, &l, num, &dp, fvL, pr))
#else         
         if (ya_s__real_to_str(&sn, &l, num, &dp, fv, pr))
#endif         
            fl |= YA_S__NEGATIVE;
      dofloatfromg:
         tail[0] = 0;
         ya_s__lead_sign(fl, lead);
         if (dp == YA_S__SPECIAL) {
			fl &= ~YA_S__LEADINGZERO;// special (nan,inf) don't have leading zeros    
			if(isupper(f[0]))
				{
				 if(*sn=='n') sn="NAN";
				 else if(*sn=='i') sn="INF";
				}			     	
            s = (char *)sn;         	        	
            cs = 0;
            pr = 0;
            goto scopy;
         }
         s = num + 64;

         // handle the three decimal varieties
         if (dp <= 0) {
            int32_t i;
            // handle 0.000*000xxxx
            *s++ = '0';
            if (pr || (fl & YA_S__LEADING_0X))
               *s++ = ya_s__period;
            n = -dp;
            if ((int32_t)n > pr)
               n = pr;
            i = n;
			memset(s,'0',i); // set memory to digit zero
			s+=i; // increment pointer
          
            if ((int32_t)(l + n) > pr)
               l = pr - n;
            i = l;
			memcpy(s,sn,i);
			s+=i;
			sn+=i;         
            tz = pr - (n + l);
            cs = 1 + (3 << 24); // how many tens did we write (for commas below)
         } else {
            cs = (fl & YA_S__TRIPLET_COMMA) ? ((600 - (uint32_t)dp) % 3) : 0;
            if ((uint32_t)dp >= l) {
               // handle xxxx000*000.0
               n = 0;
               for (;;) {
                  if ((fl & YA_S__TRIPLET_COMMA) && (++cs == 4)) {
                     cs = 0;
                     *s++ = ya_s__comma;
                  } else {
                     *s++ = sn[n];
                     ++n;
                     if (n >= l)
                        break;
                  }
               }
               if (n < (uint32_t)dp) {
                  n = dp - n;
                  if ((fl & YA_S__TRIPLET_COMMA) == 0) {
				  	 memset(s,'0',n); // set memory to digit zero
				  	 s+=n; // increment pointer
				  	 n=0;                    
                  }                  
                  while (n) {
                     if ((fl & YA_S__TRIPLET_COMMA) && (++cs == 4)) {
                        cs = 0;
                        *s++ = ya_s__comma;
                     } else {
                        *s++ = '0';
                        --n;
                     }
                  }
               }
               cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
               if (pr || (fl & YA_S__LEADING_0X)) {
                  *s++ = ya_s__period;
                  tz = pr;
               }
            } else {
               // handle xxxxx.xxxx000*000
               n = 0;
               for (;;) {
                  if ((fl & YA_S__TRIPLET_COMMA) && (++cs == 4)) {
                     cs = 0;
                     *s++ = ya_s__comma;
                  } else {
                     *s++ = sn[n];
                     ++n;
                     if (n >= (uint32_t)dp)
                        break;
                  }
               }
               cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
               if (pr || (fl & YA_S__LEADING_0X))
                  *s++ = ya_s__period;
               if ((l - dp) > (uint32_t)pr)
                  l = pr + dp;
               while (n < l) {
                  *s++ = sn[n];
                  ++n;
               }
               tz = pr - (l - dp);
            }
         }
         pr = 0;

         // handle k,m,g,t, etc. [ only for positive powers of 10 > 3 ]
         if (fl & YA_S__METRIC_SUFFIX) {
            char idx;
            idx = 1;
            if (fl & YA_S__METRIC_NOSPACE)
               idx = 0;
            tail[0] = idx;
            tail[1] = ' ';
            {
               if (fl >> 24) { // SI kilo is 'k', JEDEC and SI kibits are 'K'.
                  if (fl & YA_S__METRIC_1024)
                     tail[idx + 1] = "_KMGTPEZY"[fl >> 24];
                  else
                     tail[idx + 1] = "_kMGTPEZY"[fl >> 24];
                  idx++;
                  // If printing kibits and not in jedec, add the 'i'. see https://physics.nist.gov/cuu/Units/binary.html so get Ki, Mi, Gi, Ti, etc.
                  if (fl & YA_S__METRIC_1024 && !(fl & YA_S__METRIC_JEDEC)) {
                     tail[idx + 1] = 'i';
                     idx++;
                  }
                  tail[0] = idx;
               }
            }
         };

      flt_lead:
         // get the length that we copied
         l = (uint32_t)(s - (num + 64));
         s = num + 64;
         goto scopy;
#endif

      case 'B': // upper binary
      case 'b': // lower binary
         h = (f[0] == 'B') ? hexu : hex;
         lead[0] = 0;
         if (fl & YA_S__LEADING_0X) {
            lead[0] = 2;
            lead[1] = '0';
            lead[2] = h[0xb];
         }
         l = (8 << 4) | (1 << 8);
         goto radixnum;

      case 'o': // octal
         h = hexu;
         lead[0] = 0;
         if (fl & YA_S__LEADING_0X) {
            lead[0] = 1;
            lead[1] = '0';
         }
         l = (3 << 4) | (3 << 8);
         goto radixnum;

      case 'p': // pointer
         fl |= (sizeof(void *) == 8) ? YA_S__INTMAX : 0;
#ifdef YA_SP_LINUX_STYLE  
         fl|= YA_S__LEADING_0X; // want leading 0x      
#else
         if(fw==0) pr = sizeof(void *) * 2; // if no field width specified print leading zeros    
         fl &= ~YA_S__LEADINGZERO; 
#endif         
           // fall through - to X

      case 'X': // upper hex
      case 'x': // lower hex
         h = (f[0] == 'X') ? hexu : hex;
         l = (4 << 4) | (4 << 8);
         lead[0] = 0;
         if (fl & YA_S__LEADING_0X) {
            lead[0] = 2;
            lead[1] = '0';
            lead[2] = h[16];
         }
      radixnum:
         // get the number
#ifdef YA_SP_SPRINTF_Q       
	  	 if(fl & YA_S__Q) //128 bits  
	  	   	{u128=va_arg(va, ya_s__uint128_t);// get 128 bit integer
	  	    }
	  	 else  if(fl & YA_S__INTMAX)
            u128 = va_arg(va, uint64_t);
         else
            u128 = va_arg(va, uint32_t);
		 if(fl & YA_S__QUARTWIDTH) // 8 bits
			u128 &= 0xff;
		 else if(fl & YA_S__HALFWIDTH) // 16 bits
			u128 &= 0xffff;	
         s = num + YA_S__NUMSZ;
         dp = 0;
         // clear tail, and clear leading if value is zero
         tail[0] = 0;
         if (u128 == 0) {
            lead[0] = 0;
            if (pr == 0) {
               l = 0;
               cs = (((l >> 4) & 15)) << 24;
               goto scopy;
            }
         }
         // convert to string
         for (;;) {
            *--s = h[u128 & ((1 << (l >> 8)) - 1)];
            u128 >>= (l >> 8);
            if (!((u128) || ((int32_t)((num + YA_S__NUMSZ) - s) < pr)))
               break;
            if (fl & YA_S__TRIPLET_COMMA) {
               ++l;
               if ((l & 15) == ((l >> 4) & 15)) {
                  l &= ~15;
                  *--s = ya_s__comma;
               }
            }
         };		    
#else // not YA_SP_SPRINTF_Q  ie 64 bit max				          
         if (fl & YA_S__INTMAX)
            n64 = va_arg(va, uint64_t);
         else
            n64 = va_arg(va, uint32_t);
		 if(fl & YA_S__QUARTWIDTH) // 8 bits
			n64 &= 0xff;
		 else if(fl & YA_S__HALFWIDTH) // 16 bits
			n64 &= 0xffff;	
         s = num + YA_S__NUMSZ;
         dp = 0;
         // clear tail, and clear leading if value is zero
         tail[0] = 0;
         if (n64 == 0) {
            lead[0] = 0;
            if (pr == 0) {
               l = 0;
               cs = (((l >> 4) & 15)) << 24;
               goto scopy;
            }
         }
         // convert to string
         for (;;) {
            *--s = h[n64 & ((1 << (l >> 8)) - 1)];
            n64 >>= (l >> 8);
            if (!((n64) || ((int32_t)((num + YA_S__NUMSZ) - s) < pr)))
               break;
            if (fl & YA_S__TRIPLET_COMMA) {
               ++l;
               if ((l & 15) == ((l >> 4) & 15)) {
                  l &= ~15;
                  *--s = ya_s__comma;
               }
            }
         };
#endif         
         // get the tens and the comma pos
         cs = (uint32_t)((num + YA_S__NUMSZ) - s) + ((((l >> 4) & 15)) << 24);
         // get the length that we copied
         l = (uint32_t)((num + YA_S__NUMSZ) - s);
         // copy it
         goto scopy;

      case 'u': // unsigned
#ifdef YA_SP_LINUX_STYLE
		// space and + flags don't apply to unsigned so make sure they are not set
        fl &= ~YA_S__LEADINGPLUS;
		fl &= ~YA_S__LEADINGSPACE;
#endif      
      case 'i':
      case 'd': // integer
         // get the integer and abs it
#ifdef YA_SP_SPRINTF_Q  
		u128=0; // stop gcc complaining about possibly uninitislised u128
		if (fl & YA_S__Q) 
			{// 128 bit integer
             ya_s__int128_t i128 = va_arg(va, ya_s__int128_t);
             u128 = (ya_s__uint128_t)i128;
             if ((f[0] != 'u') && (i128 < 0)) 
				{
			     u128=~u128+1; // same as u128=-i128 but avoids issues when processing MIN_INT             	 
               	 fl |= YA_S__NEGATIVE;
            	}
            }
		 else
#endif		 	         
          if (fl & YA_S__INTMAX) {
            int64_t i64 = va_arg(va, int64_t);
            n64 = (uint64_t)i64;
            if ((f[0] != 'u') && (i64 < 0)) {
			   n64=~n64+1; // same as n64=-i64 but avoids issues when processing MIN_INT              
               fl |= YA_S__NEGATIVE;
            }
         } else {
            int32_t i = va_arg(va, int32_t);
            n64 = (uint32_t)i;
            if ((f[0] != 'u') && (i < 0)) 
				{					
				 uint32_t n=(uint32_t)(i);
				 n=~n+1; // same as -n but avoids issues when processing MIN_INT				 
                 n64 = n;
                 fl |= YA_S__NEGATIVE;
            	}
			if(fl & YA_S__QUARTWIDTH) // 8 bits
				n64 &= 0xff;
			else if(fl & YA_S__HALFWIDTH) // 16 bits
				n64 &= 0xffff;				            	
            if(f[0] != 'u')
				{// for signed 1/2 and 1/4 width need to deal with negative values here 
				 if(fl & YA_S__QUARTWIDTH) // 8 bits
					{if(n64>127) 
						{n64=256-n64; // 128->128 and -ve, 129 -> 127 and -ve
						 fl |= YA_S__NEGATIVE;
						}
					}
				 if(fl & YA_S__HALFWIDTH) // 16 bits
					{if(n64>32767) 
						{n64=32768-n64; // 
						 fl |= YA_S__NEGATIVE;
						}
					}
				}
					
					 	
         }

#ifndef YA_SP_SPRINTF_NOFLOAT
         if (fl & YA_S__METRIC_SUFFIX) {
#ifdef YA_SP_SPRINTF_Q      	
            if (((fl & YA_S__Q)?u128:n64) < 1024)
#else
			if (n64 < 1024)
#endif            
               pr = 0;
            else if (pr == -1)
               pr = 1;
#ifdef YA_SP_SPRINTF_LD  
#ifdef YA_SP_SPRINTF_Q  
		if (fl & YA_S__Q) // 128 bit integer
			fvL= u128;
		else
#endif			
            fvL = (long double)(uint64_t)n64; // needs to be unsigned as we have already stripped sign from signed numbers, but could be an unsigned number (%u)
#else       
#ifdef YA_SP_SPRINTF_Q  
		if (fl & YA_S__Q) // 128 bit integer
			fv= u128;
		else
#endif        
            fv = (double)(uint64_t)n64; // needs to be unsigned as we have already stripped sign from signed numbers, but could be an unsigned number (%u)
#endif            
            goto doafloat;
         }
#endif

         // convert to string
         s = num + YA_S__NUMSZ;
         l = 0;
#ifdef YA_SP_SPRINTF_Q  
		if (fl & YA_S__Q) 
		{// print 128 bit unsigned integer [ sign if required is dealt with above ]
         for (;;) {
            // do in 32-bit chunks (avoid lots of 128-bit divides even with constant denominators)
            char *o = s - 8;
            if (u128 >= 100000000) {
               n = (uint32_t)(u128 % 100000000);
               u128 /= 100000000;
            } else {
               n = (uint32_t)u128;
               u128 = 0;
            }
            if ((fl & YA_S__TRIPLET_COMMA) == 0) {
               do {
#ifdef YA_SP_NO_DIGITPAIR     
				  // do it the simple way - 8 bits at a time
                  *--s = (char)(n % 10) + '0';
                  n /= 10;                             
#else
				  // original code using a table lookup 2 digits at a time
                  s -= 2;
                  *(uint16_t *)s = *(uint16_t *)&ya_s__digitpair.pair[(n % 100) * 2];
                  n /= 100;
#endif                  
               } while (n);
            }
            while (n) {
               if ((fl & YA_S__TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = ya_s__comma;
                  --o;
               } else {
                  *--s = (char)(n % 10) + '0';
                  n /= 10;
               }
            }
            if (u128 == 0) {
               if ((s < (num + YA_S__NUMSZ)) && *s=='0') // original checked *s first before checking s was valid
                  ++s;
               break;
            }
            while (s != o)
               if ((fl & YA_S__TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = ya_s__comma;
                  --o;
               } else {
                  *--s = '0';
               }
         }		
		}
		else
#endif		
		{// print u64
         for (;;) {
            // do in 32-bit chunks (avoid lots of 64-bit divides even with constant denominators)
            char *o = s - 8;
            if (n64 >= 100000000) {
               n = (uint32_t)(n64 % 100000000);
               n64 /= 100000000;
            } else {
               n = (uint32_t)n64;
               n64 = 0;
            }
            if ((fl & YA_S__TRIPLET_COMMA) == 0) {
               do {

#ifdef YA_SP_NO_DIGITPAIR
				  // simple solution - do one digit at a time avoiding table lookup
                  *--s = (char)(n % 10) + '0';// avoid table lookup
                  n /= 10;                 
#else              
				  // original code using table lookup to convert 2 digits at a time
                  s -= 2;    
                  *(uint16_t *)s = *(uint16_t *)&ya_s__digitpair.pair[(n % 100) * 2];
                  n /= 100;
#endif                  
               } while (n);
            }
            while (n) {
               if ((fl & YA_S__TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = ya_s__comma;
                  --o;
               } else {
                  *--s = (char)(n % 10) + '0';
                  n /= 10;
               }
            }
            if (n64 == 0) {
               if ( (s < (num + YA_S__NUMSZ)) && (s[0] == '0'))  // was if ((s[0] == '0') && (s != (num + YA_S__NUMSZ))), but this would access s[0] before checking access was valid
                  ++s;
               break;
            }
            while (s != o)
               if ((fl & YA_S__TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = ya_s__comma;
                  --o;
               } else {
                  *--s = '0';
               }
         }
		}
         tail[0] = 0;
         ya_s__lead_sign(fl, lead);

         // get the length that we copied
         l = (uint32_t)((num + YA_S__NUMSZ) - s);
         if (l == 0) {
            *--s = '0';
            l = 1;
         }
         cs = l + (3 << 24);
         if (pr < 0)
            pr = 0;

      scopy:
         // get fw=leading/trailing space, pr=leading zeros
         if (pr < (int32_t)l)
            pr = l;
         n = pr + lead[0] + tail[0] + tz;
         if (fw < (int32_t)n)
            fw = n;
         fw -= n;
         pr -= l;

         // handle right justify and leading zeros
         if ((fl & YA_S__LEFTJUST) == 0) {
            if (fl & YA_S__LEADINGZERO) // if leading zeros, everything is in pr
            {
               pr = (fw > pr) ? fw : pr;
               fw = 0;
            } else {
               fl &= ~YA_S__TRIPLET_COMMA; // if no leading zeros, then no commas
            }
         }

         // copy the spaces and/or zeros
         if (fw + pr) {
            int32_t i;
            uint32_t c;

            // copy leading spaces (or when doing %8.4d stuff)
            if ((fl & YA_S__LEFTJUST) == 0)
               while (fw > 0) {
                  ya_s__cb_buf_clamp(i, fw);
                  fw -= i;
				  memset(bf,' ',i); // set memory to space
				  bf+=i; // increment pointer                
                  ya_s__chk_cb_buf(1);
               }

            // copy leader
            sn = lead + 1;
            while (lead[0]) {
               ya_s__chk_cb_buf(1);
               ya_s__cb_buf_clamp(i, lead[0]);
               lead[0] -= (char)i;
            	memcpy(bf,sn,i);  // memcpy should be faster than anything we invent (and string could be reasonably large to make this a worthwhile gain)
				bf+=i;
				sn+=i;               
               ya_s__chk_cb_buf(1);
            }

            // copy leading zeros
            c = cs >> 24;
            cs &= 0xffffff;
            cs = (fl & YA_S__TRIPLET_COMMA) ? ((uint32_t)(c - ((pr + cs) % (c + 1)))) : 0;
            while (pr > 0) {
               ya_s__cb_buf_clamp(i, pr);
               pr -= i;
               if ((fl & YA_S__TRIPLET_COMMA) == 0) {
				  memset(bf,'0',i); // set memory to digit zero
				  bf+=i; // increment pointer
				  i=0;
               }
               while (i) {// set to digit zero with comma every c digits
                  if ((fl & YA_S__TRIPLET_COMMA) && (cs++ == c)) {
                     cs = 0;
                     *bf++ = ya_s__comma;
                  } else
                     *bf++ = '0';
                  --i;
               }
               ya_s__chk_cb_buf(1);
            }
         }

         // copy leader if there is still one
         sn = lead + 1;
         while (lead[0]>0) { // was while lead[0] 
            int32_t i;
            ya_s__chk_cb_buf(1);
            ya_s__cb_buf_clamp(i, lead[0]);
            lead[0] -= i;
            memcpy(bf,sn,i);  // memcpy should be faster than anything we invent (and string could be reasonably large to make this a worthwhile gain)
			bf+=i;
			sn+=i;           
            ya_s__chk_cb_buf(1);
         }

         // copy the string
         n = l;
         while (n) {
            int32_t i;
            ya_s__chk_cb_buf(1);
            ya_s__cb_buf_clamp(i, n);
            n -= i;
            memcpy(bf,s,i);  // memcpy should be faster than anything we invent (and string could be reasonably large to make this a worthwhile gain)
			bf+=i;
			s+=i;                
         }

         // copy trailing zeros
         while (tz) {
            int32_t i;
            ya_s__chk_cb_buf(1);
            ya_s__cb_buf_clamp(i, tz);
            tz -= i;
			memset(bf,'0',i); // set memory to digit zero
			bf+=i; // increment pointer
            ya_s__chk_cb_buf(1);
         }

         // copy tail if there is one
         sn = tail + 1;
         while (tail[0]) {
            int32_t i;
            ya_s__chk_cb_buf(1);
            ya_s__cb_buf_clamp(i, tail[0]);
            tail[0] -= (char)i;
            memcpy(bf,sn,i);  // memcpy should be faster than anything we invent (and string could be reasonably large to make this a worthwhile gain)
			bf+=i;
			sn+=i;               
            ya_s__chk_cb_buf(1);
         }

         // handle the left justify
         if (fl & YA_S__LEFTJUST)
            if (fw > 0) {
               while (fw) {
                  int32_t i;
                  ya_s__chk_cb_buf(1);
                  ya_s__cb_buf_clamp(i, fw);
                  fw -= i;
				  memset(bf,' ',i); // set memory to space
				  bf+=i; // increment pointer                  
                  ya_s__chk_cb_buf(1);
               }
            }
         break;

      default: // unknown, just copy code
         s = num + YA_S__NUMSZ - 1;
         *s = f[0];
         l = 1;
         fw = fl = 0;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         goto scopy;
      }
      ++f;
   }
endfmt:

   if (!callback)
      *bf = 0;
   else
      ya_s__flush_cb();

done:
   return tlen + (int)(bf - buf);
}

// cleanup
#undef YA_S__LEFTJUST
#undef YA_S__LEADINGPLUS
#undef YA_S__LEADINGSPACE
#undef YA_S__LEADING_0X
#undef YA_S__LEADINGZERO
#undef YA_S__INTMAX
#undef YA_S__TRIPLET_COMMA
#undef YA_S__NEGATIVE
#undef YA_S__METRIC_SUFFIX
#undef YA_S__NUMSZ
#undef ya_s__chk_cb_bufL
#undef ya_s__chk_cb_buf
#undef ya_s__flush_cb
#undef ya_s__cb_buf_clamp

// ============================================================================
//   wrapper functions

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...)
{
   int result;
   va_list va;
   va_start(va, fmt);
   result = YA_SP_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
   va_end(va);
   return result;
}

typedef struct ya_s__context {
   char *buf;
   int count;
   int length;
   char tmp[YA_SP_SPRINTF_MIN];
} ya_s__context;

static char *ya_s__clamp_callback(const char *buf, void *user, int len)
{
   ya_s__context *c = (ya_s__context *)user;
   c->length += len;

   if (len > c->count)
      len = c->count;

   if (len) {
      if (buf != c->buf) {
         const char *s, *se;
         char *d;
         d = c->buf;
         s = buf;
         se = buf + len;
         do {
            *d++ = *s++;
         } while (s < se);
      }
      c->buf += len;
      c->count -= len;
   }

   if (c->count <= 0)
      return c->tmp;
   return (c->count >= YA_SP_SPRINTF_MIN) ? c->buf : c->tmp; // go direct into buffer if you can
}

static char * ya_s__count_clamp_callback( const char * buf, void * user, int len )
{
   ya_s__context * c = (ya_s__context*)user;
   (void) sizeof(buf);

   c->length += len;
   return c->tmp; // go direct into buffer if you can
}

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE( vsnprintf )( char * buf, int count, char const * fmt, va_list va )
{
   ya_s__context c;

   if ( (count == 0) && !buf )
   {
      c.length = 0;

      YA_SP_SPRINTF_DECORATE( vsprintfcb )( ya_s__count_clamp_callback, &c, c.tmp, fmt, va );
   }
   else
   {
      int l;

      c.buf = buf;
      c.count = count;
      c.length = 0;

      YA_SP_SPRINTF_DECORATE( vsprintfcb )( ya_s__clamp_callback, &c, ya_s__clamp_callback(0,&c,0), fmt, va );

      // zero-terminate
      l = (int)( c.buf - buf );
      if ( l >= count ) // should never be greater, only equal (or less) than count
         l = count - 1;
      buf[l] = 0;
   }

   return c.length;
}

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...)
{
   int result;
   va_list va;
   va_start(va, fmt);

   result = YA_SP_SPRINTF_DECORATE(vsnprintf)(buf, count, fmt, va);
   va_end(va);

   return result;
}

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va)
{
   return YA_SP_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
}

/* add definitions for fprintf() etc
*/
typedef struct ya_s__fcontext {
   int length;
   FILE *fp;
   char tmp[YA_SP_SPRINTF_MIN];
} ya_s__fcontext;

static char * ya_s__f_callback( const char * buf, void * user, int len ) // actually write to required stream
{
   ya_s__fcontext * c = (ya_s__fcontext*)user;
   c->length += len;
   if(len)
   		fwrite(buf,sizeof(char),len,c->fp); // actually write to stream specified
   return c->tmp; // use the buffer in the struct
}

// vfprintf() : write to stream (file)
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vfprintf)(FILE *stream, const char *format, va_list va)
{
    ya_s__fcontext c;
    c.fp=stream;
    c.length = 0;
    YA_SP_SPRINTF_DECORATE(vsprintfcb)( ya_s__f_callback, &c, ya_s__f_callback(0,&c,0), format, va );
   return c.length;	
}

// as above but to stdout
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vprintf)(const char *format, va_list va)
{ return YA_SP_SPRINTF_DECORATE(vfprintf)(stdout,format,va);
}

// fprintf() : write to stream (file)
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(fprintf)(FILE *stream, const char *format, ...)
{
   int result;
   va_list va;
   va_start(va, format);
   result = YA_SP_SPRINTF_DECORATE(vfprintf)(stream, format, va);
   va_end(va);
   return result;	
}

// printf(): like fprintf() but to stdout
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(printf) (const char *format, ...)
{
   int result;
   va_list va;
   va_start(va, format);
   result = YA_SP_SPRINTF_DECORATE(vprintf)(format, va);
   va_end(va);
   return result;
}

// =======================================================================
//   low level float utility functions

#ifndef YA_SP_SPRINTF_NOFLOAT

#ifdef YA_SP_SPRINTF_LD
// version using long doubles for constants to give higher accuracy
 static const long double ldblpowersOf10[] =  /* positive powers of 10 array for long double from 0 to +4932 */
  {
   1e0L, 1e1L, 1e2L, 1e3L, 1e4L, 1e5L, 1e6L, 1e7L, 1e8L, 1e9L, 
   1e10L, 1e11L, 1e12L, 1e13L, 1e14L, 1e15L, 1e16L, 1e17L, 1e18L, 1e19L, 
   1e20L, 1e21L, 1e22L, 1e23L, 1e24L, 1e25L, 1e26L, 1e27L, 1e28L, 1e29L, 
   1e30L, 1e31L, 1e32L, 1e33L, 1e34L, 1e35L, 1e36L, 1e37L, 1e38L, 1e39L, 
   1e40L, 1e41L, 1e42L, 1e43L, 1e44L, 1e45L, 1e46L, 1e47L, 1e48L, 1e49L, 
   1e50L, 1e51L, 1e52L, 1e53L, 1e54L, 1e55L, 1e56L, 1e57L, 1e58L, 1e59L, 
   1e60L, 1e61L, 1e62L, 1e63L, 1e64L, 1e65L, 1e66L, 1e67L, 1e68L, 1e69L, 
   1e70L, 1e71L, 1e72L, 1e73L, 1e74L, 1e75L, 1e76L, 1e77L, 1e78L, 1e79L, 
   1e80L, 1e81L, 1e82L, 1e83L, 1e84L, 1e85L, 1e86L, 1e87L, 1e88L, 1e89L, 
   1e90L, 1e91L, 1e92L, 1e93L, 1e94L, 1e95L, 1e96L, 1e97L, 1e98L, 1e99L, 
   1e100L, 1e101L, 1e102L, 1e103L, 1e104L, 1e105L, 1e106L, 1e107L, 1e108L, 1e109L, 
   1e110L, 1e111L, 1e112L, 1e113L, 1e114L, 1e115L, 1e116L, 1e117L, 1e118L, 1e119L, 
   1e120L, 1e121L, 1e122L, 1e123L, 1e124L, 1e125L, 1e126L, 1e127L, 1e128L, 1e129L, 
   1e130L, 1e131L, 1e132L, 1e133L, 1e134L, 1e135L, 1e136L, 1e137L, 1e138L, 1e139L, 
   1e140L, 1e141L, 1e142L, 1e143L, 1e144L, 1e145L, 1e146L, 1e147L, 1e148L, 1e149L, 
   1e150L, 1e151L, 1e152L, 1e153L, 1e154L, 1e155L, 1e156L, 1e157L, 1e158L, 1e159L, 
   1e160L, 1e161L, 1e162L, 1e163L, 1e164L, 1e165L, 1e166L, 1e167L, 1e168L, 1e169L, 
   1e170L, 1e171L, 1e172L, 1e173L, 1e174L, 1e175L, 1e176L, 1e177L, 1e178L, 1e179L, 
   1e180L, 1e181L, 1e182L, 1e183L, 1e184L, 1e185L, 1e186L, 1e187L, 1e188L, 1e189L, 
   1e190L, 1e191L, 1e192L, 1e193L, 1e194L, 1e195L, 1e196L, 1e197L, 1e198L, 1e199L, 
   1e200L, 1e201L, 1e202L, 1e203L, 1e204L, 1e205L, 1e206L, 1e207L, 1e208L, 1e209L, 
   1e210L, 1e211L, 1e212L, 1e213L, 1e214L, 1e215L, 1e216L, 1e217L, 1e218L, 1e219L, 
   1e220L, 1e221L, 1e222L, 1e223L, 1e224L, 1e225L, 1e226L, 1e227L, 1e228L, 1e229L, 
   1e230L, 1e231L, 1e232L, 1e233L, 1e234L, 1e235L, 1e236L, 1e237L, 1e238L, 1e239L, 
   1e240L, 1e241L, 1e242L, 1e243L, 1e244L, 1e245L, 1e246L, 1e247L, 1e248L, 1e249L, 
   1e250L, 1e251L, 1e252L, 1e253L, 1e254L, 1e255L, 1e256L, 1e257L, 1e258L, 1e259L, 
   1e260L, 1e261L, 1e262L, 1e263L, 1e264L, 1e265L, 1e266L, 1e267L, 1e268L, 1e269L, 
   1e270L, 1e271L, 1e272L, 1e273L, 1e274L, 1e275L, 1e276L, 1e277L, 1e278L, 1e279L, 
   1e280L, 1e281L, 1e282L, 1e283L, 1e284L, 1e285L, 1e286L, 1e287L, 1e288L, 1e289L, 
   1e290L, 1e291L, 1e292L, 1e293L, 1e294L, 1e295L, 1e296L, 1e297L, 1e298L, 1e299L, 
   1e300L, 1e301L, 1e302L, 1e303L, 1e304L, 1e305L, 1e306L, 1e307L, 1e308L, 1e309L, 
   1e310L, 1e311L, 1e312L, 1e313L, 1e314L, 1e315L, 1e316L, 1e317L, 1e318L, 1e319L, 
   1e320L, 1e321L, 1e322L, 1e323L, 1e324L, 1e325L, 1e326L, 1e327L, 1e328L, 1e329L, 
   1e330L, 1e331L, 1e332L, 1e333L, 1e334L, 1e335L, 1e336L, 1e337L, 1e338L, 1e339L, 
   1e340L, 1e341L, 1e342L, 1e343L, 1e344L, 1e345L, 1e346L, 1e347L, 1e348L, 1e349L, 
   1e350L, 1e351L, 1e352L, 1e353L, 1e354L, 1e355L, 1e356L, 1e357L, 1e358L, 1e359L, 
   1e360L, 1e361L, 1e362L, 1e363L, 1e364L, 1e365L, 1e366L, 1e367L, 1e368L, 1e369L, 
   1e370L, 1e371L, 1e372L, 1e373L, 1e374L, 1e375L, 1e376L, 1e377L, 1e378L, 1e379L, 
   1e380L, 1e381L, 1e382L, 1e383L, 1e384L, 1e385L, 1e386L, 1e387L, 1e388L, 1e389L, 
   1e390L, 1e391L, 1e392L, 1e393L, 1e394L, 1e395L, 1e396L, 1e397L, 1e398L, 1e399L, 
   1e400L, 1e401L, 1e402L, 1e403L, 1e404L, 1e405L, 1e406L, 1e407L, 1e408L, 1e409L, 
   1e410L, 1e411L, 1e412L, 1e413L, 1e414L, 1e415L, 1e416L, 1e417L, 1e418L, 1e419L, 
   1e420L, 1e421L, 1e422L, 1e423L, 1e424L, 1e425L, 1e426L, 1e427L, 1e428L, 1e429L, 
   1e430L, 1e431L, 1e432L, 1e433L, 1e434L, 1e435L, 1e436L, 1e437L, 1e438L, 1e439L, 
   1e440L, 1e441L, 1e442L, 1e443L, 1e444L, 1e445L, 1e446L, 1e447L, 1e448L, 1e449L, 
   1e450L, 1e451L, 1e452L, 1e453L, 1e454L, 1e455L, 1e456L, 1e457L, 1e458L, 1e459L, 
   1e460L, 1e461L, 1e462L, 1e463L, 1e464L, 1e465L, 1e466L, 1e467L, 1e468L, 1e469L, 
   1e470L, 1e471L, 1e472L, 1e473L, 1e474L, 1e475L, 1e476L, 1e477L, 1e478L, 1e479L, 
   1e480L, 1e481L, 1e482L, 1e483L, 1e484L, 1e485L, 1e486L, 1e487L, 1e488L, 1e489L, 
   1e490L, 1e491L, 1e492L, 1e493L, 1e494L, 1e495L, 1e496L, 1e497L, 1e498L, 1e499L, 
   1e500L, 1e501L, 1e502L, 1e503L, 1e504L, 1e505L, 1e506L, 1e507L, 1e508L, 1e509L, 
   1e510L, 1e511L, 1e512L, 1e513L, 1e514L, 1e515L, 1e516L, 1e517L, 1e518L, 1e519L, 
   1e520L, 1e521L, 1e522L, 1e523L, 1e524L, 1e525L, 1e526L, 1e527L, 1e528L, 1e529L, 
   1e530L, 1e531L, 1e532L, 1e533L, 1e534L, 1e535L, 1e536L, 1e537L, 1e538L, 1e539L, 
   1e540L, 1e541L, 1e542L, 1e543L, 1e544L, 1e545L, 1e546L, 1e547L, 1e548L, 1e549L, 
   1e550L, 1e551L, 1e552L, 1e553L, 1e554L, 1e555L, 1e556L, 1e557L, 1e558L, 1e559L, 
   1e560L, 1e561L, 1e562L, 1e563L, 1e564L, 1e565L, 1e566L, 1e567L, 1e568L, 1e569L, 
   1e570L, 1e571L, 1e572L, 1e573L, 1e574L, 1e575L, 1e576L, 1e577L, 1e578L, 1e579L, 
   1e580L, 1e581L, 1e582L, 1e583L, 1e584L, 1e585L, 1e586L, 1e587L, 1e588L, 1e589L, 
   1e590L, 1e591L, 1e592L, 1e593L, 1e594L, 1e595L, 1e596L, 1e597L, 1e598L, 1e599L, 
   1e600L, 1e601L, 1e602L, 1e603L, 1e604L, 1e605L, 1e606L, 1e607L, 1e608L, 1e609L, 
   1e610L, 1e611L, 1e612L, 1e613L, 1e614L, 1e615L, 1e616L, 1e617L, 1e618L, 1e619L, 
   1e620L, 1e621L, 1e622L, 1e623L, 1e624L, 1e625L, 1e626L, 1e627L, 1e628L, 1e629L, 
   1e630L, 1e631L, 1e632L, 1e633L, 1e634L, 1e635L, 1e636L, 1e637L, 1e638L, 1e639L, 
   1e640L, 1e641L, 1e642L, 1e643L, 1e644L, 1e645L, 1e646L, 1e647L, 1e648L, 1e649L, 
   1e650L, 1e651L, 1e652L, 1e653L, 1e654L, 1e655L, 1e656L, 1e657L, 1e658L, 1e659L, 
   1e660L, 1e661L, 1e662L, 1e663L, 1e664L, 1e665L, 1e666L, 1e667L, 1e668L, 1e669L, 
   1e670L, 1e671L, 1e672L, 1e673L, 1e674L, 1e675L, 1e676L, 1e677L, 1e678L, 1e679L, 
   1e680L, 1e681L, 1e682L, 1e683L, 1e684L, 1e685L, 1e686L, 1e687L, 1e688L, 1e689L, 
   1e690L, 1e691L, 1e692L, 1e693L, 1e694L, 1e695L, 1e696L, 1e697L, 1e698L, 1e699L, 
   1e700L, 1e701L, 1e702L, 1e703L, 1e704L, 1e705L, 1e706L, 1e707L, 1e708L, 1e709L, 
   1e710L, 1e711L, 1e712L, 1e713L, 1e714L, 1e715L, 1e716L, 1e717L, 1e718L, 1e719L, 
   1e720L, 1e721L, 1e722L, 1e723L, 1e724L, 1e725L, 1e726L, 1e727L, 1e728L, 1e729L, 
   1e730L, 1e731L, 1e732L, 1e733L, 1e734L, 1e735L, 1e736L, 1e737L, 1e738L, 1e739L, 
   1e740L, 1e741L, 1e742L, 1e743L, 1e744L, 1e745L, 1e746L, 1e747L, 1e748L, 1e749L, 
   1e750L, 1e751L, 1e752L, 1e753L, 1e754L, 1e755L, 1e756L, 1e757L, 1e758L, 1e759L, 
   1e760L, 1e761L, 1e762L, 1e763L, 1e764L, 1e765L, 1e766L, 1e767L, 1e768L, 1e769L, 
   1e770L, 1e771L, 1e772L, 1e773L, 1e774L, 1e775L, 1e776L, 1e777L, 1e778L, 1e779L, 
   1e780L, 1e781L, 1e782L, 1e783L, 1e784L, 1e785L, 1e786L, 1e787L, 1e788L, 1e789L, 
   1e790L, 1e791L, 1e792L, 1e793L, 1e794L, 1e795L, 1e796L, 1e797L, 1e798L, 1e799L, 
   1e800L, 1e801L, 1e802L, 1e803L, 1e804L, 1e805L, 1e806L, 1e807L, 1e808L, 1e809L, 
   1e810L, 1e811L, 1e812L, 1e813L, 1e814L, 1e815L, 1e816L, 1e817L, 1e818L, 1e819L, 
   1e820L, 1e821L, 1e822L, 1e823L, 1e824L, 1e825L, 1e826L, 1e827L, 1e828L, 1e829L, 
   1e830L, 1e831L, 1e832L, 1e833L, 1e834L, 1e835L, 1e836L, 1e837L, 1e838L, 1e839L, 
   1e840L, 1e841L, 1e842L, 1e843L, 1e844L, 1e845L, 1e846L, 1e847L, 1e848L, 1e849L, 
   1e850L, 1e851L, 1e852L, 1e853L, 1e854L, 1e855L, 1e856L, 1e857L, 1e858L, 1e859L, 
   1e860L, 1e861L, 1e862L, 1e863L, 1e864L, 1e865L, 1e866L, 1e867L, 1e868L, 1e869L, 
   1e870L, 1e871L, 1e872L, 1e873L, 1e874L, 1e875L, 1e876L, 1e877L, 1e878L, 1e879L, 
   1e880L, 1e881L, 1e882L, 1e883L, 1e884L, 1e885L, 1e886L, 1e887L, 1e888L, 1e889L, 
   1e890L, 1e891L, 1e892L, 1e893L, 1e894L, 1e895L, 1e896L, 1e897L, 1e898L, 1e899L, 
   1e900L, 1e901L, 1e902L, 1e903L, 1e904L, 1e905L, 1e906L, 1e907L, 1e908L, 1e909L, 
   1e910L, 1e911L, 1e912L, 1e913L, 1e914L, 1e915L, 1e916L, 1e917L, 1e918L, 1e919L, 
   1e920L, 1e921L, 1e922L, 1e923L, 1e924L, 1e925L, 1e926L, 1e927L, 1e928L, 1e929L, 
   1e930L, 1e931L, 1e932L, 1e933L, 1e934L, 1e935L, 1e936L, 1e937L, 1e938L, 1e939L, 
   1e940L, 1e941L, 1e942L, 1e943L, 1e944L, 1e945L, 1e946L, 1e947L, 1e948L, 1e949L, 
   1e950L, 1e951L, 1e952L, 1e953L, 1e954L, 1e955L, 1e956L, 1e957L, 1e958L, 1e959L, 
   1e960L, 1e961L, 1e962L, 1e963L, 1e964L, 1e965L, 1e966L, 1e967L, 1e968L, 1e969L, 
   1e970L, 1e971L, 1e972L, 1e973L, 1e974L, 1e975L, 1e976L, 1e977L, 1e978L, 1e979L, 
   1e980L, 1e981L, 1e982L, 1e983L, 1e984L, 1e985L, 1e986L, 1e987L, 1e988L, 1e989L, 
   1e990L, 1e991L, 1e992L, 1e993L, 1e994L, 1e995L, 1e996L, 1e997L, 1e998L, 1e999L, 
   1e1000L, 1e1001L, 1e1002L, 1e1003L, 1e1004L, 1e1005L, 1e1006L, 1e1007L, 1e1008L, 1e1009L, 
   1e1010L, 1e1011L, 1e1012L, 1e1013L, 1e1014L, 1e1015L, 1e1016L, 1e1017L, 1e1018L, 1e1019L, 
   1e1020L, 1e1021L, 1e1022L, 1e1023L, 1e1024L, 1e1025L, 1e1026L, 1e1027L, 1e1028L, 1e1029L, 
   1e1030L, 1e1031L, 1e1032L, 1e1033L, 1e1034L, 1e1035L, 1e1036L, 1e1037L, 1e1038L, 1e1039L, 
   1e1040L, 1e1041L, 1e1042L, 1e1043L, 1e1044L, 1e1045L, 1e1046L, 1e1047L, 1e1048L, 1e1049L, 
   1e1050L, 1e1051L, 1e1052L, 1e1053L, 1e1054L, 1e1055L, 1e1056L, 1e1057L, 1e1058L, 1e1059L, 
   1e1060L, 1e1061L, 1e1062L, 1e1063L, 1e1064L, 1e1065L, 1e1066L, 1e1067L, 1e1068L, 1e1069L, 
   1e1070L, 1e1071L, 1e1072L, 1e1073L, 1e1074L, 1e1075L, 1e1076L, 1e1077L, 1e1078L, 1e1079L, 
   1e1080L, 1e1081L, 1e1082L, 1e1083L, 1e1084L, 1e1085L, 1e1086L, 1e1087L, 1e1088L, 1e1089L, 
   1e1090L, 1e1091L, 1e1092L, 1e1093L, 1e1094L, 1e1095L, 1e1096L, 1e1097L, 1e1098L, 1e1099L, 
   1e1100L, 1e1101L, 1e1102L, 1e1103L, 1e1104L, 1e1105L, 1e1106L, 1e1107L, 1e1108L, 1e1109L, 
   1e1110L, 1e1111L, 1e1112L, 1e1113L, 1e1114L, 1e1115L, 1e1116L, 1e1117L, 1e1118L, 1e1119L, 
   1e1120L, 1e1121L, 1e1122L, 1e1123L, 1e1124L, 1e1125L, 1e1126L, 1e1127L, 1e1128L, 1e1129L, 
   1e1130L, 1e1131L, 1e1132L, 1e1133L, 1e1134L, 1e1135L, 1e1136L, 1e1137L, 1e1138L, 1e1139L, 
   1e1140L, 1e1141L, 1e1142L, 1e1143L, 1e1144L, 1e1145L, 1e1146L, 1e1147L, 1e1148L, 1e1149L, 
   1e1150L, 1e1151L, 1e1152L, 1e1153L, 1e1154L, 1e1155L, 1e1156L, 1e1157L, 1e1158L, 1e1159L, 
   1e1160L, 1e1161L, 1e1162L, 1e1163L, 1e1164L, 1e1165L, 1e1166L, 1e1167L, 1e1168L, 1e1169L, 
   1e1170L, 1e1171L, 1e1172L, 1e1173L, 1e1174L, 1e1175L, 1e1176L, 1e1177L, 1e1178L, 1e1179L, 
   1e1180L, 1e1181L, 1e1182L, 1e1183L, 1e1184L, 1e1185L, 1e1186L, 1e1187L, 1e1188L, 1e1189L, 
   1e1190L, 1e1191L, 1e1192L, 1e1193L, 1e1194L, 1e1195L, 1e1196L, 1e1197L, 1e1198L, 1e1199L, 
   1e1200L, 1e1201L, 1e1202L, 1e1203L, 1e1204L, 1e1205L, 1e1206L, 1e1207L, 1e1208L, 1e1209L, 
   1e1210L, 1e1211L, 1e1212L, 1e1213L, 1e1214L, 1e1215L, 1e1216L, 1e1217L, 1e1218L, 1e1219L, 
   1e1220L, 1e1221L, 1e1222L, 1e1223L, 1e1224L, 1e1225L, 1e1226L, 1e1227L, 1e1228L, 1e1229L, 
   1e1230L, 1e1231L, 1e1232L, 1e1233L, 1e1234L, 1e1235L, 1e1236L, 1e1237L, 1e1238L, 1e1239L, 
   1e1240L, 1e1241L, 1e1242L, 1e1243L, 1e1244L, 1e1245L, 1e1246L, 1e1247L, 1e1248L, 1e1249L, 
   1e1250L, 1e1251L, 1e1252L, 1e1253L, 1e1254L, 1e1255L, 1e1256L, 1e1257L, 1e1258L, 1e1259L, 
   1e1260L, 1e1261L, 1e1262L, 1e1263L, 1e1264L, 1e1265L, 1e1266L, 1e1267L, 1e1268L, 1e1269L, 
   1e1270L, 1e1271L, 1e1272L, 1e1273L, 1e1274L, 1e1275L, 1e1276L, 1e1277L, 1e1278L, 1e1279L, 
   1e1280L, 1e1281L, 1e1282L, 1e1283L, 1e1284L, 1e1285L, 1e1286L, 1e1287L, 1e1288L, 1e1289L, 
   1e1290L, 1e1291L, 1e1292L, 1e1293L, 1e1294L, 1e1295L, 1e1296L, 1e1297L, 1e1298L, 1e1299L, 
   1e1300L, 1e1301L, 1e1302L, 1e1303L, 1e1304L, 1e1305L, 1e1306L, 1e1307L, 1e1308L, 1e1309L, 
   1e1310L, 1e1311L, 1e1312L, 1e1313L, 1e1314L, 1e1315L, 1e1316L, 1e1317L, 1e1318L, 1e1319L, 
   1e1320L, 1e1321L, 1e1322L, 1e1323L, 1e1324L, 1e1325L, 1e1326L, 1e1327L, 1e1328L, 1e1329L, 
   1e1330L, 1e1331L, 1e1332L, 1e1333L, 1e1334L, 1e1335L, 1e1336L, 1e1337L, 1e1338L, 1e1339L, 
   1e1340L, 1e1341L, 1e1342L, 1e1343L, 1e1344L, 1e1345L, 1e1346L, 1e1347L, 1e1348L, 1e1349L, 
   1e1350L, 1e1351L, 1e1352L, 1e1353L, 1e1354L, 1e1355L, 1e1356L, 1e1357L, 1e1358L, 1e1359L, 
   1e1360L, 1e1361L, 1e1362L, 1e1363L, 1e1364L, 1e1365L, 1e1366L, 1e1367L, 1e1368L, 1e1369L, 
   1e1370L, 1e1371L, 1e1372L, 1e1373L, 1e1374L, 1e1375L, 1e1376L, 1e1377L, 1e1378L, 1e1379L, 
   1e1380L, 1e1381L, 1e1382L, 1e1383L, 1e1384L, 1e1385L, 1e1386L, 1e1387L, 1e1388L, 1e1389L, 
   1e1390L, 1e1391L, 1e1392L, 1e1393L, 1e1394L, 1e1395L, 1e1396L, 1e1397L, 1e1398L, 1e1399L, 
   1e1400L, 1e1401L, 1e1402L, 1e1403L, 1e1404L, 1e1405L, 1e1406L, 1e1407L, 1e1408L, 1e1409L, 
   1e1410L, 1e1411L, 1e1412L, 1e1413L, 1e1414L, 1e1415L, 1e1416L, 1e1417L, 1e1418L, 1e1419L, 
   1e1420L, 1e1421L, 1e1422L, 1e1423L, 1e1424L, 1e1425L, 1e1426L, 1e1427L, 1e1428L, 1e1429L, 
   1e1430L, 1e1431L, 1e1432L, 1e1433L, 1e1434L, 1e1435L, 1e1436L, 1e1437L, 1e1438L, 1e1439L, 
   1e1440L, 1e1441L, 1e1442L, 1e1443L, 1e1444L, 1e1445L, 1e1446L, 1e1447L, 1e1448L, 1e1449L, 
   1e1450L, 1e1451L, 1e1452L, 1e1453L, 1e1454L, 1e1455L, 1e1456L, 1e1457L, 1e1458L, 1e1459L, 
   1e1460L, 1e1461L, 1e1462L, 1e1463L, 1e1464L, 1e1465L, 1e1466L, 1e1467L, 1e1468L, 1e1469L, 
   1e1470L, 1e1471L, 1e1472L, 1e1473L, 1e1474L, 1e1475L, 1e1476L, 1e1477L, 1e1478L, 1e1479L, 
   1e1480L, 1e1481L, 1e1482L, 1e1483L, 1e1484L, 1e1485L, 1e1486L, 1e1487L, 1e1488L, 1e1489L, 
   1e1490L, 1e1491L, 1e1492L, 1e1493L, 1e1494L, 1e1495L, 1e1496L, 1e1497L, 1e1498L, 1e1499L, 
   1e1500L, 1e1501L, 1e1502L, 1e1503L, 1e1504L, 1e1505L, 1e1506L, 1e1507L, 1e1508L, 1e1509L, 
   1e1510L, 1e1511L, 1e1512L, 1e1513L, 1e1514L, 1e1515L, 1e1516L, 1e1517L, 1e1518L, 1e1519L, 
   1e1520L, 1e1521L, 1e1522L, 1e1523L, 1e1524L, 1e1525L, 1e1526L, 1e1527L, 1e1528L, 1e1529L, 
   1e1530L, 1e1531L, 1e1532L, 1e1533L, 1e1534L, 1e1535L, 1e1536L, 1e1537L, 1e1538L, 1e1539L, 
   1e1540L, 1e1541L, 1e1542L, 1e1543L, 1e1544L, 1e1545L, 1e1546L, 1e1547L, 1e1548L, 1e1549L, 
   1e1550L, 1e1551L, 1e1552L, 1e1553L, 1e1554L, 1e1555L, 1e1556L, 1e1557L, 1e1558L, 1e1559L, 
   1e1560L, 1e1561L, 1e1562L, 1e1563L, 1e1564L, 1e1565L, 1e1566L, 1e1567L, 1e1568L, 1e1569L, 
   1e1570L, 1e1571L, 1e1572L, 1e1573L, 1e1574L, 1e1575L, 1e1576L, 1e1577L, 1e1578L, 1e1579L, 
   1e1580L, 1e1581L, 1e1582L, 1e1583L, 1e1584L, 1e1585L, 1e1586L, 1e1587L, 1e1588L, 1e1589L, 
   1e1590L, 1e1591L, 1e1592L, 1e1593L, 1e1594L, 1e1595L, 1e1596L, 1e1597L, 1e1598L, 1e1599L, 
   1e1600L, 1e1601L, 1e1602L, 1e1603L, 1e1604L, 1e1605L, 1e1606L, 1e1607L, 1e1608L, 1e1609L, 
   1e1610L, 1e1611L, 1e1612L, 1e1613L, 1e1614L, 1e1615L, 1e1616L, 1e1617L, 1e1618L, 1e1619L, 
   1e1620L, 1e1621L, 1e1622L, 1e1623L, 1e1624L, 1e1625L, 1e1626L, 1e1627L, 1e1628L, 1e1629L, 
   1e1630L, 1e1631L, 1e1632L, 1e1633L, 1e1634L, 1e1635L, 1e1636L, 1e1637L, 1e1638L, 1e1639L, 
   1e1640L, 1e1641L, 1e1642L, 1e1643L, 1e1644L, 1e1645L, 1e1646L, 1e1647L, 1e1648L, 1e1649L, 
   1e1650L, 1e1651L, 1e1652L, 1e1653L, 1e1654L, 1e1655L, 1e1656L, 1e1657L, 1e1658L, 1e1659L, 
   1e1660L, 1e1661L, 1e1662L, 1e1663L, 1e1664L, 1e1665L, 1e1666L, 1e1667L, 1e1668L, 1e1669L, 
   1e1670L, 1e1671L, 1e1672L, 1e1673L, 1e1674L, 1e1675L, 1e1676L, 1e1677L, 1e1678L, 1e1679L, 
   1e1680L, 1e1681L, 1e1682L, 1e1683L, 1e1684L, 1e1685L, 1e1686L, 1e1687L, 1e1688L, 1e1689L, 
   1e1690L, 1e1691L, 1e1692L, 1e1693L, 1e1694L, 1e1695L, 1e1696L, 1e1697L, 1e1698L, 1e1699L, 
   1e1700L, 1e1701L, 1e1702L, 1e1703L, 1e1704L, 1e1705L, 1e1706L, 1e1707L, 1e1708L, 1e1709L, 
   1e1710L, 1e1711L, 1e1712L, 1e1713L, 1e1714L, 1e1715L, 1e1716L, 1e1717L, 1e1718L, 1e1719L, 
   1e1720L, 1e1721L, 1e1722L, 1e1723L, 1e1724L, 1e1725L, 1e1726L, 1e1727L, 1e1728L, 1e1729L, 
   1e1730L, 1e1731L, 1e1732L, 1e1733L, 1e1734L, 1e1735L, 1e1736L, 1e1737L, 1e1738L, 1e1739L, 
   1e1740L, 1e1741L, 1e1742L, 1e1743L, 1e1744L, 1e1745L, 1e1746L, 1e1747L, 1e1748L, 1e1749L, 
   1e1750L, 1e1751L, 1e1752L, 1e1753L, 1e1754L, 1e1755L, 1e1756L, 1e1757L, 1e1758L, 1e1759L, 
   1e1760L, 1e1761L, 1e1762L, 1e1763L, 1e1764L, 1e1765L, 1e1766L, 1e1767L, 1e1768L, 1e1769L, 
   1e1770L, 1e1771L, 1e1772L, 1e1773L, 1e1774L, 1e1775L, 1e1776L, 1e1777L, 1e1778L, 1e1779L, 
   1e1780L, 1e1781L, 1e1782L, 1e1783L, 1e1784L, 1e1785L, 1e1786L, 1e1787L, 1e1788L, 1e1789L, 
   1e1790L, 1e1791L, 1e1792L, 1e1793L, 1e1794L, 1e1795L, 1e1796L, 1e1797L, 1e1798L, 1e1799L, 
   1e1800L, 1e1801L, 1e1802L, 1e1803L, 1e1804L, 1e1805L, 1e1806L, 1e1807L, 1e1808L, 1e1809L, 
   1e1810L, 1e1811L, 1e1812L, 1e1813L, 1e1814L, 1e1815L, 1e1816L, 1e1817L, 1e1818L, 1e1819L, 
   1e1820L, 1e1821L, 1e1822L, 1e1823L, 1e1824L, 1e1825L, 1e1826L, 1e1827L, 1e1828L, 1e1829L, 
   1e1830L, 1e1831L, 1e1832L, 1e1833L, 1e1834L, 1e1835L, 1e1836L, 1e1837L, 1e1838L, 1e1839L, 
   1e1840L, 1e1841L, 1e1842L, 1e1843L, 1e1844L, 1e1845L, 1e1846L, 1e1847L, 1e1848L, 1e1849L, 
   1e1850L, 1e1851L, 1e1852L, 1e1853L, 1e1854L, 1e1855L, 1e1856L, 1e1857L, 1e1858L, 1e1859L, 
   1e1860L, 1e1861L, 1e1862L, 1e1863L, 1e1864L, 1e1865L, 1e1866L, 1e1867L, 1e1868L, 1e1869L, 
   1e1870L, 1e1871L, 1e1872L, 1e1873L, 1e1874L, 1e1875L, 1e1876L, 1e1877L, 1e1878L, 1e1879L, 
   1e1880L, 1e1881L, 1e1882L, 1e1883L, 1e1884L, 1e1885L, 1e1886L, 1e1887L, 1e1888L, 1e1889L, 
   1e1890L, 1e1891L, 1e1892L, 1e1893L, 1e1894L, 1e1895L, 1e1896L, 1e1897L, 1e1898L, 1e1899L, 
   1e1900L, 1e1901L, 1e1902L, 1e1903L, 1e1904L, 1e1905L, 1e1906L, 1e1907L, 1e1908L, 1e1909L, 
   1e1910L, 1e1911L, 1e1912L, 1e1913L, 1e1914L, 1e1915L, 1e1916L, 1e1917L, 1e1918L, 1e1919L, 
   1e1920L, 1e1921L, 1e1922L, 1e1923L, 1e1924L, 1e1925L, 1e1926L, 1e1927L, 1e1928L, 1e1929L, 
   1e1930L, 1e1931L, 1e1932L, 1e1933L, 1e1934L, 1e1935L, 1e1936L, 1e1937L, 1e1938L, 1e1939L, 
   1e1940L, 1e1941L, 1e1942L, 1e1943L, 1e1944L, 1e1945L, 1e1946L, 1e1947L, 1e1948L, 1e1949L, 
   1e1950L, 1e1951L, 1e1952L, 1e1953L, 1e1954L, 1e1955L, 1e1956L, 1e1957L, 1e1958L, 1e1959L, 
   1e1960L, 1e1961L, 1e1962L, 1e1963L, 1e1964L, 1e1965L, 1e1966L, 1e1967L, 1e1968L, 1e1969L, 
   1e1970L, 1e1971L, 1e1972L, 1e1973L, 1e1974L, 1e1975L, 1e1976L, 1e1977L, 1e1978L, 1e1979L, 
   1e1980L, 1e1981L, 1e1982L, 1e1983L, 1e1984L, 1e1985L, 1e1986L, 1e1987L, 1e1988L, 1e1989L, 
   1e1990L, 1e1991L, 1e1992L, 1e1993L, 1e1994L, 1e1995L, 1e1996L, 1e1997L, 1e1998L, 1e1999L, 
   1e2000L, 1e2001L, 1e2002L, 1e2003L, 1e2004L, 1e2005L, 1e2006L, 1e2007L, 1e2008L, 1e2009L, 
   1e2010L, 1e2011L, 1e2012L, 1e2013L, 1e2014L, 1e2015L, 1e2016L, 1e2017L, 1e2018L, 1e2019L, 
   1e2020L, 1e2021L, 1e2022L, 1e2023L, 1e2024L, 1e2025L, 1e2026L, 1e2027L, 1e2028L, 1e2029L, 
   1e2030L, 1e2031L, 1e2032L, 1e2033L, 1e2034L, 1e2035L, 1e2036L, 1e2037L, 1e2038L, 1e2039L, 
   1e2040L, 1e2041L, 1e2042L, 1e2043L, 1e2044L, 1e2045L, 1e2046L, 1e2047L, 1e2048L, 1e2049L, 
   1e2050L, 1e2051L, 1e2052L, 1e2053L, 1e2054L, 1e2055L, 1e2056L, 1e2057L, 1e2058L, 1e2059L, 
   1e2060L, 1e2061L, 1e2062L, 1e2063L, 1e2064L, 1e2065L, 1e2066L, 1e2067L, 1e2068L, 1e2069L, 
   1e2070L, 1e2071L, 1e2072L, 1e2073L, 1e2074L, 1e2075L, 1e2076L, 1e2077L, 1e2078L, 1e2079L, 
   1e2080L, 1e2081L, 1e2082L, 1e2083L, 1e2084L, 1e2085L, 1e2086L, 1e2087L, 1e2088L, 1e2089L, 
   1e2090L, 1e2091L, 1e2092L, 1e2093L, 1e2094L, 1e2095L, 1e2096L, 1e2097L, 1e2098L, 1e2099L, 
   1e2100L, 1e2101L, 1e2102L, 1e2103L, 1e2104L, 1e2105L, 1e2106L, 1e2107L, 1e2108L, 1e2109L, 
   1e2110L, 1e2111L, 1e2112L, 1e2113L, 1e2114L, 1e2115L, 1e2116L, 1e2117L, 1e2118L, 1e2119L, 
   1e2120L, 1e2121L, 1e2122L, 1e2123L, 1e2124L, 1e2125L, 1e2126L, 1e2127L, 1e2128L, 1e2129L, 
   1e2130L, 1e2131L, 1e2132L, 1e2133L, 1e2134L, 1e2135L, 1e2136L, 1e2137L, 1e2138L, 1e2139L, 
   1e2140L, 1e2141L, 1e2142L, 1e2143L, 1e2144L, 1e2145L, 1e2146L, 1e2147L, 1e2148L, 1e2149L, 
   1e2150L, 1e2151L, 1e2152L, 1e2153L, 1e2154L, 1e2155L, 1e2156L, 1e2157L, 1e2158L, 1e2159L, 
   1e2160L, 1e2161L, 1e2162L, 1e2163L, 1e2164L, 1e2165L, 1e2166L, 1e2167L, 1e2168L, 1e2169L, 
   1e2170L, 1e2171L, 1e2172L, 1e2173L, 1e2174L, 1e2175L, 1e2176L, 1e2177L, 1e2178L, 1e2179L, 
   1e2180L, 1e2181L, 1e2182L, 1e2183L, 1e2184L, 1e2185L, 1e2186L, 1e2187L, 1e2188L, 1e2189L, 
   1e2190L, 1e2191L, 1e2192L, 1e2193L, 1e2194L, 1e2195L, 1e2196L, 1e2197L, 1e2198L, 1e2199L, 
   1e2200L, 1e2201L, 1e2202L, 1e2203L, 1e2204L, 1e2205L, 1e2206L, 1e2207L, 1e2208L, 1e2209L, 
   1e2210L, 1e2211L, 1e2212L, 1e2213L, 1e2214L, 1e2215L, 1e2216L, 1e2217L, 1e2218L, 1e2219L, 
   1e2220L, 1e2221L, 1e2222L, 1e2223L, 1e2224L, 1e2225L, 1e2226L, 1e2227L, 1e2228L, 1e2229L, 
   1e2230L, 1e2231L, 1e2232L, 1e2233L, 1e2234L, 1e2235L, 1e2236L, 1e2237L, 1e2238L, 1e2239L, 
   1e2240L, 1e2241L, 1e2242L, 1e2243L, 1e2244L, 1e2245L, 1e2246L, 1e2247L, 1e2248L, 1e2249L, 
   1e2250L, 1e2251L, 1e2252L, 1e2253L, 1e2254L, 1e2255L, 1e2256L, 1e2257L, 1e2258L, 1e2259L, 
   1e2260L, 1e2261L, 1e2262L, 1e2263L, 1e2264L, 1e2265L, 1e2266L, 1e2267L, 1e2268L, 1e2269L, 
   1e2270L, 1e2271L, 1e2272L, 1e2273L, 1e2274L, 1e2275L, 1e2276L, 1e2277L, 1e2278L, 1e2279L, 
   1e2280L, 1e2281L, 1e2282L, 1e2283L, 1e2284L, 1e2285L, 1e2286L, 1e2287L, 1e2288L, 1e2289L, 
   1e2290L, 1e2291L, 1e2292L, 1e2293L, 1e2294L, 1e2295L, 1e2296L, 1e2297L, 1e2298L, 1e2299L, 
   1e2300L, 1e2301L, 1e2302L, 1e2303L, 1e2304L, 1e2305L, 1e2306L, 1e2307L, 1e2308L, 1e2309L, 
   1e2310L, 1e2311L, 1e2312L, 1e2313L, 1e2314L, 1e2315L, 1e2316L, 1e2317L, 1e2318L, 1e2319L, 
   1e2320L, 1e2321L, 1e2322L, 1e2323L, 1e2324L, 1e2325L, 1e2326L, 1e2327L, 1e2328L, 1e2329L, 
   1e2330L, 1e2331L, 1e2332L, 1e2333L, 1e2334L, 1e2335L, 1e2336L, 1e2337L, 1e2338L, 1e2339L, 
   1e2340L, 1e2341L, 1e2342L, 1e2343L, 1e2344L, 1e2345L, 1e2346L, 1e2347L, 1e2348L, 1e2349L, 
   1e2350L, 1e2351L, 1e2352L, 1e2353L, 1e2354L, 1e2355L, 1e2356L, 1e2357L, 1e2358L, 1e2359L, 
   1e2360L, 1e2361L, 1e2362L, 1e2363L, 1e2364L, 1e2365L, 1e2366L, 1e2367L, 1e2368L, 1e2369L, 
   1e2370L, 1e2371L, 1e2372L, 1e2373L, 1e2374L, 1e2375L, 1e2376L, 1e2377L, 1e2378L, 1e2379L, 
   1e2380L, 1e2381L, 1e2382L, 1e2383L, 1e2384L, 1e2385L, 1e2386L, 1e2387L, 1e2388L, 1e2389L, 
   1e2390L, 1e2391L, 1e2392L, 1e2393L, 1e2394L, 1e2395L, 1e2396L, 1e2397L, 1e2398L, 1e2399L, 
   1e2400L, 1e2401L, 1e2402L, 1e2403L, 1e2404L, 1e2405L, 1e2406L, 1e2407L, 1e2408L, 1e2409L, 
   1e2410L, 1e2411L, 1e2412L, 1e2413L, 1e2414L, 1e2415L, 1e2416L, 1e2417L, 1e2418L, 1e2419L, 
   1e2420L, 1e2421L, 1e2422L, 1e2423L, 1e2424L, 1e2425L, 1e2426L, 1e2427L, 1e2428L, 1e2429L, 
   1e2430L, 1e2431L, 1e2432L, 1e2433L, 1e2434L, 1e2435L, 1e2436L, 1e2437L, 1e2438L, 1e2439L, 
   1e2440L, 1e2441L, 1e2442L, 1e2443L, 1e2444L, 1e2445L, 1e2446L, 1e2447L, 1e2448L, 1e2449L, 
   1e2450L, 1e2451L, 1e2452L, 1e2453L, 1e2454L, 1e2455L, 1e2456L, 1e2457L, 1e2458L, 1e2459L, 
   1e2460L, 1e2461L, 1e2462L, 1e2463L, 1e2464L, 1e2465L, 1e2466L, 1e2467L, 1e2468L, 1e2469L, 
   1e2470L, 1e2471L, 1e2472L, 1e2473L, 1e2474L, 1e2475L, 1e2476L, 1e2477L, 1e2478L, 1e2479L, 
   1e2480L, 1e2481L, 1e2482L, 1e2483L, 1e2484L, 1e2485L, 1e2486L, 1e2487L, 1e2488L, 1e2489L, 
   1e2490L, 1e2491L, 1e2492L, 1e2493L, 1e2494L, 1e2495L, 1e2496L, 1e2497L, 1e2498L, 1e2499L, 
   1e2500L, 1e2501L, 1e2502L, 1e2503L, 1e2504L, 1e2505L, 1e2506L, 1e2507L, 1e2508L, 1e2509L, 
   1e2510L, 1e2511L, 1e2512L, 1e2513L, 1e2514L, 1e2515L, 1e2516L, 1e2517L, 1e2518L, 1e2519L, 
   1e2520L, 1e2521L, 1e2522L, 1e2523L, 1e2524L, 1e2525L, 1e2526L, 1e2527L, 1e2528L, 1e2529L, 
   1e2530L, 1e2531L, 1e2532L, 1e2533L, 1e2534L, 1e2535L, 1e2536L, 1e2537L, 1e2538L, 1e2539L, 
   1e2540L, 1e2541L, 1e2542L, 1e2543L, 1e2544L, 1e2545L, 1e2546L, 1e2547L, 1e2548L, 1e2549L, 
   1e2550L, 1e2551L, 1e2552L, 1e2553L, 1e2554L, 1e2555L, 1e2556L, 1e2557L, 1e2558L, 1e2559L, 
   1e2560L, 1e2561L, 1e2562L, 1e2563L, 1e2564L, 1e2565L, 1e2566L, 1e2567L, 1e2568L, 1e2569L, 
   1e2570L, 1e2571L, 1e2572L, 1e2573L, 1e2574L, 1e2575L, 1e2576L, 1e2577L, 1e2578L, 1e2579L, 
   1e2580L, 1e2581L, 1e2582L, 1e2583L, 1e2584L, 1e2585L, 1e2586L, 1e2587L, 1e2588L, 1e2589L, 
   1e2590L, 1e2591L, 1e2592L, 1e2593L, 1e2594L, 1e2595L, 1e2596L, 1e2597L, 1e2598L, 1e2599L, 
   1e2600L, 1e2601L, 1e2602L, 1e2603L, 1e2604L, 1e2605L, 1e2606L, 1e2607L, 1e2608L, 1e2609L, 
   1e2610L, 1e2611L, 1e2612L, 1e2613L, 1e2614L, 1e2615L, 1e2616L, 1e2617L, 1e2618L, 1e2619L, 
   1e2620L, 1e2621L, 1e2622L, 1e2623L, 1e2624L, 1e2625L, 1e2626L, 1e2627L, 1e2628L, 1e2629L, 
   1e2630L, 1e2631L, 1e2632L, 1e2633L, 1e2634L, 1e2635L, 1e2636L, 1e2637L, 1e2638L, 1e2639L, 
   1e2640L, 1e2641L, 1e2642L, 1e2643L, 1e2644L, 1e2645L, 1e2646L, 1e2647L, 1e2648L, 1e2649L, 
   1e2650L, 1e2651L, 1e2652L, 1e2653L, 1e2654L, 1e2655L, 1e2656L, 1e2657L, 1e2658L, 1e2659L, 
   1e2660L, 1e2661L, 1e2662L, 1e2663L, 1e2664L, 1e2665L, 1e2666L, 1e2667L, 1e2668L, 1e2669L, 
   1e2670L, 1e2671L, 1e2672L, 1e2673L, 1e2674L, 1e2675L, 1e2676L, 1e2677L, 1e2678L, 1e2679L, 
   1e2680L, 1e2681L, 1e2682L, 1e2683L, 1e2684L, 1e2685L, 1e2686L, 1e2687L, 1e2688L, 1e2689L, 
   1e2690L, 1e2691L, 1e2692L, 1e2693L, 1e2694L, 1e2695L, 1e2696L, 1e2697L, 1e2698L, 1e2699L, 
   1e2700L, 1e2701L, 1e2702L, 1e2703L, 1e2704L, 1e2705L, 1e2706L, 1e2707L, 1e2708L, 1e2709L, 
   1e2710L, 1e2711L, 1e2712L, 1e2713L, 1e2714L, 1e2715L, 1e2716L, 1e2717L, 1e2718L, 1e2719L, 
   1e2720L, 1e2721L, 1e2722L, 1e2723L, 1e2724L, 1e2725L, 1e2726L, 1e2727L, 1e2728L, 1e2729L, 
   1e2730L, 1e2731L, 1e2732L, 1e2733L, 1e2734L, 1e2735L, 1e2736L, 1e2737L, 1e2738L, 1e2739L, 
   1e2740L, 1e2741L, 1e2742L, 1e2743L, 1e2744L, 1e2745L, 1e2746L, 1e2747L, 1e2748L, 1e2749L, 
   1e2750L, 1e2751L, 1e2752L, 1e2753L, 1e2754L, 1e2755L, 1e2756L, 1e2757L, 1e2758L, 1e2759L, 
   1e2760L, 1e2761L, 1e2762L, 1e2763L, 1e2764L, 1e2765L, 1e2766L, 1e2767L, 1e2768L, 1e2769L, 
   1e2770L, 1e2771L, 1e2772L, 1e2773L, 1e2774L, 1e2775L, 1e2776L, 1e2777L, 1e2778L, 1e2779L, 
   1e2780L, 1e2781L, 1e2782L, 1e2783L, 1e2784L, 1e2785L, 1e2786L, 1e2787L, 1e2788L, 1e2789L, 
   1e2790L, 1e2791L, 1e2792L, 1e2793L, 1e2794L, 1e2795L, 1e2796L, 1e2797L, 1e2798L, 1e2799L, 
   1e2800L, 1e2801L, 1e2802L, 1e2803L, 1e2804L, 1e2805L, 1e2806L, 1e2807L, 1e2808L, 1e2809L, 
   1e2810L, 1e2811L, 1e2812L, 1e2813L, 1e2814L, 1e2815L, 1e2816L, 1e2817L, 1e2818L, 1e2819L, 
   1e2820L, 1e2821L, 1e2822L, 1e2823L, 1e2824L, 1e2825L, 1e2826L, 1e2827L, 1e2828L, 1e2829L, 
   1e2830L, 1e2831L, 1e2832L, 1e2833L, 1e2834L, 1e2835L, 1e2836L, 1e2837L, 1e2838L, 1e2839L, 
   1e2840L, 1e2841L, 1e2842L, 1e2843L, 1e2844L, 1e2845L, 1e2846L, 1e2847L, 1e2848L, 1e2849L, 
   1e2850L, 1e2851L, 1e2852L, 1e2853L, 1e2854L, 1e2855L, 1e2856L, 1e2857L, 1e2858L, 1e2859L, 
   1e2860L, 1e2861L, 1e2862L, 1e2863L, 1e2864L, 1e2865L, 1e2866L, 1e2867L, 1e2868L, 1e2869L, 
   1e2870L, 1e2871L, 1e2872L, 1e2873L, 1e2874L, 1e2875L, 1e2876L, 1e2877L, 1e2878L, 1e2879L, 
   1e2880L, 1e2881L, 1e2882L, 1e2883L, 1e2884L, 1e2885L, 1e2886L, 1e2887L, 1e2888L, 1e2889L, 
   1e2890L, 1e2891L, 1e2892L, 1e2893L, 1e2894L, 1e2895L, 1e2896L, 1e2897L, 1e2898L, 1e2899L, 
   1e2900L, 1e2901L, 1e2902L, 1e2903L, 1e2904L, 1e2905L, 1e2906L, 1e2907L, 1e2908L, 1e2909L, 
   1e2910L, 1e2911L, 1e2912L, 1e2913L, 1e2914L, 1e2915L, 1e2916L, 1e2917L, 1e2918L, 1e2919L, 
   1e2920L, 1e2921L, 1e2922L, 1e2923L, 1e2924L, 1e2925L, 1e2926L, 1e2927L, 1e2928L, 1e2929L, 
   1e2930L, 1e2931L, 1e2932L, 1e2933L, 1e2934L, 1e2935L, 1e2936L, 1e2937L, 1e2938L, 1e2939L, 
   1e2940L, 1e2941L, 1e2942L, 1e2943L, 1e2944L, 1e2945L, 1e2946L, 1e2947L, 1e2948L, 1e2949L, 
   1e2950L, 1e2951L, 1e2952L, 1e2953L, 1e2954L, 1e2955L, 1e2956L, 1e2957L, 1e2958L, 1e2959L, 
   1e2960L, 1e2961L, 1e2962L, 1e2963L, 1e2964L, 1e2965L, 1e2966L, 1e2967L, 1e2968L, 1e2969L, 
   1e2970L, 1e2971L, 1e2972L, 1e2973L, 1e2974L, 1e2975L, 1e2976L, 1e2977L, 1e2978L, 1e2979L, 
   1e2980L, 1e2981L, 1e2982L, 1e2983L, 1e2984L, 1e2985L, 1e2986L, 1e2987L, 1e2988L, 1e2989L, 
   1e2990L, 1e2991L, 1e2992L, 1e2993L, 1e2994L, 1e2995L, 1e2996L, 1e2997L, 1e2998L, 1e2999L, 
   1e3000L, 1e3001L, 1e3002L, 1e3003L, 1e3004L, 1e3005L, 1e3006L, 1e3007L, 1e3008L, 1e3009L, 
   1e3010L, 1e3011L, 1e3012L, 1e3013L, 1e3014L, 1e3015L, 1e3016L, 1e3017L, 1e3018L, 1e3019L, 
   1e3020L, 1e3021L, 1e3022L, 1e3023L, 1e3024L, 1e3025L, 1e3026L, 1e3027L, 1e3028L, 1e3029L, 
   1e3030L, 1e3031L, 1e3032L, 1e3033L, 1e3034L, 1e3035L, 1e3036L, 1e3037L, 1e3038L, 1e3039L, 
   1e3040L, 1e3041L, 1e3042L, 1e3043L, 1e3044L, 1e3045L, 1e3046L, 1e3047L, 1e3048L, 1e3049L, 
   1e3050L, 1e3051L, 1e3052L, 1e3053L, 1e3054L, 1e3055L, 1e3056L, 1e3057L, 1e3058L, 1e3059L, 
   1e3060L, 1e3061L, 1e3062L, 1e3063L, 1e3064L, 1e3065L, 1e3066L, 1e3067L, 1e3068L, 1e3069L, 
   1e3070L, 1e3071L, 1e3072L, 1e3073L, 1e3074L, 1e3075L, 1e3076L, 1e3077L, 1e3078L, 1e3079L, 
   1e3080L, 1e3081L, 1e3082L, 1e3083L, 1e3084L, 1e3085L, 1e3086L, 1e3087L, 1e3088L, 1e3089L, 
   1e3090L, 1e3091L, 1e3092L, 1e3093L, 1e3094L, 1e3095L, 1e3096L, 1e3097L, 1e3098L, 1e3099L, 
   1e3100L, 1e3101L, 1e3102L, 1e3103L, 1e3104L, 1e3105L, 1e3106L, 1e3107L, 1e3108L, 1e3109L, 
   1e3110L, 1e3111L, 1e3112L, 1e3113L, 1e3114L, 1e3115L, 1e3116L, 1e3117L, 1e3118L, 1e3119L, 
   1e3120L, 1e3121L, 1e3122L, 1e3123L, 1e3124L, 1e3125L, 1e3126L, 1e3127L, 1e3128L, 1e3129L, 
   1e3130L, 1e3131L, 1e3132L, 1e3133L, 1e3134L, 1e3135L, 1e3136L, 1e3137L, 1e3138L, 1e3139L, 
   1e3140L, 1e3141L, 1e3142L, 1e3143L, 1e3144L, 1e3145L, 1e3146L, 1e3147L, 1e3148L, 1e3149L, 
   1e3150L, 1e3151L, 1e3152L, 1e3153L, 1e3154L, 1e3155L, 1e3156L, 1e3157L, 1e3158L, 1e3159L, 
   1e3160L, 1e3161L, 1e3162L, 1e3163L, 1e3164L, 1e3165L, 1e3166L, 1e3167L, 1e3168L, 1e3169L, 
   1e3170L, 1e3171L, 1e3172L, 1e3173L, 1e3174L, 1e3175L, 1e3176L, 1e3177L, 1e3178L, 1e3179L, 
   1e3180L, 1e3181L, 1e3182L, 1e3183L, 1e3184L, 1e3185L, 1e3186L, 1e3187L, 1e3188L, 1e3189L, 
   1e3190L, 1e3191L, 1e3192L, 1e3193L, 1e3194L, 1e3195L, 1e3196L, 1e3197L, 1e3198L, 1e3199L, 
   1e3200L, 1e3201L, 1e3202L, 1e3203L, 1e3204L, 1e3205L, 1e3206L, 1e3207L, 1e3208L, 1e3209L, 
   1e3210L, 1e3211L, 1e3212L, 1e3213L, 1e3214L, 1e3215L, 1e3216L, 1e3217L, 1e3218L, 1e3219L, 
   1e3220L, 1e3221L, 1e3222L, 1e3223L, 1e3224L, 1e3225L, 1e3226L, 1e3227L, 1e3228L, 1e3229L, 
   1e3230L, 1e3231L, 1e3232L, 1e3233L, 1e3234L, 1e3235L, 1e3236L, 1e3237L, 1e3238L, 1e3239L, 
   1e3240L, 1e3241L, 1e3242L, 1e3243L, 1e3244L, 1e3245L, 1e3246L, 1e3247L, 1e3248L, 1e3249L, 
   1e3250L, 1e3251L, 1e3252L, 1e3253L, 1e3254L, 1e3255L, 1e3256L, 1e3257L, 1e3258L, 1e3259L, 
   1e3260L, 1e3261L, 1e3262L, 1e3263L, 1e3264L, 1e3265L, 1e3266L, 1e3267L, 1e3268L, 1e3269L, 
   1e3270L, 1e3271L, 1e3272L, 1e3273L, 1e3274L, 1e3275L, 1e3276L, 1e3277L, 1e3278L, 1e3279L, 
   1e3280L, 1e3281L, 1e3282L, 1e3283L, 1e3284L, 1e3285L, 1e3286L, 1e3287L, 1e3288L, 1e3289L, 
   1e3290L, 1e3291L, 1e3292L, 1e3293L, 1e3294L, 1e3295L, 1e3296L, 1e3297L, 1e3298L, 1e3299L, 
   1e3300L, 1e3301L, 1e3302L, 1e3303L, 1e3304L, 1e3305L, 1e3306L, 1e3307L, 1e3308L, 1e3309L, 
   1e3310L, 1e3311L, 1e3312L, 1e3313L, 1e3314L, 1e3315L, 1e3316L, 1e3317L, 1e3318L, 1e3319L, 
   1e3320L, 1e3321L, 1e3322L, 1e3323L, 1e3324L, 1e3325L, 1e3326L, 1e3327L, 1e3328L, 1e3329L, 
   1e3330L, 1e3331L, 1e3332L, 1e3333L, 1e3334L, 1e3335L, 1e3336L, 1e3337L, 1e3338L, 1e3339L, 
   1e3340L, 1e3341L, 1e3342L, 1e3343L, 1e3344L, 1e3345L, 1e3346L, 1e3347L, 1e3348L, 1e3349L, 
   1e3350L, 1e3351L, 1e3352L, 1e3353L, 1e3354L, 1e3355L, 1e3356L, 1e3357L, 1e3358L, 1e3359L, 
   1e3360L, 1e3361L, 1e3362L, 1e3363L, 1e3364L, 1e3365L, 1e3366L, 1e3367L, 1e3368L, 1e3369L, 
   1e3370L, 1e3371L, 1e3372L, 1e3373L, 1e3374L, 1e3375L, 1e3376L, 1e3377L, 1e3378L, 1e3379L, 
   1e3380L, 1e3381L, 1e3382L, 1e3383L, 1e3384L, 1e3385L, 1e3386L, 1e3387L, 1e3388L, 1e3389L, 
   1e3390L, 1e3391L, 1e3392L, 1e3393L, 1e3394L, 1e3395L, 1e3396L, 1e3397L, 1e3398L, 1e3399L, 
   1e3400L, 1e3401L, 1e3402L, 1e3403L, 1e3404L, 1e3405L, 1e3406L, 1e3407L, 1e3408L, 1e3409L, 
   1e3410L, 1e3411L, 1e3412L, 1e3413L, 1e3414L, 1e3415L, 1e3416L, 1e3417L, 1e3418L, 1e3419L, 
   1e3420L, 1e3421L, 1e3422L, 1e3423L, 1e3424L, 1e3425L, 1e3426L, 1e3427L, 1e3428L, 1e3429L, 
   1e3430L, 1e3431L, 1e3432L, 1e3433L, 1e3434L, 1e3435L, 1e3436L, 1e3437L, 1e3438L, 1e3439L, 
   1e3440L, 1e3441L, 1e3442L, 1e3443L, 1e3444L, 1e3445L, 1e3446L, 1e3447L, 1e3448L, 1e3449L, 
   1e3450L, 1e3451L, 1e3452L, 1e3453L, 1e3454L, 1e3455L, 1e3456L, 1e3457L, 1e3458L, 1e3459L, 
   1e3460L, 1e3461L, 1e3462L, 1e3463L, 1e3464L, 1e3465L, 1e3466L, 1e3467L, 1e3468L, 1e3469L, 
   1e3470L, 1e3471L, 1e3472L, 1e3473L, 1e3474L, 1e3475L, 1e3476L, 1e3477L, 1e3478L, 1e3479L, 
   1e3480L, 1e3481L, 1e3482L, 1e3483L, 1e3484L, 1e3485L, 1e3486L, 1e3487L, 1e3488L, 1e3489L, 
   1e3490L, 1e3491L, 1e3492L, 1e3493L, 1e3494L, 1e3495L, 1e3496L, 1e3497L, 1e3498L, 1e3499L, 
   1e3500L, 1e3501L, 1e3502L, 1e3503L, 1e3504L, 1e3505L, 1e3506L, 1e3507L, 1e3508L, 1e3509L, 
   1e3510L, 1e3511L, 1e3512L, 1e3513L, 1e3514L, 1e3515L, 1e3516L, 1e3517L, 1e3518L, 1e3519L, 
   1e3520L, 1e3521L, 1e3522L, 1e3523L, 1e3524L, 1e3525L, 1e3526L, 1e3527L, 1e3528L, 1e3529L, 
   1e3530L, 1e3531L, 1e3532L, 1e3533L, 1e3534L, 1e3535L, 1e3536L, 1e3537L, 1e3538L, 1e3539L, 
   1e3540L, 1e3541L, 1e3542L, 1e3543L, 1e3544L, 1e3545L, 1e3546L, 1e3547L, 1e3548L, 1e3549L, 
   1e3550L, 1e3551L, 1e3552L, 1e3553L, 1e3554L, 1e3555L, 1e3556L, 1e3557L, 1e3558L, 1e3559L, 
   1e3560L, 1e3561L, 1e3562L, 1e3563L, 1e3564L, 1e3565L, 1e3566L, 1e3567L, 1e3568L, 1e3569L, 
   1e3570L, 1e3571L, 1e3572L, 1e3573L, 1e3574L, 1e3575L, 1e3576L, 1e3577L, 1e3578L, 1e3579L, 
   1e3580L, 1e3581L, 1e3582L, 1e3583L, 1e3584L, 1e3585L, 1e3586L, 1e3587L, 1e3588L, 1e3589L, 
   1e3590L, 1e3591L, 1e3592L, 1e3593L, 1e3594L, 1e3595L, 1e3596L, 1e3597L, 1e3598L, 1e3599L, 
   1e3600L, 1e3601L, 1e3602L, 1e3603L, 1e3604L, 1e3605L, 1e3606L, 1e3607L, 1e3608L, 1e3609L, 
   1e3610L, 1e3611L, 1e3612L, 1e3613L, 1e3614L, 1e3615L, 1e3616L, 1e3617L, 1e3618L, 1e3619L, 
   1e3620L, 1e3621L, 1e3622L, 1e3623L, 1e3624L, 1e3625L, 1e3626L, 1e3627L, 1e3628L, 1e3629L, 
   1e3630L, 1e3631L, 1e3632L, 1e3633L, 1e3634L, 1e3635L, 1e3636L, 1e3637L, 1e3638L, 1e3639L, 
   1e3640L, 1e3641L, 1e3642L, 1e3643L, 1e3644L, 1e3645L, 1e3646L, 1e3647L, 1e3648L, 1e3649L, 
   1e3650L, 1e3651L, 1e3652L, 1e3653L, 1e3654L, 1e3655L, 1e3656L, 1e3657L, 1e3658L, 1e3659L, 
   1e3660L, 1e3661L, 1e3662L, 1e3663L, 1e3664L, 1e3665L, 1e3666L, 1e3667L, 1e3668L, 1e3669L, 
   1e3670L, 1e3671L, 1e3672L, 1e3673L, 1e3674L, 1e3675L, 1e3676L, 1e3677L, 1e3678L, 1e3679L, 
   1e3680L, 1e3681L, 1e3682L, 1e3683L, 1e3684L, 1e3685L, 1e3686L, 1e3687L, 1e3688L, 1e3689L, 
   1e3690L, 1e3691L, 1e3692L, 1e3693L, 1e3694L, 1e3695L, 1e3696L, 1e3697L, 1e3698L, 1e3699L, 
   1e3700L, 1e3701L, 1e3702L, 1e3703L, 1e3704L, 1e3705L, 1e3706L, 1e3707L, 1e3708L, 1e3709L, 
   1e3710L, 1e3711L, 1e3712L, 1e3713L, 1e3714L, 1e3715L, 1e3716L, 1e3717L, 1e3718L, 1e3719L, 
   1e3720L, 1e3721L, 1e3722L, 1e3723L, 1e3724L, 1e3725L, 1e3726L, 1e3727L, 1e3728L, 1e3729L, 
   1e3730L, 1e3731L, 1e3732L, 1e3733L, 1e3734L, 1e3735L, 1e3736L, 1e3737L, 1e3738L, 1e3739L, 
   1e3740L, 1e3741L, 1e3742L, 1e3743L, 1e3744L, 1e3745L, 1e3746L, 1e3747L, 1e3748L, 1e3749L, 
   1e3750L, 1e3751L, 1e3752L, 1e3753L, 1e3754L, 1e3755L, 1e3756L, 1e3757L, 1e3758L, 1e3759L, 
   1e3760L, 1e3761L, 1e3762L, 1e3763L, 1e3764L, 1e3765L, 1e3766L, 1e3767L, 1e3768L, 1e3769L, 
   1e3770L, 1e3771L, 1e3772L, 1e3773L, 1e3774L, 1e3775L, 1e3776L, 1e3777L, 1e3778L, 1e3779L, 
   1e3780L, 1e3781L, 1e3782L, 1e3783L, 1e3784L, 1e3785L, 1e3786L, 1e3787L, 1e3788L, 1e3789L, 
   1e3790L, 1e3791L, 1e3792L, 1e3793L, 1e3794L, 1e3795L, 1e3796L, 1e3797L, 1e3798L, 1e3799L, 
   1e3800L, 1e3801L, 1e3802L, 1e3803L, 1e3804L, 1e3805L, 1e3806L, 1e3807L, 1e3808L, 1e3809L, 
   1e3810L, 1e3811L, 1e3812L, 1e3813L, 1e3814L, 1e3815L, 1e3816L, 1e3817L, 1e3818L, 1e3819L, 
   1e3820L, 1e3821L, 1e3822L, 1e3823L, 1e3824L, 1e3825L, 1e3826L, 1e3827L, 1e3828L, 1e3829L, 
   1e3830L, 1e3831L, 1e3832L, 1e3833L, 1e3834L, 1e3835L, 1e3836L, 1e3837L, 1e3838L, 1e3839L, 
   1e3840L, 1e3841L, 1e3842L, 1e3843L, 1e3844L, 1e3845L, 1e3846L, 1e3847L, 1e3848L, 1e3849L, 
   1e3850L, 1e3851L, 1e3852L, 1e3853L, 1e3854L, 1e3855L, 1e3856L, 1e3857L, 1e3858L, 1e3859L, 
   1e3860L, 1e3861L, 1e3862L, 1e3863L, 1e3864L, 1e3865L, 1e3866L, 1e3867L, 1e3868L, 1e3869L, 
   1e3870L, 1e3871L, 1e3872L, 1e3873L, 1e3874L, 1e3875L, 1e3876L, 1e3877L, 1e3878L, 1e3879L, 
   1e3880L, 1e3881L, 1e3882L, 1e3883L, 1e3884L, 1e3885L, 1e3886L, 1e3887L, 1e3888L, 1e3889L, 
   1e3890L, 1e3891L, 1e3892L, 1e3893L, 1e3894L, 1e3895L, 1e3896L, 1e3897L, 1e3898L, 1e3899L, 
   1e3900L, 1e3901L, 1e3902L, 1e3903L, 1e3904L, 1e3905L, 1e3906L, 1e3907L, 1e3908L, 1e3909L, 
   1e3910L, 1e3911L, 1e3912L, 1e3913L, 1e3914L, 1e3915L, 1e3916L, 1e3917L, 1e3918L, 1e3919L, 
   1e3920L, 1e3921L, 1e3922L, 1e3923L, 1e3924L, 1e3925L, 1e3926L, 1e3927L, 1e3928L, 1e3929L, 
   1e3930L, 1e3931L, 1e3932L, 1e3933L, 1e3934L, 1e3935L, 1e3936L, 1e3937L, 1e3938L, 1e3939L, 
   1e3940L, 1e3941L, 1e3942L, 1e3943L, 1e3944L, 1e3945L, 1e3946L, 1e3947L, 1e3948L, 1e3949L, 
   1e3950L, 1e3951L, 1e3952L, 1e3953L, 1e3954L, 1e3955L, 1e3956L, 1e3957L, 1e3958L, 1e3959L, 
   1e3960L, 1e3961L, 1e3962L, 1e3963L, 1e3964L, 1e3965L, 1e3966L, 1e3967L, 1e3968L, 1e3969L, 
   1e3970L, 1e3971L, 1e3972L, 1e3973L, 1e3974L, 1e3975L, 1e3976L, 1e3977L, 1e3978L, 1e3979L, 
   1e3980L, 1e3981L, 1e3982L, 1e3983L, 1e3984L, 1e3985L, 1e3986L, 1e3987L, 1e3988L, 1e3989L, 
   1e3990L, 1e3991L, 1e3992L, 1e3993L, 1e3994L, 1e3995L, 1e3996L, 1e3997L, 1e3998L, 1e3999L, 
   1e4000L, 1e4001L, 1e4002L, 1e4003L, 1e4004L, 1e4005L, 1e4006L, 1e4007L, 1e4008L, 1e4009L, 
   1e4010L, 1e4011L, 1e4012L, 1e4013L, 1e4014L, 1e4015L, 1e4016L, 1e4017L, 1e4018L, 1e4019L, 
   1e4020L, 1e4021L, 1e4022L, 1e4023L, 1e4024L, 1e4025L, 1e4026L, 1e4027L, 1e4028L, 1e4029L, 
   1e4030L, 1e4031L, 1e4032L, 1e4033L, 1e4034L, 1e4035L, 1e4036L, 1e4037L, 1e4038L, 1e4039L, 
   1e4040L, 1e4041L, 1e4042L, 1e4043L, 1e4044L, 1e4045L, 1e4046L, 1e4047L, 1e4048L, 1e4049L, 
   1e4050L, 1e4051L, 1e4052L, 1e4053L, 1e4054L, 1e4055L, 1e4056L, 1e4057L, 1e4058L, 1e4059L, 
   1e4060L, 1e4061L, 1e4062L, 1e4063L, 1e4064L, 1e4065L, 1e4066L, 1e4067L, 1e4068L, 1e4069L, 
   1e4070L, 1e4071L, 1e4072L, 1e4073L, 1e4074L, 1e4075L, 1e4076L, 1e4077L, 1e4078L, 1e4079L, 
   1e4080L, 1e4081L, 1e4082L, 1e4083L, 1e4084L, 1e4085L, 1e4086L, 1e4087L, 1e4088L, 1e4089L, 
   1e4090L, 1e4091L, 1e4092L, 1e4093L, 1e4094L, 1e4095L, 1e4096L, 1e4097L, 1e4098L, 1e4099L, 
   1e4100L, 1e4101L, 1e4102L, 1e4103L, 1e4104L, 1e4105L, 1e4106L, 1e4107L, 1e4108L, 1e4109L, 
   1e4110L, 1e4111L, 1e4112L, 1e4113L, 1e4114L, 1e4115L, 1e4116L, 1e4117L, 1e4118L, 1e4119L, 
   1e4120L, 1e4121L, 1e4122L, 1e4123L, 1e4124L, 1e4125L, 1e4126L, 1e4127L, 1e4128L, 1e4129L, 
   1e4130L, 1e4131L, 1e4132L, 1e4133L, 1e4134L, 1e4135L, 1e4136L, 1e4137L, 1e4138L, 1e4139L, 
   1e4140L, 1e4141L, 1e4142L, 1e4143L, 1e4144L, 1e4145L, 1e4146L, 1e4147L, 1e4148L, 1e4149L, 
   1e4150L, 1e4151L, 1e4152L, 1e4153L, 1e4154L, 1e4155L, 1e4156L, 1e4157L, 1e4158L, 1e4159L, 
   1e4160L, 1e4161L, 1e4162L, 1e4163L, 1e4164L, 1e4165L, 1e4166L, 1e4167L, 1e4168L, 1e4169L, 
   1e4170L, 1e4171L, 1e4172L, 1e4173L, 1e4174L, 1e4175L, 1e4176L, 1e4177L, 1e4178L, 1e4179L, 
   1e4180L, 1e4181L, 1e4182L, 1e4183L, 1e4184L, 1e4185L, 1e4186L, 1e4187L, 1e4188L, 1e4189L, 
   1e4190L, 1e4191L, 1e4192L, 1e4193L, 1e4194L, 1e4195L, 1e4196L, 1e4197L, 1e4198L, 1e4199L, 
   1e4200L, 1e4201L, 1e4202L, 1e4203L, 1e4204L, 1e4205L, 1e4206L, 1e4207L, 1e4208L, 1e4209L, 
   1e4210L, 1e4211L, 1e4212L, 1e4213L, 1e4214L, 1e4215L, 1e4216L, 1e4217L, 1e4218L, 1e4219L, 
   1e4220L, 1e4221L, 1e4222L, 1e4223L, 1e4224L, 1e4225L, 1e4226L, 1e4227L, 1e4228L, 1e4229L, 
   1e4230L, 1e4231L, 1e4232L, 1e4233L, 1e4234L, 1e4235L, 1e4236L, 1e4237L, 1e4238L, 1e4239L, 
   1e4240L, 1e4241L, 1e4242L, 1e4243L, 1e4244L, 1e4245L, 1e4246L, 1e4247L, 1e4248L, 1e4249L, 
   1e4250L, 1e4251L, 1e4252L, 1e4253L, 1e4254L, 1e4255L, 1e4256L, 1e4257L, 1e4258L, 1e4259L, 
   1e4260L, 1e4261L, 1e4262L, 1e4263L, 1e4264L, 1e4265L, 1e4266L, 1e4267L, 1e4268L, 1e4269L, 
   1e4270L, 1e4271L, 1e4272L, 1e4273L, 1e4274L, 1e4275L, 1e4276L, 1e4277L, 1e4278L, 1e4279L, 
   1e4280L, 1e4281L, 1e4282L, 1e4283L, 1e4284L, 1e4285L, 1e4286L, 1e4287L, 1e4288L, 1e4289L, 
   1e4290L, 1e4291L, 1e4292L, 1e4293L, 1e4294L, 1e4295L, 1e4296L, 1e4297L, 1e4298L, 1e4299L, 
   1e4300L, 1e4301L, 1e4302L, 1e4303L, 1e4304L, 1e4305L, 1e4306L, 1e4307L, 1e4308L, 1e4309L, 
   1e4310L, 1e4311L, 1e4312L, 1e4313L, 1e4314L, 1e4315L, 1e4316L, 1e4317L, 1e4318L, 1e4319L, 
   1e4320L, 1e4321L, 1e4322L, 1e4323L, 1e4324L, 1e4325L, 1e4326L, 1e4327L, 1e4328L, 1e4329L, 
   1e4330L, 1e4331L, 1e4332L, 1e4333L, 1e4334L, 1e4335L, 1e4336L, 1e4337L, 1e4338L, 1e4339L, 
   1e4340L, 1e4341L, 1e4342L, 1e4343L, 1e4344L, 1e4345L, 1e4346L, 1e4347L, 1e4348L, 1e4349L, 
   1e4350L, 1e4351L, 1e4352L, 1e4353L, 1e4354L, 1e4355L, 1e4356L, 1e4357L, 1e4358L, 1e4359L, 
   1e4360L, 1e4361L, 1e4362L, 1e4363L, 1e4364L, 1e4365L, 1e4366L, 1e4367L, 1e4368L, 1e4369L, 
   1e4370L, 1e4371L, 1e4372L, 1e4373L, 1e4374L, 1e4375L, 1e4376L, 1e4377L, 1e4378L, 1e4379L, 
   1e4380L, 1e4381L, 1e4382L, 1e4383L, 1e4384L, 1e4385L, 1e4386L, 1e4387L, 1e4388L, 1e4389L, 
   1e4390L, 1e4391L, 1e4392L, 1e4393L, 1e4394L, 1e4395L, 1e4396L, 1e4397L, 1e4398L, 1e4399L, 
   1e4400L, 1e4401L, 1e4402L, 1e4403L, 1e4404L, 1e4405L, 1e4406L, 1e4407L, 1e4408L, 1e4409L, 
   1e4410L, 1e4411L, 1e4412L, 1e4413L, 1e4414L, 1e4415L, 1e4416L, 1e4417L, 1e4418L, 1e4419L, 
   1e4420L, 1e4421L, 1e4422L, 1e4423L, 1e4424L, 1e4425L, 1e4426L, 1e4427L, 1e4428L, 1e4429L, 
   1e4430L, 1e4431L, 1e4432L, 1e4433L, 1e4434L, 1e4435L, 1e4436L, 1e4437L, 1e4438L, 1e4439L, 
   1e4440L, 1e4441L, 1e4442L, 1e4443L, 1e4444L, 1e4445L, 1e4446L, 1e4447L, 1e4448L, 1e4449L, 
   1e4450L, 1e4451L, 1e4452L, 1e4453L, 1e4454L, 1e4455L, 1e4456L, 1e4457L, 1e4458L, 1e4459L, 
   1e4460L, 1e4461L, 1e4462L, 1e4463L, 1e4464L, 1e4465L, 1e4466L, 1e4467L, 1e4468L, 1e4469L, 
   1e4470L, 1e4471L, 1e4472L, 1e4473L, 1e4474L, 1e4475L, 1e4476L, 1e4477L, 1e4478L, 1e4479L, 
   1e4480L, 1e4481L, 1e4482L, 1e4483L, 1e4484L, 1e4485L, 1e4486L, 1e4487L, 1e4488L, 1e4489L, 
   1e4490L, 1e4491L, 1e4492L, 1e4493L, 1e4494L, 1e4495L, 1e4496L, 1e4497L, 1e4498L, 1e4499L, 
   1e4500L, 1e4501L, 1e4502L, 1e4503L, 1e4504L, 1e4505L, 1e4506L, 1e4507L, 1e4508L, 1e4509L, 
   1e4510L, 1e4511L, 1e4512L, 1e4513L, 1e4514L, 1e4515L, 1e4516L, 1e4517L, 1e4518L, 1e4519L, 
   1e4520L, 1e4521L, 1e4522L, 1e4523L, 1e4524L, 1e4525L, 1e4526L, 1e4527L, 1e4528L, 1e4529L, 
   1e4530L, 1e4531L, 1e4532L, 1e4533L, 1e4534L, 1e4535L, 1e4536L, 1e4537L, 1e4538L, 1e4539L, 
   1e4540L, 1e4541L, 1e4542L, 1e4543L, 1e4544L, 1e4545L, 1e4546L, 1e4547L, 1e4548L, 1e4549L, 
   1e4550L, 1e4551L, 1e4552L, 1e4553L, 1e4554L, 1e4555L, 1e4556L, 1e4557L, 1e4558L, 1e4559L, 
   1e4560L, 1e4561L, 1e4562L, 1e4563L, 1e4564L, 1e4565L, 1e4566L, 1e4567L, 1e4568L, 1e4569L, 
   1e4570L, 1e4571L, 1e4572L, 1e4573L, 1e4574L, 1e4575L, 1e4576L, 1e4577L, 1e4578L, 1e4579L, 
   1e4580L, 1e4581L, 1e4582L, 1e4583L, 1e4584L, 1e4585L, 1e4586L, 1e4587L, 1e4588L, 1e4589L, 
   1e4590L, 1e4591L, 1e4592L, 1e4593L, 1e4594L, 1e4595L, 1e4596L, 1e4597L, 1e4598L, 1e4599L, 
   1e4600L, 1e4601L, 1e4602L, 1e4603L, 1e4604L, 1e4605L, 1e4606L, 1e4607L, 1e4608L, 1e4609L, 
   1e4610L, 1e4611L, 1e4612L, 1e4613L, 1e4614L, 1e4615L, 1e4616L, 1e4617L, 1e4618L, 1e4619L, 
   1e4620L, 1e4621L, 1e4622L, 1e4623L, 1e4624L, 1e4625L, 1e4626L, 1e4627L, 1e4628L, 1e4629L, 
   1e4630L, 1e4631L, 1e4632L, 1e4633L, 1e4634L, 1e4635L, 1e4636L, 1e4637L, 1e4638L, 1e4639L, 
   1e4640L, 1e4641L, 1e4642L, 1e4643L, 1e4644L, 1e4645L, 1e4646L, 1e4647L, 1e4648L, 1e4649L, 
   1e4650L, 1e4651L, 1e4652L, 1e4653L, 1e4654L, 1e4655L, 1e4656L, 1e4657L, 1e4658L, 1e4659L, 
   1e4660L, 1e4661L, 1e4662L, 1e4663L, 1e4664L, 1e4665L, 1e4666L, 1e4667L, 1e4668L, 1e4669L, 
   1e4670L, 1e4671L, 1e4672L, 1e4673L, 1e4674L, 1e4675L, 1e4676L, 1e4677L, 1e4678L, 1e4679L, 
   1e4680L, 1e4681L, 1e4682L, 1e4683L, 1e4684L, 1e4685L, 1e4686L, 1e4687L, 1e4688L, 1e4689L, 
   1e4690L, 1e4691L, 1e4692L, 1e4693L, 1e4694L, 1e4695L, 1e4696L, 1e4697L, 1e4698L, 1e4699L, 
   1e4700L, 1e4701L, 1e4702L, 1e4703L, 1e4704L, 1e4705L, 1e4706L, 1e4707L, 1e4708L, 1e4709L, 
   1e4710L, 1e4711L, 1e4712L, 1e4713L, 1e4714L, 1e4715L, 1e4716L, 1e4717L, 1e4718L, 1e4719L, 
   1e4720L, 1e4721L, 1e4722L, 1e4723L, 1e4724L, 1e4725L, 1e4726L, 1e4727L, 1e4728L, 1e4729L, 
   1e4730L, 1e4731L, 1e4732L, 1e4733L, 1e4734L, 1e4735L, 1e4736L, 1e4737L, 1e4738L, 1e4739L, 
   1e4740L, 1e4741L, 1e4742L, 1e4743L, 1e4744L, 1e4745L, 1e4746L, 1e4747L, 1e4748L, 1e4749L, 
   1e4750L, 1e4751L, 1e4752L, 1e4753L, 1e4754L, 1e4755L, 1e4756L, 1e4757L, 1e4758L, 1e4759L, 
   1e4760L, 1e4761L, 1e4762L, 1e4763L, 1e4764L, 1e4765L, 1e4766L, 1e4767L, 1e4768L, 1e4769L, 
   1e4770L, 1e4771L, 1e4772L, 1e4773L, 1e4774L, 1e4775L, 1e4776L, 1e4777L, 1e4778L, 1e4779L, 
   1e4780L, 1e4781L, 1e4782L, 1e4783L, 1e4784L, 1e4785L, 1e4786L, 1e4787L, 1e4788L, 1e4789L, 
   1e4790L, 1e4791L, 1e4792L, 1e4793L, 1e4794L, 1e4795L, 1e4796L, 1e4797L, 1e4798L, 1e4799L, 
   1e4800L, 1e4801L, 1e4802L, 1e4803L, 1e4804L, 1e4805L, 1e4806L, 1e4807L, 1e4808L, 1e4809L, 
   1e4810L, 1e4811L, 1e4812L, 1e4813L, 1e4814L, 1e4815L, 1e4816L, 1e4817L, 1e4818L, 1e4819L, 
   1e4820L, 1e4821L, 1e4822L, 1e4823L, 1e4824L, 1e4825L, 1e4826L, 1e4827L, 1e4828L, 1e4829L, 
   1e4830L, 1e4831L, 1e4832L, 1e4833L, 1e4834L, 1e4835L, 1e4836L, 1e4837L, 1e4838L, 1e4839L, 
   1e4840L, 1e4841L, 1e4842L, 1e4843L, 1e4844L, 1e4845L, 1e4846L, 1e4847L, 1e4848L, 1e4849L, 
   1e4850L, 1e4851L, 1e4852L, 1e4853L, 1e4854L, 1e4855L, 1e4856L, 1e4857L, 1e4858L, 1e4859L, 
   1e4860L, 1e4861L, 1e4862L, 1e4863L, 1e4864L, 1e4865L, 1e4866L, 1e4867L, 1e4868L, 1e4869L, 
   1e4870L, 1e4871L, 1e4872L, 1e4873L, 1e4874L, 1e4875L, 1e4876L, 1e4877L, 1e4878L, 1e4879L, 
   1e4880L, 1e4881L, 1e4882L, 1e4883L, 1e4884L, 1e4885L, 1e4886L, 1e4887L, 1e4888L, 1e4889L, 
   1e4890L, 1e4891L, 1e4892L, 1e4893L, 1e4894L, 1e4895L, 1e4896L, 1e4897L, 1e4898L, 1e4899L, 
   1e4900L, 1e4901L, 1e4902L, 1e4903L, 1e4904L, 1e4905L, 1e4906L, 1e4907L, 1e4908L, 1e4909L, 
   1e4910L, 1e4911L, 1e4912L, 1e4913L, 1e4914L, 1e4915L, 1e4916L, 1e4917L, 1e4918L, 1e4919L, 
   1e4920L, 1e4921L, 1e4922L, 1e4923L, 1e4924L, 1e4925L, 1e4926L, 1e4927L, 1e4928L, 1e4929L, 
   1e4930L, 1e4931L, 1e4932L
  };

static const long double ldblnegpowersOf10[] =  /* negative powers of 10 array for long double from 0 to -4950 (min denormalised) */
  {
   1e0L, 1e-1L, 1e-2L, 1e-3L, 1e-4L, 1e-5L, 1e-6L, 1e-7L, 1e-8L, 1e-9L, 
   1e-10L, 1e-11L, 1e-12L, 1e-13L, 1e-14L, 1e-15L, 1e-16L, 1e-17L, 1e-18L, 1e-19L, 
   1e-20L, 1e-21L, 1e-22L, 1e-23L, 1e-24L, 1e-25L, 1e-26L, 1e-27L, 1e-28L, 1e-29L, 
   1e-30L, 1e-31L, 1e-32L, 1e-33L, 1e-34L, 1e-35L, 1e-36L, 1e-37L, 1e-38L, 1e-39L, 
   1e-40L, 1e-41L, 1e-42L, 1e-43L, 1e-44L, 1e-45L, 1e-46L, 1e-47L, 1e-48L, 1e-49L, 
   1e-50L, 1e-51L, 1e-52L, 1e-53L, 1e-54L, 1e-55L, 1e-56L, 1e-57L, 1e-58L, 1e-59L, 
   1e-60L, 1e-61L, 1e-62L, 1e-63L, 1e-64L, 1e-65L, 1e-66L, 1e-67L, 1e-68L, 1e-69L, 
   1e-70L, 1e-71L, 1e-72L, 1e-73L, 1e-74L, 1e-75L, 1e-76L, 1e-77L, 1e-78L, 1e-79L, 
   1e-80L, 1e-81L, 1e-82L, 1e-83L, 1e-84L, 1e-85L, 1e-86L, 1e-87L, 1e-88L, 1e-89L, 
   1e-90L, 1e-91L, 1e-92L, 1e-93L, 1e-94L, 1e-95L, 1e-96L, 1e-97L, 1e-98L, 1e-99L, 
   1e-100L, 1e-101L, 1e-102L, 1e-103L, 1e-104L, 1e-105L, 1e-106L, 1e-107L, 1e-108L, 1e-109L, 
   1e-110L, 1e-111L, 1e-112L, 1e-113L, 1e-114L, 1e-115L, 1e-116L, 1e-117L, 1e-118L, 1e-119L, 
   1e-120L, 1e-121L, 1e-122L, 1e-123L, 1e-124L, 1e-125L, 1e-126L, 1e-127L, 1e-128L, 1e-129L, 
   1e-130L, 1e-131L, 1e-132L, 1e-133L, 1e-134L, 1e-135L, 1e-136L, 1e-137L, 1e-138L, 1e-139L, 
   1e-140L, 1e-141L, 1e-142L, 1e-143L, 1e-144L, 1e-145L, 1e-146L, 1e-147L, 1e-148L, 1e-149L, 
   1e-150L, 1e-151L, 1e-152L, 1e-153L, 1e-154L, 1e-155L, 1e-156L, 1e-157L, 1e-158L, 1e-159L, 
   1e-160L, 1e-161L, 1e-162L, 1e-163L, 1e-164L, 1e-165L, 1e-166L, 1e-167L, 1e-168L, 1e-169L, 
   1e-170L, 1e-171L, 1e-172L, 1e-173L, 1e-174L, 1e-175L, 1e-176L, 1e-177L, 1e-178L, 1e-179L, 
   1e-180L, 1e-181L, 1e-182L, 1e-183L, 1e-184L, 1e-185L, 1e-186L, 1e-187L, 1e-188L, 1e-189L, 
   1e-190L, 1e-191L, 1e-192L, 1e-193L, 1e-194L, 1e-195L, 1e-196L, 1e-197L, 1e-198L, 1e-199L, 
   1e-200L, 1e-201L, 1e-202L, 1e-203L, 1e-204L, 1e-205L, 1e-206L, 1e-207L, 1e-208L, 1e-209L, 
   1e-210L, 1e-211L, 1e-212L, 1e-213L, 1e-214L, 1e-215L, 1e-216L, 1e-217L, 1e-218L, 1e-219L, 
   1e-220L, 1e-221L, 1e-222L, 1e-223L, 1e-224L, 1e-225L, 1e-226L, 1e-227L, 1e-228L, 1e-229L, 
   1e-230L, 1e-231L, 1e-232L, 1e-233L, 1e-234L, 1e-235L, 1e-236L, 1e-237L, 1e-238L, 1e-239L, 
   1e-240L, 1e-241L, 1e-242L, 1e-243L, 1e-244L, 1e-245L, 1e-246L, 1e-247L, 1e-248L, 1e-249L, 
   1e-250L, 1e-251L, 1e-252L, 1e-253L, 1e-254L, 1e-255L, 1e-256L, 1e-257L, 1e-258L, 1e-259L, 
   1e-260L, 1e-261L, 1e-262L, 1e-263L, 1e-264L, 1e-265L, 1e-266L, 1e-267L, 1e-268L, 1e-269L, 
   1e-270L, 1e-271L, 1e-272L, 1e-273L, 1e-274L, 1e-275L, 1e-276L, 1e-277L, 1e-278L, 1e-279L, 
   1e-280L, 1e-281L, 1e-282L, 1e-283L, 1e-284L, 1e-285L, 1e-286L, 1e-287L, 1e-288L, 1e-289L, 
   1e-290L, 1e-291L, 1e-292L, 1e-293L, 1e-294L, 1e-295L, 1e-296L, 1e-297L, 1e-298L, 1e-299L, 
   1e-300L, 1e-301L, 1e-302L, 1e-303L, 1e-304L, 1e-305L, 1e-306L, 1e-307L, 1e-308L, 1e-309L, 
   1e-310L, 1e-311L, 1e-312L, 1e-313L, 1e-314L, 1e-315L, 1e-316L, 1e-317L, 1e-318L, 1e-319L, 
   1e-320L, 1e-321L, 1e-322L, 1e-323L, 1e-324L, 1e-325L, 1e-326L, 1e-327L, 1e-328L, 1e-329L, 
   1e-330L, 1e-331L, 1e-332L, 1e-333L, 1e-334L, 1e-335L, 1e-336L, 1e-337L, 1e-338L, 1e-339L, 
   1e-340L, 1e-341L, 1e-342L, 1e-343L, 1e-344L, 1e-345L, 1e-346L, 1e-347L, 1e-348L, 1e-349L, 
   1e-350L, 1e-351L, 1e-352L, 1e-353L, 1e-354L, 1e-355L, 1e-356L, 1e-357L, 1e-358L, 1e-359L, 
   1e-360L, 1e-361L, 1e-362L, 1e-363L, 1e-364L, 1e-365L, 1e-366L, 1e-367L, 1e-368L, 1e-369L, 
   1e-370L, 1e-371L, 1e-372L, 1e-373L, 1e-374L, 1e-375L, 1e-376L, 1e-377L, 1e-378L, 1e-379L, 
   1e-380L, 1e-381L, 1e-382L, 1e-383L, 1e-384L, 1e-385L, 1e-386L, 1e-387L, 1e-388L, 1e-389L, 
   1e-390L, 1e-391L, 1e-392L, 1e-393L, 1e-394L, 1e-395L, 1e-396L, 1e-397L, 1e-398L, 1e-399L, 
   1e-400L, 1e-401L, 1e-402L, 1e-403L, 1e-404L, 1e-405L, 1e-406L, 1e-407L, 1e-408L, 1e-409L, 
   1e-410L, 1e-411L, 1e-412L, 1e-413L, 1e-414L, 1e-415L, 1e-416L, 1e-417L, 1e-418L, 1e-419L, 
   1e-420L, 1e-421L, 1e-422L, 1e-423L, 1e-424L, 1e-425L, 1e-426L, 1e-427L, 1e-428L, 1e-429L, 
   1e-430L, 1e-431L, 1e-432L, 1e-433L, 1e-434L, 1e-435L, 1e-436L, 1e-437L, 1e-438L, 1e-439L, 
   1e-440L, 1e-441L, 1e-442L, 1e-443L, 1e-444L, 1e-445L, 1e-446L, 1e-447L, 1e-448L, 1e-449L, 
   1e-450L, 1e-451L, 1e-452L, 1e-453L, 1e-454L, 1e-455L, 1e-456L, 1e-457L, 1e-458L, 1e-459L, 
   1e-460L, 1e-461L, 1e-462L, 1e-463L, 1e-464L, 1e-465L, 1e-466L, 1e-467L, 1e-468L, 1e-469L, 
   1e-470L, 1e-471L, 1e-472L, 1e-473L, 1e-474L, 1e-475L, 1e-476L, 1e-477L, 1e-478L, 1e-479L, 
   1e-480L, 1e-481L, 1e-482L, 1e-483L, 1e-484L, 1e-485L, 1e-486L, 1e-487L, 1e-488L, 1e-489L, 
   1e-490L, 1e-491L, 1e-492L, 1e-493L, 1e-494L, 1e-495L, 1e-496L, 1e-497L, 1e-498L, 1e-499L, 
   1e-500L, 1e-501L, 1e-502L, 1e-503L, 1e-504L, 1e-505L, 1e-506L, 1e-507L, 1e-508L, 1e-509L, 
   1e-510L, 1e-511L, 1e-512L, 1e-513L, 1e-514L, 1e-515L, 1e-516L, 1e-517L, 1e-518L, 1e-519L, 
   1e-520L, 1e-521L, 1e-522L, 1e-523L, 1e-524L, 1e-525L, 1e-526L, 1e-527L, 1e-528L, 1e-529L, 
   1e-530L, 1e-531L, 1e-532L, 1e-533L, 1e-534L, 1e-535L, 1e-536L, 1e-537L, 1e-538L, 1e-539L, 
   1e-540L, 1e-541L, 1e-542L, 1e-543L, 1e-544L, 1e-545L, 1e-546L, 1e-547L, 1e-548L, 1e-549L, 
   1e-550L, 1e-551L, 1e-552L, 1e-553L, 1e-554L, 1e-555L, 1e-556L, 1e-557L, 1e-558L, 1e-559L, 
   1e-560L, 1e-561L, 1e-562L, 1e-563L, 1e-564L, 1e-565L, 1e-566L, 1e-567L, 1e-568L, 1e-569L, 
   1e-570L, 1e-571L, 1e-572L, 1e-573L, 1e-574L, 1e-575L, 1e-576L, 1e-577L, 1e-578L, 1e-579L, 
   1e-580L, 1e-581L, 1e-582L, 1e-583L, 1e-584L, 1e-585L, 1e-586L, 1e-587L, 1e-588L, 1e-589L, 
   1e-590L, 1e-591L, 1e-592L, 1e-593L, 1e-594L, 1e-595L, 1e-596L, 1e-597L, 1e-598L, 1e-599L, 
   1e-600L, 1e-601L, 1e-602L, 1e-603L, 1e-604L, 1e-605L, 1e-606L, 1e-607L, 1e-608L, 1e-609L, 
   1e-610L, 1e-611L, 1e-612L, 1e-613L, 1e-614L, 1e-615L, 1e-616L, 1e-617L, 1e-618L, 1e-619L, 
   1e-620L, 1e-621L, 1e-622L, 1e-623L, 1e-624L, 1e-625L, 1e-626L, 1e-627L, 1e-628L, 1e-629L, 
   1e-630L, 1e-631L, 1e-632L, 1e-633L, 1e-634L, 1e-635L, 1e-636L, 1e-637L, 1e-638L, 1e-639L, 
   1e-640L, 1e-641L, 1e-642L, 1e-643L, 1e-644L, 1e-645L, 1e-646L, 1e-647L, 1e-648L, 1e-649L, 
   1e-650L, 1e-651L, 1e-652L, 1e-653L, 1e-654L, 1e-655L, 1e-656L, 1e-657L, 1e-658L, 1e-659L, 
   1e-660L, 1e-661L, 1e-662L, 1e-663L, 1e-664L, 1e-665L, 1e-666L, 1e-667L, 1e-668L, 1e-669L, 
   1e-670L, 1e-671L, 1e-672L, 1e-673L, 1e-674L, 1e-675L, 1e-676L, 1e-677L, 1e-678L, 1e-679L, 
   1e-680L, 1e-681L, 1e-682L, 1e-683L, 1e-684L, 1e-685L, 1e-686L, 1e-687L, 1e-688L, 1e-689L, 
   1e-690L, 1e-691L, 1e-692L, 1e-693L, 1e-694L, 1e-695L, 1e-696L, 1e-697L, 1e-698L, 1e-699L, 
   1e-700L, 1e-701L, 1e-702L, 1e-703L, 1e-704L, 1e-705L, 1e-706L, 1e-707L, 1e-708L, 1e-709L, 
   1e-710L, 1e-711L, 1e-712L, 1e-713L, 1e-714L, 1e-715L, 1e-716L, 1e-717L, 1e-718L, 1e-719L, 
   1e-720L, 1e-721L, 1e-722L, 1e-723L, 1e-724L, 1e-725L, 1e-726L, 1e-727L, 1e-728L, 1e-729L, 
   1e-730L, 1e-731L, 1e-732L, 1e-733L, 1e-734L, 1e-735L, 1e-736L, 1e-737L, 1e-738L, 1e-739L, 
   1e-740L, 1e-741L, 1e-742L, 1e-743L, 1e-744L, 1e-745L, 1e-746L, 1e-747L, 1e-748L, 1e-749L, 
   1e-750L, 1e-751L, 1e-752L, 1e-753L, 1e-754L, 1e-755L, 1e-756L, 1e-757L, 1e-758L, 1e-759L, 
   1e-760L, 1e-761L, 1e-762L, 1e-763L, 1e-764L, 1e-765L, 1e-766L, 1e-767L, 1e-768L, 1e-769L, 
   1e-770L, 1e-771L, 1e-772L, 1e-773L, 1e-774L, 1e-775L, 1e-776L, 1e-777L, 1e-778L, 1e-779L, 
   1e-780L, 1e-781L, 1e-782L, 1e-783L, 1e-784L, 1e-785L, 1e-786L, 1e-787L, 1e-788L, 1e-789L, 
   1e-790L, 1e-791L, 1e-792L, 1e-793L, 1e-794L, 1e-795L, 1e-796L, 1e-797L, 1e-798L, 1e-799L, 
   1e-800L, 1e-801L, 1e-802L, 1e-803L, 1e-804L, 1e-805L, 1e-806L, 1e-807L, 1e-808L, 1e-809L, 
   1e-810L, 1e-811L, 1e-812L, 1e-813L, 1e-814L, 1e-815L, 1e-816L, 1e-817L, 1e-818L, 1e-819L, 
   1e-820L, 1e-821L, 1e-822L, 1e-823L, 1e-824L, 1e-825L, 1e-826L, 1e-827L, 1e-828L, 1e-829L, 
   1e-830L, 1e-831L, 1e-832L, 1e-833L, 1e-834L, 1e-835L, 1e-836L, 1e-837L, 1e-838L, 1e-839L, 
   1e-840L, 1e-841L, 1e-842L, 1e-843L, 1e-844L, 1e-845L, 1e-846L, 1e-847L, 1e-848L, 1e-849L, 
   1e-850L, 1e-851L, 1e-852L, 1e-853L, 1e-854L, 1e-855L, 1e-856L, 1e-857L, 1e-858L, 1e-859L, 
   1e-860L, 1e-861L, 1e-862L, 1e-863L, 1e-864L, 1e-865L, 1e-866L, 1e-867L, 1e-868L, 1e-869L, 
   1e-870L, 1e-871L, 1e-872L, 1e-873L, 1e-874L, 1e-875L, 1e-876L, 1e-877L, 1e-878L, 1e-879L, 
   1e-880L, 1e-881L, 1e-882L, 1e-883L, 1e-884L, 1e-885L, 1e-886L, 1e-887L, 1e-888L, 1e-889L, 
   1e-890L, 1e-891L, 1e-892L, 1e-893L, 1e-894L, 1e-895L, 1e-896L, 1e-897L, 1e-898L, 1e-899L, 
   1e-900L, 1e-901L, 1e-902L, 1e-903L, 1e-904L, 1e-905L, 1e-906L, 1e-907L, 1e-908L, 1e-909L, 
   1e-910L, 1e-911L, 1e-912L, 1e-913L, 1e-914L, 1e-915L, 1e-916L, 1e-917L, 1e-918L, 1e-919L, 
   1e-920L, 1e-921L, 1e-922L, 1e-923L, 1e-924L, 1e-925L, 1e-926L, 1e-927L, 1e-928L, 1e-929L, 
   1e-930L, 1e-931L, 1e-932L, 1e-933L, 1e-934L, 1e-935L, 1e-936L, 1e-937L, 1e-938L, 1e-939L, 
   1e-940L, 1e-941L, 1e-942L, 1e-943L, 1e-944L, 1e-945L, 1e-946L, 1e-947L, 1e-948L, 1e-949L, 
   1e-950L, 1e-951L, 1e-952L, 1e-953L, 1e-954L, 1e-955L, 1e-956L, 1e-957L, 1e-958L, 1e-959L, 
   1e-960L, 1e-961L, 1e-962L, 1e-963L, 1e-964L, 1e-965L, 1e-966L, 1e-967L, 1e-968L, 1e-969L, 
   1e-970L, 1e-971L, 1e-972L, 1e-973L, 1e-974L, 1e-975L, 1e-976L, 1e-977L, 1e-978L, 1e-979L, 
   1e-980L, 1e-981L, 1e-982L, 1e-983L, 1e-984L, 1e-985L, 1e-986L, 1e-987L, 1e-988L, 1e-989L, 
   1e-990L, 1e-991L, 1e-992L, 1e-993L, 1e-994L, 1e-995L, 1e-996L, 1e-997L, 1e-998L, 1e-999L, 
   1e-1000L, 1e-1001L, 1e-1002L, 1e-1003L, 1e-1004L, 1e-1005L, 1e-1006L, 1e-1007L, 1e-1008L, 1e-1009L, 
   1e-1010L, 1e-1011L, 1e-1012L, 1e-1013L, 1e-1014L, 1e-1015L, 1e-1016L, 1e-1017L, 1e-1018L, 1e-1019L, 
   1e-1020L, 1e-1021L, 1e-1022L, 1e-1023L, 1e-1024L, 1e-1025L, 1e-1026L, 1e-1027L, 1e-1028L, 1e-1029L, 
   1e-1030L, 1e-1031L, 1e-1032L, 1e-1033L, 1e-1034L, 1e-1035L, 1e-1036L, 1e-1037L, 1e-1038L, 1e-1039L, 
   1e-1040L, 1e-1041L, 1e-1042L, 1e-1043L, 1e-1044L, 1e-1045L, 1e-1046L, 1e-1047L, 1e-1048L, 1e-1049L, 
   1e-1050L, 1e-1051L, 1e-1052L, 1e-1053L, 1e-1054L, 1e-1055L, 1e-1056L, 1e-1057L, 1e-1058L, 1e-1059L, 
   1e-1060L, 1e-1061L, 1e-1062L, 1e-1063L, 1e-1064L, 1e-1065L, 1e-1066L, 1e-1067L, 1e-1068L, 1e-1069L, 
   1e-1070L, 1e-1071L, 1e-1072L, 1e-1073L, 1e-1074L, 1e-1075L, 1e-1076L, 1e-1077L, 1e-1078L, 1e-1079L, 
   1e-1080L, 1e-1081L, 1e-1082L, 1e-1083L, 1e-1084L, 1e-1085L, 1e-1086L, 1e-1087L, 1e-1088L, 1e-1089L, 
   1e-1090L, 1e-1091L, 1e-1092L, 1e-1093L, 1e-1094L, 1e-1095L, 1e-1096L, 1e-1097L, 1e-1098L, 1e-1099L, 
   1e-1100L, 1e-1101L, 1e-1102L, 1e-1103L, 1e-1104L, 1e-1105L, 1e-1106L, 1e-1107L, 1e-1108L, 1e-1109L, 
   1e-1110L, 1e-1111L, 1e-1112L, 1e-1113L, 1e-1114L, 1e-1115L, 1e-1116L, 1e-1117L, 1e-1118L, 1e-1119L, 
   1e-1120L, 1e-1121L, 1e-1122L, 1e-1123L, 1e-1124L, 1e-1125L, 1e-1126L, 1e-1127L, 1e-1128L, 1e-1129L, 
   1e-1130L, 1e-1131L, 1e-1132L, 1e-1133L, 1e-1134L, 1e-1135L, 1e-1136L, 1e-1137L, 1e-1138L, 1e-1139L, 
   1e-1140L, 1e-1141L, 1e-1142L, 1e-1143L, 1e-1144L, 1e-1145L, 1e-1146L, 1e-1147L, 1e-1148L, 1e-1149L, 
   1e-1150L, 1e-1151L, 1e-1152L, 1e-1153L, 1e-1154L, 1e-1155L, 1e-1156L, 1e-1157L, 1e-1158L, 1e-1159L, 
   1e-1160L, 1e-1161L, 1e-1162L, 1e-1163L, 1e-1164L, 1e-1165L, 1e-1166L, 1e-1167L, 1e-1168L, 1e-1169L, 
   1e-1170L, 1e-1171L, 1e-1172L, 1e-1173L, 1e-1174L, 1e-1175L, 1e-1176L, 1e-1177L, 1e-1178L, 1e-1179L, 
   1e-1180L, 1e-1181L, 1e-1182L, 1e-1183L, 1e-1184L, 1e-1185L, 1e-1186L, 1e-1187L, 1e-1188L, 1e-1189L, 
   1e-1190L, 1e-1191L, 1e-1192L, 1e-1193L, 1e-1194L, 1e-1195L, 1e-1196L, 1e-1197L, 1e-1198L, 1e-1199L, 
   1e-1200L, 1e-1201L, 1e-1202L, 1e-1203L, 1e-1204L, 1e-1205L, 1e-1206L, 1e-1207L, 1e-1208L, 1e-1209L, 
   1e-1210L, 1e-1211L, 1e-1212L, 1e-1213L, 1e-1214L, 1e-1215L, 1e-1216L, 1e-1217L, 1e-1218L, 1e-1219L, 
   1e-1220L, 1e-1221L, 1e-1222L, 1e-1223L, 1e-1224L, 1e-1225L, 1e-1226L, 1e-1227L, 1e-1228L, 1e-1229L, 
   1e-1230L, 1e-1231L, 1e-1232L, 1e-1233L, 1e-1234L, 1e-1235L, 1e-1236L, 1e-1237L, 1e-1238L, 1e-1239L, 
   1e-1240L, 1e-1241L, 1e-1242L, 1e-1243L, 1e-1244L, 1e-1245L, 1e-1246L, 1e-1247L, 1e-1248L, 1e-1249L, 
   1e-1250L, 1e-1251L, 1e-1252L, 1e-1253L, 1e-1254L, 1e-1255L, 1e-1256L, 1e-1257L, 1e-1258L, 1e-1259L, 
   1e-1260L, 1e-1261L, 1e-1262L, 1e-1263L, 1e-1264L, 1e-1265L, 1e-1266L, 1e-1267L, 1e-1268L, 1e-1269L, 
   1e-1270L, 1e-1271L, 1e-1272L, 1e-1273L, 1e-1274L, 1e-1275L, 1e-1276L, 1e-1277L, 1e-1278L, 1e-1279L, 
   1e-1280L, 1e-1281L, 1e-1282L, 1e-1283L, 1e-1284L, 1e-1285L, 1e-1286L, 1e-1287L, 1e-1288L, 1e-1289L, 
   1e-1290L, 1e-1291L, 1e-1292L, 1e-1293L, 1e-1294L, 1e-1295L, 1e-1296L, 1e-1297L, 1e-1298L, 1e-1299L, 
   1e-1300L, 1e-1301L, 1e-1302L, 1e-1303L, 1e-1304L, 1e-1305L, 1e-1306L, 1e-1307L, 1e-1308L, 1e-1309L, 
   1e-1310L, 1e-1311L, 1e-1312L, 1e-1313L, 1e-1314L, 1e-1315L, 1e-1316L, 1e-1317L, 1e-1318L, 1e-1319L, 
   1e-1320L, 1e-1321L, 1e-1322L, 1e-1323L, 1e-1324L, 1e-1325L, 1e-1326L, 1e-1327L, 1e-1328L, 1e-1329L, 
   1e-1330L, 1e-1331L, 1e-1332L, 1e-1333L, 1e-1334L, 1e-1335L, 1e-1336L, 1e-1337L, 1e-1338L, 1e-1339L, 
   1e-1340L, 1e-1341L, 1e-1342L, 1e-1343L, 1e-1344L, 1e-1345L, 1e-1346L, 1e-1347L, 1e-1348L, 1e-1349L, 
   1e-1350L, 1e-1351L, 1e-1352L, 1e-1353L, 1e-1354L, 1e-1355L, 1e-1356L, 1e-1357L, 1e-1358L, 1e-1359L, 
   1e-1360L, 1e-1361L, 1e-1362L, 1e-1363L, 1e-1364L, 1e-1365L, 1e-1366L, 1e-1367L, 1e-1368L, 1e-1369L, 
   1e-1370L, 1e-1371L, 1e-1372L, 1e-1373L, 1e-1374L, 1e-1375L, 1e-1376L, 1e-1377L, 1e-1378L, 1e-1379L, 
   1e-1380L, 1e-1381L, 1e-1382L, 1e-1383L, 1e-1384L, 1e-1385L, 1e-1386L, 1e-1387L, 1e-1388L, 1e-1389L, 
   1e-1390L, 1e-1391L, 1e-1392L, 1e-1393L, 1e-1394L, 1e-1395L, 1e-1396L, 1e-1397L, 1e-1398L, 1e-1399L, 
   1e-1400L, 1e-1401L, 1e-1402L, 1e-1403L, 1e-1404L, 1e-1405L, 1e-1406L, 1e-1407L, 1e-1408L, 1e-1409L, 
   1e-1410L, 1e-1411L, 1e-1412L, 1e-1413L, 1e-1414L, 1e-1415L, 1e-1416L, 1e-1417L, 1e-1418L, 1e-1419L, 
   1e-1420L, 1e-1421L, 1e-1422L, 1e-1423L, 1e-1424L, 1e-1425L, 1e-1426L, 1e-1427L, 1e-1428L, 1e-1429L, 
   1e-1430L, 1e-1431L, 1e-1432L, 1e-1433L, 1e-1434L, 1e-1435L, 1e-1436L, 1e-1437L, 1e-1438L, 1e-1439L, 
   1e-1440L, 1e-1441L, 1e-1442L, 1e-1443L, 1e-1444L, 1e-1445L, 1e-1446L, 1e-1447L, 1e-1448L, 1e-1449L, 
   1e-1450L, 1e-1451L, 1e-1452L, 1e-1453L, 1e-1454L, 1e-1455L, 1e-1456L, 1e-1457L, 1e-1458L, 1e-1459L, 
   1e-1460L, 1e-1461L, 1e-1462L, 1e-1463L, 1e-1464L, 1e-1465L, 1e-1466L, 1e-1467L, 1e-1468L, 1e-1469L, 
   1e-1470L, 1e-1471L, 1e-1472L, 1e-1473L, 1e-1474L, 1e-1475L, 1e-1476L, 1e-1477L, 1e-1478L, 1e-1479L, 
   1e-1480L, 1e-1481L, 1e-1482L, 1e-1483L, 1e-1484L, 1e-1485L, 1e-1486L, 1e-1487L, 1e-1488L, 1e-1489L, 
   1e-1490L, 1e-1491L, 1e-1492L, 1e-1493L, 1e-1494L, 1e-1495L, 1e-1496L, 1e-1497L, 1e-1498L, 1e-1499L, 
   1e-1500L, 1e-1501L, 1e-1502L, 1e-1503L, 1e-1504L, 1e-1505L, 1e-1506L, 1e-1507L, 1e-1508L, 1e-1509L, 
   1e-1510L, 1e-1511L, 1e-1512L, 1e-1513L, 1e-1514L, 1e-1515L, 1e-1516L, 1e-1517L, 1e-1518L, 1e-1519L, 
   1e-1520L, 1e-1521L, 1e-1522L, 1e-1523L, 1e-1524L, 1e-1525L, 1e-1526L, 1e-1527L, 1e-1528L, 1e-1529L, 
   1e-1530L, 1e-1531L, 1e-1532L, 1e-1533L, 1e-1534L, 1e-1535L, 1e-1536L, 1e-1537L, 1e-1538L, 1e-1539L, 
   1e-1540L, 1e-1541L, 1e-1542L, 1e-1543L, 1e-1544L, 1e-1545L, 1e-1546L, 1e-1547L, 1e-1548L, 1e-1549L, 
   1e-1550L, 1e-1551L, 1e-1552L, 1e-1553L, 1e-1554L, 1e-1555L, 1e-1556L, 1e-1557L, 1e-1558L, 1e-1559L, 
   1e-1560L, 1e-1561L, 1e-1562L, 1e-1563L, 1e-1564L, 1e-1565L, 1e-1566L, 1e-1567L, 1e-1568L, 1e-1569L, 
   1e-1570L, 1e-1571L, 1e-1572L, 1e-1573L, 1e-1574L, 1e-1575L, 1e-1576L, 1e-1577L, 1e-1578L, 1e-1579L, 
   1e-1580L, 1e-1581L, 1e-1582L, 1e-1583L, 1e-1584L, 1e-1585L, 1e-1586L, 1e-1587L, 1e-1588L, 1e-1589L, 
   1e-1590L, 1e-1591L, 1e-1592L, 1e-1593L, 1e-1594L, 1e-1595L, 1e-1596L, 1e-1597L, 1e-1598L, 1e-1599L, 
   1e-1600L, 1e-1601L, 1e-1602L, 1e-1603L, 1e-1604L, 1e-1605L, 1e-1606L, 1e-1607L, 1e-1608L, 1e-1609L, 
   1e-1610L, 1e-1611L, 1e-1612L, 1e-1613L, 1e-1614L, 1e-1615L, 1e-1616L, 1e-1617L, 1e-1618L, 1e-1619L, 
   1e-1620L, 1e-1621L, 1e-1622L, 1e-1623L, 1e-1624L, 1e-1625L, 1e-1626L, 1e-1627L, 1e-1628L, 1e-1629L, 
   1e-1630L, 1e-1631L, 1e-1632L, 1e-1633L, 1e-1634L, 1e-1635L, 1e-1636L, 1e-1637L, 1e-1638L, 1e-1639L, 
   1e-1640L, 1e-1641L, 1e-1642L, 1e-1643L, 1e-1644L, 1e-1645L, 1e-1646L, 1e-1647L, 1e-1648L, 1e-1649L, 
   1e-1650L, 1e-1651L, 1e-1652L, 1e-1653L, 1e-1654L, 1e-1655L, 1e-1656L, 1e-1657L, 1e-1658L, 1e-1659L, 
   1e-1660L, 1e-1661L, 1e-1662L, 1e-1663L, 1e-1664L, 1e-1665L, 1e-1666L, 1e-1667L, 1e-1668L, 1e-1669L, 
   1e-1670L, 1e-1671L, 1e-1672L, 1e-1673L, 1e-1674L, 1e-1675L, 1e-1676L, 1e-1677L, 1e-1678L, 1e-1679L, 
   1e-1680L, 1e-1681L, 1e-1682L, 1e-1683L, 1e-1684L, 1e-1685L, 1e-1686L, 1e-1687L, 1e-1688L, 1e-1689L, 
   1e-1690L, 1e-1691L, 1e-1692L, 1e-1693L, 1e-1694L, 1e-1695L, 1e-1696L, 1e-1697L, 1e-1698L, 1e-1699L, 
   1e-1700L, 1e-1701L, 1e-1702L, 1e-1703L, 1e-1704L, 1e-1705L, 1e-1706L, 1e-1707L, 1e-1708L, 1e-1709L, 
   1e-1710L, 1e-1711L, 1e-1712L, 1e-1713L, 1e-1714L, 1e-1715L, 1e-1716L, 1e-1717L, 1e-1718L, 1e-1719L, 
   1e-1720L, 1e-1721L, 1e-1722L, 1e-1723L, 1e-1724L, 1e-1725L, 1e-1726L, 1e-1727L, 1e-1728L, 1e-1729L, 
   1e-1730L, 1e-1731L, 1e-1732L, 1e-1733L, 1e-1734L, 1e-1735L, 1e-1736L, 1e-1737L, 1e-1738L, 1e-1739L, 
   1e-1740L, 1e-1741L, 1e-1742L, 1e-1743L, 1e-1744L, 1e-1745L, 1e-1746L, 1e-1747L, 1e-1748L, 1e-1749L, 
   1e-1750L, 1e-1751L, 1e-1752L, 1e-1753L, 1e-1754L, 1e-1755L, 1e-1756L, 1e-1757L, 1e-1758L, 1e-1759L, 
   1e-1760L, 1e-1761L, 1e-1762L, 1e-1763L, 1e-1764L, 1e-1765L, 1e-1766L, 1e-1767L, 1e-1768L, 1e-1769L, 
   1e-1770L, 1e-1771L, 1e-1772L, 1e-1773L, 1e-1774L, 1e-1775L, 1e-1776L, 1e-1777L, 1e-1778L, 1e-1779L, 
   1e-1780L, 1e-1781L, 1e-1782L, 1e-1783L, 1e-1784L, 1e-1785L, 1e-1786L, 1e-1787L, 1e-1788L, 1e-1789L, 
   1e-1790L, 1e-1791L, 1e-1792L, 1e-1793L, 1e-1794L, 1e-1795L, 1e-1796L, 1e-1797L, 1e-1798L, 1e-1799L, 
   1e-1800L, 1e-1801L, 1e-1802L, 1e-1803L, 1e-1804L, 1e-1805L, 1e-1806L, 1e-1807L, 1e-1808L, 1e-1809L, 
   1e-1810L, 1e-1811L, 1e-1812L, 1e-1813L, 1e-1814L, 1e-1815L, 1e-1816L, 1e-1817L, 1e-1818L, 1e-1819L, 
   1e-1820L, 1e-1821L, 1e-1822L, 1e-1823L, 1e-1824L, 1e-1825L, 1e-1826L, 1e-1827L, 1e-1828L, 1e-1829L, 
   1e-1830L, 1e-1831L, 1e-1832L, 1e-1833L, 1e-1834L, 1e-1835L, 1e-1836L, 1e-1837L, 1e-1838L, 1e-1839L, 
   1e-1840L, 1e-1841L, 1e-1842L, 1e-1843L, 1e-1844L, 1e-1845L, 1e-1846L, 1e-1847L, 1e-1848L, 1e-1849L, 
   1e-1850L, 1e-1851L, 1e-1852L, 1e-1853L, 1e-1854L, 1e-1855L, 1e-1856L, 1e-1857L, 1e-1858L, 1e-1859L, 
   1e-1860L, 1e-1861L, 1e-1862L, 1e-1863L, 1e-1864L, 1e-1865L, 1e-1866L, 1e-1867L, 1e-1868L, 1e-1869L, 
   1e-1870L, 1e-1871L, 1e-1872L, 1e-1873L, 1e-1874L, 1e-1875L, 1e-1876L, 1e-1877L, 1e-1878L, 1e-1879L, 
   1e-1880L, 1e-1881L, 1e-1882L, 1e-1883L, 1e-1884L, 1e-1885L, 1e-1886L, 1e-1887L, 1e-1888L, 1e-1889L, 
   1e-1890L, 1e-1891L, 1e-1892L, 1e-1893L, 1e-1894L, 1e-1895L, 1e-1896L, 1e-1897L, 1e-1898L, 1e-1899L, 
   1e-1900L, 1e-1901L, 1e-1902L, 1e-1903L, 1e-1904L, 1e-1905L, 1e-1906L, 1e-1907L, 1e-1908L, 1e-1909L, 
   1e-1910L, 1e-1911L, 1e-1912L, 1e-1913L, 1e-1914L, 1e-1915L, 1e-1916L, 1e-1917L, 1e-1918L, 1e-1919L, 
   1e-1920L, 1e-1921L, 1e-1922L, 1e-1923L, 1e-1924L, 1e-1925L, 1e-1926L, 1e-1927L, 1e-1928L, 1e-1929L, 
   1e-1930L, 1e-1931L, 1e-1932L, 1e-1933L, 1e-1934L, 1e-1935L, 1e-1936L, 1e-1937L, 1e-1938L, 1e-1939L, 
   1e-1940L, 1e-1941L, 1e-1942L, 1e-1943L, 1e-1944L, 1e-1945L, 1e-1946L, 1e-1947L, 1e-1948L, 1e-1949L, 
   1e-1950L, 1e-1951L, 1e-1952L, 1e-1953L, 1e-1954L, 1e-1955L, 1e-1956L, 1e-1957L, 1e-1958L, 1e-1959L, 
   1e-1960L, 1e-1961L, 1e-1962L, 1e-1963L, 1e-1964L, 1e-1965L, 1e-1966L, 1e-1967L, 1e-1968L, 1e-1969L, 
   1e-1970L, 1e-1971L, 1e-1972L, 1e-1973L, 1e-1974L, 1e-1975L, 1e-1976L, 1e-1977L, 1e-1978L, 1e-1979L, 
   1e-1980L, 1e-1981L, 1e-1982L, 1e-1983L, 1e-1984L, 1e-1985L, 1e-1986L, 1e-1987L, 1e-1988L, 1e-1989L, 
   1e-1990L, 1e-1991L, 1e-1992L, 1e-1993L, 1e-1994L, 1e-1995L, 1e-1996L, 1e-1997L, 1e-1998L, 1e-1999L, 
   1e-2000L, 1e-2001L, 1e-2002L, 1e-2003L, 1e-2004L, 1e-2005L, 1e-2006L, 1e-2007L, 1e-2008L, 1e-2009L, 
   1e-2010L, 1e-2011L, 1e-2012L, 1e-2013L, 1e-2014L, 1e-2015L, 1e-2016L, 1e-2017L, 1e-2018L, 1e-2019L, 
   1e-2020L, 1e-2021L, 1e-2022L, 1e-2023L, 1e-2024L, 1e-2025L, 1e-2026L, 1e-2027L, 1e-2028L, 1e-2029L, 
   1e-2030L, 1e-2031L, 1e-2032L, 1e-2033L, 1e-2034L, 1e-2035L, 1e-2036L, 1e-2037L, 1e-2038L, 1e-2039L, 
   1e-2040L, 1e-2041L, 1e-2042L, 1e-2043L, 1e-2044L, 1e-2045L, 1e-2046L, 1e-2047L, 1e-2048L, 1e-2049L, 
   1e-2050L, 1e-2051L, 1e-2052L, 1e-2053L, 1e-2054L, 1e-2055L, 1e-2056L, 1e-2057L, 1e-2058L, 1e-2059L, 
   1e-2060L, 1e-2061L, 1e-2062L, 1e-2063L, 1e-2064L, 1e-2065L, 1e-2066L, 1e-2067L, 1e-2068L, 1e-2069L, 
   1e-2070L, 1e-2071L, 1e-2072L, 1e-2073L, 1e-2074L, 1e-2075L, 1e-2076L, 1e-2077L, 1e-2078L, 1e-2079L, 
   1e-2080L, 1e-2081L, 1e-2082L, 1e-2083L, 1e-2084L, 1e-2085L, 1e-2086L, 1e-2087L, 1e-2088L, 1e-2089L, 
   1e-2090L, 1e-2091L, 1e-2092L, 1e-2093L, 1e-2094L, 1e-2095L, 1e-2096L, 1e-2097L, 1e-2098L, 1e-2099L, 
   1e-2100L, 1e-2101L, 1e-2102L, 1e-2103L, 1e-2104L, 1e-2105L, 1e-2106L, 1e-2107L, 1e-2108L, 1e-2109L, 
   1e-2110L, 1e-2111L, 1e-2112L, 1e-2113L, 1e-2114L, 1e-2115L, 1e-2116L, 1e-2117L, 1e-2118L, 1e-2119L, 
   1e-2120L, 1e-2121L, 1e-2122L, 1e-2123L, 1e-2124L, 1e-2125L, 1e-2126L, 1e-2127L, 1e-2128L, 1e-2129L, 
   1e-2130L, 1e-2131L, 1e-2132L, 1e-2133L, 1e-2134L, 1e-2135L, 1e-2136L, 1e-2137L, 1e-2138L, 1e-2139L, 
   1e-2140L, 1e-2141L, 1e-2142L, 1e-2143L, 1e-2144L, 1e-2145L, 1e-2146L, 1e-2147L, 1e-2148L, 1e-2149L, 
   1e-2150L, 1e-2151L, 1e-2152L, 1e-2153L, 1e-2154L, 1e-2155L, 1e-2156L, 1e-2157L, 1e-2158L, 1e-2159L, 
   1e-2160L, 1e-2161L, 1e-2162L, 1e-2163L, 1e-2164L, 1e-2165L, 1e-2166L, 1e-2167L, 1e-2168L, 1e-2169L, 
   1e-2170L, 1e-2171L, 1e-2172L, 1e-2173L, 1e-2174L, 1e-2175L, 1e-2176L, 1e-2177L, 1e-2178L, 1e-2179L, 
   1e-2180L, 1e-2181L, 1e-2182L, 1e-2183L, 1e-2184L, 1e-2185L, 1e-2186L, 1e-2187L, 1e-2188L, 1e-2189L, 
   1e-2190L, 1e-2191L, 1e-2192L, 1e-2193L, 1e-2194L, 1e-2195L, 1e-2196L, 1e-2197L, 1e-2198L, 1e-2199L, 
   1e-2200L, 1e-2201L, 1e-2202L, 1e-2203L, 1e-2204L, 1e-2205L, 1e-2206L, 1e-2207L, 1e-2208L, 1e-2209L, 
   1e-2210L, 1e-2211L, 1e-2212L, 1e-2213L, 1e-2214L, 1e-2215L, 1e-2216L, 1e-2217L, 1e-2218L, 1e-2219L, 
   1e-2220L, 1e-2221L, 1e-2222L, 1e-2223L, 1e-2224L, 1e-2225L, 1e-2226L, 1e-2227L, 1e-2228L, 1e-2229L, 
   1e-2230L, 1e-2231L, 1e-2232L, 1e-2233L, 1e-2234L, 1e-2235L, 1e-2236L, 1e-2237L, 1e-2238L, 1e-2239L, 
   1e-2240L, 1e-2241L, 1e-2242L, 1e-2243L, 1e-2244L, 1e-2245L, 1e-2246L, 1e-2247L, 1e-2248L, 1e-2249L, 
   1e-2250L, 1e-2251L, 1e-2252L, 1e-2253L, 1e-2254L, 1e-2255L, 1e-2256L, 1e-2257L, 1e-2258L, 1e-2259L, 
   1e-2260L, 1e-2261L, 1e-2262L, 1e-2263L, 1e-2264L, 1e-2265L, 1e-2266L, 1e-2267L, 1e-2268L, 1e-2269L, 
   1e-2270L, 1e-2271L, 1e-2272L, 1e-2273L, 1e-2274L, 1e-2275L, 1e-2276L, 1e-2277L, 1e-2278L, 1e-2279L, 
   1e-2280L, 1e-2281L, 1e-2282L, 1e-2283L, 1e-2284L, 1e-2285L, 1e-2286L, 1e-2287L, 1e-2288L, 1e-2289L, 
   1e-2290L, 1e-2291L, 1e-2292L, 1e-2293L, 1e-2294L, 1e-2295L, 1e-2296L, 1e-2297L, 1e-2298L, 1e-2299L, 
   1e-2300L, 1e-2301L, 1e-2302L, 1e-2303L, 1e-2304L, 1e-2305L, 1e-2306L, 1e-2307L, 1e-2308L, 1e-2309L, 
   1e-2310L, 1e-2311L, 1e-2312L, 1e-2313L, 1e-2314L, 1e-2315L, 1e-2316L, 1e-2317L, 1e-2318L, 1e-2319L, 
   1e-2320L, 1e-2321L, 1e-2322L, 1e-2323L, 1e-2324L, 1e-2325L, 1e-2326L, 1e-2327L, 1e-2328L, 1e-2329L, 
   1e-2330L, 1e-2331L, 1e-2332L, 1e-2333L, 1e-2334L, 1e-2335L, 1e-2336L, 1e-2337L, 1e-2338L, 1e-2339L, 
   1e-2340L, 1e-2341L, 1e-2342L, 1e-2343L, 1e-2344L, 1e-2345L, 1e-2346L, 1e-2347L, 1e-2348L, 1e-2349L, 
   1e-2350L, 1e-2351L, 1e-2352L, 1e-2353L, 1e-2354L, 1e-2355L, 1e-2356L, 1e-2357L, 1e-2358L, 1e-2359L, 
   1e-2360L, 1e-2361L, 1e-2362L, 1e-2363L, 1e-2364L, 1e-2365L, 1e-2366L, 1e-2367L, 1e-2368L, 1e-2369L, 
   1e-2370L, 1e-2371L, 1e-2372L, 1e-2373L, 1e-2374L, 1e-2375L, 1e-2376L, 1e-2377L, 1e-2378L, 1e-2379L, 
   1e-2380L, 1e-2381L, 1e-2382L, 1e-2383L, 1e-2384L, 1e-2385L, 1e-2386L, 1e-2387L, 1e-2388L, 1e-2389L, 
   1e-2390L, 1e-2391L, 1e-2392L, 1e-2393L, 1e-2394L, 1e-2395L, 1e-2396L, 1e-2397L, 1e-2398L, 1e-2399L, 
   1e-2400L, 1e-2401L, 1e-2402L, 1e-2403L, 1e-2404L, 1e-2405L, 1e-2406L, 1e-2407L, 1e-2408L, 1e-2409L, 
   1e-2410L, 1e-2411L, 1e-2412L, 1e-2413L, 1e-2414L, 1e-2415L, 1e-2416L, 1e-2417L, 1e-2418L, 1e-2419L, 
   1e-2420L, 1e-2421L, 1e-2422L, 1e-2423L, 1e-2424L, 1e-2425L, 1e-2426L, 1e-2427L, 1e-2428L, 1e-2429L, 
   1e-2430L, 1e-2431L, 1e-2432L, 1e-2433L, 1e-2434L, 1e-2435L, 1e-2436L, 1e-2437L, 1e-2438L, 1e-2439L, 
   1e-2440L, 1e-2441L, 1e-2442L, 1e-2443L, 1e-2444L, 1e-2445L, 1e-2446L, 1e-2447L, 1e-2448L, 1e-2449L, 
   1e-2450L, 1e-2451L, 1e-2452L, 1e-2453L, 1e-2454L, 1e-2455L, 1e-2456L, 1e-2457L, 1e-2458L, 1e-2459L, 
   1e-2460L, 1e-2461L, 1e-2462L, 1e-2463L, 1e-2464L, 1e-2465L, 1e-2466L, 1e-2467L, 1e-2468L, 1e-2469L, 
   1e-2470L, 1e-2471L, 1e-2472L, 1e-2473L, 1e-2474L, 1e-2475L, 1e-2476L, 1e-2477L, 1e-2478L, 1e-2479L, 
   1e-2480L, 1e-2481L, 1e-2482L, 1e-2483L, 1e-2484L, 1e-2485L, 1e-2486L, 1e-2487L, 1e-2488L, 1e-2489L, 
   1e-2490L, 1e-2491L, 1e-2492L, 1e-2493L, 1e-2494L, 1e-2495L, 1e-2496L, 1e-2497L, 1e-2498L, 1e-2499L, 
   1e-2500L, 1e-2501L, 1e-2502L, 1e-2503L, 1e-2504L, 1e-2505L, 1e-2506L, 1e-2507L, 1e-2508L, 1e-2509L, 
   1e-2510L, 1e-2511L, 1e-2512L, 1e-2513L, 1e-2514L, 1e-2515L, 1e-2516L, 1e-2517L, 1e-2518L, 1e-2519L, 
   1e-2520L, 1e-2521L, 1e-2522L, 1e-2523L, 1e-2524L, 1e-2525L, 1e-2526L, 1e-2527L, 1e-2528L, 1e-2529L, 
   1e-2530L, 1e-2531L, 1e-2532L, 1e-2533L, 1e-2534L, 1e-2535L, 1e-2536L, 1e-2537L, 1e-2538L, 1e-2539L, 
   1e-2540L, 1e-2541L, 1e-2542L, 1e-2543L, 1e-2544L, 1e-2545L, 1e-2546L, 1e-2547L, 1e-2548L, 1e-2549L, 
   1e-2550L, 1e-2551L, 1e-2552L, 1e-2553L, 1e-2554L, 1e-2555L, 1e-2556L, 1e-2557L, 1e-2558L, 1e-2559L, 
   1e-2560L, 1e-2561L, 1e-2562L, 1e-2563L, 1e-2564L, 1e-2565L, 1e-2566L, 1e-2567L, 1e-2568L, 1e-2569L, 
   1e-2570L, 1e-2571L, 1e-2572L, 1e-2573L, 1e-2574L, 1e-2575L, 1e-2576L, 1e-2577L, 1e-2578L, 1e-2579L, 
   1e-2580L, 1e-2581L, 1e-2582L, 1e-2583L, 1e-2584L, 1e-2585L, 1e-2586L, 1e-2587L, 1e-2588L, 1e-2589L, 
   1e-2590L, 1e-2591L, 1e-2592L, 1e-2593L, 1e-2594L, 1e-2595L, 1e-2596L, 1e-2597L, 1e-2598L, 1e-2599L, 
   1e-2600L, 1e-2601L, 1e-2602L, 1e-2603L, 1e-2604L, 1e-2605L, 1e-2606L, 1e-2607L, 1e-2608L, 1e-2609L, 
   1e-2610L, 1e-2611L, 1e-2612L, 1e-2613L, 1e-2614L, 1e-2615L, 1e-2616L, 1e-2617L, 1e-2618L, 1e-2619L, 
   1e-2620L, 1e-2621L, 1e-2622L, 1e-2623L, 1e-2624L, 1e-2625L, 1e-2626L, 1e-2627L, 1e-2628L, 1e-2629L, 
   1e-2630L, 1e-2631L, 1e-2632L, 1e-2633L, 1e-2634L, 1e-2635L, 1e-2636L, 1e-2637L, 1e-2638L, 1e-2639L, 
   1e-2640L, 1e-2641L, 1e-2642L, 1e-2643L, 1e-2644L, 1e-2645L, 1e-2646L, 1e-2647L, 1e-2648L, 1e-2649L, 
   1e-2650L, 1e-2651L, 1e-2652L, 1e-2653L, 1e-2654L, 1e-2655L, 1e-2656L, 1e-2657L, 1e-2658L, 1e-2659L, 
   1e-2660L, 1e-2661L, 1e-2662L, 1e-2663L, 1e-2664L, 1e-2665L, 1e-2666L, 1e-2667L, 1e-2668L, 1e-2669L, 
   1e-2670L, 1e-2671L, 1e-2672L, 1e-2673L, 1e-2674L, 1e-2675L, 1e-2676L, 1e-2677L, 1e-2678L, 1e-2679L, 
   1e-2680L, 1e-2681L, 1e-2682L, 1e-2683L, 1e-2684L, 1e-2685L, 1e-2686L, 1e-2687L, 1e-2688L, 1e-2689L, 
   1e-2690L, 1e-2691L, 1e-2692L, 1e-2693L, 1e-2694L, 1e-2695L, 1e-2696L, 1e-2697L, 1e-2698L, 1e-2699L, 
   1e-2700L, 1e-2701L, 1e-2702L, 1e-2703L, 1e-2704L, 1e-2705L, 1e-2706L, 1e-2707L, 1e-2708L, 1e-2709L, 
   1e-2710L, 1e-2711L, 1e-2712L, 1e-2713L, 1e-2714L, 1e-2715L, 1e-2716L, 1e-2717L, 1e-2718L, 1e-2719L, 
   1e-2720L, 1e-2721L, 1e-2722L, 1e-2723L, 1e-2724L, 1e-2725L, 1e-2726L, 1e-2727L, 1e-2728L, 1e-2729L, 
   1e-2730L, 1e-2731L, 1e-2732L, 1e-2733L, 1e-2734L, 1e-2735L, 1e-2736L, 1e-2737L, 1e-2738L, 1e-2739L, 
   1e-2740L, 1e-2741L, 1e-2742L, 1e-2743L, 1e-2744L, 1e-2745L, 1e-2746L, 1e-2747L, 1e-2748L, 1e-2749L, 
   1e-2750L, 1e-2751L, 1e-2752L, 1e-2753L, 1e-2754L, 1e-2755L, 1e-2756L, 1e-2757L, 1e-2758L, 1e-2759L, 
   1e-2760L, 1e-2761L, 1e-2762L, 1e-2763L, 1e-2764L, 1e-2765L, 1e-2766L, 1e-2767L, 1e-2768L, 1e-2769L, 
   1e-2770L, 1e-2771L, 1e-2772L, 1e-2773L, 1e-2774L, 1e-2775L, 1e-2776L, 1e-2777L, 1e-2778L, 1e-2779L, 
   1e-2780L, 1e-2781L, 1e-2782L, 1e-2783L, 1e-2784L, 1e-2785L, 1e-2786L, 1e-2787L, 1e-2788L, 1e-2789L, 
   1e-2790L, 1e-2791L, 1e-2792L, 1e-2793L, 1e-2794L, 1e-2795L, 1e-2796L, 1e-2797L, 1e-2798L, 1e-2799L, 
   1e-2800L, 1e-2801L, 1e-2802L, 1e-2803L, 1e-2804L, 1e-2805L, 1e-2806L, 1e-2807L, 1e-2808L, 1e-2809L, 
   1e-2810L, 1e-2811L, 1e-2812L, 1e-2813L, 1e-2814L, 1e-2815L, 1e-2816L, 1e-2817L, 1e-2818L, 1e-2819L, 
   1e-2820L, 1e-2821L, 1e-2822L, 1e-2823L, 1e-2824L, 1e-2825L, 1e-2826L, 1e-2827L, 1e-2828L, 1e-2829L, 
   1e-2830L, 1e-2831L, 1e-2832L, 1e-2833L, 1e-2834L, 1e-2835L, 1e-2836L, 1e-2837L, 1e-2838L, 1e-2839L, 
   1e-2840L, 1e-2841L, 1e-2842L, 1e-2843L, 1e-2844L, 1e-2845L, 1e-2846L, 1e-2847L, 1e-2848L, 1e-2849L, 
   1e-2850L, 1e-2851L, 1e-2852L, 1e-2853L, 1e-2854L, 1e-2855L, 1e-2856L, 1e-2857L, 1e-2858L, 1e-2859L, 
   1e-2860L, 1e-2861L, 1e-2862L, 1e-2863L, 1e-2864L, 1e-2865L, 1e-2866L, 1e-2867L, 1e-2868L, 1e-2869L, 
   1e-2870L, 1e-2871L, 1e-2872L, 1e-2873L, 1e-2874L, 1e-2875L, 1e-2876L, 1e-2877L, 1e-2878L, 1e-2879L, 
   1e-2880L, 1e-2881L, 1e-2882L, 1e-2883L, 1e-2884L, 1e-2885L, 1e-2886L, 1e-2887L, 1e-2888L, 1e-2889L, 
   1e-2890L, 1e-2891L, 1e-2892L, 1e-2893L, 1e-2894L, 1e-2895L, 1e-2896L, 1e-2897L, 1e-2898L, 1e-2899L, 
   1e-2900L, 1e-2901L, 1e-2902L, 1e-2903L, 1e-2904L, 1e-2905L, 1e-2906L, 1e-2907L, 1e-2908L, 1e-2909L, 
   1e-2910L, 1e-2911L, 1e-2912L, 1e-2913L, 1e-2914L, 1e-2915L, 1e-2916L, 1e-2917L, 1e-2918L, 1e-2919L, 
   1e-2920L, 1e-2921L, 1e-2922L, 1e-2923L, 1e-2924L, 1e-2925L, 1e-2926L, 1e-2927L, 1e-2928L, 1e-2929L, 
   1e-2930L, 1e-2931L, 1e-2932L, 1e-2933L, 1e-2934L, 1e-2935L, 1e-2936L, 1e-2937L, 1e-2938L, 1e-2939L, 
   1e-2940L, 1e-2941L, 1e-2942L, 1e-2943L, 1e-2944L, 1e-2945L, 1e-2946L, 1e-2947L, 1e-2948L, 1e-2949L, 
   1e-2950L, 1e-2951L, 1e-2952L, 1e-2953L, 1e-2954L, 1e-2955L, 1e-2956L, 1e-2957L, 1e-2958L, 1e-2959L, 
   1e-2960L, 1e-2961L, 1e-2962L, 1e-2963L, 1e-2964L, 1e-2965L, 1e-2966L, 1e-2967L, 1e-2968L, 1e-2969L, 
   1e-2970L, 1e-2971L, 1e-2972L, 1e-2973L, 1e-2974L, 1e-2975L, 1e-2976L, 1e-2977L, 1e-2978L, 1e-2979L, 
   1e-2980L, 1e-2981L, 1e-2982L, 1e-2983L, 1e-2984L, 1e-2985L, 1e-2986L, 1e-2987L, 1e-2988L, 1e-2989L, 
   1e-2990L, 1e-2991L, 1e-2992L, 1e-2993L, 1e-2994L, 1e-2995L, 1e-2996L, 1e-2997L, 1e-2998L, 1e-2999L, 
   1e-3000L, 1e-3001L, 1e-3002L, 1e-3003L, 1e-3004L, 1e-3005L, 1e-3006L, 1e-3007L, 1e-3008L, 1e-3009L, 
   1e-3010L, 1e-3011L, 1e-3012L, 1e-3013L, 1e-3014L, 1e-3015L, 1e-3016L, 1e-3017L, 1e-3018L, 1e-3019L, 
   1e-3020L, 1e-3021L, 1e-3022L, 1e-3023L, 1e-3024L, 1e-3025L, 1e-3026L, 1e-3027L, 1e-3028L, 1e-3029L, 
   1e-3030L, 1e-3031L, 1e-3032L, 1e-3033L, 1e-3034L, 1e-3035L, 1e-3036L, 1e-3037L, 1e-3038L, 1e-3039L, 
   1e-3040L, 1e-3041L, 1e-3042L, 1e-3043L, 1e-3044L, 1e-3045L, 1e-3046L, 1e-3047L, 1e-3048L, 1e-3049L, 
   1e-3050L, 1e-3051L, 1e-3052L, 1e-3053L, 1e-3054L, 1e-3055L, 1e-3056L, 1e-3057L, 1e-3058L, 1e-3059L, 
   1e-3060L, 1e-3061L, 1e-3062L, 1e-3063L, 1e-3064L, 1e-3065L, 1e-3066L, 1e-3067L, 1e-3068L, 1e-3069L, 
   1e-3070L, 1e-3071L, 1e-3072L, 1e-3073L, 1e-3074L, 1e-3075L, 1e-3076L, 1e-3077L, 1e-3078L, 1e-3079L, 
   1e-3080L, 1e-3081L, 1e-3082L, 1e-3083L, 1e-3084L, 1e-3085L, 1e-3086L, 1e-3087L, 1e-3088L, 1e-3089L, 
   1e-3090L, 1e-3091L, 1e-3092L, 1e-3093L, 1e-3094L, 1e-3095L, 1e-3096L, 1e-3097L, 1e-3098L, 1e-3099L, 
   1e-3100L, 1e-3101L, 1e-3102L, 1e-3103L, 1e-3104L, 1e-3105L, 1e-3106L, 1e-3107L, 1e-3108L, 1e-3109L, 
   1e-3110L, 1e-3111L, 1e-3112L, 1e-3113L, 1e-3114L, 1e-3115L, 1e-3116L, 1e-3117L, 1e-3118L, 1e-3119L, 
   1e-3120L, 1e-3121L, 1e-3122L, 1e-3123L, 1e-3124L, 1e-3125L, 1e-3126L, 1e-3127L, 1e-3128L, 1e-3129L, 
   1e-3130L, 1e-3131L, 1e-3132L, 1e-3133L, 1e-3134L, 1e-3135L, 1e-3136L, 1e-3137L, 1e-3138L, 1e-3139L, 
   1e-3140L, 1e-3141L, 1e-3142L, 1e-3143L, 1e-3144L, 1e-3145L, 1e-3146L, 1e-3147L, 1e-3148L, 1e-3149L, 
   1e-3150L, 1e-3151L, 1e-3152L, 1e-3153L, 1e-3154L, 1e-3155L, 1e-3156L, 1e-3157L, 1e-3158L, 1e-3159L, 
   1e-3160L, 1e-3161L, 1e-3162L, 1e-3163L, 1e-3164L, 1e-3165L, 1e-3166L, 1e-3167L, 1e-3168L, 1e-3169L, 
   1e-3170L, 1e-3171L, 1e-3172L, 1e-3173L, 1e-3174L, 1e-3175L, 1e-3176L, 1e-3177L, 1e-3178L, 1e-3179L, 
   1e-3180L, 1e-3181L, 1e-3182L, 1e-3183L, 1e-3184L, 1e-3185L, 1e-3186L, 1e-3187L, 1e-3188L, 1e-3189L, 
   1e-3190L, 1e-3191L, 1e-3192L, 1e-3193L, 1e-3194L, 1e-3195L, 1e-3196L, 1e-3197L, 1e-3198L, 1e-3199L, 
   1e-3200L, 1e-3201L, 1e-3202L, 1e-3203L, 1e-3204L, 1e-3205L, 1e-3206L, 1e-3207L, 1e-3208L, 1e-3209L, 
   1e-3210L, 1e-3211L, 1e-3212L, 1e-3213L, 1e-3214L, 1e-3215L, 1e-3216L, 1e-3217L, 1e-3218L, 1e-3219L, 
   1e-3220L, 1e-3221L, 1e-3222L, 1e-3223L, 1e-3224L, 1e-3225L, 1e-3226L, 1e-3227L, 1e-3228L, 1e-3229L, 
   1e-3230L, 1e-3231L, 1e-3232L, 1e-3233L, 1e-3234L, 1e-3235L, 1e-3236L, 1e-3237L, 1e-3238L, 1e-3239L, 
   1e-3240L, 1e-3241L, 1e-3242L, 1e-3243L, 1e-3244L, 1e-3245L, 1e-3246L, 1e-3247L, 1e-3248L, 1e-3249L, 
   1e-3250L, 1e-3251L, 1e-3252L, 1e-3253L, 1e-3254L, 1e-3255L, 1e-3256L, 1e-3257L, 1e-3258L, 1e-3259L, 
   1e-3260L, 1e-3261L, 1e-3262L, 1e-3263L, 1e-3264L, 1e-3265L, 1e-3266L, 1e-3267L, 1e-3268L, 1e-3269L, 
   1e-3270L, 1e-3271L, 1e-3272L, 1e-3273L, 1e-3274L, 1e-3275L, 1e-3276L, 1e-3277L, 1e-3278L, 1e-3279L, 
   1e-3280L, 1e-3281L, 1e-3282L, 1e-3283L, 1e-3284L, 1e-3285L, 1e-3286L, 1e-3287L, 1e-3288L, 1e-3289L, 
   1e-3290L, 1e-3291L, 1e-3292L, 1e-3293L, 1e-3294L, 1e-3295L, 1e-3296L, 1e-3297L, 1e-3298L, 1e-3299L, 
   1e-3300L, 1e-3301L, 1e-3302L, 1e-3303L, 1e-3304L, 1e-3305L, 1e-3306L, 1e-3307L, 1e-3308L, 1e-3309L, 
   1e-3310L, 1e-3311L, 1e-3312L, 1e-3313L, 1e-3314L, 1e-3315L, 1e-3316L, 1e-3317L, 1e-3318L, 1e-3319L, 
   1e-3320L, 1e-3321L, 1e-3322L, 1e-3323L, 1e-3324L, 1e-3325L, 1e-3326L, 1e-3327L, 1e-3328L, 1e-3329L, 
   1e-3330L, 1e-3331L, 1e-3332L, 1e-3333L, 1e-3334L, 1e-3335L, 1e-3336L, 1e-3337L, 1e-3338L, 1e-3339L, 
   1e-3340L, 1e-3341L, 1e-3342L, 1e-3343L, 1e-3344L, 1e-3345L, 1e-3346L, 1e-3347L, 1e-3348L, 1e-3349L, 
   1e-3350L, 1e-3351L, 1e-3352L, 1e-3353L, 1e-3354L, 1e-3355L, 1e-3356L, 1e-3357L, 1e-3358L, 1e-3359L, 
   1e-3360L, 1e-3361L, 1e-3362L, 1e-3363L, 1e-3364L, 1e-3365L, 1e-3366L, 1e-3367L, 1e-3368L, 1e-3369L, 
   1e-3370L, 1e-3371L, 1e-3372L, 1e-3373L, 1e-3374L, 1e-3375L, 1e-3376L, 1e-3377L, 1e-3378L, 1e-3379L, 
   1e-3380L, 1e-3381L, 1e-3382L, 1e-3383L, 1e-3384L, 1e-3385L, 1e-3386L, 1e-3387L, 1e-3388L, 1e-3389L, 
   1e-3390L, 1e-3391L, 1e-3392L, 1e-3393L, 1e-3394L, 1e-3395L, 1e-3396L, 1e-3397L, 1e-3398L, 1e-3399L, 
   1e-3400L, 1e-3401L, 1e-3402L, 1e-3403L, 1e-3404L, 1e-3405L, 1e-3406L, 1e-3407L, 1e-3408L, 1e-3409L, 
   1e-3410L, 1e-3411L, 1e-3412L, 1e-3413L, 1e-3414L, 1e-3415L, 1e-3416L, 1e-3417L, 1e-3418L, 1e-3419L, 
   1e-3420L, 1e-3421L, 1e-3422L, 1e-3423L, 1e-3424L, 1e-3425L, 1e-3426L, 1e-3427L, 1e-3428L, 1e-3429L, 
   1e-3430L, 1e-3431L, 1e-3432L, 1e-3433L, 1e-3434L, 1e-3435L, 1e-3436L, 1e-3437L, 1e-3438L, 1e-3439L, 
   1e-3440L, 1e-3441L, 1e-3442L, 1e-3443L, 1e-3444L, 1e-3445L, 1e-3446L, 1e-3447L, 1e-3448L, 1e-3449L, 
   1e-3450L, 1e-3451L, 1e-3452L, 1e-3453L, 1e-3454L, 1e-3455L, 1e-3456L, 1e-3457L, 1e-3458L, 1e-3459L, 
   1e-3460L, 1e-3461L, 1e-3462L, 1e-3463L, 1e-3464L, 1e-3465L, 1e-3466L, 1e-3467L, 1e-3468L, 1e-3469L, 
   1e-3470L, 1e-3471L, 1e-3472L, 1e-3473L, 1e-3474L, 1e-3475L, 1e-3476L, 1e-3477L, 1e-3478L, 1e-3479L, 
   1e-3480L, 1e-3481L, 1e-3482L, 1e-3483L, 1e-3484L, 1e-3485L, 1e-3486L, 1e-3487L, 1e-3488L, 1e-3489L, 
   1e-3490L, 1e-3491L, 1e-3492L, 1e-3493L, 1e-3494L, 1e-3495L, 1e-3496L, 1e-3497L, 1e-3498L, 1e-3499L, 
   1e-3500L, 1e-3501L, 1e-3502L, 1e-3503L, 1e-3504L, 1e-3505L, 1e-3506L, 1e-3507L, 1e-3508L, 1e-3509L, 
   1e-3510L, 1e-3511L, 1e-3512L, 1e-3513L, 1e-3514L, 1e-3515L, 1e-3516L, 1e-3517L, 1e-3518L, 1e-3519L, 
   1e-3520L, 1e-3521L, 1e-3522L, 1e-3523L, 1e-3524L, 1e-3525L, 1e-3526L, 1e-3527L, 1e-3528L, 1e-3529L, 
   1e-3530L, 1e-3531L, 1e-3532L, 1e-3533L, 1e-3534L, 1e-3535L, 1e-3536L, 1e-3537L, 1e-3538L, 1e-3539L, 
   1e-3540L, 1e-3541L, 1e-3542L, 1e-3543L, 1e-3544L, 1e-3545L, 1e-3546L, 1e-3547L, 1e-3548L, 1e-3549L, 
   1e-3550L, 1e-3551L, 1e-3552L, 1e-3553L, 1e-3554L, 1e-3555L, 1e-3556L, 1e-3557L, 1e-3558L, 1e-3559L, 
   1e-3560L, 1e-3561L, 1e-3562L, 1e-3563L, 1e-3564L, 1e-3565L, 1e-3566L, 1e-3567L, 1e-3568L, 1e-3569L, 
   1e-3570L, 1e-3571L, 1e-3572L, 1e-3573L, 1e-3574L, 1e-3575L, 1e-3576L, 1e-3577L, 1e-3578L, 1e-3579L, 
   1e-3580L, 1e-3581L, 1e-3582L, 1e-3583L, 1e-3584L, 1e-3585L, 1e-3586L, 1e-3587L, 1e-3588L, 1e-3589L, 
   1e-3590L, 1e-3591L, 1e-3592L, 1e-3593L, 1e-3594L, 1e-3595L, 1e-3596L, 1e-3597L, 1e-3598L, 1e-3599L, 
   1e-3600L, 1e-3601L, 1e-3602L, 1e-3603L, 1e-3604L, 1e-3605L, 1e-3606L, 1e-3607L, 1e-3608L, 1e-3609L, 
   1e-3610L, 1e-3611L, 1e-3612L, 1e-3613L, 1e-3614L, 1e-3615L, 1e-3616L, 1e-3617L, 1e-3618L, 1e-3619L, 
   1e-3620L, 1e-3621L, 1e-3622L, 1e-3623L, 1e-3624L, 1e-3625L, 1e-3626L, 1e-3627L, 1e-3628L, 1e-3629L, 
   1e-3630L, 1e-3631L, 1e-3632L, 1e-3633L, 1e-3634L, 1e-3635L, 1e-3636L, 1e-3637L, 1e-3638L, 1e-3639L, 
   1e-3640L, 1e-3641L, 1e-3642L, 1e-3643L, 1e-3644L, 1e-3645L, 1e-3646L, 1e-3647L, 1e-3648L, 1e-3649L, 
   1e-3650L, 1e-3651L, 1e-3652L, 1e-3653L, 1e-3654L, 1e-3655L, 1e-3656L, 1e-3657L, 1e-3658L, 1e-3659L, 
   1e-3660L, 1e-3661L, 1e-3662L, 1e-3663L, 1e-3664L, 1e-3665L, 1e-3666L, 1e-3667L, 1e-3668L, 1e-3669L, 
   1e-3670L, 1e-3671L, 1e-3672L, 1e-3673L, 1e-3674L, 1e-3675L, 1e-3676L, 1e-3677L, 1e-3678L, 1e-3679L, 
   1e-3680L, 1e-3681L, 1e-3682L, 1e-3683L, 1e-3684L, 1e-3685L, 1e-3686L, 1e-3687L, 1e-3688L, 1e-3689L, 
   1e-3690L, 1e-3691L, 1e-3692L, 1e-3693L, 1e-3694L, 1e-3695L, 1e-3696L, 1e-3697L, 1e-3698L, 1e-3699L, 
   1e-3700L, 1e-3701L, 1e-3702L, 1e-3703L, 1e-3704L, 1e-3705L, 1e-3706L, 1e-3707L, 1e-3708L, 1e-3709L, 
   1e-3710L, 1e-3711L, 1e-3712L, 1e-3713L, 1e-3714L, 1e-3715L, 1e-3716L, 1e-3717L, 1e-3718L, 1e-3719L, 
   1e-3720L, 1e-3721L, 1e-3722L, 1e-3723L, 1e-3724L, 1e-3725L, 1e-3726L, 1e-3727L, 1e-3728L, 1e-3729L, 
   1e-3730L, 1e-3731L, 1e-3732L, 1e-3733L, 1e-3734L, 1e-3735L, 1e-3736L, 1e-3737L, 1e-3738L, 1e-3739L, 
   1e-3740L, 1e-3741L, 1e-3742L, 1e-3743L, 1e-3744L, 1e-3745L, 1e-3746L, 1e-3747L, 1e-3748L, 1e-3749L, 
   1e-3750L, 1e-3751L, 1e-3752L, 1e-3753L, 1e-3754L, 1e-3755L, 1e-3756L, 1e-3757L, 1e-3758L, 1e-3759L, 
   1e-3760L, 1e-3761L, 1e-3762L, 1e-3763L, 1e-3764L, 1e-3765L, 1e-3766L, 1e-3767L, 1e-3768L, 1e-3769L, 
   1e-3770L, 1e-3771L, 1e-3772L, 1e-3773L, 1e-3774L, 1e-3775L, 1e-3776L, 1e-3777L, 1e-3778L, 1e-3779L, 
   1e-3780L, 1e-3781L, 1e-3782L, 1e-3783L, 1e-3784L, 1e-3785L, 1e-3786L, 1e-3787L, 1e-3788L, 1e-3789L, 
   1e-3790L, 1e-3791L, 1e-3792L, 1e-3793L, 1e-3794L, 1e-3795L, 1e-3796L, 1e-3797L, 1e-3798L, 1e-3799L, 
   1e-3800L, 1e-3801L, 1e-3802L, 1e-3803L, 1e-3804L, 1e-3805L, 1e-3806L, 1e-3807L, 1e-3808L, 1e-3809L, 
   1e-3810L, 1e-3811L, 1e-3812L, 1e-3813L, 1e-3814L, 1e-3815L, 1e-3816L, 1e-3817L, 1e-3818L, 1e-3819L, 
   1e-3820L, 1e-3821L, 1e-3822L, 1e-3823L, 1e-3824L, 1e-3825L, 1e-3826L, 1e-3827L, 1e-3828L, 1e-3829L, 
   1e-3830L, 1e-3831L, 1e-3832L, 1e-3833L, 1e-3834L, 1e-3835L, 1e-3836L, 1e-3837L, 1e-3838L, 1e-3839L, 
   1e-3840L, 1e-3841L, 1e-3842L, 1e-3843L, 1e-3844L, 1e-3845L, 1e-3846L, 1e-3847L, 1e-3848L, 1e-3849L, 
   1e-3850L, 1e-3851L, 1e-3852L, 1e-3853L, 1e-3854L, 1e-3855L, 1e-3856L, 1e-3857L, 1e-3858L, 1e-3859L, 
   1e-3860L, 1e-3861L, 1e-3862L, 1e-3863L, 1e-3864L, 1e-3865L, 1e-3866L, 1e-3867L, 1e-3868L, 1e-3869L, 
   1e-3870L, 1e-3871L, 1e-3872L, 1e-3873L, 1e-3874L, 1e-3875L, 1e-3876L, 1e-3877L, 1e-3878L, 1e-3879L, 
   1e-3880L, 1e-3881L, 1e-3882L, 1e-3883L, 1e-3884L, 1e-3885L, 1e-3886L, 1e-3887L, 1e-3888L, 1e-3889L, 
   1e-3890L, 1e-3891L, 1e-3892L, 1e-3893L, 1e-3894L, 1e-3895L, 1e-3896L, 1e-3897L, 1e-3898L, 1e-3899L, 
   1e-3900L, 1e-3901L, 1e-3902L, 1e-3903L, 1e-3904L, 1e-3905L, 1e-3906L, 1e-3907L, 1e-3908L, 1e-3909L, 
   1e-3910L, 1e-3911L, 1e-3912L, 1e-3913L, 1e-3914L, 1e-3915L, 1e-3916L, 1e-3917L, 1e-3918L, 1e-3919L, 
   1e-3920L, 1e-3921L, 1e-3922L, 1e-3923L, 1e-3924L, 1e-3925L, 1e-3926L, 1e-3927L, 1e-3928L, 1e-3929L, 
   1e-3930L, 1e-3931L, 1e-3932L, 1e-3933L, 1e-3934L, 1e-3935L, 1e-3936L, 1e-3937L, 1e-3938L, 1e-3939L, 
   1e-3940L, 1e-3941L, 1e-3942L, 1e-3943L, 1e-3944L, 1e-3945L, 1e-3946L, 1e-3947L, 1e-3948L, 1e-3949L, 
   1e-3950L, 1e-3951L, 1e-3952L, 1e-3953L, 1e-3954L, 1e-3955L, 1e-3956L, 1e-3957L, 1e-3958L, 1e-3959L, 
   1e-3960L, 1e-3961L, 1e-3962L, 1e-3963L, 1e-3964L, 1e-3965L, 1e-3966L, 1e-3967L, 1e-3968L, 1e-3969L, 
   1e-3970L, 1e-3971L, 1e-3972L, 1e-3973L, 1e-3974L, 1e-3975L, 1e-3976L, 1e-3977L, 1e-3978L, 1e-3979L, 
   1e-3980L, 1e-3981L, 1e-3982L, 1e-3983L, 1e-3984L, 1e-3985L, 1e-3986L, 1e-3987L, 1e-3988L, 1e-3989L, 
   1e-3990L, 1e-3991L, 1e-3992L, 1e-3993L, 1e-3994L, 1e-3995L, 1e-3996L, 1e-3997L, 1e-3998L, 1e-3999L, 
   1e-4000L, 1e-4001L, 1e-4002L, 1e-4003L, 1e-4004L, 1e-4005L, 1e-4006L, 1e-4007L, 1e-4008L, 1e-4009L, 
   1e-4010L, 1e-4011L, 1e-4012L, 1e-4013L, 1e-4014L, 1e-4015L, 1e-4016L, 1e-4017L, 1e-4018L, 1e-4019L, 
   1e-4020L, 1e-4021L, 1e-4022L, 1e-4023L, 1e-4024L, 1e-4025L, 1e-4026L, 1e-4027L, 1e-4028L, 1e-4029L, 
   1e-4030L, 1e-4031L, 1e-4032L, 1e-4033L, 1e-4034L, 1e-4035L, 1e-4036L, 1e-4037L, 1e-4038L, 1e-4039L, 
   1e-4040L, 1e-4041L, 1e-4042L, 1e-4043L, 1e-4044L, 1e-4045L, 1e-4046L, 1e-4047L, 1e-4048L, 1e-4049L, 
   1e-4050L, 1e-4051L, 1e-4052L, 1e-4053L, 1e-4054L, 1e-4055L, 1e-4056L, 1e-4057L, 1e-4058L, 1e-4059L, 
   1e-4060L, 1e-4061L, 1e-4062L, 1e-4063L, 1e-4064L, 1e-4065L, 1e-4066L, 1e-4067L, 1e-4068L, 1e-4069L, 
   1e-4070L, 1e-4071L, 1e-4072L, 1e-4073L, 1e-4074L, 1e-4075L, 1e-4076L, 1e-4077L, 1e-4078L, 1e-4079L, 
   1e-4080L, 1e-4081L, 1e-4082L, 1e-4083L, 1e-4084L, 1e-4085L, 1e-4086L, 1e-4087L, 1e-4088L, 1e-4089L, 
   1e-4090L, 1e-4091L, 1e-4092L, 1e-4093L, 1e-4094L, 1e-4095L, 1e-4096L, 1e-4097L, 1e-4098L, 1e-4099L, 
   1e-4100L, 1e-4101L, 1e-4102L, 1e-4103L, 1e-4104L, 1e-4105L, 1e-4106L, 1e-4107L, 1e-4108L, 1e-4109L, 
   1e-4110L, 1e-4111L, 1e-4112L, 1e-4113L, 1e-4114L, 1e-4115L, 1e-4116L, 1e-4117L, 1e-4118L, 1e-4119L, 
   1e-4120L, 1e-4121L, 1e-4122L, 1e-4123L, 1e-4124L, 1e-4125L, 1e-4126L, 1e-4127L, 1e-4128L, 1e-4129L, 
   1e-4130L, 1e-4131L, 1e-4132L, 1e-4133L, 1e-4134L, 1e-4135L, 1e-4136L, 1e-4137L, 1e-4138L, 1e-4139L, 
   1e-4140L, 1e-4141L, 1e-4142L, 1e-4143L, 1e-4144L, 1e-4145L, 1e-4146L, 1e-4147L, 1e-4148L, 1e-4149L, 
   1e-4150L, 1e-4151L, 1e-4152L, 1e-4153L, 1e-4154L, 1e-4155L, 1e-4156L, 1e-4157L, 1e-4158L, 1e-4159L, 
   1e-4160L, 1e-4161L, 1e-4162L, 1e-4163L, 1e-4164L, 1e-4165L, 1e-4166L, 1e-4167L, 1e-4168L, 1e-4169L, 
   1e-4170L, 1e-4171L, 1e-4172L, 1e-4173L, 1e-4174L, 1e-4175L, 1e-4176L, 1e-4177L, 1e-4178L, 1e-4179L, 
   1e-4180L, 1e-4181L, 1e-4182L, 1e-4183L, 1e-4184L, 1e-4185L, 1e-4186L, 1e-4187L, 1e-4188L, 1e-4189L, 
   1e-4190L, 1e-4191L, 1e-4192L, 1e-4193L, 1e-4194L, 1e-4195L, 1e-4196L, 1e-4197L, 1e-4198L, 1e-4199L, 
   1e-4200L, 1e-4201L, 1e-4202L, 1e-4203L, 1e-4204L, 1e-4205L, 1e-4206L, 1e-4207L, 1e-4208L, 1e-4209L, 
   1e-4210L, 1e-4211L, 1e-4212L, 1e-4213L, 1e-4214L, 1e-4215L, 1e-4216L, 1e-4217L, 1e-4218L, 1e-4219L, 
   1e-4220L, 1e-4221L, 1e-4222L, 1e-4223L, 1e-4224L, 1e-4225L, 1e-4226L, 1e-4227L, 1e-4228L, 1e-4229L, 
   1e-4230L, 1e-4231L, 1e-4232L, 1e-4233L, 1e-4234L, 1e-4235L, 1e-4236L, 1e-4237L, 1e-4238L, 1e-4239L, 
   1e-4240L, 1e-4241L, 1e-4242L, 1e-4243L, 1e-4244L, 1e-4245L, 1e-4246L, 1e-4247L, 1e-4248L, 1e-4249L, 
   1e-4250L, 1e-4251L, 1e-4252L, 1e-4253L, 1e-4254L, 1e-4255L, 1e-4256L, 1e-4257L, 1e-4258L, 1e-4259L, 
   1e-4260L, 1e-4261L, 1e-4262L, 1e-4263L, 1e-4264L, 1e-4265L, 1e-4266L, 1e-4267L, 1e-4268L, 1e-4269L, 
   1e-4270L, 1e-4271L, 1e-4272L, 1e-4273L, 1e-4274L, 1e-4275L, 1e-4276L, 1e-4277L, 1e-4278L, 1e-4279L, 
   1e-4280L, 1e-4281L, 1e-4282L, 1e-4283L, 1e-4284L, 1e-4285L, 1e-4286L, 1e-4287L, 1e-4288L, 1e-4289L, 
   1e-4290L, 1e-4291L, 1e-4292L, 1e-4293L, 1e-4294L, 1e-4295L, 1e-4296L, 1e-4297L, 1e-4298L, 1e-4299L, 
   1e-4300L, 1e-4301L, 1e-4302L, 1e-4303L, 1e-4304L, 1e-4305L, 1e-4306L, 1e-4307L, 1e-4308L, 1e-4309L, 
   1e-4310L, 1e-4311L, 1e-4312L, 1e-4313L, 1e-4314L, 1e-4315L, 1e-4316L, 1e-4317L, 1e-4318L, 1e-4319L, 
   1e-4320L, 1e-4321L, 1e-4322L, 1e-4323L, 1e-4324L, 1e-4325L, 1e-4326L, 1e-4327L, 1e-4328L, 1e-4329L, 
   1e-4330L, 1e-4331L, 1e-4332L, 1e-4333L, 1e-4334L, 1e-4335L, 1e-4336L, 1e-4337L, 1e-4338L, 1e-4339L, 
   1e-4340L, 1e-4341L, 1e-4342L, 1e-4343L, 1e-4344L, 1e-4345L, 1e-4346L, 1e-4347L, 1e-4348L, 1e-4349L, 
   1e-4350L, 1e-4351L, 1e-4352L, 1e-4353L, 1e-4354L, 1e-4355L, 1e-4356L, 1e-4357L, 1e-4358L, 1e-4359L, 
   1e-4360L, 1e-4361L, 1e-4362L, 1e-4363L, 1e-4364L, 1e-4365L, 1e-4366L, 1e-4367L, 1e-4368L, 1e-4369L, 
   1e-4370L, 1e-4371L, 1e-4372L, 1e-4373L, 1e-4374L, 1e-4375L, 1e-4376L, 1e-4377L, 1e-4378L, 1e-4379L, 
   1e-4380L, 1e-4381L, 1e-4382L, 1e-4383L, 1e-4384L, 1e-4385L, 1e-4386L, 1e-4387L, 1e-4388L, 1e-4389L, 
   1e-4390L, 1e-4391L, 1e-4392L, 1e-4393L, 1e-4394L, 1e-4395L, 1e-4396L, 1e-4397L, 1e-4398L, 1e-4399L, 
   1e-4400L, 1e-4401L, 1e-4402L, 1e-4403L, 1e-4404L, 1e-4405L, 1e-4406L, 1e-4407L, 1e-4408L, 1e-4409L, 
   1e-4410L, 1e-4411L, 1e-4412L, 1e-4413L, 1e-4414L, 1e-4415L, 1e-4416L, 1e-4417L, 1e-4418L, 1e-4419L, 
   1e-4420L, 1e-4421L, 1e-4422L, 1e-4423L, 1e-4424L, 1e-4425L, 1e-4426L, 1e-4427L, 1e-4428L, 1e-4429L, 
   1e-4430L, 1e-4431L, 1e-4432L, 1e-4433L, 1e-4434L, 1e-4435L, 1e-4436L, 1e-4437L, 1e-4438L, 1e-4439L, 
   1e-4440L, 1e-4441L, 1e-4442L, 1e-4443L, 1e-4444L, 1e-4445L, 1e-4446L, 1e-4447L, 1e-4448L, 1e-4449L, 
   1e-4450L, 1e-4451L, 1e-4452L, 1e-4453L, 1e-4454L, 1e-4455L, 1e-4456L, 1e-4457L, 1e-4458L, 1e-4459L, 
   1e-4460L, 1e-4461L, 1e-4462L, 1e-4463L, 1e-4464L, 1e-4465L, 1e-4466L, 1e-4467L, 1e-4468L, 1e-4469L, 
   1e-4470L, 1e-4471L, 1e-4472L, 1e-4473L, 1e-4474L, 1e-4475L, 1e-4476L, 1e-4477L, 1e-4478L, 1e-4479L, 
   1e-4480L, 1e-4481L, 1e-4482L, 1e-4483L, 1e-4484L, 1e-4485L, 1e-4486L, 1e-4487L, 1e-4488L, 1e-4489L, 
   1e-4490L, 1e-4491L, 1e-4492L, 1e-4493L, 1e-4494L, 1e-4495L, 1e-4496L, 1e-4497L, 1e-4498L, 1e-4499L, 
   1e-4500L, 1e-4501L, 1e-4502L, 1e-4503L, 1e-4504L, 1e-4505L, 1e-4506L, 1e-4507L, 1e-4508L, 1e-4509L, 
   1e-4510L, 1e-4511L, 1e-4512L, 1e-4513L, 1e-4514L, 1e-4515L, 1e-4516L, 1e-4517L, 1e-4518L, 1e-4519L, 
   1e-4520L, 1e-4521L, 1e-4522L, 1e-4523L, 1e-4524L, 1e-4525L, 1e-4526L, 1e-4527L, 1e-4528L, 1e-4529L, 
   1e-4530L, 1e-4531L, 1e-4532L, 1e-4533L, 1e-4534L, 1e-4535L, 1e-4536L, 1e-4537L, 1e-4538L, 1e-4539L, 
   1e-4540L, 1e-4541L, 1e-4542L, 1e-4543L, 1e-4544L, 1e-4545L, 1e-4546L, 1e-4547L, 1e-4548L, 1e-4549L, 
   1e-4550L, 1e-4551L, 1e-4552L, 1e-4553L, 1e-4554L, 1e-4555L, 1e-4556L, 1e-4557L, 1e-4558L, 1e-4559L, 
   1e-4560L, 1e-4561L, 1e-4562L, 1e-4563L, 1e-4564L, 1e-4565L, 1e-4566L, 1e-4567L, 1e-4568L, 1e-4569L, 
   1e-4570L, 1e-4571L, 1e-4572L, 1e-4573L, 1e-4574L, 1e-4575L, 1e-4576L, 1e-4577L, 1e-4578L, 1e-4579L, 
   1e-4580L, 1e-4581L, 1e-4582L, 1e-4583L, 1e-4584L, 1e-4585L, 1e-4586L, 1e-4587L, 1e-4588L, 1e-4589L, 
   1e-4590L, 1e-4591L, 1e-4592L, 1e-4593L, 1e-4594L, 1e-4595L, 1e-4596L, 1e-4597L, 1e-4598L, 1e-4599L, 
   1e-4600L, 1e-4601L, 1e-4602L, 1e-4603L, 1e-4604L, 1e-4605L, 1e-4606L, 1e-4607L, 1e-4608L, 1e-4609L, 
   1e-4610L, 1e-4611L, 1e-4612L, 1e-4613L, 1e-4614L, 1e-4615L, 1e-4616L, 1e-4617L, 1e-4618L, 1e-4619L, 
   1e-4620L, 1e-4621L, 1e-4622L, 1e-4623L, 1e-4624L, 1e-4625L, 1e-4626L, 1e-4627L, 1e-4628L, 1e-4629L, 
   1e-4630L, 1e-4631L, 1e-4632L, 1e-4633L, 1e-4634L, 1e-4635L, 1e-4636L, 1e-4637L, 1e-4638L, 1e-4639L, 
   1e-4640L, 1e-4641L, 1e-4642L, 1e-4643L, 1e-4644L, 1e-4645L, 1e-4646L, 1e-4647L, 1e-4648L, 1e-4649L, 
   1e-4650L, 1e-4651L, 1e-4652L, 1e-4653L, 1e-4654L, 1e-4655L, 1e-4656L, 1e-4657L, 1e-4658L, 1e-4659L, 
   1e-4660L, 1e-4661L, 1e-4662L, 1e-4663L, 1e-4664L, 1e-4665L, 1e-4666L, 1e-4667L, 1e-4668L, 1e-4669L, 
   1e-4670L, 1e-4671L, 1e-4672L, 1e-4673L, 1e-4674L, 1e-4675L, 1e-4676L, 1e-4677L, 1e-4678L, 1e-4679L, 
   1e-4680L, 1e-4681L, 1e-4682L, 1e-4683L, 1e-4684L, 1e-4685L, 1e-4686L, 1e-4687L, 1e-4688L, 1e-4689L, 
   1e-4690L, 1e-4691L, 1e-4692L, 1e-4693L, 1e-4694L, 1e-4695L, 1e-4696L, 1e-4697L, 1e-4698L, 1e-4699L, 
   1e-4700L, 1e-4701L, 1e-4702L, 1e-4703L, 1e-4704L, 1e-4705L, 1e-4706L, 1e-4707L, 1e-4708L, 1e-4709L, 
   1e-4710L, 1e-4711L, 1e-4712L, 1e-4713L, 1e-4714L, 1e-4715L, 1e-4716L, 1e-4717L, 1e-4718L, 1e-4719L, 
   1e-4720L, 1e-4721L, 1e-4722L, 1e-4723L, 1e-4724L, 1e-4725L, 1e-4726L, 1e-4727L, 1e-4728L, 1e-4729L, 
   1e-4730L, 1e-4731L, 1e-4732L, 1e-4733L, 1e-4734L, 1e-4735L, 1e-4736L, 1e-4737L, 1e-4738L, 1e-4739L, 
   1e-4740L, 1e-4741L, 1e-4742L, 1e-4743L, 1e-4744L, 1e-4745L, 1e-4746L, 1e-4747L, 1e-4748L, 1e-4749L, 
   1e-4750L, 1e-4751L, 1e-4752L, 1e-4753L, 1e-4754L, 1e-4755L, 1e-4756L, 1e-4757L, 1e-4758L, 1e-4759L, 
   1e-4760L, 1e-4761L, 1e-4762L, 1e-4763L, 1e-4764L, 1e-4765L, 1e-4766L, 1e-4767L, 1e-4768L, 1e-4769L, 
   1e-4770L, 1e-4771L, 1e-4772L, 1e-4773L, 1e-4774L, 1e-4775L, 1e-4776L, 1e-4777L, 1e-4778L, 1e-4779L, 
   1e-4780L, 1e-4781L, 1e-4782L, 1e-4783L, 1e-4784L, 1e-4785L, 1e-4786L, 1e-4787L, 1e-4788L, 1e-4789L, 
   1e-4790L, 1e-4791L, 1e-4792L, 1e-4793L, 1e-4794L, 1e-4795L, 1e-4796L, 1e-4797L, 1e-4798L, 1e-4799L, 
   1e-4800L, 1e-4801L, 1e-4802L, 1e-4803L, 1e-4804L, 1e-4805L, 1e-4806L, 1e-4807L, 1e-4808L, 1e-4809L, 
   1e-4810L, 1e-4811L, 1e-4812L, 1e-4813L, 1e-4814L, 1e-4815L, 1e-4816L, 1e-4817L, 1e-4818L, 1e-4819L, 
   1e-4820L, 1e-4821L, 1e-4822L, 1e-4823L, 1e-4824L, 1e-4825L, 1e-4826L, 1e-4827L, 1e-4828L, 1e-4829L, 
   1e-4830L, 1e-4831L, 1e-4832L, 1e-4833L, 1e-4834L, 1e-4835L, 1e-4836L, 1e-4837L, 1e-4838L, 1e-4839L, 
   1e-4840L, 1e-4841L, 1e-4842L, 1e-4843L, 1e-4844L, 1e-4845L, 1e-4846L, 1e-4847L, 1e-4848L, 1e-4849L, 
   1e-4850L, 1e-4851L, 1e-4852L, 1e-4853L, 1e-4854L, 1e-4855L, 1e-4856L, 1e-4857L, 1e-4858L, 1e-4859L, 
   1e-4860L, 1e-4861L, 1e-4862L, 1e-4863L, 1e-4864L, 1e-4865L, 1e-4866L, 1e-4867L, 1e-4868L, 1e-4869L, 
   1e-4870L, 1e-4871L, 1e-4872L, 1e-4873L, 1e-4874L, 1e-4875L, 1e-4876L, 1e-4877L, 1e-4878L, 1e-4879L, 
   1e-4880L, 1e-4881L, 1e-4882L, 1e-4883L, 1e-4884L, 1e-4885L, 1e-4886L, 1e-4887L, 1e-4888L, 1e-4889L, 
   1e-4890L, 1e-4891L, 1e-4892L, 1e-4893L, 1e-4894L, 1e-4895L, 1e-4896L, 1e-4897L, 1e-4898L, 1e-4899L, 
   1e-4900L, 1e-4901L, 1e-4902L, 1e-4903L, 1e-4904L, 1e-4905L, 1e-4906L, 1e-4907L, 1e-4908L, 1e-4909L, 
   1e-4910L, 1e-4911L, 1e-4912L, 1e-4913L, 1e-4914L, 1e-4915L, 1e-4916L, 1e-4917L, 1e-4918L, 1e-4919L, 
   1e-4920L, 1e-4921L, 1e-4922L, 1e-4923L, 1e-4924L, 1e-4925L, 1e-4926L, 1e-4927L, 1e-4928L, 1e-4929L, 
   1e-4930L, 1e-4931L, 1e-4932L, 1e-4933L, 1e-4934L, 1e-4935L, 1e-4936L, 1e-4937L, 1e-4938L, 1e-4939L, 
   1e-4940L, 1e-4941L, 1e-4942L, 1e-4943L, 1e-4944L, 1e-4945L, 1e-4946L, 1e-4947L, 1e-4948L, 1e-4949L, 
   1e-4950L
  };                

static uint64_t const ya_s__powten[20] =
				{
					UINT64_C(1), 	// 10^ 0
					UINT64_C(10), 	// 10^1
					UINT64_C(100), 	// 10^2
					UINT64_C(1000),	// 10^3
					UINT64_C(10000),// 10^4
					UINT64_C(100000),// 5
					UINT64_C(1000000),// 6
					UINT64_C(10000000),// 7
					UINT64_C(100000000),// 8
					UINT64_C(1000000000),// 9   
					UINT64_C(10000000000),// 10 
					UINT64_C(100000000000),// 11  
					UINT64_C(1000000000000),// 12   
					UINT64_C(10000000000000),// 13
					UINT64_C(100000000000000),// 14
					UINT64_C(1000000000000000), // 15
					UINT64_C(10000000000000000), // 16 
					UINT64_C(100000000000000000), // 17 
					UINT64_C(1000000000000000000), // 18 
					UINT64_C(10000000000000000000)  // 19
				};


#define ya_s__tento18th UINT64_C(1000000000000000000) /* note this is 10^18 the table entry before the end ! */



static long double ya_s__raise_to_power10(  long double x, int32_t power )  // power can be +/- 9800 (in fact a bit more than that !)
 {
  if(power<0) 	
  	{power = -power;
  	 if(power>4900)
  	 	{// do in 2 steps to avoid overflowing table, split at 4900 is a "round figure" well clear of area where we go into denormalised number which are less accurate
  	 	 x*= ldblnegpowersOf10[4900];
		 power -=4900;	 
		}
	  x*= ldblnegpowersOf10[power]; // * is faster than / and gives very slightly better results in test program
  	}
  else
  	{if(power>4900)
  	 	{// do in 2 steps to avoid overflowing table
  	 	 x*= ldblpowersOf10[4900];
		 power -=4900;	 
		}
	  x*=ldblpowersOf10[power];
  	}
  return x;
 }
 
#else /* LD not defined */

static uint64_t const ya_s__powten[20] =
				{
					UINT64_C(1), 	// 10^ 0
					UINT64_C(10), 	// 10^1
					UINT64_C(100), 	// 10^2
					UINT64_C(1000),	// 10^3
					UINT64_C(10000),// 10^4
					UINT64_C(100000),// 5
					UINT64_C(1000000),// 6
					UINT64_C(10000000),// 7
					UINT64_C(100000000),// 8
					UINT64_C(1000000000),// 9   
					UINT64_C(10000000000),// 10 
					UINT64_C(100000000000),// 11  
					UINT64_C(1000000000000),// 12   
					UINT64_C(10000000000000),// 13
					UINT64_C(100000000000000),// 14
					UINT64_C(1000000000000000), // 15
					UINT64_C(10000000000000000), // 16 
					UINT64_C(100000000000000000), // 17 
					UINT64_C(1000000000000000000), // 18 
					UINT64_C(10000000000000000000)  // 19
				};


#define ya_s__tento18th UINT64_C(1000000000000000000) /* note this is 10^18 the table entry before the end ! */


static void ya_s__raise_to_power10( double *ohi, double *olo, double d, int32_t power )  // power can be -323 to +350
{ 
  double th,tl; // power10
  if(power<0) 	
  	{
	 th=NegPowerOf10_hi[-power];
	 tl=NegPowerOf10_lo[-power];
  	}
  else if (power>300) // need to do in 2 multiplies rather than 1 as the max exponent for a double is 308 and we may need to multiply by 10^350 here
  	{double ph, pl;// partial result
	 th=PosPowerOf10_hi[300];
	 tl=PosPowerOf10_lo[300];   
	 // void mult_d_dd( double *xh, double *xl,double a,double bh, double bl);  // multiplies a and double double  b to give double double "x"
     mult_d_dd(&ph,&pl,d,th,tl);// th,l*d
	 th=PosPowerOf10_hi[power-300];
	 tl=PosPowerOf10_lo[power-300];     	 
	 // void mult_dd_dd( double *xh, double *xl,double ah, double al,double bh, double bl);  // multiplies double double a and b to give double double "x"
     mult_dd_dd(ohi,olo,th,tl,ph,pl);
     return;
  	}
  else
    {
	 th=PosPowerOf10_hi[power];
	 tl=PosPowerOf10_lo[power];	
	}
  // now need to compute ohi/lo=d*th,l only using doubles 
  // void mult_d_dd( double *xh, double *xl,double a,double bh, double bl);  // multiplies a and double double  b to give double double "x"
  mult_d_dd(ohi,olo,d,th,tl);
}
#endif // LD

// given a float value, returns the significant bits in bits, and the position of the
//   decimal point in decimal_pos.  +/-INF and NAN are specified by special values
//   returned in the decimal_pos parameter.
// frac_digits is absolute normally, but if you want from first significant digits (got %g and %e), or in 0x80000000
#ifdef YA_SP_SPRINTF_LD  /* pass value as long double */
static int32_t ya_s__real_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, long double value, uint32_t frac_digits)
{
   long double d;
   uint64_t bits = 0; // was int64
   int32_t e, ng, tens;

	int expo;
   d = value;
   ng=signbit(d);
   if(ng)
   	  d= -d;
   if(isnan(value))	 
  		{*start="nan"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
#ifdef YA_SP_SIGNED_NANS
         return ng;
#else     	 
     	 return 0;// nan is always positive
#endif     	 
		}
	else if(isinf(value))
		{*start="inf"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
     	 return ng;
		}
  	else if(d==0.0)
  		{
         *decimal_pos = 1;
         *start = out;
         out[0] = '0';
         *len = 1;
         return ng;
      }
    // d is a normal number, now get exponent
	frexpl(d,&expo);
	expo--; // adjust to range code below expects

// find the decimal exponent as well as the decimal bits of the value
	{
      // log10 estimate 
      tens = expo; 
	  tens = (tens < 0) ? ((tens * 1233) / 4096) : (((tens * 1233) / 4096) + 1); // this seems to give correct exponent
	  tens+=1;// this makes no difference to result but makes code slightly faster on average by minimising the number of loops required below
	  for(int i=0;i<10;++i)
	  	{// put in a for loop so cannot get stuck forever (but we always break out of loop rather than letting it terminate)
	  	 long double l10=rintl(ya_s__raise_to_power10( d, 18-tens ));
	  	 if(i>3 && l10>=(long double)ya_s__powten[18])
		   	{ bits=l10;
		      break;// not ideal but OK to stop in case we are bouncing +/-1
			}	  	 	
	  	 if(l10<(long double)ya_s__powten[18]) tens--; // too small need to increase
	  	 else if(l10>(long double)ya_s__powten[19]) tens++; // too big need to decrease
	  	 else 
		   	{ bits=l10;
		      break;// in range - done
			}
	  	}
      if ( ((uint64_t)bits) >= ya_s__tento18th ) // note ya_s__tento18th = 10^18, an unsigned can hold 1.8e19 so we can be off by a power of 10 without overflowing mantissa 
	  	++tens; // check if we undershot
    }	  	      
#else // not LD
static int32_t ya_s__real_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, double value, uint32_t frac_digits)
{
   double d;
   uint64_t bits = 0; 
   int32_t e, ng, tens;
   int expo;
   d = value;
   ng=signbit(d);
   if(ng)
   	  d= -d;
   if(isnan(value))	 
  		{*start="nan"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
#ifdef YA_SP_SIGNED_NANS
         return ng;
#else     	 
     	 return 0;// nan is always positive
#endif     	 
		}
	else if(isinf(value))
		{*start="inf"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
     	 return ng;
		}
  	else if(d==0.0)
  		{
         *decimal_pos = 1;
         *start = out;
         out[0] = '0';
         *len = 1;
         return ng;
      }
    // d is a normal number, now get exponent
	frexp(d,&expo);
	expo--; // adjust to range code below expects

   // find the decimal exponent as well as the decimal bits of the value
   {
      // log10 estimate - very specifically tweaked to hit or undershoot by no more than 1 of log10 of all expos 1..2046
      tens = expo;
	  double ph, pl;
	  tens = (tens < 0) ? ((tens * 617) / 2048) : (((tens * 1233) / 4096) + 1);
      // move the significant bits into position and stick them into an int
      ya_s__raise_to_power10(&ph, &pl, d, 18 - tens);

      // get full as much precision from double-double as possible
	  // uint64_t ddtoU64(double xh,double xl)  // convert double double to uint64
	  bits=ddtoU64(ph,pl);    

      // check if we undershot
      if (((uint64_t)bits) >= ya_s__tento18th)
         ++tens;     
   }  	           
#endif // LD
   // now do the rounding in integer land
   frac_digits = (frac_digits & 0x80000000) ? ((frac_digits & 0x7ffffff) + 1) : (tens + frac_digits);
   if ((frac_digits < 24)) {
      uint32_t dg = 1;
      if ((uint64_t)bits >= ya_s__powten[9])
         dg = 10;
      while ((uint64_t)bits >= ya_s__powten[dg]) {
         ++dg;
         if (dg == 20)
            goto noround;
      }
      if (frac_digits < dg) {
         uint64_t r;
         // add 0.5 at the right position and round
         e = dg - frac_digits;
         if ((uint32_t)e >= 20 || e<0) // was 24 but ya_s_powten[20] so should be >=20, also added check for e<0 as e is signed [ e < 0 should be impossible with current code, but lets be sure ]
            goto noround;
         r = ya_s__powten[e];
        /* round to even, as used in IEEE std 754 */
		if((bits%r)==r/2) // == 0.5
			{if((bits/r)&1) bits += (r/2);  // round to even
			}
		else
			{ bits += (r/2); 
			}       
         if ((uint64_t)bits >= ya_s__powten[dg])
            ++tens;
         bits /= r;
      }
   noround:;
   }

   // kill long trailing runs of zeros
   if (bits) {
      uint32_t n;
      for (;;) {
         if (bits <= 0xffffffff)
            break;
         if (bits % 1000)
            goto donez;
         bits /= 1000;
      }
      n = (uint32_t)bits;
      while ((n % 1000) == 0)
         n /= 1000;
      bits = n;
   donez:;
   }

   // convert to string
   out += 64;
   e = 0;
   for (;;) {
      uint32_t n;
      char *o = out - 8;
      // do the conversion in chunks of U32s (avoid most 64-bit divides, worth it, constant denomiators be damned)
      if (bits >= 100000000) {
         n = (uint32_t)(bits % 100000000);
         bits /= 100000000;
      } else {
         n = (uint32_t)bits;
         bits = 0;
      }
      

#ifdef YA_SP_NO_DIGITPAIR
	  while (n) {// use div 10 as thats fast and avoids potential cache miss of table lookup
	     *--out=(char)(n%10)+'0';
	     n/=10;
	     ++e; 	 
		}
      if (bits == 0) {
         break;
      }    
#else      
	  // original code - uses table lookup to convert 2 digits at a time, but may then need to backup 1 digit if we created too many.  
	  while (n) {
         out -= 2; 
         *(uint16_t *)out = *(uint16_t *)&ya_s__digitpair.pair[(n % 100) * 2];
         n /= 100;
         e += 2;
      }
      if (bits == 0) {
         if ((e) && (out[0] == '0')) {
            ++out;
            --e;
         }
         break;
      }         
#endif         


      while (out != o) {
         *--out = '0';
         ++e;
      }
   }

   *decimal_pos = tens;
   *start = out;
   *len = e;
   return ng;
}

#undef YA_S__SPECIAL

#endif // YA_SP_SPRINTF_NOFLOAT

// clean up
#undef uint16_t
#undef uint32_t
#undef int32_t
#undef uint64_t
#undef int64_t

/* now restore gcc options to those set by the user */
#if YA_SP_GCC_OPTIMIZE_AWARE
#pragma GCC pop_options
#endif

#endif // YA_SP_SPRINTF_IMPLEMENTATION

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Peter Miller
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
