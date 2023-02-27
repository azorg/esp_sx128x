/*
 * Print size of standart C types
 */

//-----------------------------------------------------------------------------
#include <stdint.h> // int32_t, int64_t
#include <time.h>   // time_t
#include "print.h"
#include "print_sizeof.h"
//-----------------------------------------------------------------------------
void print_sizeof()
{
  unsigned foo = 0x5A;
  unsigned char *p = (unsigned char *) &foo;

  if (p[sizeof(foo) - 1] == 0x5A)
    print_str("Big indian");
  else if (p[0] == 0x5A)
    print_str("Little indian");
  else
    print_str("Unknown indian (?!)");
  print_eol();

#ifndef __cplusplus
  //print_ival("sizeof(void)        = ", (int) sizeof(void));
#endif // __cplusplus

  print_ival("sizeof(char)        = ", (int) sizeof(char));
  print_ival("sizeof(short)       = ", (int) sizeof(short));
  print_ival("sizeof(int)         = ", (int) sizeof(int));
  print_ival("sizeof(long)        = ", (int) sizeof(long));
  print_ival("sizeof(long long)   = ", (int) sizeof(long long));
  print_ival("sizeof(int32_t)     = ", (int) sizeof(int32_t));
  print_ival("sizeof(int64_t)     = ", (int) sizeof(int64_t));
  print_ival("sizeof(float)       = ", (int) sizeof(float));
  print_ival("sizeof(float[2])    = ", (int) sizeof(float[2]));
  print_ival("sizeof(double)      = ", (int) sizeof(double));
  print_ival("sizeof(long double) = ", (int) sizeof(long double));
  print_ival("sizeof(int*)        = ", (int) sizeof(int*));
  print_ival("sizeof(void*)       = ", (int) sizeof(void*));
  print_ival("sizeof(time_t)      = ", (int) sizeof(time_t));
  print_ival("sizeof(size_t)      = ", (int) sizeof(size_t));
}
//-----------------------------------------------------------------------------

/*** end of "print_sizeof.c" file ***/

