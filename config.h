/* config.h

 Needed by quadmath-imp.h for that to compile.
 
Sets max level of optimisation allows by gcc and clang to avoid -Ofast which does non-c99 compliant optimisations.

 Created by Peter Miller 25-5-2020
 */
#ifndef __GCC_OPTIMIZE_AWARE
#define __GCC_OPTIMIZE_AWARE (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__clang__)
  
#if __GCC_OPTIMIZE_AWARE
#pragma GCC push_options
#pragma GCC optimize ("-O3") /* cannot use Ofast */
#endif
#endif