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
  _F( 12,  -1, cli_pin,             "pin",        " gpio [0|1]",        "read/write digital pin")

  _F( 30,  -1, cli_help,            "sys",        "",                  "system information")
  _F( 31,  30, cli_sys_info,        "info",       "",                  "print system information")
  _F( 32,  30, cli_sys_time,        "time",       "",                  "print system time and ticks")
  _F( 34,  30, cli_sys_reset,       "reset",      "",                  "full system reset MCU")
  
  _F( 40,  -1, cli_help,            "eeprom",     "",                  "EEPROM commands")
  _F( 41,  40, cli_eeprom_erase,    "erase",      "",                  "erase options region of EEPROM")
  _F( 42,  40, cli_eeprom_write,    "write",      "",                  "write all options to EEPROM (Ctrl+W)")
  _F( 43,  40, cli_eeprom_read,     "read",       "",                  "read all options from EEPROM")
  _F( 44,  40, cli_eeprom_delete,   "delete",     "",                  "delete last options record from EEPROM")
  _F( 45,  40, cli_eeprom_diff,     "diff",       "",                  "find differece between current options and saved in EEPROM")
  _F( 46,  40, cli_eeprom_dump,     "dump",       " [offset size]",    "hex dump region in EEPROM")

  _F( 50,  -1, cli_def,             "def",        "",                  "set to default all options (opt_t)")

  _F( -1,  -1, NULL,                NULL,         NULL,                NULL)
};
//-----------------------------------------------------------------------------
#undef _F
#undef _O
//-----------------------------------------------------------------------------
#endif // CLI_TREE_H

/*** end of "cli_tree.h" ***/

