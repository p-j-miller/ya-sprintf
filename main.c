/* test code for ya_sprintf.c

   Written by Peter Miller 2/2020
   
 
		
WARNING - long doubles on WSL 1 are broken - see https://github.com/microsoft/WSL/issues/830 this is claimed to be fixed on WSL 2 (but that has other issues so this code has not been tested on that)...
fix (which is implemented here) is:
 #include <fpu_control.h>
...
   unsigned short Cw = 0x37f;
  _FPU_SETCW(Cw);					
					
Note -fsanitize=undefined -fsanitize-undefined-trap-on-error when used together does compile and execute OK with mingw	
	 -fsanitize=bounds 	-fsanitize-undefined-trap-on-error also works OK.		
	 without -fsanitize-undefined-trap-on-error linker complains it cannot find -lasan on mingw
Under ubuntu the -fsanatize works as expected (but also needs -g to add debugging info to executable).
	I use gcc -Wall -Ofast -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope -fstack-protector-all -g3  main.c atof.c double-double.c -lasan -lquadmath -lm -o test

also note :
#ifdef __cplusplus
#define C_STORAGECLASS “C”
#else
#define C_STORAGECLASS
#endif

#define COMPILEASSERT(x) extern C_STORAGECLASS char CompileAssert_##__FILE__##__LINE__[(x) ? 1 : -1]

is more flexible than the form I'm currently using.
	
*/
#define PART1_SPRINTF_TESTS /* if defined do detailed testing of double conversions (including "round loop" (converting double->string->double)  - these tests take ~ 2 minutes */
#define PART2_SPRINTF_TESTS /* if defined do testing of all formats and conversion types - these tests take ~ 2 seconds */
	/* define both PART1 and PART2 for full tests, just PART2 for a quick set of tests with reasonable coverage */
	
/* 

Expected output with both PART1 and PART2 defined on a 64 bit Mingw compiler with YA_SP_SPRINTF_LD  & YA_SP_SPRINTF_Q defined.

Starting PART1 sprintf tests:
 Starting random number checks:
  Just checked 2.35990565307286859e+198
  Just checked 1.40179260149423349e-21
  Just checked -1.86151169283721502e-09
  Just checked -7.54380975167362045e+279
  Just checked 2.6983308160981522e-13
  Just checked 3.43720789490223168e-94
  Just checked 3.33780187584268826e+217
  Just checked 4.74929891101584849e-298
  Just checked 1.11225457746452524e-275
  Just checked 1.51870616916189301e+102
  Just checked 3.82280303502400178e-70
  Just checked -1.03780049976893395e-303
  Just checked 3.49803461121632877e-226
  Just checked -8.69027873517310904e+248
  Just checked -7.77296701686667508e-14
  Just checked 1.7967365892391971e+106
  Just checked -7.4479380392845951e+58
  Just checked -1.25007957045730041e+180
  Just checked -4.23632488631213564e-203
  Just checked -7.7049343115635846e-16
 All double round loop tests completed in 131.658 secs
 51979798 tests 1278116 differences
 Tested ya_sprintf() double-double round loop:
 0 errors when 21 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 20 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 19 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 18 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 17 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 907378 errors when 16 sf string converted back to a double (686116 are 1 bit) (sprintf gives 907378 differences)
 Differences between built in "libc" sprintf() and tested sprintf() are:
  2 sf found 0 differences
  3 sf found 0 differences
  4 sf found 0 differences
  5 sf found 0 differences
  6 sf found 0 differences
  7 sf found 0 differences
  8 sf found 0 differences
  9 sf found 0 differences
 10 sf found 0 differences
 11 sf found 0 differences
 12 sf found 0 differences
 13 sf found 3 differences
 14 sf found 19 differences
 15 sf found 123 differences
 16 sf found 1273 differences
 17 sf found 12579 differences
 18 sf found 72956 differences
 19 sf found 283785 differences

 Now checking fast_strtof128():
 Results for fast_strtof128() tests:
  No 1 bit errors found
  No multiple bit errors found

Starting PART2 sprintf tests:
Constant strings:
printing %c:
printing %s:
printing %i:
printing %ld:
printing %lld:
printing %g:
printing %f:
Checking %p:
Now doing misc tests %b, %'d, %$d, %_$d, etc:
Now checking printing multiple items:
Now checking variable precision %*.* :
Now checking %n:
Checking round loop accuracy of %a and %A with fast_strtod():
Basic checks for %L and %Q suffixes
35146 sprintf tests completed on %L and %Q, no unexpected errors found ( 32 expected errors found)
Testing %QxXob [128 integers]:
Testing %Quid [128 integers to decimal]:
Now checking ya_printf():
This is produced by ya_printf!
 Test string PI~=3.14159 -1 in hex is 0xffffffff
 -1 (=-1) 340282366920938463463374607431768211455(= -1 as signed128) 3.40282e+38 (same as float128)
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000099 (998 zeros then 99)
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000099 (998 zeros then 99 to stderr)
PART2: All tests completed in 134.223 secs
PART2: 1960234 sprintf tests completed, no errors found


*/

// you will not normally need to touch the settings below
#if defined(__WIN64) || defined(__WIN32)
 #define USE_HR_TIMER /* define to use HR_TIMER to display execution times - only supported on windows */
#endif
#ifdef __MINGW32__ 
/* Define __USE_MINGW_ANSI_STDIO to 1 to use C99 compatible stdio functions on MinGW. see for example https://stackoverflow.com/questions/44382862/how-to-printf-a-size-t-without-warning-in-mingw-w64-gcc-7-1 */
#define __USE_MINGW_ANSI_STDIO 1 /* So mingw uses its printf not msvcrt */
#elif defined(__linux)
 // assume Linux
#ifdef __x86_64 
 #include <fpu_control.h>	/* needed for WSL 1 fix */
#endif 
 #define YA_SP_LINUX_STYLE /* tell ya_printf() to print to match Linux gcc libc */
 #define YA_SP_SIGNED_NANS /* tell ya_sprintf we want signed NAN's (to match linux gcc libc) */
#endif

// #define MINGW_SPRINTF /* if defined then test built in sprintf() - gives speed reference */ 
#define YA_SP_SPRINTF_IMPLEMENTATION /* if defined use this new sprintf.  Warning code is actually in header ya_sprintf.h */
#define Show1BitErrors  /* if defined show all 1 bit errors, if not defined then don't */
#define USE_FAST_STRTOD /* if defined use fast_strtod(), otherwise use system strtod() for round loop checks */
// #define PRINT_DIFFS /* if defined print all differences, otherwise only print major ones */
// #define YA_SP_NO_DIGITPAIR /* if defined avoid using a lookup table to convert binary to decimal [00..99] */
#define YA_SP_SPRINTF_LD /* use long doubles in ya_sprintf() also allows printing %Lg etc to print long doubles */
#if  defined(__SIZEOF_INT128__) && defined(YA_SP_SPRINTF_LD) /* only allow YA_SP_SPRINTF_Q if compiler supports __float128 & __int128 */
#define YA_SP_SPRINTF_Q  /* allows printing __float128's in ya_sprintf() via %Qg etc */
#endif
// #define PR_EXPECTED_ERRORS /* if defined with FULL_SPRINTF_TESTS & YA_SP_SPRINTF_Q  shows expected errors (not counted as "real" errors) */


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


/* the line below defines GCC_OPTIMIZE_AWARE to 1 when we can use # pragma GCC optimize ("-O2") */
#define GCC_OPTIMIZE_AWARE (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__clang__)
// define the level of gcc optimisations used as we cannot use Ofast as with gcc 9.3.0 on ubuntu this gives incorrect results around NAN's even in main test program
#if GCC_OPTIMIZE_AWARE
#pragma GCC optimize ("-O3") /* highest optimisation that works was -O3*/
#endif

#define nos_elements_in(x) (sizeof(x)/(sizeof(x[0]))) /* number of elements in x , max index is 1 less than this as we index 0... */
// #define _FILE_OFFSET_BITS 64 

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

#ifdef USE_HR_TIMER
#include "hr_timer.h"
#endif

enum fpout_type {round_even=1,round_nearest=2,notrailingzeros=4,fmt_g=8};// powers of 2 so can or them together. fmt_g automatically adds notrailingzeros

#ifdef YA_SP_SPRINTF_IMPLEMENTATION

 #include "ya_sprintf.h"  /* includes code - not just header  */

 	 /* add macro to convert
	   double_to_str_exp( x, i,round_even,sizeof(new_str), new_str);
	   to  
	   ya_s_snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 	 */ 
#define double_to_str_exp(x,i,rd,size,string) ya_s_snprintf((string),(size),(((rd)&fmt_g)?"%.*g":"%.*e"),(((rd)&fmt_g)?(i):(i-1)),(x))	
#endif

#ifdef MINGW_SPRINTF
 	 /* add macro to convert
	   double_to_str_exp( x, i,round_even,sizeof(new_str), new_str);
	   to  
	   snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 	 */ 
#define double_to_str_exp(x,i,rd,size,string) snprintf((string),(size),(((rd)&fmt_g)?"%.*g":"%.*e"),(((rd)&fmt_g)?(i):(i-1)),(x))	
#endif

#include "atof.h"
#ifndef USE_FAST_STRTOD
#define fast_strtod(s,endptr) strtod((s),(endptr)) /* use system strtod() */
#endif

#ifdef __SIZEOF_INT128__
typedef __uint128_t uint128_t; // same format as stdint.h
typedef __int128_t int128_t;
typedef __float128 f128_t;
#endif

 union _du { uint64_t u;
			  double d;
		} du; 
 union _fu { uint32_t u32;
			  float f;
		} fu;

/* a very fast 64 bit generator is at http://prng.di.unimi.it/xoshiro256plusplus.c
   This is xoshiro256++ 1.0, one of our all-purpose, rock-solid generators.
   It has excellent (sub-ns) speed, a state (256 bits) that is large
   enough for any parallel application, and it passes all tests we are
   aware of.

   For generating just floating-point numbers, xoshiro256+ is even faster.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. 
    This generator is public domain code.
*/
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}


static uint64_t s[4]= { UINT64_C(0x180ec6d33cfd0aba), UINT64_C(0xd5a61266f0c9392c), UINT64_C(0xa9582618e03fc9aa), UINT64_C(0x39abdc4529b1661c) }; // must be initialised to non-zero values.

uint64_t randu64(void) {
	const uint64_t result = rotl(s[0] + s[3], 23) + s[0];

	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	return result;
}

#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */
// functions to check 128bit int to decimal conversion functions in ya_sprintf
/* print to string which needs to be at least 40 chars long (39 digits plus trailing null).
   This function by Peter Miller 
   returns number of characters put into s (excluding final \0).
*/   
/*      UINT64_MAX 18446744073709551615ULL */
#define P10_UINT64 10000000000000000000ULL /* 19 zeroes */
#define E10_UINT64 19

#define STRINGIZER(x) # x
#define TO_STRING(x) STRINGIZER(x)

int sprint_uint128_decimal(char *s,uint128_t big) {
  size_t rc = 0,rl;
  size_t i = 0;
  if (big >> 64) 
   {
    char buf[40];
    while (big / P10_UINT64 ) 
	 {
      rc += sprintf(buf + E10_UINT64 * i, "%." TO_STRING(E10_UINT64) PRIu64, (uint64_t)(big % P10_UINT64));
      ++i;
      big /= P10_UINT64;
     }
	rl= sprintf(s, "%" PRIu64, (uint64_t)(big ));
	s+=rl;
	rc+=rl;   
    while (i--) 
		{/* created in reverse order, so copy out blocks of digits backwards */
		 for(int j=0;j<E10_UINT64;++j)
		 	{*s++= buf[ E10_UINT64 * i+j];
		 	}	
    	}
    *s=0; // ensure string null terminated    	
   } 
  else 
   {// can just print as u64 which is easy
    rc = sprintf(s,"%" PRIu64, (uint64_t)big);
   }
  return rc;
}

int sprint_int128_decimal(char *s,int128_t big) /* signed sprint, needs min 41 character buffer as potential leading minus sign */
{if(big<0)
	{*s++='-';
	 uint128_t b=(uint128_t)big;
	 b=~b+1; // same as b = -big wihout issue with MIN_INT
	 return 1+sprint_uint128_decimal(s,b); /* +1 for minus sign. Note -big will give the correct result for all values as its printed as unsigned, eg fpr 8 bits -128 (80) will stay -128 after "-" but will print as 128 unsigned which is what is needed */
	}
 else
 	{return sprint_uint128_decimal(s,big);
	}
}

void chk_fast_strtof128(void)  // tests for fast_strtof128(). These tests will detect a buggy fmaq() implementation
{ 	int errs=0,onebiterrs=0,onebiterrs40sf=0;
    f128_t r128;
	char buf128[128],buf128a[128];
	printf("\n Now checking fast_strtof128():\n"); // f128_t fast_strtof128(const char *s,char **endptr) // if endptr != NULL returns 1st character thats not in the number
	f128_t r128a;
	// test value for string to flt128 conversion. 1st row are entered as doubles so will be different to 2nd row where values are entered as F128 constants. 2nd row also has a wider range including denormalised numbers
	f128_t test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,-1e-307,-1e-308,-1e-315,0,4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,NAN,
		-FLT128_MAX,-1e+4932F128,-1e+4930F128, -1.7976931348623157e308F128 ,-12345678.F128 ,-10001.F128 ,-9999.F128 ,-1001.F128 ,-345.F128 ,-100.F128 ,-99.F128 ,-20.F128 ,-10.F128 ,-9.F128 ,-8.F128 ,-7.F128 ,-6.F128 ,-5.F128 ,-4.F128 ,-3.F128 ,-2.F128 ,-1.F128 ,-0.5F128 ,-0.1F128 ,-1e-307F128 ,-1e-308F128 ,-1e-315F128,-FLT128_MIN,-6.47517511943802511092443895822764655e-4956F128,-FLT128_DENORM_MIN ,0.F128,FLT128_DENORM_MIN,6.47517511943802511092443895822764655e-4956F128, 6.47517511943802511092443895822764655e-4946F128, 6.47517511943802511092443895822764655e-4936F128,  FLT128_MIN ,4.9406564584124654e-324F128 ,1e-315F128 ,2.2250738585072009e-308F128 ,2.2250738585072014e-308F128 ,0.1F128 ,0.5F128 ,1.F128 ,2.F128 ,3.F128 ,4.F128 ,5.F128 ,6.F128 ,7.F128 ,8.F128 ,9.F128 ,10.F128 ,20.F128 ,99.F128 ,100.F128,101.F128 ,345.F128 ,999.F128 ,1000.F128,1001.F128 ,1002.F128 ,1002.5F128 ,1002.6F128 ,10000.F128,10001.F128 ,100001.F128 ,100001.F128 ,1000001.F128 ,12345678.F128 ,1.7976931348623157e308F128,1e+4930F128,1e+4932F128,FLT128_MAX };
		
	for(int i=0;i<nos_elements_in(test_valued);++i)
		{r128=test_valued[i];
		// start with 34 sf
		quadmath_snprintf (buf128, sizeof buf128,"%.33Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.33Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{// printf("fast_strtof128(%s) bitwise equal [inf] at 34sf\n",buf128);
				}
			 else
			 	{	
			 	 // printf("fast_strtof128(%s) bitwise equal at 34sf\n",buf128);
			 	}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// printf("fast_strtof128(%s) character wise equal to 34 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			 printf("fast_strtof128(%s) 1 bit differnt  at 34 sf giving %s\n",buf128,buf128a);
#endif			 
			 ++onebiterrs;
			}		
		else
			{++errs; 
		 	 printf("Error: fast_strtof128(%s) gave %s at 34sf\n",buf128,buf128a);
			}
		// check with 35 sf
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.34Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.34Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{
			 if(isinfq(r128) || isinfq(r128a))
				{// printf("fast_strtof128(%s) bitwise equal [inf] at 35sf\n",buf128);
				}
			 else
			 	{
			     // printf("fast_strtof128(%s) bitwise equal at 35sf\n",buf128);
				}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// printf("fast_strtof128(%s) character wise equal to 35 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			 printf("fast_strtof128(%s) 1 bit differnt  at 35 sf giving %s\n",buf128,buf128a);
#endif			 
			 ++onebiterrs;
			}		
		else
			{++errs; 
		 	 printf("Error: fast_strtof128(%s) gave %s at 35sf\n",buf128,buf128a);
			}					
		// check with 36 sf	
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.35Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.35Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//printf("fast_strtof128(%s) bitwise equal [inf] at 36sf\n",buf128);
				}
			 else
			 	{	
			 	 // printf("fast_strtof128(%s) bitwise equal at 36 sf\n",buf128);
			 	}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// printf("fast_strtof128(%s) character wise equal to 36 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			printf("fast_strtof128(%s) 1 bit differnt  at 36 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			}		
		else
			{++errs; 
		 	 printf("Error: fast_strtof128(%s) gave %s at 36sf\n",buf128,buf128a);
			}	
		// check with 37 sf	
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.36Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.36Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//printf("fast_strtof128(%s) bitwise equal [inf] at 37sf\n",buf128);
				}
			 else
			 	{	
			 	 //printf("fast_strtof128(%s) bitwise equal at 37 sf\n",buf128);
			 	}
			}	
		else if(strcmp(buf128,buf128a)==0)
			{//printf("fast_strtof128(%s) character wise equal to 37 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			printf("fast_strtof128(%s) 1 bit differnt  at 37 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			}		
		else
			{++errs; 
			 printf("Error: fast_strtof128(%s) gave %s at 37 sf\n",buf128,buf128a);
			}		
		// check with 38 sf	
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.37Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.37Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//printf("fast_strtof128(%s) bitwise equal [inf] at 38sf\n",buf128);
				}
			 else
			 	{	
				 // printf("fast_strtof128(%s) bitwise equal at 38 sf\n",buf128);
				}
			}
		else if(strcmp(buf128,buf128a)==0)
			{//printf("fast_strtof128(%s) character wise equal to 38 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			printf("fast_strtof128(%s) 1 bit differnt  at 38 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			}	
		else
			{++errs; 
		 	 printf("Error: fast_strtof128(%s) gave %s at 38 sf\n",buf128,buf128a);
			}	
		// check with 39 sf	
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.38Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.38Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//printf("fast_strtof128(%s) bitwise equal [inf] at 39sf\n",buf128);
				}
			 else
			 	{	
				 // printf("fast_strtof128(%s) bitwise equal at 39 sf\n",buf128);
				}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// printf("fast_strtof128(%s) character wise equal to 39 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			printf("fast_strtof128(%s) 1 bit differnt  at 39 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			}	
		else
			{++errs; 
		 	 printf("Error: fast_strtof128(%s) gave %s at 39 sf\n",buf128,buf128a);
			}
		// check with 40 sf	(well over what would fit into a uint128 as 2^128=8.4e38)
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.39Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.39Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//printf("fast_strtof128(%s) bitwise equal [inf] at 40 sf\n",buf128);
				}
			 else
			 	{	
			 	 // printf("fast_strtof128(%s) bitwise equal at 40 sf\n",buf128);
			 	}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// printf("fast_strtof128(%s) character wise equal to 40 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			printf("fast_strtof128(%s) 1 bit differnt  at 40 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			 ++onebiterrs40sf; // record 40sf 1 bit errors especially
			}	
		else
			{++errs; 
		 	 printf("Error: fast_strtof128(%s) gave %s at 40 sf\n",buf128,buf128a);
			}			
		}
	printf(" Results for fast_strtof128() tests:\n");	
	if(onebiterrs) printf("  %d 1 bit errors found (%d at 40sf)\n",onebiterrs,onebiterrs40sf);
	else printf("  No 1 bit errors found\n");		
	if(errs) printf("  %d multi-bit errors found\n",errs);
	else printf("  No multiple bit errors found\n");
	printf("\n");
}

