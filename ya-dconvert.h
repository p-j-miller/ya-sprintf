/* support functions for double->string (used by ya_sprintf) and string->double (used by fast_strtod()).
   They offer fast "round the loop" exact conversions, and share a common "powers of 10" array of constants to minimise the "object code size".
   128 bit maths in u2_64 is leveraged (which uses native capabilities when present with a "standard C" solution as a backup).
   Written by Peter Miller 2-3-2026
   MIT license 
   
   Inspired by Grisu, Ryu , ZMIJ and rsc/fpfmt but of these only Ryu & fpfmt offer functionality that could be reasonably directly used in ya_sprintf and fast_strtod() [ but neither provides a full sprintf() or strtod() function].
   The solutions here are faster (at least using Winlibs gcc 15.2.0 with an i3-10100 using the test programs here) while providing a full sprintf() and strtod().
   
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
	 // user function that can be directly called to convert double -> string 
	 void ya_dconvert_fixed(char *dst, double f, uint32_t prec) ; /* prec is required number of digits after decimal point */

	 // 19/2/2026 Peter Miller interface to ya_sprintf()
	 int ya_d2exp_buffered_n_ya_sprintf(uint64_t ieeeMantissa,uint32_t ieeeExponent, uint32_t precision, char* buffer,int32_t *decimal_pos) ;

	 // 27/2/2026 added interface for fast_strtod()
	 double ya_conv_mant_exp_to_double(bool signedM,uint64_t m10,int32_t dec_exp); // returns m10*1o^dec_exp as a double
	 
	 // "support functions" 
	 
	/* core integer->ascii converters   */

	/* convert 2 decimal digits 00..99. */
	/* based on code already in ya_sprintf() */
	/* This is the fastest option for winlibs gcc 15.2.0 for both -m64 and -m32 */
	// only one copy of the ya_s__digitpair array is created - see https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:30,positionColumn:1,positionLineNumber:30,selectionStartColumn:1,selectionStartLineNumber:30,startColumn:1,startLineNumber:30),source:'%23include+%3Cstdint.h%3E+++//+uint64_t%0A%23include+%3Cstring.h%3E+++//+memcpy%0Aunsigned+is_big_endian_byUnion(void)%0A%7B%0A++++const+union+%7B+unsigned+u%3B+unsigned+char+c%5Bsizeof(unsigned)%5D%3B+%7D+one+%3D+%7B+1+%7D%3B+++/*+don!'t+use+static+:+performance+detrimental++*/%0A++++return+one.c%5B0%5D!!%3D1%3B%0A%7D%0A%0A+bool+is_big_endian(void)+%0A%7B%0A++int+n+%3D+1%3B%0A++return+*(char*)(%26n)+!!%3D+1%3B%0A%7D%0Avoid+inline+uitoa2(char+*out,+uint8_t+x)%0A%7B%0A+static+const+struct%0A%09%7B%0A%09+const+uint32_t+temp%3B+//+force+next+field+to+be+4-byte+aligned%0A%09+const+char+pair%5B201%5D%3B%0A%09%7D+ya_s__digitpair+%3D%0A%09%7B%0A%09++0,%0A%09+++%2200010203040506070809101112131415161718192021222324%22%0A%09+++%2225262728293031323334353637383940414243444546474849%22%0A%09+++%2250515253545556575859606162636465666768697071727374%22%0A%09+++%2275767778798081828384858687888990919293949596979899%22%0A%09%7D%3B%0A+memcpy(out,+ya_s__digitpair.pair%2B(uint8_t)(x%3C%3C1),2)%3B%0A%7D%0A+void+uint32_to_4digit_ascii(char*+dst,+uint32_t+lox)%0A%7B%0A++const+uint32_t+MASK32+%3D+((int32_t)(1)+%3C%3C+14)+-+1%3B%0A++uint32_t+lo+%3D+(lox*167772+%2B83886+)%3E%3E10%3B//+fix-point+18.14+format.+want+*2%5E14/100+%3D+163.84,+*167772/1024%3D163.83984375+,+%2B83886+gives+rounding%0A++uitoa2(dst,lo%3E%3E14)%3B%0A++lo+%3D+(lo+%26+MASK32)+*+100%3B%0A++uitoa2(dst%2B2,lo%3E%3E14)%3B%0A++//dst%5B4%5D+%3D+0%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g152,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'1',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+gcc+15.2+(Editor+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:mingw64_ucrt_gcc_1520,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'1',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:2,lang:c%2B%2B,libs:!(),options:'-O2+-m32',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+MinGW+gcc+15.2.0+(Editor+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4
	// or short link https://godbolt.org/z/5rfoxhsx1
	static void inline ya_uitoa2(char *out, uint8_t x)
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
	static inline void ya_uitoa8(char* dst, uint64_t x)
	{
	  const uint64_t POW2_57_DIV_POW100_3 = ((int64_t)(1) << 57)/100/100/100 + 1;
	  const uint64_t MASK32 = ((int64_t)(1) << 32) - 1;
	  uint64_t fp= x* POW2_57_DIV_POW100_3;// fix-point 7.57 format.
	  ya_uitoa2(dst,fp>>57);
	  // convert to format 7.32
	  // Some compilers handle it better that 7.57
	  fp = (fp >> 25)+1;
	  fp = (fp & MASK32) * 100;
	  ya_uitoa2(dst+2,fp>>32); fp = (fp & MASK32) * 100;
	  ya_uitoa2(dst+4,fp>>32); fp = (fp & MASK32) * 100;
	  ya_uitoa2(dst+6,fp>>32);  
	  //dst[8] = 0;
	}


  #if 0  /* on test program #if 1 is slower with winlibs gcc 15.2.0 for both -m32 and -m64 */
	// see https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:33,endLineNumber:39,positionColumn:31,positionLineNumber:39,selectionStartColumn:33,selectionStartLineNumber:39,startColumn:31,startLineNumber:39),source:'%23include+%3Cstdint.h%3E+++//+uint64_t%0A%23include+%3Cstring.h%3E+++//+memcpy%0Aunsigned+is_big_endian_byUnion(void)%0A%7B%0A++++const+union+%7B+unsigned+u%3B+unsigned+char+c%5Bsizeof(unsigned)%5D%3B+%7D+one+%3D+%7B+1+%7D%3B+++/*+don!'t+use+static+:+performance+detrimental++*/%0A++++return+one.c%5B0%5D!!%3D1%3B%0A%7D%0A%0A+bool+is_big_endian(void)+%0A%7B%0A++int+n+%3D+1%3B%0A++return+*(char*)(%26n)+!!%3D+1%3B%0A%7D%0Avoid+inline+uitoa2(char+*out,+uint8_t+x)%0A%7B%0A+static+const+struct%0A%09%7B%0A%09+const+uint32_t+temp%3B+//+force+next+field+to+be+4-byte+aligned%0A%09+const+char+pair%5B201%5D%3B%0A%09%7D+ya_s__digitpair+%3D%0A%09%7B%0A%09++0,%0A%09+++%2200010203040506070809101112131415161718192021222324%22%0A%09+++%2225262728293031323334353637383940414243444546474849%22%0A%09+++%2250515253545556575859606162636465666768697071727374%22%0A%09+++%2275767778798081828384858687888990919293949596979899%22%0A%09%7D%3B%0A+memcpy(out,+ya_s__digitpair.pair%2B(uint8_t)(x%3C%3C1),2)%3B%0A%7D%0A+void+uint32_to_4digit_ascii(char*+dst,+uint32_t+lox)%0A%7B%0A++const+uint32_t+MASK32+%3D+((int32_t)(1)+%3C%3C+14)+-+1%3B%0A++uint32_t+lo+%3D+(lox*167772+%2B83886+)%3E%3E10%3B//+fix-point+18.14+format.+want+*2%5E14/100+%3D+163.84,+*167772/1024%3D163.83984375+,+%2B83886+gives+rounding%0A++uitoa2(dst,lo%3E%3E14)%3B%0A++lo+%3D+(lo+%26+MASK32)+*+100%3B%0A++uitoa2(dst%2B2,lo%3E%3E14)%3B%0A++//dst%5B4%5D+%3D+0%3B%0A%7D%0Avoid+ya_uitoa4(char*+dst,+uint32_t+x)%0A%09%7B%0A%09++uint8_t+x_div_100+%3D+((uint32_t)x*0x147B+)%3E%3E19%3B//+x/100+:+9999*0x147b%3D+0x031F+F035+which+easily+fits+in+32+bits+and+obviously+the+result+(0..99)+fits+in++8+bits%0A%09++uitoa2(dst,x_div_100)%3B%0A%09++uitoa2(dst%2B2,x-(uint32_t)x_div_100*100)%3B//+x%25100%0A%09++//dst%5B4%5D+%3D+0%3B%0A%09%7D%09'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g152,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'1',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-O3',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+gcc+15.2+(Editor+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:mingw64_ucrt_gcc_1520,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'1',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:2,lang:c%2B%2B,libs:!(),options:'-O3+-m32',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+MinGW+gcc+15.2.0+(Editor+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',m:100,n:'0',o:'',t:'0')),version:4 
	// or https://godbolt.org/z/1odv9PGs6 
  	// 4 digit conversion - takes  uint16 argument, uses 1 32bit multiply and shift, one 16 bit multiply and a 16 bit subtract 
	/* constants calculated by magicgu-python3.py */
	static inline void ya_uitoa4(char* dst, uint32_t x)
	{
	  uint8_t x_div_100 = (x*0x147B )>>19;// x/100 : 9999*0x147b= 0x031F F035 which easily fits in 32 bits and obviously the result (0..99) fits in  8 bits
	  ya_uitoa2(dst,x_div_100);
	  ya_uitoa2(dst+2,x-(uint32_t)x_div_100*100);// x%100
	  //dst[4] = 0;
	}		 
  #else /* use 18.14 fixed point */
  	// 4 digit conversion - uses 18.14 format and only uint32's - uses two 32 bit multiplies, 3 32 bit shifts, a 32 bit subtract and a 32 bit and.
	static inline void ya_uitoa4(char* dst, uint32_t x)
	{
	  const uint32_t MASK14 = ((int32_t)(1) << 14) - 1;
	  uint32_t fp = (x*167772 +83886 )>>10;// fix-point 18.14 format. want *2^14/100 = 163.84, *167772/1024=163.83984375 , +83886 gives rounding
	  ya_uitoa2(dst,fp>>14);
	  fp = (fp & MASK14) * 100;
	  ya_uitoa2(dst+2,fp>>14);
	  //dst[4] = 0;
	}	  
  #endif



	#if (defined(_M_AMD64) || defined(_M_IX86)) && !defined(__BORLANDC__) // "Intel" 32 or 64 bits - note "64" or "128" functions need a 64 bit processor
	 #include <intrin.h>  // __lzcnt64/_umul128/__umulh
	#endif
 #if 1 /* 1 for normal use, 0 to test/force portable C solution */
	static inline int ya_clz(uint64_t x) 
	{ /* returns the number of leading zero's in x , x must be >0 */
	  assert(x != 0);
	#if defined(__has_builtin) && __has_builtin(__builtin_clzll) /*  is builtin for gcc and clang */
	  return __builtin_clzll(x);
	#elif defined(_M_AMD64) && defined(__AVX2__) && !defined(__BORLANDC__)
	  // Use lzcnt only on AVX2-capable CPUs that have this BMI instruction.
	  return __lzcnt64(x);
	#elif defined(_M_AMD64) || defined(_M_ARM64)
	  unsigned long idx;
	  _BitScanReverse64(&idx, x);  // Fallback to the BSR instruction.
	  return 63 - idx;
	#elif ZZ_M_IX86
	  // Fallback to the 32-bit BSR instruction. [ see comment below "ZZ" added so this is never used ]
	  unsigned long idx;
	  if (_BitScanReverse(&idx, (uint32_t)(x >> 32))) return 31 - idx;
	  _BitScanReverse(&idx, (uint32_t)(x));
	  return 63 - idx;
	#else /* portable C - both Winlibs/gcc-64 and Borland/clang have __builtin_clzll() , but this is faster than falling back to the 32-bit BSR above - so this is used for 32 bit Intel at least */
	  // based on hackers-delight nlz2 expanded to 64 bits, and optimised by PMi 
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

 #ifdef __cplusplus
    }
 #endif
#endif  // DCONVERT_H_
