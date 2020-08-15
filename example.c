/* example.c 
   =========

   A very simple example of using ya-sprintf 
*/

/* compile with: 

	gcc example.c ya-sprintf.c -lquadmath -o example 

  -lquadmath is only required for 64bit builds, it must be omitted for 32 bit builds.
  
  This example will work without "fmaq.c" - run the main test program to see if this is necessary at all.
*/

#include <stdio.h>
#include <stdlib.h>

/* Tell compiler to use ya-sprintf - this should come after #including stdio.h and stdlib.h */
#define YA_SP_SPRINTF_DEFAULT // only necessary if you wish to replace the "built-in" versions of printf, sprintf, fprintf, etc
#include "ya_sprintf.h"  /* if you comment this line out the "built-in" printf will be used */ 
  
int main(int argc, char *argv[]) 
{
	unsigned int i=12;
	printf("%d is %b in binary, %o in octal, %d in decimal and %#X in hex\n",i,i,i,i,i); // %b to print in binary is an ya-sprintf extension so will not print correctly with the glibc printf
	printf("Above should print with ya-sprintf:\n"
		   "12 is 1100 in binary, 14 in octal, 12 in decimal and 0XC in hex\n");	
	return 0;
}