#endif


// macro that helps to define test cases
#define check_double(NUM) check_float_to_str( #NUM, (NUM) )


uint64_t errs=0,bit1=0,bit2=0,nos_tests=0;// 64 bits so don't overflow if we do a lot of tests
uint64_t errsf[20];
uint64_t errs_dbl21=0,errs_printf21=0,errs_dbl21_1bit=0;
uint64_t errs_dbl20=0,errs_printf20=0,errs_dbl20_1bit=0;
uint64_t errs_dbl19=0,errs_printf19=0,errs_dbl19_1bit=0;
uint64_t errs_dbl18=0,errs_printf18=0,errs_dbl18_1bit=0;
uint64_t errs_dbl17=0,errs_printf17=0,errs_dbl17_1bit=0;
uint64_t errs_dbl16=0,errs_printf16=0,errs_dbl16_1bit=0;


void check_float_to_str(char *in_str,double x)
{char printf_str[50];
 char new_str[50];
 double nx;
 int i;
 if(in_str==NULL) in_str=printf_str; // allow in_str to be null (if for example x is the result of a calculation rather than a constant) 
 if(isfinite(x) || isnan(x) || isinf(x)) // 1st was originally isnormal , now isfinite to include subnormals
  { // do now denormalised numbers here. we also check "round the loop" conversions below which also checks the results are OK for denormalised & normal numbers
   for(i=2;i<=19;++i)
 	{nos_tests++;
 	 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 	 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str);
 	 if(strcmp(printf_str,new_str)!=0)
 	 	{ // different
 	 	  errs++;
 	 	  errsf[i]++;
#ifndef PRINT_DIFFS 	 
		  if(i<13)	  // with standard Mingw runtime expect differences at 13 and above
#endif		 
			{du.d=x; 
 	 	     printf("Different: %s (%.19g:%" PRIu64 ") to %d sg printf=>\"%s\" new=>\"%s\"\n",in_str,x,du.u,i,printf_str,new_str);  
 	 	     snprintf(printf_str,sizeof(printf_str),"%.*e",18,x);	
 	         double_to_str_exp( x, 19,round_even,sizeof(new_str), new_str);
 	         printf("   to 19 sf printf=>\"%s\" new=>\"%s\"\n",printf_str,new_str);
 	         // printf("Press return to continue>");getchar(); 
 	 		}	
 	    }	 
 	 } 
   }

 i=21; // check 21 sf (my fast_strtod() uses 19sf and uses 20th for rounding)
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double
 if(nx!=x)
 	 	{ // different when 21 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl21++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl21_1bit++;		
#ifdef  Show1BitErrors
		  	 printf("Double 1 bit Different: %s (%.21g) to 21 sg new=>\"%s\" which as a double is %.21g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 printf("Double > 1 bit Different: %s (%.21g) to 21 sg new=>\"%s\" which as a double is %.21g\n",in_str,x,new_str,nx);
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if(nx!=x)
 	 	{ // printf different when 21 sig digits version converted back to double
 	 	  errs_printf21++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  printf("printf Double Different: %s (%.21g) to 21 sg printf=>\"%s\" which as a double is %.21g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    } 
 	
 i=20; // check 20 sf (my fast_strtod() uses 19sf and uses 20th for rounding)
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double
 if(nx!=x)
 	 	{ // different when 20 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl20++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl20_1bit++;	
#ifdef Show1BitErrors			  	  	
		  	 printf("Double 1 bit Different: %s (%.20g) to 20 sg new=>\"%s\" which as a double is %.20g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 printf("Double > 1 bit Different: %s (%.20g) to 20 sg new=>\"%s\" which as a double is %.20g\n",in_str,x,new_str,nx);
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if(nx!=x)
 	 	{ // printf different when 20 sig digits version converted back to double
 	 	  errs_printf20++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  printf("printf Double Different: %s (%.20g) to 20 sg printf=>\"%s\" which as a double is %.20g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    } 
 i=19; 
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double
 if(nx!=x)
 	 	{ // different when 19 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl19++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl19_1bit++;	
#ifdef Show1BitErrors			  	  	
		  	 printf("Double 1 bit Different: %s (%.19g) to 19 sg new=>\"%s\" which as a double is %.19g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 printf("Double > 1 bit Different: %s (%.19g) to 19 sg new=>\"%s\" which as a double is %.19g\n",in_str,x,new_str,nx);
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if(nx!=x)
 	 	{ // printf different when 19 sig digits version converted back to double
 	 	  errs_printf19++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  printf("printf Double Different: %s (%.19g) to 19 sg printf=>\"%s\" which as a double is %.19g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    } 

 // now repeat with 18 sig figs
 i=18;
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if(nx!=x)
 	 	{ // different when 18 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl18++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl18_1bit++;		  
#ifdef Show1BitErrors			  	
		  	 printf("Double 1 bit Different: %s (%.18g) to 18 sg new=>\"%s\" which as a double is %.18g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{ 	  		
			 printf("Double > 1 bit Different: %s (%.18g) to 18 sg new=>\"%s\" which as a double is %.18g\n",in_str,x,new_str,nx);		 
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if(nx!=x)
 	 	{ // printf different when 18 sig digits version converted back to double
 	 	  errs_printf18++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  printf("printf Double Different: %s (%.18g) to 18 sg printf=>\"%s\" which as a double is %.18g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    }
 // now repeat with 17 sig figs
 i=17;
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if(nx!=x)
 	 	{ // different when 17 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl17++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl17_1bit++;	 	
#ifdef Show1BitErrors			  	  	
		  	 printf("Double 1 bit Different: %s (%.17g) to 17 sg new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 printf("Double > 1 bit Different: %s (%.17g) to 17 sg new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if(nx!=x)
 	 	{ // printf different when 17 sig digits version converted back to double
 	 	  errs_printf17++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  printf("printf Double Different: %s (%.17g) to 17 sg printf=>\"%s\" which as a double is %.17g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    }
  // check other formats as 17 sf - with traling zero deleted
 nos_tests++; 
 double_to_str_exp( x, i,round_even|notrailingzeros,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if(nx!=x)
 	 	{ // different when 17 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl17++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl17_1bit++;	 
#ifdef Show1BitErrors			  		  	
		  	 printf("Double 1 bit Different: %s (%.17g) to 17 sg notrailingzeros new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 printf("Double > 1 bit Different: %s (%.17g) to 17 sg notrailingzeros new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);
			}
 	    }
 nos_tests++; 
 double_to_str_exp( x, i,round_even|fmt_g,sizeof(new_str), new_str); // "g" format
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if(nx!=x)
 	 	{ // different when 17 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl17++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl17_1bit++;	 		  
#ifdef Show1BitErrors			  	
		  	 printf("Double 1 bit Different: %s (%.17g) to 17 sg fmt_g new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 printf("Double > 1 bit Different: %s (%.17g) to 17 sg fmt_g new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);
			}
 	    }		   
 // now repeat with 16 sig figs
 i=16;
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if(nx!=x)
 	 	{ // different when 16 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl16++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl16_1bit++;
#if defined(PRINT_DIFFS) && defined(Show1BitErrors) 	 		  	
		  	 printf("Double 1 bit Different: %s (%.16g) to 16 sg new=>\"%s\" which as a double is %.16g\n",in_str,x,new_str,nx);
#endif		  	 
		  	}
 	 	  else
 	 	  	{
#ifdef PRINT_DIFFS 	 
			 printf("Double > 1 bit Different: %s (%.16g) to 16 sg new=>\"%s\" which as a double is %.16g\n",in_str,x,new_str,nx);
#endif			 
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if(nx!=x)
 	 	{ // printf different when 16 sig digits version converted back to double
 	 	  errs_printf16++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  printf("printf Double Different: %s (%.16g) to 16 sg printf=>\"%s\" which as a double is %.16g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    }
}


/* support functions for checking ya_s_sprintf() 
*/
unsigned int serrs=0,scnt=0;
unsigned int expected_errs=0;
/* simple strings */
void check_str_s(char *x)
{
 int r_ya,r;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // first just check printing a literal string
 if(x==NULL)
 	{// special case as sprintf() crashes - check that ya_s_snprintf() is OK
 	 scnt++;
 	 r_ya=ya_s_sprintf(buf_ya,x);
 	 if(0!=r_ya){ ++serrs;printf ("<NULL>: ya_sprintf() returns %d expected 0\n",r_ya);}
 	  // repeat with snprintf and n=5
 	 scnt++;
 	 r_ya=ya_s_snprintf(buf_ya,5,x); 
 	 if(0!=r_ya){ ++serrs;printf ("<NULL>: ya_snprintf(5) returns %d expected 0\n",r_ya);}
 	 return;
 	}
 /* # pragma 's below turn off gcc warning for 	[-Wformat-security] which complains about formats being variable but no arguments being supplied */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security" 	
 scnt++;	
 r=sprintf(buf,x);
 r_ya=ya_s_sprintf(buf_ya,x);
 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",x,buf,buf_ya);};
 // repeat with snprintf and n=5
 scnt++;
 r=snprintf(buf,5,x);
 r_ya=ya_s_snprintf(buf_ya,5,x);
 if(r!=r_ya){ ++serrs;printf ("%s: snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,buf,buf_ya);};
#pragma GCC diagnostic pop  	 
}

/* check char's */
void check_str_c(char *x,char PAR)
{
 int r_ya,r;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // first just check printing a literal string
 scnt++;
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",x,buf,buf_ya);};	
 // repeat with snprintf and n=5
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s: snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,buf,buf_ya);};	
}

/* now check printing a string */
void check_str_str(char *x)
{
 int r_ya,r;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // first just check printing a literal string
 scnt++;	
 char * PAR=NULL; /* string to print - check NULL 1st*/
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",x,buf,buf_ya);};
 scnt++;
 	
 PAR="abcdefg"; /* string to print */
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",x,buf,buf_ya);};
 scnt++;
 if(strcmp(x,"%s")==0 && strcmp(PAR,buf_ya)) {++serrs;printf("%s: ya_sprintf() gives %s\n",x,buf_ya);}; // check string matches original when we just have %s
	// repeat with snprintf and n=5	
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s: snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,buf,buf_ya);};

	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s: snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,buf,buf_ya);};

	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s: snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf(10) gives %s ya_sprintf(10) gives %s\n",x,buf,buf_ya);}
}

void check_str_i1(char *x,int PAR)
{
 int r_ya,r;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // first just check printing a literal string
 scnt++;
	// basic check sprintf
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%d): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%d): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);};
 // now check round the loop, only if %d at start of format string
 r=strtol(buf,NULL,0);
 r_ya=strtol(buf_ya,NULL,0);
 scnt++;
 if(*x=='%' && r!=r_ya) {++serrs;printf("%s(%d): ya_sprintf() gives %d (%s) which <> %d!\n",x,PAR,r_ya,buf_ya,r);};
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%d): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%d): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%d): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%d): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%d): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%d): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
}

void check_str_i(char *x,int PAR)
{
	// following function combines all above checks into 1 and repeats for a range of options that all print integers in various formats
 char chks[100];	
    // chks is a string that holds current format string %d will be replaced with iouxX in turn
    // also check # modifier for x,X,o
 check_str_i1(x,PAR);
 check_str_i1( "%#x",PAR);
 check_str_i1( "%#X",PAR);
 check_str_i1( "%#o",PAR);
 for(char *sub="iouxX";*sub;++sub)
		{strcpy(chks,x);
		 char *sub1=strchr(chks,'d');
		 if(sub1!=NULL)
		 	{*sub1=*sub;
		 	 check_str_i1(chks,PAR);
			}
		} 
}

void check_str_l1(char *x,long PAR)
{
 int r_ya,r;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // first just check printing a literal string
 scnt++;
	// basic check sprintf
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%ld): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%ld): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);};
 long lr,lr_ya;
 lr=strtol(buf,NULL,0);
 lr_ya=strtol(buf_ya,NULL,0);
 scnt++;
 if(*x=='%' && lr!=lr_ya) {++serrs;printf("%s(%ld): ya_sprintf() gives %ld (%s) which <> %ld!\n",x,PAR,lr_ya,buf_ya,lr);}; // check "round the loop"
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%ld): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%ld): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%ld): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%ld): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%ld): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%ld): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
}

void check_str_l(char *x,long PAR)
{
	// following function combines all above checks into 1 and repeats for a range of options that all print integers in various formats
 char chks[100];	
    // chks is a string that holds current format string %d will be replaced with iouxX in turn
    // also check # modifier for x,X,o
 check_str_l1(x,PAR);
 check_str_l1( "%#lx",PAR);
 check_str_l1( "%#lX",PAR);
 check_str_l1( "%#lo",PAR);
 for(char *sub="iouxX";*sub;++sub)
		{strcpy(chks,x);
		 char *sub1=strchr(chks,'d');
		 if(sub1!=NULL)
		 	{*sub1=*sub;
		 	 check_str_l1(chks,PAR);
			}
		} 
}

void check_str_ll1(char *x,long long PAR)
{
 int r_ya,r;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // first just check printing a literal string
 scnt++;
	// basic check sprintf
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%lld): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%lld): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);};
 long long lr,lr_ya;
 lr=strtoll(buf,NULL,0);
 lr_ya=strtoll(buf_ya,NULL,0);
 scnt++;
 if(*x=='%' && lr!=lr_ya) {++serrs;printf("%s(%lld): ya_sprintf() gives %lld (%s) which <> %lld!\n",x,PAR,lr_ya,buf_ya,lr);}; // check "round the loop" 
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%lld): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%lld): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%lld): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%lld): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%lld): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%lld): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
}

void check_str_ll(char *x,long long PAR)
{
	// following function combines all above checks into 1 and repeats for a range of options that all print integers in various formats
 char chks[100];	
    // chks is a string that holds current format string %d will be replaced with iouxX in turn
    // also check # modifier for x,X,o
 check_str_ll1(x,PAR);
 check_str_ll1( "%#lx",PAR);
 check_str_ll1( "%#lX",PAR);
 check_str_ll1( "%#lo",PAR);
 for(char *sub="iouxX";*sub;++sub)
		{strcpy(chks,x);
		 char *sub1=strchr(chks,'d');
		 if(sub1!=NULL)
		 	{*sub1=*sub;
		 	 check_str_ll1(chks,PAR);
			}
		} 
}

void check_str_g1(char *x,double PAR)
{
 int r_ya,r;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // printf("check_str_g1(%s,%g)\n",x,PAR);
 // first just check printing a literal string
 scnt++;
	// basic check sprintf
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%g): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%g): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);};
 if(*x=='%') // check "round the loop" when %g is at the start of the string
 	{scnt++;
     if(!(strtod(buf,NULL)==strtod(buf_ya,NULL)|| (isnan(strtod(buf,NULL)) && isnan(strtod(buf,NULL))) || (isinf(strtod(buf,NULL)) && isinf(strtod(buf,NULL))) )) {++serrs;printf("%s(%g):[strtod()] sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);}; 
	}
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%g): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%g): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%g): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%g): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,20,x,PAR); // 20 sig figs
 r_ya=ya_s_snprintf(buf_ya,20,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%g): snprintf(20) returns %d ya_snprintf(20) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%g): snprintf(20) gives %s ya_snprintf(20) gives %s\n",x,PAR,buf,buf_ya);};	
}

void check_str_g(char *x,double PAR)
{
	// following function combines all above checks into 1 and repeats for a range of options that all print doubles in various formats
 char chks[100];	
    // chks is a string that holds current format string %g will be replaced with "GeEaA" in turn
 // printf("check_str_g(%s,%g)\n",x,PAR);    
    // also check # modifier 
 check_str_g1(x,PAR);
 check_str_g1( "%#g",PAR);
 check_str_g1( "%#G",PAR);
 check_str_g1( "%#e",PAR);
 check_str_g1( "%#E",PAR); 
 check_str_g1( "%#.5g",PAR);
 check_str_g1( "%#.5G",PAR);
 check_str_g1( "%#.5e",PAR);
 check_str_g1( "%#.5E",PAR);  
 if(PAR!=0)
 	{// built in sprintf has bug for PAR=0 !
     check_str_g1( "%#a",PAR);
 	 check_str_g1( "%#A",PAR);  	
 	 check_str_g1( "%#a",PAR); // was .5a, .5A
 	 check_str_g1( "%#A",PAR);
 	}
 for(char *sub="GeEaA";*sub;++sub)
		{strcpy(chks,x);
		 char *sub1=strchr(chks,'g');
		 if(sub1!=NULL)
		 	{*sub1=*sub;
		 	 if(!((*sub=='a' || *sub=='A') && PAR==0))
		 	 	check_str_g1(chks,PAR);
			}
		} 
}

void check_str_f1(char *x,double PAR)
{
 int r_ya,r;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // first just check printing a literal string
 scnt++;
	// basic check sprintf, need to use strtod() to check equiv doubles are the same as ansii digits vary after ~ 10th sig figure, can only do that if %f is at the start of the format string
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya && r<350 ){ ++serrs;printf ("%s(%g): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(*x=='%' && !(strtod(buf,NULL)==strtod(buf_ya,NULL)|| (isnan(strtod(buf,NULL)) && isnan(strtod(buf,NULL))) || (isinf(strtod(buf,NULL)) && isinf(strtod(buf,NULL))) )) {++serrs;printf("%s(%g):[strtod()] sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);}; 
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya && r<350 ){ ++serrs;printf ("%s(%g): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%g): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya && r<350 ){ ++serrs;printf ("%s(%g): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%g): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR); // only expect 10 sig figs to be accurate
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya && r<350 ){ ++serrs;printf ("%s(%g): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%g): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
}

void check_str_f(char *x,double PAR)
{
	// following function combines all above checks into 1 and repeats for a range of options that all print doubles in various formats
 char chks[100];	
    // chks is a string that holds current format string %g will be replaced with "GeEaA" in turn
    // also check # modifier 
 check_str_f1(x,PAR);
 check_str_f1( "%#f",PAR);
 check_str_f1( "%#F",PAR);
 check_str_f1( "%#.5f",PAR);
 check_str_f1( "%#.5F",PAR);
 check_str_f1( "%#.5e",PAR);

 for(char *sub="F";*sub;++sub)
		{strcpy(chks,x);
		 char *sub1=strchr(chks,'f');
		 if(sub1!=NULL)
		 	{*sub1=*sub;
		 	 check_str_f1(chks,PAR);
			}
		} 
}

void check_str_p(char *x,void * PAR)
{
 int r_ya,r;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // first just check printing a literal string
 // printf("     %%p=%p %%2p=%2p %%10p=%10p %%20p=%20p\n",PAR,PAR,PAR,PAR);
 // ya_s_sprintf(buf_ya,"%%p=%p %%2p=%2p %%10p=%10p %%20p=%20p\n",PAR,PAR,PAR,PAR);
 // printf("ya: %s",buf_ya);
 // ya_s_sprintf(buf_ya,"%%llx=%llx %%2llx=%2llx %%10llx=%10llx %%20llx=%20llx\n",(unsigned long long int)PAR,(unsigned long long int)PAR,(unsigned long long int)PAR,(unsigned long long int)PAR);
 // printf("ya: %s",buf_ya);
 scnt++;
	// basic check sprintf
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%p): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%p): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);};
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%p): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%p): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%p): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%p): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR); 
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;printf ("%s(%p): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;printf("%s(%p): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
}


