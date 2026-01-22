/*	ya_sprintf.h
	============
Replacement for the "printf" family of functions (printf, fprintf,sprintf etc).
Version 1.0 30/7/2020 by Peter Miller.
Version 2.0
 			8/5/2024 	 - added more #define options to tweak output formats
			10/10/2025  - added more #define options to tweak output formats (aims to match UCRT)
						- adds round loop accurate long double and f128 output (previously only %a/A was round loop accurate
						- adds %w32/64/128 from C23.
						- adds wide character functions (%C,%lC,%S,%ls)
			6-1-2026	- added option to use RYU double to string algorithm - see https://dl.acm.org/citation.cfm?doid=3366395.3360595 
						 - note that the core RYU code (in the ryu/ directory) is copyrighted by Ulf Adams and contributors,
						   and may be used freely in accordance with the Apache 2.0 license or the Boost 1.0 license.
							Using RYU adds ~ 100kBytes to the executable size (mainly in tables).
							RYU is ~ 1.5* faster on average than the standard ya_sprintf code to printf doubles (see main.c for for details on timing)
										
This started with the code from  stb_sprintf (v1.08) which is a public domain snprintf() implementation (http://github.com/nothings/stb)
which itself was originally written by Jeff Roberts / RAD Game Tools, 2015/10/20.
This version is dual licensed (MIT and Public Domain) [ except for "ryu" see above and some other external functions, but everything is available with similarly permissive licences ] - see the end of this file for details.

"ya_sprintf" stands for Yet Another sprintf.

It provides an almost full C99/C23 printf family implementation including wide characters/strings.
ya_sprintf also provides a number of extensions, the most significant is the ability to print 128 bit integers (__int128)
and 128 bit floating point numbers (__float128) for compilers which support these types (most modern compilers do, at least for 64 bit targets).
Its thread safe (with no need for any locking), does no dynamic memory allocation and is much faster than the built in "printf's" in MinGW/TDM-GCC.
It also fixes all the bugs I'm aware of in the MinGW/TDM-GCC implementations, these bugs and the slow speed are the main reason this exists. 
In comparison to stb_sprintf (v1.08) it fixes all the bugs I found, implements all of the C99 formats and adds long doubles, 128 bit ints and 128 bit floats and most C23 features.
Finally, it provides a consistent format string specification across different targets which aids portability between targets and compilers.

It assumes a compiler that supports at least C99, it assumes chars are 8 bit ascii, int's are 32 bits, long can be 32/64 bits and long long int is 64 bits. Pointers can be 32 or 64 bits.
Floating point numbers are assumed to be  IEEE 754 format, with double as 64 bits, long double as 80 bits (which may be stored in up to 128 bits depending on the api in use)
 and __float128's being 128 bits all of which are used. Integers are assumed to be stored in 2's complement representation.
These assumptions are true for almost all processors manufactured in the last 10+ years (Intel X32 & X64, ARM, PowerPC, etc).

Basic use:
  #define YA_SP_SPRINTF_IMPLEMENTATION
  #define YA_SP_SPRINTF_DEFAULT  // with this defined printf etc will call the functions in this file 
  #include "ya_sprintf.h"
   ..
  printf("Hello world\n 1+2=%d\n",1+2); // use the library 


You also need to compile it with double-double.c which contains a library of maths functions that use two floating point variables to gain extra precision 
(effectively twice the mantissa resolution of a single variable).
double-double.c supports 3 sets of functions using doubles, long doubles and __float128's
double-double.c can also be used standalone where extra precision is required in calculations.
You also need to compile it with u2_64.c (which provides 128 bits maths when the compiler does not directly support them), and nan_type.c which allows flexibility on how NAN's ("Not A Number") are displayed.

Also available (and required) are the following which are available at https://github.com/p-j-miller 
atof.c - a MIT licensed implementation of strtod() for floats, doubles, long doubles and __float128's . This is used in the test program, but can be used standalone
main.c	- a test program that checks all functions of ya_sprintf.h (supplied with ya_sprintf)
my_printf.c - a version of printf family of functions that tries to work around known bugs in the system printf - it also does a  reasonable check of the format specifier string.

To compile the test program under Linux try (tested on Ubuntu 24.04.3 LTS (GNU/Linux 6.6.87.2-microsoft-standard-WSL2 x86_64) ):
 gcc -m64 -Wall -Ofast -fexcess-precision=standard -I. -D_FORTIFY_SOURCE=1 main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c -lquadmath -lm -o test
 
 then ./test to run
 
 Under Windows with WinLibs gcc 15.2.0 (please check the paths to the compiler and the other files are correct for your setup/directory structure): 
  C:\winlibs\winlibs-x86_64-posix-seh-gcc-15.2.0-mingw-w64ucrt-13.0.0-r2\mingw64\bin\gcc -Wall -m64 -fexcess-precision=standard -Ofast  -std=gnu99 -I. main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lquadmath -static -o test.exe
   
  then test.exe to run
  
For more information on compiling see the test program "main.c".
   
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
	 The precision when printing floating point numbers is clamped at 350 (this allows the maximum buffer size to be fixed).
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
	For a following c conversion specifier applies to a wide character (wint_t) argument.
	For a following s conversion specifier applies to a pointer to a wide character (*wchar_t) argument.
 ll a following d, i, o, u, x, X, b or B conversion specifier applies to a long long or unsigned long long argument;
	or that a following n conversion specifier applies to a pointer to a long long argument.
 j 	a following d, i, o, u, x, X, b or B conversion specifier applies to an intmax_t or uintmax_t argument;
	or that a following n conversion specifier applies to a pointer to an intmax_t argument.
 z  a following d, i, o, u, x, X, b or B conversion specifier applies to a size_t or the corresponding signed integer type argument;
	or that a following n conversion specifier applies to a pointer to a signed integer type corresponding to a size_t argument.
 t  a following d, i, o, u, x, X, b or B conversion specifier applies to a ptrdiff_t or the corresponding unsigned type argument;
	or that a following n conversion specifier applies to a pointer to a ptrdiff_t argument.
 L  a following a, A, e, E, f, F, g, or G conversion specifier applies to a long double argument.
 wn  C23 added "wn" as a width modifier - w32,w64 and w128 are supported by ya_sprintf and behave identically to I32,I64 and I128 (see below)
 
 The following are not defined by C99 or POSIX.1-2017. quadmath_snprintf() does support %Q but that's all it supports and then it only prints 1 argument at a time. 
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
	Note the b and B conversion specifiers are not defined by C99 or or POSIX.1-2017, they are defined by C23.
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
 c  The int argument is converted to an unsigned char, and the resulting byte is written. With an "l" modifier a wide character is expected and written.
 C  is treated identically to %lc i.e. a wide character argument is expected. This is a Microsoft extension
 s  The argument is a pointer to an array of char. Bytes from the array is written up to (but not including) any terminating null byte.
    If the precision is specified, no more than that many bytes is written.
	If the precision is not specified or is greater than the size of the array, the application must ensure that the array contains a null byte.
	With the "l" modifier it expects a wide character array argument (*wchar_t) and writes this. In this case, if YA_SP_WCHAR_PR_CHARS is defined the precision for wide strings (%ls or %S) 
	is in characters rather than bytes - C standard says bytes (which matches Linux), but Microsoft runtimes use characters.
 S	is treated identically to %ls i.e. a *wchar_t argument is expected. This is a Microsoft extension
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

For the e, E, f, F, g, and G conversion specifiers, if the number of significant decimal digits is >= DBL_DECIMAL_DIG (for doubles, or LDBL_DECIMAL_DIG or FLT128_DECIMAL_DIG for long doubles and float128's respectively) then the result wil be "round loop exact".
This means that the value output from ya_sprintf if fed to fast_strtod()/fast_strtold()/fast_strtof128() will give a bitwise identical value to the original double/ long double/_float128.
For significant digits <= FLT128_DIG,LDBL_DIG or DBL_DIG for _float128, long double and double respectively then ya_sprintf will normally print exactly the same characters as the "system" sprintf - however this is not guaranteed, 
but this and the round the loop test above is the basis of the supplied test program (the test program cannot test every possible value in a sane amount of time hence the lack of a guarantee).

%a/%A will always print an exact representation of float, double, long double and __float128.

For Windows, the wide character conversion specifiers (%C/%lc/%S/%ls) have only been tested on utf-8 strings when using the UCRT (the msvcrt does not support utf-8).

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
Note YA_SP_SPRINTF_LD  is not used in this version - long doubles are always supported.
#define YA_SP_SPRINTF_STATIC // make the definitions of the exported functions static
#define YA_SP_SPRINTF_MIN XXX // XXX is the number of characters per callback , default 512 (see ya_s_vsprintfcb() above)
#define STB_SPRINTF_IMPLEMENTATION // for backwards compatibility with stb_sprintf()
#define YA_SP_SPRINTF_DECORATE PREFIX // define the names of the exported functions as PREFIXname , default ya_s_. If this is not defined then vsprintf, vsnprintf, sprintf, snprintf, vfprintf, vprintf, fprintf & printf are defined via macros to equal the ya_s_ versions.
#define YA_SP_SPRINTF_IMPLEMENTATION // actually include code from header file (see "use" at the start of this file for examples)
#define YA_SP_SPRINTF_CHECK_FMT // if defined requires gcc to check the supplied format string - note this may give errors for valid formats as ya_sprintf provides more than standard functionality, but using it can find errors that are otherwise easy to miss... 
#define YA_SP_SPRINTF_NOFLOAT // no floating point support (saves space if you don't need it)
#define YA_SP_SPRINTF_QI // support int128 output
#define YA_SP_SPRINTF_QF // support float128 output
#define YA_SP_SPRINTF_Q // support both int128 and float128 output ( compiler must support these types! ) [ for backwards compatibility ]
#define YA_SP_NO_DIGITPAIR // selects an alternative way to convert numbers to ascii characters. This may or may not be faster. Its likely this option will be removed in future releases.
#define YA_SP_LINUX_STYLE // make subtle changes to the output to match gcc 13.3.0 under Ubuntu . By default matches winlibs GCC 15.2.0 under windows 10 with #define __USE_MINGW_ANSI_STDIO 1
#define YA_SP_NO_NEG_LEADINGPLUS // if defined ignore %+ for unsigned conversions
#define YA_SP_NO_NEG_LEADINGSPACE // if defined ignore %  (% space) for unsigned conversions
#define YA_SP_SIGNED_NANS // if defined print NAN's as signed numbers. Default is that a NAN is considered unsigned.
#define YA_SP_SIGNED_NANS_F128 // if defined float128 NAN's (only) are signed
#define YA_SP_SIGNED_NANS_LD // if defined long double NAN's (only) are signed
#define YA_SP_NAN_IND // if defined for doubles only : print Quiet NaN as"nan", Signalling NaN as "nan(snan)" and Indefinite NaN as "nan(ind)" [as UCRT] “IND” for “indeterminate” - needs #include "nan_type.h" before including ya_sprintf
#define YA_SP_FULL_NULL // if defined only print (null) if it will be fully visible
#define YA_SP_A_FMT_ALT1 // if defined print %A in an alternative way (round to even)
#define YA_SP_A_FMT_ALT2 // if defined print %A in another alternative way (simple rounding)
#define YA_SP_A_FMT_ALT3 // if defined print %A without trailing zero suppression
#define YA_SP_A_FMT_ALT4 // ALT1 and ALT3 for doubles, ALT2 for LD
	//note only 1 of ALT1,2,3,4 should be specified. 
#define YA_SP_PTR_0X // if defined print pointers with a leading 0X
#define YA_SP_PTR_CAPS // if defined print pointers with capital hex chars (A-F) rather than lowercase
#define YA_SP_PTR_LEADINGZEROS // if defined always print pointers with leading zeros
#define YA_SP_SPRINTF_EXP3 // min 3 digits in exponent (otherwise min of 2 digits in exponent) 
#define YA_SP_WCHAR_PR_CHARS // if defined the precision for wide strings (%ls or %S) is in characters rather than bytes - C standard says bytes (which matches Linux), but Microsoft runtimes use characters
#define YA_SP_RYU // if defined use RYU algorithm for doubles which is fast and accurate for IEEE format doubles - otherwise use more portable (but slower) algorithm [the same as used for long doubles and f128's] - note ryu adds about 100kb of tables to the executable.
*/

