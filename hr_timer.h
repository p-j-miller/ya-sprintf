/* hr_timer.h

  header file for windows high resolution timer functions
  
  Peter Miller 10/7/2016
  See hr_timer.c for details.
  
*/
/*----------------------------------------------------------------------------
 * MIT License:
 *
 * Copyright (c) 2016,2020 Peter Miller
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
 
#ifndef __HR_TIMER
#define __HR_TIMER
#include <windows.h>
#include <stdint.h>
typedef uint32_t time_us ; // all time calcs should use this type, note overflows in ~ 36 mins with in32 and 1 hr 12 mins with uint32, so code should allow for this
					/* warning if this is a signed type then the behaviour is not guaranteed by the C standard
					   the behaviour for unsigned types is defined see eg http://stackoverflow.com/questions/3679047/integer-overflow-in-c-standards-and-compilers
					   the gcc option -fwrapv is probably required to use a signed type with gcc
					   this code is designed to work with unsigned 32 bit (uint32_t)
					  */   
#define ms *1000  /* scaling factors for times to us */
#define us *1
#define sec *1000000

void init_HR_Timer( void) ; // initialise the high resolution timer
double read_HR_Timer( void) ;  // read the timer , returns the time (secs) between last reset and now to ~ uS resolution
time_us read_HR_Timer_ms( void) ; // read the timer, result in ms as an integer
time_us read_HR_Timer_us( void) ; // read the timer, result in us as an integer, overflows in ~ 1.2 hours
int32_t diff_time(time_us a, time_us b); // returns a-b valid if a, b with 2^31 of each other
										 // note returns an int32_t as time_us may be unsigned and the result has to be signed
#endif