int main(int argc, char *argv[]) 
{ 
#ifdef USE_HR_TIMER
  double time_taken;
  init_HR_Timer(); // zero timer
#endif  
#if defined(__x86_64) && defined(__linux) /* running linux on x86_64 assume we are running on WSL1 and apply a workaround for a WSL1 bug - this should be OK for WSL-2 and linux */
   unsigned short Cw = 0x37f;
   _FPU_SETCW(Cw);	
#endif
#if defined(PART1_SPRINTF_TESTS) && defined(YA_SP_SPRINTF_IMPLEMENTATION)
	printf("Starting PART1 sprintf tests:\n");
    // check a lot of floating point (double) conversions, some fixed and some random
    check_double(-5.632109321500000000e+15);
 	check_double(0);
 	check_double(0.1);
 	check_double(-0.1);
 	check_double(0.2);
 	check_double(-0.2);
 	check_double(0.3);
 	check_double(-0.3);
 	check_double(0.4);
 	check_double(-0.4);
 	check_double(0.5);
 	check_double(-0.5);
 	check_double(0.6);
 	check_double(-0.6);
 	check_double(0.7);
 	check_double(-0.7);
 	check_double(0.8);
 	check_double(-0.8);
 	check_double(0.9);
 	check_double(-0.9);	 	
 	check_double(1);
 	check_double(-1);
 	check_double(2);
 	check_double(-2);
 	check_double(3);
 	check_double(-3);
 	check_double(4);
 	check_double(-4);
 	check_double(5);
 	check_double(-5);
 	check_double(6);
 	check_double(-6);
 	check_double(7);
 	check_double(-7);
 	check_double(8);
 	check_double(-8);
 	check_double(9);
 	check_double(-9);	 	 	 	 	 	 	 	  	
 	check_double(10);
 	check_double(-10);
 	check_double(100);
 	check_double(-100);
 	check_double(1000);
 	check_double(-1000);
 	check_double(10000);
 	check_double(-10000);
 	check_double(100000);
 	check_double(-100000);
 	check_double(1000000);
 	check_double(-1000000);
	check_double(1234567);
	check_double(-1234567); 
	check_double(1234567.89);
	check_double(-1234567.89); 		
	check_double(1234567.890123456789);
	check_double(-1234567.890123456789); 		 	 	 	 	  	
 	check_double(10000000);
 	check_double(-10000000);
 	check_double(1e8);
 	check_double(-1e8);
 	check_double(1e9);
 	check_double(-1e9);	 	  	
 	check_double(1e10);
 	check_double(-1e10);
 	check_double(1e20);
 	check_double(-1e20);
 	check_double(1e21);
 	check_double(-1e21);
 	check_double(1e22);
 	check_double(-1e22);
 	check_double(1e23);
 	check_double(-1e23);	 	  	
 	check_double(1e24);
 	check_double(-1e24);
 	check_double(1e25);
 	check_double(-1e25);	 
 	check_double(1e26);
 	check_double(-1e26);
 	check_double(1e27);
 	check_double(-1e27);
 	check_double(1e28);
 	check_double(-1e28);
 	check_double(1e29);
 	check_double(-1e29);	 	 	 	  	
 	check_double(1e30);
 	check_double(-1e30);
 	check_double(1e100);
 	check_double(-1e100);	 
 	check_double(1e200);
 	check_double(-1e200);
 	check_double(1e300);
 	check_double(-1e300);
 	check_double(1e308);
 	check_double(-1e308);	
	check_double(1.7976931348623157e308); // largest
	check_double(-1.7976931348623157e308); // largest
	check_double(HUGE_VAL); 
	check_double(-HUGE_VAL); 	 	 	
 	check_double(0.1);
 	check_double(-0.1);
 	check_double(0.01);
 	check_double(-0.01);
 	check_double(0.001);
 	check_double(-0.001);
 	check_double(0.0001);
 	check_double(-0.0001);
 	check_double(0.000123456789);
 	check_double(-0.000123456789); 	
 	check_double(0.00001);
 	check_double(-0.00001);
 	check_double(0.000001);
 	check_double(-0.000001);	 	 	  	
 	check_double(1e-10);
 	check_double(-1e-10);	 
 	check_double(1e-100);
 	check_double(-1e-100);	 
 	check_double(1e-200);
 	check_double(-1e-200);	 
 	check_double(1e-300);
 	check_double(-1e-300);	
 	check_double(1e-306);
 	check_double(-1e-306);	
 	check_double(1e-307);
 	check_double(-1e-307);		  	
 	check_double(2.2250738585072014e-308);// not denormalised
 	check_double(-2.2250738585072014e-308);// not denormalised	 	  
 	check_double(1e-308);// denormalised
 	check_double(-1e-308);	 
 	check_double(1e-309);
 	check_double(-1e-309);
 	check_double(1e-310);
 	check_double(-1e-310);	
 	check_double(1e-311);
 	check_double(-1e-311);
 	check_double(1e-312);
 	check_double(-1e-312);
 	check_double(1e-313);
 	check_double(-1e-313);
 	check_double(1e-314);
 	check_double(-1e-314);
 	check_double(1e-315);
 	check_double(-1e-315);
 	check_double(1e-316);
 	check_double(-1e-316);
 	check_double(1e-317);
 	check_double(-1e-317);	
 	check_double(1e-318);
 	check_double(-1e-318);
 	check_double(1e-319);
 	check_double(-1e-319);		 		 	 		 		 		 		 		 		 	 		 	 	 	 	 	 	  	  	
 	check_double(1e-320);
 	check_double(-1e-320);
 	check_double(1e-321);
 	check_double(-1e-321);	
 	check_double(1e-322);
 	check_double(-1e-322);	
 	check_double(1e-323);
 	check_double(-1e-323);		 	  	
	check_double(4.941e-324); // smallest denormalised number	 	 
	check_double(-4.941e-324); 	
// more general test cases
    check_double(1.354);
    check_double(1.35498349834);
    check_double(9384.3549823948);
    check_double(182983.234234);
    check_double(34.3420983498);
    check_double(1.0001);
    check_double(1054E-3);
    check_double(-1054E-3);
    check_double(-10.54E30);
    check_double(-345554.54e-5);
    check_double(-34555.534954e-5);
    check_double(-34555.534954e-5);
    check_double(549067);
    check_double(567);
    check_double(446);
    check_double(73);
    check_double(256);
    check_double(5676);
    check_double(738);
    check_double(684);
    check_double(26);
    check_double(673.678e-56);
    check_double(53);
    check_double(67);
    check_double(684);
    check_double(-5437E24);
    check_double(84);
    check_double(56733.68);
    check_double(786);
    check_double(6478);
    check_double(34563.65683598734);
    check_double(5673);
    check_double(784e-3);
    check_double(8678);
    check_double(46784);
    check_double(-54.0888e-6);
    check_double(12.345);
    check_double(12.345e19);
    check_double(-.1e+9);
    check_double(.125);
    check_double(1e20);
    check_double(400012);
    check_double(5.9e-76);	
	// printf("%" PRIu64 " differences so far\n",errs);
 	printf(" Starting random number checks:\n");
#if 1
 	for(int j=0; j<20;++j) // 20 itns is approx 100 secs on my PC, but by using a constant number of iterations, checks should be identical on any PC
#else /* use code below to find a sensible number of iterations */
 	clock_t start_t;
 	start_t=clock();
 	while(clock()-start_t< 100*CLOCKS_PER_SEC) // do for approx 100 secs, my PC does ~ 30 iterations in 100 secs so could make a constant 30 to make results reproducable on different PC?
#endif
 		{
 	 
	 	 for(int i=0;i<100000;++i)  
 			{// do a reasonable number of times so impact of call to clock() is small
 		 	 du.u=randu64();
 		 	 if(isfinite(du.d))
 		 		{
 		 		// void check_float_to_str(char *in_str,double x)
 		 	 	check_float_to_str(NULL,du.d);
 		 		}
			}
	 	 printf("  Just checked %.18g\n",du.d);	
		}    
	
	// end of tests 
#ifdef USE_HR_TIMER	
	time_taken=read_HR_Timer();
 	printf(" All double round loop tests completed in %g secs\n",time_taken);
#endif 	
	printf(" %" PRIu64 " tests %" PRIu64 " differences\n",nos_tests,errs);

#ifdef 	YA_SP_SPRINTF_IMPLEMENTATION
	printf(" Tested ya_sprintf() double-double round loop:\n");
#elif defined(MINGW_SPRINTF)
	printf("Tested MinGW built in sprintf:\n");
#else
	printf("Tested double_to_str_exp:\n"); 
#endif
	printf(" %" PRIu64 " errors when 21 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl21,errs_dbl21_1bit,errs_printf21);
	printf(" %" PRIu64 " errors when 20 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl20,errs_dbl20_1bit,errs_printf20);
	printf(" %" PRIu64 " errors when 19 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl19,errs_dbl19_1bit,errs_printf19);		
	printf(" %" PRIu64 " errors when 18 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl18,errs_dbl18_1bit,errs_printf18);
	printf(" %" PRIu64 " errors when 17 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl17,errs_dbl17_1bit,errs_printf17);
	printf(" %" PRIu64 " errors when 16 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl16,errs_dbl16_1bit,errs_printf16);	
	printf(" Differences between built in \"libc\" sprintf() and tested sprintf() are:\n");
	for(int i=2;i<=19;++i)
		{
		 printf(" %2d sf found %" PRIu64 " differences\n",i,errsf[i]);
		}
#ifdef COUNT_EXP_LOOPS /* defined in float_to_str.h */
	printf(" %" PRIu64 " exponent corrections done in float-to-string()\n",count_exp_loops);
#endif	
#endif
#ifdef __SIZEOF_INT128__ 
	chk_fast_strtof128() ; // tests for fast_strtof128() if compiler supports __float128 data type
#endif
#if defined(PART2_SPRINTF_TESTS) && defined(YA_SP_SPRINTF_IMPLEMENTATION)
	
	printf("Starting PART2 sprintf tests:\n");

	printf("Constant strings:\n");
    check_str_s(NULL);
	check_str_s("1");
	check_str_s("12");
	check_str_s("123");
	check_str_s("1234");
	check_str_s("12345");
	check_str_s("123456");
	check_str_s("1234567");
	check_str_s("12345678");
	check_str_s("123456789");
	check_str_s("1234567890");
	check_str_s("%%");
	check_str_s("1%%");
	check_str_s("1%%2");
	check_str_s("1%%2%%3");
	check_str_s("1%%2%%3%%4");
	check_str_s("%%1%%2%%3%%4%%5%%");
	check_str_s("123%%456");
	check_str_s("1234567%%");
	check_str_s("12%%34%%5678");
	check_str_s("12%%345%%6789");
	check_str_s("12%%34567%%890");
	check_str_s("1\n");
	check_str_s("1\n2\n");
	check_str_s("12\n3\n");
	check_str_s("1\n2\n3\n4\n");
	check_str_s("12345\n");
	check_str_s("123456\n");
	check_str_s("1234567\n");
	check_str_s("12345\n678");
	check_str_s("123456\n789");
	check_str_s("123%%456%%78\n90");		
		
	// now check printing a character
	printf("printing %%c:\n");
	for(int i=0;i<=255;++i)
		{
		 check_str_c("%c",i);
		 check_str_c("1%c",i);
		 check_str_c("%c1",i);		
		 check_str_c("1%c2",i);
		 check_str_c("1%c23",i);
		 check_str_c("1%c234",i);
		 check_str_c("1%c2345",i);
		 check_str_c("1%c23456",i);
		 check_str_c("1%c234567",i);
		 check_str_c("1%c2345678",i);
		 check_str_c("1%c23456789",i);
	 	 check_str_c("12%c34567890",i);	
		 check_str_c("123%c4567890",i);
		 check_str_c("1234%c567890",i);
		 check_str_c("12345%c67890",i);
		 check_str_c("123456%c7890",i);
		 check_str_c("1234567%c890",i);
		 check_str_c("12345678%c90",i);
		 check_str_c("123456789%c0",i);
		 check_str_c("1234567890%c",i);
		 check_str_c("%c1234567890",i);	
		 // now try printing as an integer
	 	 check_str_i("%hhd",i);
	 	 check_str_i("%5hhd",i);
		 check_str_i("%-5hhd",i);
		 // now try printing as an short integer
		 volatile short int si=i*i; // convert 8 bits to 16
	 	 check_str_i("%hd",si);
	 	 check_str_i("%5hd",si);
		 check_str_i("%-5hd",si);		 
		}
	// now check printing a string paramater (%s)	
	printf("printing %%s:\n");	
#define CHECK_STR(s) check_str_str(s)
	CHECK_STR("%s");
	CHECK_STR("1%s");
	CHECK_STR("%s1");		
	CHECK_STR("1%s2");
	CHECK_STR("1%s23");
	CHECK_STR("1%s234");
	CHECK_STR("1%s2345");
	CHECK_STR("1%s23456");
	CHECK_STR("1%s234567");
	CHECK_STR("1%s2345678");
	CHECK_STR("1%s23456789");
	CHECK_STR("12%s34567890");	
	CHECK_STR("123%s4567890");
	CHECK_STR("1234%s567890");
	CHECK_STR("12345%s67890");
	CHECK_STR("123456%s7890");
	CHECK_STR("1234567%s890");
	CHECK_STR("12345678%s90");
	CHECK_STR("123456789%s0");
	CHECK_STR("1234567890%s");
	CHECK_STR("%s1234567890");	
	// min field width of 5
	CHECK_STR("%5s");
	CHECK_STR("1%5s");
	CHECK_STR("%5s1");		
	CHECK_STR("1%5s2");
	CHECK_STR("1%5s23");
	CHECK_STR("1%5s234");
	CHECK_STR("1%5s2345");
	CHECK_STR("1%5s23456");
	CHECK_STR("1%5s234567");
	CHECK_STR("1%5s2345678");
	CHECK_STR("1%5s23456789");
	CHECK_STR("12%5s34567890");	
	CHECK_STR("123%5s4567890");
	CHECK_STR("1234%5s567890");
	CHECK_STR("12345%5s67890");
	CHECK_STR("123456%5s7890");
	CHECK_STR("1234567%5s890");
	CHECK_STR("12345678%5s90");
	CHECK_STR("123456789%5s0");
	CHECK_STR("1234567890%5s");
	CHECK_STR("%5s1234567890");
	// min field width of 9	
	CHECK_STR("%9s");
	CHECK_STR("1%9s");
	CHECK_STR("%9s1");		
	CHECK_STR("1%9s2");
	CHECK_STR("1%9s23");
	CHECK_STR("1%9s234");
	CHECK_STR("1%9s2345");
	CHECK_STR("1%9s23456");
	CHECK_STR("1%9s234567");
	CHECK_STR("1%9s2345678");
	CHECK_STR("1%9s23456789");
	CHECK_STR("12%9s34567890");	
	CHECK_STR("123%9s4567890");
	CHECK_STR("1234%9s567890");
	CHECK_STR("12345%9s67890");
	CHECK_STR("123456%9s7890");
	CHECK_STR("1234567%9s890");
	CHECK_STR("12345678%9s90");
	CHECK_STR("123456789%9s0");
	CHECK_STR("1234567890%9s");
	CHECK_STR("%9s1234567890");		
	// min field width of 9 left aligned in field
	CHECK_STR("%-9s");
	CHECK_STR("1%-9s");
	CHECK_STR("%-9s1");		
	CHECK_STR("1%-9s2");
	CHECK_STR("1%-9s23");
	CHECK_STR("1%-9s234");
	CHECK_STR("1%-9s2345");
	CHECK_STR("1%-9s23456");
	CHECK_STR("1%-9s234567");
	CHECK_STR("1%-9s2345678");
	CHECK_STR("1%-9s23456789");
	CHECK_STR("12%-9s34567890");	
	CHECK_STR("123%-9s4567890");
	CHECK_STR("1234%-9s567890");
	CHECK_STR("12345%-9s67890");
	CHECK_STR("123456%-9s7890");
	CHECK_STR("1234567%-9s890");
	CHECK_STR("12345678%-9s90");
	CHECK_STR("123456789%-9s0");
	CHECK_STR("1234567890%-9s");
	CHECK_STR("%-9s1234567890");		
	// max field length of 5
	CHECK_STR("%.5s");
	CHECK_STR("1%.5s");
	CHECK_STR("%.5s1");		
	CHECK_STR("1%.5s2");
	CHECK_STR("1%.5s23");
	CHECK_STR("1%.5s234");
	CHECK_STR("1%.5s2345");
	CHECK_STR("1%.5s23456");
	CHECK_STR("1%.5s234567");
	CHECK_STR("1%.5s2345678");
	CHECK_STR("1%.5s23456789");
	CHECK_STR("12%.5s34567890");	
	CHECK_STR("123%.5s4567890");
	CHECK_STR("1234%.5s567890");
	CHECK_STR("12345%.5s67890");
	CHECK_STR("123456%.5s7890");
	CHECK_STR("1234567%.5s890");
	CHECK_STR("12345678%.5s90");
	CHECK_STR("123456789%.5s0");
	CHECK_STR("1234567890%.5s");
	CHECK_STR("%.5s1234567890");
	// vary max and min field lengths
	CHECK_STR("%1.3s");
	CHECK_STR("%2.4s");		
	CHECK_STR("%3.5s");
	CHECK_STR("%4.6s");		
	CHECK_STR("%5.7s");
	CHECK_STR("%6.8s");
	CHECK_STR("%7.9s");
	CHECK_STR("%8.10s");
	CHECK_STR("%9.11s");
	CHECK_STR("%10.12s");
	CHECK_STR("%11.13s");							
			
							
	// now check printing of integer paramaters (%diouxX )
#undef CHECK_STR
#define CHECK_STR(x) check_str_i(x,param)
	printf("printing %%i:\n");	
  int test_value[]={INT_MIN,-12345678,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,20,99,100,345,12345678,INT_MAX};
  for(int i=0;i<nos_elements_in(test_value);++i)
   {int param=test_value[i]; // test all values in test_value array 	
	if(sizeof(int)==sizeof(size_t))
		{
		 CHECK_STR("%zd"); // %zd is size_t
		 CHECK_STR("%5zd"); 
		 CHECK_STR("%9zd"); 
		 CHECK_STR("%-9zd"); 
		 CHECK_STR("%02zd"); 
		 CHECK_STR("%1.3zd");
		 CHECK_STR("%2.4zd");		
		 CHECK_STR("%3.5zd");
		 CHECK_STR("%4.6zd");		
		 CHECK_STR("%5.7zd");
		 CHECK_STR("%6.8zd");
		 CHECK_STR("%7.9zd");
		 CHECK_STR("%8.10zd");
		 CHECK_STR("%9.11zd");
		 CHECK_STR("%10.12zd");
		 CHECK_STR("%11.13zd");	
		}	
	if(sizeof(int)==sizeof(ptrdiff_t))
		{
		 CHECK_STR("%td"); // %td is ptrdiff_t
		 CHECK_STR("%5td"); 
		 CHECK_STR("%9td"); 
		 CHECK_STR("%-9td"); 
		 CHECK_STR("%02td"); 
		 CHECK_STR("%1.3td");
		 CHECK_STR("%2.4td");		
		 CHECK_STR("%3.5td");
		 CHECK_STR("%4.6td");		
		 CHECK_STR("%5.7td");
		 CHECK_STR("%6.8td");
		 CHECK_STR("%7.9td");
		 CHECK_STR("%8.10td");
		 CHECK_STR("%9.11td");
		 CHECK_STR("%10.12td");
		 CHECK_STR("%11.13td");	
		} 
#ifdef __MINGW32__  /* %I only available with MINGW runtime */		
	if(sizeof(void *)==4 )
		{
		 CHECK_STR("%Id"); // %Id is intptr_t (32 or 64 bit)
		 CHECK_STR("%5Id"); 
		 CHECK_STR("%9Id"); 
		 CHECK_STR("%-9Id"); 
		 CHECK_STR("%02Id"); 
		 CHECK_STR("%1.3Id");
		 CHECK_STR("%2.4Id");		
		 CHECK_STR("%3.5Id");
		 CHECK_STR("%4.6Id");		
		 CHECK_STR("%5.7Id");
		 CHECK_STR("%6.8Id");
		 CHECK_STR("%7.9Id");
		 CHECK_STR("%8.10Id");
		 CHECK_STR("%9.11Id");
		 CHECK_STR("%10.12Id");
		 CHECK_STR("%11.13Id");	
		} 
	if(sizeof(int)==4 )
		{
		 CHECK_STR("%I32d"); // %I32d is 32 bit
		 CHECK_STR("%5I32d"); 
		 CHECK_STR("%9I32d"); 
		 CHECK_STR("%-9I32d"); 
		 CHECK_STR("%02I32d"); 
		 CHECK_STR("%1.3I32d");
		 CHECK_STR("%2.4I32d");		
		 CHECK_STR("%3.5I32d");
		 CHECK_STR("%4.6I32d");		
		 CHECK_STR("%5.7I32d");
		 CHECK_STR("%6.8I32d");
		 CHECK_STR("%7.9I32d");
		 CHECK_STR("%8.10I32d");
		 CHECK_STR("%9.11I32d");
		 CHECK_STR("%10.12I32d");
		 CHECK_STR("%11.13I32d");	
		} 	
#endif		
	// check standard %d	(int)  
	CHECK_STR("%d");
	CHECK_STR("z%d");
	CHECK_STR("%dz");		
	CHECK_STR("z%dz");
	CHECK_STR("z%d23");
	CHECK_STR("z%d234");
	CHECK_STR("z%d2345");
	CHECK_STR("z%d23456");
	CHECK_STR("z%d234567");
	CHECK_STR("z%d2345678");
	CHECK_STR("z%d23456789");
	CHECK_STR("zz%d34567890");	
	CHECK_STR("123%d4567890");
	CHECK_STR("1234%d567890");
	CHECK_STR("12345%d67890");
	CHECK_STR("123456%d7890");
	CHECK_STR("1234567%d890");
	CHECK_STR("12345678%d90");
	CHECK_STR("123456789%d0");
	CHECK_STR("1234567890%d");
	CHECK_STR("%dz1234567890");	
	// min field width of 5
	CHECK_STR("%5d");
	CHECK_STR("1%5d");
	CHECK_STR("%5dz1");		
	CHECK_STR("1%5d2");
	CHECK_STR("1%5d23");
	CHECK_STR("1%5d234");
	CHECK_STR("1%5d2345");
	CHECK_STR("1%5d23456");
	CHECK_STR("1%5d234567");
	CHECK_STR("1%5d2345678");
	CHECK_STR("1%5d23456789");
	CHECK_STR("12%5d34567890");	
	CHECK_STR("123%5d4567890");
	CHECK_STR("1234%5d567890");
	CHECK_STR("12345%5d67890");
	CHECK_STR("123456%5d7890");
	CHECK_STR("1234567%5d890");
	CHECK_STR("12345678%5d90");
	CHECK_STR("123456789%5d0");
	CHECK_STR("1234567890%5d");
	CHECK_STR("%5dz1234567890");
	// min field width of 9	
	CHECK_STR("%9d");
	CHECK_STR("1%9d");
	CHECK_STR("%9dz1");		
	CHECK_STR("1%9d2");
	CHECK_STR("1%9d23");
	CHECK_STR("1%9d234");
	CHECK_STR("1%9d2345");
	CHECK_STR("1%9d23456");
	CHECK_STR("1%9d234567");
	CHECK_STR("1%9d2345678");
	CHECK_STR("1%9d23456789");
	CHECK_STR("12%9d34567890");	
	CHECK_STR("123%9d4567890");
	CHECK_STR("1234%9d567890");
	CHECK_STR("12345%9d67890");
	CHECK_STR("123456%9d7890");
	CHECK_STR("1234567%9d890");
	CHECK_STR("12345678%9d90");
	CHECK_STR("123456789%9d0");
	CHECK_STR("1234567890%9d");
	CHECK_STR("%9dz1234567890");		
	// min field width of 9 left aligned in field
	CHECK_STR("%-9d");
	CHECK_STR("1%-9d");
	CHECK_STR("%-9dz1");		
	CHECK_STR("1%-9d2");
	CHECK_STR("1%-9d23");
	CHECK_STR("1%-9d234");
	CHECK_STR("1%-9d2345");
	CHECK_STR("1%-9d23456");
	CHECK_STR("1%-9d234567");
	CHECK_STR("1%-9d2345678");
	CHECK_STR("1%-9d23456789");
	CHECK_STR("12%-9d34567890");	
	CHECK_STR("123%-9d4567890");
	CHECK_STR("1234%-9d567890");
	CHECK_STR("12345%-9d67890");
	CHECK_STR("123456%-9d7890");
	CHECK_STR("1234567%-9d890");
	CHECK_STR("12345678%-9d90");
	CHECK_STR("123456789%-9d0");
	CHECK_STR("1234567890%-9d");
	CHECK_STR("%-9dz1234567890");
	// leading zero's with varying field widths
	CHECK_STR("%01d");
	CHECK_STR("1%02d");
	CHECK_STR("%03dz1");		
	CHECK_STR("1%04d2");
	CHECK_STR("1%05d23");
	CHECK_STR("1%06d234");
	CHECK_STR("1%07d2345");
	CHECK_STR("1%08d23456");
	CHECK_STR("1%09d234567");
	CHECK_STR("1%01d2345678");
	CHECK_STR("1%02d23456789");
	CHECK_STR("12%03d34567890");	
	CHECK_STR("123%04d4567890");
	CHECK_STR("1234%05d567890");
	CHECK_STR("12345%06d67890");
	CHECK_STR("123456%07d7890");
	CHECK_STR("1234567%08d890");
	CHECK_STR("12345678%09d90");
	CHECK_STR("123456789%010d0");
	CHECK_STR("1234567890%011d");
	CHECK_STR("%012dz1234567890");				
	// max field length of 5
	CHECK_STR("%.5d");
	CHECK_STR("1%.5d");
	CHECK_STR("%.5dz1");		
	CHECK_STR("1%.5d2");
	CHECK_STR("1%.5d23");
	CHECK_STR("1%.5d234");
	CHECK_STR("1%.5d2345");
	CHECK_STR("1%.5d23456");
	CHECK_STR("1%.5d234567");
	CHECK_STR("1%.5d2345678");
	CHECK_STR("1%.5d23456789");
	CHECK_STR("12%.5d34567890");	
	CHECK_STR("123%.5d4567890");
	CHECK_STR("1234%.5d567890");
	CHECK_STR("12345%.5d67890");
	CHECK_STR("123456%.5d7890");
	CHECK_STR("1234567%.5d890");
	CHECK_STR("12345678%.5d90");
	CHECK_STR("123456789%.5d0");
	CHECK_STR("1234567890%.5d");
	CHECK_STR("%.5dz1234567890");
	// vary max and min field lengths
	CHECK_STR("%1.3d");
	CHECK_STR("%2.4d");		
	CHECK_STR("%3.5d");
	CHECK_STR("%4.6d");		
	CHECK_STR("%5.7d");
	CHECK_STR("%6.8d");
	CHECK_STR("%7.9d");
	CHECK_STR("%8.10d");
	CHECK_STR("%9.11d");
	CHECK_STR("%10.12d");
	CHECK_STR("%11.13d");		
	// check + and space modifiers
	CHECK_STR("%+d");
	CHECK_STR("% d");
	CHECK_STR("% +d"); // space ignored if + also present
	CHECK_STR("%+ d"); // space ignored if + also present
	// check 0, - modifiers
	CHECK_STR("%010d");
	CHECK_STR("%-10d");
	CHECK_STR("%-010d");// if - and 0 both present 0 is ignored			
	CHECK_STR("%0-10d");// if - and 0 both present 0 is ignored		
	CHECK_STR("%010.5d"); // if 0 and precision specified 0 is ignored
								
   } // end of for loop checking int's 
		
	// now check printing of long integer paramaters (%diouxX )
	printf("printing %%ld:\n");		
#undef CHECK_STR
#define CHECK_STR(x) check_str_l(x,param)
  long int test_valuel[]={LONG_MIN,INT_MIN,-12345678,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,20,99,100,345,12345678,INT_MAX,LONG_MAX};
  for(int i=0;i<nos_elements_in(test_valuel);++i)
   {long int param=test_valuel[i]; // test all values in test_value array 
	
	CHECK_STR("%ld");
	CHECK_STR("1%ld");
	CHECK_STR("%ld1");		
	CHECK_STR("1%ld2");
	CHECK_STR("1%ld23");
	CHECK_STR("1%ld234");
	CHECK_STR("1%ld2345");
	CHECK_STR("1%ld23456");
	CHECK_STR("1%ld234567");
	CHECK_STR("1%ld2345678");
	CHECK_STR("1%ld23456789");
	CHECK_STR("12%ld34567890");	
	CHECK_STR("123%ld4567890");
	CHECK_STR("1234%ld567890");
	CHECK_STR("12345%ld67890");
	CHECK_STR("123456%ld7890");
	CHECK_STR("1234567%ld890");
	CHECK_STR("12345678%ld90");
	CHECK_STR("123456789%ld0");
	CHECK_STR("1234567890%ld");
	CHECK_STR("%ld1234567890");	
	// min field width of 5
	CHECK_STR("%5ld");
	CHECK_STR("1%5ld");
	CHECK_STR("%5ld1");		
	CHECK_STR("1%5ld2");
	CHECK_STR("1%5ld23");
	CHECK_STR("1%5ld234");
	CHECK_STR("1%5ld2345");
	CHECK_STR("1%5ld23456");
	CHECK_STR("1%5ld234567");
	CHECK_STR("1%5ld2345678");
	CHECK_STR("1%5ld23456789");
	CHECK_STR("12%5ld34567890");	
	CHECK_STR("123%5ld4567890");
	CHECK_STR("1234%5ld567890");
	CHECK_STR("12345%5ld67890");
	CHECK_STR("123456%5ld7890");
	CHECK_STR("1234567%5ld890");
	CHECK_STR("12345678%5ld90");
	CHECK_STR("123456789%5ld0");
	CHECK_STR("1234567890%5ld");
	CHECK_STR("%5ld1234567890");
	// min field width of 9	
	CHECK_STR("%9ld");
	CHECK_STR("1%9ld");
	CHECK_STR("%9ld1");		
	CHECK_STR("1%9ld2");
	CHECK_STR("1%9ld23");
	CHECK_STR("1%9ld234");
	CHECK_STR("1%9ld2345");
	CHECK_STR("1%9ld23456");
	CHECK_STR("1%9ld234567");
	CHECK_STR("1%9ld2345678");
	CHECK_STR("1%9ld23456789");
	CHECK_STR("12%9ld34567890");	
	CHECK_STR("123%9ld4567890");
	CHECK_STR("1234%9ld567890");
	CHECK_STR("12345%9ld67890");
	CHECK_STR("123456%9ld7890");
	CHECK_STR("1234567%9ld890");
	CHECK_STR("12345678%9ld90");
	CHECK_STR("123456789%9ld0");
	CHECK_STR("1234567890%9ld");
	CHECK_STR("%9ld1234567890");		
	// min field width of 9 left aligned in field
	CHECK_STR("%-9ld");
	CHECK_STR("1%-9ld");
	CHECK_STR("%-9ld1");		
	CHECK_STR("1%-9ld2");
	CHECK_STR("1%-9ld23");
	CHECK_STR("1%-9ld234");
	CHECK_STR("1%-9ld2345");
	CHECK_STR("1%-9ld23456");
	CHECK_STR("1%-9ld234567");
	CHECK_STR("1%-9ld2345678");
	CHECK_STR("1%-9ld23456789");
	CHECK_STR("12%-9ld34567890");	
	CHECK_STR("123%-9ld4567890");
	CHECK_STR("1234%-9ld567890");
	CHECK_STR("12345%-9ld67890");
	CHECK_STR("123456%-9ld7890");
	CHECK_STR("1234567%-9ld890");
	CHECK_STR("12345678%-9ld90");
	CHECK_STR("123456789%-9ld0");
	CHECK_STR("1234567890%-9ld");
	CHECK_STR("%-9ld1234567890");
	// leading zero's with varying field widths
	CHECK_STR("%01ld");
	CHECK_STR("1%02ld");
	CHECK_STR("%03ld1");		
	CHECK_STR("1%04ld2");
	CHECK_STR("1%05ld23");
	CHECK_STR("1%06ld234");
	CHECK_STR("1%07ld2345");
	CHECK_STR("1%08ld23456");
	CHECK_STR("1%09ld234567");
	CHECK_STR("1%01ld2345678");
	CHECK_STR("1%02ld23456789");
	CHECK_STR("12%03ld34567890");	
	CHECK_STR("123%04ld4567890");
	CHECK_STR("1234%05ld567890");
	CHECK_STR("12345%06ld67890");
	CHECK_STR("123456%07ld7890");
	CHECK_STR("1234567%08ld890");
	CHECK_STR("12345678%09ld90");
	CHECK_STR("123456789%010ld0");
	CHECK_STR("1234567890%011ld");
	CHECK_STR("%012ld1234567890");				
	// max field length of 5
	CHECK_STR("%.5ld");
	CHECK_STR("1%.5ld");
	CHECK_STR("%.5ld1");		
	CHECK_STR("1%.5ld2");
	CHECK_STR("1%.5ld23");
	CHECK_STR("1%.5ld234");
	CHECK_STR("1%.5ld2345");
	CHECK_STR("1%.5ld23456");
	CHECK_STR("1%.5ld234567");
	CHECK_STR("1%.5ld2345678");
	CHECK_STR("1%.5ld23456789");
	CHECK_STR("12%.5ld34567890");	
	CHECK_STR("123%.5ld4567890");
	CHECK_STR("1234%.5ld567890");
	CHECK_STR("12345%.5ld67890");
	CHECK_STR("123456%.5ld7890");
	CHECK_STR("1234567%.5ld890");
	CHECK_STR("12345678%.5ld90");
	CHECK_STR("123456789%.5ld0");
	CHECK_STR("1234567890%.5ld");
	CHECK_STR("%.5ld1234567890");
	// vary max and min field lengths
	CHECK_STR("%1.3ld");
	CHECK_STR("%2.4ld");		
	CHECK_STR("%3.5ld");
	CHECK_STR("%4.6ld");		
	CHECK_STR("%5.7ld");
	CHECK_STR("%6.8ld");
	CHECK_STR("%7.9ld");
	CHECK_STR("%8.10ld");
	CHECK_STR("%9.11ld");
	CHECK_STR("%10.12ld");
	CHECK_STR("%11.13ld");						
								
   } // end of for loop checking long int's 
   
	// now check printing of long long integer paramaters (%diouxX )
#undef CHECK_STR
#define CHECK_STR(x) check_str_ll(x,param)
	printf("printing %%lld:\n");	
  long long int test_valuell[]={LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,20,99,100,345,12345678,INT_MAX,LONG_MAX,LLONG_MAX};
  for(int i=0;i<nos_elements_in(test_valuell);++i)
   {long long int param=test_valuell[i]; // test all values in test_value array 
	CHECK_STR("%lld");
	CHECK_STR("1%lld");
	CHECK_STR("%lld1");		
	CHECK_STR("1%lld2");
	CHECK_STR("1%lld23");
	CHECK_STR("1%lld234");
	CHECK_STR("1%lld2345");
	CHECK_STR("1%lld23456");
	CHECK_STR("1%lld234567");
	CHECK_STR("1%lld2345678");
	CHECK_STR("1%lld23456789");
	CHECK_STR("12%lld34567890");	
	CHECK_STR("123%lld4567890");
	CHECK_STR("1234%lld567890");
	CHECK_STR("12345%lld67890");
	CHECK_STR("123456%lld7890");
	CHECK_STR("1234567%lld890");
	CHECK_STR("12345678%lld90");
	CHECK_STR("123456789%lld0");
	CHECK_STR("1234567890%lld");
	CHECK_STR("%lld1234567890");	
	if(sizeof(long long int)==	sizeof(intmax_t))
		{
		 CHECK_STR("%jd"); // %jd is int_max_t
		 CHECK_STR("%5jd"); 
		 CHECK_STR("%9jd"); 
		 CHECK_STR("%-9jd"); 
		 CHECK_STR("%02jd"); 
		 CHECK_STR("%1.3jd");
		 CHECK_STR("%2.4jd");		
		 CHECK_STR("%3.5jd");
		 CHECK_STR("%4.6jd");		
		 CHECK_STR("%5.7jd");
		 CHECK_STR("%6.8jd");
		 CHECK_STR("%7.9jd");
		 CHECK_STR("%8.10jd");
		 CHECK_STR("%9.11jd");
		 CHECK_STR("%10.12jd");
		 CHECK_STR("%11.13jd");	
		}
	if(sizeof(long long int)==	sizeof(size_t))
		{
		 CHECK_STR("%zd"); // %zd is size_t
		 CHECK_STR("%5zd"); 
		 CHECK_STR("%9zd"); 
		 CHECK_STR("%-9zd"); 
		 CHECK_STR("%02zd"); 
		 CHECK_STR("%1.3zd");
		 CHECK_STR("%2.4zd");		
		 CHECK_STR("%3.5zd");
		 CHECK_STR("%4.6zd");		
		 CHECK_STR("%5.7zd");
		 CHECK_STR("%6.8zd");
		 CHECK_STR("%7.9zd");
		 CHECK_STR("%8.10zd");
		 CHECK_STR("%9.11zd");
		 CHECK_STR("%10.12zd");
		 CHECK_STR("%11.13zd");	
		}	
	if(sizeof(long long int)==	sizeof(ptrdiff_t))
		{
		 CHECK_STR("%td"); // %td is ptrdiff_t
		 CHECK_STR("%5td"); 
		 CHECK_STR("%9td"); 
		 CHECK_STR("%-9td"); 
		 CHECK_STR("%02td"); 
		 CHECK_STR("%1.3td");
		 CHECK_STR("%2.4td");		
		 CHECK_STR("%3.5td");
		 CHECK_STR("%4.6td");		
		 CHECK_STR("%5.7td");
		 CHECK_STR("%6.8td");
		 CHECK_STR("%7.9td");
		 CHECK_STR("%8.10td");
		 CHECK_STR("%9.11td");
		 CHECK_STR("%10.12td");
		 CHECK_STR("%11.13td");	
		}	
#ifdef __MINGW32__  /* %I only available with MINGW runtime */			
	if(sizeof(void *)==8 )
		{
		 CHECK_STR("%Id"); // %Id is intptr_t (32 or 64 bit)
		 CHECK_STR("%5Id"); 
		 CHECK_STR("%9Id"); 
		 CHECK_STR("%-9Id"); 
		 CHECK_STR("%02Id"); 
		 CHECK_STR("%1.3Id");
		 CHECK_STR("%2.4Id");		
		 CHECK_STR("%3.5Id");
		 CHECK_STR("%4.6Id");		
		 CHECK_STR("%5.7Id");
		 CHECK_STR("%6.8Id");
		 CHECK_STR("%7.9Id");
		 CHECK_STR("%8.10Id");
		 CHECK_STR("%9.11Id");
		 CHECK_STR("%10.12Id");
		 CHECK_STR("%11.13Id");	
		} 
	if(sizeof(long long int)==8 )
		{
		 CHECK_STR("%I64d"); // %I64d is 64 bit
		 CHECK_STR("%5I64d"); 
		 CHECK_STR("%9I64d"); 
		 CHECK_STR("%-9I64d"); 
		 CHECK_STR("%02I64d"); 
		 CHECK_STR("%1.3I64d");
		 CHECK_STR("%2.4I64d");		
		 CHECK_STR("%3.5I64d");
		 CHECK_STR("%4.6I64d");		
		 CHECK_STR("%5.7I64d");
		 CHECK_STR("%6.8I64d");
		 CHECK_STR("%7.9I64d");
		 CHECK_STR("%8.10I64d");
		 CHECK_STR("%9.11I64d");
		 CHECK_STR("%10.12I64d");
		 CHECK_STR("%11.13I64d");	
		} 
#endif						
	// min fielld width of 5
	CHECK_STR("%5lld");
	CHECK_STR("1%5lld");
	CHECK_STR("%5lld1");		
	CHECK_STR("1%5lld2");
	CHECK_STR("1%5lld23");
	CHECK_STR("1%5lld234");
	CHECK_STR("1%5lld2345");
	CHECK_STR("1%5lld23456");
	CHECK_STR("1%5lld234567");
	CHECK_STR("1%5lld2345678");
	CHECK_STR("1%5lld23456789");
	CHECK_STR("12%5lld34567890");	
	CHECK_STR("123%5lld4567890");
	CHECK_STR("1234%5lld567890");
	CHECK_STR("12345%5lld67890");
	CHECK_STR("123456%5lld7890");
	CHECK_STR("1234567%5lld890");
	CHECK_STR("12345678%5lld90");
	CHECK_STR("123456789%5lld0");
	CHECK_STR("1234567890%5lld");
	CHECK_STR("%5lld1234567890");
	// min fielld width of 9	
	CHECK_STR("%9lld");
	CHECK_STR("1%9lld");
	CHECK_STR("%9lld1");		
	CHECK_STR("1%9lld2");
	CHECK_STR("1%9lld23");
	CHECK_STR("1%9lld234");
	CHECK_STR("1%9lld2345");
	CHECK_STR("1%9lld23456");
	CHECK_STR("1%9lld234567");
	CHECK_STR("1%9lld2345678");
	CHECK_STR("1%9lld23456789");
	CHECK_STR("12%9lld34567890");	
	CHECK_STR("123%9lld4567890");
	CHECK_STR("1234%9lld567890");
	CHECK_STR("12345%9lld67890");
	CHECK_STR("123456%9lld7890");
	CHECK_STR("1234567%9lld890");
	CHECK_STR("12345678%9lld90");
	CHECK_STR("123456789%9lld0");
	CHECK_STR("1234567890%9lld");
	CHECK_STR("%9lld1234567890");		
	// min fielld width of 9 left aligned in fielld
	CHECK_STR("%-9lld");
	CHECK_STR("1%-9lld");
	CHECK_STR("%-9lld1");		
	CHECK_STR("1%-9lld2");
	CHECK_STR("1%-9lld23");
	CHECK_STR("1%-9lld234");
	CHECK_STR("1%-9lld2345");
	CHECK_STR("1%-9lld23456");
	CHECK_STR("1%-9lld234567");
	CHECK_STR("1%-9lld2345678");
	CHECK_STR("1%-9lld23456789");
	CHECK_STR("12%-9lld34567890");	
	CHECK_STR("123%-9lld4567890");
	CHECK_STR("1234%-9lld567890");
	CHECK_STR("12345%-9lld67890");
	CHECK_STR("123456%-9lld7890");
	CHECK_STR("1234567%-9lld890");
	CHECK_STR("12345678%-9lld90");
	CHECK_STR("123456789%-9lld0");
	CHECK_STR("1234567890%-9lld");
	CHECK_STR("%-9lld1234567890");
	// leading zero's with varying fielld widths
	CHECK_STR("%01lld");
	CHECK_STR("1%02lld");
	CHECK_STR("%03lld1");		
	CHECK_STR("1%04lld2");
	CHECK_STR("1%05lld23");
	CHECK_STR("1%06lld234");
	CHECK_STR("1%07lld2345");
	CHECK_STR("1%08lld23456");
	CHECK_STR("1%09lld234567");
	CHECK_STR("1%01lld2345678");
	CHECK_STR("1%02lld23456789");
	CHECK_STR("12%03lld34567890");	
	CHECK_STR("123%04lld4567890");
	CHECK_STR("1234%05lld567890");
	CHECK_STR("12345%06lld67890");
	CHECK_STR("123456%07lld7890");
	CHECK_STR("1234567%08lld890");
	CHECK_STR("12345678%09lld90");
	CHECK_STR("123456789%010lld0");
	CHECK_STR("1234567890%011lld");
	CHECK_STR("%012lld1234567890");				
	// max fielld length of 5
	CHECK_STR("%.5lld");
	CHECK_STR("1%.5lld");
	CHECK_STR("%.5lld1");		
	CHECK_STR("1%.5lld2");
	CHECK_STR("1%.5lld23");
	CHECK_STR("1%.5lld234");
	CHECK_STR("1%.5lld2345");
	CHECK_STR("1%.5lld23456");
	CHECK_STR("1%.5lld234567");
	CHECK_STR("1%.5lld2345678");
	CHECK_STR("1%.5lld23456789");
	CHECK_STR("12%.5lld34567890");	
	CHECK_STR("123%.5lld4567890");
	CHECK_STR("1234%.5lld567890");
	CHECK_STR("12345%.5lld67890");
	CHECK_STR("123456%.5lld7890");
	CHECK_STR("1234567%.5lld890");
	CHECK_STR("12345678%.5lld90");
	CHECK_STR("123456789%.5lld0");
	CHECK_STR("1234567890%.5lld");
	CHECK_STR("%.5lld1234567890");
	// vary max and min fielld lengths
	CHECK_STR("%1.3lld");
	CHECK_STR("%2.4lld");		
	CHECK_STR("%3.5lld");
	CHECK_STR("%4.6lld");		
	CHECK_STR("%5.7lld");
	CHECK_STR("%6.8lld");
	CHECK_STR("%7.9lld");
	CHECK_STR("%8.10lld");
	CHECK_STR("%9.11lld");
	CHECK_STR("%10.12lld");
	CHECK_STR("%11.13lld");							
								
   } // end of for loop checking long long int's    

	// now check printing of doubles paramaters (%gGeEAa )
#undef CHECK_STR
#define CHECK_STR(x) check_str_g(x,param)
	printf("printing %%g:\n");	
  // built in sprintf has issues with powers of 10, eg -10000, -1000, 100, 1000, etc so don't test these!
  // list below includes max, smallest normal number, max subnormal number, min subnormal number,zero, inf, nan ie all the special cases.
  double test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,-1e-307,-1e-308,-1e-315,0,4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,-HUGE_VAL,NAN ,-NAN };
  //double test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-10000,-1000,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,0,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,100,345,1000,1001,1002,1002.5,1002.6,10000,100000,100000,1000000,12345678,INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,NAN};
  for(int i=0;i<nos_elements_in(test_valued);++i)
   {double param=test_valued[i]; // test all values in test_value array 
	CHECK_STR("%g");
	CHECK_STR("1%g");
	CHECK_STR("%g1");		
	CHECK_STR("1%g2");
	CHECK_STR("1%g23");
	CHECK_STR("1%g234");
	CHECK_STR("1%g2345");
	CHECK_STR("1%g23456");
	CHECK_STR("1%g234567");
	CHECK_STR("1%g2345678");
	CHECK_STR("1%g23456789");
	CHECK_STR("12%g34567890");	
	CHECK_STR("123%g4567890");
	CHECK_STR("1234%g567890");
	CHECK_STR("12345%g67890");
	CHECK_STR("123456%g7890");
	CHECK_STR("1234567%g890");
	CHECK_STR("12345678%g90");
	CHECK_STR("123456789%g0");
	CHECK_STR("1234567890%g");
	CHECK_STR("%g1234567890");	
	// min flag width of 5
	CHECK_STR("%5g");
	CHECK_STR("1%5g");
	CHECK_STR("%5g1");		
	CHECK_STR("1%5g2");
	CHECK_STR("1%5g23");
	CHECK_STR("1%5g234");
	CHECK_STR("1%5g2345");
	CHECK_STR("1%5g23456");
	CHECK_STR("1%5g234567");
	CHECK_STR("1%5g2345678");
	CHECK_STR("1%5g23456789");
	CHECK_STR("12%5g34567890");	
	CHECK_STR("123%5g4567890");
	CHECK_STR("1234%5g567890");
	CHECK_STR("12345%5g67890");
	CHECK_STR("123456%5g7890");
	CHECK_STR("1234567%5g890");
	CHECK_STR("12345678%5g90");
	CHECK_STR("123456789%5g0");
	CHECK_STR("1234567890%5g");
	CHECK_STR("%5g1234567890");
	// min flag width of 9	
	CHECK_STR("%9g");
	CHECK_STR("1%9g");
	CHECK_STR("%9g1");		
	CHECK_STR("1%9g2");
	CHECK_STR("1%9g23");
	CHECK_STR("1%9g234");
	CHECK_STR("1%9g2345");
	CHECK_STR("1%9g23456");
	CHECK_STR("1%9g234567");
	CHECK_STR("1%9g2345678");
	CHECK_STR("1%9g23456789");
	CHECK_STR("12%9g34567890");	
	CHECK_STR("123%9g4567890");
	CHECK_STR("1234%9g567890");
	CHECK_STR("12345%9g67890");
	CHECK_STR("123456%9g7890");
	CHECK_STR("1234567%9g890");
	CHECK_STR("12345678%9g90");
	CHECK_STR("123456789%9g0");
	CHECK_STR("1234567890%9g");
	CHECK_STR("%9g1234567890");		
	// min field width of 9 left aligned in field
	CHECK_STR("%-9g");
	CHECK_STR("1%-9g");
	CHECK_STR("%-9g1");		
	CHECK_STR("1%-9g2");
	CHECK_STR("1%-9g23");
	CHECK_STR("1%-9g234");
	CHECK_STR("1%-9g2345");
	CHECK_STR("1%-9g23456");
	CHECK_STR("1%-9g234567");
	CHECK_STR("1%-9g2345678");
	CHECK_STR("1%-9g23456789");
	CHECK_STR("12%-9g34567890");	
	CHECK_STR("123%-9g4567890");
	CHECK_STR("1234%-9g567890");
	CHECK_STR("12345%-9g67890");
	CHECK_STR("123456%-9g7890");
	CHECK_STR("1234567%-9g890");
	CHECK_STR("12345678%-9g90");
	CHECK_STR("123456789%-9g0");
	CHECK_STR("1234567890%-9g");
	CHECK_STR("%-9g1234567890");
	// leading zero's with varying field widths
	CHECK_STR("%01g");
	CHECK_STR("1%02g");
	CHECK_STR("%03g1");		
	CHECK_STR("1%04g2");
	CHECK_STR("1%05g23");
	CHECK_STR("1%06g234");
	CHECK_STR("1%07g2345");
	CHECK_STR("1%08g23456");
	CHECK_STR("1%09g234567");
	CHECK_STR("1%01g2345678");
	CHECK_STR("1%02g23456789");
	CHECK_STR("12%03g34567890");	
	CHECK_STR("123%04g4567890");
	CHECK_STR("1234%05g567890");
	CHECK_STR("12345%06g67890");
	CHECK_STR("123456%07g7890");
	CHECK_STR("1234567%08g890");
	CHECK_STR("12345678%09g90");
	CHECK_STR("123456789%010g0");
	CHECK_STR("1234567890%011g");
	CHECK_STR("%012g1234567890");				
	// max field length of 5
	CHECK_STR("%.5g");
	CHECK_STR("1%.5g");
	CHECK_STR("%.5g1");		
	CHECK_STR("1%.5g2");
	CHECK_STR("1%.5g23");
	CHECK_STR("1%.5g234");
	CHECK_STR("1%.5g2345");
	CHECK_STR("1%.5g23456");
	CHECK_STR("1%.5g234567");
	CHECK_STR("1%.5g2345678");
	CHECK_STR("1%.5g23456789");
	CHECK_STR("12%.5g34567890");	
	CHECK_STR("123%.5g4567890");
	CHECK_STR("1234%.5g567890");
	CHECK_STR("12345%.5g67890");
	CHECK_STR("123456%.5g7890");
	CHECK_STR("1234567%.5g890");
	CHECK_STR("12345678%.5g90");
	CHECK_STR("123456789%.5g0");
	CHECK_STR("1234567890%.5g");
	CHECK_STR("%.5g1234567890");
	// vary max and min fieldlengths
	CHECK_STR("%1.3g");
	CHECK_STR("%2.4g");		
	CHECK_STR("%3.5g");
	CHECK_STR("%4.6g");		
	CHECK_STR("%5.7g");
	CHECK_STR("%6.8g");
	CHECK_STR("%7.9g");
	CHECK_STR("%8.10g");
	CHECK_STR("%9.11g");
	CHECK_STR("%10.12g");
	CHECK_STR("%11.13g");							
								
   } // end of for loop checking doubles   

	// now check printing of doubles paramaters with %f or %F
	// checks below are that length matches and ya_snprintf() gives the same values as a double via strtod() [only for pure sprintf]
	// we cannot compare strings (as done above) as most of results below have a large number of digits and its not reasonable to expect them all to be exactly identical
	// use same test values as above
	// also allows max length to be clipped at 350 ... 
#undef CHECK_STR
#define CHECK_STR(x) check_str_f(x,param)	
  printf("printing %%f:\n");	
  for(int i=0;i<nos_elements_in(test_valued);++i)
   {double param=test_valued[i]; // test all values in test_value array 

	// we check values with strtod() if format starts with % , so make most tests that way	
	CHECK_STR("%f");
	CHECK_STR("%fz");		
	CHECK_STR("%fzZ");	
	CHECK_STR("%fzZz");
	CHECK_STR("%fzZzZ");		
	CHECK_STR("%fzZzZz");
	CHECK_STR("1%f2");
	// min field width of 5
	CHECK_STR("%5f");
	CHECK_STR("%5fz");		
	CHECK_STR("%5fzZ");	
	CHECK_STR("%5fzZz");
	CHECK_STR("%5fzZzZ");		
	CHECK_STR("%5fzZzZz");	
	CHECK_STR("1%5f2");
	// min field width of 9	
	CHECK_STR("%9f");
	CHECK_STR("%9fz");		
	CHECK_STR("%9fzZ");	
	CHECK_STR("%9fzZz");
	CHECK_STR("%9fzZzZ");		
	CHECK_STR("%9fzZzZz");	
	CHECK_STR("1%9f2");
	// min field width of 9 left aligned in field
	CHECK_STR("%-9f");
	CHECK_STR("%-9fz");		
	CHECK_STR("%-9fzZ");	
	CHECK_STR("%-9fzZz");
	CHECK_STR("%-9fzZzZ");		
	CHECK_STR("%-9fzZzZz");	
	CHECK_STR("1%-9f2");
	// leading zero's with varying field widths
	CHECK_STR("%01f");
	CHECK_STR("%02f");
	CHECK_STR("%03fz");		
	CHECK_STR("%04fz");
	CHECK_STR("%05fzZ");
	CHECK_STR("%06fzZz");
	CHECK_STR("%07fz2345");
	CHECK_STR("%08fz23456");
	CHECK_STR("%09fz234567");
	CHECK_STR("%010fz");
	CHECK_STR("%012fZ1234567890");
	CHECK_STR("1%05f2");				
	// max flag length of 5
	CHECK_STR("%.5f");
	CHECK_STR("%.5fz");		
	CHECK_STR("%.5fzZ");	
	CHECK_STR("%.5fzZz");
	CHECK_STR("%.5fzZzZ");		
	CHECK_STR("%.5fzZzZz");		
	CHECK_STR("1%.5f2");
	// vary max and min flag lengths
	CHECK_STR("%1.3f");
	CHECK_STR("%2.4f");		
	CHECK_STR("%3.5f");
	CHECK_STR("%4.6f");		
	CHECK_STR("%5.7f");
	CHECK_STR("%6.8f");
	CHECK_STR("%7.9f");
	CHECK_STR("%8.10f");
	CHECK_STR("%9.11f");
	CHECK_STR("%10.12f");
	CHECK_STR("%11.13f");	
	CHECK_STR("%11.100f"); // these can be silly, eg for 1e308 this would print 408 digits
	CHECK_STR("%11.200f");
	CHECK_STR("%11.300f");
	CHECK_STR("%11.400f");
	CHECK_STR("%11.500f");	// as min double is 4.94e-324 this is very silly - but checks no buffers overflow...
	// check more mixtures of flags
	CHECK_STR("%-+5.7f"); // pairs		
	CHECK_STR("%-#5.7f");	
	CHECK_STR("%-05.7f");
	CHECK_STR("%- 5.7f");	
	CHECK_STR("%+#5.7f");	
	CHECK_STR("%+05.7f");	
	CHECK_STR("%+ 5.7f");
	CHECK_STR("%#05.7f");
	CHECK_STR("%# 5.7f");
	CHECK_STR("%0 5.7f");	
	CHECK_STR("%-+#5.7f");	// triples
	CHECK_STR("%-+05.7f");	
	CHECK_STR("%-#05.7f");			
	CHECK_STR("%+#05.7f");	
	CHECK_STR("%-+#05.7f"); //  4
	CHECK_STR("%0#+-5.7f");
	CHECK_STR("%0-+#05.7f"); // OK 05 is a number	
	CHECK_STR("% 0-+#05.7f");
	CHECK_STR("%0-+# 05.7f");
	CHECK_STR("%0- +#05.7f");												
   } // end of for loop checking doubles  with %f
  printf("Checking %%p:\n");
  for(char *cp="hello world";*cp;cp++)
  	{check_str_p("%p",(void *) cp);
	 check_str_p("%2p",(void *) cp);
	 check_str_p("%10p",(void *) cp);
	 check_str_p("%20p",(void *) cp);
	}
  printf("Now doing misc tests %%b, %%'d, %%$d, %%_$d, etc:\n"); 	
  // now do various other tests inline
  { int r_ya,r;// return codes
    static char buf_ya[10000],buf[10000];// need to be big as we check sprintf where result is unlimited
    int i;
    unsigned int u;
    int64_t i64;
    double d;
#ifdef YA_SP_SPRINTF_LD    
    long double ld;
#endif    
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */    
    f128_t d128;
    int128_t i128;
#endif    
	// check invalid flag combination by hand as sprintf considers this format string as valid
    scnt++;
    d=123.4567;
    r_ya=ya_s_sprintf(buf_ya,"%0-+0#05.7f",d);// two 0's
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%0-+0#05.7f(%g) gives <%s> length %d should be length 1\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"#05.7f")) 
		{++serrs;
    	 printf("%%0-+0#05.7f(%g) gives <%s> length %d should give <0> \n",d,buf_ya,r_ya);
    	}   
    r_ya=ya_s_sprintf(buf_ya,"%    5.7f",d); // multiple spaces
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%    5.7f(%g) gives <%s> length %d should be length 1\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"   5.7f")) 
		{++serrs;
    	 printf("%%    5.7f(%g) gives <%s> length %d should give <0> \n",d,buf_ya,r_ya);
    	}   				
    // check %b by hand as this is not supported in sprintf()
    // %b(0, 0x0) gives <0> length 1
	// %b(17, 0x11) gives <10001> length 5
	// %b(-9, 0xfffffff7) gives <11111111111111111111111111110111> length 32
    scnt++;
    u=0;
    r_ya=ya_s_sprintf(buf_ya,"%b",u);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%b(%d, 0x%x) gives <%s> length %d should be length 1\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%b(%d, 0x%x) gives <%s> length %d should give <0> \n",u,u,buf_ya,r_ya);
    	}    	
    scnt++;
    u=17;
    r_ya=ya_s_sprintf(buf_ya,"%b",u);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%b(%d, 0x%x) gives <%s> length %d should be length 5\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10001")) 
		{++serrs;
    	 printf("%%b(%d, 0x%x) gives <%s> length %d should give <10001> \n",u,u,buf_ya,r_ya);
    	}       
    scnt++;
    u=0xeffffff7;
    r_ya=ya_s_sprintf(buf_ya,"%b",u);
    if(r_ya!=32) 
		{++serrs;
    	 printf("%%b(%d, 0x%x) gives <%s> length %d should be length 32\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"11101111111111111111111111110111")) 
		{++serrs;
    	 printf("%%b(%d, 0x%x) gives <%s> length %d should give <11101111111111111111111111110111> \n",u,u,buf_ya,r_ya);
    	}      
	// repeat for %B
    scnt++;
    u=0;
    r_ya=ya_s_sprintf(buf_ya,"%B",u);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%B(%d, 0x%x) gives <%s> length %d should be length 1\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%B(%d, 0x%x) gives <%s> length %d should give <0> \n",u,u,buf_ya,r_ya);
    	}    	
    scnt++;
    u=17;
    r_ya=ya_s_sprintf(buf_ya,"%B",u);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%B(%d, 0x%x) gives <%s> length %d should be length 5\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10001")) 
		{++serrs;
    	 printf("%%B(%d, 0x%x) gives <%s> length %d should give <10001> \n",u,u,buf_ya,r_ya);
    	}       
    scnt++;
    u=0xeffffff7;
    r_ya=ya_s_sprintf(buf_ya,"%B",u);
    if(r_ya!=32) 
		{++serrs;
    	 printf("%%B(%d, 0x%x) gives <%s> length %d should be length 32\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"11101111111111111111111111110111")) 
		{++serrs;
    	 printf("%%B(%d, 0x%x) gives <%s> length %d should give <11101111111111111111111111110111> \n",u,u,buf_ya,r_ya);
    	}  
	/* %#b(0, 0x0) gives <0> length 1
	   %#b(17, 0x11) gives <0b10001> length 7
	   %#b(-9, 0xfffffff7) gives <0b11111111111111111111111111110111> length 34
	*/	
    scnt++;
    u=0;
    r_ya=ya_s_sprintf(buf_ya,"%#b",u);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%#b(%d, 0x%x) gives <%s> length %d should be length 1\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%#b(%d, 0x%x) gives <%s> length %d should give <0> \n",u,u,buf_ya,r_ya);
    	}    	
    scnt++;
    u=17;
    r_ya=ya_s_sprintf(buf_ya,"%#b",u);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%#b(%d, 0x%x) gives <%s> length %d should be length 7\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0b10001")) 
		{++serrs;
    	 printf("%%#b(%d, 0x%x) gives <%s> length %d should give <0b10001> \n",u,u,buf_ya,r_ya);
    	}       
    scnt++;
    u=0xeffffff7;
    r_ya=ya_s_sprintf(buf_ya,"%#b",u);
    if(r_ya!=34) 
		{++serrs;
    	 printf("%%#b(%d, 0x%x) gives <%s> length %d should be length 34\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0b11101111111111111111111111110111")) 
		{++serrs;
    	 printf("%%b(%d, 0x%x) gives <%s> length %d should give <0b11101111111111111111111111110111> \n",u,u,buf_ya,r_ya);
    	}      
	// repeat for %B
    scnt++;
    u=0;
    r_ya=ya_s_sprintf(buf_ya,"%#B",u);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%#B(%d, 0x%x) gives <%s> length %d should be length 1\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%#B(%d, 0x%x) gives <%s> length %d should give <0> \n",u,u,buf_ya,r_ya);
    	}    	
    scnt++;
    u=17;
    r_ya=ya_s_sprintf(buf_ya,"%#B",u);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%#B(%d, 0x%x) gives <%s> length %d should be length 7\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0B10001")) 
		{++serrs;
    	 printf("%%#B(%d, 0x%x) gives <%s> length %d should give <0B10001> \n",u,u,buf_ya,r_ya);
    	}       
    scnt++;
    u=0xeffffff7;
    r_ya=ya_s_sprintf(buf_ya,"%#B",u);
    if(r_ya!=34) 
		{++serrs;
    	 printf("%%#B(%d, 0x%x) gives <%s> length %d should be length 34\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0B11101111111111111111111111110111")) 
		{++serrs;
    	 printf("%%#B(%d, 0x%x) gives <%s> length %d should give <0B11101111111111111111111111110111> \n",u,u,buf_ya,r_ya);
    	}  
	/*
	Like some GCCs, for integers and floats, you can use a ' (single quote)
	specifier and commas will be inserted on the thousands: "%'d" on 12345
	would print 12,345.

	For integers and floats, you can use a "$" specifier and the number
	will be converted to float and then divided to get kilo, mega, giga or
	tera and then printed, so "%$d" 1000 is "1.0 k", "%$.2d" 2536000 is
	"2.53 M", etc. For byte values, use two $:s, like "%$$d" to turn
	2536000 to "2.42 Mi". If you prefer JEDEC suffixes to SI ones, use three
	$:s: "%$$$d" -> "2.42 M". To remove the space between the number and the
	suffix, add "_" specifier: "%_$d" -> "2.53M".			
	*/	 
	/*
	%'d(0, 0x0) gives <0> length 1
	%'d(1000, 0x3e8) gives <1,000> length 5
	%'d(2536000, 0x26b240) gives <2,536,000> length 9
	*/	  
	// start by checking ints  
	i=0;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should be length 1\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should give <0> \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should give <1,000> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=-1000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should give <-1,000> \n",i,i,buf_ya,r_ya);
    	}     
    scnt++;
    i=10000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should give <10,000> \n",i,i,buf_ya,r_ya);
    	}  
    scnt++;
    i=100000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should give <100,000> \n",i,i,buf_ya,r_ya);
    	}  				    	
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=9) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should be length 9\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should give <2,536,000> \n",i,i,buf_ya,r_ya);
    	}   
    scnt++;
    i=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=10) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 printf("%%'d(%d, 0x%x) gives <%s> length %d should give <-2,536,000> \n",i,i,buf_ya,r_ya);
    	} 	
	/*
	 check ' and 0 flags together
	 %'010d(0, 0x0) gives <00,000,000> length 10
	 %'010d(1000, 0x3e8) gives <00,001,000> length 10
	%'010d(2536000, 0x26b240) gives <02,536,000> length 10
	*/ 
    scnt++;
    i=0;
    r_ya=ya_s_sprintf(buf_ya,"%'010d",i);
    if(r_ya!=10) 
		{++serrs;
    	 printf("%%'010d(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"00,000,000")) 
		{++serrs;
    	 printf("%%'010d(%d, 0x%x) gives <%s> length %d should give <00,000,000> \n",i,i,buf_ya,r_ya);
    	} 
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%'010d",i);
    if(r_ya!=10) 
		{++serrs;
    	 printf("%%'010d(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"00,001,000")) 
		{++serrs;
    	 printf("%%'010d(%d, 0x%x) gives <%s> length %d should give <00,001,000> \n",i,i,buf_ya,r_ya);
    	} 
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'010d",i);
    if(r_ya!=10) 
		{++serrs;
    	 printf("%%'010d(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"02,536,000")) 
		{++serrs;
    	 printf("%%'010d(%d, 0x%x) gives <%s> length %d should give <02,536,000> \n",i,i,buf_ya,r_ya);
    	} 
    scnt++;
    i=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'011d",i);
    if(r_ya!=11) 
		{++serrs;
    	 printf("%%'011d(%d, 0x%x) gives <%s> length %d should be length 11\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-02,536,000")) 
		{++serrs;
    	 printf("%%'011d(%d, 0x%x) gives <%s> length %d should give <-02,536,000> \n",i,i,buf_ya,r_ya);
    	} 					    	
    // check unsigned
    scnt++;
    i=-2536000;
    u=i;
    r_ya=ya_s_sprintf(buf_ya,"%'u",u);
    if(r_ya!=13) 
		{++serrs;
    	 printf("%%'u(%u, 0x%x) gives <%s> length %d should be length 13\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"4,292,431,296")) 
		{++serrs;
    	 printf("%%'u(%u, 0x%x) gives <%s> length %d should give <4,292,431,296> \n",u,u,buf_ya,r_ya);
    	} 	    
	// Now check 64 bit ints 
	i=0;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 1\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should give <0> \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should give <1,000> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=-1000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should give <-1,000> \n",i,i,buf_ya,r_ya);
    	}     
    scnt++;
    i=10000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should give <10,000> \n",i,i,buf_ya,r_ya);
    	}  
    scnt++;
    i=100000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should give <100,000> \n",i,i,buf_ya,r_ya);
    	}  				    	
    scnt++;
    i=2536000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=9) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 9\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should give <2,536,000> \n",i,i,buf_ya,r_ya);
    	}   
    scnt++;
    i=-2536000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=10) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 printf("%%'lld(%d, 0x%x) gives <%s> length %d should give <-2,536,000> \n",i,i,buf_ya,r_ya);
    	}	
    // check 64 bit unsigned
    scnt++;
    uint64_t u64=i64;
    r_ya=ya_s_sprintf(buf_ya,"%'llu",u64);
    if(r_ya!=26) 
		{++serrs;
    	 printf("%%'llu(%llu, 0x%llx) gives <%s> length %d should be length 26\n",(long long unsigned int) u64,(long long unsigned int)u64,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"18,446,744,073,707,015,616")) 
		{++serrs;
    	 printf("%%'llu(%llu, 0x%llx) gives <%s> length %d should give <18,446,744,073,707,015,616> \n",(long long unsigned int)u64,(long long unsigned int)u64,buf_ya,r_ya);
    	} 
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */			    	
	// Now check 128 bit ints 
	i=0;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 1\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <0> \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <1,000> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=-1000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <-1,000> \n",i,i,buf_ya,r_ya);
    	}     
    scnt++;
    i=10000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <10,000> \n",i,i,buf_ya,r_ya);
    	}  
    scnt++;
    i=100000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <100,000> \n",i,i,buf_ya,r_ya);
    	}  				    	
    scnt++;
    i=2536000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=9) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 9\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <2,536,000> \n",i,i,buf_ya,r_ya);
    	}   
    scnt++;
    i=-2536000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=10) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 printf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <-2,536,000> \n",i,i,buf_ya,r_ya);
    	}	
    // check 128 bit unsigned
    scnt++;
    uint128_t u128=i128;
    r_ya=ya_s_sprintf(buf_ya,"%'Qu",u128);
    if(r_ya!=51) 
		{++serrs;
    	 printf("%%'Qu(%u, 0x%x) gives <%s> length %d should be length 51\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340,282,366,920,938,463,463,374,607,431,765,675,456")) 
		{++serrs;
    	 printf("%%'Qu(%u, 0x%x) gives <%s> length %d should give <340,282,366,920,938,463,463,374,607,431,765,675,456> \n",i,i,buf_ya,r_ya);
    	} 		
