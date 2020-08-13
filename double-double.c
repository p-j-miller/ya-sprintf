/* double-double.c
   ================
   
   double-double routines for basic doubles, long doubles (__float80) and __float128 types.
   
   Note TDM-GCC 9.2.2 fmaq() appears to be broken, see https://github.com/jmeubank/tdm-gcc/issues/14
    A working  fmaq() can be found at https://fossies.org/linux/gcc/libquadmath/math/fmaq.c  
	which is described as Member "gcc-9.3.0/libquadmath/math/fmaq.c" (12 Mar 2020, 9888 Bytes) of package /linux/misc/gcc-9.3.0.tar.xz:
	The corresponding header file was at https://raw.githubusercontent.com/gcc-mirror/gcc/master/libquadmath/quadmath-imp.h and 
	https://github.com/gcc-mirror/gcc/blob/master/libquadmath/quadmath-imp.h
   
   This file Peter Miller 25/5/2020 but parts go back a long way.
   
   1v0 - 1st version of this file.
   1v1 - issue in f128_twosum() when both arguments are infinity fixed - I assume this effects other functions as well ?
   1v2 - issue in f128_mult_dd_dd() where it would return inf prematurely fixed

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
*/
 
#define __USE_MINGW_ANSI_STDIO 1 /* So mingw uses its printf not msvcrt */
/* the line below defines GCC_OPTIMIZE_AWARE when we can use # pragma GCC optimize ("-O2") */
#define GCC_OPTIMIZE_AWARE (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__clang__)

#define _FILE_OFFSET_BITS 64
#include <quadmath.h> /* see https://gcc.gnu.org/onlinedocs/libquadmath/quadmath_005fsnprintf.html#quadmath_005fsnprintf - also needs quadmath library linking in */
#include <inttypes.h> /* defines PRI64 etc */
#include <limits.h>
#include <float.h> /* for limits for float, double , long double */
#include <stdio.h>
#include <time.h>      /* for time_t */
#include <sys/types.h> /* for off_t */
#include <string.h>
#include <stdint.h> /* for int32_t etc */
#include <stdbool.h> /* for bool, true, false */
#include <ctype.h> /* isspace() etc */
#include <math.h> /* fma() etc */

#include "double-double.h"

#define nos_elements_in(x) (sizeof(x)/(sizeof(x[0]))) /* number of elements in x , max index is 1 less than this as we index 0... */



/* double and double double functions 
   See IEEE trans computers Vol 58 nos 7 July 2009 pp 994 "Accurate floating point product and exponentiation".
   The basic idea for double doubles is in Knuth "The art of computer programming - vol 2 Seminumerical algorithms" eg pp 237.
   see also the paper by Boldo and Melquiond at http://www.lri.fr/~melquion/doc/08-tc.pdf 
*/


/* code below cannot be compiled with -Ofast as this makes the compiler break some C rules that we need, so make sure of this here */
#if GCC_OPTIMIZE_AWARE
#pragma GCC push_options
#pragma GCC optimize ("-O3") /* cannot use Ofast */
#endif

static void fasttwosum(double *xh, double *xl,double a,double b)  // adds a and b to give double double "x". Requires |a| >= |b|
{double x,z;
 x=a+b;
 z=x-a;
 *xh=x;
 *xl=b-z;
}
  
static void twosum(double *xh, double *xl,double a,double b)  // adds a and b to give double double "x". 
{double x,y,z;
 x=a+b;
 z=x-a;
 y=(a-(x-z))+(b-z); 
 *xh=x;
 *xl=y;
}
 
static void twomult(double *xh, double *xl,double a,double b)  // multiplies  a and b to give double double "x"
{double x,y;
 x=a*b;
 y=fma(a,b,-x);
 *xh=x;
 *xl=y;
}

void add_dd_dd(double *zh, double *zl,double xh, double xl,double yh, double yl)  // adds double double x and y to give double double "z"
{double sh,sl,th,tl,vh,vl,c,w;
twosum(&sh,&sl,xh,yh);
twosum(&th,&tl,xl,yl);
c=sl+th;
fasttwosum(&vh,&vl,sh,c);
w=tl+vl;
fasttwosum(zh,zl,vh,w);	
}

