/* double-double.h
   ================
   
   double-double routines for basic doubles, long doubles (__float80) and __float128 types.
   
   Note TDM-GCC 9.2.2 fmaq() appears to be broken, see https://github.com/jmeubank/tdm-gcc/issues/14 
   
   This file Peter Miller 25/5/2020 but parts go back a long way.
   
   1v0 - 1st version of this file.
   1v1 - bracketed by __DOUBLE_DOUBLE_H. Added ddtoU64()

NOTE: long double (__float80) is NOT the same as __float128.
	  long double is Intel extended double which has a 64 bit mantissa [ 18 decimal digits] (vs 53 bit in standard double [ 15 decimal digits] )
	  __float128 is IEEE double extended which has a 113 bit mantissa (33 decimal digits)
	  double double functions below give ~ 30 digits accuracy (but without the extended exponent range of long double or __float128).
	  double double using f80 (long double) or float128 give ~ 36 and 66 significant digits respectively with a much wider exponent range (+4932, -4966)
	  
WARNING:
  The functions using long doubles have been much less heavily used/tested than versions using doubles and __float128.	  	  
*/   

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
*//*
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
 
#ifndef __DOUBLE_DOUBLE_H
#define __DOUBLE_DOUBLE_H

#ifdef __SIZEOF_INT128__ /* only allow if compiler supports __float128 & __int128 */
typedef __uint128_t uint128_t; // same format as stdint.h
typedef __int128_t int128_t;
typedef __float128 f128_t;
#endif
typedef long double f80_t; // same as __float80 

// double double functions.
void add_dd_dd(double *zh, double *zl,double xh, double xl,double yh, double yl);  // adds double double x and y to give double double "z"
void sub_dd_dd(double *zh, double *zl,double xh, double xl,double yh, double yl);  // subtracts double double y from x to give double double "z"
void mult_dd_dd( double *xh, double *xl,double ah, double al,double bh, double bl);  // multiplies double double a and b to give double double "x"
void mult_d_dd( double *xh, double *xl,double a,double bh, double bl);  // multiplies a and double double  b to give double double "x"
void dd_power(double *rh, double *rl,double x, unsigned int n);// raise x to nth power - return double double result , uses double double maths internally to minimise the error
void div_dd_dd( double *xh, double *xl,double ah, double al,double bh, double bl) ; // divides double double a by b to give double double "x"
void U64toDD(uint64_t u64,double *xh,double *xl); /* convert uint64 to double double */
uint64_t ddtoU64(double xh,double xl);  // convert double double to uint64, deal with case where x may be a very small negative number

#ifdef __SIZEOF_INT128__ /* only allow if compiler supports __float128 & __int128 */
// double double based on __float128
void f128_add_dd_dd(f128_t *zh, f128_t *zl,f128_t xh, f128_t xl,f128_t yh, f128_t yl);  // adds double double x and y to give double double "z"
void f128_sub_dd_dd(f128_t *zh, f128_t *zl,f128_t xh, f128_t xl,f128_t yh, f128_t yl);  // subtracts double double y from x to give double double "z"
void f128_mult_dd_dd( f128_t *xh, f128_t *xl,f128_t ah, f128_t al,f128_t bh, f128_t bl);  // multiplies double double a and b to give double double "x"
void f128_mult_d_dd( f128_t *xh, f128_t *xl,f128_t a,f128_t bh, f128_t bl) ; // multiplies a and double double  b to give double double "x"
void f128_dd_power(f128_t *rh, f128_t *rl,f128_t x, unsigned int n); // raise x to nth power - return double double result , uses double double maths internally to minimise the error
void f128_div_dd_dd( f128_t *xh, f128_t *xl,f128_t ah, f128_t al,f128_t bh, f128_t bl);  // divides double double a by b to give double double "x"
void U128toDD_f128(uint128_t u128,f128_t *xh,f128_t *xl); /* convert uint128 to double double f128 */
#endif
// double double based on long double (__float80)
void f80_add_dd_dd(f80_t *zh, f80_t *zl,f80_t xh, f80_t xl,f80_t yh, f80_t yl);  // adds double double x and y to give double double "z"
void f80_sub_dd_dd(f80_t *zh, f80_t *zl,f80_t xh, f80_t xl,f80_t yh, f80_t yl);  // subtracts double double y from x to give double double "z"
void f80_mult_dd_dd( f80_t *xh, f80_t *xl,f80_t ah, f80_t al,f80_t bh, f80_t bl);  // multiplies double double a and b to give double double "x"
void f80_mult_d_dd( f80_t *xh, f80_t *xl,f80_t a,f80_t bh, f80_t bl);  // multiplies a and double double  b to give double double "x"
void f80_dd_power(f80_t *rh, f80_t *rl,f80_t x, unsigned int n); // raise x to nth power - return double double result , uses double double maths internally to minimise the error
void f80_div_dd_dd( f80_t *xh, f80_t *xl,f80_t ah, f80_t al,f80_t bh, f80_t bl) ; // divides double double a by b to give double double "x"
#ifdef __SIZEOF_INT128__ /* only allow if compiler supports __float128 & __int128 */
void U128toDD_f80(uint128_t u128,f80_t *xh,f80_t *xl); /* convert uint128 to double double f128 */
#endif
#endif // ifndef __DOUBLE_DOUBLE_H