#endif				   				   
	// repeat for doubles
	d=0;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should be length 1\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should give <0> \n",d,buf_ya,r_ya);
    	}    	
    scnt++;
    d=1000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should be length 5\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should give <1,000> \n",d,buf_ya,r_ya);
    	}      
    scnt++;
    d=-1000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should give <-1,000> \n",d,buf_ya,r_ya);
    	}   
    scnt++;
    d=10000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should give <10,000> \n",d,buf_ya,r_ya);
    	}  
    scnt++;
    d=100000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should be length 7\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should give <100,000> \n",d,buf_ya,r_ya);
    	}  				  		 
    scnt++;
    d=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=9) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should be length 9\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should give <2,536,000> \n",d,buf_ya,r_ya);
    	} 	
    scnt++;
    d=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=10) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should be length 10\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 printf("%%'.0f(%g) gives <%s> length %d should give <-2,536,000> \n",d,buf_ya,r_ya);
    	} 	    	
    scnt++;
    d=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.6f",d);// check we only get comma's before the decimal point
    if(r_ya!=16) 
		{++serrs;
    	 printf("%%'.6f(%g) gives <%s> length %d should be length 16\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000.000000")) 
		{++serrs;
    	 printf("%%'.6f(%g) gives <%s> length %d should give <2,536,000.000000> \n",d,buf_ya,r_ya);
    	} 
    scnt++;
    d=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.6f",d);// check we only get comma's before the decimal point
    if(r_ya!=17) 
		{++serrs;
    	 printf("%%'.6f(%g) gives <%s> length %d should be length 17\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000.000000")) 
		{++serrs;
    	 printf("%%'.6f(%g) gives <%s> length %d should give <-2,536,000.000000> \n",d,buf_ya,r_ya);
    	} 	
