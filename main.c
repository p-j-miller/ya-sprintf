/* test code for ya_sprintf.c

   Written by Peter Miller 2/2020
   This version 2/26 - latest changes were to support dconvert derived double conversion in ya_sprintf.
   					 - this still generates correct numbers, but especially for denormals generates different results to the other double converters which required some changes to this test suite to avoid "false failures".
 
 This version adds tests for %w32/64/128 - which is a C23 addition.
 %b/%B another C23 addition was already covered by these tests.  
 
		WARNING - long doubles on WSL 1 are broken - see https://github.com/microsoft/WSL/issues/830 this is claimed to be fixed on WSL 2 (WSL 2 under Windows 11 is now used for testing, but the WSL 1 "fix" is still present in the code).
The fix for WSL 1 (which is implemented here) is:
 #include <fpu_control.h>
...
   unsigned short Cw = 0x37f;
  _FPU_SETCW(Cw);					
					
	 
Under Ubuntu Linux ( Ubuntu 24.04.3 LTS (GNU/Linux 6.6.87.2-microsoft-standard-WSL2 x86_64)) the -fsanatize works as expected (but also needs -g to add debugging info to executable).	
	With recommended directory layout this gives (note fmaq.c is optional, but gives a significant speed up)
	gcc -m64 -Wall -Ofast  -I. -D_FORTIFY_SOURCE=1 main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ryu/s2d_fast_atof.c ya-dconvert.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lquadmath -lm -o test
	
 or using the sanitizer capabilities (which results in a significantly slower runtime, but provides many additional checks for errors when used with the test program):
 	gcc -m64 -Wall -Ofast -fsanitize=address -fsanitize=undefined -fstack-protector-all -g3 -fexcess-precision=standard -I. -D_FORTIFY_SOURCE=1 main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ryu/s2d_fast_atof.c ya-dconvert.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lasan -lquadmath -lm -o test


 The -D_FORTIFY_SOURCE=1 is described at https://gcc.gnu.org/legacy-ml/gcc-patches/2004-09/msg02055.html and https://wiki.ubuntu.com/ToolChain/CompilerFlags . 
 The 1st link says that when -D_FORTIFY_SOURCE=1 is used at gcc optimization level 1 and above, security measures that shouldn't change behaviour of conforming programs are taken,
 but at level 2 (and above) some conforming programs might fail. The 2nd link says that -D_FORTIFY_SOURCE=2 is the default in Ubuntu 8.10 and this was updated to -D_FORITFY_SOURCE=3 in Ubuntu 24.04.
 The test program fails "*** %n in writable segment detected ***" followed by "Aborted (core dumped)" with the default settings (as the test program checks %n functionality). ya_sprintf itself can be compiled and run with the default settings. 
 
 To get 32 capabilities on Ubuntu try: sudo apt install gcc-multilib g++-multilib 
 then -m32 -msse2 (instead of -m64) will work with the command lines above (see e.g. https://www.geeksforgeeks.org/cpp/compile-32-bit-program-64-bit-gcc-c-c/ ), e.g.
	gcc -m32  -Wall -Ofast  -I. -D_FORTIFY_SOURCE=1 main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ryu/s2d_fast_atof.c ya-dconvert.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lquadmath -lm -o test
 
 
 Under Windows using winlibs gcc 15.2.0 the command line becomes (note omitting fmaq significantly increases the runtime ) - please check the paths to the compiler and the other files are correct for your setup/directory structure:
 
 	C:\winlibs\winlibs-x86_64-posix-seh-gcc-15.2.0-mingw-w64ucrt-13.0.0-r2\mingw64\bin\gcc -Wall -m64  -Ofast  -std=gnu99 -I. main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ryu/s2d_fast_atof.c ya-dconvert.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lquadmath -static -o test.exe
 	
 	if you wish to use the sanitizer then try:
	
	C:\winlibs\winlibs-x86_64-posix-seh-gcc-15.2.0-mingw-w64ucrt-13.0.0-r2\mingw64\bin\gcc -fsanitize=undefined -fsanitize-trap=all -fsanitize=bounds-strict -fsanitize-address-use-after-scope -Wall -m64 -fexcess-precision=standard -O3  -std=gnu99 -I. main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ryu/s2d_fast_atof.c ya-dconvert.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lquadmath -static -o test.exe
 	
	Note -fsanitize-trap=all is required (otherwise the linker will complain about missing library asan which is not present with Winlibs), the other -fsanitize options are optional.
 	
 Using the static program analyser (-fanalyzer) dramatically slows the compilation, and highlights some (very long) paths to potential errors (see https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Static-Analyzer-Options.html )
 These seem to be due to potential issues if the format string does not match the arguments which is inherent to a function like sprintf().
 Using the YA_SP_SPRINTF_CHECK_FMT #define will cause gcc to check format string against the arguments, so resolving these issues (but at a loss to functionality as it does not support all sprintf's extensions).
 
 For windows 32 bit using winlibs gcc 15.2.0 the command line becomes (omitting fmaq significantly increases the runtime)
 
 	C:\winlibs\winlibs-i686-posix-dwarf-gcc-15.2.0-mingw-w64ucrt-13.0.0-r2\mingw32\bin\gcc -m32 -Wall -Ofast  -std=gnu99 -I. main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ryu/s2d_fast_atof.c ya-dconvert.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lquadmath -static -o test.exe
 	
Warning - there are a number of assert() calls in this code - these should never fail but can be removed by defining NDEBUG (e.g. adding the compiler option -DNDEBUG), note this made no measurable difference to the execution time of the test program. 

gcc -static seems be be necessary to create a standalone executable 

This test program now has a work around that allows compiling with -Ofast (this works for Windows and Linux, both 32 and 64 bit as long as -msse2 is passed to gcc for 32 bit builds (its the default for 64 bit builds) 

  // enables denormalised floating point numbers in case compiler has turned them off (which -Ofast may do with gcc >= 13) Note it does need gcc -msse 
  #include <xmmintrin.h> // needed for _mm_getcsr() & _mm_setcsr() which are present in both Windows & Linux 
  _mm_setcsr(_mm_getcsr() & ~0x8040U); // clear FTZ & DAZ bits in MXCSR see https://stackoverflow.com/questions/11671430/flushing-denormalised-numbers-to-zero

Without this "fix" using -Ofast causes denormals to be flushed to zero which causes the test program to fail.
Note the "fix" has only been tested with Intel processors, a fix that should work on all WIndows versions (including ARM) is given below - this does work on windows 32/64:
	// allow denorms for Windows (only) - in theory this is portable to Windows/ARM but this is untested ! 
  _control87(_DN_SAVE, _MCW_DN);     // set FPU control word to allow denorms - see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/control87-controlfp-control87-2?view=msvc-170&redirectedfrom=MSDN 
Much more information on -Ofast is provided at https://simonbyrne.github.io/notes/fastmath/ - it states that either -Ofast or -ffast-math must be present as an argument to the linking stage of the compilation to cause denormals to be flushed to zero..
This explains why the gcc command lines above show this issue, but a makefile with it just present on the C compilation steps, and not on the linking step do not show it.
The overhead of using denormalised numbers appears to be effectively zero with AMD and ARM processors, but potentially significant with INTEL processors - see e.g https://en.wikipedia.org/wiki/Subnormal_number
(the latest data for Intel processors is at https://www.agner.org/optimize/instruction_tables.pdf , but this just says "subnormal may increase the latency" for recent Intel processors ,
 https://www.intel.com/content/www/us/en/docs/onemkl/developer-guide-windows/2025-2/operating-on-denormals.html says "Floating-point operations on denormals are slower than on normalized operands" again without saying how much slower).
 https://soundquality.org/2025/09/fading-audio-is-rough-on-cpus/ says "older processors like Intel Pentium 4 struggled significantly when processing these subnormal values", 
 "It seems that for Intel CPUs it stopped being an issue with Sandybridge. I have a 2014 laptop with no subnormal penalty at all", 
 "AMD Ryzen seems to only take a handful of extra clock cycles". The Intel Sandybridge was introduced in 2011 and discontinued in 2013, suggesting that in 2026 this is not an issue.
  

Note that for recent gcc releases to compile correctly (passing this test program) for windows 32 then gcc needs the -fexcess-precision=standard option (this is present in the example command lines above).
This is needed for gcc 15.2.0 (for both MSVCRT and UCRT), and is not required for gcc 9.2.0 ( https://github.com/google/highway/issues/1708 suggests its required for gcc 13 and above).
Compiling for windows 64 bit works without a -fexcess-precision=standard argument to gcc (but it's recommended to add it in case things change with a future gcc release ) - adding it made no changes to the run-time of the test program.

All gcc (15.2.0) arguments are listed at https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Invoking-GCC.html , with x86 specific ones at https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/x86-Options.html


The Pentium4 was the 1st Intel processor supporting sse2 and was launched in November 2000 initially as a 32 bit only single core processor, SSE2 was also the 1st floating point unit to fully comply with the IEEE 754 standard.
It is possible to put this within the source files as #pragma GCC target("sse2") , which HAS been done here - but note it had to be added to this file as well which may be easy to forget.
gcc 64 bit compiles have -msse2 set by default.
Because of these pragma's the current code would probably not work on processors not supporting sse2 - the previous release of ya_sprintf could be used if support for such old processors was required. 
Any processor that will run Windows 10 or 11 should work with the latest version of ya_sprintf as these operating systems require a processor that has sse2.

Note that -std=gnu99 is required to compile fmaq.c (everything else will compile with C99). Using the supplied fmaq.c is optional with recent gcc's ,
 but its approx twice the speed the the version supplied in the winlibs version of gcc 15.2.0, so its use is still recommended.

Timing (all on the same PC with gcc 15.2.0 and Windows 11 25H2 with Intel i3-10100 CPU @ 3.6GHz ):
for doubles via snprintf("%.*e") using part 1 of this test program:
Timing W64:	All under Windows 11 using Intel i3-10100					23/02/2026
algorithm				part1 (secs)	part2 (secs)	part 1 delta (secs)	* speedup ref ucrt	* speedup ref base ya_sprintf	* speedup ref Ryu
ya-dconvert				32.3486			108.466			6.0836				3.9					2.1								1.3
Ryu						34.3185			109.934			8.0535				2.9					1.6								1.0
standard ya_sprintf		38.8029			114.35			12.5379				1.9					1.0	
standard sprintf(ucrt)	49.8741			125.268			23.6091				1.0		
Null (timing baseline)	26.265			102.7			0			
						
						
Timing W32:	All under Windows 11 using Intel i3-10100					
algorithm				part1 (secs)	part2 (secs)	part 1 delta (secs)	* speedup ref ucrt	* speedup ref base ya_sprintf	* speedup ref Ryu
ya-dconvert				55.4514			187.632			9.0108				5.1					3.2								1.5
Ryu						59.8388			194.182			13.3982				3.5					2.2								1.0
standard ya_sprintf		75.3432			208.611			28.9026				1.6					1.0	
standard sprintf(ucrt)	92.6915			227.139			46.2509				1.0		
Null (timing baseline)	46.4406			180.281			0			

Timing m64:	All under Linux using Intel i3-10100					23/02/2026
algorithm				part1 (secs)	part2 (secs)	part 1 delta (secs)	* speedup ref sprintf	* speedup ref base ya_sprintf	* speedup ref Ryu
ya-dconvert				37.9381			85.9848			7.046				4.4						1.5								1.3
Ryu						40.3468			90.3686			9.4547				3.3						1.1								1.0
standard ya_sprintf		41.4482			89.7552			10.5561				2.9						1.0	
standard sprintf(ucrt)	61.9352			110.294			31.0431				1.0		
Null (timing baseline)	30.8921			79.6092			0			
						
						
Timing m32:	All under Linux using Intel i3-10100					
algorithm				part1 (secs)	part2 (secs)	part 1 delta (secs)	* speedup ref sprintf	* speedup ref base ya_sprintf	* speedup ref Ryu
ya-dconvert				53.3466			172.517			8.4241				4.6						1.8								1.4
Ryu						57.0975			181.741			12.175				3.2						1.2								1.0
standard ya_sprintf		59.9574			180.099			15.0349				2.6						1.0	
standard sprintf(ucrt)	83.3688			203.831			38.4463				1.0		
Null (timing baseline)	44.9225			164.968			0			
	
*/
#define PART1_SPRINTF_TESTS /* if defined do detailed testing of double conversions (including "round loop" (converting double->string->double)  - these tests take ~ 2 minutes */


#define PART2_SPRINTF_TESTS /* if defined do testing of all formats and conversion types - these tests take ~ 2 seconds */
	/* define both PART1 and PART2 for full tests, just PART2 for a quick set of tests with reasonable coverage */

//#define YA_SP_RYU // if defined use RYU algorithm for doubles which is reasonably fast and accurate for IEEE format doubles - otherwise use a faster algorithm [from ya_dconvert.c]

//#define DO_NOT_STRCMP_SUBNORMALS // avoid string compare tests on sub-normals or other tests that might fail even though the result is actually correct, is not currently required for any of the double->string converters
//#define TEST_NULL /* if defined gives timing for the test harness only */
//#define MINGW_SPRINTF /* use standard library sprintf for timing comparison */

//#define IGNORE_LD_SIGNED_NANS /* if defined don't test -NAN for LD - this option is no longer required as ya_sprintf now allows NAN's to be treated differently for LD's compared to doubles  */
	
/* 

Expected output with both PART1 and PART2 defined on a 64 bit WinLibs gcc compiler showing clean compile & execution (using ryu for double->string conversions):

D:\>cd D:\dev-cpp-files\ya-sprintf

D:\dev-cpp-files\ya-sprintf>C:\winlibs\winlibs-x86_64-posix-seh-gcc-15.2.0-mingw-w64ucrt-13.0.0-r2\mingw64\bin\gcc -Wall -m64 -fexcess-precision=standard -Ofast  -std=gnu99 -I. main.c ../atof-and-ftoa/atof.c ../double-double/double-double.c ../u2_64-128bits-with-two-u64/u2_64.c ../my_printf/my_printf.c ../nan_type/nan_type.c ryu/d2fixed_ya_sprintf.c ryu/s2d_fast_atof.c ../hr_timer/hr_timer.c ../fma/fmaq.c -lquadmath -static -o test.exe

D:\dev-cpp-files\ya-sprintf>test
__GNUC__ defined, __GNUC__=15 __GNUC_MINOR__=2
_M_X64 is defined (64 bit)
__SSE2__ is defined
__MINGW32__ is defined
__USE_MINGW_ANSI_STDIO is defined as 0
__MINGW64__ is defined
_UCRT is defined
__MSVCRT__ is defined
_WIN32 is defined
_WIN64 is defined
__builtin_bswap64() is available
__builtin_clzll is available
YA_SP_NO_NEG_LEADINGPLUS is defined
YA_SP_NO_NEG_LEADINGSPACE is defined
YA_SP_A_FMT_ALT4 is defined
YA_SP_PTR_CAPS is defined
YA_SP_PTR_LEADINGZEROS is defined
YA_SP_SIGNED_NANS is defined
YA_SP_NAN_IND  is defined
YA_SP_WCHAR_PR_CHARS is defined
GNU C Library not found
YA_SP_SPRINTF_QI defined (128bit integers)
YA_SP_SPRINTF_QF defined (128 bit floating point)
sizeof(long double)=16 sizeof(double)=8 sizeof(float)=4 sizeof(int)=4 sizeof(char*)=8 sizeof(int *)=8 sizeof(size_t)=8
LDBL_MAX_10_EXP defined and equal to 4932 meaning we have "true" long doubles
__SIZEOF_FLOAT128__ is defined as 16
__SIZEOF_INT128__ is defined as 16
Byte order is LITTLE ENDIAN (1234)
Microsoft runtime: %n support is enabled

Using my_sprintf() - which works around limitations of native sprintf()

Starting PART1 sprintf tests:
 Testing doubles:
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
 All double round loop tests completed in 31.7012 secs
 Average time per test was 587.3 ns
 53979237 tests 2695716 differences
 Tested ya_sprintf() double-double round loop:
 0 errors when 21 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 20 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 19 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 18 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 0 errors when 17 sf string converted back to a double (0 are 1 bit) (sprintf gives 0 differences)
 907369 errors when 16 sf string converted back to a double (686113 are 1 bit) (sprintf gives 907369 differences)
 Differences in string compares between built in "libc" sprintf() and tested sprintf() are:
  1 significant figures found 0 differences
  2 significant figures found 0 differences
  3 significant figures found 0 differences
  4 significant figures found 0 differences
  5 significant figures found 0 differences
  6 significant figures found 0 differences
  7 significant figures found 0 differences
  8 significant figures found 0 differences
  9 significant figures found 0 differences
 10 significant figures found 0 differences
 11 significant figures found 0 differences
 12 significant figures found 0 differences
 13 significant figures found 0 differences
 14 significant figures found 0 differences
 15 significant figures found 0 differences
 16 significant figures found 0 differences
 17 significant figures found 0 differences
 18 significant figures found 0 differences
 19 significant figures found 1788347 differences
 Should get 0 round the loop errors for >=17 sig fig, and 0 differences on string compares at <= 15 sig figs :0 errors found

 Now checking fast_strtof128():
 Results for fast_strtof128() tests:
  No 1 bit errors found
  No multiple bit errors found


Now checking f128's in detail: ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x)):
 1 sig fig 30171 tests: 0 differences on string compares, 20158 round the loop errors with quadmath_snprintf() and 20158 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
 2 sig fig 30171 tests: 0 differences on string compares, 20143 round the loop errors with quadmath_snprintf() and 20143 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
 3 sig fig 30171 tests: 0 differences on string compares, 20133 round the loop errors with quadmath_snprintf() and 20133 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
 4 sig fig 30171 tests: 0 differences on string compares, 20124 round the loop errors with quadmath_snprintf() and 20124 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
 5 sig fig 30171 tests: 0 differences on string compares, 20124 round the loop errors with quadmath_snprintf() and 20124 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
 6 sig fig 30171 tests: 0 differences on string compares, 20118 round the loop errors with quadmath_snprintf() and 20118 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
 7 sig fig 30171 tests: 0 differences on string compares, 20113 round the loop errors with quadmath_snprintf() and 20113 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
 8 sig fig 30171 tests: 0 differences on string compares, 20097 round the loop errors with quadmath_snprintf() and 20097 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
 9 sig fig 30171 tests: 0 differences on string compares, 20092 round the loop errors with quadmath_snprintf() and 20092 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
10 sig fig 30171 tests: 0 differences on string compares, 20096 round the loop errors with quadmath_snprintf() and 20096 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
11 sig fig 30171 tests: 0 differences on string compares, 20080 round the loop errors with quadmath_snprintf() and 20080 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
12 sig fig 30171 tests: 0 differences on string compares, 20080 round the loop errors with quadmath_snprintf() and 20080 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
13 sig fig 30171 tests: 0 differences on string compares, 20086 round the loop errors with quadmath_snprintf() and 20086 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
14 sig fig 30171 tests: 0 differences on string compares, 20085 round the loop errors with quadmath_snprintf() and 20085 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
15 sig fig 30171 tests: 0 differences on string compares, 20083 round the loop errors with quadmath_snprintf() and 20083 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
16 sig fig 30171 tests: 0 differences on string compares, 20071 round the loop errors with quadmath_snprintf() and 20071 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
17 sig fig 30171 tests: 0 differences on string compares, 20065 round the loop errors with quadmath_snprintf() and 20065 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
18 sig fig 30171 tests: 0 differences on string compares, 20062 round the loop errors with quadmath_snprintf() and 20062 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
19 sig fig 30171 tests: 0 differences on string compares, 20066 round the loop errors with quadmath_snprintf() and 20066 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
20 sig fig 30171 tests: 0 differences on string compares, 20054 round the loop errors with quadmath_snprintf() and 20054 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
21 sig fig 30171 tests: 0 differences on string compares, 20052 round the loop errors with quadmath_snprintf() and 20052 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
22 sig fig 30171 tests: 0 differences on string compares, 20048 round the loop errors with quadmath_snprintf() and 20048 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
23 sig fig 30171 tests: 0 differences on string compares, 20047 round the loop errors with quadmath_snprintf() and 20047 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
24 sig fig 30171 tests: 0 differences on string compares, 20046 round the loop errors with quadmath_snprintf() and 20046 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
25 sig fig 30171 tests: 0 differences on string compares, 20052 round the loop errors with quadmath_snprintf() and 20052 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
26 sig fig 30171 tests: 0 differences on string compares, 20052 round the loop errors with quadmath_snprintf() and 20052 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
27 sig fig 30171 tests: 0 differences on string compares, 20039 round the loop errors with quadmath_snprintf() and 20039 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
28 sig fig 30171 tests: 0 differences on string compares, 20038 round the loop errors with quadmath_snprintf() and 20038 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
29 sig fig 30171 tests: 0 differences on string compares, 20045 round the loop errors with quadmath_snprintf() and 20045 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
30 sig fig 30171 tests: 0 differences on string compares, 20032 round the loop errors with quadmath_snprintf() and 20032 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
31 sig fig 30171 tests: 0 differences on string compares, 20038 round the loop errors with quadmath_snprintf() and 20038 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
32 sig fig 30171 tests: 0 differences on string compares, 20036 round the loop errors with quadmath_snprintf() and 20036 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
33 sig fig 30171 tests: 0 differences on string compares, 20022 round the loop errors with quadmath_snprintf() and 20022 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
34 sig fig 30171 tests: 0 differences on string compares, 10046 round the loop errors with quadmath_snprintf() and 10046 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
35 sig fig 30171 tests: 1 differences on string compares, 13 round the loop errors with quadmath_snprintf() and 13 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
36 sig fig 30171 tests: 49 differences on string compares, 0 round the loop errors with quadmath_snprintf() and 0 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
37 sig fig 30171 tests: 498 differences on string compares, 0 round the loop errors with quadmath_snprintf() and 0 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
38 sig fig 30171 tests: 5273 differences on string compares, 0 round the loop errors with quadmath_snprintf() and 0 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
39 sig fig 30171 tests: 24741 differences on string compares, 0 round the loop errors with quadmath_snprintf() and 0 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
40 sig fig 30171 tests: 29240 differences on string compares, 0 round the loop errors with quadmath_snprintf() and 0 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))
41 sig fig 30171 tests: 29833 differences on string compares, 0 round the loop errors with quadmath_snprintf() and 0 with ya_s_snprintf(buf,sizeof (buf),"%.*Qe",sf,(x))

Now checking f128 rounding for %e,%f,%g
 1 sig fig 75712 tests: 0 differences on string compares
 2 sig fig 75712 tests: 0 differences on string compares
 3 sig fig 75712 tests: 0 differences on string compares
 4 sig fig 75712 tests: 0 differences on string compares
 5 sig fig 75712 tests: 0 differences on string compares
 6 sig fig 75712 tests: 0 differences on string compares
 7 sig fig 75712 tests: 0 differences on string compares
 8 sig fig 75712 tests: 0 differences on string compares
 9 sig fig 75712 tests: 0 differences on string compares
10 sig fig 75712 tests: 0 differences on string compares
11 sig fig 75712 tests: 0 differences on string compares
12 sig fig 75712 tests: 0 differences on string compares
13 sig fig 75712 tests: 0 differences on string compares
14 sig fig 75712 tests: 0 differences on string compares
15 sig fig 75712 tests: 0 differences on string compares
16 sig fig 75712 tests: 0 differences on string compares
17 sig fig 75712 tests: 0 differences on string compares
18 sig fig 75712 tests: 0 differences on string compares
19 sig fig 75712 tests: 0 differences on string compares
20 sig fig 60342 tests: 0 differences on string compares
21 sig fig 60342 tests: 0 differences on string compares
22 sig fig 60342 tests: 0 differences on string compares
23 sig fig 60342 tests: 0 differences on string compares
24 sig fig 60342 tests: 0 differences on string compares
25 sig fig 60342 tests: 0 differences on string compares
26 sig fig 60342 tests: 0 differences on string compares
27 sig fig 60342 tests: 0 differences on string compares
28 sig fig 60342 tests: 0 differences on string compares
29 sig fig 60342 tests: 0 differences on string compares
30 sig fig 60342 tests: 0 differences on string compares
31 sig fig 60342 tests: 0 differences on string compares
32 sig fig 60342 tests: 0 differences on string compares
33 sig fig 60342 tests: 0 differences on string compares
34 sig fig 60342 tests: 0 differences on string compares
 Float128 should show 0 string differences at <= 33 sig figs and zero round the loop errors for >= 36 sig figs

Now checking long doubles:
 1 sig fig 30129 tests: 0 differences on string compares, 20127 round the loop errors with my_snprintf() and 20127 with ya_s_snprintf()
 2 sig fig 30129 tests: 0 differences on string compares, 20111 round the loop errors with my_snprintf() and 20111 with ya_s_snprintf()
 3 sig fig 30129 tests: 0 differences on string compares, 20101 round the loop errors with my_snprintf() and 20101 with ya_s_snprintf()
 4 sig fig 30129 tests: 0 differences on string compares, 20091 round the loop errors with my_snprintf() and 20091 with ya_s_snprintf()
 5 sig fig 30129 tests: 0 differences on string compares, 20092 round the loop errors with my_snprintf() and 20092 with ya_s_snprintf()
 6 sig fig 30129 tests: 0 differences on string compares, 20084 round the loop errors with my_snprintf() and 20084 with ya_s_snprintf()
 7 sig fig 30129 tests: 0 differences on string compares, 20080 round the loop errors with my_snprintf() and 20080 with ya_s_snprintf()
 8 sig fig 30129 tests: 0 differences on string compares, 20064 round the loop errors with my_snprintf() and 20064 with ya_s_snprintf()
 9 sig fig 30129 tests: 0 differences on string compares, 20062 round the loop errors with my_snprintf() and 20062 with ya_s_snprintf()
10 sig fig 30129 tests: 0 differences on string compares, 20065 round the loop errors with my_snprintf() and 20065 with ya_s_snprintf()
11 sig fig 30129 tests: 0 differences on string compares, 20051 round the loop errors with my_snprintf() and 20051 with ya_s_snprintf()
12 sig fig 30129 tests: 0 differences on string compares, 20049 round the loop errors with my_snprintf() and 20049 with ya_s_snprintf()
13 sig fig 30129 tests: 0 differences on string compares, 20057 round the loop errors with my_snprintf() and 20057 with ya_s_snprintf()
14 sig fig 30129 tests: 0 differences on string compares, 20055 round the loop errors with my_snprintf() and 20055 with ya_s_snprintf()
15 sig fig 30129 tests: 0 differences on string compares, 20053 round the loop errors with my_snprintf() and 20053 with ya_s_snprintf()
16 sig fig 30129 tests: 0 differences on string compares, 20041 round the loop errors with my_snprintf() and 20041 with ya_s_snprintf()
17 sig fig 30129 tests: 0 differences on string compares, 20034 round the loop errors with my_snprintf() and 20034 with ya_s_snprintf()
18 sig fig 30129 tests: 0 differences on string compares, 20032 round the loop errors with my_snprintf() and 20032 with ya_s_snprintf()
19 sig fig 30129 tests: 30 differences on string compares, 13378 round the loop errors with my_snprintf() and 13378 with ya_s_snprintf()
20 sig fig 30129 tests: 4270 differences on string compares, 3358 round the loop errors with my_snprintf() and 3358 with ya_s_snprintf()
21 sig fig 30129 tests: 9090 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
22 sig fig 30129 tests: 9760 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
23 sig fig 30129 tests: 9829 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
24 sig fig 30129 tests: 9836 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
25 sig fig 30129 tests: 9837 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
26 sig fig 30129 tests: 9850 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
27 sig fig 30129 tests: 36967 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
28 sig fig 30129 tests: 39506 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
29 sig fig 30129 tests: 39747 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
30 sig fig 30129 tests: 39779 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
31 sig fig 30129 tests: 39779 differences on string compares, 0 round the loop errors with my_snprintf() and 0 with ya_s_snprintf()
 Should show 0 string differences at <=18 sig figs and zero round the loop errors for >= 21 sig figs

At end of Part 1 after 103.583 secs : 58494990 tests, 0 errors

Starting PART2 sprintf tests:
Constant strings:
 checking %c:
 checking %s:
 checking %i:
 checking %ld:
 checking %lld:
 checking %g:
 checking %f:
 checking %p:
 Now doing misc tests %b, %'d, %$d, %_$d, etc:
 Checking Wide character handling:
  Initial locale=C
  after setlocale(LC_ALL,NULL), locale=English_United Kingdom.1252
  after setlocale(LC_ALL, ".UTF8"), locale=English_United Kingdom.utf8
  Windows - console code page CP_UTF8 enabled
  Euro symbol = €
  Euro symbol(%lc) "€" as %C is "€" and as %s is "€"
                      "12345"
  Euro symbol (%5lc)  "  €"
  Euro symbol(%-5lc)  "€  "
  Euro symbol(%.5lc)  "€"
  Euro symbol(%5.3lc) "  €"
  Euro symbol(%5.3C)  "  €"
  wide character string test: "賀正" ("happy new year" in Japanese)
                                           "123456789012345678901234567890"
   "賀正" as a utf8 string (%s) is         "賀正"
   "賀正" as a wide string (%ls) is        "賀正"
   "賀正" as a wide string (%S) is         "賀正"
   "賀正" as a wide string (%20ls) is      "                  賀正"
   "賀正" as a wide string (%-20ls) is     "賀正                  "
   "賀正" as a wide string (%.5ls) is      "賀正"
   "賀正" as a wide string (%.*ls)[*=0] is ""
   "賀正" as a wide string (%.*ls)[*=1] is "賀"
   "賀正" as a wide string (%.*ls)[*=2] is "賀正"
   "賀正" as a wide string (%.*ls)[*=3] is "賀正"
   "賀正" as a wide string (%.*ls)[*=4] is "賀正"
   "賀正" as a wide string (%.*ls)[*=5] is "賀正"
   "賀正" as a wide string (%.*ls)[*=6] is "賀正"
   "賀正" as a wide string (%.*ls)[*=7] is "賀正"
   "賀正" as a wide string (%20.5ls) is    "                  賀正"
   "賀正" as a wide string (%20.5S) is     "                  賀正"
 Now checking printing multiple items:
 Now checking variable precision %*.* :
 Now checking %n:
 Checking round loop accuracy of %a and %A with fast_strtod():
 Basic checks for %L and %Q suffixes
  35748 sprintf tests completed on %L and %Q, no unexpected errors found
 Testing %QxXob [128 integers]:
 Testing %Quid [128 integers to decimal]:
 Now checking ya_printf():
  This is produced by ya_printf!
  Test string PI~=3.14159 -1 in hex is 0xffffffff
  -1 (=-1) 340282366920938463463374607431768211455(= -1 as signed128) 3.40282e+38 (same as float128)
  0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000099 (998 zeros then 99)
  0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000099 (998 zeros then 99 to stderr)
All tests completed in 105.277 secs
PART2: 1911700 sprintf tests completed, no errors found
Test program finished - a total of 60406690 tests executed
*/