void sub_dd_dd(double *zh, double *zl,double xh, double xl,double yh, double yl)  // subtracts double double y from x to give double double "z"
{
 add_dd_dd(zh,zl,xh,xl,-yh,-yl);
}

void mult_dd_dd( double *xh, double *xl,double ah, double al,double bh, double bl)  // multiplies double double a and b to give double double "x"
{double t1,t2,t3;
 twomult(&t1,&t2,ah,bh);
 t3=((ah*bl)+(al*bh))+t2;
 twosum(xh,xl,t1,t3);
}

void mult_d_dd( double *xh, double *xl,double a,double bh, double bl)  // multiplies a and double double  b to give double double "x"
{double t1,t2,t3;
 twomult(&t1,&t2,a,bh);
 t3=(a*bl)+t2;
 twosum(xh,xl,t1,t3);
}

void dd_power(double *rh, double *rl,double x, unsigned int n)// raise x to nth power - return double double result , uses double double maths internally to minimise the error
/* even though this requires an initial (integer) loop to find lt, it is faster than starting from lsb as that needs two calls to mult_dd_dd() rather than one mult_dd_dd() and one mult_d_dd() */
{int lt=n,t;
 double hi=1.0,lo=0.0;
 while( (t=lt&(lt-1)) != 0) lt=t; // find msb of n
 // printf("dd_power(%g,%u [0x%x]): lt=%u [0x%x]\n",x,n,n,lt,lt);
 for(t=lt;t>0;t>>=1)
 	{// 1 bit at a time from msb
 	 mult_dd_dd(&hi,&lo,hi,lo,hi,lo);// r=r*r;
 	 if(t&n) mult_d_dd(&hi,&lo,x,hi,lo);//r=r*x;
 	}
 *rh=hi;
 *rl=lo;
}

void div_dd_dd( double *xh, double *xl,double ah, double al,double bh, double bl)  // divides double double a by b to give double double "x"
{double hi;
 double ch,cl,uh,ul;
 ch=ah/bh; // initial estimate
 uh=ch*bh;
 ul=fma(ch,bh,-uh);
 cl=(((ah-uh)-ul)+al-ch*bl)/bh;
 hi=ch+cl;
 *xl=cl+(ch-hi);
 *xh=hi;
}

void U64toDD(uint64_t u64,double *xh,double *xl) /* convert uint64 to double double */
{uint64_t u64_l=u64 & UINT64_C(0xffffffff );// bottom 32 bits [ double has a 52 bit mantissa so 32 bits can fit exactlty  ]
 uint64_t u64_h=u64^u64_l;// upper 32 bits
 if(u64_h==0)
 	{*xh=u64; // <= 32 bits so will fit into one double
 	 *xl=0;
 	 return;
 	}
 twosum(xh,xl,u64_h,u64_l);// combine two halves of u64 to make the double double could probably be fasttwosum() but lets be sure	
}


uint64_t ddtoU64(double xh,double xl)  // convert double double to uint64, deal with case where x may be a very small negative number
{ 
  double t;
  uint64_t ob;
  if(xh<0.0)
  	{t=rint(xh+xl);
  	 if(t<0) return 0;
  	  return t;
  	}
  ob = (uint64_t)xh;	
  t = ( xh - (double)ob );
  t += xl;
  ob += (uint64_t)rint(t);
  return ob;
}

#ifdef __SIZEOF_INT128__ /* only allow if compiler supports __float128 & __int128 */
/* double double functions using flt128 so in theory give ~ 66 significant digits.

   
*/

