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

#if 0 
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


#define YA_SP_SPRINTF_LD /* use long doubles in ya_sprintf() also allows printing %Lg etc to print long doubles */
#if  defined(__SIZEOF_INT128__) && defined(YA_SP_SPRINTF_LD) /* only allow YA_SP_SPRINTF_Q if compiler supports __float128 & __int128 */
#define YA_SP_SPRINTF_Q  /* allows printing __float128's in ya_sprintf() via %Qg etc, doing this requires quadmath library linking in ( -lquadmath ) */
#endif

#ifndef __MINGW32__ 
 /* assume Linux, adjust style of printed output to match the printf() family functions in gcc's libc */
 #define YA_SP_LINUX_STYLE 
 #define YA_SP_SIGNED_NANS 
#endif

#define YA_SP_SPRINTF_IMPLEMENTATION /* This tells the compiler to compile the code in the header file ya_sprintf.h */
#include "ya_sprintf.h"  /* because of the define on the line above, this includes code - not just header  */