#define YAPRINTF_DEBUG /* if defined print out reason for each error , otherwise just count errors */
//#define WANT_MINGW_ANSI_STDIO /* if defined force use of mingw printf, even if UCRT is available */


// you will not normally need to touch the settings below
#if defined(__SIZEOF_FLOAT128__)   && !defined(__BORLANDC__)      /* Builder C++ Community version 12.1 patch 1 defines __SIZEOF_FLOAT128__ but __float128's cannot be used in sensible programs due to compiler bugs */
 #define F128_TABLE /* we want the F128 powers of 10 table in table.h for the test program below */
#endif

#if defined(__WIN64) || defined(__WIN32) || defined(__linux)
 #define USE_HR_TIMER /* define to use HR_TIMER to display execution times - supported on windows & Linux */
#endif



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
  
/* Define __USE_MINGW_ANSI_STDIO to 1 to use C99 compatible stdio functions on MinGW. see for example https://stackoverflow.com/questions/44382862/how-to-printf-a-size-t-without-warning-in-mingw-w64-gcc-7-1 */
   /* warning to pass all tests #define __USE_MINGW_ANSI_STDIO 1 , not doing so means you are using UCRT or msvcrt both of which give
      a number of test failures due to lack of functional support in these runtimes */
 //#define __USE_MINGW_ANSI_STDIO 1 /* __USE_MINGW_ANSI_STDIO So mingw uses its printf not msvcrt - code below now forces this as some stdio.h files set this by default */
 				/* WARNING - even with winlib gcc's using UCRT __USE_MINGW_ANSI_STDIO=1 is required to avoid LOTs of errors */
 

#if defined(__linux)
 // assume Linux
#ifdef __x86_64 
 #include <fpu_control.h>	/* needed for WSL 1 fix */
#endif 
 #define YA_SP_LINUX_STYLE /* tell ya_printf() to print to match Linux gcc libc */
#endif

//#define MINGW_SPRINTF /* if defined then test built in sprintf() - gives speed reference */ 

#define YA_SP_SPRINTF_IMPLEMENTATION /* if defined use this new sprintf.  Warning code is actually in header ya_sprintf.h */
#define Show1BitErrors  /* if defined show all 1 bit errors, if not defined then don't */
#define USE_FAST_STRTOD /* if defined use fast_strtod(), otherwise use system strtod() for round loop checks */
// #define PRINT_DIFFS /* if defined print all differences, otherwise only print major ones */
//#define YA_SP_NO_DIGITPAIR /* if defined avoid using a lookup table to convert binary to decimal [00..99] */
	/* on the test program with winlibs gcc 15.2.0 w64/UCRT its the same speed with this defined, whereas defining this on w32/UCRT makes it a little slower (6%) on part 2 of the tests [it does not alter part 1 results ] */

#if  defined(__SIZEOF_INT128__) && (!defined(__BORLANDC__) || (defined(__BORLANDC__) && defined(_UCRT)) )  /*C++Builder 12.1 defines __SIZEOF_INT128__ for both 64 bit versions of the compiler, but "plain" 64 bit compiler fails with link time errors if they are used, 64-bit (Modern) Compiler is OK */
 #define YA_SP_SPRINTF_QI  /* allows printing __int128's in ya_sprintf() via %Qd %I128d etc */
#endif
#if defined(__SIZEOF_FLOAT128__)   && !defined(__BORLANDC__)      /* Builder C++ Community version 12.1 patch 1 defines __SIZEOF_FLOAT128__ for 64 bit(Modern) compiler but __float128's cannot be used in sensible programs due to link time errors if they are used */
 #define YA_SP_SPRINTF_QF  /* allows printing __float128's in ya_sprintf() via %Qg etc , does not need __int128 */
#endif


/*----------------------------------------------------------------------------
 *
 * MIT License:
 *
 * Copyright (c) 2020,2025 Peter Miller
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



#define nos_elements_in(x) (sizeof(x)/(sizeof(x[0]))) /* number of elements in x , max index is 1 less than this as we index 0... */
// #define _FILE_OFFSET_BITS 64 
#if defined(__GNUC__) && defined(__SIZEOF_FLOAT128__)   && !defined(__BORLANDC__)      /* Builder C++ Community version 12.1 patch 1 defines __SIZEOF_FLOAT128__ but __float128's cannot be used in sensible programs due to compiler bugs */
  #include <quadmath.h> /* see https://gcc.gnu.org/onlinedocs/libquadmath/quadmath_005fsnprintf.html#quadmath_005fsnprintf - also needs quadmath library linking in - this is only available with gcc */
#endif
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

/* code below cannot be compiled with -Ofast as this makes the compiler break some C rules that we need, so make sure of this here */
/* we also need -msse2 and -mfpmath=sse to actually use the sse instructions for float and double maths */
/* there seems to be no way to duplicate "-fexcess-precision=standard" using a pragma - so that must be present on the command line [see comments at head of this file that suggest "-fexcess-precision=standard" is not required any more ] */
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__clang__)
 #pragma GCC push_options
 #pragma GCC optimize ("-O3") /* cannot use Ofast, normally -O3 is OK. Note macro expansion does not work here ! */
 // based on  https://jdebp.uk/FGA/predefined-macros-processor.html "__i386__" is set by GCC,Clang,Intel which is good enough as the outer #if limits us to gcc and clang
 #ifdef __i386__
   #pragma GCC target("sse2,fpmath=sse") /* -msse2 and -mfpmath=sse */
 #endif 
#endif


#ifdef USE_HR_TIMER
 #include "../hr_timer/hr_timer.h"
#else
 #include <time.h>
#endif
#ifdef __GLIBC__ /* see https://stackoverflow.com/questions/9705660/check-glibc-version-for-a-particular-gcc-compiler */
#include <gnu/libc-version.h>
#endif

#include "../nan_type/nan_type.h"
#include "../my_printf/my_printf.h"
#include "../power10/table10.h" /* used only in the tests - as it has full power of 10 tables */


#if defined(__BORLANDC__)  && !( LDBL_MANT_DIG>DBL_MANT_DIG && LDBL_MAX_EXP>=DBL_MAX_EXP)  /* work around Builder C++ 12.1 bugs */
 #define ldexpl(x,i) ldexp((double)(x),(i))  /* long double == double - this must be after #include <math.h>. This is not defined (but extern is in math.h) */
 #define frexpl(x,i) frexp((double)(x),(i))  /* this is defined but incorrect in Builder C++ 12.1 */
#endif

enum fpout_type {round_even=1,round_nearest=2,notrailingzeros=4,fmt_g=8};// powers of 2 so can or them together. fmt_g automatically adds notrailingzeros

#ifdef YA_SP_SPRINTF_IMPLEMENTATION
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
 #include "ya_sprintf.h"  /* includes code - not just header  */

 	 /* add macro to convert
	   double_to_str_exp( x, i,round_even,sizeof(new_str), new_str);
	   to  
	   ya_s_snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 	 */ 
#define double_to_str_exp(x,i,rd,size,string) ya_s_snprintf((string),(size),(((rd)&fmt_g)?"%.*g":"%.*e"),(((rd)&fmt_g)?(i):(i-1)),(x))	

#else
 #define ya_s_sprintf(...) (0)
 #define ya_s_snprintf(...) (0)
#endif
#if 1 /* always use my_printf() as this adds %w and gives some flexibility to work around some bugs/issues with the native sprintf() */
   #define snprintf(...) my_snprintf(__VA_ARGS__) /* this scans format string and selects UCRT or mingw code as appropiate */
   #define sprintf(...) my_sprintf(__VA_ARGS__)
   #define USING_SPRINTF "my_sprintf() - which works around limitations of native sprintf()" 
#else
#ifdef __MINGW32__ 
 #if defined(__USE_MINGW_ANSI_STDIO) && __USE_MINGW_ANSI_STDIO==1 && defined(WANT_MINGW_ANSI_STDIO)
  #define snprintf(...) __mingw_snprintf(__VA_ARGS__)
  #define sprintf(...) __mingw_sprintf(__VA_ARGS__)
  #define USING_SPRINTF "__mingw_sprintf() [MINGW_ANSI_STDIO]" 
 #else
  #ifdef _UCRT
   #define snprintf(...) my_snprintf(__VA_ARGS__) /* this scans format string and selects UCRT or mingw code as appropiate */
   #define sprintf(...) my_sprintf(__VA_ARGS__)
   #define USING_SPRINTF "my_sprintf() - which works around limitations of native sprintf()" 
  #else /* if _UCRT NOT defined */
   #if 0 /* if 0 just use native versions, if 1 force ms ones - which only appear to be present for w64? */
    #define snprintf(...) __ms_snprintf(__VA_ARGS__)
    #define sprintf(...) __ms_sprintf(__VA_ARGS__)
    #define USING_SPRINTF "__ms_sprintf()" 
   #else
    #define USING_SPRINTF "sprintf() - standard library function"  
   #endif
  #endif
 #endif
#else
 #define USING_SPRINTF "sprintf() - standard library function" 
#endif 
#endif

#ifdef MINGW_SPRINTF
 	 /* add macro to convert
	   double_to_str_exp( x, i,round_even,sizeof(new_str), new_str);
	   to  
	   snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 	 */ 
#ifdef double_to_str_exp
 #undef double_to_str_exp
#endif
#define double_to_str_exp(x,i,rd,size,string) snprintf((string),(size),(((rd)&fmt_g)?"%.*g":"%.*e"),(((rd)&fmt_g)?(i):(i-1)),(x))	
#undef USING_SPRINTF
#define USING_SPRINTF "sprintf() - standard library function" 
#endif

// TEST_NULL
#ifdef TEST_NULL
 	 /* add macro to convert
	    double_to_str_exp( x, i,round_even,sizeof(new_str), new_str);
	   to  
	    strcpy(new_str,printf_str)
	   i.e. just copy over known correct string
       ONLY works for doubles in part 1 - other tests in part 1 and all of part 2 still call ya_sprintf()
 	 */ 
  #ifdef double_to_str_exp
   #undef double_to_str_exp
  #endif
  #define double_to_str_exp(x,i,rd,size,string) strcpy(string,printf_str)
  //#define ya_s_sprintf(string,...) (strcpy(string,buf),r)    // more work needed to enable these to work
  //#define ya_s_snprintf(string,...) (strcpy(string,buf),r)   // more work needed to enable these to work
  #undef USING_SPRINTF
  #define USING_SPRINTF "sprintf() - TEST_NULL to time double test harness"
#endif


#include "../atof-and-ftoa/atof.h"
#ifndef USE_FAST_STRTOD
#define fast_strtod(s,endptr) strtod((s),(endptr)) /* use system strtod() */
#endif
#ifdef USE_FAST_STRTOD
 #define  strtoflt128(s,endptr)      fast_strtof128((s),(endptr)) /* use my fast converter for f128 */
#endif

#ifdef YAPRINTF_DEBUG
#define dprintf(...) my_printf(__VA_ARGS__)
#else
#define dprintf(...) /* do nothing */ 
#endif



#ifdef z__SIZEOF_INT128__      /* gives a dedefinition of typedef error with clang */
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

uint64_t total_nos_tests=0;// total number of tests conducted (part 1 & 2) [ approximate - depends on your definition of "test" ]


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

#define STRINGIZER(x) # x
#define TO_STRING(x) STRINGIZER(x)

#ifdef YA_SP_SPRINTF_QI /* 128 bit variables (int ) supported by compiler */
// functions to check 128bit int to decimal conversion functions in ya_sprintf
/* print to string which needs to be at least 40 chars long (39 digits plus trailing null).
   This function by Peter Miller 
   returns number of characters put into s (excluding final \0).
*/   
/*      UINT64_MAX 18446744073709551615ULL */
#define P10_UINT64 10000000000000000000ULL /* 19 zeroes */
#define E10_UINT64 19

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
#endif

#ifdef YA_SP_SPRINTF_QF
int chk_fast_strtof128(void)  // tests for fast_strtof128(). These tests will detect a buggy fmaq() implementation
{ 	int errs=0,onebiterrs=0,onebiterrs40sf=0;
    f128_t r128;
	char buf128[128],buf128a[128];
	printf("\n Now checking fast_strtof128():\n"); // f128_t fast_strtof128(const char *s,char **endptr) // if endptr != NULL returns 1st character thats not in the number
	f128_t r128a;
	// test value for string to flt128 conversion. 1st row are entered as doubles so will be different to 3rd row where values are entered as F128 constants. 3rd/4th row also has a wider range including denormalised numbers
	f128_t test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,0.01,0.001,-1e-307,-1e-308,-1e-315,0,4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.001,0.01,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,
		NAN,-NAN,nanl(""),-nanl(""),nanl("0"),-nanl("0"),nanl("1"),-nanl("1"),
		-FLT128_MAX,-1e+4932Q,-1e+4930Q, -1.7976931348623157e308Q ,-12345678.Q ,-10001.Q ,-9999.Q ,-1001.Q ,-345.Q ,-100.Q ,-99.Q ,-20.Q ,-10.Q ,-9.Q ,-8.Q ,-7.Q ,-6.Q ,-5.Q ,-4.Q,-3.Q,-2.Q,-1.Q,-0.5Q,-0.1Q,-0.01Q,-0.001Q,-1e-307Q,-1e-308Q,-1e-315Q,-FLT128_MIN,-6.47517511943802511092443895822764655e-4956Q,-FLT128_DENORM_MIN ,0.Q,FLT128_DENORM_MIN,
		 6.47517511943802511092443895822764655e-4956Q, 6.47517511943802511092443895822764655e-4946Q, 6.47517511943802511092443895822764655e-4936Q,  FLT128_MIN ,4.9406564584124654e-324Q,1e-315Q,2.2250738585072009e-308Q,2.2250738585072014e-308Q,0.001Q,0.01Q,0.1Q,0.5Q,1.Q,2.Q,3.Q,4.Q,5.Q,6.Q,7.Q,8.Q,9.Q,10.Q,20.Q,99.Q,100.Q,101.Q,345.Q,999.Q,1000.Q,1001.Q,1002.Q,1002.5Q,1002.6Q,10000.Q,10001.Q,100001.Q,100001.Q,1000001.Q,12345678.Q,1.7976931348623157e308Q,1e+4930Q,1e+4932Q,FLT128_MAX };
		
	for(int i=0;i<nos_elements_in(test_valued);++i)
		{r128=test_valued[i];
		// start with 34 sf
		total_nos_tests++;
		quadmath_snprintf (buf128, sizeof buf128,"%.33Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.33Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{// dprintf("fast_strtof128(%s) bitwise equal [inf] at 34sf\n",buf128);
				}
			 else
			 	{	
			 	 // dprintf("fast_strtof128(%s) bitwise equal at 34sf\n",buf128);
			 	}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// dprintf("fast_strtof128(%s) character wise equal to 34 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			 dprintf("fast_strtof128(%s) 1 bit differnt  at 34 sf giving %s\n",buf128,buf128a);
#endif			 
			 ++onebiterrs;
			}		
		else
			{++errs; 
		 	 dprintf("Error: fast_strtof128(%s) gave %s at 34sf\n",buf128,buf128a);
			}
		// check with 35 sf
		total_nos_tests++;
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.34Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.34Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{
			 if(isinfq(r128) || isinfq(r128a))
				{// dprintf("fast_strtof128(%s) bitwise equal [inf] at 35sf\n",buf128);
				}
			 else
			 	{
			     // dprintf("fast_strtof128(%s) bitwise equal at 35sf\n",buf128);
				}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// dprintf("fast_strtof128(%s) character wise equal to 35 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			 dprintf("fast_strtof128(%s) 1 bit differnt  at 35 sf giving %s\n",buf128,buf128a);
#endif			 
			 ++onebiterrs;
			}		
		else
			{++errs; 
		 	 dprintf("Error: fast_strtof128(%s) gave %s at 35sf\n",buf128,buf128a);
			}					
		// check with 36 sf	
		total_nos_tests++;
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.35Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.35Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//dprintf("fast_strtof128(%s) bitwise equal [inf] at 36sf\n",buf128);
				}
			 else
			 	{	
			 	 // dprintf("fast_strtof128(%s) bitwise equal at 36 sf\n",buf128);
			 	}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// dprintf("fast_strtof128(%s) character wise equal to 36 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			dprintf("fast_strtof128(%s) 1 bit differnt  at 36 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			}		
		else
			{++errs; 
		 	 dprintf("Error: fast_strtof128(%s) gave %s at 36sf\n",buf128,buf128a);
			}	
		// check with 37 sf	
		total_nos_tests++;
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.36Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.36Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//dprintf("fast_strtof128(%s) bitwise equal [inf] at 37sf\n",buf128);
				}
			 else
			 	{	
			 	 //dprintf("fast_strtof128(%s) bitwise equal at 37 sf\n",buf128);
			 	}
			}	
		else if(strcmp(buf128,buf128a)==0)
			{//dprintf("fast_strtof128(%s) character wise equal to 37 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			dprintf("fast_strtof128(%s) 1 bit differnt  at 37 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			}		
		else
			{++errs; 
			 dprintf("Error: fast_strtof128(%s) gave %s at 37 sf\n",buf128,buf128a);
			}		
		// check with 38 sf	
		total_nos_tests++;
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.37Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.37Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//dprintf("fast_strtof128(%s) bitwise equal [inf] at 38sf\n",buf128);
				}
			 else
			 	{	
				 // dprintf("fast_strtof128(%s) bitwise equal at 38 sf\n",buf128);
				}
			}
		else if(strcmp(buf128,buf128a)==0)
			{//dprintf("fast_strtof128(%s) character wise equal to 38 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			dprintf("fast_strtof128(%s) 1 bit differnt  at 38 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			}	
		else
			{++errs; 
		 	 dprintf("Error: fast_strtof128(%s) gave %s at 38 sf\n",buf128,buf128a);
			}	
		// check with 39 sf	
		total_nos_tests++;
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.38Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.38Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//dprintf("fast_strtof128(%s) bitwise equal [inf] at 39sf\n",buf128);
				}
			 else
			 	{	
				 // dprintf("fast_strtof128(%s) bitwise equal at 39 sf\n",buf128);
				}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// dprintf("fast_strtof128(%s) character wise equal to 39 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			dprintf("fast_strtof128(%s) 1 bit differnt  at 39 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			}	
		else
			{++errs; 
		 	 dprintf("Error: fast_strtof128(%s) gave %s at 39 sf\n",buf128,buf128a);
			}
		// check with 40 sf	(well over what would fit into a uint128 as 2^128=8.4e38)
		total_nos_tests++;
		r128=test_valued[i];
		quadmath_snprintf (buf128, sizeof buf128,"%.39Qe",r128); // 33 is full precision for full 128bit ieee double
		r128a=fast_strtof128(buf128,NULL);
		r128=strtoflt128(buf128,NULL); // assumed accurate answer
		quadmath_snprintf (buf128a, sizeof buf128a,"%.39Qe",r128a); // 33 is full precision for full 128bit ieee double
		if(r128==r128a || (isinfq(r128) && isinfq(r128a))) 
			{if(isinfq(r128) || isinfq(r128a))
				{//dprintf("fast_strtof128(%s) bitwise equal [inf] at 40 sf\n",buf128);
				}
			 else
			 	{	
			 	 // dprintf("fast_strtof128(%s) bitwise equal at 40 sf\n",buf128);
			 	}
			}
		else if(strcmp(buf128,buf128a)==0)
			{// dprintf("fast_strtof128(%s) character wise equal to 40 sf (ie round loop exact)\n",buf128);
			}
		else if(nextafterq(r128a,r128)==r128)
			{
#ifdef Show1BitErrors				
			dprintf("fast_strtof128(%s) 1 bit differnt  at 40 sf giving %s\n",buf128,buf128a);
#endif			
			 ++onebiterrs;
			 ++onebiterrs40sf; // record 40sf 1 bit errors especially
			}	
		else
			{++errs; 
		 	 dprintf("Error: fast_strtof128(%s) gave %s at 40 sf\n",buf128,buf128a);
			}			
		}
	printf(" Results for fast_strtof128() tests:\n");	
	if(onebiterrs) printf("  %d 1 bit errors found (%d at 40sf)\n",onebiterrs,onebiterrs40sf);
	else printf("  No 1 bit errors found\n");		
	if(errs) printf("  %d multi-bit errors found\n",errs);
	else printf("  No multiple bit errors found\n");
	printf("\n");
	return onebiterrs+errs;	  	// should be zero round loop errors at max precision
}

