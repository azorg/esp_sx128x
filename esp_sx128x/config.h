/*
 * File: "config.h"
 */

#pragma once
#ifndef CONFIG_H
#define CONFIG_H
//-----------------------------------------------------------------------------
#if defined(ESP32)
#  define ARDUINO_ESP
#  define ARDUINO_ESP32
#  define HOST_NAME "ESP32"
#elif defined(ESP8266)
#  define ARDUINO_ESP
#  define ARDUINO_ESP8266
#  define HOST_NAME "ESP8266"
#else
#  define HOST_NAME "Arduino"
#  define ARDUINO
#endif
//-----------------------------------------------------------------------------
// Use Wemos S2 mini ESP32 board (Lolin S2 mini)
#define WEMOS_S2_MINI
//-----------------------------------------------------------------------------
// user USB-CDC on "Wemos S2 mini ESP32" board etc
#ifdef WEMOS_S2_MINI
#  define ARDUINO_USBCDC
#endif
//-----------------------------------------------------------------------------
#define ABOUT_STRING HOST_NAME " + SX128x test"
#define VERSION_SOFTWARE_STR "0.0.1"  // software version
#define VERSION_HARDWARE_STR "1"      // hardware revision

#define VERBOSE 0  // verbose level by default
//-----------------------------------------------------------------------------
// UART
#define UART_BAUDRATE 115200  // UART baudrate

#ifdef WEMOS_S2_MINI
#  define UART_RXPIN 2
#  define UART_TXPIN 3
#endif
//-----------------------------------------------------------------------------
#define TICKER_MS 10                  // ticker period [мс]
#define TICKER_HZ (1000 / TICKER_MS)  // ticker frequency [Гц]
//-----------------------------------------------------------------------------
#define LED_PIN LED_BUILTIN  // onboard LED pin
#ifdef ARDUINO_ESP8266
#define LED_INVERT true  // LED pin ivert
#else
#define LED_INVERT false  // LED pin ivert
#endif
#define LED_BLINK_ON 30   // LED on time [мс]
#define LED_BLINK_OFF 70  // LED offtime [мс]

#define BUTTON_PIN 0  // onboard BOOT button (GPIO0)
//-----------------------------------------------------------------------------
// SX128x config
#define SX128X_USE_LORA     // use LoRa mode
#define SX128X_USE_RANGING  // use LoRa Ranging mode
#define SX128X_USE_FLRC     // use FLRC mode
#define SX128X_USE_GFSK     // use GFSK mode
#define SX128X_USE_BLE      // use BLE mode

#ifdef WEMOS_S2_MINI
#  define SX128X_BUSY_PIN  8 // GPIO8
#  define SX128X_NRST_PIN  9 // GPIO9
#  define SX128X_NSS_PIN  10 // GPIO10
#  define SX128X_DIO1_PIN 11 // GPIO11
#  define SX128X_SCK_PIN  12 // GPIO12
#  define SX128X_MOSI_PIN 13 // GPIO13
#  define SX128X_MISO_PIN 14 // GPIO14
// Note: TXEN/RXEN don't use with E28-2G4M12S
#  define SX128X_SPI_ALTERNATE_PINS
#  define SX128X_SPI_CLOCK 1000000UL // 1 MHz
#else // ESP32, ESP8266
#  define SX128X_NRST_PIN  4 // GPIO4
#  define SX128X_NSS_PIN   5 // GPIO5 VSPI_SS
#  define SX128X_DIO1_PIN 16 // GPIO16
#  define SX128X_BUSY_PIN 17 // GPIO17
#  define SX128X_SCK_PIN  18 // GPIO18 VSPI_SCLK
#  define SX128X_MISO_PIN 19 // GPIO19 VSPI_MISO
#  define SX128X_MOSI_PIN 23 // GPIO23 VSPI_MOSI
#  define SX128X_TXEN_PIN 32 // GPIO32
#  define SX128X_RXEN_PIN 33 // GPIO33
#  define SX128X_SPI_CLOCK 12000000UL // 12 MHz
//#define SX128X_SPI_ALTERNATE_PINS // only for ESP32, not for ESP8266
//#ifndef SX128X_HW_SPI vspi // or hspi or (&SPI)
#endif
//-----------------------------------------------------------------------------
#define SX128X_USE_EXTRA   // use some extra functions
#define SX128X_USE_BUGFIX  // use bug fix of known limitations
//-----------------------------------------------------------------------------
//#define SX128X_DEBUG       // debug print
//#define SX128X_DEBUG_IRQ   // debug verbose IRQ print
//#define SX128X_DEBUG_EXTRA // extra debug verbose print
//-----------------------------------------------------------------------------
#define TFS_ARDUINO
//#define TFS_DEBUG // TFS debug output
#define TFS_PAGE_SIZE 1024
#define TFS_PAGE_NUM 2
//-----------------------------------------------------------------------------
#define USE_DIO1_INTERRUPT
#define CLI_HELP
#define EXTRA
#define PRINT_SERIAL
//-----------------------------------------------------------------------------
// selected time function
#if 1
#define TIME_FUNC() micros()  // us
#define TIME_FACTOR 1000
#else
#define TIME_FUNC() millis()  // ms
#define TIME_FACTOR 1
#endif
//-----------------------------------------------------------------------------
#define OPT_DATA_SIZE 128  // max saved packet size [bytes]
#define OPT_CODE_SIZE 15   // max saved OOK code size
//-----------------------------------------------------------------------------
#define OPT_AUTOSTART 0        // auto start FSM TX on reboot {0|1}
#define OPT_AUTOSTART_DELAY 3  // auto start delay [sec]
//-----------------------------------------------------------------------------
#define WIFI_TIMEOUT 15 // Wi-Fi connetion timeout [seconds]
#define MQTT_RETRIES 0  // MQTT connetion retries times (0 - no retries)
#define MQTT_CHECK_PERIOD 60 // MQTT check connection period [seconds] 
//-----------------------------------------------------------------------------
#endif // CONFIG_H

/*** end of "config.h" file ***/