#ifdef YA_SP_SPRINTF_LD  				
	// repeat for long doubles
	ld=0;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should be length 1\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should give <0> \n",ld,buf_ya,r_ya);
    	}    	
    scnt++;
    ld=1000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should be length 5\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should give <1,000> \n",ld,buf_ya,r_ya);
    	}      
    scnt++;
    ld=-1000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should be length 6\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should give <-1,000> \n",ld,buf_ya,r_ya);
    	}   
    scnt++;
    ld=10000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should be length 6\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should give <10,000> \n",ld,buf_ya,r_ya);
    	}  
    scnt++;
    ld=100000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should be length 7\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should give <100,000> \n",ld,buf_ya,r_ya);
    	}  				  		 
    scnt++;
    ld=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=9) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should be length 9\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should give <2,536,000> \n",ld,buf_ya,r_ya);
    	} 	
    scnt++;
    ld=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=10) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should be length 10\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 printf("%%'.0Lf(%Lg) gives <%s> length %d should give <-2,536,000> \n",ld,buf_ya,r_ya);
    	} 	    	
    scnt++;
    ld=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.6Lf",ld);// check we only get comma's before the decimal point
    if(r_ya!=16) 
		{++serrs;
    	 printf("%%'.6Lf(%Lg) gives <%s> length %d should be length 16\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000.000000")) 
		{++serrs;
    	 printf("%%'.6Lf(%Lg) gives <%s> length %d should give <2,536,000.000000> \n",ld,buf_ya,r_ya);
    	} 
    scnt++;
    ld=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.6Lf",ld);// check we only get comma's before the decimal point
    if(r_ya!=17) 
		{++serrs;
    	 printf("%%'.6Lf(%Lg) gives <%s> length %d should be length 17\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000.000000")) 
		{++serrs;
    	 printf("%%'.6Lf(%Lg) gives <%s> length %d should give <-2,536,000.000000> \n",ld,buf_ya,r_ya);
    	} 	