#ifdef YA_SP_LINUX_STYLE /* LINUX STYLE just sets other defines (left for backwards compatibility) - set to match Ubuntu 24.04.3 LTS (GNU/Linux 6.6.87.2-microsoft-standard-WSL2 x86_64) with gcc 13.3.0 */
 #define YA_SP_NO_NEG_LEADINGPLUS // if defined ignore %+ for unsigned conversions
 #define YA_SP_NO_NEG_LEADINGSPACE // if defined ignore %  (% space) for unsigned conversions
 #define YA_SP_FULL_NULL // if defined only print (null) if it will be fully visible
 #define YA_SP_A_FMT_ALT1 // if defined print %A in an alternative way
 #define YA_SP_PTR_0X // if defined print pointers with a leading 0X
 #define YA_SP_SIGNED_NANS /* tell ya_sprintf we want signed NAN's (to match gcc libc) */
 #define YA_SP_SIGNED_NANS_LD /* we also want signed NAN's for Long doubles */
#endif

#ifdef YA_SP_SPRINTF_Q
 #define YA_SP_SPRINTF_QI /* 128 bit integers */
 #define YA_SP_SPRINTF_QF /* 128 bit floats */
#endif


#include "../u2_64-128bits-with-two-u64/u2_64.h" // needed to get 128 bit int functions 


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
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#if defined(__SIZEOF_FLOAT128__) && defined(YA_SP_SPRINTF_QF ) && !defined(__BORLANDC__) /* if compiler supports __float128  and support for this is requested YA_SP_SPRINTF_QF */
 #include <quadmath.h> /* see https://gcc.gnu.org/onlinedocs/libquadmath/index.html#SEC_Contents - also needs quadmath library linking in */
#endif

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

 /* for attribute definitions see e.g. https://stackoverflow.com/questions/11621043/how-should-i-properly-use-attribute-format-printf-x-y-inside-a-class 
    and https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes
    Note %w (defined in C23) has been supported by the gnu_printf attribute since gcc 13 
    %Q , $ and  _ are the main cause of false errors
    https://www.gnu.org/software/libc/manual/html_node/Formatted-Output.html describes what formats are accepted by gcc for "gnu_printf"
 */
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va)
#if defined(YA_SP_SPRINTF_CHECK_FMT) && ( defined(__GNUC__) || defined(__clang__) )
	__attribute__ ((format (gnu_printf, 2, 0))) 
#endif
  ;
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vsnprintf)(char *buf, int count, char const *fmt, va_list va)
#if defined(YA_SP_SPRINTF_CHECK_FMT) && ( defined(__GNUC__) || defined(__clang__) )
	__attribute__ ((format (gnu_printf, 3, 0))) 
#endif
  ;
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...)
#if defined(YA_SP_SPRINTF_CHECK_FMT) && ( defined(__GNUC__) || defined(__clang__) )
	__attribute__ ((format (gnu_printf, 2, 3))) 
#endif
  ;
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...)
#if defined(YA_SP_SPRINTF_CHECK_FMT) && ( defined(__GNUC__) || defined(__clang__) )
	__attribute__ ((format (gnu_printf, 3, 4))) 
#endif
  ;

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vsprintfcb)(YA_S_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va)
#if defined(YA_SP_SPRINTF_CHECK_FMT) && ( defined(__GNUC__) || defined(__clang__) )
	__attribute__ ((format (gnu_printf, 4, 0))) 
#endif
  ;
  
YA_S__PUBLICDEF void YA_SP_SPRINTF_DECORATE(set_separators)(char comma, char period);

YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vfprintf)(FILE *stream, const char *format, va_list va)
#if defined(YA_SP_SPRINTF_CHECK_FMT) && ( defined(__GNUC__) || defined(__clang__) )
	__attribute__ ((format (gnu_printf, 2, 0))) 
#endif
  ;

// as above but to stdout
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(vprintf)(const char *format, va_list va)
#if defined(YA_SP_SPRINTF_CHECK_FMT) && ( defined(__GNUC__) || defined(__clang__) )
	__attribute__ ((format (gnu_printf, 1, 0))) 
#endif
  ;

// fprintf() : write to stream (file)
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(fprintf)(FILE *stream, const char *format, ...)
#if defined(YA_SP_SPRINTF_CHECK_FMT) && ( defined(__GNUC__) || defined(__clang__) )
	__attribute__ ((format (gnu_printf, 2, 3))) 
#endif
  ;

// printf(): like fprintf() but to stdout
YA_S__PUBLICDEF int YA_SP_SPRINTF_DECORATE(printf) (const char *format, ...)
#if defined(YA_SP_SPRINTF_CHECK_FMT) && ( defined(__GNUC__) || defined(__clang__) )
	__attribute__ ((format (gnu_printf, 1, 2))) 
#endif
  ;

#endif // YA_SP_SPRINTF_H_INCLUDE

#ifdef YA_SP_SPRINTF_IMPLEMENTATION

#include <stdlib.h> // for va_arg()
#include <string.h> // strlen() , memcpy(), memset() 
#include <stdint.h>  /* for int64_t etc */
#include <stdbool.h> /* for bool, true/false */
#include <wchar.h> /* for wide characters */
#include <wctype.h> /* for wide characters */
#include "../double-double/double-double.h"


// define the level of gcc optimisations used as we cannot use Ofast as with gcc 9.3.0 on ubuntu this gives incorrect results around NAN's even in main test program
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__clang__)
 #pragma GCC push_options
 #pragma GCC optimize ("-O3") /* cannot use Ofast, normally -O3 is OK. Note macro expansion does not work here ! */
 #if defined(_WIN32) && !defined(_WIN64)
  #pragma GCC target("sse2")
 #endif
#endif


#ifndef YA_SP_SPRINTF_NOFLOAT
// internal float utility functions

static bool ya_s__DD_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, double v, uint32_t frac_digits); // convert double
static bool ya_s__LD_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, long double value, uint32_t frac_digits);// used for long doubles
#if defined(YA_SP_SPRINTF_QF)  && defined(__SIZEOF_FLOAT128__) 
static bool ya_s__real128_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, __float128 value, uint32_t frac_digits);// used for f128
#endif

#define YA_S__SPECIAL 0x7000

#define YA_SP_STATIC_ASSERT(condition) extern int static_assert_##__FILE__##__LINE__[!!(condition)-1]  /* check at compile time and works in a global or function context see https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */

#ifdef YA_SP_SPRINTF_QF /* 128 bit f.p. support needs __float128  */
#ifdef __SIZEOF_FLOAT128__ /* if 128 bit integers are supported */
typedef __float128 ya_s__f128_t;
#else
#error "compiler __float128 to support Q (define YA_SP_SPRINTF_QF)"
#endif
#endif 
#else // YA_SP_SPRINTF_NOFLOAT
 #if defined(YA_SP_SPRINTF_QF) 
  #error "Cannot support both YA_SP_SPRINTF_QF and YA_SP_SPRINTF_NOFLOAT"  /* NOFLOAT "overrides" SPRINTF_QF but this check here avoids having to check both cases in the code below */
 #endif
#endif // YA_SP_SPRINTF_NOFLOAT

#ifdef YA_SP_SPRINTF_QI /* 128 bit support needs  __int128 's */
#ifdef __SIZEOF_INT128__  /* if 128 bit integers are supported */
typedef __uint128_t ya_s__uint128_t; // same format as used above
typedef __int128_t ya_s__int128_t;
#else
#error "need compiler __int128  to support QI (define YA_SP_SPRINTF_QI)"
#endif
#endif 

static char ya_s__period = '.';
static char ya_s__comma = ',';
#ifndef YA_SP_NO_DIGITPAIR
static struct
{
   uint16_t temp; // force next field to be 2-byte aligned
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
#define YA_S__L 16384 /* %Lg etc for long double */
#define YA_S__Q 32768 /* %Qg etc for __float128 - needs YA_SP_SPRINTF_Q defined to work */
#define YA_S__l 65536 /* %l - lower case "L" -> as in %ls or %lc [ numeric uses of %l are "hardcoded" ] */

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
      	 fl |= YA_S__l ; /* note the l flag - the code below handles the numeric use of "l" which is more complex, %lc and %ls use YA_S__l [ note ll is also treated as "l" in this use] */
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
      case 'w': /* %w32 etc is new in C23 */
         if ((f[1] == '6') && (f[2] == '4')) {
            fl |= YA_S__INTMAX;
            f += 3;
         } else if ((f[1] == '3') && (f[2] == '2')) {
            f += 3;
#if defined( YA_SP_SPRINTF_QI) || defined(YA_SP_SPRINTF_QF)             
         } else if ((f[1] == '1') && (f[2] == '2')&& (f[3] == '8') ) {
         	fl|= YA_S__Q; // I128 = Quad double (__float128,__int128)
            f += 4;
#endif             
         }  
		 else if(f[0]=='I') { /* %w must be followed by a number, whereas %I can appear by its self */
            fl |= ((sizeof(void *) == 8) ? YA_S__INTMAX : 0);
            ++f;
         }
         break;

