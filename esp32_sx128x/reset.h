/*
 * File: "reset.h"
 */
#pragma once
#ifndef RESET_H
#define RESET_H
//-----------------------------------------------------------------------------
#include <Arduino.h>
#include "config.h"
//-----------------------------------------------------------------------------
#define RESET_ADDRESS 0
//-----------------------------------------------------------------------------
#ifndef INLINE
#  define INLINE static inline
#endif
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
// reset MCU
INLINE void Reset()
{
#if defined(ARDUINO_ESP) || defined(ARDUINO_ESP8266) || defined(ARDUINO_ESP32)
  ESP.restart();
#else // Arduino (AVR)
  void(*pReset)() = (void(*)()) RESET_ADDRESS;
  (*pReset)();
#endif
}
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // RESET_H

/*** end of "reset.h" file ***/

