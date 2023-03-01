/*
 * File: "config.h"
 */

#pragma once
#ifndef CONFIG_H
#define CONFIG_H
//-----------------------------------------------------------------------------
//#define ARDUINO_ESP // ESP32 or ESP8266
//#define ARDUINO_ESP8266
#define ARDUINO_ESP32
//-----------------------------------------------------------------------------
#define ABOUT_STRING "ESP32 + SX128x test"
#define VERSION_SOFTWARE_STR "0.0.1" // software version
#define VERSION_HARDWARE_STR "1"     // hardware revision

#define VERBOSE 0 // verbose level by default

#define BAUDRATE 115200 // UART baudrate

#define TICKER_MS 10                 // ticker period [мс]
#define TICKER_HZ (1000 / TICKER_MS) // ticker frequency [Гц]

#define LED_PIN LED_BUILTIN // onboard LED pin (GPIO2)
#ifdef ARDUINO_ESP8266
#  define LED_INVERT true // LED pin ivert
#else
#  define LED_INVERT false // LED pin ivert
#endif
#define LED_BLINK_ON  30    // LED on time [мс]
#define LED_BLINK_OFF 70    // LED offtime [мс]

#define BUTTON_PIN 0 // onboard BOOT button (GPIO0)
//-----------------------------------------------------------------------------
// SX128x config
#define SX128X_USE_LORA    // use LoRa mode
#define SX128X_USE_RANGING // use LoRa Ranging mode
#define SX128X_USE_FLRC    // use FLRC mode
#define SX128X_USE_GFSK    // use GFSK mode
#define SX128X_USE_BLE     // use BLE mode

#define SX128X_NRST_PIN 4  // GPIO4
#define SX128X_NSS_PIN  5  // GPIO5 VSPI_SS
#define SX128X_DIO1_PIN 16 // GPIO16 
#define SX128X_BUSY_PIN 17 // GPIO17
#define SX128X_SCK_PIN  18 // GPIO18 VSPI_SCLK
#define SX128X_MISO_PIN 19 // GPIO19 VSPI_MISO
#define SX128X_MOSI_PIN 23 // GPIO23 VSPI_MOSI
#define SX128X_TXEN_PIN 32 // GPIO32
#define SX128X_RXEN_PIN 33 // GPIO33

//#define SX128X_SPI_ALTERNATE_PINS // only for ESP32, not for ESP8266
//#ifndef SX128X_HW_SPI vspi // or hspi or (&SPI)

#define SX128X_SPI_CLOCK 1000000UL // 1 MHz
//-----------------------------------------------------------------------------
#define SX128X_USE_EXTRA   // use some extra functions
#define SX128X_USE_BUGFIX  // use bug fix of known limitations
//-----------------------------------------------------------------------------
#define SX128X_DEBUG       // debug print
#define SX128X_DEBUG_IRQ   // debug verbose IRQ print
#define SX128X_DEBUG_EXTRA // extra debug verbose print
//-----------------------------------------------------------------------------
#define CLI_HELP
#define PRINT_SERIAL
#define DEBUG
//-----------------------------------------------------------------------------
#define TFS_ARDUINO // use Arduino "EEPROM.h"
#define TFS_DEBUG // TFS debug output
//-----------------------------------------------------------------------------
#endif // CONFIG_H

/*** end of "config.h" file ***/