	  case 'L': /* long doubles are always supported in this version, however if they are the same as doubles we ignore the L */
#if LDBL_MANT_DIG>DBL_MANT_DIG && LDBL_MAX_EXP>=DBL_MAX_EXP	  
	  		fl|= YA_S__L; // long double 
#endif
	  		++f;
			break;
	
#if defined( YA_SP_SPRINTF_QI) || defined(YA_SP_SPRINTF_QF)         
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
#ifdef YA_SP_SPRINTF_QF       
	  __float128 fv128;  // Quad double (__float128)
	  bool isf128;
	  fv128=0;
	  isf128=false;
#endif   
	  bool is_LD; /* set true for long double arguments */
	  is_LD=false;  
	  char *s="";
      // handle each replacement
      switch (f[0]) {  
		 #define YA_S__NUMSZ 20000 // clipped to 350 digits after decimal point and could have 4932 before plus commas...         
         char num[YA_S__NUMSZ];
         char const *h;
         uint32_t l, n, cs;
         uint64_t n64;
#ifndef YA_SP_SPRINTF_NOFLOAT
         double fv;    
	  	 long double fvL; // long double

#endif
#ifdef YA_SP_SPRINTF_QI    
	      ya_s__uint128_t u128; // 128 bit integer
#endif	         
         int32_t dp;
         char const *sn;

	  case 'S': // wide character string (microsoft)
	  			fl |= YA_S__l ; // set "l" flag then treat as %ls [ falls through to that code below  ]       
      case 's':
         // get the string
         if(fl & YA_S__l)
         	{ /* %ls expect a *wchar_t argument : C standard says: 
					If an l length modifier is present, the argument shall be a pointer to storage of wchar_t
					type. Wide characters from the storage are converted to multibyte characters (each as if
					by a call to the wcrtomb function, with the conversion state described by an mbstate_t
					object initialized to zero before the first wide character is converted) up to and including
					a terminating null wide character. The resulting multibyte characters are written up to
					(but not including) the terminating null character (byte). If no precision is specified, the
					storage shall contain a null wide character. If a precision is specified, no more than that
					many bytes are written (including shift sequences, if any), and the storage shall contain
					a null wide character if, to equal the multibyte character sequence length given by the
					precision, the function would need to access a wide character one past the end of the array.
					In no case is a partial multibyte character written.				
				For this to have any impact, setlocale(LC_CTYPE, ""); // or "en_US.UTF-8" etc
				or similar must have been called previoulsy.
			  */
         	 const wchar_t *pwc=(wchar_t *)va_arg(va, wchar_t *);
         	 if(pwc!=NULL)
         	 	{s=num;
#ifdef YA_SP_WCHAR_PR_CHARS  	/*  precision (pr) in (wide) characters not bytes [ this matches mingw and ucrt ] field width is also in characters [this does not agree with the C standard] */
	         	 int i;// nos (wide) characters
	         	 size_t r=0;// holds return value of wcrtomb
			     mbstate_t cstate; // need a state initialised to zero at the start as per C standard (cannot use wctomb(NULL,*pwc); as this would impact "global state")
			     memset(&cstate, 0, sizeof cstate);		         	 
				 for(i=0;  i<(uint32_t)pr && *pwc ; s+=r,i++) 
				 	{r=wcrtomb(s, *pwc++,&cstate);// copy over at most pr characters. wcrtomb() converts a wide char to a multibyte sequence using the current locale and returns nos bytes placed into s 
				 	 if(r==(size_t)-1) break;// invalid wchar_t 
				 	 if(i+MB_CUR_MAX>=sizeof(num)) break;// the array num is "big" so this is very unlikely - silently truncate (C standard only requires a 4096 byte buffer, this is much more than that).
				 	}
			     if (r == (size_t)-1) 
				 	{
			        /* conversion failed */
			         s="?";
			         l=1;
			     	}
			     else
			     	{/* conversion ok */
			     	 l=s-num;// actual length char * string
			     	 if(fw!=0)
			     	 	fw+=l-i;// if field wide set by user, adjust it to reflect difference in nos of wide chars and nos of chars
			     	 s=num;
					}	
								 
#else  /*  precision (pr) in bytes [ as per C standard ] filed width also in bytes (C standard says characters) - this matches Linux (at least  Ubuntu 24.04.3 LTS (GNU/Linux 6.6.87.2-microsoft-standard-WSL2 x86_64) using gcc64 13.2.0 */         	 	
	         	 size_t max_len= sizeof(num); // abs max length
	         	 if( (uint32_t)pr<sizeof(num)) max_len=pr; // we use (uint32_t)pr as pr is -1 if not set (cast makes -1 become a very large number)
			     mbstate_t cstate; // need a state initialised to zero at the start as per C standard (cannot use wctomb(NULL,*pwc); as this would impact "global state")
			     memset(&cstate, 0, sizeof cstate);			         	 
			     /* wcsrtombs converts a wide char string to a multibyte sequence using current locale */
  #if defined(__BORLANDC__) && !defined(_UCRT)
             	 size_t n = wcstombs(s, pwc, max_len);// builder C++ Windows32 & Windows-64 don't have wcstombs() , note 2nd arg don't have "&" - this is correct.
   #else
			     size_t n = wcsrtombs(s, &pwc, max_len,&cstate);
  #endif
			     if (n == (size_t)-1) 
				 	{
			        /* conversion failed */
			         s="?";
			         l=1;
			     	}
			     else
			     	{/* conversion ok */
			     	 l=n;
					}
#endif					
				}
			 else s=NULL;
			}
		 else
		 	{ // just %s         
         	 s = va_arg(va, char *);
         	 if(s!=NULL)
         	 	{
         	 	 // get the length
				 l=strlen(s);        
		         // clamp to precision
		         if (l > (uint32_t)pr)
		            l = pr;
		        }
         	}
         if (s == NULL)  
         	{
#ifdef YA_SP_FULL_NULL
			 if((uint32_t)pr<strlen("(null)") )
				s="" ; // print nothing for null strings when a precision is specified thats too small to fully print "(null)"
			 else
				s = "(null)"; // PMi was "null" changed to match built in sprintf()	
#else         
             s = "(null)"; // PMi was "null" changed to match built in sprintf()
#endif      
     	 	 // get the length
			 l=strlen(s);        
	         // clamp to precision
	         if (l > (uint32_t)pr)
	            l = pr;
      		}

         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         // copy the string in
         goto scopy;

	  case 'C': // wide character (microsoft)
	  			fl |= YA_S__l ; // set "l" flag then treat as %lc [ falls through to that code below ]
      case 'c': // char
         // get the character
         // s = num + YA_S__NUMSZ - 1;
         s=num;
         if(fl & YA_S__l)
         	{ /* %lc expect a wint_t argument : C standard says: 
	         	     If an l length modifier is present, the wint_t argument is converted as if by a call to
					the wcrtomb function with a pointer to storage of at least MB_CUR_MAX bytes, the wint_t
					argument converted to wchar_t, and an initial shift state.
				
				For this to have any impact, setlocale(LC_CTYPE, ""); // or "en_US.UTF-8" 
				or similar must have been called previoulsy.
			  */
         	 wint_t wc=(wint_t)va_arg(va, int);// Note that 'wint_t' {aka 'short unsigned int'} is promoted to 'int' when passed through '...', so the type passed to va_arg must be "int"
		     mbstate_t state;
		     memset(&state, 0, sizeof state); // initial shift state
		     wchar_t w = (wchar_t)wc;
		     /* wcrtomb converts one wide char to a multibyte sequence using current locale */
  #if defined(__BORLANDC__) && !defined(_UCRT)
             size_t n = wctomb(s, w);// builder C++ Windows32 & Windows-64 don't have wcrtomb()
   #else
		     size_t n = wcrtomb(s, w, &state);// sizeof(s) is much larger than MB_CUR_MAX
  #endif
		     if (n == (size_t)-1) 
			 	{
		        /* conversion failed */
		         s="?";
		         l=1;
		     	}
		     else
		     	{/* conversion ok */
		     	 s[n] = '\0';// not strinctly necessary as l defines the length
		     	 l=n;
				}
			}
		 else
		 	{ /* just %c */	
         	 *s = (char)va_arg(va, int);
         	 l = 1;
         	}
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
#ifdef YA_SP_SPRINTF_QI  
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
         l=strlen(s);
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
	     if(fl & YA_S__L) // long double 
	     	{
			 fvL=va_arg(va, long double);		 
         	 if (isnan(fvL) || isinf(fvL)) 
		   		{
				 fl &= ~YA_S__LEADINGZERO;// special (nan,inf) don't have leading zeros
				 if(isupper(f[0]))
					{	
#ifdef YA_SP_SIGNED_NANS_LD
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
#ifdef YA_SP_SIGNED_NANS_LD
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
           	 // the code below uses long doubles even for doubles 
			}
	  	 else	
#ifdef YA_SP_SPRINTF_QF       
	  	   if(fl & YA_S__Q) //__float128  
	  	   	{
			 fv128=va_arg(va, __float128);
         	 if (isnanq(fv128) || isinfq(fv128)) 
		   		{
				 fl &= ~YA_S__LEADINGZERO;// special (nan,inf) don't have leading zeros
				 if(isupper(f[0]))
					{	
#if defined( YA_SP_SIGNED_NANS) || defined(YA_SP_SIGNED_NANS_F128)
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
#if defined( YA_SP_SIGNED_NANS) || defined(YA_SP_SIGNED_NANS_F128)
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
			  // we need to process float128 differently to others as mantissa is > 64 bits (we will use ya_s__uint128_t for mantissa if compiler supports int128 types, otherwise we use u2_64 types)
			 int origpr=pr;
		 	 if(pr== -1) pr=28; // default - full resolution of 112 bit float128 mantissa
		 	 if(signbitq(fv128) )
		 		{fl |= YA_S__NEGATIVE;
			 	 fv128= -fv128;	
				}
		 	 s = num + 64;
         	 ya_s__lead_sign(fl, lead);
		 	 ya_s__f128_t dmant=frexpq(fv128,&dp); // get mantissa and exponent, 0.5<=dmant<1
#ifdef __SIZEOF_INT128__ /* we can use compiler "int128" types */
		 	 //  quadmath_snprintf(%a) prints 1.xxx whereas ya_s_snprintf(%a) by default prints 8.xxx [which is better as it gives us more resolution for a given number of digits ]
		 	 // also does not let exponent go below -16382
		 	 __uint128_t n128=rintq(ldexpq(dmant,113)); // convert mantissa to a uint128 by multiplying by 2^113
		 	 if(fv128!=0)
			 	dp-=1; // have 1.x before decimal point		
		 	 if(dp< -16382)
		 		{n128>>=(-16382-dp);
		 		 // n128 >>= 1;
			 	 dp= -16382;
				}		  
	        if (pr < 28)
	            {
				 n128 += (((__uint128_t)8) << (108 - pr*4)) ; // rounding
	             n128 &= ~ (((__uint128_t)8) << (108 - pr*4)) ; // get rid of bit we added for rounding as it messes up trailing zero detection below
	             if(n128 & ((__uint128_t)16)<<112 ) // we use & 15 (0x0f) below to extract 1st digit so 16 is 1 bit up
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
	         n128 ^= n128 & ((__uint128_t)15)<<112 ; // delete digit just printed
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
	             n128 ^= n128 & ((__uint128_t)15)<<112 ; // delete digit just printed
	             n128 <<= 4;
	         	}
			 goto a_pr_axp; // print exponent like long double
			}			 
	  	 else
#else // compiler does not support "int128" so this version uses u2_64's to emulate a 128 bit data type (see files u2_64.[ch])
		 	 //  quadmath_snprintf(%a) prints 1.xxx whereas ya_s_snprintf(%a) by default prints 8.xxx [which is better as it gives us more resolution for a given number of digits ]
		 	 // also does not let exponent go below -16382
		 	 // this version uses u2_64 to give 128 bits maths using just uint64_t types - from u2_64.[ch]
		 	 u2_64 n128=flt128_to_u2_64(rintq(ldexpq(dmant,113))); // convert mantissa to a uint128 by multiplying by 2^113
		 	 if(fv128!=0)
			 	dp-=1; // have 1.x before decimal point		
		 	 if(dp< -16382)
		 		{// n128>>=(-16382-dp);
		 		 n128=rshift_u2_64(n128,-16382-dp);
			 	 dp= -16382;
				}		  
	        if (pr < 28)
	            {
				 //n128 += (((__uint128_t)8) << (108 - pr*4)) ; // rounding
				 n128=uadd_u2_64(n128,lshift_u2_64(u64_to_u2_64(0,8),108 - pr*4));// rounding
	             // n128 &= ~ (((__uint128_t)8) << (108 - pr*4)) ; // get rid of bit we added for rounding as it messes up trailing zero detection below
	             n128=and_u2_64(n128,not_u2_64(lshift_u2_64(u64_to_u2_64(0,8),108 - pr*4)));  // get rid of bit we added for rounding as it messes up trailing zero detection below
	             //if(n128 & ((__uint128_t)16)<<112 ) // we use & 15 (0x0f) below to extract 1st digit so 16 is 1 bit up
	             if(cmp_u2_64(u64_to_u2_64(0,0),and_u2_64(n128,lshift_u2_64(u64_to_u2_64(0,16),112))) !=0)
	             	{// rounding caused overflow, fix this
	             	 //n128>>=1; // divide by 2 (we did not actually overflow n128, just the range we are using) and adjust exponent to compensate
	             	 n128=rshift_u2_64(n128,1);
	             	 dp++;
	             	}
	        	}	
  	
			// add leading chars
	         lead[1 + lead[0]] = '0';
	         lead[2 + lead[0]] = (f[0] == 'A') ?'X':'x';
	         lead[0] += 2;
	         //*s++ = h[(n128 >> 112) & 15];//  we want 1. for 1st digit as thats what quadmath_snprintf () generates
	         {u2_64 t128=rshift_u2_64(n128,112);
	          *s++=h[t128.lo & 15];
	         }
	         //n128 ^= n128 & ((__uint128_t)15)<<112 ; // delete digit just printed
	         n128=xor_u2_64(n128,and_u2_64(n128,lshift_u2_64(u64_to_u2_64(0,15),112)));
	         //n128 <<= 4;
	         n128=lshift_u2_64(n128,4);
	         //if (origpr>=0 || (origpr<0 && n128 !=0) || (fl & YA_S__LEADING_0X) ) // PMi only print period if non-zero digits after dp, or user requested it (eg %.5A) or %#A (LEADING_0X flag)
	         if (origpr>=0 || (origpr<0 && (n128.hi | n128.lo) !=0  ) || (fl & YA_S__LEADING_0X) ) // PMi only print period if non-zero digits after dp, or user requested it (eg %.5A) or %#A (LEADING_0X flag)
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
	         while (n-- && (origpr!= -1 || (n128.hi | n128.lo )!=0)) 
			 	{ // PMi only print if rest are non-zero or user requested it (eg %.5A)
	             // *s++ = h[(n128 >> 112) & 15];
	             u2_64 t128=rshift_u2_64(n128,112);
	             *s++=h[t128.lo & 15];
	             //n128 ^= n128 & ((__uint128_t)15)<<112 ; // delete digit just printed
	             n128=xor_u2_64(n128,and_u2_64(n128,lshift_u2_64(u64_to_u2_64(0,15),112)));
	             // n128 <<= 4;
	             n128=lshift_u2_64(n128,4);
	         	}
			 goto a_pr_axp; // print exponent like long double
			}			 
	  	 else
#endif	  	 
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
 #ifdef YA_SP_NAN_IND
                        if(signbit(fv)) 
							{if(ya_is_indefinite_double(fv)) sn="-NAN(IND)";
							 else if(ya_is_snan_double(fv)) sn="-NAN(SNAN)";
							 else sn="-NAN";
							}
				 	    else
							{if(ya_is_indefinite_double(fv)) sn="NAN(IND)";
							 else if(ya_is_snan_double(fv)) sn="NAN(SNAN)";
							 else sn="NAN";
							}				 	    
 #else
                        if(signbit(fv)) sn="-NAN";
				 	    else		      sn= "NAN";
 #endif				 	    
				       }
#else				
 #ifdef YA_SP_NAN_IND
 					 if(isnan(fv)) 
 					 		{if(ya_is_indefinite_double(fv)) sn="NAN(IND)";
							 else if(ya_is_snan_double(fv)) sn="NAN(SNAN)";
							 else sn="NAN";
							}
 #else		
				 	 if(isnan(fv)) 	sn="NAN";
 #endif				 	 
#endif				 	 
				 	 else if(signbit(fv))	sn="-INF";
				 	 else 			sn="INF";
					}
				 else
					{
#ifdef YA_SP_SIGNED_NANS
                     if(isnan(fv))
                       {
 #ifdef YA_SP_NAN_IND
                        if(signbit(fv)) 
							{if(ya_is_indefinite_double(fv)) sn="-nan(ind)";
							 else if(ya_is_snan_double(fv)) sn="-nan(snan)";
							 else sn="-nan";
							}
				 	    else
							{if(ya_is_indefinite_double(fv)) sn="nan(ind)";
							 else if(ya_is_snan_double(fv)) sn="nan(snan)";
							 else sn="nan";
							}	
 #else
                        if(signbit(fv)) sn="-nan";
				 	    else		      sn= "nan";
 #endif				 	    
				       }
#else				
 #ifdef YA_SP_NAN_IND
 					 if(isnan(fv)) 
 					 		{if(ya_is_indefinite_double(fv)) sn="nan(ind)";
							 else if(ya_is_snan_double(fv)) sn="nan(snan)";
							 else sn="nan";
							}
 #else		
				 	 if(isnan(fv)) 	sn="nan";
 #endif				 	 
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
 //#if defined(YA_SP_A_FMT__ALT3) || defined (YA_SP_A_FMT__ALT4)
		 int origpr=pr;
 //#endif		 
		 if(fl & YA_S__L) // long double 
		 	{ if(pr== -1) pr=15; // default - full resolution 64 bit mantissa is present [ total of 16 hex digits as 1 before DP ]
			  if(signbit(fvL) && !isnan(fvL))
				{fl |= YA_S__NEGATIVE;
				 fvL= -fvL;	
				}
		 	}
		 else // just normal double
		 	{ if(pr==-1) pr=13 ; // all thats needed for full resolution with a double (52 bit mantissa + hidden bit )	
			  if(signbit(fv) && !isnan(fv))
				{fl |= YA_S__NEGATIVE;
				 fv= -fv;	
				}				
		 	  fvL=fv; // using long doubles now
		 	}

		 s = num + 64;
         ya_s__lead_sign(fl, lead);

#if ( LDBL_MANT_DIG>DBL_MANT_DIG && LDBL_MAX_EXP>=DBL_MAX_EXP)	  /* ie if we have real long doubles */		 
		 long double dmant=frexpl(fvL,&dp); // get mantissa and exponent, 0.5<=dmant<1
	 	 n64=(uint64_t)rintl(ldexpl(dmant,64)); // convert mantissa to a uint64 by multiplying by 2^64
		 if(fvL!=0)
		 		dp-=4; // have 1 hex digit before decimal point		 
#else   /* no real long doubles, use doubles as otherwise Builder C++ 12.1 Windows 64-bit (Modern) gives silly values */
		 double dmant=frexp(fv,&dp); // get mantissa and exponent, 0.5<=dmant<1
	 	 n64=(uint64_t)rint(ldexp(dmant,64)); // convert mantissa to a uint64 by multiplying by 2^64
		 if(fv!=0)
		 		dp-=4; // have 1 hex digit before decimal point		 
#endif	

		 // printf("\n n64=0x%llx, dmant=%Lg ldexpl(dmant,64)=%Lg lrintl(ldexpl(dmant,64))=%Lg dp=%d\n",n64,dmant,ldexpl(dmant,64),rintl(ldexpl(dmant,64)),dp);
			  
#ifdef YA_SP_A_FMT_ALT1
		// %a is printed as 1p+0 rather than 8p-3 also min exponent is clamped to -1022. Round to even
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
#elif defined(YA_SP_A_FMT_ALT2)
		// %a is printed as 1p+0 rather than 8p-3 also min exponent is clamped to -1022. Basic rounding
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
             uint64_t rnd=(((uint64_t)8) << (56-pr*4) ); // bit to add for rounding
			 n64 += rnd; // rounding
             n64 &= ~ rnd; // PMi : get rid of bit we added for rounding as it messes up trailing zero detection below
             if(n64<orig_n64)
             	{// rounding caused overflow, fix this
             	 n64=(((uint64_t)8) << 60);
             	 dp++;
             	}
        	}	
#elif defined(YA_SP_A_FMT_ALT4)
		// ALT1 for doubles, ALT2 for LD
		if((fl & YA_S__L)==0 && n64!=0)  // ! long double [ ie a double]  & n64!=0
			{n64>>=3;
			 dp+=3;
             if(dp< -1022)
               {n64>>=(-1022-dp);
                dp=-1022;
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
			}
		 else if(fl & YA_S__L)	// long double
		 	{
		 	 if(dp<-16385) // denorm
			 	{n64>>=(-16385-dp);
			 	 dp=-16385;
			 	}
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
#ifdef YA_SP_A_FMT_ALT3 /* no trailing zero suppression - so always need dp */
		 *s++ = ya_s__period;
#elif defined(YA_SP_A_FMT_ALT4) // ALT3 for double only
		 if((fl & YA_S__L)==0) // double)
		 	{*s++ = ya_s__period;
		 	}
		 else 
		 	{// LD
		 	 if (origpr>=0 || (origpr<0 && n64 !=0) || (fl & YA_S__LEADING_0X) ) // PMi only print period if non-zero digits after dp, or user requested it (eg %.5A) or %#A (LEADING_0X flag)
            	*s++ = ya_s__period;
		 	}
#else         
         if (origpr>=0 || (origpr<0 && n64 !=0) || (fl & YA_S__LEADING_0X) ) // PMi only print period if non-zero digits after dp, or user requested it (eg %.5A) or %#A (LEADING_0X flag)
            *s++ = ya_s__period;
#endif            
         sn = s;

         // print the bits
         if(pr<0) n=0; // PMi
         else n = pr;
         if (n > 15)
            n = 15;           
         if (pr > (int32_t)n)
            tz = pr - n;
         pr = 0;
#ifdef YA_SP_A_FMT_ALT3 /* no trailing zero suppression */
         while (n-- ) { // no trailing zero suppression
            *s++ = h[(n64 >> 60) & 15];
            n64 <<= 4;
#elif defined(YA_SP_A_FMT_ALT4) // ALT3 for double only
		 if((fl & YA_S__L)==0) // double)
		 	{
		 	 while (n-- ) { // no trailing zero suppression
	            *s++ = h[(n64 >> 60) & 15];
	            n64 <<= 4;
			 	}
			}
		 else
		 	{// LD
	         while (n-- && (origpr!= -1 || n64!=0)) { // PMi only print if rest are non-zero or user requested it (eg %.5A)
	            *s++ = h[(n64 >> 60) & 15];
	            n64 <<= 4;		 	
		 	}
#else         
         while (n-- && (origpr!= -1 || n64!=0)) { // PMi only print if rest are non-zero or user requested it (eg %.5A)
            *s++ = h[(n64 >> 60) & 15];
            n64 <<= 4;
#endif            
         }
#ifdef  YA_SP_SPRINTF_QF 
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
	     if(fl & YA_S__L) // long double 
	     	{
			 fvL=va_arg(va, long double);
			 is_LD=true;
			 fv=(double)fvL; // simple way to allow original code to work until something better is added
			}
	  	 else	
#ifdef YA_SP_SPRINTF_QF       
	  	   if(fl & YA_S__Q) //__float128  
	  	   	{
			 fv128=va_arg(va, __float128);
			 isf128=true; // we have a float128
			 fv=(double)fv128; // simple way to allow original code to work until something better is added
			 fvL=(long double)fv128;
			}			 
	  	 else
#endif     
			{       
             fv = va_arg(va, double); // might be trailing part of an else ..             
             fvL=fv; // process as long double            
        	}
         if (pr == -1)
            pr = 6; // default is 6
         else if (pr == 0)
            pr = 1; 
         if(pr>350) pr=350; // limit number of digits after dp to something sensible [so buffer size is limited]            
         // convert the double into a string  
#if defined(YA_SP_SPRINTF_QF)  && defined(__SIZEOF_FLOAT128__)        
         if(fl & YA_S__Q && isf128)
         	{// we have a __float128
		 	if (ya_s__real128_to_str(&sn, &l, num, &dp, fv128, (pr - 1) | 0x80000000))         
            	fl |= YA_S__NEGATIVE;         	
         	}
         else
#endif        
		    if(is_LD)
         	{
		 	if (ya_s__LD_to_str(&sn, &l, num, &dp, fvL, (pr - 1) | 0x80000000))         
            	fl |= YA_S__NEGATIVE;
            }			
		 else 
         	{
		 	if (ya_s__DD_to_str(&sn, &l, num, &dp, fv, (pr - 1) | 0x80000000))  // only ever pass a double   
            	fl |= YA_S__NEGATIVE;
            }

        
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
	     if(fl & YA_S__L) // long double 
	     	{
			 fvL=va_arg(va, long double);
			 is_LD=true;
			 fv=(double)fvL; // simple way to allow original code to work until something better is added
			}
	  	 else
#ifdef YA_SP_SPRINTF_QF       
	  	   if(fl & YA_S__Q) //__float128  
	  	   	{
			 fv128=va_arg(va, __float128);
			 isf128=true;
			 fv=(double)fv128; // simple way to allow original code to work until something better is added
			 fvL=(long double)fv128;
			}			 
	  	 else
#endif    
			{        
             fv = va_arg(va, double); // is part of trailing else...
			 fvL=fv;
			}
         if (pr == -1)
            pr = 6; // default is 6
         if(pr>350) pr=350; // limit number of digits after dp to something sensible [so buffer size is limited]            
         // convert the double into a string    
#if defined(YA_SP_SPRINTF_QF)  && defined(__SIZEOF_FLOAT128__)        
         if(fl & YA_S__Q && isf128)
         	{// we have a __float128
		 	if (ya_s__real128_to_str(&sn, &l, num, &dp, fv128, pr | 0x80000000))         
            	fl |= YA_S__NEGATIVE;         	
         	}
         else
#endif        
		    if(fl & YA_S__L && is_LD)
         	{
		 	if (ya_s__LD_to_str(&sn, &l, num, &dp, fvL, pr | 0x80000000))         
            	fl |= YA_S__NEGATIVE;
            }			
		 else 
         	{
		 	if (ya_s__DD_to_str(&sn, &l, num, &dp, fv, pr | 0x80000000))     // only ever pass a double   
            	fl |= YA_S__NEGATIVE;
            }


          
      doexpfromg:
         tail[0] = 0;
         ya_s__lead_sign(fl, lead);
         if (dp == YA_S__SPECIAL) {
			fl &= ~YA_S__LEADINGZERO;// special (nan,inf) don't have leading zeros
#ifdef YA_SP_NAN_IND  
			if(( fl & YA_S__L)	|| (fl & YA_S__Q)  )		
				{// if long double or float128 only use "nan"
				 if(strcmp(sn,"nan(snan)")==0 ) sn="nan";
				 else if(strcmp(sn,"nan(ind)")==0 ) sn="nan";
				}
#endif			
			if(isupper(f[0]))
				{
#ifdef YA_SP_NAN_IND 
				 /* more options where we need to change case */
				 if(strcmp(sn,"nan(snan)")==0 ) sn="NAN(SNAN)";
				 else if(strcmp(sn,"nan(ind)")==0 ) sn="NAN(IND)";
				 else if(strcmp(sn,"nan")==0 ) sn="NAN";
				 else if(*sn=='i') sn="INF";
#else			 /* simple case changes */	
				 if(*sn=='n') sn="NAN";
				 else if(*sn=='i') sn="INF";
#endif				 
				}
           
            s = (char *)sn;
            l=strlen(s);
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
#ifdef  YA_SP_SPRINTF_EXP3 /* min 3 digits in exponent (otherwise min of 2 digits in exponent) */

         n = dp>=1000?6:((dp >= 100) ? 5 : 5);
#else    /* 2 digits in exponent is default (unless more needed) */      
         n = dp>=1000?6:((dp >= 100) ? 5 : 4);
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
	     if(fl & YA_S__L) // long double 
	     	{
			 fvL=va_arg(va, long double);
			 is_LD=true;
			 fv=(double)fvL; // simple way to allow original code to work until something better is added
			}
	  	 else	
#ifdef YA_SP_SPRINTF_QF       
	  	   if(fl & YA_S__Q) //__float128  
	  	   	{
			 fv128=va_arg(va, __float128);
			 isf128=true;
			 fv=(double)fv128; // simple way to allow original code to work until something better is added
			 fvL=(long double)fv128;
			}			 
	  	 else
#endif      
			{   
             fv = va_arg(va, double); // might be part of a trailing else ... 
			 fvL=fv;            
         	}
      doafloat: 
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
#ifdef YA_SP_SPRINTF_QF                
               fv128/=divisor;
#endif               
               fl += 0x1000000;
            }
         }         
         if (pr == -1)
            pr = 6; // default is 6
         if(pr>350) pr=350; // limit number of digits after dp to something sensible [so buffer size is limited]
         // convert the double into a string  
#if defined(YA_SP_SPRINTF_QF)  && defined(__SIZEOF_FLOAT128__)        
         if(fl & YA_S__Q && isf128)
         	{// we have a __float128
		 	 if (ya_s__real128_to_str(&sn, &l, num, &dp, fv128, pr))         
            	fl |= YA_S__NEGATIVE;         	
         	}
         else
#endif        
		    if(fl & YA_S__L && is_LD)
         	{
		 	 if (ya_s__LD_to_str(&sn, &l, num, &dp, fvL, pr))         
            	fl |= YA_S__NEGATIVE;
            }
		 else if (fl & YA_S__METRIC_SUFFIX) 
		 	{
		 	 if (ya_s__LD_to_str(&sn, &l, num, &dp, fvL, pr))  // gets here with a long double - we always have long doubles (but they might be identical to doubles) so can use the LD conversion function       
            	fl |= YA_S__NEGATIVE;
		 	}				
		 else 
         	{  // used only for doubles 
#pragma GCC diagnostic push
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__BORLANDC__)
 #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
		 	 if (ya_s__DD_to_str(&sn, &l, num, &dp, fv, pr))       
            	fl |= YA_S__NEGATIVE;
#pragma GCC diagnostic pop
            }
           
      dofloatfromg:
         tail[0] = 0;
         ya_s__lead_sign(fl, lead);
         if (dp == YA_S__SPECIAL) {
			fl &= ~YA_S__LEADINGZERO;// special (nan,inf) don't have leading zeros   
#ifdef YA_SP_NAN_IND 
			if(( fl & YA_S__L)	|| (fl & YA_S__Q)  )
				{// if long double or float128 only use "nan"
				 if(strcmp(sn,"nan(snan)")==0 ) sn="nan";
				 else if(strcmp(sn,"nan(ind)")==0 ) sn="nan";
				}
#endif			 
			if(isupper(f[0]))
				{
#ifdef YA_SP_NAN_IND 
				 /* more options where we need to change case */
				 if(strcmp(sn,"nan(snan)")==0 ) sn="NAN(SNAN)";
				 else if(strcmp(sn,"nan(ind)")==0 ) sn="NAN(IND)";
				 else if(strcmp(sn,"nan")==0 ) sn="NAN";
				 else if(*sn=='i') sn="INF";
#else			 /* simple case changes */	
				 if(*sn=='n') sn="NAN";
				 else if(*sn=='i') sn="INF";
#endif				
				}			     	
            s = (char *)sn;      
			l=strlen(s);   	        	
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
#ifdef YA_SP_PTR_0X  
         fl|= YA_S__LEADING_0X; // want leading 0x      
#else
#ifdef YA_SP_PTR_LEADINGZEROS
		 pr = sizeof(void *) * 2; // always print leading zeros  
#else
         if(fw==0) pr = sizeof(void *) * 2; // if no field width specified print leading zeros    
#endif         
         fl &= ~YA_S__LEADINGZERO; 
#endif         
           // fall through - to X

      case 'X': // upper hex
      case 'x': // lower hex
#ifdef YA_SP_PTR_CAPS
		 h = (f[0] == 'X' || f[0] == 'p') ? hexu : hex; // *p in upper case
#else
         h = (f[0] == 'X') ? hexu : hex;
#endif         
         l = (4 << 4) | (4 << 8);
         lead[0] = 0;
         if (fl & YA_S__LEADING_0X) {
            lead[0] = 2;
            lead[1] = '0';
            lead[2] = h[16];
         }
      radixnum:
         // get the number
#ifdef YA_SP_SPRINTF_QI       
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

#ifdef YA_SP_NO_NEG_LEADINGPLUS
		// + flags dosn't apply to unsigned so make sure its not set
        fl &= ~YA_S__LEADINGPLUS;
#endif    
#ifdef YA_SP_NO_NEG_LEADINGSPACE
		// space flags doesn't apply to unsigned so make sure its not set
		fl &= ~YA_S__LEADINGSPACE;
#endif      
      case 'i':
      case 'd': // integer
         // get the integer and abs it
#ifdef YA_SP_SPRINTF_QI  
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
#ifdef YA_SP_SPRINTF_QI      	
            if (((fl & YA_S__Q)?u128:n64) < 1024)
#else
			if (n64 < 1024)
#endif            
               pr = 0;
            else if (pr == -1)
               pr = 1; 
#ifdef YA_SP_SPRINTF_QI  
		if (fl & YA_S__Q) // 128 bit integer
			fvL= u128;
		else
#endif			
            fvL = (long double)(uint64_t)n64; // needs to be unsigned as we have already stripped sign from signed numbers, but could be an unsigned number (%u)            
          goto doafloat;
         }
#endif

         // convert to string
         s = num + YA_S__NUMSZ;
         l = 0;
#ifdef YA_SP_SPRINTF_QI  
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

// ya_s__DD_to_str() given a double-double value, returns the significant digits in len, and the position of the
//   decimal point in decimal_pos.  +/-INF and NAN are specified by special values
//   returned in the decimal_pos parameter.
// frac_digits is absolute normally, but if you want from first significant digits (got %g and %e), or in 0x80000000
// returns the sign (false=positive, true=negative)
// #define MONITOR_POWER10_ACCURACY /* if defined monitor power of 10 estimation accuracy for doubles */

#ifdef MONITOR_POWER10_ACCURACY
uint64_t nos_10_correct=0,nos_10_wrong=0;
#endif

#ifdef YA_SP_RYU /* use ryu algorithm - which should be fast and accurate */
 // this code is based on function d2exp_buffered_n_ya_sprintf() in d2fixed_ya_sprintf.c
 // This function was added by Peter Miller, but heavily leaverages the existing ryu code.
 #include "ryu/d2fixed_ya_sprintf.h"

 /* returns sign of v */
static bool ya_s__DD_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, double v, uint32_t frac_digits)
{ // 1st deal with the various special cases
 /* assume ieee format doubles (which is what ryu does */
 /* assume ieee format double */
 union {
    double real_d;
    uint64_t bits_d;
  } u = { v };
    // Decode bits into sign, mantissa, and exponent.
 const bool ng = (u.bits_d  & ((uint64_t)1<<63)) != 0;// true if v is negative 1<<63 as sign bit is the msb 
 const uint64_t mantissa = u.bits_d & 0xFFFFFFFFFFFFFULL;
 const int32_t expo=(int)((u.bits_d>>52) & 0x7ff) ; 
 if(expo==0x7ff)
 	{// nan or inf
 	 if(mantissa) // nan
 	 	{
 #ifdef _UCRT 	 	
		   if(u.bits_d==0xFFF8000000000000ULL) // test against bits_d as this value is actually -ve
 		 	{
 		 	 *start="nan(ind)"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 8;
     	 	}
		  else if( (mantissa & ((uint64_t)1 << 51)) == 0)
 		 	{
 		 	 *start="nan(snan)"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 9;
     	 	} 
 	 	 else 
 		 	{
 		 	 *start="nan"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 3;
     	 	}	 	 	 
 #else
	 	 *start="nan"; 
 	 	 *decimal_pos = YA_S__SPECIAL;
 	 	 *len = 3;	
 #endif 
#ifdef YA_SP_SIGNED_NANS
         return ng;
#else     	 
     	 return false;// nan is always positive
#endif   	 	 
 	 	}
 	 else 
		{*start="inf"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
     	 return ng;
		} 	 
 	}	 	
 if(expo==0 && mantissa==0)
 	{// zero is special as we cannot scale it into range : return 0 
     *decimal_pos = 1;
     *start = out;
     out[0] = '0';
     *len = 1;
     return ng;
 	}	
 // d2exp_buffered_n_ya_sprintf() ignores the sign of vh, so we don't need to make it positive here

  // ryu code starts here
  int32_t dec_exp,digits=0;
  // frac_digits is absolute normally, but if you want from first significant digits (got %g and %e), or in 0x80000000
  if( (frac_digits & 0x80000000)==0)
  	{// %f format - we need to know 10's exponent to fix decimal point 
  	 int32_t tens=expo-1022;// convert to a "proper" (2^x) signed exponent (tens will eventually be 10^x exponent)
	 if(tens==-1022)
	 	{// find correct exponent for denormalised numbers
	#if 1
		 tens+=12-__builtin_clzll(mantissa);// see https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html#index-_005f_005fbuiltin_005fclzll - this is normally much faster than the code below as the builtin is normally a single CPU instruction.	 
	#else
	 	 uint64_t mant=mantissa;
	 	 while((mant & ((uint64_t)1<<51)) == 0) { mant<<=1;tens--;}
	#endif
	 	}
	 // log10 estimate 
	 tens=(tens <= 0) ? (((tens * 78913) / 262144)-1) : (((tens * 78913) / 262144) ); //  78913/2^18=0.301029205322265625 (2^18= 262,144), log10(2)=0.30102999566398119521373889472449
	 digits=(int32_t)(tens + (int32_t)frac_digits); 
  	 if(digits>18) digits=18; // 18 is a sensible limit (any other value gives part 2 errors) - this must be limited - the "main code" will add extra zero's if the user asks for something "silly".
  	 if(digits<0) digits=0;	 
  	 *len=d2exp_buffered_n_ya_sprintf(mantissa,expo, digits, out, &dec_exp);// 1st call , 86% of the time will be correct (on test program), the remainder takes a 2nd attempt
  	 // printf("tens=%d dec_exp=%d\n",tens,dec_exp);
  	 if(tens!=dec_exp) 
	   	{
 #ifdef MONITOR_POWER10_ACCURACY	   	
		 nos_10_wrong++;
 #endif
		 digits=(int32_t)(dec_exp + (int32_t)frac_digits);// 2nd attempt - this should always work - needed ~ 14% of the time
		 if(digits>18) digits=18; // 18 is a sensible limit (any other value gives part 2 errors) - this must be limited - the "main code" will add extra zero's if the user asks for something "silly".
  	 	 if(digits<0) digits=0;	 
  	 	 *len=d2exp_buffered_n_ya_sprintf(mantissa,expo, digits, out, &dec_exp);
  	 	}
#ifdef MONITOR_POWER10_ACCURACY  	 	
  	 else nos_10_correct++;
#endif  	 
  	}
  else 
  	{digits=(int32_t)(frac_digits & 0x7ffffff);
  	 if(digits>18) digits=18; // 18 is a sensible limit (any other value gives part 2 errors) - this must be limited - the "main code" will add extra zero's if the user asks for something "silly".
  	 if(digits<0) digits=0;
  	 // int d2exp_buffered_n_ya_sprintf(double d, uint32_t precision, char* result,int32_t *decimal_pos);
  	 *len=d2exp_buffered_n_ya_sprintf(mantissa,expo, digits, out, &dec_exp);
  	}
  // we don't print the exponent here, so we can just return at this point
   *decimal_pos = dec_exp+1;// based on .xxx not x.xxx
   *start = out;
   return ng;  
}

#else
// It uses basically the same algorithm as the LD & float128 versions below, but using double-double and a uint64_t for the mantissa (the others use a u2_64 for the mantissa).
// The conversion method is based on Wirth book "Algorithms+Data Structures=Programs" pp 47-49, but with a different method for rounding and using double-doubles for accuracy.
// This version is ~ the same speed as the previous "long double" version (but that now gives 3 errors with the test program), and significantly faster than the previous double-double version (which also now gives errors with the test program).
      
static bool ya_s__DD_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, double v, uint32_t frac_digits)
{
 int32_t tens,d,digits;
#if 1 /* if 1 assume ieee format double (basically same "front end" as used above), if 0 use portable C code which is slighly slower but gives identical results */
 union {
    double real_d;
    uint64_t bits_d;
  } u = { v };
    // Decode bits into sign, mantissa, and exponent.
 const bool ng = (u.bits_d  & ((uint64_t)1<<63)) != 0;// true if v is negative 1<<63 as sign bit is the msb 
 uint64_t mantissa = u.bits_d & 0xFFFFFFFFFFFFFULL;
 int32_t expo=(int)((u.bits_d>>52) & 0x7ff) ; 
 if(expo==0x7ff)
 	{// nan or inf
 	 if(mantissa) // nan
 	 	{
 #ifdef _UCRT 	 	
		   if(u.bits_d==0xFFF8000000000000ULL) // test against bits_d as this value is actually -ve
 		 	{
 		 	 *start="nan(ind)"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 8;
     	 	}
		  else if( (mantissa & ((uint64_t)1 << 51)) == 0)
 		 	{
 		 	 *start="nan(snan)"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 9;
     	 	} 
 	 	 else 
 		 	{
 		 	 *start="nan"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 3;
     	 	}	 	 	 
 #else
	 	 *start="nan"; 
 	 	 *decimal_pos = YA_S__SPECIAL;
 	 	 *len = 3;	
 #endif 
#ifdef YA_SP_SIGNED_NANS
         return ng;
#else     	 
     	 return false;// nan is always positive
#endif   	 	 
 	 	}
 	 else 
		{*start="inf"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
     	 return ng;
		} 	 
 	}	 	
 if(expo==0 && mantissa==0)
 	{// zero is special as we cannot scale it into range : return 0 
     *decimal_pos = 1;
     *start = out;
     out[0] = '0';
     *len = 1;
     return ng;
 	}	

 expo-=1022; // remove exponet bias
 if(expo==-1022)
	 	{// fix for denormalised numbers
 #if 1
 		 expo+=12-__builtin_clzll(mantissa);// see https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html#index-_005f_005fbuiltin_005fclzll - this is normally much faster than the code below as the builtin is normally a single CPU instruction.	 
 #else	 	
	 	 while((mantissa & ((uint64_t)1<<51)) == 0) { mantissa<<=1;expo--;}
 #endif
	 	}
 if(ng)
   	  {v= -v;
   	  } 	
#else /* portable pure C code - slightly slower than above */
   int expo; // must be "int" as used with frexp()
   const bool ng =signbit(v);
	  
   if(isnan(v))	 
  		{
 #ifdef YA_SP_NAN_IND
 		 if(ya_is_indefinite_double(v)) // use "double" function here as this is only used for double
 		 	{
 		 	 *start="nan(ind)"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 8;
     	 	}
     	 else if(ya_is_snan_double(v))
 		 	{
 		 	 *start="nan(snan)"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 9;
     	 	}   
		else  
 		 	{
 		 	 *start="nan"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 3;
     	 	}			 
 #else  		
		 *start="nan"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
 #endif     	 
#ifdef YA_SP_SIGNED_NANS
         return ng;
#else     	 
     	 return false;// nan is always positive
#endif     	 
		}   	  
	else if(isinf(v))
		{*start="inf"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
     	 return ng;
		}		
  	else if(v==0.0 )
  		{
         *decimal_pos = 1;
         *start = out;
         out[0] = '0';
         *len = 1;
         return ng;
      }
   if(ng)
   	  {v= -v;
   	  }      
    // v is a normal number, now get exponent
 	frexp(v,&expo);// 0.5<=mant<1 
 #endif    

    // find the decimal exponent as well as the decimal bits of the value
    // log10 estimate 
    tens = expo-1; 
	tens=(tens < 0) ? (((tens * 78913) / 262144)-1) : (((tens * 78913) / 262144) ); //  78913/2^18=0.301029205322265625 (2^18= 262,144), log10(2)=0.30102999566398119521373889472449
	 /* accuracy results from above:
	  Accuracy check of power of 10's estimates:
	       0 (0.0%) would need tens+=2
	  3570882 (17.0%) would need tens+=1
	  17469165 (83.0%) are exact
	       0 (0.0%) would need tens-=1
	       0 (0.0%) would need tens-=2
	 */    

	double xh,xl; 
 	// want 1<=x<10  , note xl can be +/- but should always be very small compared to xh	  
  #define ya_M_SHFT 60 /* power of 2 we shift mantissa, 2^64/10=1.84e18  so use 2^60 = 1.15e18 */	  
	uint64_t m=0;// to avoid [Warning] 'm'/'d' may be used uninitialized errors below	
	  /* new power of 10 reduction method - uses u2_64 for 1st step then swaps to u64 when power of 10 has been found */
	expo=tens;// save a copy in case we need it for double-double digit extraction
	dd_mult_power10( &xh,&xl,v,0.0,-tens );// this is correct ~ 80% of the time
	u2_64 m_u2=d_to_u2_64(ldexp(xh,ya_M_SHFT )); // this should be exact
	if(xl<0.0)
	 	{
	 	 m_u2= usub_u2_64(m_u2,d_to_u2_64(ldexp(-xl,ya_M_SHFT )+0.6)); //  ~ 0.5 needed to avoid "part 2" errors, 0.6 reduces errors @14 sf from 16 to 15 [ still one error at 13sf]
	 	}
	 else 
	 	{ m_u2=uadd_u2_64(m_u2,d_to_u2_64(ldexp(xl, ya_M_SHFT )+0.5));/* +ve values are easy !  */	
		} 
	
	 // extract 1st digit of mantissa - want it between 1 and 9
	u2_64 d_u2=rshift_u2_64(m_u2,ya_M_SHFT);
	d=d_u2.lo;// expecting 0-100 - using u2_64 as just a single uint64_t would overflow at 16 (= 2^64/2^60)
	if(d>9 ) 
	 	{
 #ifdef MONITOR_POWER10_ACCURACY	   	
		 nos_10_wrong++;
 #endif		 
		 tens++;// this 2nd step is needed ~ 20% of the time, need to divide by 10 which we can do with m_u2 directly [ using the remainder for rounding is necessary to pass part 2, without this you get  ]
	 	 uint32_t rem=uremainder_u2_64_by_u32(m_u2,10); // remainder, for rounding 
	 	 m_u2=udiv_u2_64_by_u32(m_u2,10); // divide by 10
		 m=m_u2.lo;
		 if(rem>=5) m++;// rounding 
	 	 d=m>>ya_M_SHFT;
		} 
	else
		{
 #ifdef MONITOR_POWER10_ACCURACY	   	
		 nos_10_correct++;
 #endif		
	 	 m=m_u2.lo; // fits in a single uint64_t already (<=9), d already set correctly
		}
  	bool roundup=false; 
  	 // frac_digits = (frac_digits & 0x80000000) ? (frac_digits & 0x7ffffff) : (tens + frac_digits-1);
  	 digits =(int32_t) ((frac_digits & 0x80000000) ? (int32_t)(frac_digits & 0x7ffffff) : (int32_t)(tens + (int32_t)frac_digits)); // digits is signed
   	if(digits>18) digits=18; // 18 is a sensible limit (any other value gives more part 2 errors) - this must be limited - the "main code" will add extra zero's if the user asks for something "silly".	   	
   	char *s=out;
   	if(digits>=12 && digits<=15 ) /* digits==12 matches 13sf, 13 matches 14sf, so this is 13 to 16sf on test program  - note even for W32 the uint64_t method is faster than the double-double method */
	 	{// for these values of digits the (fast) integer solution sometimes fails to give the correct rounding, so use a floating point solution for those	(which is slightly slower but more accurate)
	 	 bool force0=false,force9=false;// used to deal with underflow and overflow as we are using floating point maths here which is not exact
	 	 // need to repeat part of powers of 10 reduction:
	 	 tens=expo;
		 d=xh;
		 if(d>9) 
		 	{tens++;// this 2nd step is needed ~ 20% of the time
		 	 double orig_xh=xh,orig_xl=xl; // just in case!
			 div_dd_dd( &xh,&xl,xh,xl,10.0,0 );; //  /10 
			 d=xh;
			 if(d==0)
			 	{tens--;// this is needed just a few times
				 xh=orig_xh; xl=orig_xl;
				 d=xh; // >9 is dealt with below
				}
			} 
		 if(d>9)
		 	{
			 // assume this means 9.999999
			 d=9;
		 	}		 	 
	 	 *s++=d+'0';// 1st significant digit 
  		 /* "hybrid" - use integer maths whenever possible, while still keeping double-double accuracy */
		 sub_dd_d(&xh,&xl,xh,xl,d); // x-=d - we have already extracted 1st digit
		 for(int i=1;i<=digits;++i)
		 	{if(!force0 && !force9)
				{/* we extract 8 or 4 or 2 digits at a time if we need that many digits, otherwise we generate 1 at a time, each "block" (of 8/4/2/1) is processed as an integer for speed */	
				 if(i+8<=digits) // digits is unsigned so cannot use digits-8
				 	{// get 8 digits. d is an int32_t which can hold upto 2.1e9 so will easily hold 8 digits (9.9999e+7)
				 	 mult_d_dd(&xh,&xl,100000000.0,xh,xl); // x*=100,000,000
				 	 d=xh;
				 	 sub_dd_d(&xh,&xl,xh,xl,d); 
					 if(d<0) force0=true;
					 if(d>99999999) force9=true;
					 // now process force0/9 and d
					 if(force0) {d=0;*s++='0';*s++='0';*s++='0';*s++='0';;*s++='0';*s++='0';*s++='0';*s++='0';}
			 		 else if(force9) {d=9;*s++='9';*s++='9';*s++='9';*s++='9';*s++='9';*s++='9';*s++='9';*s++='9';}
			 		 else { // create 8 digits	 		 
			 		 		uint8_t d1=d/1000000;// 1st 2 digits of the 4 (can only be 0-99)
			 		 		d%=1000000;
					  	    *s++=d1/10+'0';// next significant digit 
					  		*s++=d1%10+'0';// next significant digit 
							d1=d/10000;	
							d%=10000;
					  	    *s++=d1/10+'0';// next significant digit 
					  		*s++=d1%10+'0';// next significant digit 
							d1=d/100;	
					  	    *s++=d1/10+'0';// next significant digit 
					  		*s++=d1%10+'0';// next significant digit					  					 		 
			 		 		d1=d%100;// last 2 digits 
					  	    *s++=d1/10+'0';// next significant digit 
					  	    d1%=10; 
					  		*s++=d1+'0';// next significant digit 
							d=d1;// last digit needs to be in d in case needed for rounding						  	 	 
				 	 	  }
				 	 i+=7; // we have created an extra 7 digits (making 8 in total)
					}
				 else if(i+4<=digits) // digits is unsigned so cannot use digits-4
				 	{// get 4 digits
				 	 mult_d_dd(&xh,&xl,10000.0,xh,xl); // x*=10000
				 	 d=xh;
				 	 sub_dd_d(&xh,&xl,xh,xl,d); 
					 if(d<0) force0=true;
					 if(d>9999) force9=true;
					 // now process force0/9 and d
					 if(force0) {d=0;*s++='0';*s++='0';*s++='0';*s++='0';}
			 		 else if(force9) {d=9;*s++='9';*s++='9';*s++='9';*s++='9';}
			 		 else { uint8_t d1=d/100;// 1st 2 digits of the 4 (can only be 0-99)
					  	    *s++=d1/10+'0';// next significant digit 
					  		*s++=d1%10+'0';// next significant digit 	 		 
			 		 		d1=d%100;// last 2 digits of the 4
					  	    *s++=d1/10+'0';// next significant digit 
					  	    d1%=10; 
					  		*s++=d1+'0';// next significant digit 
							d=d1;// last digit needs to be in d in case needed for rounding				 	 
				 	 	  }
				 	 i+=3; // we have created an extra 3 digits
					}
				 else if(i<digits)
				 	{// get 2 digits
				 	 mult_d_dd(&xh,&xl,100.0,xh,xl); // x*=100
				 	 d=xh;// was xh+xl but with the double-doubles maths routines we known that xh+xl=xh
				 	 sub_dd_d(&xh,&xl,xh,xl,d); 
					 if(d<0) force0=true;
					 if(d>99) force9=true;
					 // now process force0/9 and d
					 if(force0) {d=0;*s++='0';*s++='0';}
			 		 else if(force9) {d=9;*s++='9';*s++='9';}
			 		 else { uint8_t d1=d;
					  	    *s++=d1/10+'0';// next significant digit 
					  	    d1%=10; 
					  		*s++=d1+'0';// next significant digit 
							d=d1;// last digit needs to be in d in case needed for rounding				 	 
				 	 	  }
				 	 ++i; // we have created an extra digit
					}
				 else		 
				 	{
			 	 	 //get 1 digit
				 	 mult_d_dd(&xh,&xl,10.0,xh,xl); // x*=10
				 	 d=xh;// was xh+xl but with the double-doubles maths routines we known that xh+xl=xh
				 	 sub_dd_d(&xh,&xl,xh,xl,d); // x-=d
					 if(d<0) force0=true;
					 if(d>9) force9=true;
					 if(force0) {d=0;*s++='0';}
			 		 else if(force9) {d=9;*s++='9';}
			 		 else *s++=d+'0';// next significant digit 
				 	}
			 	}
			 else
			 	{// deal with force0/9 - d is already correctly set
			 	 if(force0) *s++='0';
				 else *s++='9'; // must be force9
			 	}		 	
			} 
	 	  if(force9) roundup=true;
		  else if(!force0) // force0 is effectively rounding up already
		 	{
			 //xh-=d; // no need for double-double on last (rounding) digit)	 
			 if(xh>0.5) roundup=true; 
			 if(xh==0.5 && d&1) roundup=true; // round to even
		 	}					 	
	 	}
	 else
	 	{// from here on all maths is done using integers, so is exact (we therefore guarantee each digit is 0..9 and don't need to check this) 
		 *s++=d+'0';// 1st significant digit 
		 for(int i=1;i<=digits;++i)
		 	{m=(m-((uint64_t)d<<ya_M_SHFT))*10; 
			 d=m>> ya_M_SHFT; 
			 *s++=d+'0';// next significant digit 
			 // cannot do break below as that messes up new rounding code
			 // if(m==0) break; // no remainder - we don't need to generate lots of trailing zero's as the surrounding code will do that if necessary, lastd will =0 so rounding will work OK (it will not roundup).
			}  
 #ifndef DBL_DECIMAL_DIG
  #define DBL_DECIMAL_DIG __DBL_DECIMAL_DIG__	/* 17 */
 #endif				
		 int lastd=d;// might need last digit for rounding to even decision 
		 if(digits<0) lastd='0';// digit before needed
		 else
		 	{// now need to do rounding - create 1 more digit "d" for rounding
			 m=(m-((uint64_t)d<<ya_M_SHFT))*10;
			 d=m>> ya_M_SHFT; 
			}
		 if(d>5) roundup=true;
		 else if(d==5 && digits+1 > DBL_DECIMAL_DIG) roundup=true; // resolution limit - without this we get 24 more part 2 errors
		 else  // more complex rounding needed 	 
		 	{ 
			  if(d==5)
			  	{
			  	 // need to check that to a total of DBL_DECIMAL_DIG significant digits to get 0's (i.e. result is xxx50000000) 
		 		 // We have already created 1 more digit (the "5")
	 		 
		 		 bool non_zero=false;
				 for(int r=digits+1; r<=DBL_DECIMAL_DIG;++r)
				 	{// create next digit
				 	 m=(m-((uint64_t)d<<ya_M_SHFT))*10;
		 			 d=m>> ya_M_SHFT;
		 			 if(d>0)
		 			 	{non_zero=true;
		 			 	 break;
		 			 	}
		 			}
		 		 if(!non_zero)
				 	{// create next digit - this time its >=5 that we look for
				 	 m=(m-((uint64_t)d<<ya_M_SHFT))*10;
		 			 d=m>> ya_M_SHFT;
		 			 if(d>=5)
		 			 	{non_zero=true;
		 			 	}
		 			} 	
				 if(non_zero || (lastd & 1) ) roundup=true; // > 5 or exactly 5 (to DBL_DECIMAL_DIG) , round to even    
		 		}
		 	 else if(d==4  ) // xxxx4999999 would round up to 500000	
			  	{		
				 bool non_nine=false;
				 for(int r=digits+1; r<=DBL_DECIMAL_DIG;++r)
				 	{// create next digit
				 	 m=(m-((uint64_t)d<<ya_M_SHFT))*10;
		 			 d=m>> ya_M_SHFT;
		 			 if(d!=9)
		 			 	{non_nine=true;
		 			 	 break;
		 			 	}
		 			}
		 		 if(!non_nine)
				 	{// create next digit - this time its >=5 that we look for
				 	 m=(m-((uint64_t)d<<ya_M_SHFT))*10;
		 			 d=m>> ya_M_SHFT;
		 			 if(d<5)
		 			 	{non_nine=true;
		 			 	}
		 			} 	
				 if(!non_nine && (lastd & 1) ) roundup=true; // exactly 5 (to DBL_DECIMAL_DIG) , round to even	
				}			 
		 	}
		}   
	if(digits<-1)
		{*out='0';
		}
	else if(digits==-1)
		{
		 *out=roundup?'1':'0';// 0.5=>1 for %.0f
		 tens++;		
		}
	else if(roundup)
	 	{ // add 1 to lsd, propagate carries up as required (remembering to trap overflow)
		 char *p=s;
	 	 while(--p>=out)
	 	 	{
			 *p=*p+1; // add 1
	 	 	 if(*p<='9') break;
	 	 	 *p='0'; // 9->0 and a carry to next digit
	 	 	}
	 	 if(p<out && *out=='0')
	 	 	{ // overflow 9.9999 => 10.0000 , just set 1st char as 1 (rest are already 0) and adjust exponent
	 	 	 *out='1';
	 	 	 tens++;
	 	 	} 	
	 	}    
   *decimal_pos = tens+1;// based on .xxx not x.xxx
   *start = out;
   *len = s-out;
   return ng;
}
#endif /* YA_SP_RYU */

#if defined(YA_SP_SPRINTF_QF)  && defined(__SIZEOF_FLOAT128__) 
/* float128 output - like ya_s__real_to_str() but for float128 */
/* this version can give round-loop (with fast_strtof128() from atof.[ch]) exact results with >=35 sig figs - see f128_to_a.c for more details and a test program.
   It uses u2_64 variables to avoid the need for int128's 
*/
static bool ya_s__real128_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, __float128 value, uint32_t frac_digits)
{
   __float128 v;
   int32_t tens,d,digits;
   int expo;
   v = value;
   const bool ng=signbitq(v);
   if(ng)
   	  v= -v;
   if(isnanq(value))	 
  		{
		 *start="nan"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;	 
#if defined( YA_SP_SIGNED_NANS) || defined(YA_SP_SIGNED_NANS_F128)
         return ng;
#else     	 
     	 return false;// nan is always positive
#endif     	 
		}
	else if(isinfq(value))
		{*start="inf"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
     	 return ng;
		}
  	else if(v==0.0)
  		{
         *decimal_pos = 1;
         *start = out;
         out[0] = '0';
         *len = 1;
         return ng;
      }
    // d is a normal number, now get exponent
	frexpq(v,&expo);
	expo--; // adjust to range code below expects

    // find the decimal exponent as well as the decimal bits of the value
    // log10 estimate 
    tens = expo; 
	tens = (tens < 0) ? (((tens * 1233) / 4096)-1) : (((tens * 1233) / 4096) ); // this seems to give correct exponent for 9<=x<=1
	__float128 xh,xl; 
 	// want 1<=x<10  , note xl can be +/- but should always be very small compared to xh	 
#ifdef ya_M_SHFT
 #undef ya_M_SHFT
#endif	  
	#define ya_M_SHFT 124 /* power of 2 we shift mantissa, 1^128/10=3.4e37  so use 2^124 = 2.13e37 */	  
	u2_64 m={0,0};// to avoid compiler warning use use before setting.
	d=0;
	for(int i=0;i<20;++i) // for loop avoids an infinite loop - but should always quickly terminate
	 	{u2_64 ml;// signed as xl is also signed */
		 f128_mult_power10( &xh,&xl,v,0.0Q,-tens );
		 if(xh>11.0Q) {tens++; continue;}// avoid risk of overflow below
		 // extract 1st digit of mantissa - want it between 1 and 9
		 m=flt128_to_u2_64(ldexpq(xh, ya_M_SHFT )); // this should be exact

		 if(xl<0.0Q)
		 	{if((int32_t) ((frac_digits & 0x80000000) ? (int32_t)(frac_digits & 0x7ffffff) : (int32_t)(tens + (int32_t)frac_digits))<3)
		 	 	ml= flt128_to_u2_64(ldexpq(-xl, ya_M_SHFT )+0.4Q); // xl -ve, so result is positive. Constant must be >=0.2 [10 is still OK with test program] - used 0.4 . Digits <2,3,4 all OK - used <3
		 	 else
		 	 	ml= flt128_to_u2_64(ldexpq(-xl, ya_M_SHFT )+0.025Q); // xl -ve, so result is positive. Constant can be +0.08 to -0.03 gives 2 string differences at 34 sf in test program. Midpoint (0.025) used.
		 	 ml= not_u2_64(ml); // 2's comp = 1's comp+1
		 	 ml= uadd_u2_64(ml,u64_to_u2_64(0,1));
		 	}
		 else ml= flt128_to_u2_64(ldexpq(xl, ya_M_SHFT )+0.625Q); /* +ve values are easy ! Constant 0.6 to 0.65 gives 1 string difference at 34 sf in test program - Midpoint (0.625) used */	  
		 m=uadd_u2_64(m,ml) ; // m+=ml;	
		 
		 d=rshift_u2_64(m,ya_M_SHFT).lo ;// d=m>> ya_M_SHFT;
	 	 if(d==0) tens--;
	 	 else if(d>9) tens++;
	 	 else 
		  	{// found correct exponent so 1st digit is 1..9
			 break;
			}
		 if(i>3 &&d>9) break; // fix below [ should only need i>1 but lets be sure ]
	 	}
   bool roundup=false; 
   // frac_digits = (frac_digits & 0x80000000) ? (frac_digits & 0x7ffffff) : (tens + frac_digits-1);
   digits =(int32_t) ((frac_digits & 0x80000000) ? (int32_t)(frac_digits & 0x7ffffff) : (int32_t)(tens + (int32_t)frac_digits)); // digits is signed
   if(digits>40) digits=40; // sensible limit - this must be limited - the "main code" will add extra zero's if the user asks for something "silly".	   
	// from here on all maths is done using integers, so is exact (we therefore guarantee each digit is 0..9 and don't need to check this, apart from 1st digit which is checked below) 	  
   char *s=out;
   if(d>9)
	 	{// this never happens in the test program - left in "just in case"	
	 	 // solution here is different to int128 code below - that uses a divide which has not been implemented for u2_64's
		 // assume this means 10.00000 - exponent has already been adjusted above so we need 1.0000 here
		 *s++='1';
		 *s++='0'; // we only need one zero, the surrounding code will add more if required 
		 //for(int i=1;i<=frac_digits;++i) 
		 //	*s++='0';
	 	 // don't need to change tens as for loop above has already done that (before "break")	
	 	}
	 else
	 	{	
		 *s++=d+'0';// 1st significant digit 
		 for(int i=1;i<=digits;++i)
		 	{m=umul_u2_64_by_ten(usub_u2_64(m,lshift_u2_64(u64_to_u2_64(0,d),ya_M_SHFT))) ;//m=(m-((unsigned __int128)d<<ya_M_SHFT))*10;	 
			 d=rshift_u2_64(m,ya_M_SHFT).lo ;// d=m>> ya_M_SHFT;  
			 *s++=d+'0';// next significant digit 
			 // uncommenting the line below makes the test program slower - presumably because m==0 is very unlikely
			 //if((m.hi|m.lo)==0) break; // no remainder - we don't need to generate lots of trailing zero's as the surrounding code wil do that if necessary, lastd will =0 so rounding will work OK (it will not roundup).
			}  
		 int lastd=d;// might need last digit for rounding to even decision 
		 if(digits<0) lastd='0';// digit before needed
		 else
		 	{// now need to do rounding - create 1 more digit "d" for rounding
			 m=umul_u2_64_by_ten(usub_u2_64(m,lshift_u2_64(u64_to_u2_64(0,d),ya_M_SHFT))) ;//m=(m-((unsigned __int128)d<<ya_M_SHFT))*10;
			 d=rshift_u2_64(m,ya_M_SHFT).lo ;//d=m>> ya_M_SHFT; 
			}
		 if(d>5) roundup=true;
		 else
		 	{ 
			  if(d==5)
			  	{
				 m=umul_u2_64_by_ten(usub_u2_64(m,lshift_u2_64(u64_to_u2_64(0,d),ya_M_SHFT))) ;//m=(m-((unsigned __int128)d<<ya_M_SHFT))*10; // remainder
				 if((m.hi|m.lo)!=0) roundup=true; // >5
			     else if(lastd & 1) roundup=true; // exactly 5, round to even	     
		 		}
		 	}
		}   
	if(digits<-1)
		{*out='0';
		}
	else if(digits==-1)
		{
		 *out=roundup?'1':'0';// 0.5=>1 for %.0f
		 tens++;		
		}
	else if(roundup)
	 	{ // add 1 to lsd, propagate carries up as required (remembering to trap overflow)
		 char *p=s;
	 	 while(--p>=out)
	 	 	{
			 *p=*p+1; // add 1
	 	 	 if(*p<='9') break;
	 	 	 *p='0'; // 9->0 and a carry to next digit
	 	 	}
	 	 if(p<out && *out=='0')
	 	 	{ // overflow 9.9999 => 10.0000 , just set 1st char as 1 (rest are already 0) and adjust exponent
	 	 	 *out='1';
	 	 	 tens++;
	 	 	} 	
	 	}    
   *decimal_pos = tens+1;// based on .xxx not x.xxx
   *start = out;
   *len = s-out;
   return ng;
}

#endif // __SIZEOF_FLOAT128__

#if LDBL_MAX_10_EXP==4932 /* "true" long double */

static bool ya_s__LD_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, long double value, uint32_t frac_digits)
{
   long double v;
   int32_t tens,d,digits;
   int expo;
   v = value;
   const bool ng=signbit(v);
   if(ng)
   	  v= -v;
   if(isnan(value))	 
  		{
 #ifdef YA_SP_NAN_IND
 		 if(ya_is_indefinite_double((double)value)) // use "double" function here as this is only used for double
 		 	{
 		 	 *start="nan(ind)"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 8;
     	 	}
     	 else if(ya_is_snan_double(value))
 		 	{
 		 	 *start="nan(snan)"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 9;
     	 	}   
		else  
 		 	{
 		 	 *start="nan"; 
  	 	 	 *decimal_pos = YA_S__SPECIAL;
     	 	 *len = 3;
     	 	}			 
 #else  		
		 *start="nan"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
 #endif     	 
#ifdef YA_SP_SIGNED_NANS_LD
         return ng;
#else     	 
     	 return false;// nan is always positive
#endif     	 
		}   	  
	else if(isinf(value))
		{*start="inf"; 
  	 	 *decimal_pos = YA_S__SPECIAL;
     	 *len = 3;
     	 return ng;
		}
  	else if(v==0.0)
  		{
         *decimal_pos = 1;
         *start = out;
         out[0] = '0';
         *len = 1;
         return ng;
      }
    // d is a normal number, now get exponent
	frexpl(v,&expo);
	expo--; // adjust to range code below expects


    // find the decimal exponent as well as the decimal bits of the value
    // log10 estimate 
    tens = expo; 
	tens = (tens < 0) ? (((tens * 1233) / 4096)-1) : (((tens * 1233) / 4096) ); // this seems to give correct exponent for 9<=x<=1
	long double xh,xl; 
 	// want 1<=x<10  , note xl can be +/- but should always be very small compared to xh	
	#ifdef ya_M_SHFT
	 #undef ya_M_SHFT
	#endif		   
	#define ya_M_SHFT 124 /* power of 2 we shift mantissa, 1^128/10=3.4e37  so use 2^124 = 2.13e37 */	  
	u2_64 m={0,0};// to avoid used before set compiler warnings
	d=0;
	for(int i=0;i<20;++i) // for loop avoids an infinite loop - but should always quickly terminate
	 	{u2_64 ml;// signed as xl is also signed */
		 ldd_mult_power10( &xh,&xl,v,0.0L,-tens );		 
		 // extract 1st digit of mantissa - want it between 1 and 9
		 if(xh>11.0L) {tens++; continue;} // avoid risk of overflow below
		 m=ld_to_u2_64(ldexpl(xh, ya_M_SHFT )); // this should be exact
		 /* rounded - based on required digits - this works , passing the test program */ 
		 if(xl<0.0L)
		 	{
		 	 ml= ld_to_u2_64(ldexpl(-xl, ya_M_SHFT )+0.55L); // xl -ve, so result is positive. Constant can be +0.4 to +0.7   Midpoint (0.55) used.
		 	 ml= not_u2_64(ml); // 2's comp = 1's comp+1
		 	 ml= uadd_u2_64(ml,u64_to_u2_64(0,1));
		 	}
		 else 
		 	{ ml= ld_to_u2_64(ldexpl(xl, ya_M_SHFT )+1.0L);/* +ve values are easy ! any value >0.9 seems to work for test program (to at least 256) set to 1.0 */	
			} 
		 //my_printf(" digits=%d xh=%Lg xl=%Lg\n",digits,xh,xl);
		 //printf("before exponent removal m= 0X%016"PRIx64"%016"PRIx64" ml=0X%016"PRIx64"%016"PRIx64"\n",m.hi,m.lo,ml.hi,ml.lo);
		 m=uadd_u2_64(m,ml) ; // m+=ml;		 
		 d=rshift_u2_64(m,ya_M_SHFT).lo ;// d=m>> ya_M_SHFT;
	 	 if(d==0) tens--;
	 	 else if(d>9) tens++;
	 	 else 
		  	{// found correct exponent so 1st digit is 1..9
			 break;
			}
		 if(i>3 &&d>9) break; // fix below [ should only need i>1 but lets be sure ]
	 	}
   bool roundup=false; 
   // frac_digits = (frac_digits & 0x80000000) ? (frac_digits & 0x7ffffff) : (tens + frac_digits-1);
   digits =(int32_t) ((frac_digits & 0x80000000) ? (int32_t)(frac_digits & 0x7ffffff) : (int32_t)(tens + (int32_t)frac_digits)); // digits is signed
   if(digits>25) digits=25; // sensible limit - this must be limited - the "main code" will add extra zero's if the user asks for something "silly".	   
	// from here on all maths is done using integers, so is exact (we therefore guarantee each digit is 0..9 and don't need to check this, apart from 1st digit which is checked below) 	  
   char *s=out;
   if(d>9)
	 	{// this never happens in the test program - left in "just in case"	
		 // assume this means 10.00000 - exponent has already been adjusted above so we need 1.0000 here
		 *s++='1';
		 *s++='0'; // we only need one zero, the surrounding code will add more if required 
		 //for(int i=1;i<=frac_digits;++i) 
		 //	*s++='0';
	 	 // don't need to change tens as for loop above has already done that (before "break")	
	 	}
	 else
	 	{	
		 *s++=d+'0';// 1st significant digit 
		 for(int i=1;i<=digits;++i)
		 	{m=umul_u2_64_by_ten(usub_u2_64(m,lshift_u2_64(u64_to_u2_64(0,d),ya_M_SHFT))) ;//m=(m-((unsigned __int128)d<<ya_M_SHFT))*10;	 
			 d=rshift_u2_64(m,ya_M_SHFT).lo ;// d=m>> ya_M_SHFT;  
			 *s++=d+'0';// next significant digit 
			 // commenting out the next line (m==0) makes the test program slower (this is the opposite of the double and f128 cases where commenting it out makes the test program faster).
			 if((m.hi|m.lo)==0) break; // no remainder - we don't need to generate lots of trailing zero's as the surrounding code wil do that if necessary, lastd will =0 so rounding will work OK (it will not roundup).
			}  
		 int lastd=d;// might need last digit for rounding to even decision 
		 if(digits<0) lastd='0';// digit before needed
		 else
		 	{// now need to do rounding - create 1 more digit "d" for rounding
			 m=umul_u2_64_by_ten(usub_u2_64(m,lshift_u2_64(u64_to_u2_64(0,d),ya_M_SHFT))) ;//m=(m-((unsigned __int128)d<<ya_M_SHFT))*10;
			 d=rshift_u2_64(m,ya_M_SHFT).lo ;//d=m>> ya_M_SHFT; 
			}
		 if(d>5) roundup=true;
		 else
		 	{ 
			  if(d==5)
			  	{
				 m=umul_u2_64_by_ten(usub_u2_64(m,lshift_u2_64(u64_to_u2_64(0,d),ya_M_SHFT))) ;//m=(m-((unsigned __int128)d<<ya_M_SHFT))*10; // remainder
				 if((m.hi|m.lo)!=0) roundup=true; // >5
			     else if(lastd & 1) roundup=true; // exactly 5, round to even	     
		 		}
		 	}
		}   
	if(digits<-1)
		{*out='0';
		}
	else if(digits==-1)
		{
		 *out=roundup?'1':'0';// 0.5=>1 for %.0f
		 tens++;		
		}
	else if(roundup)
	 	{ // add 1 to lsd, propagate carries up as required (remembering to trap overflow)
		 char *p=s;
	 	 while(--p>=out)
	 	 	{
			 *p=*p+1; // add 1
	 	 	 if(*p<='9') break;
	 	 	 *p='0'; // 9->0 and a carry to next digit
	 	 	}
	 	 if(p<out && *out=='0')
	 	 	{ // overflow 9.9999 => 10.0000 , just set 1st char as 1 (rest are already 0) and adjust exponent
	 	 	 *out='1';
	 	 	 tens++;
	 	 	} 	
	 	}    
   *decimal_pos = tens+1;// based on .xxx not x.xxx
   *start = out;
   *len = s-out;
   return ng;
}
#else // LDBL_MAX_10_EXP!=4932 - assume double==long double
static bool ya_s__LD_to_str(char const **start, uint32_t *len, char *out, int32_t *decimal_pos, long double value, uint32_t frac_digits)
{
 return ya_s__DD_to_str(start,len, out, decimal_pos, (double) value,  frac_digits); // use double converter as double==long double
}
#endif // LDBL_MAX_10_EXP==4932 /* "true" long double */

#undef YA_S__SPECIAL

#endif // YA_SP_SPRINTF_NOFLOAT

// clean up


/* now restore gcc options to those set by the user */
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__clang__)
#pragma GCC pop_options
#endif

#endif // YA_SP_SPRINTF_IMPLEMENTATION

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020,2025 Peter Miller
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
