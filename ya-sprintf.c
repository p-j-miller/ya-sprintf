/* ya-sprintf.c - use ya_sprintf.h 

 This is a C file provided as an example of how to include the header file.
 If creates a standalone ya-sprintf.o which can be linked with the other files in the project.
 Any file that uses any of the ya-sprintf functions must have the following lines at its start:
 
  #define YA_SP_SPRINTF_DEFAULT // only necessary if you wish to replace the "built-in" versions of printf, sprintf, fprintf, etc
  #include "ya_sprintf.h"
 
 see example.c for a very simple example using this file.
 
 Created 3/7/2020
 
 Peter Miller
 */
 /*----------------------------------------------------------------------------
 *
 * MIT License:
 *
 * Copyright (c) 2020 Peter Miller
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *--------------------------------------------------------------------------*/

#if 1 
#include <quadmath.h> /* see https://gcc.gnu.org/onlinedocs/libquadmath/quadmath_005fsnprintf.html#quadmath_005fsnprintf - also needs quadmath library linking in */
#include <inttypes.h> /* to print uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> /* for bool */
#include <ctype.h>
#include <float.h>
#include <time.h>
#include <math.h>
#include <stdint.h>  /* for int64_t etc */
#include <sys/types.h> 

#include <limits.h>
#endif

#include "../nan_type/nan_type.h"
 // warning _UCRT is not defined before this point in the file! 
#ifdef __MINGW32__
 #include <_mingw.h> /* this defines _UCRT when its needed */
#endif
#if defined(WANT_MINGW_ANSI_STDIO) || (!defined(_UCRT) && defined(__MINGW32__) )
 #ifdef __USE_MINGW_ANSI_STDIO
  #undef __USE_MINGW_ANSI_STDIO
 #endif
 #define __USE_MINGW_ANSI_STDIO 1 /* must turn this on if using the mingw runtime */
 #define WANT_MINGW_ANSI_STDIO /* my way to indice this as stdio.h can override __USE_MINGW_ANSI_STDIO */
#else
 #ifdef __USE_MINGW_ANSI_STDIO
  #undef __USE_MINGW_ANSI_STDIO
 #endif
 #define __USE_MINGW_ANSI_STDIO 0 /* if on by default turn off */
#endif 

#if  defined(__SIZEOF_INT128__) && (!defined(__BORLANDC__) || (defined(__BORLANDC__) && defined(_UCRT)) )  /*C++Builder 12.1 defines __SIZEOF_INT128__ for both 64 bit versions of the compiler, but "plain" 64 bit compiler fails with link time errors if they are used, 64-bit (Modern) Compiler is OK */
 #define YA_SP_SPRINTF_QI  /* allows printing __int128's in ya_sprintf() via %Qd %I128d etc */
#endif
#if defined(__SIZEOF_FLOAT128__)   && !defined(__BORLANDC__)      /* Builder C++ Community version 12.1 patch 1 defines __SIZEOF_FLOAT128__ for 64 bit(Modern) compiler but __float128's cannot be used in sensible programs due to link time errors if they are used */
 #define YA_SP_SPRINTF_QF  /* allows printing __float128's in ya_sprintf() via %Qg etc , does not need __int128 */
#endif


#define YA_SP_SPRINTF_DEFAULT // only necessary if you wish to replace the "built-in" versions of printf, sprintf, fprintf, etc
#define YA_SP_SPRINTF_IMPLEMENTATION /* This tells the compiler to compile the code in the header file ya_sprintf.h */

 #if defined(_UCRT) || ( (defined(__GNUC__) && __GNUC__>=13) && defined(_WIN32))
  #define YA_SP_WCHAR_PR_CHARS /* needed for all Windows implementations precision for wide strings is in characters [not bytes] */
  #ifdef WANT_MINGW_ANSI_STDIO
   // #define YA_SP_LINUX_STYLE /* tell ya_printf() to print to match Linux gcc libc */
   #define YA_SP_NO_NEG_LEADINGPLUS // if defined ignore %+ for unsigned conversions
   // #define YA_SP_NO_NEG_LEADINGSPACE // if defined ignore %  (% space) for unsigned conversions
   // #define YA_SP_FULL_NULL // if defined only print (null) if it will be fully visible
   //#define YA_SP_A_FMT_ALT1 // if defined print %A in an alternative way
   #define YA_SP_A_FMT_ALT2 // if defined print %A in another alternative way   
   // #define YA_SP_PTR_0X // if defined print pointers with a leading 0X  
   #ifndef _UCRT
     #define YA_SP_SIGNED_NANS_F128 // if defined print F128 NAN's as signed numbers. Default is that a NAN is considered unsigned. 
   #endif 
  #else
   /* just using UCRT and not MINGW_ANSI_STDIO, note we use my_printf as long double format used by gcc-mingw is different to what the UCRT uses [ UCRT assumes long double=double]*/ 
   // #undef YA_SP_SPRINTF_Q  // this now works OK
   #define YA_SP_NO_NEG_LEADINGPLUS // if defined ignore %+ for unsigned conversions
   #define YA_SP_NO_NEG_LEADINGSPACE // if defined ignore %  (% space) for unsigned conversions
   // #define YA_SP_FULL_NULL // if defined only print (null) if it will be fully visible
   #define YA_SP_A_FMT_ALT4 // if defined print %A in an alternative way
   //#define YA_SP_A_FMT_ALT2 // if defined print %A in another alternative way   
   //#define YA_SP_A_FMT_ALT3 // if defined print %A in an alternative way (no trailing zero suppression)
   #define YA_SP_SIGNED_NANS // if defined print NAN's as signed numbers. Default is that a NAN is considered unsigned. 
   #define YA_SP_NAN_IND // if defined print nan(ind) rather than just nan “IND” for “indeterminate” 
   #define YA_SP_PTR_CAPS // %p in uppercase hex chars
   #define YA_SP_PTR_LEADINGZEROS // if defined always print pointers with leading zeros
   /* what we need is:
   		type		string printed (signed)
   		Quiet NaN 	nan
		Signaling NaN 	nan(snan)
		Indefinite NaN 	nan(ind)
	uses code in #include "nan_type.h" to define these
   */
   // #define YA_SP_PTR_0X // if defined print pointers with a leading 0X   
  #endif
 #endif
 #if !defined(_UCRT) && defined(__GNUC__) && __GNUC__>=13 && !defined(WANT_MINGW_ANSI_STDIO) && defined(_WIN32)
  /* there are LOTS of differences in this case as msvcrt is not very standards conforming ! - its recommended that MINGW_ANSI_STDIO is used instead */
  #define YA_SP_SPRINTF_EXP3 /* need 3 digit exponents if using msvcrt */
  #define YA_SP_NO_NEG_LEADINGSPACE // if defined ignore %  (% space) for unsigned conversions
 #endif

// #define YA_SP_RYU /* if defined use RYU in ya_sprintf() for doubles */

#include "ya_sprintf.h"  /* because of the define on the line above, this includes code - not just header  */