#endif    	
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */				
	// repeat for float128
	d=0;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should be length 1\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should give <0> \n",d,buf_ya,r_ya);
    	}    	
    scnt++;
    d=1000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should be length 5\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should give <1,000> \n",d,buf_ya,r_ya);
    	}      
    scnt++;
    d=-1000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should give <-1,000> \n",d,buf_ya,r_ya);
    	}   
    scnt++;
    d=10000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should give <10,000> \n",d,buf_ya,r_ya);
    	}  
    scnt++;
    d=100000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should be length 7\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should give <100,000> \n",d,buf_ya,r_ya);
    	}  				  		 
    scnt++;
    d=2536000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=9) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should be length 9\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should give <2,536,000> \n",d,buf_ya,r_ya);
    	} 	
    scnt++;
    d=-2536000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=10) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should be length 10\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 printf("%%'.0Qf(%g) gives <%s> length %d should give <-2,536,000> \n",d,buf_ya,r_ya);
    	} 	    	
    scnt++;
    d=2536000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.6Qf",d128);// check we only get comma's before the decimal point
    if(r_ya!=16) 
		{++serrs;
    	 printf("%%'.6Qf(%g) gives <%s> length %d should be length 16\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000.000000")) 
		{++serrs;
    	 printf("%%'.6Qf(%g) gives <%s> length %d should give <2,536,000.000000> \n",d,buf_ya,r_ya);
    	} 
    scnt++;
    d=-2536000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.6Qf",d128);// check we only get comma's before the decimal point
    if(r_ya!=17) 
		{++serrs;
    	 printf("%%'.6Qf(%g) gives <%s> length %d should be length 17\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000.000000")) 
		{++serrs;
    	 printf("%%'.6Qf(%g) gives <%s> length %d should give <-2,536,000.000000> \n",d,buf_ya,r_ya);
    	} 
