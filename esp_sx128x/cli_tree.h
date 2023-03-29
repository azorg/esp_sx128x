/*
 * File "cli_tree.h"
 */

#pragma once
#ifndef CLI_TREE_H
#define CLI_TREE_H
//-----------------------------------------------------------------------------
#ifdef CLI_HELP
#  define _F(id, parent_id, func, name, args, help) \
   { id, parent_id, func, name, args, help }, // CLI help ON
#else
#  define _F(id, parent_id, func, name, args, help) \
   { id, parent_id, func, name }, // CLI help OFF
#endif

#define _O(id, parent_id, func, name, args, help)
//-----------------------------------------------------------------------------
// all commands and options tree
cli_cmd_t const cli_tree[] = {
  //  ID   Par Callback             Name          Args                 Help
#ifdef CLI_HELP
  _F(  0,  -1, cli_help,            "help",       "",                  "print this help (Ctrl+X)")
#endif
  _F(  1,  -1, cli_version,         "version",    "",                  "print version")
  _F(  2,  -1, cli_clear,           "clear",      "",                  "clear screen (Ctrl+L)")
  _F(  3,  -1, cli_verbose,         "verbose",    " [n]",              "set/get verbose leval 0...3")

  _F( 10,  -1, cli_led,             "led",        " {0|1}",            "1=on/0=off LED")
  _F( 11,  10, cli_led_blink,       "blink",      " [N]",              "blink LED N times")
  _F( 12,  -1, cli_pin,             "pin",        " gpio [0|1]",       "read/write digital pin")

  _F( 20,  -1, cli_help,            "sys",        "",                  "system information")
  _F( 21,  20, cli_sys_info,        "info",       "",                  "print system information")
  _F( 22,  20, cli_sys_time,        "time",       "",                  "print system time and ticks")
  _F( 24,  20, cli_sys_reset,       "reset",      "",                  "full system reset MCU")
  
  _F( 30,  -1, cli_help,            "eeprom",     "",                  "EEPROM commands")
  _F( 31,  30, cli_eeprom_erase,    "erase",      "",                  "erase options region of EEPROM")
  _F( 32,  30, cli_eeprom_write,    "write",      "",                  "write all options to EEPROM (Ctrl+W)")
  _F( 33,  30, cli_eeprom_read,     "read",       "",                  "read all options from EEPROM")
  _F( 34,  30, cli_eeprom_delete,   "delete",     "",                  "delete last options record from EEPROM")
  _F( 35,  30, cli_eeprom_diff,     "diff",       "",                  "find differece between current options and saved in EEPROM")
  _F( 36,  30, cli_eeprom_dump,     "dump",       " [offset size]",    "hex dump region in EEPROM")

  _F( 40,  -1, cli_def,             "def",        "",                  "set to default all options (opt_t)")
  
  _F( 50,  -1, cli_help,            "hw",         "",                  "hardware direct/status commands")
  _F( 51,  50, cli_hw_reset,        "reset",      " [t1 t2]",          "hardware reset SX128x by NRST")
  _F( 52,  50, cli_hw_rxen,         "rxen",       " [0|1]",            "1-set/0-reset RXEN line (on/off LNA)")
  _F( 53,  50, cli_hw_txen,         "txen",       " [0|1]",            "1-set/0-reset TXEN line (on/off PowerAmp)")
  _F( 54,  50, cli_hw_busy,         "busy",       "",                  "get BUSY status")

  _F( 55,  -1, cli_spi,             "spi",        " [b0 b1..]",        "exchange bytes (b0 b1...) by SPI")

  _F( 60,  -1, cli_help,            "radio",      "",                  "SX128x radio module commands")
  _F( 61,  60, cli_radio_status,    "status",     "",                  "get status")
  _F( 62,  61, cli_radio_stat_last, "last",       "",                  "get last status")
  _F( 63,  60, cli_radio_ver,       "ver",        "",                  "get firmware version (16 bits)")
  _F( 64,  60, cli_radio_sleep,     "sleep",      " [config]",         "set Sleep mode (Ctrl+Z)")
  _F( 65,  60, cli_radio_standby,   "standby",    " [config]",         "set Standby mode (Ctrl+D)")
  _F( 66,  60, cli_radio_wakeup,    "wakeup",     " [config]",         "set Standby mode (no BUSY wait)")
  _F( 67,  60, cli_radio_mode,      "mode",       " [mode]",           "set/get mode (packet type)")
  _F( 68,  60, cli_radio_save,      "save",       " ",                 "save context")
  _F( 69,  60, cli_radio_fs,        "fs",         "",                  "set FS mode")
  _F( 70,  60, cli_radio_tx,        "tx",         " [to base]",        "set TX mode")
  _F( 71,  60, cli_radio_rx,        "rx",         " [to base]",        "set RX mode")

#if defined(SX128X_USE_LORA) || defined(SX128X_RANGING)
  _F( 72,  60, cli_radio_cad,       "cad",        "",                  "set LoRa CAD mode")
#endif // SX128X_USE_LORA || SX128X_RANGING

  _F( 73,  60, cli_radio_freq,      "freq",       " [Hz]",             "set/get RF frequency SX [Hz]")
  _F( 74,  60, cli_radio_pwr,       "pwr",        " [dBm us]",         "set/get TX power level [dBm] and rampTime [us]")
  _F( 75,  60, cli_radio_lna,       "lna",        " [1|0]",            "on/off/get LNA boost")
  _F( 76,  60, cli_radio_gain,      "gain",       " [rx_gain]",        "set RX gain (0-AGC, 1-min, 13-max")
  _F( 77,  60, cli_radio_dcdc,      "dcdc",       " [1|0]",            "DC-DC 0=off/1=on")
  _F( 78,  60, cli_radio_auto_fs,   "auto_fs",    " {0|1}",            "set auto FS: 1-enable, 0-disable")
  _F( 79,  60, cli_radio_irq,       "irq",        " [mask]",           "get/clear by MASK IRQ status")
  _F( 80,  60, cli_radio_rxdc,      "rxdc",       " [rx sleep base]",  "set RX duty cycle")
  _F( 81,  60, cli_radio_wave,      "wave",       "",                  "set TX Continuous Wave (RF tone) mode (Ctrl+Y)")
  _F( 82,  60, cli_radio_preamble,  "preamble",   "",                  "set TX Continuous Preamble mode")
  _F( 93,  60, cli_radio_reg,       "reg",        " [addr val]",       "read/write register")
  _F( 94,  60, cli_radio_restore,   "restore",    "",                  "restore all parameters (Ctrl+T)")

#if defined(SX128X_USE_LORA) || defined(SX128X_USE_GFSK)
  _F( 95,  60, cli_radio_lp,        "lp",         " {0|1}",            "set Long Preamble: 1-enable, 0-disable")
#endif // SX128X_USE_LORA || SX128X_USE_GFSK

#if defined(SX128X_USE_LORA) || defined(SX128X_RANGING)
  _F(100,  -1, cli_help,            "lora",       "",                  "set/get LoRa params/results")
  _F(101, 100, cli_lora_mod,        "mod",        " [BW SF CR ]",      "set/get LoRa modulation params")
  _F(102, 100, cli_lora_packet,     "packet",     " [PR CRC INV]",     "set/get LoRa packet pars (Preamble, CRC, invertIQ)")
  _F(103, 100, cli_lora_sw,         "sw",         " [SW]",             "set/get LoRa SyncWord (0x12 or 0x34)")
  _F(104, 100, cli_lora_cad,        "cad",        " [sym_num]",        "set LoRa CAD mode sym_num [1...16]")
  _F(105, 100, cli_lora_fei,        "fei",        "",                  "get LoRa frequency error indicator [Hz]")
  _F(106, 100, cli_lora_rssi,       "rssi",       "",                  "get LoRa instantaneous RSSI")
#endif // SX128X_USE_LORA || SX128X_RANGING

#ifdef SX128X_USE_RANGING
  _F(110,  -1, cli_ranging,         "ranging",    " [role MA SA SM]",  "set/get Ranging [Role, MasterAddr, SlaveAddr, SlaveMode]")
  _F(111, 110, cli_ranging_advanced, "advanced",  " {1|0}",            "get/set Advanced Ranging [0-off, 1-on]")
  _F(112, 110, cli_ranging_calib,   "calib",      " [calibration]",    "set/get Ranging calibration")
  _F(113, 110, cli_ranging_result,  "result",     " [filter]",         "get Ranging result (filter: 0-Raw, 1-Filtered, 2-as-is)")
#endif // SX128X_USE_RANGING

#ifdef SX128X_USE_FLRC
  _F(120,  -1, cli_help,            "flrc",       "",                  "set/get FLRC params/results")
  _F(121, 120, cli_flrc_mod,        "mod",        " [BR CR BT]",       "set/get FLRC modulation params")
  _F(122, 120, cli_flrc_packet,     "packet",     " [PR SW SWM CRC]",  "set/get FLRC packet pars")
  _F(123, 120, cli_flrc_swt,        "swt",        " [0..15]",          "get/set SyncWord Tolerance in FLRC")
  _F(124, 120, cli_flrc_sw,         "sw",         " [SW1 SW2 SW3]",    "get/set SyncWord 1..3 in FLRC")
#endif // SX128X_USE_FLRC

#ifdef SX128X_USE_GFSK
  _F(130,  -1, cli_help,            "gfsk",       "",                  "set/get GFSK params/results")
  _F(131, 130, cli_gfsk_mod,        "mod",        " [BR DSB MI BT]",   "set/get GFSK modulation params")
  _F(132, 130, cli_gfsk_packet,     "packet",     " [PR SWL SWM CRC W]","set/get GFSK packet pars")
  _F(133, 130, cli_gfsk_swt,        "swt",        " [0..15]",          "get/set SyncWord Tolerance in GFSK")
  _F(134, 130, cli_gfsk_sw,         "sw",         " [SW1 SW2 SW3]",    "get/set SyncWord 1..3 in GFSK")
#endif // SX128X_USE_GFSK

#ifdef SX128X_USE_BLE
  _F(140,  -1, cli_ble,             "ble",        " [ST TST CRC W]",   "set/get BLE params")
  _F(141, 140, cli_ble_auto_tx,     "auto_tx",    " [delay]",          "set BLE auto TX delay [us], 0=off")
#endif // SX128X_USE_BLE

  _F(150,  -1, cli_help,            "buffer",     "",                  "TX/RX buffer commands")
  _F(151, 150, cli_buffer_base,     "base",       " TxAd RxAd",        "set TX/RX buffer base addreses")
  _F(152, 150, cli_buffer_read,     "read",       " Ad [Num]",         "read from RX/TX buffer")
  _F(153, 150, cli_buffer_write,    "write",      " Ad [b0 b1..]",     "write data to RX/TX buffer")

  _F(160,  -1, cli_tx_timeout,      "tx_timeout", " [ms]",             "get/set TX timeout [ms]")

  _F(165,  -1, cli_fixed,           "fixed",      " [0|1]",            "get/set fixed/variable size of send packet")
  
  _F(170,  -1, cli_data,            "data",       " [b0 b1..]",        "get/set TX data bytes")
  _F(171, 170, cli_data_fill,       "fill",       " [size value]",     "fill TX data bytes")
  _F(172, 170, cli_data_size,       "size",       " [size]",           "get/set RX/TX data size")
  
  _F(175,  -1, cli_code,            "code",       " [101001]",         "get/set OOK code")

  _F(180,  -1, cli_status,          "status",     "",                  "get packet status")

  _F(190,  -1, cli_send,            "send",       " [to]",             "send packet [timeout] (Strl+S)")
  _F(191,  -1, cli_recv,            "recv",       " [size to]",        "receive packet [timeout] (Strl+V)")
  
  _F(200,  -1, cli_mode,            "mode",       " [0..9]",           "get/set FSM mode (0-CW, 1-OOK, 2-TX, 3-RX, 4-RQ, 5-RP, 6-RM, 7-RS, 8-AR, 9-SG)")
  
  _F(201,  -1, cli_fsm,             "fsm",        " [T dT dC WUT]",    "get/set FSM parameters (T-period[ms], dT-CW[ms], dC-code[ms], WUT-wakeup time[ms])")
  
  _F(202,  -1, cli_sweep,           "sweep",      " [Fmin Fmax S]",    "get/set sweep generator pars (Fmin/Fmax - kHz, S - kHz/sec)")
  
  _F(203,  -1, cli_start,           "start",      "",                  "start FSM loop (Ctrl+S)")
  _F(204,  -1, cli_stop,            "stop",       "",                  "stop FSM loop (Ctrl+C)")

  _F(205,  -1, cli_autostart,       "autostart",  " [1|0 delay]",      "get/set autostart on reboot flag and delay [sec]")

  _F( -1,  -1, NULL,                NULL,         NULL,                NULL)
};
//-----------------------------------------------------------------------------
#undef _F
#undef _O
//-----------------------------------------------------------------------------
#endif // CLI_TREE_H

/*** end of "cli_tree.h" ***/

