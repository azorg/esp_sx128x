/*
 * Simple CRC8 (copy-paste from stackoverflow.com)
 * file "crc8.h"
 */

#pragma once
#ifndef CRC8_H
#define CRC8_H
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdint.h>
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
uint8_t crc8(const uint8_t *data, size_t len);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // CRC8_H

/*** end of "crc8.h" file ***/


