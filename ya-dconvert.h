/* support functions for double->string (used by ya_sprintf) and string->double (used by fast_strtod()).
   They offer fast "round the loop" exact conversions, and share a common "powers of 10" array of constants to minimise the "object code size".
   128 bit maths in u2_64 is leveraged (which uses native capabilities when present with a "standard C" solution as a backup).
   Written by Peter Miller 2-3-2026
   MIT license 
   
   Inspired by Grisu, Ryu , ZMIJ and rsc/fpfmt but of these only Ryu & fpfmt offer functionality that could be reasonably directly used in ya_sprintf and fast_strtod() [ but neither provides a full sprintf() or strtod() function].
   The solutions here are faster (at least using Winlibs gcc 15.2.0 with an i3-10100 using the test programs here) while providing a full sprintf() and strtod().
 
   3/4/2026 - optimised ya_uitoa8() and ya_uitoa4() - now uses different implementations of ya_uitoa4() for 32 bit and 64 bit compiles. 
*/
/*----------------------------------------------------------------------------
 * MIT License:
 *
 * Copyright (c) 2026 Peter Miller
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
 
#ifndef DCONVERT_H_
 #define DCONVERT_H_
 #ifdef __cplusplus
 extern "C" {
 #endif 
	 #include <stdint.h> // uint32_t
	 #include <stdbool.h> // bool
	 #include <assert.h>
	 #include <string.h> // memcpy
	 // user functions that can be directly called to convert double -> string 
	 // all return a pointer to the trailing '0' in dst which allows another string to be easily added on, or the length calculated without needing to use strcat() or strlen()
	 // the 1st 2 emulate functionality in sprintf() - but are significantly faster as sprintf() needs to parse the format string and extract the precision
	 // the two ya_short functions provide functionality not available in sprintf(), which is especially useful when writing numerical data to a file in text format, as writing fewer characters is both faster and gives a smaller file size.
	 // ya_shortf() also works directly on floats which also gives it a significant speed advantage.
	char *ya_dconvert_efmt(char *dst, double f, uint32_t prec) ; /* prec is required number of digits after decimal point - emulates sprintf(dst,"%.*e",prec,f). dst must have space for at least 9+prec characters*/
	char *ya_dconvert_ffmt(char *dst, double f, uint32_t prec) ; /* prec is required number of digits after decimal point - emulates sprintf(dst,"%.*f",prec,f). dst must have space for at least 308+prec+3 characters if f is DBL_MAX (1.8e308) */
	char *ya_dconvert_gfmt(char *dst, double f, int32_t prec) ; /* prec is required total number of mantissa digits (prec<0 gives default = 6) - emulates sprintf(dst,"%.*g",prec,f). dst must have space for at least 8+prec characters */
	char *ya_shortd(char *dst, double f) ; /* create shortest string that accurately represents double "f" using either fixed point or exponential notation. dst must have space for at least 25 characters*/
	char *ya_shortf(char *dst, float f) ; /* create shortest string that accurately represents float "f" using either fixed point or exponential notation. dst must have space for at least 16 characters */
	 
	 // "support functions" 

	 // 19/2/2026 Peter Miller interface to ya_sprintf()
	 int ya_d2exp_buffered_n_ya_sprintf(uint64_t ieeeMantissa,uint32_t ieeeExponent, int32_t precision, char* buffer,int32_t *decimal_pos) ;

	 // 27/2/2026 added interface for fast_strtod()
	 double ya_conv_mant_exp_to_double(bool signedM,uint64_t m10,int32_t dec_exp); // returns m10*1o^dec_exp as a double	 
	 
	/* core integer->ascii converters   */

	/* convert 2 decimal digits 00..99. */
	/* based on code already in ya_sprintf() */
	/* This is the fastest option for winlibs gcc 15.2.0 for both -m64 and -m32 */
	// only one copy of the ya_s__digitpair array is created - see https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:30,positionColumn:1,positionLineNumber:30,selectionStartColumn:1,selectionStartLineNumber:30,startColumn:1,startLineNumber:30),source:'%23include+%3Cstdint.h%3E+++//+uint64_t%0A%23include+%3Cstring.h%3E+++//+memcpy%0Aunsigned+is_big_endian_byUnion(void)%0A%7B%0A++++const+union+%7B+unsigned+u%3B+unsigned+char+c%5Bsizeof(unsigned)%5D%3B+%7D+one+%3D+%7B+1+%7D%3B+++/*+don!'t+use+static+:+performance+detrimental++*/%0A++++return+one.c%5B0%5D!!%3D1%3B%0A%7D%0A%0A+bool+is_big_endian(void)+%0A%7B%0A++int+n+%3D+1%3B%0A++return+*(char*)(%26n)+!!%3D+1%3B%0A%7D%0Avoid+inline+uitoa2(char+*out,+uint8_t+x)%0A%7B%0A+static+const+struct%0A%09%7B%0A%09+const+uint32_t+temp%3B+//+force+next+field+to+be+4-byte+aligned%0A%09+const+char+pair%5B201%5D%3B%0A%09%7D+ya_s__digitpair+%3D%0A%09%7B%0A%09++0,%0A%09+++%2200010203040506070809101112131415161718192021222324%22%0A%09+++%2225262728293031323334353637383940414243444546474849%22%0A%09+++%2250515253545556575859606162636465666768697071727374%22%0A%09+++%2275767778798081828384858687888990919293949596979899%22%0A%09%7D%3B%0A+memcpy(out,+ya_s__digitpair.pair%2B(uint8_t)(x%3C%3C1),2)%3B%0A%7D%0A+void+uint32_to_4digit_ascii(char*+dst,+uint32_t+lox)%0A%7B%0A++const+uint32_t+MASK32+%3D+((int32_t)(1)+%3C%3C+14)+-+1%3B%0A++uint32_t+lo+%3D+(lox*167772+%2B83886+)%3E%3E10%3B//+fix-point+18.14+format.+want+*2%5E14/100+%3D+163.84,+*167772/1024%3D163.83984375+,+%2B83886+gives+rounding%0A++uitoa2(dst,lo%3E%3E14)%3B%0A++lo+%3D+(lo+%26+MASK32)+*+100%3B%0A++uitoa2(dst%2B2,lo%3E%3E14)%3B%0A++//dst%5B4%5D+%3D+0%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g152,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'1',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+gcc+15.2+(Editor+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:mingw64_ucrt_gcc_1520,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'1',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:2,lang:c%2B%2B,libs:!(),options:'-O2+-m32',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+MinGW+gcc+15.2.0+(Editor+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4
	// or short link https://godbolt.org/z/5rfoxhsx1
	static void inline ya_uitoa2(char *out, uint8_t x) // w32 uint8 42.7ns , int8 42.7, w64 uint8 32.0 , int8 32.0ns 
	{
	 static const struct
		{
		 const uint32_t temp; // force next field to be 4-byte aligned
		 const char pair[201];
		} ya_s__digitpair =
		{
		  0,
		   "00010203040506070809101112131415161718192021222324"
		   "25262728293031323334353637383940414243444546474849"
		   "50515253545556575859606162636465666768697071727374"
		   "75767778798081828384858687888990919293949596979899"
		};
	 memcpy(out, ya_s__digitpair.pair+(uint8_t)(x<<1),2);
	}

	// various routines to (quickly) convert unsigned integers to n digit decimal strings, all using uitoa2() above to convert 2 digit segments. 
	// Based on fixed point conversion routine in "Algorithms+Data Structures=Programs", N.Wirth, 1976, page 49 and ideas from https://groups.google.com/g/comp.arch/c/mXx7ITjLTGc
	
	// 10 digit conversion 
	static inline void ya_uitoa10(char* dst, uint64_t x)
	{
	  const uint64_t POW2_57_DIV_POW100_4 = ((int64_t)(1) << 57)/100/100/100/100 + 1;
	  const uint64_t MASK32 = ((int64_t)(1) << 32) - 1;

	  // Strip LS bit of x. It will be added back at last step.
	  // Removal of LS bit reduces the need for precision in DIV factor, which allow to calculate
	  // a sufficiently precise 7.57 representation without needing a 64x64=>128 multiplication.
	  // 7 before the decimal point allows 0-127 but we only use 0-99 here
	  int64_t lox = x & (uint64_t)(-2);
	  uint64_t fp = lox*POW2_57_DIV_POW100_4;// fix-point 7.57 format.
	  ya_uitoa2(dst,fp>>57);

	  // convert to format 7.32
	  // Some compilers handle it better that 7.57
	  fp = (fp >> 25) + 1;
	  fp = (fp & MASK32) * 100;
	  ya_uitoa2(dst+2,fp>>32); fp = (fp & MASK32) * 100;
	  ya_uitoa2(dst+4,fp>>32); fp = (fp & MASK32) * 100;
	  ya_uitoa2(dst+6,fp>>32); fp = (fp & MASK32) * 100;      
	  fp >>= 32;
	  // restore LS bit of fp
	  fp = (fp & (-2)) | (x & 1);
	  ya_uitoa2(dst+8,fp); 
	  //dst[10] = 0;  
	}

	// 8 digit conversion - basically the same as the code above, with the last step removed.
	// Useful if we generate up to 18 digits, in that case the above function does 10 digits and this does the remaining 8
	static inline void ya_uitoa8(char* dst, uint32_t x)
	{
	  const uint64_t POW2_57_DIV_POW100_3 = ((int64_t)(1) << 57)/100/100/100 + 1;
	  uint64_t fp= x* POW2_57_DIV_POW100_3;// fix-point 7.57 format.
	  ya_uitoa2(dst,fp>>57);
	  // convert to 7.25 as this fits into 32 bits, 2^25=33,554,432 so we can easily extract 6 digits this way
	  const uint32_t MASK25 = ((int32_t)(1) << 25) - 1;
	  uint32_t f32=(fp>>32)+1;
	  f32 = (f32 & MASK25) * 100;
	  ya_uitoa2(dst+2,f32>>25); f32 = (f32 & MASK25) * 100;
	  ya_uitoa2(dst+4,f32>>25); f32 = (f32 & MASK25) * 100;
	  ya_uitoa2(dst+6,f32>>25);   
	  //dst[8] = 0;
	}


  #if _M_IX86 // 32.2ns 64bit, 42.7ns w32, so use for w32
	// 4 digit conversion uses 32.32 fp and uint64's - uses two 64 bit multiplies, two 64 bit shifts (shifts by 32 are trivial in most cases)
	static inline void ya_uitoa4(char* dst, uint32_t x)
	{
	  uint64_t d64=x*(uint64_t)42949673;//  42949673 = ceil(2^32 / 10^2), allows >>32 
	  ya_uitoa2(dst,d64>>32);
	  d64=(uint32_t)d64*(uint64_t)100;// (uint32_t)d64 extracts lower 32 bits of d64
	  ya_uitoa2(dst+2,d64>>32);
	  //dst[4] = 0;
	}  
  #else// 32.0ns win64, 43.1ns on w32, so use for w64
    // 4 digit conversion - uses 7.25 format and only uint32's - uses two 32 bit multiplies, two 32 bit shifts and a 32 bit and.
	static inline void ya_uitoa4(char* dst, uint32_t x)
	{
	  uint32_t d32=x*(uint32_t)335545;//  335545 = ceil(2^25 / 10^2)
	  const uint32_t MASK25 = ((int32_t)(1) << 25) - 1;
	  ya_uitoa2(dst,d32>>25);
	  d32=(d32 & MASK25)*100;
	  ya_uitoa2(dst+2,d32>>25);
	  //dst[4] = 0;
	}  		  
  #endif
	
	// similar to ya_uitoa8() above, but gives result as  8 byte BCD with 1 byte per BCD digit - uses SWAR approach to do the calculations for the various digits in parallel
	// This still needs 6 multiplies (64bit) , whereas ya_uitoa8() only uses 1 64 bit and 3 32 bit multiplies.
	// if you just want the value written to memory then you should use ya_uitoa8(), but by returning BCD in a "register" ya_to_BCD8() allows for example simple/quick trailing zero removal using the ya_ctz64()function below.
	// bitwise or the value returned by this function with 0x0101010101010101u * '0' to get ascii, you can then use memcpy(,,8) to copy the result to memory.
	// warning - if is_big_endian() is false then you need to call bswap64() before memcpy to get the digits in the expected order (this can easily be avoided, but then the simple trailing zero removal does not work)
	// The idea of converting a uint64_t from binary to 8 decimal ascii characters with the characters "packed" within a uint64 seems to come from https://homepage.divms.uiowa.edu/~jones/bcd/decimal.html
    // & https://pvk.ca/Blog/2017/12/22/appnexus-common-framework-its-out-also-how-to-print-integers-faster/
	static inline uint64_t ya_to_BCD8(uint64_t abcdefgh) 
	{ // div and mod are evaluated simultaneously as, e.g.
	  //   (x/10000)<<32 + (x%10000)
	  //  =(x/10000)<<32 + (x-(x/10000)*10000)
	  //  =x +(x/10000)*(1<<32-10000)
	  // where the division on the RHS is implemented by the usual multiply + shift "trick" 
	  // and the fractional bits are masked away.
	 const uint64_t abcd_efgh = abcdefgh + 0xffffd8f0* ((abcdefgh * 0x68db8bbull) >> 40);// 0xffffd8f0= 1<<32 - 10000 and *0x68db8bbull >> 40 is divide by 10000 implemented by multiply & shift
	 const uint64_t ab_cd_ef_gh = abcd_efgh + 0xff9c * (((abcd_efgh * 0x147b) >> 19) & 0x7f0000007f);// 0xff9c=0x10000 - 100
	 return  ab_cd_ef_gh + 0xf6 * (((ab_cd_ef_gh * 0x67) >> 10) & 0xf000f000f000f);// 0xf6= 0x100 - 10    
	}

	#if (defined(_M_AMD64) || defined(_M_IX86)) && !defined(__BORLANDC__) // "Intel" 32 or 64 bits - note "64" or "128" functions need a 64 bit processor
	 #include <intrin.h>  // __lzcnt64/_umul128/__umulh
	#endif
	
	// is_big_endian(void) : see https://godbolt.org/z/hb8fK69Pb , this compiles to a constant, so should be inline
	static inline bool is_big_endian(void) 
	{
	 const union { unsigned u; unsigned char c[sizeof(unsigned)]; } one = { 1 };   
	 return one.c[0]!= 1;
	}
	
	#ifndef __linux /* linux already has bswap64(x) see https://www.man7.org/linux/man-pages/man3/bswap.3.html */
	 static inline uint64_t bswap64(uint64_t x) 
	 {
	 #if defined(__has_builtin) && __has_builtin(__builtin_bswap64) /*  is builtin for gcc and clang */
	  #ifndef NDEBUG
	   #pragma message( "bswap64() using __builtin_bswap64")
	  #endif
	  return __builtin_bswap64(x);
	 #elif defined(__MSVCRT__)
	  #ifndef NDEBUG
	   #pragma message( "bswap64() using _byteswap_uint64")
	  #endif
	  return _byteswap_uint64(x); // in msvcrt.dll
	 #else /* portable C solution */
	  #ifndef NDEBUG
	   #pragma message( "bswap64() using C version")
	  #endif
	  return ((x & 0xff00000000000000) >> 56) | ((x & 0x00ff000000000000) >> 40) |
			 ((x & 0x0000ff0000000000) >> 24) | ((x & 0x000000ff00000000) >> +8) |
			 ((x & 0x00000000ff000000) << +8) | ((x & 0x0000000000ff0000) << 24) |
			 ((x & 0x000000000000ff00) << 40) | ((x & 0x00000000000000ff) << 56);
	 #endif
	 }	 
	#endif // __linux

	
 #if 1 /* 1 for normal use, 0 to test/force portable C solution */
	static inline int ya_clz(uint64_t x) 
	{ /* returns the number of leading zero's in x , x must be >0 */
	  assert(x != 0);
	#if defined(__has_builtin) && __has_builtin(__builtin_clzll) /*  is builtin for gcc and clang */
	 #ifndef NDEBUG
	  #pragma message( "ya_clz() using __builtin_clzll()")
	 #endif
	  return __builtin_clzll(x);
	#elif defined(_M_AMD64) && defined(__AVX2__) && !defined(__BORLANDC__)
	  // Use lzcnt only on AVX2-capable CPUs that have this BMI instruction.
	  #ifndef NDEBUG
	   #pragma message( "ya_clz() using __lzcnt64()")
	  #endif
	  return __lzcnt64(x);
	#elif defined(_M_AMD64) || defined(_M_ARM64)
	  #ifndef NDEBUG
	   #pragma message( "ya_clz() using _BitScanReverse64()")
	  #endif
	  unsigned long idx;
	  _BitScanReverse64(&idx, x);  // Fallback to the BSR instruction.
	  return 63 - idx;
	#elif _M_IX86
	  // Fallback to the 32-bit BSR instruction. 
	  #ifndef NDEBUG
	   #pragma message( "ya_clz() using _BitScanReverse()")
	  #endif
	  unsigned long idx;
	  if (_BitScanReverse(&idx, (uint32_t)(x >> 32))) return 31 - idx;
	  _BitScanReverse(&idx, (uint32_t)(x));
	  return 63 - idx;
	#else /* portable C - both Winlibs/gcc-64 and Borland/clang have __builtin_clzll() , but this is faster than falling back to the 32-bit BSR above - so this is used for 32 bit Intel at least */
	  // based on hackers-delight nlz2 expanded to 64 bits, and optimised by PMi 
	  #ifndef NDEBUG
	   #pragma message( "ya_clz() using C code")
	  #endif
	  uint32_t y,z=x;
	  int32_t n=64;
	  y = x >>32;  if (y != 0) {n = n -32;  z = y;}
	  y = z >>16;  if (y != 0) {n = n -16;  z = y;}
	  y = z >> 8;  if (y != 0) {n = n - 8;  z = y;}
	  y = z >> 4;  if (y != 0) {n = n - 4;  z = y;}
	  y = z >> 2;  if (y != 0) {n = n - 2;  z = y;}
	  y = z >> 1;  if (y != 0) return n - 2;
	  return n - z;
	#endif
	}
 #else /* this version in portable C - based on hackers-delight nlz2 expanded to 64 bits by PMi */
	/* https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGEgKykrgAyeAyYAHI%2BAEaYxCCBAA6oCoRODB7evgGkyamOAqHhUSyx8YF2mA7pQgRMxASZPn5cFZj2BQy19QRFkTFxCbZ1DU3ZrcM9fSVlCQCUtqhexMjsHOYAzGHI3lgA1CYbbgoE%2BIIAdAiH2Hu3APR3e15hBABskgD6BCYaAIKb212mAORxOxDCwEu11ueweezYLGQiQAnj9fi89simB8dgAvCDPQTvL57VRzA5/H4AThMAHYLLCAFR7YiYAjLBgKPYEBDAhgDYh7VBUPb0JhnYB7XFxVBgDhcsKk26Mu5o6kAkVcWGPBi0XFmNEwwkEDZmEnI0i4w4AEVUhysfxhL1NJIY9rVjtuDBB1r273dnsxPqV12upvttzwIogyL2YDANr2GnJdIs3sT3oAtOGNgypcHUbm6dbDbdY4ncSDsNcuK8I3so3sY3GExtfcmDvT0229lna/XK4nC1ZaSXA%2BWe4ONtXp3sABz1xvN%2BOJjup7u%2BrPzgcF92j0tBitV6GSRfR2MrntrrvBren3O3Ke%2B4fFg8T31Tmc3A0PhvnlurimN4ZnsmZ7D%2BeZPpie5jr8MLvvmoazlwZ5NherbtuSrLssQ3pbhBB7YRyvagVKAb/BoNJmBs7RKNqvZ6ihlKUU6gi9jaOYOhRVJOtGqhVk2xrEgQcwaKoGgSZJEkAGKyXJslAWmwYaBG/GJmp05hgR%2B7MTxkZ8SCbiruJknyYpG4kdY/YPhpvoaUZRzWSOsHUrxTb2cZMnydJ5m3gcljbjZwYeUcgXOR6el/u5hmed5CmdkpIHWHs94MrZIZHIcbgpTBEVuRAIXtqoGxxT5CUWemAUQRldkxaF2kuSx%2BnRVlxm0qVvlJQFKFFjp3EwkRuHsUWumbK4UYRcWHALLQnD%2BLwfgcFopCoJwRmWMlChLCswKbDwpAEJo00LAA1iAGwbOcF3XTdt36JwkgLUdK2cLwCggBoB1HQscCwEgaAsIkdBxOQlAA0D9DxMArRmHwdAEHE70QNEz3RGE9TIpw%2B1o8wxDIgA8tE2hVId3C8ADbCCPjuqY0tvBYNEXjAG4Yi0O9ZOkFgLCGMA4h05zeCstUABumDs8tmCqFUXgI1jvAvO0z20Hg0TEBjHhYM9BDgiwcukKLxDRCkmDWpg3NGMrRjfXwBjAAoABqeCYAA7vjiSMHr/CCCIYjsFIMiCIoKjqPzuhcPoPMoNY1j6Cr72QAsqCJJ07OZvjGy8KgBvglg8cQAslTVM4ECuGMLRBAw6DTAKeh5GkAhl7XKT1ww1elIM4eF503SjJ4zR6F3NQjL0YT9O38Sd8PjeT1Mo8zB3BfbasEgzXNT386tHCknOryZu8ezAMgyB7K05xmE2uCECQ/kbFwcy8KTWhzKd52Xbd7/XfdHCPaQi3LZvb0PpfTpj9GAiAUCoEBsDMgFAIDg2gSAFgEJnbEi8MgBoHxD7IA%2BDDT6NBaAI2IEjFG/McYYz1mQvGhNiYOD1hTRgBBqa0FpstBmTMWa0DZnrLmPM%2BasMFiTPAotxa8EltLWWHMFazX5srVW6sMBrGWtrPAusOYGyNkoU25teYQmtlQW2Dsnau3dotfaXthCiHEP7cxQc1DPV0LDAwVtTAbUsLHaIedE7J3SKndOoEWCmkztnPAud4AF3aII4upc%2B7jArlXOeNdw5106NPXIzdOht1mJ3CJRcuhTxieXQeAge4j2KIkyYvcsiFOHpkheixljLzvl/eav9nqbz2AAWTCAAcQAOoHyPiffwZ9zgaAvvgIggo9r32AU/F%2BF0rof3fl/H%2Bf9M6vVsEAx%2Bx0v5mF4LrLgElWkbw2ds5%2B%2BtEbpBAJIIAA%3D   */
	/* code optimised to give good performance for both w64 and w32 - both only have 1 branch with gcc 15.2.0 ! */
	#ifndef NDEBUG
	 #pragma message( "ya_clz() using C code [forced]")
	#endif
	static inline int ya_clz(uint64_t x) 
	{ /* returns the number of leading zero's in x  */
	  // based on hackers-delight nlz2 expanded to 64 bits by PMi 
	  uint32_t y,z=x;
	  int32_t n=64;
	  y = x >>32;  if (y != 0) {n = n -32;  z = y;}
	  y = z >>16;  if (y != 0) {n = n -16;  z = y;}
	  y = z >> 8;  if (y != 0) {n = n - 8;  z = y;}
	  y = z >> 4;  if (y != 0) {n = n - 4;  z = y;}
	  y = z >> 2;  if (y != 0) {n = n - 2;  z = y;}
	  y = z >> 1;  if (y != 0) return n - 2;
	  return n - z;
	}
 #endif	 