/* following are predefined by gcc
__FLT128_MAX_10_EXP__ 4932
__FLT128_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966F128
__FLT128_MIN_EXP__ (-16381)
__FLT128_MIN_10_EXP__ (-4931)
__FLT128_MANT_DIG__ 113
__FLT128_HAS_INFINITY__ 1
__FLT128_MAX_EXP__ 16384
__FLT128_HAS_DENORM__ 1
__FLT128_DIG__ 33
__FLT128_MAX__ 1.18973149535723176508575932662800702e+4932F128
__SIZEOF_FLOAT128__ 16
__FLT128_MIN__ 3.36210314311209350626267781732175260e-4932F128
__FLT128_HAS_QUIET_NAN__ 1
__FLT128_EPSILON__ 1.92592994438723585305597794258492732e-34F128
__FLT128_DECIMAL_DIG__ 36


__SIZEOF_INT128__ 16

These should be referenced as :

	FLT128_MAX: largest finite number
	FLT128_MIN: smallest positive number with full precision
	FLT128_EPSILON: difference between 1 and the next larger representable number 
	FLT128_DENORM_MIN: smallest positive denormalized number
	FLT128_MANT_DIG: number of digits in the mantissa (bit precision)
	FLT128_MIN_EXP: maximal negative exponent
	FLT128_MAX_EXP: maximal positive exponent
	FLT128_DIG: number of decimal digits in the mantissa
	FLT128_MIN_10_EXP: maximal negative decimal exponent
	FLT128_MAX_10_EXP: maximal positive decimal exponent
	
*/





static void f128_fasttwosum(f128_t *xh, f128_t *xl,f128_t a,f128_t b)  // adds a and b to give double double "x". Requires |a| >= |b|
{f128_t x,z;
 x=a+b;
 z=x-a;
 *xh=x;
 *xl=b-z;
}
 

static void f128_twosum(f128_t *xh, f128_t *xl,f128_t a,f128_t b)  // adds a and b to give double double "x". 
{f128_t x,y,z;
 // need to check for infinity as otherwise this function will return nan when given inf and inf as arguments 
 if(isinfq(a))
 	{*xh=a;
 	 *xl=0;
 	 return;
 	}
 else if(isinfq(b))
 	{*xh=b;
	 *xl=0;
	 return;
	}
 x=a+b;
 z=x-a;
 y=(a-(x-z))+(b-z); 
 *xh=x;
 *xl=y;
} 


static void f128_twomult(f128_t *xh, f128_t *xl,f128_t a,f128_t b)  // multiplies  a and b to give double double "x"
{f128_t x,y;
 x=a*b;
 y=fmaq(a,b,-x);
 *xh=x;
 *xl=y;
}


 void f128_add_dd_dd(f128_t *zh, f128_t *zl,f128_t xh, f128_t xl,f128_t yh, f128_t yl)  // adds double double x and y to give double double "z"
{f128_t sh,sl,th,tl,vh,vl,c,w;
f128_twosum(&sh,&sl,xh,yh);
f128_twosum(&th,&tl,xl,yl);
c=sl+th;
f128_fasttwosum(&vh,&vl,sh,c);
w=tl+vl;
f128_fasttwosum(zh,zl,vh,w);	
}

 void f128_sub_dd_dd(f128_t *zh, f128_t *zl,f128_t xh, f128_t xl,f128_t yh, f128_t yl)  // subtracts double double y from x to give double double "z"
{
 f128_add_dd_dd(zh,zl,xh,xl,-yh,-yl);
}