#endif    	
	// test $d "base 1000" only for numbers > 1 (k,M,G,T)
	/* %$d(0, 0x0) gives <0 > length 2
	   %$d(1000, 0x3e8) gives <1 k> length 3
	   %$.3d(2536000, 0x26b240) gives <2.536 M> length 7
	*/	
	i=0;
    r_ya=ya_s_sprintf(buf_ya,"%$d",i);
    if(r_ya!=2) 
		{++serrs;
    	 printf("%%$d(%d, 0x%x) gives <%s> length %d should be length 2\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0 ")) 
		{++serrs;
    	 printf("%%$d(%d, 0x%x) gives <%s> length %d should give <0 > \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%$d",i);
    if(r_ya!=3) 
		{++serrs;
    	 printf("%%$d(%d, 0x%x) gives <%s> length %d should be length 3\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1 k")) 
		{++serrs;
    	 printf("%%$d(%d, 0x%x) gives <%s> length %d should give <1 k> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%$.3d",i);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$.3d(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 printf("%%$.3d(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}   
    scnt++;
    i=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%$.3d",i);
    if(r_ya!=8) 
		{++serrs;
    	 printf("%%$.3d(%d, 0x%x) gives <%s> length %d should be length 8\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2.536 M")) 
		{++serrs;
    	 printf("%%$.3d(%d, 0x%x) gives <%s> length %d should give <-2.536 M> \n",i,i,buf_ya,r_ya);
    	}   
    // check unsigned 32 bit	
    scnt++;
    i=-2536000;
    u=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3u",u);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$.3u(%u, 0x%x) gives <%s> length %d should be length 7\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"4.292 G")) 
		{++serrs;
    	 printf("%%$.3u(%u, 0x%x) gives <%s> length %d should give <4.292 G> \n",u,u,buf_ya,r_ya);
    	}   		    	
	// now check i64
    scnt++;
    i=2536000;
    i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3lld",i64);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$.3lld(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 printf("%%$.3lld(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}   
    // now check unsigned i64
    scnt++;
    i= -2536000;
    u64=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3llu",u64);
    if(r_ya!=8) 
		{++serrs;
    	 printf("%%$.3llu(%llu, 0x%llx) gives <%s> length %d should be length 8\n",(long long unsigned int)u64,(long long unsigned int)u64,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"18.447 E")) 
		{++serrs;
    	 printf("%%$.3llu(%llu, 0x%llx) gives <%s> length %d should give <18.447 E> \n",(long long unsigned int)u64,(long long unsigned int)u64,buf_ya,r_ya);
    	}     
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */    	
	// now check i128
    scnt++;
    i=2536000;
    i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3Qd",i128);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$.3Qd(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 printf("%%$.3Qd(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}  
    // now check unsigned 128 bits
    scnt++;
    i=-2536000;
    u128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3Qu",u128);
    r=ya_s_sprintf(buf,"%Qu 0x%Qx",u128,u128);
    if(r_ya!=21) 
		{++serrs;
    	 printf("%%$.3Qu(%s) gives <%s> length %d should be length 21\n",buf,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340282366920938.464 Y")) 
		{++serrs;
    	 printf("%%$.3Qu(%s) gives <%s> length %d should give <340282366920938.464 Y> \n",buf,buf_ya,r_ya);
    	} 
    r_ya=ya_s_sprintf(buf_ya,"%$.3I128u",u128);
    if(r_ya!=21) 
		{++serrs;
    	 printf("%%$.3I128u(%s) gives <%s> length %d should be length 21\n",buf,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340282366920938.464 Y")) 
		{++serrs;
    	 printf("%%$.3I128u(%s) gives <%s> length %d should give <340282366920938.464 Y> \n",buf,buf_ya,r_ya);
    	}     	
#endif				
	// now check double
    scnt++;
    i=2536000;
    d=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3f",d);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$.3f(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 printf("%%$.3f(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}  
#ifdef YA_SP_SPRINTF_LD  		     	
	// now check long double
    scnt++;
    i=2536000;
    ld=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3Lf",ld);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$.3Lf(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 printf("%%$.3Lf(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}  
#endif    	
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */    	
	// now check float128
    scnt++;
    i=2536000;
    d128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3Qf",d128);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$.3Qf(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 printf("%%$.3Qf(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}   
#endif				 			    			 		 			    	
	/* now check $$d - "base 1024" only for numbers > 1 (k,M,G,T)
	  %$$d(0, 0x0) gives <0 > length 2
	  %$$d(1000, 0x3e8) gives <1000 > length 5
	  %$$.3d(2536000, 0x26b240) gives <2.419 Mi> length 8   		
	*/
	i=0;
    r_ya=ya_s_sprintf(buf_ya,"%$$d",i);
    if(r_ya!=2) 
		{++serrs;
    	 printf("%%$$d(%d, 0x%x) gives <%s> length %d should be length 2\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0 ")) 
		{++serrs;
    	 printf("%%$$d(%d, 0x%x) gives <%s> length %d should give <0 > \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%$$d",i);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%$$d(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1000 ")) 
		{++serrs;
    	 printf("%%$$d(%d, 0x%x) gives <%s> length %d should give <1000 > \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%$$.3d",i);
    if(r_ya!=8) 
		{++serrs;
    	 printf("%%$$.3d(%d, 0x%x) gives <%s> length %d should be length 8\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 Mi")) 
		{++serrs;
    	 printf("%%$$.3d(%d, 0x%x) gives <%s> length %d should give <2.419 Mi> \n",i,i,buf_ya,r_ya);
    	}  	
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */    	
    // check i128
    scnt++;
    i=2536000;
    i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$$.3Qd",i128);
    if(r_ya!=8) 
		{++serrs;
    	 printf("%%$$.3Qd(%d, 0x%x) gives <%s> length %d should be length 8\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 Mi")) 
		{++serrs;
    	 printf("%%$$.3Qd(%d, 0x%x) gives <%s> length %d should give <2.419 Mi> \n",i,i,buf_ya,r_ya);
    	}  	
    // check float128	
    scnt++;
    i=2536000;
    d128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$$.3Qf",d128);
    if(r_ya!=8) 
		{++serrs;
    	 printf("%%$$.3Qf(%d, 0x%x) gives <%s> length %d should be length 8\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 Mi")) 
		{++serrs;
    	 printf("%%$$.3Qf(%d, 0x%x) gives <%s> length %d should give <2.419 Mi> \n",i,i,buf_ya,r_ya);
    	}  	
#endif						
    /* $$$d "base 1024" with Jedec suffixes , only for values > 1 and k,M,G,T 
      %$$$d(0, 0x0) gives <0 > length 2
	  %$$$d(1000, 0x3e8) gives <1000 > length 5
	  %$$$.3d(2536000, 0x26b240) gives <2.419 M> length 7
	*/
	i=0;
    r_ya=ya_s_sprintf(buf_ya,"%$$$d",i);
    if(r_ya!=2) 
		{++serrs;
    	 printf("%%$$$d(%d, 0x%x) gives <%s> length %d should be length 2\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0 ")) 
		{++serrs;
    	 printf("%%$$$d(%d, 0x%x) gives <%s> length %d should give <0 > \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%$$$d",i);
    if(r_ya!=5) 
		{++serrs;
    	 printf("%%$$$d(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1000 ")) 
		{++serrs;
    	 printf("%%$$$d(%d, 0x%x) gives <%s> length %d should give <1000 > \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%$$$.3d",i);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$$$.3d(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 M")) 
		{++serrs;
    	 printf("%%$$$.3d(%d, 0x%x) gives <%s> length %d should give <2.419 M> \n",i,i,buf_ya,r_ya);
    	}  	
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */    	
	// check i128
    scnt++;
    i=2536000;
    i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$$$.3Qd",i128);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$$$.3d(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 M")) 
		{++serrs;
    	 printf("%%$$$.3d(%d, 0x%x) gives <%s> length %d should give <2.419 M> \n",i,i,buf_ya,r_ya);
    	}  			  
 	// check f128
    scnt++;
    i=2536000;
    d128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$$$.3Qf",d128);
    if(r_ya!=7) 
		{++serrs;
    	 printf("%%$$$.3Qf(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 M")) 
		{++serrs;
    	 printf("%%$$$.3Qf(%d, 0x%x) gives <%s> length %d should give <2.419 M> \n",i,i,buf_ya,r_ya);
    	} 
#endif		   	
	/* check %_$$$d the _ deleted the space before the metric suffix otherwise identical to $$$ above
	  %_$$$d(0, 0x0) gives <0> length 1
	  %_$$$d(1000, 0x3e8) gives <1000> length 4
	  %_$$$.3d(2536000, 0x26b240) gives <2.419M> length 6    	
	*/
	i=0;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$d",i);
    if(r_ya!=1) 
		{++serrs;
    	 printf("%%_$$$d(%d, 0x%x) gives <%s> length %d should be length 1\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 printf("%%_$$$d(%d, 0x%x) gives <%s> length %d should give <0> \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$d",i);
    if(r_ya!=4) 
		{++serrs;
    	 printf("%%_$$$d(%d, 0x%x) gives <%s> length %d should be length 4\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1000")) 
		{++serrs;
    	 printf("%%_$$$d(%d, 0x%x) gives <%s> length %d should give <1000> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3d",i);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%_$$$.3d(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 printf("%%_$$$.3d(%d, 0x%x) gives <%s> length %d should give <2.419M> \n",i,i,buf_ya,r_ya);
    	}  	 
	// repeat for i64
    scnt++;
    i=2536000;
    i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3lld",i64);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%_$$$.3lld(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 printf("%%_$$$.3lld(%d, 0x%x) gives <%s> length %d should give <2.419M> \n",i,i,buf_ya,r_ya);
    	}  	
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */				 
    // repeat for i128	
    scnt++;
    i=2536000;
    i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3Qd",i128);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%_$$$.3Qd(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 printf("%%_$$$.3Qd(%d, 0x%x) gives <%s> length %d should give <2.419M> \n",i,i,buf_ya,r_ya);
    	}  
#endif					 
	// repeat for double with %f
    scnt++;
    d=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3f",d);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%_$$$.3f(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 printf("%%_$$$.3f(%g) gives <%s> length %d should give <2.419M> \n",d,buf_ya,r_ya);
    	} 	
#ifdef YA_SP_SPRINTF_LD      	
	// check long double
    scnt++;
    d=2536000;
    ld=d;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3Lf",ld);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%_$$$.3Lf(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 printf("%%_$$$.3Lf(%g) gives <%s> length %d should give <2.419M> \n",d,buf_ya,r_ya);
    	} 	
#endif    	
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */		    	
	// check f128
    scnt++;
    d=2536000;
    d128=d;
    ld=d;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3Qf",d128);
    if(r_ya!=6) 
		{++serrs;
    	 printf("%%_$$$.3Qf(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 printf("%%_$$$.3Qf(%g) gives <%s> length %d should give <2.419M> \n",d,buf_ya,r_ya);
    	}  
#endif		  	
	// check multiple variables printed by comparing to sprintf
	printf("Now checking printing multiple items:\n");
	scnt++;
	i=123;d=5.67;
	char *fstr="%5d %3.1g\n";// int then double (with a range of different formats applied)
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 	
 	scnt++;
	fstr="%5d %g\n";
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	 
 	
 	scnt++;
	fstr="%d %g\n"; 	
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	 
 	
 	scnt++;
	fstr="%d %-.2g\n"; 	
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 	
 	scnt++;
	fstr="%-6d %g\n"; 	
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
	
	scnt++;
	fstr="%2.1g %d\n"; 	// double then int
	r=sprintf(buf,fstr,d,i);
    r_ya=ya_s_sprintf(buf_ya,fstr,d,i);
 	if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 	
 	scnt++;
	fstr="%6d %d\n"; // two ints	
	r=sprintf(buf,fstr,i,i);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,i);
 	if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 	
	printf("Now checking variable precision %%*.* :\n"); 	 	 	 	 	
	for(int j=-100;j<=100;++j) // -ve j sets - flag (left justified in field)
		for(int k=-2;k<=100; ++k) // -ve k acts as if precision were omitted, start from -2 in case -1 is used as a default flag in "printf" code.
			{scnt++;
			 fstr="%#*.*g %0*d"; 	// double (%g) then int. Note %#g is required as by default %g does not print trailing zeros which makes the lengths different for sprintf and ya_sprintf
			 r=sprintf(buf,fstr,j,k,d,j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,j+k,i);
 			 if(r!=r_ya){ ++serrs;printf ("%s (*=%d,%d,%d): sprintf() returns %d ya_sprintf() returns %d\n",fstr,j,k,j+k,r,r_ya);}
#ifdef YA_SP_SPRINTF_LD  
 			 if(k<20 && strcmp(buf,buf_ya)) {++serrs;printf("%s (*=%d,%d,%d): sprintf() gives %s ya_sprintf() gives %s\n",fstr,j,k,j+k,buf,buf_ya);};	// %g accurate to 19 sf (18 digits after dp
#else
			 if(k<19 && strcmp(buf,buf_ya)) {++serrs;printf("%s (*=%d,%d,%d): sprintf() gives %s ya_sprintf() gives %s\n",fstr,j,k,j+k,buf,buf_ya);}; // without LD defined results are not quite as accurate
#endif 			 
			 scnt++;
			 fstr="%*.*f %0*d"; 	// double (%f) then int
			 r=sprintf(buf,fstr,j,k,d,j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,j+k,i);
 			 if(r!=r_ya){ ++serrs;printf ("%s (*=%d,%d,%d): sprintf() returns %d ya_sprintf() returns %d\n",fstr,j,k,j+k,r,r_ya);}
#ifdef YA_SP_SPRINTF_LD   			 
 			 if(k<19 && strcmp(buf,buf_ya)) {++serrs;printf("%s (*=%d,%d,%d): sprintf() gives %s ya_sprintf() gives %s\n",fstr,j,k,j+k,buf,buf_ya);};	 
#else
			 if(k<18 && strcmp(buf,buf_ya)) {++serrs;printf("%s (*=%d,%d,%d): sprintf() gives %s ya_sprintf() gives %s\n",fstr,j,k,j+k,buf,buf_ya);};
#endif 			 
			 scnt++;
			 fstr="%*.*e %0*d"; 	// double (%e) then int
			 r=sprintf(buf,fstr,j,k,d,j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,j+k,i);
 			 if(r!=r_ya){ ++serrs;printf ("%s (*=%d,%d,%d): sprintf() returns %d ya_sprintf() returns %d\n",fstr,j,k,j+k,r,r_ya);}
#ifdef YA_SP_SPRINTF_LD   			 
 			 if(k<19 && strcmp(buf,buf_ya)) {++serrs;printf("%s (*=%d,%d,%d): sprintf() gives %s ya_sprintf() gives %s\n",fstr,j,k,j+k,buf,buf_ya);};			  			 
#else
			 if(k<18 && strcmp(buf,buf_ya)) {++serrs;printf("%s (*=%d,%d,%d): sprintf() gives %s ya_sprintf() gives %s\n",fstr,j,k,j+k,buf,buf_ya);};
#endif 			 
			}
	
	printf("Now checking %%n:\n");
	union {int ui;
		   signed char uc;
		   short int u_s;
		   long int ul;
		   long long int ull;
		  } un,ya_un; // used to get the result of %n and check the correct type was returned
 
	for(int j=1;j<=20;++j)
		for(int k=1;k<=10; ++k)
			{scnt++;
			 fstr="%-*.*g %n %0*d"; 	// double then int with basic %n
			 un.ull=INT64_C(-1); // -1 has all bits set so we can easily see if its been overwritten by writing to another variable in the union 
			 ya_un.ull=INT64_C(-1);	 
			 r=sprintf(buf,fstr,j,k,d,&(un.ui),j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&(ya_un.ui),j+k,i);
 			 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;printf ("%s: sprintf(%%n) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);}
 			 scnt++;
			 fstr="%-*.*g %hhn %0*d\n"; 	// double then int with %hhn which expects char *
			 un.ull=INT64_C(-1); // -1 has all bits set so we can easily see if its been overwritten by writing to another variable in the union 
			 ya_un.ull=INT64_C(-1);	 
			 r=sprintf(buf,fstr,j,k,d,&(un.uc),j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&(ya_un.uc),j+k,i);
    		 // printf("%%hhn: un.uc=%hhx un.ull=%llx ya_un.uc=%hhx ya_un.ull=%llx\n",un.uc,un.ull,ya_un.uc,ya_un.ull);
 			 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;printf ("%s: sprintf(%%hhn) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);} 	
 			 scnt++;
			 fstr="%-*.*g %hn %0*d\n"; 	// double then int with %hn which expects short *
			 un.ull=INT64_C(-1); // -1 has all bits set so we can easily see if its been overwritten by writing to another variable in the union 
			 ya_un.ull=INT64_C(-1);	 
			 r=sprintf(buf,fstr,j,k,d,&(un.u_s),j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&(ya_un.u_s),j+k,i);
 			 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;printf ("%s: sprintf(%%hn) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);} 				  		 
 			 scnt++;
			 fstr="%-*.*g %ln %0*d\n"; 	// double then int with %ln which expects long *
			 un.ull=INT64_C(-1); // -1 has all bits set so we can easily see if its been overwritten by writing to another variable in the union 
			 ya_un.ull=INT64_C(-1);	 
			 r=sprintf(buf,fstr,j,k,d,&(un.ul),j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&(ya_un.ul),j+k,i);
 			 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;printf ("%s: sprintf(%%ln) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);} 	 			 
 			 scnt++;
			 fstr="%-*.*g %lln %0*d\n"; 	// double then int with %hhn which expects long long *
			 un.ull=INT64_C(-1); // -1 has all bits set so we can easily see if its been overwritten by writing to another variable in the union 
			 ya_un.ull=INT64_C(-1);	 
			 r=sprintf(buf,fstr,j,k,d,&(un.ull),j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&(ya_un.ull),j+k,i);
 			 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;printf ("%s: sprintf(%%lln) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);} 	 
#ifdef  YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */
			 __int128 i128=-1;// set to a known wrong value to check its changed below.
			 fstr="%-*.*g %Qn %0*d\n"; 	// double then int with %Qn which expects int128 *
			 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&i128,j+k,i); // sprintf does not support %Q but values produced should otherwise be the same as for long long int above
			 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
			 if(un.ull!=i128){ ++serrs;printf ("%s: sprintf(%%lln) returns %llx ya_sprintf(%%Qn) returns %llx\n",fstr,un.ull,(long long int)i128);} 	
			 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
			 fstr="%-*.*g %I128n %0*d\n"; 	// double then int with %I128n which expects int128 *
			 i128=-1; // set to a known wrong value to check its changed below.
			 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&i128,j+k,i);
			 if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
			 if(un.ull!=i128){ ++serrs;printf ("%s: sprintf(%%lln) returns %llx ya_sprintf(%%I128n) returns %llx\n",fstr,un.ull,(long long int)i128);} 	
			 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};				 
#endif			  			 
			}
	 printf("Checking round loop accuracy of %%a and %%A with fast_strtod():\n");
	 {
	  double d,dr;
	  float f,fr;
	  char *endp;
      //%a  for fast_strtod()
      d=0; // check 0 1st then a "selection" of numbers
      f=d;
      scnt++;
	  ya_s_sprintf(buf,"%a",d);
	  dr=fast_strtod(buf,&endp);
	  if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(d!=dr) {++serrs;printf("%%a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	  scnt++;
	  fr=fast_strtof(buf,&endp);
	  if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	  scnt++;
	  ya_s_sprintf(buf,"%A",d);
	  dr=fast_strtod(buf,&endp);
	  if(*endp) {++serrs;printf("%%A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(d!=dr) {++serrs;printf("%%A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	   
	  scnt++;
	  fr=fast_strtof(buf,&endp);
	  if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}		     
      // check %#a/A
	  scnt++;
	  ya_s_sprintf(buf,"%#a",d);
	  dr=fast_strtod(buf,&endp);
	  if(*endp) {++serrs;printf("%%#a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(d!=dr) {++serrs;printf("%%#a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	  scnt++;
	  fr=fast_strtof(buf,&endp);
	  if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}		  
	  scnt++;
	  ya_s_sprintf(buf,"%#A",d);
	  dr=fast_strtod(buf,&endp);
	  if(*endp) {++serrs;printf("%%#A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(d!=dr) {++serrs;printf("%%#A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}
	  scnt++;
	  fr=fast_strtof(buf,&endp);
	  if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}		  	   
	  	 
	  for(d=2;d<=1e308;d*=10)
	 	{f=d;
		 scnt++;	
	 	 ya_s_sprintf(buf,"%a",d);
	 	 dr=fast_strtod(buf,&endp);
	 	 if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	 	 if(d!=dr) {++serrs;printf("%%a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}		 	 
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%A",d);
	 	 dr=fast_strtod(buf,&endp);
	 	 if(*endp) {++serrs;printf("%%A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	 	 if(d!=dr) {++serrs;printf("%%A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}			  	 
		  
      	 // check %#a/A
	  	 scnt++;
	  	 ya_s_sprintf(buf,"%#a",d);
	  	 dr=fast_strtod(buf,&endp);
	  	 if(*endp) {++serrs;printf("%%#a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(d!=dr) {++serrs;printf("%%#a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}		  	 
	  	 scnt++;
	  	 ya_s_sprintf(buf,"%#A",d);
	  	 dr=fast_strtod(buf,&endp);
	  	 if(*endp) {++serrs;printf("%%#A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(d!=dr) {++serrs;printf("%%#A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}			   	  	 
	 	}
	  for(d=2;d>=1e-323;d/=10)
	 	{f=d;
		 scnt++;
	 	 ya_s_sprintf(buf,"%a",d);
	 	 dr=fast_strtod(buf,&endp);
	 	 if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	 	 if(d!=dr) {++serrs;printf("%%a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}		 	 
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%A",d);
	 	 dr=fast_strtod(buf,&endp);
	 	 if(*endp) {++serrs;printf("%%A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	 	 if(d!=dr) {++serrs;printf("%%A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(f!=fr) {++serrs;printf("%%a: (%g)=%s fast_strtof() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}			  	   	
	 	 // no point checking %#a here as decimal point will be needed for these cases anyway
	 	}
	}
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */	
	{ printf("Basic checks for %%L and %%Q suffixes\n");
	 unsigned int start_scnt=scnt,start_serrs=serrs;
	 double d;
	 long double dl;
	 __float128 d128;
	 // 1st check exponent is correct (using long doubles) 
	 for(int e=-16382;e<16384;++e)
	 	{dl=ldexpl(1.5,e); // iee mantissa is 1.f*2^e so 1.5 can be represented exactly
	 	 ++scnt; 		 
		 fstr="%.4Le"; // only .4 as we are not checking accuracy of mantissa, just decimal exponent
		 r=sprintf(buf,fstr,dl);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf(1.5*2^%d) gives %s ya_sprintf() gives %s\n",fstr,e,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf(1.5*2^%d) returns %d ya_sprintf() returns %d\n",fstr,e,r,r_ya);}	
		}
	 __float128 test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,-1e-307,-1e-308,-1e-315,0,4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,NAN,
		-FLT128_MAX,-1e+4932F128,-1e+4930F128, -1.7976931348623157e308F128 ,-12345678.F128 ,-10001.F128 ,-9999.F128 ,-1001.F128 ,-345.F128 ,-100.F128 ,-99.F128 ,-20.F128 ,-10.F128 ,-9.F128 ,-8.F128 ,-7.F128 ,-6.F128 ,-5.F128 ,-4.F128 ,-3.F128 ,-2.F128 ,-1.F128 ,-0.5F128 ,
		-0.1F128 ,-1e-307F128 ,-1e-308F128 ,-1e-315F128,-FLT128_MIN,-6.47517511943802511092443895822764655e-4956F128,-FLT128_DENORM_MIN ,0.F128,FLT128_DENORM_MIN,6.47517511943802511092443895822764655e-4956F128,
		 6.47517511943802511092443895822764655e-4946F128, 6.47517511943802511092443895822764655e-4936F128,  FLT128_MIN ,4.9406564584124654e-324F128 ,1e-315F128 ,2.2250738585072009e-308F128 ,
		2.2250738585072014e-308F128 ,0.1F128 ,0.5F128 ,1.F128 ,2.F128 ,3.F128 ,4.F128 ,5.F128 ,6.F128 ,7.F128 ,8.F128 ,9.F128 ,10.F128 ,20.F128 ,99.F128 ,100.F128,101.F128 ,345.F128 ,999.F128 ,
		1000.F128,1001.F128 ,1002.F128 ,1002.5F128 ,1002.6F128 ,10000.F128,10001.F128 ,100001.F128 ,100001.F128 ,1000001.F128 ,12345678.F128 ,1.7976931348623157e308F128,1e+4930F128,1e+4932F128,FLT128_MAX 
		};// 6.475175e-4946 is denorm so gives slightly different results with long double and float128 as float128 has a lot more mantissa digits
	  	for(int i=0;i<nos_elements_in(test_valued);++i)
		{d128=test_valued[i];
		 dl=(long double) d128;
		 d=(double) d128;
		 /* do %g 1st */
		 // first check __float128
		 ++scnt;
		 fstr="%Qg";
		 // r=sprintf(buf,fstr,d128);
		 r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(!isnan(dl) &&(d128==6.47517511943802511092443895822764655e-4946F128 || (dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) 
		 	{++expected_errs;
#ifdef PR_EXPECTED_ERRORS		 	
			 printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);
#endif			 
			}		 
		 else if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
 		 ++scnt;// now check %I128 as alternative to %Q
		 fstr="%I128g";
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(!isnan(dl) &&(d128==6.47517511943802511092443895822764655e-4946F128 || (dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) 
		 	{++expected_errs;
#ifdef PR_EXPECTED_ERRORS		 	
			 printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);
#endif			 
			}		 
		 else if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
 		 		 	
 		 // now check long double
		 ++scnt; 		 
		 fstr="%Lg";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%g";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 		
		 /* do %e  */
		 // first check __float128
		 ++scnt;
		 fstr="%Qe";
		 // r=sprintf(buf,fstr,d128);
		 r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(!isnan(dl) &&(d128==6.47517511943802511092443895822764655e-4946F128 || (dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) 
		 	{++expected_errs;
#ifdef PR_EXPECTED_ERRORS		 	
			 printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);
#endif			 
			}		 
		 else if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		 ++scnt;
		 fstr="%I128e";
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(!isnan(dl) &&(d128==6.47517511943802511092443895822764655e-4946F128 || (dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) 
		 	{++expected_errs;
#ifdef PR_EXPECTED_ERRORS		 	
			 printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);
#endif			 
			}		 
		 else if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 er
 		 	
 		 // now check long double
		 ++scnt; 		 
		 fstr="%Le";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 //if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%e";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 
		 /* do %f note we use strncmp here as %f may generate very long numbers but only expect first few digits to match exactly */
		 // first check __float128
		 ++scnt;
		 fstr="%Qf";
		 // r=sprintf(buf,fstr,d128);
		 r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(!isnan(dl) &&(/*(d==0 && dl!=0) ||*/ (isinf(dl) && !isinfq(d128) ) )) 
		 	{++expected_errs;
#ifdef PR_EXPECTED_ERRORS		 	
			 printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);
#endif			 
			}
		 else if(strncmp(buf,buf_ya,10)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		 ++scnt;
		 fstr="%I128f";
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(!isnan(dl) &&(/*(d==0 && dl!=0) ||*/ (isinf(dl) && !isinfq(d128) ) )) 
		 	{++expected_errs;
#ifdef PR_EXPECTED_ERRORS		 	
			 printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);
#endif			 
			}
		 else if(strncmp(buf,buf_ya,10)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		  		 	
 		 // now check long double
		 ++scnt; 		 
		 fstr="%Lf";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 //if(!isnan(d) &&(/*(d==0 && dl!=0) ||*/ (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strncmp(buf,buf_ya,12)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%f";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strncmp(buf,buf_ya,12)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 
		 		 /* do %a  */
#if 1 // quadmath_snprintf(%a) prints 1.xxx whereas ya_s_snprintf(%a) prints 8.xxx 		 		 
		 // first check __float128
		 ++scnt;
		 fstr="%.14Qa"; // progess as long double so limited in resolution [ %.15Q gives 2 errors due to "rounding" ], 14 is OK
		 // r=sprintf(buf,fstr,d128);
		 r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 // if(!isnan(dl) &&((dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		 ++scnt;
		 fstr="%Qa"; // full resolution
		 // r=sprintf(buf,fstr,d128);
		 r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 // if(!isnan(dl) &&((dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test		
		 ++scnt;
		 fstr="%I128a"; // full resolution
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 // if(!isnan(dl) &&((dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test			  
#endif 		 	
 		 // now check long double
		 ++scnt; 		 
		 fstr="%La";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // no expected errors as %a code for long doubles is in place
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%a";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}			 			    
		}
	  if(serrs-start_serrs==0)
		printf("%u sprintf tests completed on %%L and %%Q, no unexpected errors found ( %u expected errors found)\n",scnt-start_scnt,expected_errs);
      else	
		printf("%u sprintf tests completed on %%L and %%Q, %u unexpected errors found (+ %u expected errors)\n",scnt-start_scnt,serrs-start_serrs,expected_errs);	
	 } 
#else // only 32 bit compiler - can check %L but not %Q
#ifdef YA_SP_SPRINTF_LD  
	{ printf("Basic checks for %%L  suffix\n");
	 unsigned int start_scnt=scnt,start_serrs=serrs;
	 double d;
	 long double dl;
	 // 1st check exponent is correct (using long doubles) 
	 for(int e=-16382;e<16384;++e)
	 	{dl=ldexpl(1.5,e); // iee mantissa is 1.f*2^e so 1.5 can be represented exactly
	 	 ++scnt; 		 
		 fstr="%.4Le"; // only .4 as we are not checking accuracy of mantissa, just decimal exponent
		 r=sprintf(buf,fstr,dl);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf(1.5*2^%d) gives %s ya_sprintf() gives %s\n",fstr,e,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf(1.5*2^%d) returns %d ya_sprintf() returns %d\n",fstr,e,r,r_ya);}	
		}
	 long double test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,-1e-307,-1e-308,-1e-315,0,4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,NAN,
		-1e+4932L,-1e+4930L, -1.7976931348623157e308L ,
		-0.1L ,-1e-307L ,-1e-308L ,-1e-315L,-6.475175e-4946L,-3.64519953188247460253e-4951L,0.L,3.64519953188247460253e-4951L,6.475175e-4946L,
		1.7976931348623157e308L,1e+4930L,1e+4932L 
		};// 6.475175e-4946 is denorm so gives slightly different results with long double and float128 as float128 has a lot more mantissa digits
	  	for(int i=0;i<nos_elements_in(test_valued);++i)
		{dl=test_valued[i];
		 d=(double) dl;
		 /* do %g 1st */
		  		 // now check long double
		 ++scnt; 		 
		 fstr="%Lg";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%g";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 		
		 /* do %e  */
 		 // now check long double
		 ++scnt; 		 
		 fstr="%Le";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 //if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%e";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 
		 /* do %f note we use strncmp here as %f may generate very long numbers but only expect first few digits to match exactly */
 		 // now check long double
		 ++scnt; 		 
		 fstr="%Lf";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 //if(!isnan(d) &&(/*(d==0 && dl!=0) ||*/ (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strncmp(buf,buf_ya,12)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%f";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strncmp(buf,buf_ya,12)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 
		 		 /* do %a  */
 		 	
 		 // now check long double
		 ++scnt; 		 
		 fstr="%La";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // no expected errors as %a code for long doubles is in place
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%a";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}			 			    
		}
	  if(serrs-start_serrs==0)
		printf("%u sprintf tests completed on %%L, no unexpected errors found ( %u expected errors found)\n",scnt-start_scnt,expected_errs);
      else	
		printf("%u sprintf tests completed on %%L, %u unexpected errors found (+ %u expected errors)\n",scnt-start_scnt,serrs-start_serrs,expected_errs);	
	 } 	 
#endif	 
#endif
	 
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
#ifndef __SIZEOF_INT128__ 
#define TYPE_MAX(t) \
  ((t) (! TYPE_SIGNED (t) \
        ? (t) -1 \
        : ~ (~ (unsigned t) 0 << (sizeof (t) * CHAR_BIT - 1))))        
#else
#define TYPE_MAX(t) \
  ((t) (! TYPE_SIGNED (t) \
        ? (t) -1 \
        : (t) ~ (~ (__uint128_t) 0 << (sizeof (t) * CHAR_BIT - 1))))       
#endif    
        		 
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */	
	{uint128_t ui128=TYPE_MAX(uint128_t);	 
	 printf("Testing %%QxXob [128 integers]:\n");
	 // %x
	 scnt++;
	 r=sprintf(buf,"%016"PRIx64"%016"PRIx64,(uint64_t)(ui128>>64),(uint64_t)ui128);
	 fstr="%032Qx";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qx max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	   
	 fstr="%032I128x";// repeat with I128 instead Q
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qx max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	   	   
	 // %X
	 scnt++;
	 r=sprintf(buf,"%016"PRIX64"%016"PRIX64,(uint64_t)(ui128>>64),(uint64_t)ui128);
	 fstr="%032QX";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%QX max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
	// %o
	 scnt++;
	 // more complex than hex as need to split at boundries that can be represented exactly in whole octal digits
	 r=sprintf(buf,"%01o%021"PRIo64"%021"PRIo64,(uint32_t)(ui128>>126),(uint64_t)(ui128>>63)& 0x7fffffffffffffff,((uint64_t)ui128) & 0x7fffffffffffffff );
	 fstr="%043Qo";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qo max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
	// %b
	 scnt++;
	 // sprintf cannot convert to binary so do a simpler check on basis we expect a string with all 1's in it
	 char * bp;
	 fstr="%043Qb";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qb max128 ya_sprintf gives <%s> length %d\n",buf_ya,r_ya);
     bp=strrchr(buf_ya,'1'); // pointer to last 1
	 if(bp[1]!=0) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(128!=r_ya){ ++serrs;printf ("%s: ya_sprintf() returns %d should return 128\n",fstr,r_ya);}	 
	 // now check with zero	 
	 ui128=0;
	 scnt++;
	 fstr="%032Qx";
	 r=sprintf(buf,"%016"PRIx64"%016"PRIx64,(uint64_t)(ui128>>64),(uint64_t)ui128);
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qx 0 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}		 
	 	 // %X
	 scnt++;
	 r=sprintf(buf,"%016"PRIX64"%016"PRIX64,(uint64_t)(ui128>>64),(uint64_t)ui128);
	 fstr="%032QX";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%QX max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
	// %o
	 scnt++;
	 // more complex than hex as need to split at boundries that can be represented exactly in whole octal digits
	 r=sprintf(buf,"%01o%021"PRIo64"%021"PRIo64,(uint32_t)(ui128>>126),(uint64_t)(ui128>>63)& 0x7fffffffffffffff,((uint64_t)ui128) & 0x7fffffffffffffff );
	 fstr="%043Qo";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qo max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
	// %b
	 scnt++;
	 // sprintf cannot convert to binary so do a simpler check on basis we expect a string with just 0 in it
	 fstr="%Qb";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qb max128 ya_sprintf gives <%s> length %d\n",buf_ya,r_ya);
	 if(strcmp("0",buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(1!=r_ya){ ++serrs;printf ("%s: ya_sprintf() returns %d should return 1\n",fstr,r_ya);}	 
	 
	 
	 for(ui128=1;ui128!=TYPE_MAX(uint128_t);ui128=(ui128<<1)|1)
	 	{// try a range of numbers for x,X,o (cannot do b in general as sprintf does not support it)
	 	 // %x
	 	 scnt++;
	 	 fstr="%032Qx";
	 	 r=sprintf(buf,"%016"PRIx64"%016"PRIx64,(uint64_t)(ui128>>64),(uint64_t)ui128);
     	 r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     	 //printf("%%Qx 111... sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
	 	 // %X
		 scnt++;
		 r=sprintf(buf,"%016"PRIX64"%016"PRIX64,(uint64_t)(ui128>>64),(uint64_t)ui128);
		 fstr="%032QX";
	     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
	     //printf("%%QX max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 // %o
		 scnt++;
		 // more complex than hex as need to split at boundries that can be represented exactly in whole octal digits
		 r=sprintf(buf,"%01o%021"PRIo64"%021"PRIo64,(uint32_t)(ui128>>126),(uint64_t)(ui128>>63)& 0x7fffffffffffffff,((uint64_t)ui128) & 0x7fffffffffffffff );
		 fstr="%043Qo";
	     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
	     //printf("%%Qo max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 	 
	 	}
	}
	// the builtin sprintf() cannot print int128's so use the two functions below to do this 
	// int sprint_uint128_decimal(char *s,uint128_t big); /* print a uint128 to string which needs to be at least 40 character [including trailing null] */
    // int sprint_int128_decimal(char *s,int128_t big); /* signed sprint, needs min 41 character buffer as potential leading minus sign */
	{uint128_t ui128=TYPE_MAX(uint128_t);	 
	 int128_t i128=TYPE_MAX(int128_t);
	 printf("Testing %%Quid [128 integers to decimal]:\n");
	 // %u
	 scnt++;
	 r=sprint_uint128_decimal(buf,ui128);
	 fstr="%Qu";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     // printf("%%Qu max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	      
	 // %d
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qd";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
	 fstr="%I128d";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 	 
	 // %i
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qi";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
	 // now check -max (signed only)
	 i128= ~i128; // 1's complement gives most negative number
	 // %d
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qd";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
	 // %i
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qi";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 	 
	 // now check zero:
	 ui128=0;
	 i128=0;
	 // %u
	 scnt++;
	 r=sprint_uint128_decimal(buf,ui128);
	 fstr="%Qu";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qu max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	      
	 // %d
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qd";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
	 // %i
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qi";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 	
	 // now try for a reasonable range of numbers	 
	 for(ui128=1;ui128!=TYPE_MAX(uint128_t);ui128=(ui128<<1)|1)
	 	{i128=ui128;
	 	 // %u
		 scnt++;
		 r=sprint_uint128_decimal(buf,ui128);
		 fstr="%Qu";
	     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
	     //printf("%%Qu max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	      
		 // %d
		 scnt++;
		 r=sprint_int128_decimal(buf,i128);
		 fstr="%Qd";
	     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
	     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
		 // %i
		 scnt++;
		 r=sprint_int128_decimal(buf,i128);
		 fstr="%Qi";
	     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
	     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 // check negative numbers
		 i128= -(int128_t)ui128;
		 // %d
		 scnt++;
		 r=sprint_int128_decimal(buf,i128);
		 fstr="%Qd";
	     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
	     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
		 // %i
		 scnt++;
		 r=sprint_int128_decimal(buf,i128);
		 fstr="%Qi";
	     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
	     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}			 
	 	}
	}	
#endif	// YA_SP_SPRINTF_Q
	printf("Now checking ya_printf():\n");
	{
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */	
	 __float128 f1;
	 int128_t i1=-1;
	 uint128_t u1=i1;
	 f1=u1;
#endif	 
	 YA_SP_SPRINTF_DECORATE(printf) ("This is produced by ya_printf!\n");
	 YA_SP_SPRINTF_DECORATE(printf)(" %s PI~=%g -1 in hex is 0x%x\n","Test string",3.1415926,-1);
#ifdef YA_SP_SPRINTF_Q /* 128 bit variables (int  & float) supported by compiler */		 
	 YA_SP_SPRINTF_DECORATE(printf)(" %I128d (=-1) %I128u(= -1 as signed128) %I128g (same as float128)\n",i1,u1,f1);
#endif	 
	 YA_SP_SPRINTF_DECORATE(printf)( "%01000d (998 zeros then 99)\n",99); // "local" buffer inside printf is only 512 chars so this checks that code works correctly if more than this output in a single call to printf()
	 YA_SP_SPRINTF_DECORATE(fprintf)( stderr,"%01000d (998 zeros then 99 to stderr)\n",99); 
	}

#if 0	 		 
	// prints to see whats actually produced..	
    scnt++;
    i=0;
    r_ya=ya_s_sprintf(buf_ya,"%'010d",i);
    printf("%%'010d(%d, 0x%x) gives <%s> length %d\n",i,i,buf_ya,r_ya);
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%'010d",i);
    printf("%%'010d(%d, 0x%x) gives <%s> length %d\n",i,i,buf_ya,r_ya);    
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'010d",i);
    printf("%%'010d(%d, 0x%x) gives <%s> length %d\n",i,i,buf_ya,r_ya);      
	i=-2536000; 
    r_ya=ya_s_sprintf(buf_ya,"%'011d",i);
    printf("%%'011d(%d, 0x%x) gives <%s> length %d\n",i,i,buf_ya,r_ya);      	
#endif    
  }
#ifdef USE_HR_TIMER  
  time_taken=read_HR_Timer();
  printf("PART2: All tests completed in %g secs\n",time_taken);  
#endif  
  if(serrs==0)
		printf("PART2: %u sprintf tests completed, no errors found\n",scnt);
  else	
		printf("PART2: %u sprintf tests completed, %u errors found (%u errors expected)\n",scnt,serrs,expected_errs);
#endif 	  	  
  return 0;
}