/* more detailed roound loop tests for f128 originally setup for f128_to_a() */
// #define PR_128 /* print failures for test below */
#define f128_to_a(string,x,sg) ya_s_snprintf(string,sizeof (string),"%.*Qe",sg,(x))
int chk_f128_to_a(void)  // tests for f128_to_a(). 
{ 	int errs=0,round_errs=0,round_errsa=0,nos_tests=0;
    __float128 r128;
	char buf128[128],buf128a[128];
	printf("\nNow checking f128's in detail: %s:\n",TO_STRING(f128_to_a(buf,x,sf))); 
	// test values for string to flt128 conversion. 1st row are entered as doubles so will be different to 3rd row where values are entered as F128 constants. 3rd/4th row also has a wider range including denormalised numbers
	__float128 test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,0.01,0.001,-1e-307,-1e-308,-1e-315,0,
		4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.001,0.01,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,
		135.375, /* 135.375 specially selected for rounding tests */
		INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,
		NAN,-NAN,nanl(""),-nanl(""),nanl("0"),-nanl("0"),nanl("1"),-nanl("1"),INFINITY,-INFINITY,
		-FLT128_MAX,-1e+4932Q,-1e+4930Q, -1.7976931348623157e308Q,-12345678.Q,-10001.Q,-9999.Q,-1001.Q,-345.Q,-100.Q,-99.Q,-20.Q,
		-10.Q,-9.Q,-8.Q,-7.Q,-6.Q,-5.Q,-4.Q,-3.Q,-2.Q,-1.Q,-0.5Q,-0.1Q,-0.01Q,-0.001Q,-1e-307Q,-1e-308Q,-1e-315Q,-FLT128_MIN,
		-6.47517511943802511092443895822764655e-4956Q,-FLT128_DENORM_MIN ,0.Q,FLT128_DENORM_MIN,
		 6.47517511943802511092443895822764655e-4956Q, 6.47517511943802511092443895822764655e-4946Q, 6.47517511943802511092443895822764655e-4936Q,  FLT128_MIN ,
		 4.9406564584124654e-324Q,1e-315Q,2.2250738585072009e-308Q,2.2250738585072014e-308Q,0.001Q,0.01Q,0.1Q,0.5Q,
		 1.Q,2.Q,3.Q,4.Q,5.Q,6.Q,7.Q,8.Q,9.Q,10.Q,20.Q,99.Q,100.Q,101.Q,345.Q,999.Q,1000.Q,1001.Q,1002.Q,1002.5Q,1002.6Q,
		 10000.Q,10001.Q,100001.Q,100001.Q,1000001.Q,12345678.Q,1.7976931348623157e308Q,1e+4930Q,1e+4932Q,FLT128_MAX };
	
	for(int sg=0;sg<=40;++sg) // number of significant digits to test
	  {errs=0;;round_errs=0;round_errsa=0;nos_tests=0;
	   for(int nearby=0;nearby<3;++nearby)
	   	   {/* nearby gives +/-1 bit change from reference values */
		   // first check values in test_valued[] which includes values over the whole range and special values like "nan"
		   for(int i=0;i<nos_elements_in(test_valued);++i)
			{r128=test_valued[i];
			 if(nearby==1) r128=nextafterq(r128,FLT128_MAX);// up 1 bit
			 else if(nearby==2) r128=nextafterq(r128,-FLT128_MAX);// down 1 bit
			 nos_tests++;
			 quadmath_snprintf (buf128, sizeof buf128,"%.*Qe",sg,r128);// __FLT128_DECIMAL_DIG__ 36 means 36 digits are needed to guarantee round loop accuracy - this test program shows 34 is OK (for the limited selection of values checked)
			 f128_to_a(buf128a,r128,sg);
			 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
			 if(strcmp(buf128,buf128a)!=0)
			 	{++errs;
	#ifdef PR_128		 	
			 	 printf("** Different quadmath_snprintf() gives %s %s gives %s\n",buf128,TO_STRING(f128_to_a(buf,x,sf)),buf128a);
	#else
			 	 if(sg<=32) printf("** Different quadmath_snprintf(%.20e) gives %s %s gives %s\n",(double)r128,buf128,TO_STRING(f128_to_a(buf,x,sf)),buf128a);
	#endif		 	 
			 	}
			 if((!isnanq(r128) && fast_strtof128(buf128,NULL)!=r128) || ( isnanq(r128) && !isnanq(fast_strtof128(buf128,NULL)) ))
			 	{++round_errs;
	#ifdef PR_128		 	
			 	 printf("** round the loop error (fast_strtof128() of quadmath_snprintf() which gives  %s\n",buf128);
	#endif		 	 
			 	}	
			 if((!isnanq(r128) && fast_strtof128(buf128a,NULL)!=r128) || (isnanq(r128) && !isnanq(fast_strtof128(buf128a,NULL))) )
			 	{++round_errsa;
	#ifdef PR_128		 	
			 	 printf("** round the loop error (fast_strtof128() of %s which gives  %s\n",TO_STRING(f128_to_a(buf,x,sf)),buf128a);
	#endif		 	 
			 	}			 	 	
			}
	
			// now check flt128PosPowersOf10[] 
		   for(int i=0;i<nos_elements_in(flt128PosPowersOf10);++i)
			{r128=flt128PosPowersOf10[i];
			 if(nearby==1) r128=nextafterq(r128,FLT128_MAX);// up 1 bit
			 else if(nearby==2) r128=nextafterq(r128,-FLT128_MAX);// down 1 bit			
			 nos_tests++;
			 quadmath_snprintf (buf128, sizeof buf128,"%.*Qe",sg,r128);// __FLT128_DECIMAL_DIG__ 36 means 36 digits are needed to guarantee round loop accuracy - this test program shows 34 is OK (for the limited selection of values checked)
			 f128_to_a(buf128a,r128,sg);
			 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
			 if(strcmp(buf128,buf128a)!=0)
			 	{++errs;
	#ifdef PR_128		 	
			 	 printf("** Different quadmath_snprintf() gives %s %s gives %s\n",buf128,TO_STRING(f128_to_a(buf,x,sf)),buf128a);
	#else
			 	 if(sg<=32) printf("** Different quadmath_snprintf(%.20e) gives %s %s gives %s\n",(double)r128,buf128,TO_STRING(f128_to_a(buf,x,sf)),buf128a);
	#endif		 	 
			 	}
			 if((!isnanq(r128) && fast_strtof128(buf128,NULL)!=r128) || ( isnanq(r128) && !isnanq(fast_strtof128(buf128,NULL)) ))
			 	{++round_errs;
	#ifdef PR_128		 	
			 	 printf("** round the loop error (fast_strtof128() of quadmath_snprintf() which gives  %s\n",buf128);
	#endif		 	 
			 	}	
			 if((!isnanq(r128) && fast_strtof128(buf128a,NULL)!=r128) || (isnanq(r128) && !isnanq(fast_strtof128(buf128a,NULL))) )
			 	{++round_errsa;
	#ifdef PR_128		 	
			 	 printf("** round the loop error (fast_strtof128() of %s which gives  %s\n",TO_STRING(f128_to_a(buf,x,sf)),buf128a);
	#endif		 	 
			 	}			 	 	
			}
		
		   // now check flt128NegPowersOf10	
		   for(int i=0;i<nos_elements_in(flt128NegPowersOf10);++i)
			{r128=flt128NegPowersOf10[i];
			 if(nearby==1) r128=nextafterq(r128,FLT128_MAX);// up 1 bit
			 else if(nearby==2) r128=nextafterq(r128,-FLT128_MAX);// down 1 bit			
			 nos_tests++;
			 quadmath_snprintf (buf128, sizeof buf128,"%.*Qe",sg,r128);// __FLT128_DECIMAL_DIG__ 36 means 36 digits are needed to guarantee round loop accuracy - this test program shows 35 is OK (for the limited selection of values checked)
			 f128_to_a(buf128a,r128,sg);
			 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
			 if(strcmp(buf128,buf128a)!=0)
			 	{++errs;
	#ifdef PR_128		 	
			 	 printf("** Different quadmath_snprintf() gives %s %s gives %s\n",buf128,TO_STRING(f128_to_a(buf,x,sg)),buf128a);
	#else
			 	 if(sg<=32) printf("** Different quadmath_snprintf(%.20e) gives %s %s gives %s\n",(double)r128,buf128,TO_STRING(f128_to_a(buf,x,sf)),buf128a);
	#endif		 	 
			 	}
			 if((!isnanq(r128) && fast_strtof128(buf128,NULL)!=r128) || ( isnanq(r128) && !isnanq(fast_strtof128(buf128,NULL)) ))
			 	{++round_errs;
	#ifdef PR_128		 	
			 	 printf("** round the loop error (fast_strtof128() of quadmath_snprintf() which gives  %s\n",buf128);			 	 
	#endif		 	 
			 	}	
			 if((!isnanq(r128) && fast_strtof128(buf128a,NULL)!=r128) || (isnanq(r128) && !isnanq(fast_strtof128(buf128a,NULL))) )
			 	{++round_errsa;
	#ifdef PR_128		 	
			 	 printf("** round the loop error (fast_strtof128() of %s which gives  %s\n",TO_STRING(f128_to_a(buf,x,sf)),buf128a);
	#else
				 if(round_errs==0) printf("** round the loop error (fast_strtof128() of %s which gives  %s [should be %s]\n",TO_STRING(f128_to_a(buf,x,sf)),buf128a,buf128);
	#endif		 	 
			 	}			 	 	
			}	
		  }	
	  printf("%2d sig fig %d tests: %d differences on string compares, %d round the loop errors with quadmath_snprintf() and %d with %s\n",sg+1,nos_tests,errs,round_errs,round_errsa,TO_STRING(f128_to_a(buf,x,sf)));
	  total_nos_tests+=nos_tests;
	 } 	
 return round_errs+round_errsa;	  	// should be zero round loop errors aa max precision
}


