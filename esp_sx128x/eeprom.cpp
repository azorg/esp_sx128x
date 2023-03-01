/*
 * Arduino EEPROM C++ to C wrappers
 * File "eeprom.cpp"
 */

//-----------------------------------------------------------------------------
#include <EEPROM.h>
#include "eeprom.h"
//-----------------------------------------------------------------------------
// begin EEPROM
#if defined(ARDUINO_ESP) || defined(ARDUINO_ESP32) || defined(ARDUINO_ESP8266)
void eeprom_begin(size_t size)
{
  EEPROM.begin(size);
}
#else
void eeprom_begin()
{
  EEPROM.begin();
}
#endif
//-----------------------------------------------------------------------------
#ifdef EEPROM_COMMIT
// commit EEPROM
void eeprom_commit()
{
  EEPROM.commit();
}
#endif
//-----------------------------------------------------------------------------
// read from EEPROM
void eeprom_read(unsigned address, unsigned count, uint8_t *data)
{
  for (unsigned i = 0; i < count; i++)
    data[i] = EEPROM.read(address + i);
}
//-----------------------------------------------------------------------------
// write to EEPROM
void eeprom_write(unsigned address, unsigned count, const uint8_t *data)
{
  for (unsigned i = 0; i < count; i++)
    EEPROM.write(address + i, data[i]);
}
//-----------------------------------------------------------------------------
// verify EEPROM
// (return 1 if OK else 0)
int8_t eeprom_verify(unsigned address, unsigned count, const uint8_t *data)
{
  unsigned i;
  
  for (i = 0; i < count; i++)
    if (EEPROM.read(address + i) != data[i])
      return 0; // fail

  return 1; // ok
}
//-----------------------------------------------------------------------------
// erase/fill+check EEPROM
// (return 1 if EEPROM filled OK else 0)
int8_t eeprom_erase(unsigned address, unsigned count, uint8_t value)
{
  unsigned i;

  // fill EEPROM
  for (i = 0; i < count; i++)
    EEPROM.write(address + i, value);

  // check EEPROM
  for (i = 0; i < count; i++)
    if (EEPROM.read(address + i) != value)
      return 0; // fail

  return 1; // ok
}
//-----------------------------------------------------------------------------

/*** end of "eeprom.cpp" file ***/

