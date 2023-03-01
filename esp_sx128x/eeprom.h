/*
 * Arduino EEPROM C++ to C wrappers
 * File "eeprom.h"
 */

#pragma once
#ifndef EEPROM_H
#define EEPROM_H
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdlib.h>
#include "config.h"
#if defined(ARDUINO_ESP) || defined(ARDUINO_ESP32) || defined(ARDUINO_ESP8266)
#  define EEPROM_SIZE 4096
#  define EEPROM_BEGIN(size) eeprom_begin(size)
#  define EEPROM_COMMIT() eeprom_commit()
#else
#  define EEPROM_SIZE 1024
#  define EEPROM_BEGIN(size) eeprom_begin()
#endif
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
// begin EEPROM
#if defined(ARDUINO_ESP) || defined(ARDUINO_ESP32) || defined(ARDUINO_ESP8266)
void eeprom_begin(size_t size);
#else
void eeprom_begin();
#endif
//-----------------------------------------------------------------------------
#ifdef EEPROM_COMMIT
// commit EEPROM
void eeprom_commit();
#endif
//-----------------------------------------------------------------------------
// read from EEPROM
void eeprom_read(unsigned address, unsigned count, uint8_t *data);
//-----------------------------------------------------------------------------
// write to EEPROM
void eeprom_write(unsigned address, unsigned count, const uint8_t *data);
//-----------------------------------------------------------------------------
// verify EEPROM
// (return 1 if OK else 0)
int8_t eeprom_verify(unsigned address, unsigned count, const uint8_t *data);
//-----------------------------------------------------------------------------
// erase/fill+check EEPROM
// (return 1 if EEPROM filled OK else 0)
int8_t eeprom_erase(unsigned address, unsigned count, uint8_t value);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // EEPROM_H

/*** end of "eeprom.h" file ***/

