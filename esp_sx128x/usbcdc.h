/*
 * USB-CDC support for Arduino ESP32
 * File: "usbcdc.h"
 */

//-----------------------------------------------------------------------------
#pragma once
#ifndef USBCDC_H
#define USBDCD_H
//-----------------------------------------------------------------------------
#include "config.h"
#ifdef ARDUINO_USBCDC // look "config.h"
//-----------------------------------------------------------------------------
#if !ARDUINO_USB_MODE
#include "USB.h"
#if ARDUINO_USB_CDC_ON_BOOT
// Serial0 -> UART, Serial -> USB-CDC
#  define HWSerial  Serial0
#  define USBSerial Serial
#else
// Seria0 -> UART, Serial1 -> USB-CDC
#  warning Recomented set `USB CDC On Boot: "Enabled"` 
#  define HWSerial Serial
extern USBCDC USBSerial;
#  define Serial1 USBSerial
#endif
#else
#warning This sketch should be used when USB is in OTG mode
#endif
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
void usbcdc_begin();
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // ARDUINO_USBCDC
#endif // USBCDC_H
//-----------------------------------------------------------------------------

/*** end of "usbcdc.h" file ***/

