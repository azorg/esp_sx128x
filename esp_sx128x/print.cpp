/*
 * file "print.cpp"
 */

//-----------------------------------------------------------------------------
#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "print.h"
//-----------------------------------------------------------------------------
// print string
int print_str(const char *msg)
{
#if defined(PRINT_SERIAL)
  int retv = Serial.print(msg);
#else
  int retv = 0;
#endif
  return retv;
}
//-----------------------------------------------------------------------------
// print end of line
int print_eol()
{
  return print_str("\r\n");
}
//-----------------------------------------------------------------------------
// print char
int print_chr(char c)
{
#if defined(PRINT_SERIAL)
  return Serial.print(c);
#endif
}
//-----------------------------------------------------------------------------
// print long integer value
int print_int(long i)
{
  char minus = 0;
  char buf[12];
  char *ptr = buf + sizeof(buf) - 1;

  if (i < 0)
  {
    i = -i;
    minus = 1;
  }

  *ptr = '\0';
  do {
    *--ptr = '0' + (i % 10);
    i /= 10;
  } while (i != 0);

  if (minus)
    *--ptr = '-';

  return print_str(ptr);
}
//-----------------------------------------------------------------------------
// print unsigned long integer
int print_uint(unsigned long i)
{
  char buf[12];
  char *ptr = buf + sizeof(buf) - 1;

  *ptr = '\0';
  do {
    *--ptr = '0' + (i % 10);
    i /= 10;
  } while (i != 0);

  return print_str(ptr);
}
//-----------------------------------------------------------------------------
// print unsigned long integer with '0' on the begin
int print_uint_ex(unsigned long i, char digits)
{
  char buf[12];
  char *ptr = buf + sizeof(buf) - 1;

  if (digits > 10) digits = 10; // 2**32 = 4294967296 

  *ptr = '\0';
  do {
    *--ptr = '0' + (i % 10);
    digits--;
    i /= 10;
  } while (i != 0);

  while (digits--)
    *--ptr = '0';

  return print_str(ptr);
}
//-----------------------------------------------------------------------------
// print binary unsigned long integer value
int print_bin(unsigned long i, char digits)
{
  char buf[33];
  char *ptr = buf + 32;
  if (digits > 32) digits = 32; 

  *ptr = '\0';
  while (digits-- > 0)
  {
    *--ptr = '0' + (i & 1);
    i >>= 1;
  }

  return print_str(ptr);
}
//-----------------------------------------------------------------------------
// hex print unsigned long integer value
int print_hex(unsigned long i, char digits)
{
  char buf[9];
  char *ptr = buf + 8;
  if (digits > 8) digits = 8; 

  *ptr = '\0';
  while (digits-- > 0)
  {
    int c = i & 0xf;
    if (c < 10)
      c += '0';
    else
      c += 'A' - 10;
    *--ptr = c;
    i >>= 4;
  }

  return print_str(ptr);
}
//-----------------------------------------------------------------------------
// print long integer as float in NNN.D format [d = (int) (f * 10.)]
void print_dint(long d)
{
  print_int(d / 10);
  print_chr('.');
  print_int(abs(d % 10));
}
//-----------------------------------------------------------------------------
// print "ident = value\r\n" (long integer value)
void print_ival(const char *ident, long i)
{
  print_str(ident);
  print_int(i);
  print_eol();
}
//-----------------------------------------------------------------------------
// print "ident = NNN.D\r\n" (long integer value as float)
void print_dval(const char *ident, long d)
{
  print_str(ident);
  print_dint(d);
  print_eol();
}
//-----------------------------------------------------------------------------
// print "ident = value\r\n" (unsigned long integer value)
void print_uval(const char *ident, unsigned long i)
{
  print_str(ident);
  print_uint(i);
  print_eol();
}
//-----------------------------------------------------------------------------
// print "ident = value\r\n" (unsigned long bin value)
void print_bval(const char *ident, unsigned long i, char digits)
{
  print_str(ident);
  print_bin(i, digits);
  print_eol();
}
//-----------------------------------------------------------------------------
// print "ident = value\r\n" (unsigned long hex value)
void print_hval(const char *ident, unsigned long i, char digits)
{
  print_str(ident);
  print_hex(i, digits);
  print_eol();
}
//-----------------------------------------------------------------------------
// print "ident = value\r\n" (string value)
void print_sval(const char *ident, const char *str)
{
  print_str(ident);
  print_str(str);
  print_eol();
}
//-----------------------------------------------------------------------------
// flush UART/LPUART or USB-CDC TX buffers
void print_flush()
{
#if defined(PRINT_SERIAL)
  Serial.flush();
#endif
}
//-----------------------------------------------------------------------------

/*** end of "print.cpp" file ***/