#if 1 /* 1 for normal use, 0 to test/force portable C solution */
	static inline int ya_clz32(uint32_t x) 
	{ /* returns the number of leading zero's in x , x must be >0 */
	  assert(x != 0);
	#if defined(__has_builtin) && __has_builtin(__builtin_clz) /*  is builtin for gcc and clang */
	  #ifndef NDEBUG
	   #pragma message( "ya_clz32() using __builtin_clz(x)")
	  #endif
	  return __builtin_clz(x);
	#elif defined(_M_AMD64) || defined(_M_ARM64) || defined(_M_IX86) 
	  #ifndef NDEBUG
	   #pragma message( "ya_clz32() using _BitScanReverse(x)")
	  #endif
	  unsigned long idx;
	  _BitScanReverse(&idx, x);  // Fallback to the BSR instruction.
	  return 31 - idx;
	#else /* portable C - both Winlibs/gcc-64 and Borland/clang have __builtin_clz() */
	  // based on hackers-delight nlz2 
	  #ifndef NDEBUG
	   #pragma message( "ya_clz32() using C code")
	  #endif
     uint32_t y;
     int32_t n=32;
     y = x >>16;  if (y != 0) {n = n -16;  x = y;}
     y = x >> 8;  if (y != 0) {n = n - 8;  x = y;}
     y = x >> 4;  if (y != 0) {n = n - 4;  x = y;}
     y = x >> 2;  if (y != 0) {n = n - 2;  x = y;}
     y = x >> 1;  if (y != 0) return n - 2;
     return n - x;
	#endif
	}
 #else /* this version in portable C - based on hackers-delight nlz2  - only slightly slower than __builtin_clz() ! */
	#ifndef NDEBUG
	 #pragma message( "ya_clz32() using C code [forced]")
	#endif
	static inline int ya_clz32(uint32_t x) 
	{ /* returns the number of leading zero's in x  */
	  // based on hackers-delight nlz2 
     uint32_t y;
     int32_t n=32;
     y = x >>16;  if (y != 0) {n = n -16;  x = y;}
     y = x >> 8;  if (y != 0) {n = n - 8;  x = y;}
     y = x >> 4;  if (y != 0) {n = n - 4;  x = y;}
     y = x >> 2;  if (y != 0) {n = n - 2;  x = y;}
     y = x >> 1;  if (y != 0) return n - 2;
     return n - x;
	}
 #endif	  

 #if 1 /* if 0 force C version */
	static inline int ya_ctz64(uint64_t x) 
	{ /* returns the number of trailing zero's in x , x must be >0 */
	  assert(x != 0);
	#if defined(__has_builtin) && __has_builtin(__builtin_ctzll) /*  is builtin for gcc and clang */
	  #ifndef NDEBUG
	   #pragma message( "ya_ctz64() using __builtin_ctzll()")
	  #endif
	  return __builtin_ctzll(x);
	#elif defined(_M_ARM) || defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC)
	 #ifndef NDEBUG
	  #pragma message( "ya_ctz64() using _CountTrailingZeros64()")
	 #endif
     return (int)_CountTrailingZeros64(x);
	#elif defined(_WIN64)
	 #if defined(__AVX2__) || defined(__BMI__)
	  #ifndef NDEBUG
	   #pragma message( "ya_ctz64() using _tzcnt_u64()")
	  #endif
      return (int)_tzcnt_u64(x);
	 #else
	  #ifndef NDEBUG
	   #pragma message( "ya_ctz64() using _BitScanForward64()")
	  #endif
      unsigned long r;
      _BitScanForward64(&r, x);
      return (int)r;
	 #endif	  
	#else /* portable C solution - based on hackers delight ntz7 expanded to 64 bits by Peter Miller, this version gives better 32 bit code */
	/* both versions don't check for x==0 as that's not necessary here */
	 #ifndef NDEBUG
	  #pragma message( "ya_ctz64() using C code")
	 #endif
	 uint_fast8_t b5, b4, b3, b2, b1, b0;
   #if 1 /* fastest for both w64 and w32 */
     uint32_t y;
     b5 = (x & 0x00000000FFFFFFFF) ? 0 : 32;             
     y = b5?x>>32:(uint32_t)x;
     y= y & -y;               // Isolate rightmost 1-bit.
     b4 = (y & 0x0000FFFF) ? 0 : 16;
     b3 = (y & 0x00FF00FF) ? 0 : 8;
     b2 = (y & 0x0F0F0F0F) ? 0 : 4;
     b1 = (y & 0x33333333) ? 0 : 2;
     b0 = (y & 0x55555555) ? 0 : 1;
     return b5 + b4 + b3 + b2 + b1 + b0;
   #else     
	 x&= -x;// Isolate rightmost 1-bit.             
	 b5 = (x & 0x00000000FFFFFFFF) ? 0 : 32;
     b4 = (x & 0x0000FFFF0000FFFF) ? 0 : 16;
     b3 = (x & 0x00FF00FF00FF00FF) ? 0 : 8;
     b2 = (x & 0x0F0F0F0F0F0F0F0F) ? 0 : 4;
     b1 = (x & 0x3333333333333333) ? 0 : 2;
     b0 = (x & 0x5555555555555555) ? 0 : 1;
     return b5 + b4 + b3 + b2 + b1 + b0;
   #endif     
	#endif
 #else
	  /* forced C version */
	// see https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIAMykrgAyeAyYAHI%2BAEaYxCBmABykAA6oCoRODB7evgGp6ZkCoeFRLLHxSbaY9o4CQgRMxAQ5Pn5c1bVZDU0EJZExcSDSCo3NrXkdo739ZRXDAJS2qF7EyOwc5v5hyN5YANQm/m6j%2BIIAdAhH2Pu3APR3%2B15hBABskgD6BCYaAIJbOz2mEOx1GxDCwEu11u%2Bwe%2BzYLGQKQAnj9/mZtgxdl4DkcTgR8Kgof5sGiXvtkUwPrsAF4QZ6Cd5ffaqBaHP4/ACcJgA7BZYQAqfbETAEVYMBT7AgIYEMQbEfaoKhS4hMOgQ/Y0uKoMAcSVhFm3AV3NFcp4vD5UJijRLM/bRACspHt0ntgXtZmd0Q69o0RysHIxeGVXDRMNuDII/jMzNR/gDv3D9odIIAIvsIKpDmZXvsNKoNIWi4WAGJl8tltlHEt5/YgfbR/1J5u3MPh5Fp5PV1TXa7RyiR6NfBY9%2BNtmFx9Md8y5gC0cf5LZbcIAkgpPEwCMDwcAEAQWOkCPsuLPooRzuPbtFJJ2INOc3mC0WK1X/DWNHXj69/Ze3bf77m%2BaliWwGvu%2Bn6JD%2BfxJtEZj/tmgEFiByEaCWYG1vWkhQYm4bevBM6Pv4RHEUR6EfvWZjYTBH5HOmd4IY%2BDpMcxTFkZ%2BoZjtB4YimKxAMMm2b8tegl/tYHoiXhYnRH6nHov4NRKEmpoaJyLIzrR%2ByzqOFiruuBhbsKeC7vuh7Hqe55LkpHIqQJGmZgxQHFs%2BFblmx9aNrJME3nZWYEY5wFlsWL4guB9ZcN%2Bnm4f4t6%2BQ%2BjmBahIGJaBIUYfskGRTCsExQ5SGoflKGFW5%2BxYZlV5cDlfmqCRNW1f4xWUWVvqVXFqgse1HUOsVHEJkmPHirZlguhJ0VSXBUkVVJMkJlsrjBlZ/wqbyqZoiwaoMBACymnykZMkeo6pj1Yb8MQma0T12mjm4eLnVtXEpOCghUBA5iUWYDrBOo2YOugJgOm4DCvaQqikJS1K0HSrJbbJ22phwSy0JwDq8H4HBaKQqCcNdlhieuqzrNm/g8KQBCaPDSwANYJE6iMcJIKNkxjnC8AoIAaCTZNLHAsBIGgLApHQcTkJQfMC/Q8TAFwDqejQtBbsQrMQNEjNnswxDIpwxOq00yIAPLRNomAOJrvB82wgi6wwtAa2jvBYNEXjAG4Yi0Kz3B25ga1GOItukPgIoOHgABumBu%2BjmCqEbXhbib5CCDUjO0Hg0SqurHhYIzBDgiwsch8Q0TpJgqae4YwBJ0YnN8AYwAKAAangmAAO66ykjCx/wggiGI7BSDIgiKCo6i%2B7oHQGBXpg45Y%2BjJ6zkBLKgKR1BKnCzrr/i8Kgefglgs%2BbZ0RtLy4DDuJ4bR6CEYQDOUQwdGkGRLxM7QFPfWSzPKeh2Af3RjC0p%2BTPvgd6g/zftfeIUwf6Pw/sAy%2Bcwb5LDxmsHuCMkYM19pjDgLJEivFnO8fYwBkDIGPA6c4cEIC4EICQQmXAFi8FJrbBYlNqb6E4PTUgqN0boJZmzDm9DSDc0QCgVA/NBZkAoBAUWIiQAsAhI3JkXhkDNA%2BPg5AHwpZmHZrLeWitla%2B21urWOei9YGwPrHM2jACCW2toze2jtna0FdrHLAXsy4bHRv7L%2BIcw68AjlHGO7s45blpujJOKcdbp1cbQ7Ouc4gFyUMXZx5dQC8KoNXOuDdm6t1RsTDuwhRDiF7jkgeahGa6E9GPYw1hrDT2iLveei8shu1XtFWcLBowby3ngHe8B4E1C/s4CArhIEdAvqUd%2Bt9CgPz/k/O%2BRQGAgPmFMXpgCGA9HGFMj%2BSyl6rL6DAsZtgIHrPATMXZoCJDwJWIgs5zCODIzYYzdB%2BwACyYQADiAB1PBBCiEkPOB%2BMh%2BAiAKi2NQ2hnNGHvWuaw9hG9ma2G4XQrQXMYACJ8cgaOJBhYQCaDXZQhgahCAQKgRuWTTZCLFpuLIuLwi0AJUS6FpAJHixAJLaWDKyUiIiKwDYpLhHi11tHWlxLGaot%2BMQGusLUUNDOLCnJXd8nSEKUoYpw89DlJQJUqeITakY3qQIRpa92lxG3qHbppBiBeEEHgNgAAVVAnhtUIPWFAs4VL8WEqFf4rOmB2Ds0bqqFIJtkE3NQRwzg2BI5osBZg7BuDlHfNIdjKwU99jkKjcCmhPDEXgppiwkNMKOBcPZgi8m1yzC8BzlwQsdy0GwuLQw0gecMjOEkEAA%3D%3D%3D
	#ifndef NDEBUG
	 #pragma message( "ya_ctz64() using C code [forced]")
	#endif
	static inline int ya_ctz64(uint64_t x) 
		{ /* returns the number of trailing zero's in x  */
		 uint_fast8_t  b5, b4, b3, b2, b1, b0;
	#if 1 // w64 1=>32.0ns, 0=>32.5 ; w32 1=>48.1, 0=>48.7
		 uint32_t y;
		 b5 = (x & 0x00000000FFFFFFFF) ? 0 : 32;             
		 y = b5?x>>32:(uint32_t)x;
		 y= y & -y;               // Isolate rightmost 1-bit.
		 b4 = (y & 0x0000FFFF) ? 0 : 16;
		 b3 = (y & 0x00FF00FF) ? 0 : 8;
		 b2 = (y & 0x0F0F0F0F) ? 0 : 4;
		 b1 = (y & 0x33333333) ? 0 : 2;
		 b0 = (y & 0x55555555) ? 0 : 1;
		 return b5 + b4 + b3 + b2 + b1 + b0;
	#else     
		 x&= -x;// Isolate rightmost 1-bit.             
		 b5 = (x & 0x00000000FFFFFFFF) ? 0 : 32;
		 b4 = (x & 0x0000FFFF0000FFFF) ? 0 : 16;
		 b3 = (x & 0x00FF00FF00FF00FF) ? 0 : 8;
		 b2 = (x & 0x0F0F0F0F0F0F0F0F) ? 0 : 4;
		 b1 = (x & 0x3333333333333333) ? 0 : 2;
		 b0 = (x & 0x5555555555555555) ? 0 : 1;
		 return b5 + b4 + b3 + b2 + b1 + b0;
	#endif     

 #endif
	}
 #ifdef __cplusplus
    }
 #endif
#endif  // DCONVERT_H_