void f128_mult_dd_dd( f128_t *xh, f128_t *xl,f128_t ah, f128_t al,f128_t bh, f128_t bl)  // multiplies double double a and b to give double double "x"
{f128_t t1,t2,t3;
 f128_twomult(&t1,&t2,ah,bh);
 t3=((ah*bl)+(al*bh))+t2;
 if(isinfq(t1) && isinfq(t3) && !isinfq(fmaq(ah,bh,((ah*bl)+(al*bh)))))
 	{ // trap incorrect overflow and return highest non infinite value [ can happen as ((ah*bl)+(al*bh)) can be negative ]
 	  // printf("\n\n f128_mult_dd_dd t1=inf, t3=inf but ah*bh != inf\n");
 	  *xh=FLT128_MAX;
 	  *xl=0;
 	  return;
 	}
 f128_twosum(xh,xl,t1,t3);
}

 void f128_mult_d_dd( f128_t *xh, f128_t *xl,f128_t a,f128_t bh, f128_t bl)  // multiplies a and double double  b to give double double "x"
{f128_t t1,t2,t3;
 f128_twomult(&t1,&t2,a,bh);
 t3=(a*bl)+t2;
 f128_twosum(xh,xl,t1,t3);
}

 void f128_dd_power(f128_t *rh, f128_t *rl,f128_t x, unsigned int n)// raise x to nth power - return double double result , uses double double maths internally to minimise the error
{int lt=n,t;
 f128_t hi=1.0F128,lo=0.0F128;
 while( (t=lt&(lt-1)) != 0) lt=t; // find msb of n
 // printf("dd_power(%g,%u [0x%x]): lt=%u [0x%x]\n",x,n,n,lt,lt);
 for(t=lt;t>0;t>>=1)
 	{// 1 bit at a time from msb
 	 f128_mult_dd_dd(&hi,&lo,hi,lo,hi,lo);// r=r*r;
 	 if(t&n) f128_mult_d_dd(&hi,&lo,x,hi,lo);//r=r*x;
 	}
 *rh=hi;
 *rl=lo;
}

 void f128_div_dd_dd( f128_t *xh, f128_t *xl,f128_t ah, f128_t al,f128_t bh, f128_t bl)  // divides double double a by b to give double double "x"
{f128_t hi;
 f128_t ch,cl,uh,ul;
 ch=ah/bh; // initial estimate
 uh=ch*bh;
 ul=fmaq(ch,bh,-uh);
 cl=(((ah-uh)-ul)+al-ch*bl)/bh;
 hi=ch+cl;
 *xl=cl+(ch-hi);
 *xh=hi;
}

#if 0 /* both options here give identical results with the current test program */
  void U128toDD_f128(uint128_t u128,f128_t *xh,f128_t *xl) /* convert uint128 to double double f128 */
{ int128_t r; // needs to be signed, cannot overflow as x takes most of the bits in u128 before assignment to r
 f128_t x=u128;
 r=u128-(uint128_t)x;
 *xh=x;
 *xl=r;
}
#else
  void U128toDD_f128(uint128_t u128,f128_t *xh,f128_t *xl) /* convert uint128 to double double f128 */
{uint128_t u128_l=u128 & UINT64_C(0xffffffffffffffff );// bottom 64 bits [ f128 has a 112 bit mantissa so 64 bits can fit exactlty  ]
 uint128_t u128_h=u128^u128_l;// upper 64 bits
 if(u128_h==0)
 	{*xh=u128; // <= 64 bits so will fit into one f128
 	 *xl=0;
 	 return;
 	}
 f128_twosum(xh,xl,u128_h,u128_l);// combine two halves of u128 to make the double double could probably be f128_fasttwosum() but lets be sure	
}
#endif
#endif

/* double double functions using long double so in theory give ~ 36 significant digits.  
*/

static void f80_fasttwosum(f80_t *xh, f80_t *xl,f80_t a,f80_t b)  // adds a and b to give double double "x". Requires |a| >= |b|
{f80_t x,z;
 x=a+b;
 z=x-a;
 *xh=x;
 *xl=b-z;
}
  
static void f80_twosum(f80_t *xh, f80_t *xl,f80_t a,f80_t b)  // adds a and b to give double double "x". 
{f80_t x,y,z;
 x=a+b;
 z=x-a;
 y=(a-(x-z))+(b-z); 
 *xh=x;
 *xl=y;
}
 