int chk_f128_rounding(void)  // tests for f128 rounding  
{ 	int errs=0,nos_tests=0,total_errs=0;
    __float128 r128;
	char buf128[128],buf128a[128],buf128e[129];
	printf("\nNow checking f128 rounding for %%e,%%f,%%g\n"); 
	// test values for string to flt128 conversion. 1st row are entered as doubles so will be different to 3rd row where values are entered as F128 constants. 3rd/4th row also has a wider range including denormalised numbers
	__float128 test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,0.01,0.001,-1e-307,-1e-308,-1e-315,0,
		4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.001,0.01,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,
		135.375, /* 135.375 specially selected for rounding tests */
		INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,
		NAN,-NAN,nanl(""),-nanl(""),nanl("0"),-nanl("0"),nanl("1"),-nanl("1"),INFINITY,-INFINITY,
		-FLT128_MAX,-1e+4932Q,-1e+4930Q, -1.7976931348623157e308Q,-12345678.Q,-10001.Q,-9999.Q,-1001.Q,-345.Q,-100.Q,-99.Q,-20.Q,
		-10.Q,-9.Q,-8.Q,-7.Q,-6.Q,-5.Q,-4.Q,-3.Q,-2.Q,-1.Q,-0.5Q,-0.1Q,-0.01Q,-0.001Q,-1e-307Q,-1e-308Q,-1e-315Q,-FLT128_MIN,
		-6.47517511943802511092443895822764655e-4956Q,-FLT128_DENORM_MIN ,0.Q,FLT128_DENORM_MIN,
		 6.47517511943802511092443895822764655e-4956Q, 6.47517511943802511092443895822764655e-4946Q, 6.47517511943802511092443895822764655e-4936Q,  FLT128_MIN ,
		 4.9406564584124654e-324Q,1e-315Q,2.2250738585072009e-308Q,2.2250738585072014e-308Q,0.001Q,0.01Q,0.1Q,0.5Q,
		 1.Q,2.Q,3.Q,4.Q,5.Q,6.Q,7.Q,8.Q,9.Q,10.Q,20.Q,99.Q,100.Q,101.Q,345.Q,999.Q,1000.Q,1001.Q,1002.Q,1002.5Q,1002.6Q,
		 10000.Q,10001.Q,100001.Q,100001.Q,1000001.Q,12345678.Q,1.7976931348623157e308Q,1e+4930Q,1e+4932Q,FLT128_MAX };
	
	for(int sg=0;sg<=33;++sg) // number of significant digits to test
	  {errs=0;nos_tests=0;
	   for(int nearby=0;nearby<3;++nearby)
	   	   {/* nearby gives +/-1 bit change from reference values */
		   // first check values in test_valued[] which includes values over the whole range and special values like "nan"
		   for(int i=0;i<nos_elements_in(test_valued);++i)
			{r128=test_valued[i];
			 if(nearby==1) r128=nextafterq(r128,FLT128_MAX);// up 1 bit
			 else if(nearby==2) r128=nextafterq(r128,-FLT128_MAX);// down 1 bit
			 nos_tests++;
			 quadmath_snprintf (buf128, sizeof buf128,"%.*Qe",sg,r128);
			 ya_s_snprintf(buf128a, sizeof buf128,"%.*Qe",sg,r128);
			 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
			 if(strcmp(buf128,buf128a)!=0)
			 	{++errs;
			 	 quadmath_snprintf (buf128e, sizeof buf128e,"%.35Qe",r128);// "exact"
	#ifdef PR_128		 	
			 	 printf("** Different %%Qe quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#else
			 	 if(sg<=32) printf("** Different %%Qe quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#endif		 	 
			 	}	
			 if(sg<=18 && r128>-1e20 && r128<1e20  )
			 	{/* only do %f over a limited range as otherwise could require a LOT of digits for very large numbers */
				 nos_tests++;
				 quadmath_snprintf (buf128, sizeof buf128,"%.*Qf",sg,r128);
				 ya_s_snprintf(buf128a, sizeof buf128,"%.*Qf",sg,r128);
				 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
				 if(strcmp(buf128,buf128a)!=0)
				 	{++errs;
				 	quadmath_snprintf (buf128e, sizeof buf128e,"%.35Qf",r128);// "exact"
		#ifdef PR_128		 	
				 	 printf("** Different %%Qf quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
		#else
				 	 if(sg<=32) printf("** Different %%Qf quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
		#endif		 	 
				 	}
				}
			 nos_tests++;
			 quadmath_snprintf (buf128, sizeof buf128,"%.*Qg",sg,r128);
			 ya_s_snprintf(buf128a, sizeof buf128,"%.*Qg",sg,r128);
			 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
			 if(strcmp(buf128,buf128a)!=0)
			 	{++errs;
			 	 quadmath_snprintf (buf128e, sizeof buf128e,"%.35Qg",r128);// "exact"
	#ifdef PR_128		 	
			 	 printf("** Different %%Qg quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#else
			 	 if(sg<=32) printf("** Different %%Qg quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#endif		 	 
			 	}				 				 	 	 	
			}
  		  }	
  		
  		// now check flt128PosPowersOf10[] 	
	   for(int nearby=0;nearby<3;++nearby)
	   	   {/* nearby gives +/-1 bit change from reference values */
		   for(int i=0;i<nos_elements_in(flt128PosPowersOf10);++i)
			{r128=flt128PosPowersOf10[i];
			 if(nearby==1) r128=nextafterq(r128,FLT128_MAX);// up 1 bit
			 else if(nearby==2) r128=nextafterq(r128,-FLT128_MAX);// down 1 bit
			 nos_tests++;
			 quadmath_snprintf (buf128, sizeof buf128,"%.*Qe",sg,r128);
			 ya_s_snprintf(buf128a, sizeof buf128,"%.*Qe",sg,r128);
			 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
			 if(strcmp(buf128,buf128a)!=0)
			 	{++errs;
			 	 quadmath_snprintf (buf128e, sizeof buf128e,"%.35Qe",r128);// "exact"
	#ifdef PR_128		 	
			 	 printf("** Different %%Qe quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#else
			 	 if(sg<=32) printf("** Different %%Qe quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#endif		 	 
			 	}	
			 if(sg<=18 && r128>-1e20 && r128<1e20  )
			 	{/* only do %f over a limited range as otherwise could require a LOT of digits for very large numbers */
				 nos_tests++;
				 quadmath_snprintf (buf128, sizeof buf128,"%.*Qf",sg,r128);
				 ya_s_snprintf(buf128a, sizeof buf128,"%.*Qf",sg,r128);
				 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
				 if(strcmp(buf128,buf128a)!=0)
				 	{++errs;
				 	quadmath_snprintf (buf128e, sizeof buf128e,"%.35Qf",r128);// "exact"
		#ifdef PR_128		 	
				 	 printf("** Different %%Qf quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
		#else
				 	 if(sg<=32) printf("** Different %%Qf quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
		#endif		 	 
				 	}
				}
			 nos_tests++;
			 quadmath_snprintf (buf128, sizeof buf128,"%.*Qg",sg,r128);
			 ya_s_snprintf(buf128a, sizeof buf128,"%.*Qg",sg,r128);
			 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
			 if(strcmp(buf128,buf128a)!=0)
			 	{++errs;
			 	 quadmath_snprintf (buf128e, sizeof buf128e,"%.35Qg",r128);// "exact"
	#ifdef PR_128		 	
			 	 printf("** Different %%Qg quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#else
			 	 if(sg<=32) printf("** Different %%Qg quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#endif		 	 
			 	}				 				 	 	 	
			}
  		  }	
  	
		// now check flt128NegPowersOf10		  

	   for(int nearby=0;nearby<3;++nearby)
	   	   {/* nearby gives +/-1 bit change from reference values */
		   for(int i=0;i<nos_elements_in(flt128NegPowersOf10);++i)
			{r128=flt128NegPowersOf10[i];
			 if(nearby==1) r128=nextafterq(r128,FLT128_MAX);// up 1 bit
			 else if(nearby==2) r128=nextafterq(r128,-FLT128_MAX);// down 1 bit
			 nos_tests++;
			 quadmath_snprintf (buf128, sizeof buf128,"%.*Qe",sg,r128);
			 ya_s_snprintf(buf128a, sizeof buf128,"%.*Qe",sg,r128);
			 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
			 if(strcmp(buf128,buf128a)!=0)
			 	{++errs;
			 	 quadmath_snprintf (buf128e, sizeof buf128e,"%.35Qe",r128);// "exact"
	#ifdef PR_128		 	
			 	 printf("** Different %%Qe quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#else
			 	 if(sg<=32) printf("** Different %%Qe quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#endif		 	 
			 	}	
			 if(sg<=18 && r128>-1e20 && r128<1e20  )
			 	{/* only do %f over a limited range as otherwise could require a LOT of digits for very large numbers */
				 nos_tests++;
				 quadmath_snprintf (buf128, sizeof buf128,"%.*Qf",sg,r128);
				 ya_s_snprintf(buf128a, sizeof buf128,"%.*Qf",sg,r128);
				 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
				 if(strcmp(buf128,buf128a)!=0)
				 	{++errs;
				 	quadmath_snprintf (buf128e, sizeof buf128e,"%.35Qf",r128);// "exact"
		#ifdef PR_128		 	
				 	 printf("** Different %%Qf quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
		#else
				 	 if(sg<=32) printf("** Different %%Qf quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
		#endif		 	 
				 	}
				}
			 nos_tests++;
			 quadmath_snprintf (buf128, sizeof buf128,"%.*Qg",sg,r128);
			 ya_s_snprintf(buf128a, sizeof buf128,"%.*Qg",sg,r128);
			 // printf("Checking %s: f128_to_a() gives %s\n",buf128,buf128a);
			 if(strcmp(buf128,buf128a)!=0)
			 	{++errs;
			 	 quadmath_snprintf (buf128e, sizeof buf128e,"%.35Qg",r128);// "exact"
	#ifdef PR_128		 	
			 	 printf("** Different %%Qg quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#else
			 	 if(sg<=32) printf("** Different %%Qg quadmath_snprintf(%s) gives %s ya_s_snprintf gives %s\n",buf128e,buf128,buf128a);
	#endif		 	 
			 	}				 				 	 	 	
			}
  		  }	  		  
  		
	  printf("%2d sig fig %d tests: %d differences on string compares\n",sg+1,nos_tests,errs);
	  total_errs+=errs;
	  total_nos_tests+=nos_tests;
	 } 	
 return total_errs;	  	// should be zero round loop errors at max precision
}


#endif // "Q" / f128

#if defined(LDBL_MAX_10_EXP) && LDBL_MAX_10_EXP==4932 /* "true" long double - so test them */ 
/* check Long doubles */
/* baseline is "my_printf" (which uses UCRT/MingW) and fast_strtoLD */

static long double fast_strtoLD(const char *s,char **endptr) /* dummy version using f128 */
{ 
#if 1 /* use "proper" long double function */
 return fast_strtold(s,endptr);
#else /* use f128 function */
 return fast_strtof128(s,endptr);
#endif
}

#if 0 /* if 1 use 128 bit function instead of long-double function */
 // #define LD_to_a(s,v,digits) {if(isnan(v) && signbit(v)) strcpy(s,"nan");else f128_to_a(s,(__float128)v,digits);}
 #define LD_to_a(string,x,sg) ya_s_snprintf(string,sizeof (string),"%.*Qe",sg,(__float128)(x))
#else
 #define  LD_to_a(s,v,digits) ya_s_snprintf(s,sizeof(s),"%.*Le",digits,(long double)(v))
#endif


#define LDBL_TRUE_MIN __LDBL_DENORM_MIN__ /* for gcc */
#define sg_STR_EXACT (LDBL_DIG - 1) /* -1 as we always print 1 digit before decimal point */
#define sg_ROUND_EXACT (__LDBL_DECIMAL_DIG__ -1) /* LDBL_DECIMAL_DIGIT is 21 so we should be round loop exact from sg=20 */
#define sg_MAX 30 /* LDBL_DECIMAL_DIGIT is 21 so we should be round loop exact from sg=20 - 30 is therefore a reasonable max to check */
int chk_LD_to_a(void)  // tests for long double 
{ 	int errs=0,round_errs=0,round_errsa=0,nos_tests=0,return_errs=0;;
    long double r_LD;
	char buf_LD[128],buf_LDa[128];
	printf("\nNow checking long doubles:\n"); 
	// test values for string to flt128 conversion. 1st row are entered as doubles so will be different to 3rd row where values are entered as L constants. 3rd/4th row also has a wider range including denormalised numbers
	long double test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,0.01,0.001,-1e-307,-1e-308,-1e-315,0,
		4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.001,0.01,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,
		INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,
		NAN,-NAN,nanl(""),-nanl(""),nanl("0"),-nanl("0"),nanl("1"),-nanl("1"),INFINITY,-INFINITY,
		-LDBL_MAX,-1e+4932L,-1e+4930L, -1.7976931348623157e308L ,-12345678.L ,-10001.L ,-9999.L ,-1001.L ,-345.L ,-100.L ,-99.L ,-20.L ,
		-10.L ,-9.L ,-8.L ,-7.L ,-6.L ,-5.L ,-4.L ,-3.L ,-2.L ,-1.L ,-0.5L ,-0.1L ,-0.01L,-0.001L,-1e-307L ,-1e-308L ,-1e-315L,-LDBL_MIN,
		-3.64519953188247460252840593361941982e-4951L,-LDBL_TRUE_MIN ,0.L,LDBL_TRUE_MIN,
		 135.375,-135.375, /* specially selected for rounding tests */
		 3.64519953188247460252840593361941982e-4951L, 3.64519953188247460252840593361941982e-4941L, 3.64519953188247460252840593361941982e-4931L,  LDBL_MIN ,
		 4.9406564584124654e-324L ,1e-315L ,2.2250738585072009e-308L ,2.2250738585072014e-308L ,0.001L,0.01L,0.1L ,0.5L ,
		 1.L ,2.L ,3.L ,4.L ,5.L ,6.L ,7.L ,8.L ,9.L ,10.L ,20.L ,99.L ,100.L,101.L ,345.L ,999.L ,1000.L,1001.L ,1002.L ,1002.5L ,1002.6L ,
		 10000.L,10001.L ,100001.L ,100001.L ,1000001.L ,12345678.L ,1.7976931348623157e308L,1e+4930L,1e+4932L,LDBL_MAX };
	
	for(int sg=0;sg<=sg_MAX;++sg) // number of digits after dp (format x.xxxE+/-xx )
	  {errs=0;;round_errs=0;round_errsa=0;nos_tests=0;
	   for(int nearby=0;nearby<3;++nearby)
	   	   {/* nearby gives +/-1 bit change from reference values */
		   // first check values in test_valued[] which includes values over the whole range and special values like "nan"
		   for(int i=0;i<nos_elements_in(test_valued);++i)
			{r_LD=test_valued[i];
#if 1
			 if(nearby==1) r_LD=nextafterl(r_LD,LDBL_MAX);// up 1 bit
			 else if(nearby==2) r_LD=nextafterl(r_LD,-LDBL_MAX);// down 1 bit
#endif
			 nos_tests++;
			 my_snprintf (buf_LD, sizeof buf_LD,"%.*Le",sg,r_LD);
			 LD_to_a(buf_LDa,r_LD,sg);
			 // printf("Checking %s: LD_to_a() gives %s\n",buf_LD,buf_LDa);
			 if(strcmp(buf_LD,buf_LDa)!=0)
			 	{++errs;
	#ifdef PR_LD		 	
			 	 printf("** Different my_snprintf() gives %s ya_s_snprintf() gives %s\n",buf_LD,buf_LDa);
	#else
			 	 if(sg<=sg_STR_EXACT) {return_errs++; printf("** Different my_snprintf(%.20e) gives %s ya_s_snprintf() gives %s\n",(double)r_LD,buf_LD,buf_LDa);}
	#endif		 	 
			 	}
			 if((!isnan(r_LD) && fast_strtoLD(buf_LD,NULL)!=r_LD) || ( isnan(r_LD) && !isnan(fast_strtoLD(buf_LD,NULL)) ))
			 	{++round_errs;
	#ifdef PR_LD		 	
			 	 printf("** round the loop error (fast_strtoLD() of my_snprintf() which gives  %s\n",buf_LD);
	#else
				if(sg>=sg_ROUND_EXACT)
					{return_errs++;printf("** round the loop error (fast_strtoLD() of my_snprintf() which gives  %s\n",buf_LD);}
	#endif		 	 
			 	}	
			 if((!isnan(r_LD) && fast_strtoLD(buf_LDa,NULL)!=r_LD) || (isnan(r_LD) && !isnan(fast_strtoLD(buf_LDa,NULL))) )
			 	{++round_errsa;
	#ifdef PR_LD		 	
			 	 printf("** round the loop error (fast_strtoLD() of ya_s_snprintf() which gives  %s\n",buf_LDa);
	#else
				if(sg>=sg_ROUND_EXACT) {return_errs++;printf("** round the loop error (fast_strtoLD() of ya_s_snprintf() which gives  %s [should be %s]\n",buf_LDa,buf_LD);}	 	 
	#endif		 	 
			 	}			 	 	
			}
	
			// now check ldblpowersOf10 
		   for(int i=0;i<nos_elements_in(ldblpowersOf10);++i)
			{r_LD=ldblpowersOf10[i];
			 if(nearby==1) r_LD=nextafterl(r_LD,LDBL_MAX);// up 1 bit
			 else if(nearby==2) r_LD=nextafterl(r_LD,-LDBL_MAX);// down 1 bit			
			 nos_tests++;
			 my_snprintf (buf_LD, sizeof buf_LD,"%.*Le",sg,r_LD);
			 LD_to_a(buf_LDa,r_LD,sg);
			 // printf("Checking %s: LD_to_a() gives %s\n",buf_LD,buf_LDa);
			 if(nearby==0) /* if exact power of 10 we know required result exactly */
			 	{char buf10[128],*b10;
			 	 b10=buf10; // exact conversion 
			 	 *b10++='1';
			 	 if(sg>0) *b10++='.';
			 	 for(int dp=1;dp<=sg;++dp)
			 	 	*b10++='0';
			 	 *b10++='e';
			 	 char cexp[4];// holds ascii decimal exponent , lsd 1st
				 char *pcexp=cexp;
				 int exp_digits;
				 int tens=i; // +ve powers of 10 
				 // exponent sign
				 if(tens<0)
				 	{*b10++='-';
				 	 tens=-tens;
				 	}
				 else *b10++='+';
				 /* "simple" way to extract exponent digits, uses 1 / and 1 % operation/digit. */
				 while(tens>0) // convert lsd 1st, keep going till tens=0
					{
				 	 int e1=tens%10;// extract lsd
				 	 tens/=10;
				 	 *pcexp++=e1+'0';
					} 
				exp_digits=pcexp-cexp;// nos digits actually created above (0..4)
				#define min_exp_digits 2
				for(;exp_digits<min_exp_digits;++exp_digits)
					*b10++='0'; // put in leading zeros as required
				while(pcexp>cexp)
					*b10++=*--pcexp;// copy over digits (reversing the order as we created lsd 1st, and we need msd 1st
				*b10=0;
				/* should now have exact result in buf10 - compare it */
 #if 0 /* ignore errors for my_snprintf */					
				if(strcmp(buf_LD,buf10)!=0)
				 	{++errs;
		#ifdef PR_LD		 	
				 	 printf("** Different (power10) exact %s : my_snprintf() gives %s\n",buf10,buf_LD);
		#else
				 	 if(sg<=sg_STR_EXACT) printf("** Different (power10) exact %s : my_snprintf() gives %s\n",buf10,buf_LD);
		#endif		 	 
				 	}
 #endif					 	
				 if(strcmp(buf10,buf_LDa)!=0)
				 	{++errs;
		#ifdef PR_LD		 	
				 	 printf("** Different (power10)  exact %s ya_s_snprintf() gives %s\n",buf10,buf_LDa);
		#else
				 	 if(sg<=sg_STR_EXACT) {return_errs++;printf("** Different (power10)  exact %s ya_s_snprintf() gives %s\n",buf10,buf_LDa);}
		#endif		 	 
				 	}				 	
				}								 
			 if(strcmp(buf_LD,buf_LDa)!=0)
			 	{++errs;
	#ifdef PR_LD		 	
			 	 printf("** Different my_snprintf() gives %s ya_s_snprintf() gives %s\n",buf_LD,buf_LDa);
	#else
			 	 if(sg<=sg_STR_EXACT) {return_errs++;printf("** Different my_snprintf(%.20e) gives %s ya_s_snprintf() gives %s\n",(double)r_LD,buf_LD,buf_LDa);}
	#endif		 	 
			 	}
			 if((!isnan(r_LD) && fast_strtoLD(buf_LD,NULL)!=r_LD) || ( isnan(r_LD) && !isnan(fast_strtoLD(buf_LD,NULL)) ))
			 	{++round_errs;
	#ifdef PR_LD		 	
			 	 printf("** round the loop error (fast_strtoLD() of my_snprintf() which gives  %s\n",buf_LD);
	#else
				if(sg>=sg_ROUND_EXACT)
					{return_errs++;printf("** round the loop error (fast_strtoLD() of my_snprintf() which gives  %s\n",buf_LD);}			 	 
	#endif		 	 
			 	}	
			 if((!isnan(r_LD) && fast_strtoLD(buf_LDa,NULL)!=r_LD) || (isnan(r_LD) && !isnan(fast_strtoLD(buf_LDa,NULL))) )
			 	{++round_errsa;
	#ifdef PR_LD		 	
			 	 printf("** round the loop error (fast_strtoLD() of ya_s_snprintf() which gives  %s\n",buf_LDa);
	#else
				 if(sg>=sg_ROUND_EXACT) {return_errs++;printf("** round the loop error (fast_strtoLD() of ya_s_snprintf() which gives  %s [should be %s]\n",buf_LDa,buf_LD);}			 	 
	#endif		 	 
			 	}			 	 	
			}
		
		   // now check ldblnegpowersOf10	
		   for(int i=0;i<nos_elements_in(ldblnegpowersOf10);++i)
			{r_LD=ldblnegpowersOf10[i];
			 if(nearby==1) r_LD=nextafterl(r_LD,LDBL_MAX);// up 1 bit
			 else if(nearby==2) r_LD=nextafterl(r_LD,-LDBL_MAX);// down 1 bit			
			 nos_tests++;
			 my_snprintf (buf_LD, sizeof buf_LD,"%.*Le",sg,r_LD);
			 LD_to_a(buf_LDa,r_LD,sg);
			 // printf("Checking %s: LD_to_a() gives %s\n",buf_LD,buf_LDa);
			 if(nearby==0 && r_LD>=LDBL_MIN) /* if exact power of 10 (and not denormalised) we know required result exactly */
			 	{char buf10[128],*b10;
			 	 b10=buf10; // exact conversion 
			 	 *b10++='1';
			 	 if(sg>0) *b10++='.';
			 	 for(int dp=1;dp<=sg;++dp)
			 	 	*b10++='0';
			 	 *b10++='e';
			 	 char cexp[4];// holds ascii decimal exponent , lsd 1st
				 char *pcexp=cexp;
				 int exp_digits;
				 int tens=-i; // -ve powers of 10 
				 // exponent sign
				 if(tens<0)
				 	{*b10++='-';
				 	 tens=-tens;
				 	}
				 else *b10++='+';
				 /* "simple" way to extract exponent digits, uses 1 / and 1 % operation/digit. */
				 while(tens>0) // convert lsd 1st, keep going till tens=0
					{
				 	 int e1=tens%10;// extract lsd
				 	 tens/=10;
				 	 *pcexp++=e1+'0';
					} 
				exp_digits=pcexp-cexp;// nos digits actually created above (0..4)
				#define min_exp_digits 2
				for(;exp_digits<min_exp_digits;++exp_digits)
					*b10++='0'; // put in leading zeros as required
				while(pcexp>cexp)
					*b10++=*--pcexp;// copy over digits (reversing the order as we created lsd 1st, and we need msd 1st
				*b10=0;
				/* should now have exact result in buf10 - compare it */
 #if 0 /* ignore errors for my_snprintf */				
				if(strcmp(buf_LD,buf10)!=0)
				 	{++errs;
		#ifdef PR_LD		 	
				 	 printf("** Different (power10) exact %s : my_snprintf() gives %s\n",buf10,buf_LD);
		#else
				 	 if(sg<=sg_STR_EXACT) printf("** Different (power10) exact %s : my_snprintf() gives %s\n",buf10,buf_LD);
		#endif		 	 
				 	}
 #endif	 				 	
				 if(strcmp(buf10,buf_LDa)!=0)
				 	{++errs;
		#ifdef PR_LD		 	
				 	 printf("** Different (power10)  exact %s ya_s_snprintf() gives %s\n",buf10,buf_LDa);
		#else
				 	 if(sg<=sg_STR_EXACT) {return_errs++;printf("** Different (power10)  exact %s ya_s_snprintf() gives %s\n",buf10,buf_LDa);}
		#endif		 	 
				 	}				 	
				}		
	 	
			 if(strcmp(buf_LD,buf_LDa)!=0)
			 	{++errs;
	#ifdef PR_LD		 	
			 	 printf("** Different my_snprintf() gives %s ya_s_snprintf() gives %s\n",buf_LD,buf_LDa);
	#else
			 	 if(sg<=sg_STR_EXACT) {return_errs++;printf("** Different my_snprintf(%.20e) gives %s ya_s_snprintf() gives %s\n",(double)r_LD,buf_LD,buf_LDa);}
	#endif		 	 
			 	}
			 if((!isnan(r_LD) && fast_strtoLD(buf_LD,NULL)!=r_LD) || ( isnan(r_LD) && !isnan(fast_strtoLD(buf_LD,NULL)) ))
			 	{++round_errs;
	#ifdef PR_LD		 	
			 	 printf("** round the loop error (fast_strtoLD() of my_snprintf() which gives  %s\n",buf_LD);	
	#else			 	 
				if(sg>=sg_ROUND_EXACT)
					{return_errs++;printf("** round the loop error (fast_strtoLD() of my_snprintf() which gives  %s\n",buf_LD);}				  		 	 
	#endif		 	 
			 	}	
			 if((!isnan(r_LD) && fast_strtoLD(buf_LDa,NULL)!=r_LD) || (isnan(r_LD) && !isnan(fast_strtoLD(buf_LDa,NULL))) )
			 	{++round_errsa;
	#ifdef PR_LD		 	
			 	 printf("** round the loop error (fast_strtoLD() of ya_s_snprintf() which gives  %s\n",buf_LDa);
	#else
				 if(sg>=sg_ROUND_EXACT) {return_errs++;printf("** round the loop error (fast_strtoLD() of ya_s_snprintf() which gives  %s [should be %s]\n",buf_LDa,buf_LD);}
	#endif		 	 
			 	}			 	 	
			}	
		  }	
	  printf("%2d sig fig %d tests: %d differences on string compares, %d round the loop errors with my_snprintf() and %d with ya_s_snprintf()\n",sg+1,nos_tests,errs,round_errs,round_errsa);
	  total_nos_tests+=nos_tests;
	 } 	
 printf(" Should show 0 string differences at <=%d sig figs and zero round the loop errors for >= %d sig figs\n", sg_STR_EXACT+1,	sg_ROUND_EXACT+1);  
 return return_errs; // total "unexpected" errors	
}

#endif /* TEST LD */


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

const int dbl_expect_err_sf=13; // by default <13 used. Below this value string compare errors are reported

void check_float_to_str(char *in_str,double x)
{char printf_str[50];
 char new_str[50];
 double nx;
 int i;
 if(in_str==NULL) in_str=printf_str; // allow in_str to be null (if for example x is the result of a calculation rather than a constant)
 if(isfinite(x) || isnan(x) || isinf(x)) // 1st was originally isnormal , now isfinite to include subnormals
  { // do now denormalised numbers here. we also check "round the loop" conversions below which also checks the results are OK for denormalised & normal numbers
   for(i=1;i<=19;++i)
 	{nos_tests++;
 	 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);
 	 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str);
 	 if((isnormal(x) && strcmp(printf_str,new_str)!=0) ||(!isnormal(x) &&  !isnan(x) && !isinf(x) && fast_strtod(printf_str,NULL)!=fast_strtod(new_str,NULL)  ) ) 
 	 	{ // different - for normals we use string compare, for subnormals we convert to doubles via fast_strtod() and compare as there are multiple valid sub-normal representations [which shows up with the dconvert based approach in particular]
 	 	  errs++;
 	 	  errsf[i]++;
#ifndef PRINT_DIFFS 	 
		  if(i<dbl_expect_err_sf)	  // with standard Mingw runtime expect differences at 13 and above
#endif		 
			{du.d=x; 
 	 	     dprintf("Different: %s %.20e (0X%016"PRIx64 ") to %d sg printf=>\"%s\" new=>\"%s\"\n",in_str,x,du.u,i,printf_str,new_str);  
 	 	     snprintf(printf_str,sizeof(printf_str),"%.*e",19,x);	
 	         double_to_str_exp( x, 20,round_even,sizeof(new_str), new_str);
 	         dprintf("   to 20 sf printf=>\"%s\" new=>\"%s\"\n",printf_str,new_str);
 	         if(isnormal(x)) dprintf("   value is \"normal\" so test was comparing strings \"%s\" and \"%s\"\n",printf_str,new_str);
 	         if(!isnormal(x) &&  !isnan(x) && !isinf(x)) dprintf("   value is \"sub-normal\" so test was comparing doubles %.20e and %.20e\n",fast_strtod(printf_str,NULL),fast_strtod(new_str,NULL) );
 	         printf(" Value %.20e is:\n",x);
 	         printf("  isnormal() returns %s\n",isnormal(x)?"true":"false");
 	#ifdef isnormal
 			 printf("  isnormal macro defined as %s\n",TO_STRING(isnormal(x)));
 	#endif 	 
	         printf("  isnormal((double)x) returns %s\n",isnormal((double)x)?"true":"false");
 	         printf("  isfinite() returns %s\n",isfinite(x)?"true":"false");
 	         printf("  isnan() returns %s\n",isnan(x)?"true":"false");
 	         printf("  isinf() returns %s\n",isinf(x)?"true":"false");
			 int fpc=fpclassify((double)x);
			 printf(" fpclassify returns 0x%x = ",fpc);
			 if(fpc==FP_INFINITE) printf("FP_INFINITE\n");
			 if(fpc==FP_NAN) printf("FP_NAN\n");
			 if(fpc==FP_NORMAL) printf("FP_NORMAL\n");
			 if(fpc==FP_SUBNORMAL) printf("FP_SUBNORMAL\n");
			 if(fpc==FP_ZERO) printf("FP_ZERO\n");
    #ifndef  __BORLANDC__
			 printf(" __fpclassify(x=%g) returns 0x%x , __fpclassifyf(x) returns 0x%x __fpclassifyl(x) returns 0x%x\n",x,__fpclassify(x),__fpclassifyf(x),__fpclassifyl(x));
			 printf(" __fpclassify(du.d=%g) returns 0x%x , __fpclassifyf(du.d) returns 0x%x __fpclassifyl(du.d) returns 0x%x\n",du.d,__fpclassify(du.d),__fpclassifyf(du.d),__fpclassifyl(du.d));
	#endif
			 printf(" __builtin_isfinite(x) returns %s\n",__builtin_isfinite(x)?"true":"false");
			 printf(" __builtin_isnormal(x) returns %s\n",__builtin_isnormal(x)?"true":"false");
			 printf(" __builtin_isnan(x) returns %s\n",__builtin_isnan(x)?"true":"false");
			 printf(" __builtin_isinf_sign(x) returns %s\n",__builtin_isinf_sign(x)?"true":"false");
			 printf(" __builtin_signbit(x) returns %s\n",__builtin_signbit(x)?"true":"false");
			 
			 
			 printf(" x= %.20e (0X%016"PRIx64 ")\n",x,du.u);
			 uint64_t mantissa = du.u & 0xFFFFFFFFFFFFFULL;
			 int expo=(int)((du.u>>52) & 0x7ff) ; 
			 bool isdenormal=(expo==0 && mantissa!=0);
			 printf(" Mantissa=0X%016"PRIx64 ", biased exponent=%d (unbiased=%d), isdenormal=%s\n",mantissa,expo,expo-1022,isdenormal?"true":"false"); 			 
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
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // different when 21 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl21++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl21_1bit++;		
#ifdef  Show1BitErrors
		  	 dprintf("Double 1 bit Different: %s (%.21g) to 21 sg new=>\"%s\" which as a double is %.21g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 dprintf("Double > 1 bit Different: %s (%.21g) to 21 sg new=>\"%s\" which as a double is %.21g\n",in_str,x,new_str,nx);
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // printf different when 21 sig digits version converted back to double
 	 	  errs_printf21++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  dprintf("printf Double Different: %s (%.21g) to 21 sg printf=>\"%s\" which as a double is %.21g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    } 
 	
 i=20; // check 20 sf (my fast_strtod() uses 19sf and uses 20th for rounding)
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // different when 20 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl20++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl20_1bit++;	
#ifdef Show1BitErrors			  	  	
		  	 dprintf("Double 1 bit Different: %s (%.20g) to 20 sg new=>\"%s\" which as a double is %.20g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 dprintf("Double > 1 bit Different: %s (%.20g) to 20 sg new=>\"%s\" which as a double is %.20g\n",in_str,x,new_str,nx);
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // printf different when 20 sig digits version converted back to double
 	 	  errs_printf20++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  dprintf("printf Double Different: %s (%.20g) to 20 sg printf=>\"%s\" which as a double is %.20g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    } 
 i=19; 
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // different when 19 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl19++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl19_1bit++;	
#ifdef Show1BitErrors			  	  	
		  	 dprintf("Double 1 bit Different: %s (%.19g) to 19 sg new=>\"%s\" which as a double is %.19g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 dprintf("Double > 1 bit Different: %s (%.19g) to 19 sg new=>\"%s\" which as a double is %.19g\n",in_str,x,new_str,nx);
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // printf different when 19 sig digits version converted back to double
 	 	  errs_printf19++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  dprintf("printf Double Different: %s (%.19g) to 19 sg printf=>\"%s\" which as a double is %.19g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    } 

 // now repeat with 18 sig figs
 i=18;
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // different when 18 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl18++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl18_1bit++;		  
#ifdef Show1BitErrors			  	
		  	 dprintf("Double 1 bit Different: %s (%.18g) to 18 sg new=>\"%s\" which as a double is %.18g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{ 	  		
			 dprintf("Double > 1 bit Different: %s (%.18g) to 18 sg new=>\"%s\" which as a double is %.18g\n",in_str,x,new_str,nx);		 
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // printf different when 18 sig digits version converted back to double
 	 	  errs_printf18++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  dprintf("printf Double Different: %s (%.18g) to 18 sg printf=>\"%s\" which as a double is %.18g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    }
 // now repeat with 17 sig figs
 i=17;
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // different when 17 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl17++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl17_1bit++;	 	
#ifdef Show1BitErrors			  	  	
		  	 dprintf("Double 1 bit Different: %s (%.17g) to 17 sg new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 dprintf("Double > 1 bit Different: %s (%.17g) to 17 sg new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // printf different when 17 sig digits version converted back to double
 	 	  errs_printf17++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  dprintf("printf Double Different: %s (%.17g) to 17 sg printf=>\"%s\" which as a double is %.17g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    }
  // check other formats as 17 sf - with traling zero deleted
 nos_tests++; 
 double_to_str_exp( x, i,round_even|notrailingzeros,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // different when 17 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl17++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl17_1bit++;	 
#ifdef Show1BitErrors			  		  	
		  	 dprintf("Double 1 bit Different: %s (%.17g) to 17 sg notrailingzeros new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 dprintf("Double > 1 bit Different: %s (%.17g) to 17 sg notrailingzeros new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);
			}
 	    }
 nos_tests++; 
 double_to_str_exp( x, i,round_even|fmt_g,sizeof(new_str), new_str); // "g" format
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // different when 17 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl17++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl17_1bit++;	 		  
#ifdef Show1BitErrors			  	
		  	 dprintf("Double 1 bit Different: %s (%.17g) to 17 sg fmt_g new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);	  	 
#endif		  	 
		  	}
 	 	  else
 	 	  	{
			 dprintf("Double > 1 bit Different: %s (%.17g) to 17 sg fmt_g new=>\"%s\" which as a double is %.17g\n",in_str,x,new_str,nx);
			}
 	    }		   
 // now repeat with 16 sig figs
 i=16;
 nos_tests++; 	    
 snprintf(printf_str,sizeof(printf_str),"%.*e",i-1,x);	
 double_to_str_exp( x, i,round_even,sizeof(new_str), new_str); 
 nx=fast_strtod(new_str,NULL);// convert back to double	    
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // different when 16 sig digits version converted back to double
 	 	  errs++;
 	 	  errs_dbl16++;
		  if(nx==nextafter(x,nx))
		  	{errs_dbl16_1bit++;
#if defined(PRINT_DIFFS) && defined(Show1BitErrors) 	 		  	
		  	 dprintf("Double 1 bit Different: %s (%.16g) to 16 sg new=>\"%s\" which as a double is %.16g\n",in_str,x,new_str,nx);
#endif		  	 
		  	}
 	 	  else
 	 	  	{
#ifdef PRINT_DIFFS 	 
			 dprintf("Double > 1 bit Different: %s (%.16g) to 16 sg new=>\"%s\" which as a double is %.16g\n",in_str,x,new_str,nx);
#endif			 
			}
 	    }
 nx=fast_strtod(printf_str,NULL);// check printf for comparison
 if((!isnan(x) && nx!=x) || (isnan(x) && isnan(nx)!=isnan(x)))
 	 	{ // printf different when 16 sig digits version converted back to double
 	 	  errs_printf16++;
#ifdef PRINT_DIFFS 	  	 	  
 	 	  dprintf("printf Double Different: %s (%.16g) to 16 sg printf=>\"%s\" which as a double is %.16g\n",in_str,x,printf_str,nx);
#endif 	 	  
 	    }
}


/* support functions for checking ya_s_sprintf() 
*/
unsigned int serrs=0,scnt=0;
/* simple strings */
void check_str_s(char *x)
{
 int r_ya,r=0;// return codes
 static char buf_ya[1000],buf[1000];// need to be big as we check sprintf where result is unlimited
 // first just check printing a literal string
 if(x==NULL)
 	{// special case as sprintf() crashes - check that ya_s_snprintf() is OK
 	 scnt++;
 	 r_ya=ya_s_sprintf(buf_ya,x);
 	 if(0!=r_ya){ ++serrs;dprintf ("<NULL>: ya_sprintf() returns %d expected 0\n",r_ya);}
 	  // repeat with snprintf and n=5
 	 scnt++;
 	 r_ya=ya_s_snprintf(buf_ya,5,x); 
 	 if(0!=r_ya){ ++serrs;dprintf ("<NULL>: ya_snprintf(5) returns %d expected 0\n",r_ya);}
 	 return;
 	}
 /* # pragma 's below turn off gcc warning for 	[-Wformat-security] which complains about formats being variable but no arguments being supplied */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security" 	
 scnt++;	
 r=sprintf(buf,x);
 r_ya=ya_s_sprintf(buf_ya,x);
 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",x,buf,buf_ya);};
 // repeat with snprintf and n=5
 scnt++;
 r=snprintf(buf,5,x);
 r_ya=ya_s_snprintf(buf_ya,5,x);
 if(r!=r_ya){ ++serrs;dprintf ("%s: snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,buf,buf_ya);};
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
 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",x,buf,buf_ya);};	
 // repeat with snprintf and n=5
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s: snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,buf,buf_ya);};	
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
 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",x,buf,buf_ya);};
 scnt++;
 	
 PAR="abcdefg"; /* string to print */
 r=sprintf(buf,x,PAR);
 r_ya=ya_s_sprintf(buf_ya,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",x,buf,buf_ya);};
 scnt++;
 if(strcmp(x,"%s")==0 && strcmp(PAR,buf_ya)) {++serrs;dprintf("%s: ya_sprintf() gives %s\n",x,buf_ya);}; // check string matches original when we just have %s
	// repeat with snprintf and n=5	
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s: snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,buf,buf_ya);};

	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s: snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,buf,buf_ya);};

	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s: snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf(10) gives %s ya_sprintf(10) gives %s\n",x,buf,buf_ya);}
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
 if(r!=r_ya){ ++serrs;dprintf ("%s(%d): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%d): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);};
 // now check round the loop, only if %d at start of format string
 r=strtol(buf,NULL,0);
 r_ya=strtol(buf_ya,NULL,0);
 scnt++;
 if(*x=='%' && r!=r_ya) {++serrs;dprintf("%s(%d): ya_sprintf() gives %d (%s) which <> %d!\n",x,PAR,r_ya,buf_ya,r);};
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%d): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%d): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%d): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%d): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%d): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%d): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
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
 if(r!=r_ya){ ++serrs;dprintf ("%s(%ld): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%ld): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);};
 long lr,lr_ya;
 lr=strtol(buf,NULL,0);
 lr_ya=strtol(buf_ya,NULL,0);
 scnt++;
 if(*x=='%' && lr!=lr_ya) {++serrs;dprintf("%s(%ld): ya_sprintf() gives %ld (%s) which <> %ld!\n",x,PAR,lr_ya,buf_ya,lr);}; // check "round the loop"
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%ld): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%ld): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%ld): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%ld): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%ld): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%ld): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
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
 if(r!=r_ya){ ++serrs;dprintf("%s(%lld): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%lld): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);};
 long long lr,lr_ya;
 lr=strtoll(buf,NULL,0);
 lr_ya=strtoll(buf_ya,NULL,0);
 scnt++;
 if(*x=='%' && lr!=lr_ya) {++serrs;dprintf("%s(%lld): ya_sprintf() gives %lld (%s) which <> %lld!\n",x,PAR,lr_ya,buf_ya,lr);}; // check "round the loop" 
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf("%s(%lld): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%lld): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf("%s(%lld): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%lld): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%lld): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%lld): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
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
 bool charafterg=false;
 for(char *x1=x;*x1;++x1)
 	if(tolower(*x1)=='g' && x1[1]!=0) charafterg=true;// char after g eg "%-9g1"
 if(*x=='%' && !charafterg) // check "round the loop" when %g is at the start of the string and there is nothing after the 'g' (if there is something after the 'g', in particular a number strtod() may not return whats expected
 	{scnt++;
     if(!(fast_strtod(buf,NULL)==fast_strtod(buf_ya,NULL)|| (isnan(fast_strtod(buf,NULL)) && isnan(fast_strtod(buf,NULL))) || (isinf(fast_strtod(buf,NULL)) && isinf(fast_strtod(buf,NULL))) )) {++serrs;dprintf("%s(%g):[strtod()] sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);}; 
	}
	// snprintf(5)
#ifdef DO_NOT_STRCMP_SUBNORMALS
 if(!(isnormal(PAR) || isnan(PAR) || isinf(PAR))) 
 	{
	 return; // cannot use string compares for sub-normals with dconvert as it generates different(correct) values [ I tried just n=2 and this still gave some errors]
	}
#endif 
 if(r!=r_ya){ ++serrs;dprintf ("%s(%g): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%g): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);}; 
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%g): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%g): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%g): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%g): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,20,x,PAR); // 20 sig figs
 r_ya=ya_s_snprintf(buf_ya,20,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%g): snprintf(20) returns %d ya_snprintf(20) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%g): snprintf(20) gives %s ya_snprintf(20) gives %s\n",x,PAR,buf,buf_ya);};	
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
 if(r!=r_ya && r<350 ){ ++serrs;dprintf ("%s(%g): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(*x=='%' && !(fast_strtod(buf,NULL)==fast_strtod(buf_ya,NULL)|| (isnan(fast_strtod(buf,NULL)) && isnan(fast_strtod(buf,NULL))) || (isinf(fast_strtod(buf,NULL)) && isinf(fast_strtod(buf,NULL))) )) {++serrs;dprintf("%s(%g):[strtod()] sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);}; 
 // repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya && r<350 ){ ++serrs;dprintf ("%s(%g): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%g): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
#ifdef DO_NOT_STRCMP_SUBNORMALS
 if(!(isnormal(PAR) || isnan(PAR) || isinf(PAR))) return; // cannot use string compares for sub-normals with dconvert as it generates different(correct) values
#endif  
 // snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya && r<350 ){ ++serrs;dprintf ("%s(%g): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%g): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	

	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR); // only expect 10 sig figs to be accurate
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya && r<350 ){ ++serrs;dprintf ("%s(%g): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%g): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
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
 if(r!=r_ya){ ++serrs;dprintf ("%s(%p): sprintf() returns %d ya_sprintf() returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%p): sprintf() gives %s ya_sprintf() gives %s\n",x,PAR,buf,buf_ya);};
	// snprintf(5)
 scnt++;
 r=snprintf(buf,5,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,5,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%p): snprintf(5) returns %d ya_snprintf(5) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%p): snprintf(5) gives %s ya_snprintf(5) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=2	
 scnt++;
 r=snprintf(buf,2,x,PAR);
 r_ya=ya_s_snprintf(buf_ya,2,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%p): snprintf(2) returns %d ya_snprintf(2) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%p): snprintf(2) gives %s ya_snprintf(2) gives %s\n",x,PAR,buf,buf_ya);};	
	// repeat with snprintf and n=10	
 scnt++;
 r=snprintf(buf,10,x,PAR); 
 r_ya=ya_s_snprintf(buf_ya,10,x,PAR);
 if(r!=r_ya){ ++serrs;dprintf ("%s(%p): snprintf(10) returns %d ya_snprintf(10) returns %d\n",x,PAR,r,r_ya);}
 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s(%p): snprintf(10) gives %s ya_snprintf(10) gives %s\n",x,PAR,buf,buf_ya);};	
}

//#if !defined(__BORLANDC__)
#include <xmmintrin.h> /* needed for _mm_getcsr() & _mm_setcsr() which are present in both Windows & Linux */
//#endif

int main(int argc, char *argv[]) 
{ 
#ifdef USE_HR_TIMER
  double time_taken;
  init_HR_Timer(); // zero timer
 #else
  clock_t time_taken;
#endif  
#if defined(__x86_64) && defined(__linux) /* running linux on x86_64 assume we are running on WSL1 and apply a workaround for a WSL1 bug - this should be OK for WSL-2 and linux */
   unsigned short Cw = 0x37f;
   _FPU_SETCW(Cw);	
#endif
#if !defined(__BORLANDC__) /* enables denormalised floating point numbers in case compiler has turned them off (which -Ofast may do with gcc >= 13) Note it does need gcc -msse. Works with Linux & Windows, X32 & x64*/
 _mm_setcsr(_mm_getcsr() & ~0x8040U); // clear FTZ & DAZ bits in MXCSR see https://stackoverflow.com/questions/11671430/flushing-denormalised-numbers-to-zero
#endif
#if 0 /* allow denorms for Windows (only) works for both W32 & W64 - in theory this is portable to Windows/ARM but this is untested ! */
 // note with builder C++ _control87 gives a linker error with "Windows 64-bit", and does not appear to do anything with "Windows 32-bit"
  _control87(_DN_SAVE, _MCW_DN);     /* set FPU control word to allow denorms - see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/control87-controlfp-control87-2?view=msvc-170&redirectedfrom=MSDN */
  //_control87(0, 0x03000000); // values from   https://doxygen.reactos.org/d2/d35/sdk_2include_2ucrt_2float_8h.html &  https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/control87-controlfp-control87-2?view=msvc-170
#endif
#ifdef __clang__
 printf("__clang__ defined =%u\n",__clang__);
#endif
#ifdef __GNUC__
 printf("__GNUC__ defined, __GNUC__=%u __GNUC_MINOR__=%u\n",__GNUC__ ,__GNUC_MINOR__);
#endif
#ifdef _MSC_VER
 printf("_MSC_VER is defined =%u\n",_MSC_VER);
#endif
#ifdef __BORLANDC__
 printf("__BORLANDC__ defined =0X%x __CODEGEARC_VERSION__=0X%x \n",__BORLANDC__,__CODEGEARC_VERSION__);
#endif
#ifdef _M_IX86
 printf("_M_IX86 is defined (32 bit)\n"); /* defined for W32 */
#endif
#ifdef _M_X64
 printf("_M_X64 is defined (64 bit)\n"); /* defined for W64 */
#endif
#ifdef __SSE2__
 printf("__SSE2__ is defined\n");
#endif
#ifdef __MINGW32__ 
 printf("__MINGW32__ is defined\n");
 #ifdef __USE_MINGW_ANSI_STDIO
  printf("__USE_MINGW_ANSI_STDIO is defined as %d\n",(int)(__USE_MINGW_ANSI_STDIO));
 #else
  printf("__USE_MINGW_ANSI_STDIO is NOT defined\n");
 #endif
#else
 printf("__MINGW32__ is NOT defined\n");
#endif 
#ifdef __MINGW64__
 printf("__MINGW64__ is defined\n");
#endif
#ifdef _CRTBLD
 printf("_CRTBLD is defined!\n");
#endif
#ifdef _UCRT
 printf("_UCRT is defined\n");
#endif
#ifdef __MSVCRT__
 printf("__MSVCRT__ is defined\n");
#endif
#ifdef  _WIN32 /* true even on w64 */
 printf("_WIN32 is defined\n");
#endif
#ifdef  _WIN64 /* true even on w64 */
 printf("_WIN64 is defined\n");
#endif
#ifdef __linux
 printf("__linux is defined\n");
#endif
#if defined(__has_builtin) && __has_builtin(__builtin_bswap64) /*  is builtin for gcc and clang */
printf("__builtin_bswap64() is available\n");
#endif 
#if defined(__has_builtin) && __has_builtin(__builtin_clzll) /*  is builtin for gcc and clang */
printf("__builtin_clzll is available\n");
#endif
#if defined(__has_builtin) && __has_builtin(__builtin_clz) /*  is builtin for gcc and clang */
printf("__builtin_clz is available\n");
#endif
#ifdef  YA_SP_LINUX_STYLE /* tell ya_printf() to print to match Linux gcc libc */
 printf("YA_SP_LINUX_STYLE is defined\n");
#endif
#ifdef   YA_SP_NO_NEG_LEADINGPLUS // if defined ignore %+ for unsigned conversions
 printf("YA_SP_NO_NEG_LEADINGPLUS is defined\n");
#endif 
#ifdef   YA_SP_NO_NEG_LEADINGSPACE // if defined ignore %  (% space) for unsigned conversions
 printf("YA_SP_NO_NEG_LEADINGSPACE is defined\n");
#endif 
#ifdef YA_SP_FULL_NULL // if defined only print (null) if it will be fully visible
 printf("YA_SP_FULL_NULL is defined\n");
#endif  
#ifdef YA_SP_A_FMT_ALT1 // if define print %A in an alternative way
 printf("YA_SP_A_FMT_ALT1 is defined\n");
#endif  
#ifdef YA_SP_A_FMT_ALT2 // if define print %A in another alternative way
 printf("YA_SP_A_FMT_ALT2 is defined\n");
#endif  
#ifdef YA_SP_A_FMT_ALT3 // if define print %A in an alternative way
 printf("YA_SP_A_FMT_ALT3 is defined\n");
#endif  
#ifdef YA_SP_A_FMT_ALT4 // if define print %A in another alternative way
 printf("YA_SP_A_FMT_ALT4 is defined\n");
#endif  
#ifdef YA_SP_PTR_0X // if define print pointers with a leading 0X
 printf("YA_SP_PTR_0X is defined\n");
#endif  
#ifdef YA_SP_PTR_CAPS // %p in uppercase hex
 printf("YA_SP_PTR_CAPS is defined\n");
#endif
#ifdef YA_SP_PTR_LEADINGZEROS // if defined always print pointers with leading zeros
 printf("YA_SP_PTR_LEADINGZEROS is defined\n"); 
#endif 

#ifdef YA_SP_SIGNED_NANS /* tell ya_sprintf we want signed NAN's (to match linux gcc libc) */
 printf("YA_SP_SIGNED_NANS is defined\n");
#endif
#ifdef YA_SP_SIGNED_NANS_LD /* tell ya_sprintf we want signed NAN's for long doubles */
 printf("YA_SP_SIGNED_NANS_LD is defined\n");
#endif
#if  defined(YA_SP_SIGNED_NANS_F128) /* signed NANs for float128 only */
 printf("YA_SP_SIGNED_NANS_F128 is defined\n");
#endif
#ifdef YA_SP_NAN_IND // nan(string) printed for some nan's rather than just "nan"
 printf("YA_SP_NAN_IND  is defined\n");
#endif
#ifdef YA_SP_SPRINTF_EXP3
 printf("YA_SP_SPRINTF_EXP3 is defined\n");
#endif
#ifdef YA_SP_WCHAR_PR_CHARS
 printf("YA_SP_WCHAR_PR_CHARS is defined\n");
#endif
#ifdef YA_SP_RYU 
printf("YA_SP_RYU is defined, so using ryu for double->string conversion - WARNING this option is slower than the default solution!\n");
#endif


#ifdef __GLIBC__  /* see https://stackoverflow.com/questions/9705660/check-glibc-version-for-a-particular-gcc-compiler */
  printf("GNU libc compile-time version: %u.%u\n", __GLIBC__, __GLIBC_MINOR__);
  printf("GNU libc runtime version:      %s\n", gnu_get_libc_version());
#else
  printf("GNU C Library not found\n");
#endif
#ifdef YA_SP_SPRINTF_LD 
 printf("YA_SP_SPRINTF_LD defined [ WARNING: its NOT used now by ya_sprintf!]\n");
#endif
#ifdef YA_SP_SPRINTF_Q
 printf("YA_SP_SPRINTF_Q defined\n");
#endif
#ifdef YA_SP_SPRINTF_QI
 printf("YA_SP_SPRINTF_QI defined (128bit integers)\n");
#endif
#ifdef YA_SP_SPRINTF_QF
 printf("YA_SP_SPRINTF_QF defined (128 bit floating point)\n");
#endif

printf("sizeof(long double)=%zu sizeof(double)=%zu sizeof(float)=%zu sizeof(int)=%zu sizeof(char*)=%zu sizeof(int *)=%zu sizeof(size_t)=%zu\n",sizeof(long double), sizeof(double), sizeof(float), sizeof(int), sizeof(char*), sizeof(int *),sizeof(size_t));

#if defined(LDBL_MAX_10_EXP) && LDBL_MAX_10_EXP==4932 /* "true" long double */
 printf("LDBL_MAX_10_EXP defined and equal to 4932 meaning we have \"true\" long doubles\n");
#endif
#ifdef __SIZEOF_FLOAT128__
 printf("__SIZEOF_FLOAT128__ is defined as %u\n",__SIZEOF_FLOAT128__);
#endif
#ifdef __SIZEOF_INT128__
 printf("__SIZEOF_INT128__ is defined as %u\n",__SIZEOF_INT128__);
#endif
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
 printf("Byte order is LITTLE ENDIAN (%d)\n",__ORDER_LITTLE_ENDIAN__);
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
 printf("Byte order is BIG ENDIAN (%d)\n",__ORDER_BIG_ENDIAN__);
#endif
#if defined(_WIN32) && defined(_UCRT) 
 _set_printf_count_output(1); /* enable %n for microsoft runtime - see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/set-printf-count-output?view=msvc-170 */
 printf("Microsoft runtime: %%n support is %s\n",_get_printf_count_output() ? "enabled" : "disabled" );
#endif
printf("\nUsing %s\n\n",USING_SPRINTF);
#ifdef DO_NOT_STRCMP_SUBNORMALS
printf(" Tests will not check sub-normals using string compares as this can cause false failures (DO_NOT_STRCMP_SUBNORMALS defined)\n");
#endif
#if 0  /* some simple checks - used for quickly debugging specific issues */
{char printf_str[4096];
 char new_str[4096];
 double x=4.941e-324;// smallest denormal
 char *fmt="%.20e"; // eg "%.20e"
 printf("Checking x=%g: isfinite=%s,isnan=%s,isinf=%s\n",x,(isfinite(x)?"True":"False"),(isnan(x)?"True":"False"),(isinf(x)?"True":"False"));

 snprintf(printf_str,sizeof(printf_str),fmt,x);
 ya_s_snprintf(new_str,sizeof(new_str),fmt,x);
  //double_to_str_exp( x, i,round_even,sizeof(new_str), new_str);  //   matches "%.*e" with i-1 as param
 printf("snprintf(%s)     =>\"%s\"\n",fmt,printf_str);
 printf("ya_s_snprintf(%s)=>\"%s\"\n",fmt, new_str);
 #ifdef __BORLANDC__
  printf("Press return to exit:\n");
  getchar();
 #endif
 return 1;
 }
#endif
#if defined(PART1_SPRINTF_TESTS) && defined(YA_SP_SPRINTF_IMPLEMENTATION)
	printf("Starting PART1 sprintf tests:\n");
	printf(" Testing doubles:\n");
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
    // check various forms of NAN
    check_double(NAN);
    check_double(-NAN);
    check_double(nan(""));
    check_double(-nan(""));
    check_double(nan("0"));
    check_double(-nan("0"));
    check_double(nan("1"));
    check_double(-nan("1"));
    
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
 	printf(" Average time per test was %.1f ns\n",1e9*time_taken/(double)nos_tests);
 #else
	time_taken=clock();
	printf(" All double round loop tests completed in %g secs\n",(double)time_taken/(double)CLOCKS_PER_SEC);
	printf(" Average time per test was %.1f ns\n",1e9*( (double)time_taken/(double)CLOCKS_PER_SEC )/(double)nos_tests);
#endif 	
	printf(" %" PRIu64 " tests %" PRIu64 " differences\n",nos_tests,errs);
	total_nos_tests+=nos_tests;

#ifdef 	YA_SP_SPRINTF_IMPLEMENTATION
	printf(" Tested ya_sprintf() double-double round loop:\n");
#elif defined(MINGW_SPRINTF)
	printf("Tested MinGW built in sprintf:\n");
#else
	printf("Tested double_to_str_exp:\n"); 
#endif
    // note we set figures after decimal point, but there is 1 digit before decimal point, so we add 1 below to reflect this
	// __DBL_DIG__ 15 , __DBL_DECIMAL_DIG__ 17
 #ifndef DBL_DECIMAL_DIG
   #define DBL_DECIMAL_DIG 17
 #endif
 #ifndef DBL_DIG
   #define DBL_DIG 15
 #endif	
	printf(" %" PRIu64 " errors when 21 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl21,errs_dbl21_1bit,errs_printf21);
	printf(" %" PRIu64 " errors when 20 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl20,errs_dbl20_1bit,errs_printf20);
	printf(" %" PRIu64 " errors when 19 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl19,errs_dbl19_1bit,errs_printf19);		
	printf(" %" PRIu64 " errors when 18 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl18,errs_dbl18_1bit,errs_printf18);
	printf(" %" PRIu64 " errors when 17 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl17,errs_dbl17_1bit,errs_printf17);
	printf(" %" PRIu64 " errors when 16 sf string converted back to a double (%" PRIu64 " are 1 bit) (sprintf gives %" PRIu64  " differences)\n",errs_dbl16,errs_dbl16_1bit,errs_printf16);	
	printf(" Differences in string compares between built in \"libc\" sprintf() and tested sprintf() are:\n");
    errs=0;// just want errors from the summary that are real errors
	for(int i=1;i<=19;++i)
		{if(i<=DBL_DIG) errs+=errsf[i];// differences are allowed at >DBL_DIG
		 printf(" %2d significant figures found %" PRIu64 " differences\n",i,errsf[i]);
		}
#ifdef COUNT_EXP_LOOPS /* defined in float_to_str.h */
	printf(" %" PRIu64 " exponent corrections done in float-to-string()\n",count_exp_loops);
#endif	
    errs+=errs_dbl17+errs_dbl18+errs_dbl19+errs_dbl20+errs_dbl21; // count errors at 17  as we expect zero at >= 17sf

	printf(" Should get 0 round the loop errors for >=%d sig fig, and 0 differences on string compares at <= %d sig figs :%" PRIu64 " errors found\n",DBL_DECIMAL_DIG,DBL_DIG ,errs);

#if defined( __SIZEOF_FLOAT128__)  && defined(YA_SP_SPRINTF_QF) 
	errs+=chk_fast_strtof128() ; // tests for fast_strtof128() if compiler supports __float128 data type
	errs+=chk_f128_to_a(); /* more advanced tests checking ya_sprintf %Qe as well */
	errs+=chk_f128_rounding(); /* check rounding */
	printf(" Float128 should show 0 string differences at <= 33 sig figs and zero round the loop errors for >= 36 sig figs\n"); // FLT128_FLT_DIG=33, FLT128_DECIMAL_DIG=36
#else
	printf("Cannot check Float128's as they are not supported by the compiler\n");
#endif
#if defined(LDBL_MAX_10_EXP) && LDBL_MAX_10_EXP==4932 /* "true" long double */
    errs+=chk_LD_to_a(); /* round loop checks for long doubles */
#else
	printf("Not checking long doubles as they are identical to doubles in this compiler\n");
#endif
    printf("\nAt end of Part 1 after %g secs : %" PRIu64 " tests, %" PRIu64 " errors\n\n",read_HR_Timer(),total_nos_tests,errs);
#endif /* PART1_SPRINTF_TESTS */    
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
	printf(" checking %%c:\n");
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
	printf(" checking %%s:\n");	
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
  printf(" checking %%i:\n");	
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
		 CHECK_STR("%w32d"); // %w32d is 32 bit	- C23
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
  printf(" checking %%ld:\n");		
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
  printf(" checking %%lld:\n");	
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
		 CHECK_STR("%w64d"); // %w64d is 64 bit, C23
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
  printf(" checking %%g:\n");	
  // built in sprintf has issues with powers of 10, eg -10000, -1000, 100, 1000, etc so don't test these!
  // list below includes max, smallest normal number, max subnormal number, min subnormal number,zero, inf, nan ie all the special cases.
  double test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,-1e-307,-1e-308,-1e-315,0,4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,INT_MAX,LONG_MAX,(double)LLONG_MAX,1.7976931348623157e308,HUGE_VAL,-HUGE_VAL,NAN ,-NAN };
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
  printf(" checking %%f:\n");	
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
  printf(" checking %%p:\n");
  for(char *cp="hello world";*cp;cp++)
  	{check_str_p("%p",(void *) cp);
	 check_str_p("%2p",(void *) cp);
	 check_str_p("%10p",(void *) cp);
	 check_str_p("%20p",(void *) cp);
	}
  printf(" Now doing misc tests %%b, %%'d, %%$d, %%_$d, etc:\n"); 	
  // now do various other tests inline
  { int r_ya,r;// return codes
    static char buf_ya[10000],buf[10000];// need to be big as we check sprintf where result is unlimited
    int i;
    unsigned int u;
    int64_t i64;
    double d;
    long double ld;   
#ifdef YA_SP_SPRINTF_QI /* 128 bit variables (int  ) supported by compiler */    
    int128_t i128;
#endif   
#ifdef YA_SP_SPRINTF_QF /* 128 bit variables (float) supported by compiler */    
    f128_t d128;
#endif  
	// check invalid flag combination by hand as sprintf considers this format string as valid
    scnt++;
    buf[0]=0; // make sure buf is a null termi9nated string (for NULL test case)
    d=123.4567;
    r=6;strcpy(buf,"#05.7f"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%0-+0#05.7f",d);// two 0's
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%0-+0#05.7f(%g) gives <%s> length %d should be length 1\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"#05.7f"))
		{++serrs;
    	 dprintf("%%0-+0#05.7f(%g) gives <%s> length %d should give <0> \n",d,buf_ya,r_ya);
    	}
	r=7;strcpy(buf,"   5.7f"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%    5.7f",d); // multiple spaces
    if(r_ya!=7)
		{++serrs;
    	 dprintf("%%    5.7f(%g) gives <%s> length %d should be length 1\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"   5.7f"))
		{++serrs;
    	 dprintf("%%    5.7f(%g) gives <%s> length %d should give <0> \n",d,buf_ya,r_ya);
    	}   				
    // check %b by hand as this is not supported in sprintf()
    // %b(0, 0x0) gives <0> length 1
	// %b(17, 0x11) gives <10001> length 5
	// %b(-9, 0xfffffff7) gives <11111111111111111111111111110111> length 32
    scnt++;
    u=0;
    r=1;strcpy(buf,"0"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%b",u);
    if(r_ya!=1) 
		{++serrs;
    	 dprintf("%%b(%d, 0x%x) gives <%s> length %d should be length 1\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0"))
		{++serrs;
    	 dprintf("%%b(%d, 0x%x) gives <%s> length %d should give <0> \n",u,u,buf_ya,r_ya);
    	}    	
    scnt++;
    u=17;
    r=5;strcpy(buf,"10001"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%b",u);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%b(%d, 0x%x) gives <%s> length %d should be length 5\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10001"))
		{++serrs;
    	 dprintf("%%b(%d, 0x%x) gives <%s> length %d should give <10001> \n",u,u,buf_ya,r_ya);
    	}       
    scnt++;
    u=0xeffffff7;
    r=32;strcpy(buf,"11101111111111111111111111110111"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%b",u);
    if(r_ya!=32) 
		{++serrs;
    	 dprintf("%%b(%d, 0x%x) gives <%s> length %d should be length 32\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"11101111111111111111111111110111"))
		{++serrs;
    	 dprintf("%%b(%d, 0x%x) gives <%s> length %d should give <11101111111111111111111111110111> \n",u,u,buf_ya,r_ya);
    	}      
	// repeat for %B
    scnt++;
    u=0;
    r=1;strcpy(buf,"0"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%B",u);
    if(r_ya!=1) 
		{++serrs;
    	 dprintf("%%B(%d, 0x%x) gives <%s> length %d should be length 1\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 dprintf("%%B(%d, 0x%x) gives <%s> length %d should give <0> \n",u,u,buf_ya,r_ya);
    	}    	
    scnt++;
    u=17;
    r=5;strcpy(buf,"10001"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%B",u);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%B(%d, 0x%x) gives <%s> length %d should be length 5\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10001")) 
		{++serrs;
    	 dprintf("%%B(%d, 0x%x) gives <%s> length %d should give <10001> \n",u,u,buf_ya,r_ya);
    	}       
    scnt++;
    u=0xeffffff7;
    r=32;strcpy(buf,"11101111111111111111111111110111"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%B",u);
    if(r_ya!=32) 
		{++serrs;
    	 dprintf("%%B(%d, 0x%x) gives <%s> length %d should be length 32\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"11101111111111111111111111110111")) 
		{++serrs;
    	 dprintf("%%B(%d, 0x%x) gives <%s> length %d should give <11101111111111111111111111110111> \n",u,u,buf_ya,r_ya);
    	}  
	/* %#b(0, 0x0) gives <0> length 1
	   %#b(17, 0x11) gives <0b10001> length 7
	   %#b(-9, 0xfffffff7) gives <0b11111111111111111111111111110111> length 34
	*/	
    scnt++;
    u=0;
    r=1;strcpy(buf,"0"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%#b",u);
    if(r_ya!=1) 
		{++serrs;
    	 dprintf("%%#b(%d, 0x%x) gives <%s> length %d should be length 1\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 dprintf("%%#b(%d, 0x%x) gives <%s> length %d should give <0> \n",u,u,buf_ya,r_ya);
    	}    	
    scnt++;
    u=17;
    r=7;strcpy(buf,"0b10001"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%#b",u);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%#b(%d, 0x%x) gives <%s> length %d should be length 7\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0b10001"))
		{++serrs;
    	 dprintf("%%#b(%d, 0x%x) gives <%s> length %d should give <0b10001> \n",u,u,buf_ya,r_ya);
    	}       
    scnt++;
    u=0xeffffff7;
    r=34;strcpy(buf,"0b11101111111111111111111111110111"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%#b",u);
    if(r_ya!=34) 
		{++serrs;
    	 dprintf("%%#b(%d, 0x%x) gives <%s> length %d should be length 34\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0b11101111111111111111111111110111")) 
		{++serrs;
    	 dprintf("%%b(%d, 0x%x) gives <%s> length %d should give <0b11101111111111111111111111110111> \n",u,u,buf_ya,r_ya);
    	}      
	// repeat for %B
    scnt++;
    u=0;
    r=1;strcpy(buf,"0"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%#B",u);
    if(r_ya!=1) 
		{++serrs;
    	 dprintf("%%#B(%d, 0x%x) gives <%s> length %d should be length 1\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 dprintf("%%#B(%d, 0x%x) gives <%s> length %d should give <0> \n",u,u,buf_ya,r_ya);
    	}    	
    scnt++;
    u=17;
    r=7;strcpy(buf,"0B10001"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%#B",u);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%#B(%d, 0x%x) gives <%s> length %d should be length 7\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0B10001")) 
		{++serrs;
    	 dprintf("%%#B(%d, 0x%x) gives <%s> length %d should give <0B10001> \n",u,u,buf_ya,r_ya);
    	}       
    scnt++;
    u=0xeffffff7;
    r=34;strcpy(buf,"0B11101111111111111111111111110111"); // expected results
    r_ya=ya_s_sprintf(buf_ya,"%#B",u);
    if(r_ya!=34) 
		{++serrs;
    	 dprintf("%%#B(%d, 0x%x) gives <%s> length %d should be length 34\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0B11101111111111111111111111110111")) 
		{++serrs;
    	 dprintf("%%#B(%d, 0x%x) gives <%s> length %d should give <0B11101111111111111111111111110111> \n",u,u,buf_ya,r_ya);
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
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should be length 1\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should give <0> \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should give <1,000> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=-1000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should give <-1,000> \n",i,i,buf_ya,r_ya);
    	}     
    scnt++;
    i=10000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should give <10,000> \n",i,i,buf_ya,r_ya);
    	}  
    scnt++;
    i=100000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should give <100,000> \n",i,i,buf_ya,r_ya);
    	}  				    	
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=9) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should be length 9\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should give <2,536,000> \n",i,i,buf_ya,r_ya);
    	}   
    scnt++;
    i=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'d",i);
    if(r_ya!=10) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 dprintf("%%'d(%d, 0x%x) gives <%s> length %d should give <-2,536,000> \n",i,i,buf_ya,r_ya);
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
    	 dprintf("%%'010d(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"00,000,000")) 
		{++serrs;
    	 dprintf("%%'010d(%d, 0x%x) gives <%s> length %d should give <00,000,000> \n",i,i,buf_ya,r_ya);
    	} 
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%'010d",i);
    if(r_ya!=10) 
		{++serrs;
    	 dprintf("%%'010d(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"00,001,000")) 
		{++serrs;
    	 dprintf("%%'010d(%d, 0x%x) gives <%s> length %d should give <00,001,000> \n",i,i,buf_ya,r_ya);
    	} 
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'010d",i);
    if(r_ya!=10) 
		{++serrs;
    	 dprintf("%%'010d(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"02,536,000")) 
		{++serrs;
    	 dprintf("%%'010d(%d, 0x%x) gives <%s> length %d should give <02,536,000> \n",i,i,buf_ya,r_ya);
    	} 
    scnt++;
    i=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'011d",i);
    if(r_ya!=11) 
		{++serrs;
    	 dprintf("%%'011d(%d, 0x%x) gives <%s> length %d should be length 11\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-02,536,000")) 
		{++serrs;
    	 dprintf("%%'011d(%d, 0x%x) gives <%s> length %d should give <-02,536,000> \n",i,i,buf_ya,r_ya);
    	} 					    	
    // check unsigned
    scnt++;
    i=-2536000;
    u=i;
    r_ya=ya_s_sprintf(buf_ya,"%'u",u);
    if(r_ya!=13) 
		{++serrs;
    	 dprintf("%%'u(%u, 0x%x) gives <%s> length %d should be length 13\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"4,292,431,296")) 
		{++serrs;
    	 dprintf("%%'u(%u, 0x%x) gives <%s> length %d should give <4,292,431,296> \n",u,u,buf_ya,r_ya);
    	} 	    
	// Now check 64 bit ints 
	i=0;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=1) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 1\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should give <0> \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should give <1,000> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=-1000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should give <-1,000> \n",i,i,buf_ya,r_ya);
    	}     
    scnt++;
    i=10000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should give <10,000> \n",i,i,buf_ya,r_ya);
    	}  
    scnt++;
    i=100000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should give <100,000> \n",i,i,buf_ya,r_ya);
    	}  				    	
    scnt++;
    i=2536000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=9) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 9\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should give <2,536,000> \n",i,i,buf_ya,r_ya);
    	}   
    scnt++;
    i=-2536000;
	i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%'lld",i64);
    if(r_ya!=10) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 dprintf("%%'lld(%d, 0x%x) gives <%s> length %d should give <-2,536,000> \n",i,i,buf_ya,r_ya);
    	}	
    // check 64 bit unsigned
    scnt++;
    uint64_t u64=i64;
    r_ya=ya_s_sprintf(buf_ya,"%'llu",u64);
    if(r_ya!=26) 
		{++serrs;
    	 dprintf("%%'llu(%llu, 0x%llx) gives <%s> length %d should be length 26\n",(long long unsigned int) u64,(long long unsigned int)u64,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"18,446,744,073,707,015,616")) 
		{++serrs;
    	 dprintf("%%'llu(%llu, 0x%llx) gives <%s> length %d should give <18,446,744,073,707,015,616> \n",(long long unsigned int)u64,(long long unsigned int)u64,buf_ya,r_ya);
    	} 
#ifdef YA_SP_SPRINTF_QI /* 128 bit variables (int ) supported by compiler */			    	
	// Now check 128 bit ints 
	i=0;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=1) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 1\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <0> \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <1,000> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=-1000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <-1,000> \n",i,i,buf_ya,r_ya);
    	}     
    scnt++;
    i=10000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <10,000> \n",i,i,buf_ya,r_ya);
    	}  
    scnt++;
    i=100000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <100,000> \n",i,i,buf_ya,r_ya);
    	}  				    	
    scnt++;
    i=2536000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=9) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 9\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <2,536,000> \n",i,i,buf_ya,r_ya);
    	}   
    scnt++;
    i=-2536000;
	i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%'Qd",i128);
    if(r_ya!=10) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should be length 10\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 dprintf("%%'Qd(%d, 0x%x) gives <%s> length %d should give <-2,536,000> \n",i,i,buf_ya,r_ya);
    	}	
    // check 128 bit unsigned
    scnt++;
    uint128_t u128=i128;
    r_ya=ya_s_sprintf(buf_ya,"%'Qu",u128);
    if(r_ya!=51) 
		{++serrs;
    	 dprintf("%%'Qu(%u, 0x%x) gives <%s> length %d should be length 51\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340,282,366,920,938,463,463,374,607,431,765,675,456")) 
		{++serrs;
    	 dprintf("%%'Qu(%u, 0x%x) gives <%s> length %d should give <340,282,366,920,938,463,463,374,607,431,765,675,456> \n",i,i,buf_ya,r_ya);
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
    	 dprintf("%%'.0f(%g) gives <%s> length %d should give <0> \n",d,buf_ya,r_ya);
    	}    	
    scnt++;
    d=1000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should be length 5\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should give <1,000> \n",d,buf_ya,r_ya);
    	}      
    scnt++;
    d=-1000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should give <-1,000> \n",d,buf_ya,r_ya);
    	}   
    scnt++;
    d=10000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should give <10,000> \n",d,buf_ya,r_ya);
    	}  
    scnt++;
    d=100000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should be length 7\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should give <100,000> \n",d,buf_ya,r_ya);
    	}  				  		 
    scnt++;
    d=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=9) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should be length 9\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should give <2,536,000> \n",d,buf_ya,r_ya);
    	} 	
    scnt++;
    d=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0f",d);
    if(r_ya!=10) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should be length 10\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 dprintf("%%'.0f(%g) gives <%s> length %d should give <-2,536,000> \n",d,buf_ya,r_ya);
    	} 	    	
    scnt++;
    d=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.6f",d);// check we only get comma's before the decimal point
    if(r_ya!=16) 
		{++serrs;
    	 dprintf("%%'.6f(%g) gives <%s> length %d should be length 16\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000.000000")) 
		{++serrs;
    	 dprintf("%%'.6f(%g) gives <%s> length %d should give <2,536,000.000000> \n",d,buf_ya,r_ya);
    	} 
    scnt++;
    d=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.6f",d);// check we only get comma's before the decimal point
    if(r_ya!=17) 
		{++serrs;
    	 dprintf("%%'.6f(%g) gives <%s> length %d should be length 17\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000.000000")) 
		{++serrs;
    	 dprintf("%%'.6f(%g) gives <%s> length %d should give <-2,536,000.000000> \n",d,buf_ya,r_ya);
    	} 	
				
	// repeat for long doubles
	ld=0;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=1) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should be length 1\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should give <0> \n",ld,buf_ya,r_ya);
    	}    	
    scnt++;
    ld=1000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should be length 5\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should give <1,000> \n",ld,buf_ya,r_ya);
    	}      
    scnt++;
    ld=-1000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should be length 6\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should give <-1,000> \n",ld,buf_ya,r_ya);
    	}   
    scnt++;
    ld=10000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should be length 6\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should give <10,000> \n",ld,buf_ya,r_ya);
    	}  
    scnt++;
    ld=100000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should be length 7\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should give <100,000> \n",ld,buf_ya,r_ya);
    	}  				  		 
    scnt++;
    ld=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=9) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should be length 9\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should give <2,536,000> \n",ld,buf_ya,r_ya);
    	} 	
    scnt++;
    ld=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Lf",ld);
    if(r_ya!=10) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should be length 10\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 dprintf("%%'.0Lf(%Lg) gives <%s> length %d should give <-2,536,000> \n",ld,buf_ya,r_ya);
    	} 	    	
    scnt++;
    ld=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.6Lf",ld);// check we only get comma's before the decimal point
    if(r_ya!=16) 
		{++serrs;
    	 dprintf("%%'.6Lf(%Lg) gives <%s> length %d should be length 16\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000.000000")) 
		{++serrs;
    	 dprintf("%%'.6Lf(%Lg) gives <%s> length %d should give <2,536,000.000000> \n",ld,buf_ya,r_ya);
    	} 
    scnt++;
    ld=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%'.6Lf",ld);// check we only get comma's before the decimal point
    if(r_ya!=17) 
		{++serrs;
    	 dprintf("%%'.6Lf(%Lg) gives <%s> length %d should be length 17\n",ld,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000.000000")) 
		{++serrs;
    	 dprintf("%%'.6Lf(%Lg) gives <%s> length %d should give <-2,536,000.000000> \n",ld,buf_ya,r_ya);
    	} 	
   	
#ifdef YA_SP_SPRINTF_QF /* 128 bit variables (float) supported by compiler */				
	// repeat for float128
	d=0;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=1) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should be length 1\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should give <0> \n",d,buf_ya,r_ya);
    	}    	
    scnt++;
    d=1000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should be length 5\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1,000")) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should give <1,000> \n",d,buf_ya,r_ya);
    	}      
    scnt++;
    d=-1000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-1,000")) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should give <-1,000> \n",d,buf_ya,r_ya);
    	}   
    scnt++;
    d=10000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"10,000")) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should give <10,000> \n",d,buf_ya,r_ya);
    	}  
    scnt++;
    d=100000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should be length 7\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"100,000")) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should give <100,000> \n",d,buf_ya,r_ya);
    	}  				  		 
    scnt++;
    d=2536000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=9) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should be length 9\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000")) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should give <2,536,000> \n",d,buf_ya,r_ya);
    	} 	
    scnt++;
    d=-2536000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.0Qf",d128);
    if(r_ya!=10) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should be length 10\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000")) 
		{++serrs;
    	 dprintf("%%'.0Qf(%g) gives <%s> length %d should give <-2,536,000> \n",d,buf_ya,r_ya);
    	} 	    	
    scnt++;
    d=2536000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.6Qf",d128);// check we only get comma's before the decimal point
    if(r_ya!=16) 
		{++serrs;
    	 dprintf("%%'.6Qf(%g) gives <%s> length %d should be length 16\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2,536,000.000000")) 
		{++serrs;
    	 dprintf("%%'.6Qf(%g) gives <%s> length %d should give <2,536,000.000000> \n",d,buf_ya,r_ya);
    	} 
    scnt++;
    d=-2536000;
	d128=d;
    r_ya=ya_s_sprintf(buf_ya,"%'.6Qf",d128);// check we only get comma's before the decimal point
    if(r_ya!=17) 
		{++serrs;
    	 dprintf("%%'.6Qf(%g) gives <%s> length %d should be length 17\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2,536,000.000000")) 
		{++serrs;
    	 dprintf("%%'.6Qf(%g) gives <%s> length %d should give <-2,536,000.000000> \n",d,buf_ya,r_ya);
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
    	 dprintf("%%$d(%d, 0x%x) gives <%s> length %d should be length 2\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0 ")) 
		{++serrs;
    	 dprintf("%%$d(%d, 0x%x) gives <%s> length %d should give <0 > \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%$d",i);
    if(r_ya!=3) 
		{++serrs;
    	 dprintf("%%$d(%d, 0x%x) gives <%s> length %d should be length 3\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1 k")) 
		{++serrs;
    	 dprintf("%%$d(%d, 0x%x) gives <%s> length %d should give <1 k> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%$.3d",i);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$.3d(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 dprintf("%%$.3d(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}   
    scnt++;
    i=-2536000;
    r_ya=ya_s_sprintf(buf_ya,"%$.3d",i);
    if(r_ya!=8) 
		{++serrs;
    	 dprintf("%%$.3d(%d, 0x%x) gives <%s> length %d should be length 8\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"-2.536 M")) 
		{++serrs;
    	 dprintf("%%$.3d(%d, 0x%x) gives <%s> length %d should give <-2.536 M> \n",i,i,buf_ya,r_ya);
    	}   
    // check unsigned 32 bit	
    scnt++;
    i=-2536000;
    u=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3u",u);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$.3u(%u, 0x%x) gives <%s> length %d should be length 7\n",u,u,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"4.292 G")) 
		{++serrs;
    	 dprintf("%%$.3u(%u, 0x%x) gives <%s> length %d should give <4.292 G> \n",u,u,buf_ya,r_ya);
    	}   		    	
	// now check i64
    scnt++;
    i=2536000;
    i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3lld",i64);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$.3lld(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 dprintf("%%$.3lld(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}   
    // now check unsigned i64
    scnt++;
    i= -2536000;
    u64=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3llu",u64);
    if(r_ya!=8) 
		{++serrs;
    	 dprintf("%%$.3llu(%llu, 0x%llx) gives <%s> length %d should be length 8\n",(long long unsigned int)u64,(long long unsigned int)u64,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"18.447 E")) 
		{++serrs;
    	 dprintf("%%$.3llu(%llu, 0x%llx) gives <%s> length %d should give <18.447 E> \n",(long long unsigned int)u64,(long long unsigned int)u64,buf_ya,r_ya);
    	}     
#ifdef YA_SP_SPRINTF_QI /* 128 bit variables (int ) supported by compiler */    	
	// now check i128
    scnt++;
    i=2536000;
    i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3Qd",i128);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$.3Qd(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 dprintf("%%$.3Qd(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}  
    // now check unsigned 128 bits
    scnt++;
    i=-2536000;// 340282366920938 463 463374607431765675456 
    		   // 340282366920938.463  (next digit is 4)
               // note if true long double is not available then it will print as 340282366920938.375 , code below allows for this via #if below
#if defined(LDBL_MAX_10_EXP) && LDBL_MAX_10_EXP==4932 /* "true" long double */
     u128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3Qu",u128);
    r=ya_s_sprintf(buf,"%Qu 0x%Qx",u128,u128);
    if(r_ya!=21)
		{++serrs;
    	 dprintf("%%$.3Qu(%s) gives <%s> length %d should be length 21\n",buf,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340282366920938.463 Y"))
		{++serrs;
    	 dprintf("%%$.3Qu(%s) gives <%s> length %d should give <340282366920938.463 Y> \n",buf,buf_ya,r_ya);
    	}
    r_ya=ya_s_sprintf(buf_ya,"%$.3I128u",u128);
    if(r_ya!=21)
		{++serrs;
    	 dprintf("%%$.3I128u(%s) gives <%s> length %d should be length 21\n",buf,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340282366920938.463 Y"))
		{++serrs;
    	 dprintf("%%$.3I128u(%s) gives <%s> length %d should give <340282366920938.463 Y> \n",buf,buf_ya,r_ya);
    	}
    r_ya=ya_s_sprintf(buf_ya,"%$.3w128u",u128);// %w128 is C23
    if(r_ya!=21)
		{++serrs;
    	 dprintf("%%$.3w128u(%s) gives <%s> length %d should be length 21\n",buf,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340282366920938.463 Y"))
		{++serrs;
    	 dprintf("%%$.3w128u(%s) gives <%s> length %d should give <340282366920938.463 Y> \n",buf,buf_ya,r_ya);
    	}
 #else   /* true long double not available, expected result is slightly different (340282366920938.375) */
    u128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3Qu",u128);
    r=ya_s_sprintf(buf,"%Qu 0x%Qx",u128,u128);
    if(r_ya!=21)
		{++serrs;
    	 dprintf("%%$.3Qu(%s) gives <%s> length %d should be length 21\n",buf,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340282366920938.375 Y"))
		{++serrs;
    	 dprintf("%%$.3Qu(%s) gives <%s> length %d should give <340282366920938.375 Y> \n",buf,buf_ya,r_ya);
    	}
    r_ya=ya_s_sprintf(buf_ya,"%$.3I128u",u128);
    if(r_ya!=21)
		{++serrs;
    	 dprintf("%%$.3I128u(%s) gives <%s> length %d should be length 21\n",buf,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340282366920938.375 Y"))
		{++serrs;
    	 dprintf("%%$.3I128u(%s) gives <%s> length %d should give <340282366920938.375 Y> \n",buf,buf_ya,r_ya);
    	}
    r_ya=ya_s_sprintf(buf_ya,"%$.3w128u",u128);// %w128 is C23
    if(r_ya!=21)
		{++serrs;
    	 dprintf("%%$.3w128u(%s) gives <%s> length %d should be length 21\n",buf,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"340282366920938.375 Y"))
		{++serrs;
    	 dprintf("%%$.3w128u(%s) gives <%s> length %d should give <340282366920938.375 Y> \n",buf,buf_ya,r_ya);
    	}
 #endif
#endif				
	// now check double
    scnt++;
    i=2536000;
    d=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3f",d);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$.3f(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 dprintf("%%$.3f(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}  
 		     	
	// now check long double
    scnt++;
    i=2536000;
    ld=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3Lf",ld);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$.3Lf(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 dprintf("%%$.3Lf(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
    	}  
   	
#ifdef YA_SP_SPRINTF_QF /* 128 bit variables (float) supported by compiler */    	
	// now check float128
    scnt++;
    i=2536000;
    d128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$.3Qf",d128);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$.3Qf(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.536 M")) 
		{++serrs;
    	 dprintf("%%$.3Qf(%d, 0x%x) gives <%s> length %d should give <2.536 M> \n",i,i,buf_ya,r_ya);
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
    	 dprintf("%%$$d(%d, 0x%x) gives <%s> length %d should be length 2\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0 ")) 
		{++serrs;
    	 dprintf("%%$$d(%d, 0x%x) gives <%s> length %d should give <0 > \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%$$d",i);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%$$d(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1000 ")) 
		{++serrs;
    	 dprintf("%%$$d(%d, 0x%x) gives <%s> length %d should give <1000 > \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%$$.3d",i);
    if(r_ya!=8) 
		{++serrs;
    	 dprintf("%%$$.3d(%d, 0x%x) gives <%s> length %d should be length 8\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 Mi")) 
		{++serrs;
    	 dprintf("%%$$.3d(%d, 0x%x) gives <%s> length %d should give <2.419 Mi> \n",i,i,buf_ya,r_ya);
    	}  	
#ifdef YA_SP_SPRINTF_QI /* 128 bit variables (int ) supported by compiler */    	
    // check i128
    scnt++;
    i=2536000;
    i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$$.3Qd",i128);
    if(r_ya!=8) 
		{++serrs;
    	 dprintf("%%$$.3Qd(%d, 0x%x) gives <%s> length %d should be length 8\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 Mi")) 
		{++serrs;
    	 dprintf("%%$$.3Qd(%d, 0x%x) gives <%s> length %d should give <2.419 Mi> \n",i,i,buf_ya,r_ya);
    	}  	
#endif
#ifdef YA_SP_SPRINTF_QF /* 128 bit variables (float ) supported by compiler */      	
    // check float128	
    scnt++;
    i=2536000;
    d128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$$.3Qf",d128);
    if(r_ya!=8) 
		{++serrs;
    	 dprintf("%%$$.3Qf(%d, 0x%x) gives <%s> length %d should be length 8\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 Mi")) 
		{++serrs;
    	 dprintf("%%$$.3Qf(%d, 0x%x) gives <%s> length %d should give <2.419 Mi> \n",i,i,buf_ya,r_ya);
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
    	 dprintf("%%$$$d(%d, 0x%x) gives <%s> length %d should be length 2\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0 ")) 
		{++serrs;
    	 dprintf("%%$$$d(%d, 0x%x) gives <%s> length %d should give <0 > \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%$$$d",i);
    if(r_ya!=5) 
		{++serrs;
    	 dprintf("%%$$$d(%d, 0x%x) gives <%s> length %d should be length 5\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1000 ")) 
		{++serrs;
    	 dprintf("%%$$$d(%d, 0x%x) gives <%s> length %d should give <1000 > \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%$$$.3d",i);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$$$.3d(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 M")) 
		{++serrs;
    	 dprintf("%%$$$.3d(%d, 0x%x) gives <%s> length %d should give <2.419 M> \n",i,i,buf_ya,r_ya);
    	}  	
#ifdef YA_SP_SPRINTF_QI /* 128 bit variables (int ) supported by compiler */    	
	// check i128
    scnt++;
    i=2536000;
    i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$$$.3Qd",i128);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$$$.3d(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 M")) 
		{++serrs;
    	 dprintf("%%$$$.3d(%d, 0x%x) gives <%s> length %d should give <2.419 M> \n",i,i,buf_ya,r_ya);
    	}  	
#endif
#ifdef YA_SP_SPRINTF_QF /* 128 bit variables (float ) supported by compiler */ 				  
 	// check f128
    scnt++;
    i=2536000;
    d128=i;
    r_ya=ya_s_sprintf(buf_ya,"%$$$.3Qf",d128);
    if(r_ya!=7) 
		{++serrs;
    	 dprintf("%%$$$.3Qf(%d, 0x%x) gives <%s> length %d should be length 7\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419 M")) 
		{++serrs;
    	 dprintf("%%$$$.3Qf(%d, 0x%x) gives <%s> length %d should give <2.419 M> \n",i,i,buf_ya,r_ya);
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
    	 dprintf("%%_$$$d(%d, 0x%x) gives <%s> length %d should be length 1\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"0")) 
		{++serrs;
    	 dprintf("%%_$$$d(%d, 0x%x) gives <%s> length %d should give <0> \n",i,i,buf_ya,r_ya);
    	}    	
    scnt++;
    i=1000;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$d",i);
    if(r_ya!=4) 
		{++serrs;
    	 dprintf("%%_$$$d(%d, 0x%x) gives <%s> length %d should be length 4\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"1000")) 
		{++serrs;
    	 dprintf("%%_$$$d(%d, 0x%x) gives <%s> length %d should give <1000> \n",i,i,buf_ya,r_ya);
    	}       
    scnt++;
    i=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3d",i);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%_$$$.3d(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 dprintf("%%_$$$.3d(%d, 0x%x) gives <%s> length %d should give <2.419M> \n",i,i,buf_ya,r_ya);
    	}  	 
	// repeat for i64
    scnt++;
    i=2536000;
    i64=i;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3lld",i64);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%_$$$.3lld(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 dprintf("%%_$$$.3lld(%d, 0x%x) gives <%s> length %d should give <2.419M> \n",i,i,buf_ya,r_ya);
    	}  	
#ifdef YA_SP_SPRINTF_QI /* 128 bit variables (int ) supported by compiler */				 
    // repeat for i128	
    scnt++;
    i=2536000;
    i128=i;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3Qd",i128);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%_$$$.3Qd(%d, 0x%x) gives <%s> length %d should be length 6\n",i,i,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 dprintf("%%_$$$.3Qd(%d, 0x%x) gives <%s> length %d should give <2.419M> \n",i,i,buf_ya,r_ya);
    	}  
#endif					 
	// repeat for double with %f
    scnt++;
    d=2536000;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3f",d);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%_$$$.3f(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 dprintf("%%_$$$.3f(%g) gives <%s> length %d should give <2.419M> \n",d,buf_ya,r_ya);
    	} 	
     	
	// check long double
    scnt++;
    d=2536000;
    ld=d;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3Lf",ld);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%_$$$.3Lf(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 dprintf("%%_$$$.3Lf(%g) gives <%s> length %d should give <2.419M> \n",d,buf_ya,r_ya);
    	} 	
   	
#ifdef YA_SP_SPRINTF_QF /* 128 bit variables (float) supported by compiler */		    	
	// check f128
    scnt++;
    d=2536000;
    d128=d;
    ld=d;
    r_ya=ya_s_sprintf(buf_ya,"%_$$$.3Qf",d128);
    if(r_ya!=6) 
		{++serrs;
    	 dprintf("%%_$$$.3Qf(%g) gives <%s> length %d should be length 6\n",d,buf_ya,r_ya);
    	}
    if(strcmp(buf_ya,"2.419M")) 
		{++serrs;
    	 dprintf("%%_$$$.3Qf(%g) gives <%s> length %d should give <2.419M> \n",d,buf_ya,r_ya);
    	}  
#endif		
  	// check wide character handling
	/* %lc expect a wint_t argument : C standard says: 
     	     If an l length modifier is present, the wint_t argument is converted as if by a call to
			the wcrtomb function with a pointer to storage of at least MB_CUR_MAX bytes, the wint_t
			argument converted to wchar_t, and an initial shift state.
		
		For this to have any impact, setlocale(LC_CTYPE, ""); // or "en_US.UTF-8" 
		or similar must have been called previoulsy.
	*/
	/* %ls expect a *wchar_t argument : C standard says: 
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
		For this to have any impact, setlocale(LC_CTYPE, ""); // or "en_US.UTF-8" 
		or similar must have been called previoulsy.
	*/
	char *fstr;	
#if (defined( __MSVCRT__) && !defined(_UCRT) && defined(_WIN32)) || (defined(__BORLANDC__) && !defined(CP_UTF8)) // only if MSVCRT  is being used (without the UCRT) on Windows
	printf(" **** Warning: Cannot check Wide character handling with MSVCRT as cannot change to a mode that allows uft8\n");
	// https://www.mail-archive.com/mingw-w64-public@lists.sourceforge.net/msg25621.html states only UCRT supports utf-8 (and has other useful info on setlocale() on different windows CRT's).
#else
	printf(" Checking Wide character handling:\n");
 #include <locale.h>	
	char *curr_locale=setlocale(LC_ALL,NULL);  	
	printf("  Initial locale=%s\n",curr_locale);
	setlocale( LC_ALL, "" ); // set locale to value set by operating system (user)
	curr_locale=setlocale(LC_ALL,NULL);  	
	printf("  after setlocale(LC_ALL,NULL), locale=%s\n",curr_locale);	// looking for utf8/UTF8
 #ifdef _WIN32
 	setlocale(LC_ALL, ".UTF8"); // enable windows utf8 mode - see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-170 . This is only supported in  Windows 10 version 1803 (10.0.17134.0) or above (or if statically linked to UCRT).
	curr_locale=setlocale(LC_ALL,NULL);  	
	printf("  after setlocale(LC_ALL, \".UTF8\"), locale=%s\n",curr_locale);	// looking for utf8/UTF8
	if(strstr(curr_locale,"UTF")==NULL && strstr(curr_locale,"utf")==NULL)
		{// failed to move to a utf page - try a "backup" solution
		 setlocale(LC_ALL,"English_United Kingdom.utf8");
		 curr_locale=setlocale(LC_ALL,NULL); 
		 printf("  after setlocale(LC_ALL, \"English_United Kingdom.utf8\"), locale=%s\n",curr_locale);	// looking for utf8/UTF8
		}
	if(strstr(curr_locale,"UTF")==NULL && strstr(curr_locale,"utf")==NULL)
		{printf("**** Warning cannot set uft8 locale - wide character tests below may not work correctly\n");
		}	
	if(IsValidCodePage(CP_UTF8))
	  	{
	     if(SetConsoleOutputCP(CP_UTF8)) // see https://stackoverflow.com/questions/46512441/how-do-i-print-unicode-to-the-output-console-in-c-with-visual-studio   Only seems to work with UCRT
	      	printf("  Windows - console code page CP_UTF8 enabled\n");
	     else 
	     	printf("  *** Windows - console code page CP_UTF8 could NOT be enabled\n");
		  // this still seems to be required, even in Windows 11 and with setlocale(LC_ALL, ".UTF8") before the console will display utf8 correctly
		}  			
 #endif	
	// wint_t euro = L'€';	// E2,AC ??       wchar_t
#ifdef __BORLANDC__
    wchar_t euro = (wchar_t) u'€';  // C++ Builder 12.1 helptext says the version below will work, but it gives a "blank".
#else
     wchar_t euro = L'€';
#endif
	char *u8_euro= (char *)u8"€"; // euro as utf-8
	printf((char *)u8"  Euro symbol = €\n");
	fstr="  Euro symbol(%%lc) \"%lc\" as %%C is \"%C\" and as %%s is \"%s\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,euro,euro,u8_euro);
  	scnt++;
	r=sprintf(buf,fstr,euro,euro,u8_euro);
    r_ya=ya_s_sprintf(buf_ya,fstr,euro,euro,u8_euro);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};
 	
 	// with field width
 	printf("                      \"12345\"\n"); 	
 	fstr=  "  Euro symbol (%%5lc)  \"%5lc\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,euro);
  	scnt++;
	r=sprintf(buf,fstr,euro);
    r_ya=ya_s_sprintf(buf_ya,fstr,euro);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};
 	
 	// with field width, left alligned
 	fstr="  Euro symbol(%%-5lc)  \"%-5lc\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,euro);
  	scnt++;
	r=sprintf(buf,fstr,euro);
    r_ya=ya_s_sprintf(buf_ya,fstr,euro);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}; 	
 	
 	// with precision
	fstr="  Euro symbol(%%.5lc)  \"%.5lc\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,euro);
  	scnt++;
	r=sprintf(buf,fstr,euro);
    r_ya=ya_s_sprintf(buf_ya,fstr,euro);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};
 	
 	// with field width and precision
 	fstr="  Euro symbol(%%5.3lc) \"%5.3lc\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,euro);
  	scnt++;
	r=sprintf(buf,fstr,euro);
    r_ya=ya_s_sprintf(buf_ya,fstr,euro);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}; 	
 	
 	// with field width and precision - %C
 	fstr="  Euro symbol(%%5.3C)  \"%5.3C\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,euro);
  	scnt++;
	r=sprintf(buf,fstr,euro);
    r_ya=ya_s_sprintf(buf_ya,fstr,euro);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};  	
	  	
 	
  	printf((char *)u8"  wide character string test: \"賀正\" (\"happy new year\" in Japanese)\n");// "happy new year" in Japanese
  	printf("                                           \"123456789012345678901234567890\"\n");
  	wchar_t *wtext=L"賀正";
  	char *u8_wtext=(char *)u8"賀正"; // utf8 string
  	
  	// initially print as utf8 string via %s
  	fstr=(char *)u8"   \"賀正\" as a utf8 string (%%s) is         \"%s\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,u8_wtext);
   	scnt++;
	r=sprintf(buf,fstr,u8_wtext);// was __mingw_sprintf
    r_ya=ya_s_sprintf(buf_ya,fstr,u8_wtext);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};  
 	
 	// %ls
  	fstr=(char *)u8"   \"賀正\" as a wide string (%%ls) is        \"%ls\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,wtext);
   	scnt++;
	r=sprintf(buf,fstr,wtext);// was __mingw_sprintf
    r_ya=ya_s_sprintf(buf_ya,fstr,wtext);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};   	
 	
 	// using %S rather than %ls
  	fstr=(char *)u8"   \"賀正\" as a wide string (%%S) is         \"%S\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,wtext);
   	scnt++;
	r=sprintf(buf,fstr,wtext);
    r_ya=ya_s_sprintf(buf_ya,fstr,wtext);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);} 	
	 
	// with field width
  	fstr=(char *)u8"   \"賀正\" as a wide string (%%20ls) is      \"%20ls\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,wtext);
   	scnt++;
	r=sprintf(buf,fstr,wtext);
    r_ya=ya_s_sprintf(buf_ya,fstr,wtext);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}; 
 	
	// with field width, left justified 
  	fstr=(char *)u8"   \"賀正\" as a wide string (%%-20ls) is     \"%-20ls\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,wtext);
   	scnt++;
	r=sprintf(buf,fstr,wtext);
    r_ya=ya_s_sprintf(buf_ya,fstr,wtext);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};  	
	 	
	// with precision
  	fstr=(char *)u8"   \"賀正\" as a wide string (%%.5ls) is      \"%.5ls\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,wtext);
   	scnt++;
	r=sprintf(buf,fstr,wtext);
    r_ya=ya_s_sprintf(buf_ya,fstr,wtext);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}; 
	 	
	// with variable precision  
	for(int pr=0;pr<=7;++pr)
		{
	  	 fstr=(char *)u8"   \"賀正\" as a wide string (%%.*ls)[*=%d] is \"%.*ls\"\n";
	  	 YA_SP_SPRINTF_DECORATE(printf)(fstr,pr,pr,wtext);
	   	 scnt++;
		 r=sprintf(buf,fstr,pr,pr,wtext);
	     r_ya=ya_s_sprintf(buf_ya,fstr,pr,pr,wtext);
	 	 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
	 	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}; 
	 	}
 	
	// with field width & precision
  	fstr=(char *)u8"   \"賀正\" as a wide string (%%20.5ls) is    \"%20.5ls\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,wtext);
   	scnt++;
	r=sprintf(buf,fstr,wtext);
    r_ya=ya_s_sprintf(buf_ya,fstr,wtext);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};  
	 
	// with field width & precision - %S
  	fstr=(char *)u8"   \"賀正\" as a wide string (%%20.5S) is     \"%20.5S\"\n";
  	YA_SP_SPRINTF_DECORATE(printf)(fstr,wtext);
   	scnt++;
	r=sprintf(buf,fstr,wtext);
    r_ya=ya_s_sprintf(buf_ya,fstr,wtext);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};  		 	
 #endif // __MSVCRT__ - for wide chars	
  	
	// check multiple variables printed by comparing to sprintf
	printf(" Now checking printing multiple items:\n");
	scnt++;
	i=123;d=5.67;
	fstr="%5d %3.1g\n";// int then double (with a range of different formats applied)
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 	
 	scnt++;
	fstr="%5d %g\n";
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	 
 	
 	scnt++;
	fstr="%d %g\n"; 	
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	 
 	
 	scnt++;
	fstr="%d %-.2g\n"; 	
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 	
 	scnt++;
	fstr="%-6d %g\n"; 	
	r=sprintf(buf,fstr,i,d);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,d);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
	
	scnt++;
	fstr="%2.1g %d\n"; 	// double then int
	r=sprintf(buf,fstr,d,i);
    r_ya=ya_s_sprintf(buf_ya,fstr,d,i);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 	
 	scnt++;
	fstr="%6d %d\n"; // two ints	
	r=sprintf(buf,fstr,i,i);
    r_ya=ya_s_sprintf(buf_ya,fstr,i,i);
 	if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 	if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 	
	printf(" Now checking variable precision %%*.* :\n"); 	 	 	 	 	
	for(int j=-100;j<=100;++j) // -ve j sets - flag (left justified in field)
		for(int k=-2;k<=16; ++k) // -ve k acts as if precision were omitted, start from -2 in case -1 is used as a default flag in "printf" code. Only 17 digits need be exact, so precision limited to 16
			{			
			 scnt++;
			 fstr="%#*.*g %0*d"; 	// double (%g) then int. Note %#g is required as by default %g does not print trailing zeros which makes the lengths different for sprintf and ya_sprintf
			 r=sprintf(buf,fstr,j,k,d,j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,j+k,i);
 			 if(r!=r_ya){ ++serrs;dprintf ("%s (*=%d,%d,%d): sprintf() returns %d ya_sprintf() returns %d\n",fstr,j,k,j+k,r,r_ya);}
 			 if(k<20 && strcmp(buf,buf_ya)) {++serrs;dprintf("%s (*=%d,%d,%d): sprintf() gives %s ya_sprintf() gives %s\n",fstr,j,k,j+k,buf,buf_ya);};	// %g accurate to 19 sf (18 digits after dp 			 
			 scnt++;
			 fstr="%*.*f %0*d"; 	// double (%f) then int
			 r=sprintf(buf,fstr,j,k,d,j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,j+k,i);
 			 if(r!=r_ya){ ++serrs;dprintf ("%s (*=%d,%d,%d): sprintf() returns %d ya_sprintf() returns %d\n",fstr,j,k,j+k,r,r_ya);}   			 
 			 if(k<19 && strcmp(buf,buf_ya)) {++serrs;dprintf("%s (*=%d,%d,%d): sprintf() gives %s ya_sprintf() gives %s\n",fstr,j,k,j+k,buf,buf_ya);};	 			 
			 scnt++;
			 fstr="%*.*e %0*d"; 	// double (%e) then int
			 r=sprintf(buf,fstr,j,k,d,j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,j+k,i);
 			 if(r!=r_ya){ ++serrs;dprintf ("%s (*=%d,%d,%d): sprintf() returns %d ya_sprintf() returns %d\n",fstr,j,k,j+k,r,r_ya);} 			 
 			 if(k<19 && strcmp(buf,buf_ya)) {++serrs;dprintf("%s (*=%d,%d,%d): sprintf() gives %s ya_sprintf() gives %s\n",fstr,j,k,j+k,buf,buf_ya);};			  			  			 
			}
	
	printf(" Now checking %%n:\n");
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
 			 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;dprintf ("%s: sprintf(%%n) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);}
 			 scnt++;
			 fstr="%-*.*g %hhn %0*d\n"; 	// double then int with %hhn which expects char *
			 un.ull=INT64_C(-1); // -1 has all bits set so we can easily see if its been overwritten by writing to another variable in the union 
			 ya_un.ull=INT64_C(-1);	 
			 r=sprintf(buf,fstr,j,k,d,&(un.uc),j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&(ya_un.uc),j+k,i);
    		 // printf("%%hhn: un.uc=%hhx un.ull=%llx ya_un.uc=%hhx ya_un.ull=%llx\n",un.uc,un.ull,ya_un.uc,ya_un.ull);
 			 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;dprintf ("%s: sprintf(%%hhn) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);} 	
 			 scnt++;
			 fstr="%-*.*g %hn %0*d\n"; 	// double then int with %hn which expects short *
			 un.ull=INT64_C(-1); // -1 has all bits set so we can easily see if its been overwritten by writing to another variable in the union 
			 ya_un.ull=INT64_C(-1);	 
			 r=sprintf(buf,fstr,j,k,d,&(un.u_s),j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&(ya_un.u_s),j+k,i);
 			 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;dprintf ("%s: sprintf(%%hn) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);} 				  		 
 			 scnt++;
			 fstr="%-*.*g %ln %0*d\n"; 	// double then int with %ln which expects long *
			 un.ull=INT64_C(-1); // -1 has all bits set so we can easily see if its been overwritten by writing to another variable in the union 
			 ya_un.ull=INT64_C(-1);	 
			 r=sprintf(buf,fstr,j,k,d,&(un.ul),j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&(ya_un.ul),j+k,i);
 			 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;dprintf ("%s: sprintf(%%ln) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);} 	 			 
 			 scnt++;
			 fstr="%-*.*g %lln %0*d\n"; 	// double then int with %hhn which expects long long *
			 un.ull=INT64_C(-1); // -1 has all bits set so we can easily see if its been overwritten by writing to another variable in the union 
			 ya_un.ull=INT64_C(-1);	 
			 r=sprintf(buf,fstr,j,k,d,&(un.ull),j+k,i);
    		 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&(ya_un.ull),j+k,i);
 			 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
 			 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
 			 if(un.ull!=ya_un.ull){ ++serrs;dprintf ("%s: sprintf(%%lln) returns %llx ya_sprintf() returns %llx\n",fstr,un.ull,ya_un.ull);} 	 
#ifdef  YA_SP_SPRINTF_QI /* 128 bit variables (int ) supported by compiler */
			 __int128 i128=-1;// set to a known wrong value to check its changed below.
			 fstr="%-*.*g %Qn %0*d\n"; 	// double then int with %Qn which expects int128 *
			 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&i128,j+k,i); // sprintf does not support %Q but values produced should otherwise be the same as for long long int above
			 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
			 if(un.ull!=i128){ ++serrs;dprintf ("%s: sprintf(%%lln) returns %llx ya_sprintf(%%Qn) returns %llx\n",fstr,un.ull,(long long int)i128);} 	
			 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};	
			 fstr="%-*.*g %I128n %0*d\n"; 	// double then int with %I128n which expects int128 *
			 i128=-1; // set to a known wrong value to check its changed below.
			 r_ya=ya_s_sprintf(buf_ya,fstr,j,k,d,&i128,j+k,i);
			 if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
			 if(un.ull!=i128){ ++serrs;dprintf ("%s: sprintf(%%lln) returns %llx ya_sprintf(%%I128n) returns %llx\n",fstr,un.ull,(long long int)i128);} 	
			 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);};				 
