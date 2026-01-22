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
#ifndef _ryu_fast_atof_h
#define _ryu_fast_atof_h

#include <inttypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif



// Following function created by Peter Miller, 1/2026 to convert a sign (bool signedM, true=-ve),
// integer mantissa (uint64_t m10) and an integer (power of 10) exponent (uint32_t e10) to convert to a double
double ryu_conv_mant_exp_to_double(bool signedM,uint64_t m10,int32_t e10) ;

#ifdef __cplusplus
}
#endif

#endif // _ryu_fast_atof_h