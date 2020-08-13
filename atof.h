/* fast_atof by Peter Miller 23/1/2020
    See below for license information.
    
    This atof also returns a "good number" flag so its NOT a drop in replacement for the standard atof.
    
	 The code should be robust to anything thrown at it (lots of leading of trailing zeros, extremely large numbers of digits, etc ).
	 As well as floating point numbers this also accepts NAN and INF (case does not matter).
	 
	 As the name suggests its also written to be fast (much faster than the built in strtod or atof library functions at least for mingw64 ).
	 				  
 */ 
 /*----------------------------------------------------------------------------
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
 
#ifndef __ATOF_H
#define __ATOF_H
double fast_atof(const char *s,bool * not_number);
double fast_atof_nan(const char *s);// like fast_atof, but returns NAN if whole string is not a valid number
double fast_strtod(const char *s,char ** endptr);
float fast_strtof(const char *s,char **endptr); // if endptr != NULL returns 1st character thats not in the number

#ifdef __SIZEOF_INT128__ /* only allow if compiler supports __float128 & __int128 */ 
__float128 fast_strtof128(const char *s,char **endptr); // if endptr != NULL returns 1st character thats not in the number
#endif
#endif