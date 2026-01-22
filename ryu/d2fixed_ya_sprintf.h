/* This header file written by Peter Miller 5-1-2026
 Copyright 2026 Peter Miller

 The contents of this file may be used under the terms of the Apache License,
 Version 2.0.

    (See accompanying file LICENSE-Apache or copy at
     http://www.apache.org/licenses/LICENSE-2.0)

 Alternatively, the contents of this file may be used under the terms of
 the Boost Software License, Version 1.0.
    (See accompanying file LICENSE-Boost or copy at
     https://www.boost.org/LICENSE_1_0.txt)

 Unless required by applicable law or agreed to in writing, this software
 is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 KIND, either express or implied.
*/
#ifndef _ryu_ya_sprintf_h
#define _ryu_ya_sprintf_h

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif



//d2exp_buffered_n_ya_sprintf converts "double" ieeeMantissa/ieeeExponent  into exponential format x.xxxe+/-xx
// int32_t *decimal_pos is set to the exponent
// result has just numbers (no decimal point) and no exponent
// exponent is returned in "decimal_pos
// returns length of string stored in result, with precision chars after decimal point (based on x.xxx)
// no checks for special case (nan, inf, etc) provided here as its assumed they are dealt with by ya_sprintf() before calling this function
// sign is also not dealt with here - its assumed the double is positive
//
// 1st 2 params must previously have been calculated as:
//	const uint64_t ieeeMantissa = bits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
//	const uint32_t ieeeExponent = (uint32_t) ((bits >> DOUBLE_MANTISSA_BITS) & ((1u << DOUBLE_EXPONENT_BITS) - 1));
// 
int d2exp_buffered_n_ya_sprintf(const uint64_t ieeeMantissa,const uint32_t ieeeExponent, uint32_t precision, char* result,int32_t *decimal_pos) ;

#ifdef __cplusplus
}
#endif

#endif // _ryu_ya_sprintf_h