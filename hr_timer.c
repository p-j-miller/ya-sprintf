/* high resolution timer functions for Windows

Peter Miller 10/7/2016

See https://msdn.microsoft.com/en-us/library/windows/desktop/ms644904%28v=vs.85%29.aspx 
and https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408%28v=vs.85%29.aspx

Note that GetTickCount() returns a DWORD which is the number of ms windows has been running,  but this has a resolution of typically 5ms
You can also use the windows function call Sleep(DWORD ms) to pause for at least ms 

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
 
#include "hr_timer.h"


static LARGE_INTEGER time_of_last_reset;
void init_HR_Timer( void) 
{ // initialise the high resolution timer
  // zero the timer by reading (and storing via argument) curent time
 QueryPerformanceCounter(&time_of_last_reset) ;
}

double read_HR_Timer( void) 
{  // read the timer , returns the time (secs) between last reset and now to ~ uS resolution
 LARGE_INTEGER now;
 LARGE_INTEGER deltaTime;
 static double mult_to_secs=0; // this is initialised on the 1st call - see below. Is mult as faster than a division
 if( mult_to_secs==0)
 	{ // need to intialise frequency (or rather 1/freq)
 	 LARGE_INTEGER frequency;
 	 QueryPerformanceFrequency( &frequency ) ; // frequency that time is updated at
 	 mult_to_secs=1.0/(double)frequency.QuadPart ;
 	}		 
 QueryPerformanceCounter(&now) ; // current time
 deltaTime.QuadPart=now.QuadPart-time_of_last_reset.QuadPart; // subtract to get time since last reset
 return mult_to_secs*(double)deltaTime.QuadPart;// scale to secs, note this looses some resolution
}

time_us read_HR_Timer_ms( void) 
{  // read the timer , returns the time (ms) between last reset and now
   // value is truncated to get ms.
   // note return value is signed, this is so comparisons will work even if result overflows (will overflow in ~ 25 days)
   // This version uses a fp multiply , which is relatively expensive in processor time, but accurate
 LARGE_INTEGER now;
 LARGE_INTEGER deltaTime;
 static double mult_to_msecs=0; // this is initialised on the 1st call - see below. Is mult as faster than a division
 if( mult_to_msecs==0)
 	{ // need to intialise frequency (or rather 1/freq)
 	 LARGE_INTEGER frequency;
 	 QueryPerformanceFrequency( &frequency ) ; // frequency that time is updated at
 	 mult_to_msecs=1000.0/(double)frequency.QuadPart ;
 	}		 
 QueryPerformanceCounter(&now) ; // current time
 deltaTime.QuadPart=now.QuadPart-time_of_last_reset.QuadPart; // subtract to get time since last reset
 return (int64_t)(mult_to_msecs*(double)deltaTime.QuadPart);// scale to msecs, note this looses some resolution, cast via LARGE_INTEGER ensures result truncated
}

time_us read_HR_Timer_us( void) 
{  // read the timer , returns the time (us) between last reset and now
   // value is truncated to get ms.
   // note return value is signed, this is so comparisons will work even if result overflows (will overflow in ~ 35mins)
   // This version uses a fp multiply , which is relatively expensive in processor time, but accurate
 LARGE_INTEGER now;
 LARGE_INTEGER deltaTime;
 static double mult_to_usecs=0; // this is initialised on the 1st call - see below. Is mult as faster than a division
 if( mult_to_usecs==0)
 	{ // need to intialise frequency (or rather 1/freq)
 	 LARGE_INTEGER frequency;
 	 QueryPerformanceFrequency( &frequency ) ; // frequency that time is updated at
 	 mult_to_usecs=1e6/(double)frequency.QuadPart ;
 	}		 
 QueryPerformanceCounter(&now) ; // current time
 deltaTime.QuadPart=now.QuadPart-time_of_last_reset.QuadPart; // subtract to get time since last reset
 return (int64_t)(mult_to_usecs*(double)deltaTime.QuadPart);// scale to usecs, note this looses some resolution, cast via LARGE_INTEGER ensures result truncated
}

int32_t  diff_time(time_us a, time_us b) // returns a-b valid if a, b within 2^31 of each other
	// warning overflow of signed integers is not defined by C standard, but overflow of unsigned is.
	// however this function must return a signed type (here int32_t rather than time_us which may be unsigned).
{ 
#if 1
 /* this version meets the C standard and works with time_us an unsigned */
 time_us r;
 r=a+(~b+1); // a-b , 2's complement = 1's complement +1
 if(r&0x80000000)
 	{int32_t r1;
 	 r= ~r +1; // 2's complement so positive
 	 r1=r;
 	 return -r1;
 	}
 else
 	{return r; // already positive
	}	
#else
return a-b; /* this probably works if time_us is int32_t */
#endif 
}