#endif			  			 
			}
	 printf(" Checking round loop accuracy of %%a and %%A with fast_strtod():\n");
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
	  if(*endp) {++serrs;dprintf("%%a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(d!=dr) {++serrs;dprintf("%%a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	  scnt++;
	  ya_s_sprintf(buf,"%a",f); // need to do a special conversion for floats as may give a slightly different value
	  fr=fast_strtof(buf,&endp);
	  if(*endp) {++serrs;dprintf("%%a: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  if(f!=fr) {++serrs;dprintf("%%a: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}		
	  scnt++;
	  ya_s_sprintf(buf,"%A",d);
	  dr=fast_strtod(buf,&endp);
	  if(*endp) {++serrs;dprintf("%%A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(d!=dr) {++serrs;dprintf("%%A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	   
	  scnt++;
	  ya_s_sprintf(buf,"%A",f); 
	  fr=fast_strtof(buf,&endp);
	  if(*endp) {++serrs;dprintf("%%A: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  if(f!=fr) {++serrs;dprintf("%%A: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}			     
      // check %#a/A
	  scnt++;
	  ya_s_sprintf(buf,"%#a",d);
	  dr=fast_strtod(buf,&endp);
	  if(*endp) {++serrs;dprintf("%%#a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(d!=dr) {++serrs;dprintf("%%#a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	  scnt++;
	  ya_s_sprintf(buf,"%#a",f); 
	  fr=fast_strtof(buf,&endp);
	  if(*endp) {++serrs;dprintf("%%#a: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  if(f!=fr) {++serrs;dprintf("%%#a: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}		  
	  scnt++;
	  ya_s_sprintf(buf,"%#A",d);
	  dr=fast_strtod(buf,&endp);
	  if(*endp) {++serrs;dprintf("%%#A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  if(d!=dr) {++serrs;dprintf("%%#A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}
	  scnt++;
	  ya_s_sprintf(buf,"%#A",f); 
	  fr=fast_strtof(buf,&endp);
	  if(*endp) {++serrs;dprintf("%%#A: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  if(f!=fr) {++serrs;dprintf("%%#A: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}		  	   
	  	 
	  for(d=2;d<=1e308;d*=10)
	 	{f=d;
		 scnt++;	
	 	 ya_s_sprintf(buf,"%a",d);
	 	 dr=fast_strtod(buf,&endp);
	 	 if(*endp) {++serrs;dprintf("%%a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	 	 if(d!=dr) {++serrs;dprintf("%%a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%a",f); 
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;dprintf("%%a: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  	 if(f!=fr) {++serrs;dprintf("%%a: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}	 
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%A",d);
	 	 dr=fast_strtod(buf,&endp);
	 	 if(*endp) {++serrs;dprintf("%%A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	 	 if(d!=dr) {++serrs;dprintf("%%A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%A",f); 
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;dprintf("%%A: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  	 if(f!=fr) {++serrs;dprintf("%%A: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}			  	 
		  
      	 // check %#a/A
	  	 scnt++;
	  	 ya_s_sprintf(buf,"%#a",d);
	  	 dr=fast_strtod(buf,&endp);
	  	 if(*endp) {++serrs;dprintf("%%#a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(d!=dr) {++serrs;dprintf("%%#a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%#a",f); 
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;dprintf("%%#a: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  	 if(f!=fr) {++serrs;dprintf("%%#a: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}			  	 
	  	 scnt++;
	  	 ya_s_sprintf(buf,"%#A",d);
	  	 dr=fast_strtod(buf,&endp);
	  	 if(*endp) {++serrs;dprintf("%%#A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	  	 if(d!=dr) {++serrs;dprintf("%%#A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%#A",f); 
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;dprintf("%%#A: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  	 if(f!=fr) {++serrs;dprintf("%%#A: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}				   	  	 
	 	}
	  for(d=2;d>=1e-323;d/=10)
	 	{f=d;
		 scnt++;
	 	 ya_s_sprintf(buf,"%a",d);
	 	 dr=fast_strtod(buf,&endp);
	 	 if(*endp) {++serrs;dprintf("%%a: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	 	 if(d!=dr) {++serrs;dprintf("%%a: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%a",f); 
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;dprintf("%%a: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  	 if(f!=fr) {++serrs;dprintf("%%a: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}		 	 
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%A",d);
	 	 dr=fast_strtod(buf,&endp);
	 	 if(*endp) {++serrs;dprintf("%%A: (%g)=%s fast_strtod() returns %g *endp=0x%x (should be 0)\n",d,buf,dr,*endp);}
	 	 if(d!=dr) {++serrs;dprintf("%%A: (%g)=%s fast_strtod() returns %g (wrong!) *endp=0x%x\n",d,buf,dr,*endp);}	
	 	 scnt++;
	 	 ya_s_sprintf(buf,"%A",f); 
	  	 fr=fast_strtof(buf,&endp);
	  	 if(*endp) {++serrs;dprintf("%%A: (%.9g)=%s fast_strtof() returns %.9g *endp=0x%x (should be 0)\n",f,buf,fr,*endp);}
	  	 if(f!=fr) {++serrs;dprintf("%%A: (%.9g)=%s fast_strtof() returns %.9g (wrong!) *endp=0x%x\n",f,buf,fr,*endp);}			  	   	
	 	 // no point checking %#a here as decimal point will be needed for these cases anyway
	 	}
	}
#ifdef YA_SP_SPRINTF_QF /* 128 bit variables (float) supported by compiler */	
	{ printf(" Basic checks for %%L and %%Q suffixes\n");
	 unsigned int start_scnt=scnt,start_serrs=serrs;
	 double d;
	 long double dl;
	 __float128 d128;
	 // 1st check exponent is correct (using long doubles) 
	 for(int e=-16382;e<16384;++e)
	 	{dl=ldexpl(1.5,e); // ie mantissa is 1.f*2^e so 1.5 can be represented exactly
	 	 ++scnt; 		 
		 fstr="%.4Le"; // only .4 as we are not checking accuracy of mantissa, just decimal exponent
		 r=sprintf(buf,fstr,dl);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf(1.5*2^%d) gives %s ya_sprintf() gives %s\n",fstr,e,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf(1.5*2^%d) returns %d ya_sprintf() returns %d\n",fstr,e,r,r_ya);}	
		}
	 __float128 test_valued[]={0,1,2,-1.7976931348623157e308,LLONG_MIN,LONG_MIN,INT_MIN,-12345678,-10001,-9999,-1001,-345,-100,-99,-20,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,-0.5,-0.1,-1e-307,-1e-308,-1e-315,0,
	 	4.9406564584124654e-324,1e-315,2.2250738585072009e-308,2.2250738585072014e-308,0.1,0.5,1,2,3,4,5,6,7,8,9,10,20,99,101,345,999,1001,1002,1002.5,1002.6,10001,100001,100001,1000001,12345678,
		 INT_MAX,LONG_MAX,LLONG_MAX,1.7976931348623157e308,HUGE_VAL,NAN, -NAN, INFINITY,-INFINITY
		-FLT128_MAX,-1e+4932Q,-1e+4930Q, -1.7976931348623157e308Q,-12345678.Q,-10001.Q,-9999.Q,-1001.Q,-345.Q,-100.Q,-99.Q,-20.Q,-10.Q,-9.Q,-8.Q,-7.Q,
		-6.Q,-5.Q,-4.Q,-3.Q,-2.Q,-1.Q,-0.5Q,
		-0.1Q,-1e-307Q,-1e-308Q,-1e-315Q,-FLT128_MIN,-6.47517511943802511092443895822764655e-4956Q,-FLT128_DENORM_MIN ,0.Q,FLT128_DENORM_MIN,6.47517511943802511092443895822764655e-4956Q,
		 6.47517511943802511092443895822764655e-4946Q, 6.47517511943802511092443895822764655e-4936Q,  FLT128_MIN ,4.9406564584124654e-324Q,1e-315Q,2.2250738585072009e-308Q,
		2.2250738585072014e-308Q,0.1Q,0.5Q,1.Q,2.Q,3.Q,4.Q,5.Q,6.Q,7.Q,8.Q,9.Q,10.Q,20.Q,99.Q,100.Q,101.Q,345.Q,999.Q,
		1000.Q,1001.Q,1002.Q,1002.5Q,1002.6Q,10000.Q,10001.Q,100001.Q,100001.Q,1000001.Q,12345678.Q,1.7976931348623157e308Q,1e+4930Q,1e+4932Q,FLT128_MAX
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
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
 		 ++scnt;// now check %I128 as alternative to %Q
		 fstr="%I128g";
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		 ++scnt;// now check %w128 as alternative to %Q - C23
		 fstr="%w128g";
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
 		 		 	
 		 // now check long double
		 ++scnt; 		 
		 fstr="%Lg";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
#ifdef IGNORE_LD_SIGNED_NANS
		 if(isnan(dl) && signbit(dl) ) /* ignore -NAN */ ; else
#endif		 		 
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
#ifndef DO_NOT_STRCMP_SUBNORMALS  /*  sub-normals might cause false errors, as we have already well tested doubles , just skip here rather than try and select tests that will work/fail */
		 ++scnt;		  	
		 fstr="%g";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
#endif		 		
		 /* do %e  */
		 // first check __float128
		 ++scnt;
		 fstr="%Qe";
		 // r=sprintf(buf,fstr,d128);
		 r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(strcmp(buf,buf_ya)) {++serrs;printf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;printf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		 ++scnt;
		 fstr="%I128e";
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error
		 ++scnt;
		 fstr="%w128e";  // C23
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error		 
 		 	
 		 // now check long double
		 ++scnt; 		 
		 fstr="%Le";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 //if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
#ifdef IGNORE_LD_SIGNED_NANS
		 if(isnan(dl) && signbit(dl) ) /* ignore -NAN */ ; else
#endif			 
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
#ifndef DO_NOT_STRCMP_SUBNORMALS  /* sub-normals might cause false errors, as we have already well tested doubles , just skip here rather than try and select tests that will work/fail */		  
		 ++scnt;		  	
		 fstr="%e";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
#endif		 
		 /* do %f note we use strncmp here as %f may generate very long numbers but only expect first QF_CHARS_TO_CHECK digits (characters) to match exactly */
		 // first check __float128
	#define QF_CHARS_TO_CHECK 33 /* for f128 expect 33 digits to match  */
		 ++scnt;
		 fstr="%Qf";
		 // r=sprintf(buf,fstr,d128);
		 r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(strncmp(buf,buf_ya,QF_CHARS_TO_CHECK)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		 ++scnt;
		 fstr="%I128f";
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(strncmp(buf,buf_ya,QF_CHARS_TO_CHECK)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		 ++scnt;
		 fstr="%w128f";// C23
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 if(strncmp(buf,buf_ya,QF_CHARS_TO_CHECK)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test		 
		  		 	
 		 // now check long double
#undef QF_CHARS_TO_CHECK
#define QF_CHARS_TO_CHECK 18 /* for long doubles max we should expect is 18 */ 		 
		 ++scnt; 		 
		 fstr="%Lf";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 //if(!isnan(d) &&(/*(d==0 && dl!=0) ||*/ (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
#ifdef IGNORE_LD_SIGNED_NANS
		 if(isnan(dl) && signbit(dl) ) /* ignore -NAN */ ; else
#endif			 
		 if(strncmp(buf,buf_ya,QF_CHARS_TO_CHECK)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf("%s: sprintf(%Lf) returns %d (%s) ya_sprintf() returns %d (%s)\n",fstr,dl,r,buf,r_ya,buf_ya);}
		  // finally check double [ already checked extensively so should be OK ]
#undef QF_CHARS_TO_CHECK
#define QF_CHARS_TO_CHECK 15 /* for doubles max we should expect is 15 */
#ifndef DO_NOT_STRCMP_SUBNORMALS  /* sub-normals might cause false errors, as we have already well tested doubles , just skip here rather than try and select tests that will work/fail */		  
		 ++scnt;		  	
		 fstr="%f";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strncmp(buf,buf_ya,QF_CHARS_TO_CHECK)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf("%s: sprintf(%f) returns %d (%s) ya_sprintf() returns %d (%s)\n",fstr,d,r,buf,r_ya,buf_ya);}	
#endif		 
		 		 /* do %a  */
#if 1 // quadmath_snprintf(%a) prints 1.xxx whereas ya_s_snprintf(%a) prints 8.xxx 	- now fixed so test enabled	 		 
		 // first check __float128
		 ++scnt;
		 fstr="%.14Qa"; 
		 // r=sprintf(buf,fstr,d128);
		 r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 // if(!isnan(dl) &&((dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		 ++scnt;
		 fstr="%Qa"; // full resolution
		 // r=sprintf(buf,fstr,d128);
		 r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 // if(!isnan(dl) &&((dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test		
		 ++scnt;
		 fstr="%I128a"; // full resolution
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 // if(!isnan(dl) &&((dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test
		 ++scnt;
		 fstr="%w128a"; // %w is C23
		 r_ya=ya_s_sprintf(buf_ya,fstr,d128);
		 // if(!isnan(dl) &&((dl==0 && d128!=0) || (isinf(dl) && !isinfq(d128) ))) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);} // else if so only count 1 error per test		 			  
#endif 		 	
 		 // now check long double
		 ++scnt; 		 
		 fstr="%La";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // no expected errors as %a code for long doubles is in place
#ifdef IGNORE_LD_SIGNED_NANS
		 if(isnan(dl) && signbit(dl) ) /* ignore -NAN */ ; else
#endif			 
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%a";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}			 			    
		}
	  if(serrs-start_serrs==0)
		printf("  %u sprintf tests completed on %%L and %%Q, no unexpected errors found \n",scnt-start_scnt);
      else	
		printf("  %u sprintf tests completed on %%L and %%Q, %u unexpected errors found \n",scnt-start_scnt,serrs-start_serrs);	
	 } 
#else //  can check %L but not %Q
#if  LDBL_MANT_DIG>DBL_MANT_DIG && LDBL_MAX_10_EXP>=DBL_MAX_10_EXP /* i.e. if we have "true" f80 style long doubles */
	{ printf(" Basic checks for %%L  suffix\n");
	 unsigned int start_scnt=scnt,start_serrs=serrs;
	 double d;
	 long double dl;
	 // 1st check exponent is correct (using long doubles) 
	 for(int e=-16382;e<16384;++e)
	 	{dl=ldexpl(1.5,e); // ie mantissa is 1.f*2^e so 1.5 can be represented exactly
	 	 ++scnt; 		 
		 fstr="%.4Le"; // only .4 as we are not checking accuracy of mantissa, just decimal exponent
		 r=sprintf(buf,fstr,dl);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf(1.5*2^%d) gives %s ya_sprintf() gives %s\n",fstr,e,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf(1.5*2^%d) returns %d ya_sprintf() returns %d\n",fstr,e,r,r_ya);}	
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
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%g";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 		
		 /* do %e  */
 		 // now check long double
		 ++scnt; 		 
		 fstr="%Le";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 //if(!isnan(d) &&((d==0 && dl!=0) || (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%e";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 
		 /* do %f note we use strncmp here as %f may generate very long numbers but only expect first few digits to match exactly */
 		 // now check long double
		 ++scnt; 		 
		 fstr="%Lf";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 //if(!isnan(d) &&(/*(d==0 && dl!=0) ||*/ (isinf(d) && !isinf(dl) ) )) {++expected_errs;printf("%s: Expected error: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 if(strncmp(buf,buf_ya,12)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%f";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strncmp(buf,buf_ya,12)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 
		 		 /* do %a  */
 		 	
 		 // now check long double
		 ++scnt; 		 
		 fstr="%La";
		 r=sprintf(buf,fstr,dl);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,dl);
		 // no expected errors as %a code for long doubles is in place
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		  // finally check double [ already checked extensively so should be OK ]
		 ++scnt;		  	
		 fstr="%a";
		 r=sprintf(buf,fstr,d);
		 // r=quadmath_snprintf (buf, sizeof buf,fstr,d128);
		 r_ya=ya_s_sprintf(buf_ya,fstr,d);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}			 			    
		}
	  if(serrs-start_serrs==0)
		printf("  %u sprintf tests completed on %%L, no unexpected errors found \n",scnt-start_scnt);
      else	
		printf("  %u sprintf tests completed on %%L, %u unexpected errors found \n",scnt-start_scnt,serrs-start_serrs);	
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
        		 
#ifdef YA_SP_SPRINTF_QI /* 128 bit variables (int ) supported by compiler */	
	{uint128_t ui128=TYPE_MAX(uint128_t);	 
	 printf(" Testing %%QxXob [128 integers]:\n");
	 // %x
	 scnt++;
	 r=sprintf(buf,"%016"PRIx64"%016"PRIx64,(uint64_t)(ui128>>64),(uint64_t)ui128);
	 fstr="%032Qx";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qx max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	   
	 fstr="%032I128x";// repeat with I128 instead Q
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qx max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	   	
	 fstr="%032w128x";// repeat with w128 instead Q -C23
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qx max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 	    
	 // %X
	 scnt++;
	 r=sprintf(buf,"%016"PRIX64"%016"PRIX64,(uint64_t)(ui128>>64),(uint64_t)ui128);
	 fstr="%032QX";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%QX max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
	// %o
	 scnt++;
	 // more complex than hex as need to split at boundries that can be represented exactly in whole octal digits
	 r=sprintf(buf,"%01o%021"PRIo64"%021"PRIo64,(uint32_t)(ui128>>126),(uint64_t)(ui128>>63)& 0x7fffffffffffffff,((uint64_t)ui128) & 0x7fffffffffffffff );
	 fstr="%043Qo";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qo max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
	// %b
	 scnt++;
	 // sprintf cannot convert to binary so do a simpler check on basis we expect a string with all 1's in it
	 char * bp;
	 fstr="%043Qb";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qb max128 ya_sprintf gives <%s> length %d\n",buf_ya,r_ya);
     bp=strrchr(buf_ya,'1'); // pointer to last 1
	 if(bp[1]!=0) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(128!=r_ya){ ++serrs;dprintf ("%s: ya_sprintf() returns %d should return 128\n",fstr,r_ya);}	 
	 // now check with zero	 
	 ui128=0;
	 scnt++;
	 fstr="%032Qx";
	 r=sprintf(buf,"%016"PRIx64"%016"PRIx64,(uint64_t)(ui128>>64),(uint64_t)ui128);
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qx 0 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}		 
	 	 // %X
	 scnt++;
	 r=sprintf(buf,"%016"PRIX64"%016"PRIX64,(uint64_t)(ui128>>64),(uint64_t)ui128);
	 fstr="%032QX";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%QX max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
	// %o
	 scnt++;
	 // more complex than hex as need to split at boundries that can be represented exactly in whole octal digits
	 r=sprintf(buf,"%01o%021"PRIo64"%021"PRIo64,(uint32_t)(ui128>>126),(uint64_t)(ui128>>63)& 0x7fffffffffffffff,((uint64_t)ui128) & 0x7fffffffffffffff );
	 fstr="%043Qo";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qo max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
	// %b
	 scnt++;
	 // sprintf cannot convert to binary so do a simpler check on basis we expect a string with just 0 in it
	 fstr="%Qb";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qb max128 ya_sprintf gives <%s> length %d\n",buf_ya,r_ya);
	 if(strcmp("0",buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(1!=r_ya){ ++serrs;dprintf ("%s: ya_sprintf() returns %d should return 1\n",fstr,r_ya);}	 
	 
	 
	 for(ui128=1;ui128!=TYPE_MAX(uint128_t);ui128=(ui128<<1)|1)
	 	{// try a range of numbers for x,X,o (cannot do b in general as sprintf does not support it)
	 	 // %x
	 	 scnt++;
	 	 fstr="%032Qx";
	 	 r=sprintf(buf,"%016"PRIx64"%016"PRIx64,(uint64_t)(ui128>>64),(uint64_t)ui128);
     	 r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     	 //printf("%%Qx 111... sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}
	 	 // %X
		 scnt++;
		 r=sprintf(buf,"%016"PRIX64"%016"PRIX64,(uint64_t)(ui128>>64),(uint64_t)ui128);
		 fstr="%032QX";
	     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
	     //printf("%%QX max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 // %o
		 scnt++;
		 // more complex than hex as need to split at boundries that can be represented exactly in whole octal digits
		 r=sprintf(buf,"%01o%021"PRIo64"%021"PRIo64,(uint32_t)(ui128>>126),(uint64_t)(ui128>>63)& 0x7fffffffffffffff,((uint64_t)ui128) & 0x7fffffffffffffff );
		 fstr="%043Qo";
	     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
	     //printf("%%Qo max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 	 
	 	}
	}
	// the builtin sprintf() cannot print int128's so use the two functions below to do this 
	// int sprint_uint128_decimal(char *s,uint128_t big); /* print a uint128 to string which needs to be at least 40 character [including trailing null] */
    // int sprint_int128_decimal(char *s,int128_t big); /* signed sprint, needs min 41 character buffer as potential leading minus sign */
	{uint128_t ui128=TYPE_MAX(uint128_t);	 
	 int128_t i128=TYPE_MAX(int128_t);
	 printf(" Testing %%Quid [128 integers to decimal]:\n");
	 // %u
	 scnt++;
	 r=sprint_uint128_decimal(buf,ui128);
	 fstr="%Qu";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     // printf("%%Qu max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	      
	 // %d
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qd";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
	 fstr="%I128d";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 	
	 fstr="%w128d";// %w is C23
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}		  
	 // %i
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qi";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
	 // now check -max (signed only)
	 i128= ~i128; // 1's complement gives most negative number
	 // %d
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qd";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
	 // %i
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qi";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 	 
	 // now check zero:
	 ui128=0;
	 i128=0;
	 // %u
	 scnt++;
	 r=sprint_uint128_decimal(buf,ui128);
	 fstr="%Qu";
     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
     //printf("%%Qu max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	      
	 // %d
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qd";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
	 // %i
	 scnt++;
	 r=sprint_int128_decimal(buf,i128);
	 fstr="%Qi";
     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
	 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
	 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 	
	 // now try for a reasonable range of numbers	 
	 for(ui128=1;ui128!=TYPE_MAX(uint128_t);ui128=(ui128<<1)|1)
	 	{i128=ui128;
	 	 // %u
		 scnt++;
		 r=sprint_uint128_decimal(buf,ui128);
		 fstr="%Qu";
	     r_ya=ya_s_sprintf(buf_ya,fstr,ui128);    
	     //printf("%%Qu max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	      
		 // %d
		 scnt++;
		 r=sprint_int128_decimal(buf,i128);
		 fstr="%Qd";
	     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
	     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
		 // %i
		 scnt++;
		 r=sprint_int128_decimal(buf,i128);
		 fstr="%Qi";
	     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
	     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	
		 // check negative numbers
		 i128= -(int128_t)ui128;
		 // %d
		 scnt++;
		 r=sprint_int128_decimal(buf,i128);
		 fstr="%Qd";
	     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
	     //printf("%%Qd max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}	 
		 // %i
		 scnt++;
		 r=sprint_int128_decimal(buf,i128);
		 fstr="%Qi";
	     r_ya=ya_s_sprintf(buf_ya,fstr,i128);    
	     //printf("%%Qi max128 sprintf gives <%s> length %d ya_sprintf gives <%s> length %d\n",buf,r,buf_ya,r_ya);
		 if(strcmp(buf,buf_ya)) {++serrs;dprintf("%s: sprintf() gives %s ya_sprintf() gives %s\n",fstr,buf,buf_ya);}
		 else if(r!=r_ya){ ++serrs;dprintf ("%s: sprintf() returns %d ya_sprintf() returns %d\n",fstr,r,r_ya);}			 
	 	}
	}	
#else
	printf("Cannot test INT128 variables as they are not supported by the compiler\n");
#endif	// YA_SP_SPRINTF_QI /* 128 bit variables (int ) supported by compiler */	
	printf(" Now checking ya_printf():\n");
	{
#ifdef YA_SP_SPRINTF_QI /* 128 bit variables (int) supported by compiler */	
	 int128_t i1=-1;
	 uint128_t u1=i1;
#endif
#ifdef YA_SP_SPRINTF_QF /* 128 bit variables (float) supported by compiler */	
	 __float128 f1=-1;
#ifdef YA_SP_SPRINTF_QI
	 f1=u1;
#endif	 
#endif	 
	 YA_SP_SPRINTF_DECORATE(printf) ("  This is produced by ya_printf!\n");
	 YA_SP_SPRINTF_DECORATE(printf)("  %s PI~=%g -1 in hex is 0x%x\n","Test string",3.1415926,-1);
#if defined( YA_SP_SPRINTF_QI) && defined( YA_SP_SPRINTF_QF) /* 128 bit variables (int  & float) supported by compiler */		 
	 YA_SP_SPRINTF_DECORATE(printf)("  %I128d (=-1) %I128u(= -1 as signed128) %I128g (same as float128)\n",i1,u1,f1);
#elif defined( YA_SP_SPRINTF_QF) /* only float128 */
	 YA_SP_SPRINTF_DECORATE(printf)("   %I128g (-1 as float128)\n",f1);
#elif defined( YA_SP_SPRINTF_QI) /* only int128 */
	 YA_SP_SPRINTF_DECORATE(printf)("  %I128d (=-1) %I128u(= -1 as signed128)\n",i1,u1);
#endif	 
	 YA_SP_SPRINTF_DECORATE(printf)( "  %01000d (998 zeros then 99)\n",99); // "local" buffer inside printf is only 512 chars so this checks that code works correctly if more than this output in a single call to printf()
	 YA_SP_SPRINTF_DECORATE(fprintf)( stderr,"  %01000d (998 zeros then 99 to stderr)\n",99); 
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
  printf("All tests completed in %g secs\n",time_taken);  
 #else  
  time_taken=clock();
  printf("All tests completed in %g secs\n",(double)time_taken/(double)CLOCKS_PER_SEC);
#endif  
  if(serrs==0)
		printf("PART2: %u sprintf tests completed, no errors found\n",scnt);
  else	
		printf("PART2: %u sprintf tests completed, %u errors found \n",scnt,serrs);
#endif 	
  total_nos_tests+=scnt;
  printf("Test program finished - a total of %" PRIu64  " tests executed\n",total_nos_tests);  	
#ifdef MONITOR_POWER10_ACCURACY /* enable to monitor power10 extraction for doubles in %f format - defined in ya_sprintf.h */  
  extern uint64_t nos_10_correct,nos_10_wrong;  
  printf("double %%f conversion: power 10 estimates correct %zu times (%g%%), wrong %zu times(%g%%)\n",nos_10_correct,100.0*(double)nos_10_correct/(double)(nos_10_correct+nos_10_wrong),nos_10_wrong,100.0*(double)nos_10_wrong/(double)(nos_10_correct+nos_10_wrong));
#endif
#ifdef __BORLANDC__
  printf("Press return to exit:\n");
  getchar();
#endif
  return 0;
}
