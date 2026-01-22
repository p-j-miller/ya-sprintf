# ya-sprintf
A fast replacement for the sprintf family of functions (printf, fprintf, sprintf, etc).

This started with the code from  stb_sprintf (v1.08) which is a public domain snprintf() implementation (http://github.com/nothings/stb)
which itself was originally written by Jeff Roberts / RAD Game Tools, 2015/10/20.
This version is dual licensed (MIT and Public Domain) - but note some of the required parts in other repositories and files in the ryu directory have different licences. 

"ya-sprintf" stands for Yet Another sprintf.

It now provides an almost full C23 printf family implementation including wide characters/strings.
ya_sprintf also provides a number of extensions, the most significant is the ability to print 128 bit integers (__int128)
and 128 bit floating point numbers (__float128) for compilers which support thse types (most modern compilers do for 64 bit targets).
Its thread safe (with no need for any locking), does no dynamic memory allocation and is much faster than the built in "printf's" in MinGW/TDM-GCC.
It also fixes all the bugs I'm aware of in the MinGW/TDM-GCC implementations, these bugs and the slow speed are the main reason this exists. 
In comparison to stb_sprintf (v1.08) it fixes all the bugs I found, implements the C23 formats and adds long doubles, 128 bit ints and 128 bit floats.
Finally, it provides a consistent format string specification across different targets which aids portability between targets and compilers.

This version optionally allows the Ryu algorithm to be used to print doubles. Ryu is believed to be the fastest algorithm for this function, but it does add ~ 100kB to the size of the resultant executable (and its licensing is a little different) so is a compile time option via a #define (see Options section below).

Timing (all on the same PC with gcc 15.2.0 and Windows 11 25H2 with Intel i3-10100 CPU @ 3.6GHz ):
~~~
For doubles via snprintf("%.*e") average time per test for precisions from 0 to 21 over a wide range of values
 On Windows 11 (using the Winlibs 15.2.0 gcc compiler - see below)
 W64 MSVCRT/mingw	:3395.3ns   baseline
 W64 UCRT/Winlibs  	: 445.9ns	7.6* faster
 W64 ya_sprintf		: 219.9ns	15.4* faster, *2 vs UCRT
 W64 ya_sprintf/ryu : 136.5ns	24.9* faster, *3.3 vs UCRT ( *1.6 vs standard ya_sprintf)
 
 W32 MSVCRT/mingw	:3859.7ns   baseline
 W32 UCRT/Winlibs  	: 834.4ns	4.6* faster
 W32 ya_sprintf		: 262.6ns	14.7* faster, *3.2 vs UCRT
 W32 ya_sprintf/ryu : 215.4ns	17.9* faster, *3.9 vs UCRT ( *1.2 vs standard ya_sprintf)

 Timing with Ubuntu Linux ( Ubuntu 24.04.3 LTS (GNU/Linux 6.6.87.2-microsoft-standard-WSL2 x86_64)) using gcc 13.3.0 :
 X64 glibc 2.39		: 552.5ns	baseline
 X64 ya_sprintf		: 190.7ns	2.9* faster
 X64 ya_sprintf/ryu	: 142.3ns	3.9* faster (1.3* faster than standard ya_sprintf)
 
 X32 glibc 2.39		: 723.6ns	baseline
 X32 ya_sprintf		: 315.2ns	2.3* faster
 X32 ya_sprintf/ryu	: 212.6ns	3.4* faster (1.5* faster than standard ya_sprintf) 
~~~

It assumes a compiler that supports at least C99, it assumes chars are 8 bit ascii, int's are 32 bits, long can be 32/64 bits and long long int is 64 bits. Pointers can be 32 or 64 bits.
Floating point numbers are assumed to be  IEEE 754 format, with double as 64 bits, long double (which can be identical to doubles, or a type with more bits) and __float128's being 128 bits all of which are used.
Integers are assumed to be stored in 2's complement representation.
These assumptions are true for almost all processors manufactured in the last 10+ years (Intel X32 & X64, ARM, PowerPC, etc).

The following functions from https://github.com/p-j-miller?tab=repositories are also required (they were split out to make maintenance simpler as they are also used elsewhere):
* hr_timer (used only by the test program)
* nan_type
* my_printf (used only by the test program)
* power10
* atof (used only by the test program) - in directory atof-and-ftoa
* u2_64 - in directory u2_64-128bits-with-two-u64
* double-double
* fma (optional - but may provide faster execution with __float128's)

# Installation 
Copy the files here to a directory ya-sprintf.

The overall "local" directory structure should look like:
~~~
06/01/2026  21:09    <DIR>          ya-sprintf
06/01/2026  21:26    <DIR>          hr_timer
07/01/2026  21:08    <DIR>          nan_type
08/01/2026  17:52    <DIR>          my_printf
08/01/2026  21:33    <DIR>          power10
09/01/2026  13:46    <DIR>          atof-and-ftoa
09/01/2026  17:39    <DIR>          u2_64-128bits-with-two-u64
12/01/2026  21:10    <DIR>          double-double
13/01/2026  12:07    <DIR>          fma
~~~

To compile the test program under Ubuntu Linux ( Ubuntu 24.04.3 LTS (GNU/Linux 6.6.87.2-microsoft-standard-WSL2 x86_64)), with the command executed from within the directory ya-sprintf:
~~~
gcc -m64 -Wall -Ofast -fexcess-precision=standard -I. -D_FORTIFY_SOURCE=1 main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ryu/s2d_fast_atof.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lquadmath -lm -o test
~~~
This should compile with no errors or warnings. The resultant file can be run with
~~~
./test
~~~
The expected output from this is given at the start of main.c, but it should finish with:
~~~
PART2: 1964008 sprintf tests completed, no errors found
Test program finished - a total of 60458998 tests executed
~~~
To compile under Windows using the Winlibs compiler https://winlibs.com/  (again executed within the ya-sprintf directory):
~~~
C:\winlibs\winlibs-x86_64-posix-seh-gcc-15.2.0-mingw-w64ucrt-13.0.0-r2\mingw64\bin\gcc -Wall -m64 -fexcess-precision=standard -Ofast  -std=gnu99 -I. main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ryu/s2d_fast_atof.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lquadmath -static -o test.exe
~~~
Again it should compile with no errors or warnings. This time the resultant file is run with:
~~~
test
~~~
It should give the same output.

These files have also been tested with Embarcadero C++ Builder 12.1 Community edition Version 29.0.52631.8427 - which is the latest version as of 1/2026. See  https://blogs.embarcadero.com/the-new-clang-toolchain-in-cbuilder-12-1/ & https://docwiki.embarcadero.com/RADStudio/Athens/en/Clang-enhanced_C%2B%2B_Compilers . Using the Windows 64-bit (Modern) target which is based on Clang 15 using the UCRT should give a clean compile and a small number of errors from the test program. These errors all appear to be errors in the Builder C++ versions of "printf" where the output is not correctly rounded to even in a small number of cases. For example:
~~~
Different: .125 (0.125:4593671619917905920) to 2 sg printf=>"1.3e-01" new=>"1.2e-01"
   to 20 sf printf=>"1.2500000000000000000e-01" new=>"1.2500000000000000000e-01"
~~~
Printf should round to even, so 0.125 to 2 significant places should be 0.12 (as the "5" is exact" and "2" is even). The output above says the printf supplied with the compiler gives 1.3e-01 (which is incorrect) and ya-sprintf ("new") is correct with 1.2e-01. Note the exponential format used here is specified by the test, printf could display this as "0.12" with a suitable format string. The test program finds 246 errors in its part 1 and 25 errors in its part 2. Note the test program just flags differences between the "printf" supplied with the compiler and ya-sprintf, in most cases (including the one shown here) it does not try and identify which is correct.

The test program also uses "atof" ( https://github.com/p-j-miller/atof ) to convert the string of characters produced by both printf and ya-sprint back to a number - this is call a "round the loop" test. When given enough digits (at least 17 for IEEE format 64 bit doubles) this process should result in exactly the same number as was originally provided to "printf". The results for C++ Builder 12.1 with the 64-bit (Modern) compiler are shown below, which show it passes all the "round the loop" tests:
~~~
Tested ya_sprintf() double-double round loop:
 0 errors when 21 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 20 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 19 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 18 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 17 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 907369 errors when 16 sf string converted back to a double (686113 are 1 bit) (sprintf gives 907369 differences)
 Differences in string compares between built in "libc" sprintf() and tested sprintf() are:
  1 significant figures found 0 differences
  2 significant figures found 1 differences
  3 significant figures found 0 differences
  4 significant figures found 1 differences
  5 significant figures found 0 differences
  6 significant figures found 0 differences
  7 significant figures found 0 differences
  8 significant figures found 0 differences
  9 significant figures found 0 differences
 10 significant figures found 0 differences
 11 significant figures found 0 differences
 12 significant figures found 1 differences
 13 significant figures found 3 differences
 14 significant figures found 37 differences
 15 significant figures found 203 differences
 16 significant figures found 601 differences
 17 significant figures found 751 differences
 18 significant figures found 635 differences
 19 significant figures found 712 differences
 Should get 0 round the loop errors for >=17 sig fig, and 0 differences on string compares at <= 15 sig figs :246 errors found
~~~

Using the C++ Builder 12.1 CE Windows 64-bit or Windows 32 bit targets (which are based on Clang 5) will give a few minor warnings on compilation (which can be ignored), and then a lot of errors when run which are either errors or omissions in the functionality of the Builder C++ versions of "printf". In these cases ya-sprintf provides a modern replacement for the versions supplied by Embarcadero.
# Functionality
~~~
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

For the e, E, f, F, g, and G conversion specifiers, if the number of significant decimal digits is >= DBL_DECIMAL_DIG (for doubles, or LDBL_DECIMAL_DIG or FLT128_DECIMAL_DIG for long doubles and float128's respectively) then the result will be "round loop exact".
This means that the value output from ya_sprintf if fed to fast_strtod()/fast_strtold()/fast_strtof128() will give a bitwise identical value to the original double/ long double/_float128.
For significant digits <= FLT128_DIG,LDBL_DIG or DBL_DIG for _float128, long double and double respectively then ya_sprintf will normally print exactly the same characters as the "system" sprintf - however this is not guaranteed, 
but this and the round the loop test above is the basis of the supplied test program (the test program cannot test every possible value in a sane amount of time hence the lack of a guarantee).

%a/%A will always print an exact representation of float, double, long double and __float128.

For Windows, the wide character conversion specifiers (%C/%lc/%S/%ls) have only been tested on utf-8 strings when using the UCRT (the msvcrt does not support utf-8).

~~~
# Functions provided
If #define YA_SP_SPRINTF_DEFAULT  is present before the #include "ya-sprintf.h", then the functions will be defined as printf(), sprintf() etc (in which case they will effectively replace those provided by the compiler).
Default names are used here:
~~~
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
~~~
# Options
ya_sprintf is very configurable at compile time via #define statements, mainly to allow it to exactly emulate the output from a particular compiler (this is how the test program works).

Note YA_SP_SPRINTF_LD  is not used in this version - long doubles are always supported.

~~~
#define YA_SP_SPRINTF_STATIC // make the definitions of the exported functions static
#define YA_SP_SPRINTF_MIN XXX // XXX is the number of characters per callback , default 512 (see ya_s_vsprintfcb() above)
#define STB_SPRINTF_IMPLEMENTATION // for backwards compatibility with stb_sprintf()
#define YA_SP_SPRINTF_DECORATE PREFIX // define the names of the exported functions as PREFIXname , default ya_s_. If this is not defined then vsprintf, vsnprintf, sprintf, snprintf, vfprintf, vprintf, fprintf & printf are defined via macros to equal the ya_s_ versions.
#define YA_SP_SPRINTF_IMPLEMENTATION // actually include code from header file (see ya-sprintf.c for an example)
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

~~~
# Algorithms to print floating point numbers
By default (YA_SP_RYU not defined) ya_sprintf uses a new algorithm developed by the author based on a combination of the algorithm in Niklaus Wirth's book "Algorithms+Data Structures=Programs" (1976) pp 47-49, and Errol ("Printing Floating-Point Numbers, An Always Correct Method", Marc Andrysco, Ranjit Jhala & Sorin Lerner, 2016, https://dl.acm.org/doi/10.1145/2837614.2837654 ). It uses double-doubles for accuracy ( https://github.com/p-j-miller/double-double ) at least for extracting the decimal exponent, then it uses integers to convert the mantissa to decimal. It differs from Errol because that gives the smallest number of digits required to correctly represent a given number, but for printf the format specifies the number of significant digits to be displayed (which normally requires rounding the result). This algorithm is implemented in ya_s__DD_to_str(), ya_s__real128_to_str and ya_s__LD_to_str().

If Ryu selected then its only used for double conversions (i.e. in ya_s__DD_to_str()). Ryu is described in "Ryū revisited: printf floating point conversion", Ulf Adams, 2019, https://dl.acm.org/doi/10.1145/3360595 .
