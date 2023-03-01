/*
 * Simple CRC8 (copy-paste from stackoverflow.com)
 * file "crc8.c"
 */

//-----------------------------------------------------------------------------
#include "crc8.h"
//-----------------------------------------------------------------------------
uint8_t crc8(const uint8_t *data, size_t len)
{
  uint8_t crc = 0xFF;
  size_t i, j;
  for (i = 0; i < len; i++)
  {
    crc ^= *data++;
    for (j = 0; j < 8; j++)
    {
      if (crc & 0x80)
        crc = (uint8_t)((crc << 1) ^ 0x31);
      else
	crc <<= 1;
    }
  }
  return crc;
}
//-----------------------------------------------------------------------------

/*** end of "crc8.c" file ***/