static void f80_twomult(f80_t *xh, f80_t *xl,f80_t a,f80_t b)  // multiplies  a and b to give double double "x"
{f80_t x,y;
 x=a*b;
#if 1  /* if 1 use fmaq() as we know that works OK */
 y=fmaq(a,b,-x);
#else 
 y=fmal(a,b,-x);
#endif 
 *xh=x;
 *xl=y;
}


 void f80_add_dd_dd(f80_t *zh, f80_t *zl,f80_t xh, f80_t xl,f80_t yh, f80_t yl)  // adds double double x and y to give double double "z"
{f80_t sh,sl,th,tl,vh,vl,c,w;
f80_twosum(&sh,&sl,xh,yh);
f80_twosum(&th,&tl,xl,yl);
c=sl+th;
f80_fasttwosum(&vh,&vl,sh,c);
w=tl+vl;
f80_fasttwosum(zh,zl,vh,w);	
}

 void f80_sub_dd_dd(f80_t *zh, f80_t *zl,f80_t xh, f80_t xl,f80_t yh, f80_t yl)  // subtracts double double y from x to give double double "z"
{
 f80_add_dd_dd(zh,zl,xh,xl,-yh,-yl);
}

 void f80_mult_dd_dd( f80_t *xh, f80_t *xl,f80_t ah, f80_t al,f80_t bh, f80_t bl)  // multiplies double double a and b to give double double "x"
{f80_t t1,t2,t3;
 f80_twomult(&t1,&t2,ah,bh);
 t3=((ah*bl)+(al*bh))+t2;
 f80_twosum(xh,xl,t1,t3);
}

 void f80_mult_d_dd( f80_t *xh, f80_t *xl,f80_t a,f80_t bh, f80_t bl)  // multiplies a and double double  b to give double double "x"
{f80_t t1,t2,t3;
 f80_twomult(&t1,&t2,a,bh);
 t3=(a*bl)+t2;
 f80_twosum(xh,xl,t1,t3);
}

 void f80_dd_power(f80_t *rh, f80_t *rl,f80_t x, unsigned int n)// raise x to nth power - return double double result , uses double double maths internally to minimise the error
{int lt=n,t;
 f80_t hi=1.0F128,lo=0.0F128;
 while( (t=lt&(lt-1)) != 0) lt=t; // find msb of n
 // printf("dd_power(%g,%u [0x%x]): lt=%u [0x%x]\n",x,n,n,lt,lt);
 for(t=lt;t>0;t>>=1)
 	{// 1 bit at a time from msb
 	 f80_mult_dd_dd(&hi,&lo,hi,lo,hi,lo);// r=r*r;
 	 if(t&n) f80_mult_d_dd(&hi,&lo,x,hi,lo);//r=r*x;
 	}
 *rh=hi;
 *rl=lo;
}

 void f80_div_dd_dd( f80_t *xh, f80_t *xl,f80_t ah, f80_t al,f80_t bh, f80_t bl)  // divides double double a by b to give double double "x"
{f80_t hi;
 f80_t ch,cl,uh,ul;
 ch=ah/bh; // initial estimate
 uh=ch*bh;
#if 1  /* if 1 use fmaq() as we know that works OK */
 ul=fmaq(ch,bh,-uh);
#else  
 ul=fmal(ch,bh,-uh);
#endif 
 cl=(((ah-uh)-ul)+al-ch*bl)/bh;
 hi=ch+cl;
 *xl=cl+(ch-hi);
 *xh=hi;
}

#ifdef __SIZEOF_INT128__ /* only allow if compiler supports __float128 & __int128 */
#if 0/* both options here give identical results with the current test program */
  void U128toDD_f80(uint128_t u128,f80_t *xh,f80_t *xl) /* convert uint128 to double double f80 */
{ int128_t r; // needs to be signed, cannot overflow as x takes most of the bits in u128 before assignment to r
 f80_t x=u128;
 r=u128-(uint128_t)x;
 f80_twosum(xh,xl,x,r);// combine two halves of u128 to make the double double
}
#else
  void U128toDD_f80(uint128_t u128,f80_t *xh,f80_t *xl) /* convert uint128 to double double f80 */
{uint128_t u128_l=u128 & UINT64_C(0xffffffffffffffff );// bottom 64 bits [ f80 has a 64 bit mantissa so 64 bits can fit exactly  ]
 uint128_t u128_h=u128^u128_l;// upper 64 bits
 if(u128_h==0)
 	{*xh=u128; // <= 64 bits so will fit into one f80
 	 *xl=0;
 	 return;
 	}
 f80_twosum(xh,xl,u128_h,u128_l);// combine two halves of u128 to make the double double
}
#endif
#endif

/* now restore gcc options to those set by the user */
#if GCC_OPTIMIZE_AWARE
#pragma GCC pop_options
#endif


