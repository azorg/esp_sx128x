/*
 * File: "opt.h"
 */
#pragma once
#ifndef OPT_H
#define OPT_H
//-----------------------------------------------------------------------------
#include "config.h"
#include "sx128x.h"
#include "tfs.h"
#include "afsm.h"
//----------------------------------------------------------------------------
#ifndef OPT_WIFI
#  define OPT_WIFI 33 // 32 chars + '\0' at the end
#endif

#ifndef OPT_MQTT
#  define OPT_MQTT 33 // 32 chars + '\0' at the end
#endif

#ifndef OPT_DATA_SIZE
#  define OPT_DATA_SIZE 64
#endif
#define OPT_DATA_DEFAULT { 0x01, 0x02, 0x03, 0x04 }

#ifndef OPT_CODE_SIZE
#  define OPT_CODE_SIZE 16
#endif
#define OPT_CODE_DEFAULT { '1', '0', '0', '1', '0', '1' }

#ifndef OPT_AUTOSTART
#  define OPT_AUTOSTART 0 // {0|1}
#endif

#ifndef OPT_AUTOSTART_DELAY
#  define OPT_AUTOSTART_DELAY 3 // seconds
#endif
//-----------------------------------------------------------------------------
// all options (saved to / restore from FLASH)
typedef struct {
  uint8_t verbose;              // verbose level (0, 1, 2 or 3)

  char wifi_ssid[OPT_WIFI];     // Wi-Fi SSID
  char wifi_passwd[OPT_WIFI];   // WiFi password

  char mqtt_host[OPT_MQTT];     // MQTT broker
  uint16_t mqtt_port;           // MQTT TCP/SSL port (8883 by default) 
  char mqtt_id[OPT_MQTT];       // MQTT client ID
  char mqtt_user[OPT_MQTT];     // MQTT user name
  char mqtt_key[OPT_MQTT];      // MQTT user key/password

  uint8_t rxen;                 // RXEN state {0|1}
  uint8_t txen;                 // TXEN state {0|1}

  sx128x_pars_t radio;          // radio options of SX128x
  uint32_t tx_timeout;          // TX timeout (0 - disable) [ms]

  uint8_t data[OPT_DATA_SIZE];  // RX/TX packet data
  uint8_t data_size;            // RX/TX packet data size (bytes)

  char code[OPT_CODE_SIZE + 1]; // OOK code (like "100101")
  uint8_t code_size;            // OOK code size (chips) = strlen(code)

  afsm_pars_t fsm;              // FSM options
  
  uint8_t autostart; // auto start FSM TX on reboot {0|1}
  uint32_t delay;    // auto start delay [sec]
} opt_t;
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
void opt_default(opt_t *opt); // set to default all options
void opt_read_from_flash(opt_t *opt, tfs_t *tfs); // restore options from TFS
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // OPT_H

/*** end of "opt.h" file ***/

