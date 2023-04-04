/*
 * File: "cli.c"
 */
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdlib.h>
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "global.h"
#include "print.h"
#include "print_sizeof.h"
#include "eeprom.h"
#include "cli.h"
#include "mrl.h"
#include "limit.h"
#include "reset.h"
#include "sx128x.h"
#include "sx128x_hw_arduino.h"
#include "wifi.h"
#include "mqtt.h"
//-----------------------------------------------------------------------------
#ifdef ARDUINO_USBCDC
#  include "usbcdc.h"
#endif
//-----------------------------------------------------------------------------
#define CLI_COMPL_NUM 50 // max variant of MicroRL complettion
#define CLI_MAX_CMD   10 // maximum number of tokens
//-----------------------------------------------------------------------------
#define CLI_KEYCODE_CTRL_C  3 // Ctrl+C key code
#define CLI_KEYCODE_CTRL_D  4 // Ctrl+D key code
#define CLI_KEYCODE_CTRL_S 19 // Ctrl+S key code
#define CLI_KEYCODE_CTRL_T 20 // Ctrl+T key code
#define CLI_KEYCODE_CTRL_Z 26 // Ctrl+Z key code
#define CLI_KEYCODE_CTRL_V 22 // Ctrl+V key code
#define CLI_KEYCODE_CTRL_O 15 // Ctrl+O key code
#define CLI_KEYCODE_CTRL_Q 17 // Ctrl+Q key code
#define CLI_KEYCODE_CTRL_W 23 // Ctrl+W key code
#define CLI_KEYCODE_CTRL_X 24 // Ctrl+X key code
#define CLI_KEYCODE_CTRL_Y 25 // Ctrl+Y key code
//-----------------------------------------------------------------------------
#ifdef MRL_USE_COMPLETE
// array for MicroRL comletion
static const char *cli_compl_world[CLI_COMPL_NUM + 1];
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
#ifndef MRL_PRINT_ESC_OFF
const char *cli_prompt = "\033[32m$\033[0m "; // color prompt
#else
const char *cli_prompt = "$ "; // no color
#endif
int cli_prompt_len = 2; // strlen("$ ")
//-----------------------------------------------------------------------------
// Arduino wrapper for MicroRL
static void cli_print(const char *str)
{
  Serial.print(str);
}
//-----------------------------------------------------------------------------
// получить код нажатой клавиши
int cli_inkey()
{ // return -1 if no data is available
#ifdef ARDUINO_USBCDC
  uint8_t b;
  int retv = USBSerial.available();
  if (retv >= 1)
    retv = USBSerial.read(&b, 1);
    if (retv == 1) return (int) b;
  return -1;
#else
  return Serial.read();
#endif
}
//-----------------------------------------------------------------------------
// execute callback for microrl library
static void cli_execute_cb(int argc, char * const argv[])
{
  int i, parent_id = -1, arg_shift;
  const cli_cmd_t *found = (cli_cmd_t*) NULL;

  for (i = 0; i < argc; i++)
  {
    const cli_cmd_t *cmd = cli_tree;
    while (cmd->name != NULL)
    {
      if (cmd->parent_id == parent_id && strcmp(cmd->name, argv[i]) == 0)
      { // command/option found
        found     = cmd;
        parent_id = cmd->id;
        arg_shift = i + 1;
        break;
      }
      cmd++;
    } // while
  } // for

  if (found != (cli_cmd_t*) NULL) // command found
    found->fn(argc - arg_shift, argv + arg_shift, found);
  else
  {
    print_str("command ");
    print_str(argv[0]);
    print_str(" not found\r\n");
  }
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_COMPLETE
// completion callback for microrl library
static const char** cli_complete_cb(int argc, char * const argv[])
{
  int i, parent_id = -1, count = 0;

  for (i = 0; i < argc ; i++)
  {
    const cli_cmd_t *cmd = cli_tree;
    while (cmd->name != NULL)
    {
      if (cmd->parent_id == parent_id &&
          strstr(cmd->name, argv[i]) == cmd->name &&
          (parent_id != -1 || i == 0))
      { // substring found => add it to completion set

        if (i == argc - 1 && count < CLI_COMPL_NUM)
          cli_compl_world[count++] = cmd->name;

        if (strcmp(cmd->name, argv[i]) == 0)
        { // command/option full found
          parent_id = cmd->id;
          break;
        }
      }
      cmd++;
    } // while
  } // for

  cli_compl_world[count] = NULL;
  return cli_compl_world;
}
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
#ifdef CLI_HELP
// find command record by ID
static const cli_cmd_t *cli_cmd_by_id(int id)
{
  const cli_cmd_t *o;
  for (o = cli_tree; o->name != NULL; o++)
    if (o->id == id)
      break;
  return o;
}
#endif // CLI_HELP
//-----------------------------------------------------------------------------
// show help based on command/option tree
void cli_help(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // help
#ifdef CLI_HELP
  if (cmd->id == 0)
  { // root (full) help
    const cli_cmd_t *o;

    print_str(ABOUT_STRING "\r\n"
              "Use TAB key for completion\r\n"
              "Command:\r\n");

    for (o = cli_tree; o->name != NULL; o++)
    {
      int i, parents_cnt = 0;
      const cli_cmd_t *parents[CLI_MAX_CMD];
      const cli_cmd_t *p = o;

      // find all parents
      for (i = 0; i < CLI_MAX_CMD; i++)
        if (p->parent_id >= 0)
          p = parents[parents_cnt++] = cli_cmd_by_id((int) p->parent_id);
        else
          break;

      // print all parents
      print_chr(' ');
      while (--parents_cnt >= 0)
      {
        print_str(parents[parents_cnt]->name);
        print_chr(' ');
      }

      // print command and help
#ifndef MRL_PRINT_ESC_OFF
      print_str(o->name);
      print_str("\033[33m");
      print_str(o->args);
      print_str("\033[0m - \033[32m");
      print_str(o->help);
      print_str("\033[0m\r\n");
#else
      print_str(o->name);
      print_str(o->args);
      print_str(" - ");
      print_str(o->help);
      print_str("\r\n");
#endif
    } // for
  }
  else
  { // command (partial) help
    const cli_cmd_t *o;

    // print command help header
    print_str(cmd->help);
    print_str(":\r\n");

    // find all parent/children
    for (o = cli_tree; o->name != NULL; o++)
    {
      int i, flg = 0, parents_cnt = 0;
      const cli_cmd_t *parents[CLI_MAX_CMD];
      const cli_cmd_t *p = o;

      // find all parents
      for (i = 0; i < CLI_MAX_CMD; i++)
      {
        if (p->parent_id >= 0)
        {
          if (p->parent_id == cmd->id)
            flg = 1;
          p = parents[parents_cnt++] = cli_cmd_by_id((int) p->parent_id);
        }
        else
          break;
      }

      if (flg)
      {
        // print all parents
        print_chr(' ');
        while (--parents_cnt >= 0)
        {
          print_str(parents[parents_cnt]->name);
          print_chr(' ');
        }

        // print command and help
#ifndef MRL_PRINT_ESC_OFF
        print_str(o->name);
        print_str("\033[33m");
        print_str(o->args);
        print_str("\033[0m - \033[32m");
        print_str(o->help);
        print_str("\033[0m\r\n");
#else
        print_str(o->name);
        print_str(o->args);
        print_str(" - ");
        print_str(o->help);
        print_str("\r\n");
#endif
      }
    } // for
  }
#endif // CLI_HELP
}
//=============================================================================
void cli_version(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // version
  print_str("hw_revision="  VERSION_HARDWARE_STR "\r\n"
            "soft_version=" VERSION_SOFTWARE_STR "\r\n");
}
//-----------------------------------------------------------------------------
void cli_clear(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // clear
#ifndef MRL_PRINT_ESC_OFF
  print_str("\033[2J"  // ESC seq for clear entire screen
            "\033[H"); // ESC seq for move cursor at left-top corner
#else
  print_str("\r\n\r\n");
#endif
}
//-----------------------------------------------------------------------------
void cli_verbose(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // verbose [n]
  if (argc)
  {
    Opt.verbose = mrl_str2int(argv[0], Opt.verbose, 0);
    Opt.verbose = LIMIT(Opt.verbose, 0, 3);
    print_str("set ");
  }
  print_ival("verbose=", Opt.verbose);
}
//-----------------------------------------------------------------------------
void cli_led(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // led {0|1}
  if (argc == 1)
  {
    int on = !!mrl_str2int(argv[0], 0, 10);
    Led.blink(0);
    Led.set(on);
    if (Opt.verbose)
      print_ival("LED=", on);
  }
}
//-----------------------------------------------------------------------------
void cli_led_blink(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // led blink [N]
  int n = 1;
  if (argc > 0) n = mrl_str2int(argv[0], 0, 10);
  Led.blink(n);

  if (n && Opt.verbose)
  {
    print_str("blink LED ");
    print_int(n);
    print_str(" times\r\n");
  }
}
//-----------------------------------------------------------------------------
void cli_pin(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // pin gpio [0|1]
  if (argc > 0)
  {
    int value, gpio = mrl_str2int(argv[0], 0, 10);
    if (argc > 1)
    { // set pin
      value = !!mrl_str2int(argv[1], 0, 10);
      pinMode(gpio, OUTPUT);
      digitalWrite(gpio, value);
      print_str("set");
    }
    else
    { // read pin
      pinMode(gpio, INPUT);
      value = digitalRead(gpio);
      print_str("read");
    }

    print_str(" gpio");
    print_int(gpio);
    print_uval("=", value);
  }
}
//-----------------------------------------------------------------------------
void cli_sys_info(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // sys info
#if defined(ARDUINO_ESP32)
  print_str("ESP32\r\n");
  print_ival("Chip Revision: ",       ESP.getChipRevision());
  print_sval("Chip Model: ",          ESP.getChipModel());
  print_ival("Chip Cores: ",          ESP.getChipCores());
  print_uval("CPU Freq MHz: ",        ESP.getCpuFreqMHz());
  print_uval("Cycle Count: ",         ESP.getCycleCount());
  print_sval("SDK Verion: ",          ESP.getSdkVersion());
  print_uval("FLASH Chip Size: ",     ESP.getFlashChipSize());
  print_uval("FLASH Chip Speed Hz: ", ESP.getFlashChipSpeed());
#elif defined(ARDUINO_ESP8266)
  print_str("ESP8266\r\n");
  print_hval("Chip ID: 0x",            ESP.getChipId(), 8);
  print_uval("Cycle Count: ",          ESP.getCycleCount());
  print_uval("FLASH Chip Size: ",      ESP.getFlashChipSize());
  //print_uval("FLASH Chip Real Size: ", ESP.getFlashChipRealSize());
  print_uval("FLASH Chip Speed Hz: ",  ESP.getFlashChipSpeed());
#endif
  print_sizeof();
}
//-----------------------------------------------------------------------------
void cli_sys_time(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // sys time
  unsigned long ms = millis();
  unsigned long us = micros();
  print_uval("seconds  = ", Seconds);
  print_uval("ticks    = ", Ticks);
  print_uval("millis() = ", ms);
  print_uval("micros() = ", us);
}
//-----------------------------------------------------------------------------
void cli_sys_reset(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // sys reset
  Reset();
}
//=============================================================================
void cli_eeprom_erase(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // eeprom erase
  uint16_t retv = tfs_erase(&Tfs);
  if (Opt.verbose || retv != TFS_SUCCESS)
  {
    print_str("EEPROM erase ");
    if (retv == TFS_SUCCESS)
      print_str("success\r\n");
    else
    {
      print_str("fail (retv=0x");
      print_hex((unsigned) retv, 4);
      print_str(")\r\n");
    }
  }
}
//-----------------------------------------------------------------------------
void cli_eeprom_write(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // eeprom write
  uint16_t retv;

  Led.on();
  retv = tfs_write(&Tfs, (void*) &Opt, sizeof(opt_t));
  Led.off();

  if (Opt.verbose || retv != TFS_SUCCESS)
  {
    print_str("write EEPROM ");
    if (retv == TFS_SUCCESS)
      print_str("success\r\n");
    else
    {
      print_str("fail (retv=0x");
      print_hex((unsigned) retv, 4);
      print_str(")\r\n");
    }
  }
}
//-----------------------------------------------------------------------------
void cli_eeprom_read(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // eeprom read
  uint16_t size, cnt, retv;
  retv = tfs_read(&Tfs, (void*) &Opt, sizeof(opt_t), &size, &cnt);
  
  if (Opt.verbose || retv != TFS_SUCCESS)
  {
    print_str("read EEPROM ");
    if ((retv & ~TFS_ERR_DELETED) == TFS_SUCCESS && size == sizeof(opt_t))
      print_str("success");
    else
      print_str("fail");

    print_str(" (retv=0x");
    print_hex((unsigned) retv, 4);
    print_str(")\r\n");
  }
}
//-----------------------------------------------------------------------------
void cli_eeprom_delete(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // eeprom delete
  uint16_t retv = tfs_delete(&Tfs);
  
  if (Opt.verbose || retv != TFS_SUCCESS)
  {
    print_str("delete record from EEPROM ");
    
    if (retv == TFS_SUCCESS)
      print_str("success\r\n");
    else
    {
      print_str("fail (retv=0x");
      print_hex((unsigned) retv, 4);
      print_str(")\r\n");
    }
  }
}
//-----------------------------------------------------------------------------
void cli_eeprom_diff(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // eeprom diff
  uint16_t retv;
  opt_t opt;
  const uint8_t *src = (const uint8_t*) &Opt;
  const uint8_t *dst = (const uint8_t*) &opt;
  uint16_t size, cnt;

  if (Opt.verbose)
    print_str("check EEPROM\r\n");
  
  retv = tfs_read(&Tfs, (void*) &opt, sizeof(opt_t), &size, &cnt);

  if ((retv & ~TFS_ERR_DELETED) != TFS_SUCCESS)
  {
    if (Opt.verbose)
      print_str("error: can't read record from EEPROM; exit\r\n");
    return;
  }

  if (size != sizeof(opt_t))
  {
    if (Opt.verbose)
      print_str("error: bad size of record in EEPROM; exit\r\n");
    return;
  }

  for (; size; size--)
    if (*src++ != *dst++) break;

  if (size == 0)
    print_str("no diff\r\n");
  else
    print_str("is diff\r\n");
}
//-----------------------------------------------------------------------------
void cli_eeprom_dump(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // eeprom dump [Offset Size]
  unsigned i, addr = 0, size = 16; // 16 bytes by default
  if (argc > 0) addr = (unsigned) mrl_str2int(argv[0], addr, 10);
  if (argc > 1) size = (unsigned) mrl_str2int(argv[1], size, 10);

  if (Opt.verbose)
    print_str("EEPROM dump:");

  for (i = 0; i < size; i++)
  {
    uint8_t b;
    eeprom_read(addr + i, sizeof(uint8_t), &b);
    print_str(" 0x"); print_hex(b, 2);
  }
  print_eol();
}
//=============================================================================
void cli_def(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // def
  int8_t retv;
  opt_default(&Opt); // set to default all options

  retv = sx128x_standby(&Radio, SX128X_STANDBY_RC);
  if (retv != SX128X_ERR_NONE) return;

  retv = sx128x_restore(&Radio);
  if (retv != SX128X_ERR_NONE) return;

  print_str("set to default all options\r\n");
}
//=============================================================================
void cli_hw_reset(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // hw reset [t1 t2]
  uint8_t status;
  int t1 = SX128X_HW_RESET_T1, t2 = SX128X_HW_RESET_T2;
  if (argc > 0) t1 = mrl_str2int(argv[0], t1, 10);
  if (argc > 1) t2 = mrl_str2int(argv[1], t2, 10);
  sx128x_hw_reset(t1, t2, NULL);
  sx128x_status(&Radio, &status);
  print_str("hard reset SX128x\r\n");
  Led.blink();
}
//-----------------------------------------------------------------------------
void cli_hw_rxen(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // hw rxen [0|1]
  if (argc > 0) Opt.rxen = RXEN = !!mrl_str2int(argv[0], RXEN, 0);
  setRXEN(RXEN);
  print_uval("RXEN=", RXEN);
}
//-----------------------------------------------------------------------------
void cli_hw_txen(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // hw txen [0|1]
  if (argc > 0) Opt.txen = TXEN = !!mrl_str2int(argv[0], TXEN, 0);
  setTXEN(TXEN);
  print_uval("TXEN=", TXEN);
}
//-----------------------------------------------------------------------------
void cli_hw_busy(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // hw busy
  print_uval("BUSY=", sx128x_hw_busy(NULL));
}
//-----------------------------------------------------------------------------
void cli_spi(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // spi [b0 b1..]
  uint8_t *tx, *rx;
  uint8_t retv;
  int i;

  if (argc == 0) return;
  if (argc > 256) argc = 256;

  tx = (uint8_t*) malloc(argc * 2);
  rx = tx + argc;
  if (tx == (uint8_t*) NULL) return;

  for (i = 0; i < argc; i++)
  {
    tx[i] = (uint8_t) mrl_str2int(argv[i], 0, 0);
    rx[i] = 0;
  }

  retv = sx128x_hw_exchange(rx, tx, argc, NULL);
  if (!retv) return;

  print_str("SPI TX:");
  for (i = 0; i < argc; i++)
  {
    print_chr(' ');
    print_hex(tx[i], 2);
  }

  print_str("\r\nSPI RX:");
  for (i = 0; i < argc; i++)
  {
    print_chr(' ');
    print_hex(rx[i], 2);
  }

  print_eol();

  free(tx);
}
//=============================================================================
void cli_radio_status_print(uint8_t status)
{
  uint8_t mode, stat;
  sx128x_status_unpack(status, &mode, &stat);
  print_str("status=0x");
  print_hex(status, 2);
  if (Opt.verbose)
  {
#ifdef SX128X_USE_EXTRA
    print_str(": ChipMode="); print_str(sx128x_status_mode_string[mode]);
    print_str(" CmdStatus="); print_str(sx128x_status_cmd_string[stat]);
#else
    print_str(": ChipMode="); print_uint((unsigned) mode);
    print_str(" CmdStatus="); print_uint((unsigned) stat);
#endif
    print_str(" CpuBusy=");   print_int((int) (status & 1));
    print_str(" DmaBusy=");   print_int((int) (status >> 1) & 1);
  }
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_radio_status(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio status
  uint8_t status;
  int8_t retv = sx128x_status(&Radio, &status);
  if (retv == SX128X_ERR_NONE) cli_radio_status_print(status);
  else print_ival("status error: retv=", retv);
}
//-----------------------------------------------------------------------------
void cli_radio_stat_last(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio status last
  cli_radio_status_print(sx128x_last_status(&Radio));
}
//-----------------------------------------------------------------------------
void cli_radio_ver(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio ver
  uint16_t fw_version;
  int8_t retv = sx128x_fw_version(&Radio, &fw_version);
  if (retv != SX128X_ERR_NONE) return;
  print_hval("fw_version=0x", fw_version, 4);
}
//-----------------------------------------------------------------------------
void cli_radio_sleep(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio sleep [config]
  int8_t retv;
  uint8_t conf = 0; // SX128X_SLEEP_RAM_RETENTION | SX128X_SLEEP_BUF_RETENTION;
  if (argc > 0) conf = (uint8_t) mrl_str2int(argv[0], conf, 0);

  Led.off();
  
  setRXEN(0);
  setTXEN(0);

  if (sx128x_get_sleep(&Radio))
  {
    if (Opt.verbose)
      print_str("radio already sleep\r\n");
    return;
  }

  retv = sx128x_sleep(&Radio, conf);
  if (retv != SX128X_ERR_NONE) return;

  print_hval("radio sleep: conf=0x", conf, 2);
  if (Opt.verbose)
  {
    print_uval("  config_retention=", !!(conf & SX128X_SLEEP_RAM_RETENTION));
    print_uval("  buffer_retention=", !!(conf & SX128X_SLEEP_BUF_RETENTION));
  }
}
//-----------------------------------------------------------------------------
static void sx128x_standby_mode_print(uint8_t conf)
{
  print_str("conf=0x");
  print_hex(conf, 2);
  
  if (Opt.verbose)
  {
    print_str(" (");
    print_str(conf == SX128X_STANDBY_RC ? "RC13M" : "XOSC");
    print_chr(')');
  }
  print_eol();

}
//-----------------------------------------------------------------------------
void cli_radio_standby(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio standby [config]
  int8_t retv;
  uint8_t conf = SX128X_STANDBY_RC; // SX128X_STANDBY_RC or SX128X_STANDBY_XOSC
  if (argc > 0) conf = (uint8_t) mrl_str2int(argv[0], conf, 0);

  Led.off();

  retv = sx128x_standby(&Radio, conf);
  if (retv != SX128X_ERR_NONE) return;
  
  setRXEN(Opt.rxen);
  setTXEN(Opt.txen);

  print_str("radio standby: ");
  sx128x_standby_mode_print(conf);
}
//-----------------------------------------------------------------------------
void cli_radio_wakeup(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio wakeup [config]
  int8_t retv;
  uint8_t conf = SX128X_STANDBY_RC; // SX128X_STANDBY_RC or SX128X_STANDBY_XOSC
  if (argc > 0) conf = (uint8_t) mrl_str2int(argv[0], conf, 0);

  Led.off();
  
  retv = sx128x_wakeup(&Radio, conf);
  if (retv != SX128X_ERR_NONE)
  {
    print_ival("radio wakeup error: retv=", retv);
    return;
  }

  setRXEN(Opt.rxen);
  setTXEN(Opt.txen);

  print_str("radio wakeup: ");
  sx128x_standby_mode_print(conf);
}
//-----------------------------------------------------------------------------
void cli_radio_mode(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio mode [mode]
#ifdef EXTRA
  static const char *modes[] = {
    "GFSK", "LoRa", "Ranging", "FLRC", "BLE", "Reserved" };
#endif
  int8_t retv;
  uint8_t mode;

  if (argc == 0)
  { // read packet type
    retv = sx128x_read_mode(&Radio, &mode);
    if (retv != SX128X_ERR_NONE) return;
    print_str("read");
  }
  else // argc > 0
  { // set packet type
    mode = (uint8_t) mrl_str2int(argv[0], 1, 10);
    retv = sx128x_mode(&Radio, mode);
    if (retv != SX128X_ERR_NONE) return;
    print_str("set");
  }

  print_str(" mode=");
  print_int(mode);
#ifdef EXTRA
  if (mode > 5) mode = 5;
  print_str(" (");
  print_str(modes[mode]);
  print_str(")\r\nmodes: " SX128X_PACKET_TYPE_HELP);
#endif
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_radio_save(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio save
  int8_t retv = sx128x_save_context(&Radio);
  if (retv != SX128X_ERR_NONE) return;
  print_str("save context\r\n");
}
//-----------------------------------------------------------------------------
void cli_radio_fs(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio fs
  int8_t retv = sx128x_fs(&Radio);
  if (retv != SX128X_ERR_NONE) return;
  print_str("FS\r\n");
}
//-----------------------------------------------------------------------------
#ifdef EXTRA
static const char *cli_time_base[] =
{ "15.625us", "62.5us", "1ms", "4ms" };
#endif
//-----------------------------------------------------------------------------
void cli_radio_tx(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio tx [to base]
  uint16_t timeout = 0;
  uint8_t base = 0;
  int8_t retv;

  if (argc > 0) timeout = (uint16_t) mrl_str2int(argv[0], timeout, 10);
  if (argc > 1) base    = (uint8_t)  mrl_str2int(argv[1], base,    10);
  if (base > 3) base = 3;
  
  retv = sx128x_tx(&Radio, timeout, base);
  if (retv != SX128X_ERR_NONE) return;

  Fsm.tx_start(TIME_FUNC());

  print_str("TX: timeout=0x");
  print_hex(timeout, 4);
  print_str(" base=0x");
  print_hex(base, 2);;
#ifdef EXTRA
  print_str(" (");
  print_str(cli_time_base[base]);
  print_chr(')');
#endif
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_radio_rx(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio rx [to base]
  uint16_t timeout = SX128X_RX_TIMEOUT_CONTINUOUS; // RX Continuous mode
  uint8_t base = SX128X_TIME_BASE_1MS; // 1ms
  int8_t retv;

  Led.off();

  if (argc > 0) timeout = (uint16_t) mrl_str2int(argv[0], timeout, 10);
  if (argc > 1) base    = (uint8_t)  mrl_str2int(argv[1], base,    10);
  if (base > 3) base = 3;

  retv = sx128x_rx(&Radio, timeout, base);
  if (retv != SX128X_ERR_NONE) return;

  print_str("RX: timeout=0x");
  print_hex(timeout, 4);
  print_str(" base=0x");
  print_hex(base, 2);;
#ifdef EXTRA
  print_str(" (");
  print_str(cli_time_base[base]);
  print_chr(')');
#endif
  print_eol();
}
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_RANGING)
void cli_radio_cad(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio cad
  int8_t retv = sx128x_cad(&Radio);
  if (retv != SX128X_ERR_NONE) return;
  print_str("CAD\r\n");
}
#endif // SX128X_USE_LORA || SX128X_RANGING
//-----------------------------------------------------------------------------
void cli_radio_freq(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio freq [Hz]
  uint32_t freq = sx128x_get_frequency(&Radio);
  if (argc > 0)
  { // set RF frequency
    freq = (uint32_t) mrl_str2int(argv[0], freq, 10);
    int8_t retv = sx128x_set_frequency(&Radio, freq);
    if (retv != SX128X_ERR_NONE) return;

    freq = sx128x_get_frequency(&Radio);
    print_str("set");
  }
  else
    print_str("get");

  print_str(" freq=");
  print_uint(freq);
  print_str("Hz\r\n");
}
//-----------------------------------------------------------------------------
void cli_radio_pwr(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio pwr [dBm us]
  int8_t power;
  uint8_t ramp;
  sx128x_get_power(&Radio, &power, &ramp); // read old defaults

  if (argc > 0)
  { // set power level and ramp time
    int8_t retv;
    power              = (int8_t)  mrl_str2int(argv[0], power, 10);
    if (argc > 1) ramp = (uint8_t) mrl_str2int(argv[1], ramp,  10);
    retv = sx128x_set_power(&Radio, power, ramp);
    if (retv != SX128X_ERR_NONE) return;
    print_str("set TX power=");
  }
  else
  { // get power level and ramp time
    print_str("TX power=");
  }
  print_int(Opt.radio.power);
  print_str("dBm rampTime=");
  print_uint(Opt.radio.ramp);
  print_str("us\r\n");
}
//-----------------------------------------------------------------------------
void cli_radio_lna(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio lna [1|0]
  int8_t retv;
  uint8_t lna_boost;
  if (argc > 0)
  { // set LNA boost
    lna_boost = !!mrl_str2int(argv[0], 0, 10);
    retv = sx128x_set_lna_boost(&Radio, lna_boost);
    if (retv != SX128X_ERR_NONE) return;
    print_str("set LNA boost ");
  }
  else
  { // get LNA boost
    lna_boost = sx128x_get_lna_boost(&Radio);
    print_str("LNA boost is ");
  }
  print_str(lna_boost ? "ON" : "OFF");
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_radio_gain(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio gain [rx_gain]
  int8_t retv;
  uint8_t gain = sx128x_get_gain(&Radio);
  if (argc > 0)
  { // set RX gain
    gain = mrl_str2int(argv[0], gain, 10);
    retv = sx128x_set_gain(&Radio, gain);
    if (retv != SX128X_ERR_NONE) return;
    gain = sx128x_get_gain(&Radio);
    print_str("set RX gain=");
  }
  else
  { // get RX gain
    print_str("get RX gain=");
  }
  print_uint(gain);
#ifdef SX128X_USE_EXTRA
  print_str(" (");
  print_str(sx128x_gain_string[gain]);
  print_str(")");
#endif
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_radio_dcdc(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio dcdc [1|0]
  int8_t retv;
  uint8_t dcdc = sx128x_get_dcdc(&Radio);
  if (argc > 0)
  { // set DC-DC
    dcdc = !!mrl_str2int(argv[0], dcdc, 10);
    retv = sx128x_set_dcdc(&Radio, dcdc);
    if (retv != SX128X_ERR_NONE) return;
    print_str("set DC-DC ");
  }
  else
  { // get DC-DC
    print_str("DC-DC is ");
  }
  print_str(dcdc ? "ON" : "OFF");
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_radio_auto_fs(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio auto_fs [enable]
  int8_t retv;
  uint8_t enable = 0; // disable by default
  if (argc > 0) enable = (uint8_t) !!mrl_str2int(argv[0], enable, 10);
  retv = sx128x_auto_fs(&Radio, enable);
  if (retv != SX128X_ERR_NONE) return;
  print_uval("set auto FS: enable=", enable);
}
//-----------------------------------------------------------------------------
void cli_radio_irq(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio irq [MASK]
  uint16_t irq;

  // get IRQ
  int8_t retv = sx128x_get_irq(&Radio, &irq);
  if (retv != SX128X_ERR_NONE) return;

  print_str("get IRQ=0b");
  print_bin(irq, 16);

  if (Opt.verbose)
  {
    print_str(":\r\n");
    print_ival("               TxDone=", !!(irq & SX128X_IRQ_TX_DONE              ));
    print_ival("               RxDone=", !!(irq & SX128X_IRQ_RX_DONE              ));
    print_ival("        SyncWordValid=", !!(irq & SX128X_IRQ_SYNC_WORD_VALID      ));
    print_ival("        SyncWordError=", !!(irq & SX128X_IRQ_SYNC_WORD_ERROR      ));
    print_ival("          HeaderValid=", !!(irq & SX128X_IRQ_HEADER_VALID         ));
    print_ival("          HeaderError=", !!(irq & SX128X_IRQ_HEADER_ERROR         ));
    print_ival("               CrcErr=", !!(irq & SX128X_IRQ_CRC_ERROR            ));
    print_ival("    SlaveResponseDone=", !!(irq & SX128X_IRQ_SLAVE_RESPONSE_DONE  ));
    print_ival("  SlaveRequestDiscard=", !!(irq & SX128X_IRQ_SLAVE_REQUEST_DISCARD));
    print_ival("    MasterResultValid=", !!(irq & SX128X_IRQ_MASTER_RESULT_VALID  ));
    print_ival("        MasterTimeout=", !!(irq & SX128X_IRQ_MASTER_TIMEOUT       ));
    print_ival("    SlaveRequestValid=", !!(irq & SX128X_IRQ_SLAVE_REQUEST_VALID  ));
    print_ival("              CadDone=", !!(irq & SX128X_IRQ_CAD_DONE             ));
    print_ival("          CadDetected=", !!(irq & SX128X_IRQ_CAD_DETECTED         ));
    print_ival("          RxTxTimeout=", !!(irq & SX128X_IRQ_RX_TX_TIMEOUT        ));
    if (sx128x_get_advanced_ranging(&Radio))
      print_ival("  AdvancedRangingDone=", !!(irq & SX128X_IRQ_ADVANCED_RANGING_DONE));
    else
      print_ival("     PreambleDetected=", !!(irq & SX128X_IRQ_PREAMBLE_DETECTED    ));
  }
  else
    print_eol();


  if (argc > 0)
  { // clear IRQ status by mask
    uint16_t mask = (uint16_t) mrl_str2int(argv[0], 0, 10);
    retv = sx128x_clear_irq(&Radio, mask);
    if (retv != SX128X_ERR_NONE) return;

    print_str("clear IRQ status by mask=0b"); print_bin(mask, 16);
    if (Opt.verbose && mask)
    {
      print_str(":\r\n");
      if (mask & SX128X_IRQ_TX_DONE)               print_str("  - TxDone\r\n");
      if (mask & SX128X_IRQ_RX_DONE)               print_str("  - RxDone\r\n");
      if (mask & SX128X_IRQ_SYNC_WORD_VALID)       print_str("  - SyncWordValid\r\n");
      if (mask & SX128X_IRQ_SYNC_WORD_ERROR)       print_str("  - SyncWordError\r\n");
      if (mask & SX128X_IRQ_HEADER_VALID)          print_str("  - HeaderValid\r\n");
      if (mask & SX128X_IRQ_HEADER_ERROR)          print_str("  - HeaderError\r\n");
      if (mask & SX128X_IRQ_CRC_ERROR)             print_str("  - CrcErr\r\n");
      if (mask & SX128X_IRQ_SLAVE_RESPONSE_DONE)   print_str("  - SlaveResponseDone\r\n");
      if (mask & SX128X_IRQ_SLAVE_REQUEST_DISCARD) print_str("  - SlaveRequestDiscard\r\n");
      if (mask & SX128X_IRQ_MASTER_RESULT_VALID)   print_str("  - MasterResultValid\r\n");
      if (mask & SX128X_IRQ_MASTER_TIMEOUT)        print_str("  - MasterTimeout\r\n");
      if (mask & SX128X_IRQ_SLAVE_REQUEST_VALID)   print_str("  - SlaveRequestValid\r\n");
      if (mask & SX128X_IRQ_CAD_DONE)              print_str("  - CadDone\r\n");
      if (mask & SX128X_IRQ_CAD_DETECTED)          print_str("  - CadDetected\r\n");
      if (mask & SX128X_IRQ_RX_TX_TIMEOUT)         print_str("  - RxTxTimeout\r\n");
      if (sx128x_get_advanced_ranging(&Radio))
      {
        if (mask & SX128X_IRQ_ADVANCED_RANGING_DONE) print_str("  - AdvancedRangingDone\r\n");
      }
      else
      {
        if (mask & SX128X_IRQ_PREAMBLE_DETECTED)     print_str("  - PreambleDetected\r\n");
      }
    }
    else
      print_eol();
  }
}
//-----------------------------------------------------------------------------
void cli_radio_rxdc(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio rxdc [rx sleep base]
  uint16_t rx = 700;
  uint16_t sleep = 1000;
  uint8_t base = 0x02; // 1ms
  int8_t retv;

  if (argc > 0) rx    = (uint16_t) mrl_str2int(argv[0], rx,    10);
  if (argc > 1) sleep = (uint16_t) mrl_str2int(argv[1], sleep, 10);
  if (argc > 2) base  = (uint8_t)  mrl_str2int(argv[2], base,  10);
  if (base > 3) base = 3;

  retv = sx128x_rx_duty_cycle(&Radio, rx, sleep, base);
  if (retv != SX128X_ERR_NONE) return;

  print_str("RX duty cycle: rx=0x");
  print_hex(rx, 4);
  print_str(" sleep=0x");
  print_hex(sleep, 4);
  print_str(" base=0x");
  print_hex(base, 2);
#ifdef EXTRA
  print_str(" (");
  print_str(cli_time_base[base]);
  print_chr(')');
#endif
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_radio_wave(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio wave
  int8_t retv = sx128x_restore(&Radio);
  if (retv != SX128X_ERR_NONE) return;
  
  retv = sx128x_tx_wave(&Radio);
  if (retv != SX128X_ERR_NONE) return;
  
  Led.on();
  
  print_str("continuous wave\r\n");
}
//-----------------------------------------------------------------------------
void cli_radio_preamble(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio preamble
  int8_t retv = sx128x_standby(&Radio, SX128X_STANDBY_XOSC);
  if (retv != SX128X_ERR_NONE) return;
  
  retv = sx128x_tx_preamble(&Radio);
  if (retv != SX128X_ERR_NONE) return;

  Led.on();

  print_str("continuous preamble\r\n");
}
//-----------------------------------------------------------------------------
void cli_radio_reg(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio reg [addr val]
  uint16_t addr = 0x891; // FIXME: magic;
  uint8_t value = 0x00;
  int8_t retv;

  if (argc < 1) return; // no address
  addr = (uint16_t) mrl_str2int(argv[0], addr, 0);

  if (argc > 1)
  { // write register (1 byte)
    value = (uint8_t) mrl_str2int(argv[1], value, 0);
    retv = sx128x_reg_write(&Radio, addr, &value, 1);
    if (retv != SX128X_ERR_NONE) return;

    print_str("reg[0x");
    print_hex(addr, 4);
    print_hval("] <= 0x", (unsigned) value, 2);
  }
  else
  { // read register (1 byte)
    retv = sx128x_reg_read(&Radio, addr, &value, 1);
    if (retv != SX128X_ERR_NONE) return;

    print_str("reg[0x");
    print_hex(addr, 4);
    print_hval("] => 0x", (unsigned) value, 2);
  }
}
//-----------------------------------------------------------------------------
void cli_radio_restore(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio restore
  int8_t retv;

  Led.off();
  
  retv = sx128x_restore(&Radio);
  if (retv != SX128X_ERR_NONE) return;
  
  setRXEN(Opt.rxen);
  setTXEN(Opt.txen);

  print_str("restore\r\n");
}
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_RANGING)
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_USE_GFSK)
void cli_radio_lp(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // radio lp [enable]
  int8_t retv;
  uint8_t enable = 0; // disable by default
  if (argc > 0) enable = (uint8_t) !!mrl_str2int(argv[0], enable, 10);
  retv = sx128x_long_preamble(&Radio, enable);
  if (retv != SX128X_ERR_NONE) return;
  print_uval("set Long Preamble: enable=", enable);
}
#endif // SX128X_USE_LORA || SX128X_USE_GFSK
//-----------------------------------------------------------------------------
void cli_lora_mod(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // lora mod [BW SF CR]
  if (argc > 0) // BW, SF, CR
  { // set modulation params in LoRa or Ranging mode
    int8_t retv;
    uint16_t bw = Opt.radio.bw;
    uint8_t  sf = Opt.radio.sf;
    uint8_t  cr = Opt.radio.cr;

    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_LORA &&
        mode != SX128X_PACKET_TYPE_RANGING)
    { // change mode to LoRa
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_LORA);
      if (retv != SX128X_ERR_NONE) return;
    }

    bw               = (uint16_t) mrl_str2int(argv[0], bw, 10);
    if (argc > 1) sf = (uint8_t)  mrl_str2int(argv[1], sf, 10);
    if (argc > 2) cr = (uint8_t)  mrl_str2int(argv[2], cr, 10);

    retv = sx128x_mod_lora(&Radio, bw, sf, cr);
    if (retv != SX128X_ERR_NONE) return;
    print_str("lora mod: set");
  }
  else
    print_str("lora mod:");

  print_str(" BW=");    print_uint(Opt.radio.bw);
  print_str("kHz SF="); print_uint(Opt.radio.sf);
  print_str(" CR=");    print_uint(Opt.radio.cr);
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_lora_packet(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // lora packet [PR CRC INV]
  if (argc > 0) // PR, CRC, INV
  { // set packet params in LoRa mode or Ranging mode
    int8_t retv;
    uint32_t preamble = Opt.radio.preamble;
    uint8_t crc       = Opt.radio.crc;
    uint8_t invert_iq = Opt.radio.invert_iq;

    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_LORA &&
        mode != SX128X_PACKET_TYPE_RANGING)
    { // change mode to LoRa
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_LORA);
      if (retv != SX128X_ERR_NONE) return;
    }

    preamble                = (uint32_t) mrl_str2int(argv[0], preamble,  0);
    if (argc > 1) crc       = (uint8_t)  mrl_str2int(argv[1], crc,       0);
    if (argc > 2) invert_iq =          !!mrl_str2int(argv[2], invert_iq, 0);

    retv = sx128x_packet_lora(&Radio, preamble, crc, invert_iq,
                              Opt.radio.fixed, Opt.radio.payload_size);
    if (retv != SX128X_ERR_NONE) return;

    print_str("lora packet: set");
  }
  else
    print_str("lora packet:");

  print_str(" Preamble="); print_uint(Opt.radio.preamble);
  print_str(" CRC=");      print_uint(Opt.radio.crc);
  print_str(" invertIQ="); print_uint(Opt.radio.invert_iq);
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_lora_sw(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // lora sw [SW]
  int8_t retv;
  uint8_t sw;
  if (argc > 0)
  { // set SW
    sw = mrl_str2int(argv[0], 0, 10);
    retv = sx128x_set_sw_lora(&Radio, sw);
    if (retv != SX128X_ERR_NONE) return;
    print_hval("set LoRa SW=0x", sw, 2);
  }
  else
  { // get SW
    sw = sx128x_get_sw_lora(&Radio);
    print_hval("LoRa SW=0x", sw, 2);
  }
}
//-----------------------------------------------------------------------------
void cli_lora_cad(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // lora cad [sym_num]
  uint8_t sym_num = sx128x_get_cad_lora(&Radio);
  if (argc > 0)
  { // set CAD sym_num
    int8_t retv;
    sym_num = (uint8_t) mrl_str2int(argv[0], sym_num, 10);
    retv = sx128x_set_cad_lora(&Radio, sym_num);
    if (retv != SX128X_ERR_NONE) return;

    sym_num = sx128x_get_cad_lora(&Radio);
    print_str("set ");
  }
  else
    print_str("get ");
  print_uval("LoRa CAD sym_num=", sym_num);
}
//-----------------------------------------------------------------------------
void cli_lora_fei(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // lora fei
  int32_t fei;
  int8_t retv = sx128x_fei_lora(&Radio, &fei);
  if (retv != SX128X_ERR_NONE) return;
  print_str("LoRa FEI=");
  print_int(fei);
  print_str("Hz\r\n");
}
//-----------------------------------------------------------------------------
void cli_lora_rssi(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // lora rssi
  static const char dp[2] = { '0', '5' };
  int rssi_db;
  char rssi_0db5;
  uint8_t rssi;

  int8_t retv = sx128x_rssi_lora(&Radio, &rssi);
  if (retv != SX128X_ERR_NONE) return;

  rssi_db   = ((int) rssi) / -2;
  rssi_0db5 = dp[rssi & 1]; // FIXME: check me!

  print_str("LoRa RSSI=");
  print_int(rssi_db);
  print_chr('.');
  print_chr(rssi_0db5);
  print_str("dB\r\n");
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_LORA || SX128X_RANGING
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_RANGING
//-----------------------------------------------------------------------------
void cli_ranging(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // ranging [role MA SA SM]

  if (argc > 0) // role, MA, SA, SM
  {
    int8_t retv;
    uint8_t role = Opt.radio.role;
    uint32_t ma  = Opt.radio.master_address;
    uint32_t sa  = Opt.radio.slave_address;
    uint8_t  sm  = Opt.radio.slave_mode;

    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_LORA &&
        mode != SX128X_PACKET_TYPE_RANGING)
    { // change mode to Ranging
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_RANGING);
      if (retv != SX128X_ERR_NONE) return;
    }

    role             = (uint8_t)  mrl_str2int(argv[0], role, 0);
    if (argc > 1) ma = (uint32_t) mrl_str2int(argv[1], ma,   0);
    if (argc > 2) sa = (uint32_t) mrl_str2int(argv[2], sa,   0);
    if (argc > 3) sm = (uint8_t)  mrl_str2int(argv[3], sm,   0);

    // set Ranging role (master/slave)
    retv = sx128x_ranging_role(&Radio, role);
    if (retv != SX128X_ERR_NONE) return;

    // set Ranging master request address
    sx128x_ranging_master_address(&Radio, ma);
    if (retv != SX128X_ERR_NONE) return;

    // set Ranging slave respond address and bits check mode
    sx128x_ranging_slave_address(&Radio, sa, sm);
    if (retv != SX128X_ERR_NONE) return;

    print_str("ranging: set");
  }
  else
    print_str("ranging:");

  print_str(" role="); print_uint(Opt.radio.role);
#ifdef EXTRA
  print_str(Opt.radio.role ? " (Master)" : " (Slave)");
#endif
  print_str(" MasterAddress=0x"); print_hex( Opt.radio.master_address, 8);
  print_str(" SlaveAddres=0x");   print_hex( Opt.radio.slave_address, 8);
  print_str(" SlaveMode=");       print_uint(Opt.radio.slave_mode);
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_ranging_advanced(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // ranging advanced {0|1}
  uint8_t on = sx128x_get_advanced_ranging(&Radio);
  if (argc > 0)
  { // on/off Advanced Ranging
    int8_t retv;
    on = !!mrl_str2int(argv[0], on, 0);
    retv = sx128x_set_advanced_ranging(&Radio, on);
    if (retv != SX128X_ERR_NONE) return;
    print_str("set ");
  }
  print_str("Advanced Ranging ");
  print_str(on ? "ON" : "OFF");
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_ranging_calib(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // ranging calib [calibration]
  uint32_t calib;
  int8_t retv = sx128x_ranging_get_calibration(&Radio, &calib);
  if (retv != SX128X_ERR_NONE) return;

  if (argc > 0)
  { // set calibration
    calib = (uint32_t) mrl_str2int(argv[0], calib, 0);

    retv = sx128x_ranging_set_calibration(&Radio, calib);
    if (retv != SX128X_ERR_NONE) return;

    print_str("set ");
  }

  print_str("ranging Callibration=");
  print_int(calib);
  print_str(" (0x");
  print_hex(calib, 8);
  print_str(")\r\n");
}
//-----------------------------------------------------------------------------
void cli_ranging_result(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // ranging result [filter]
  int8_t   retv;
  uint8_t  filter = 2; // result type (0-off, 1-on, 2-as-is)
  uint32_t result;     // raw result
  int32_t  distance;   // distance [dm]
  uint8_t  rssi;       // RSSI of last exchange

  if (argc > 0) filter = (uint8_t) mrl_str2int(argv[0], filter, 0);

  retv = sx128x_ranging_result(&Radio, &filter, &result, &distance, &rssi);
  if (retv != SX128X_ERR_NONE) return;

  print_str("ranging result=0x");
  print_hex((unsigned) result, 8);
  print_str(" distance=");
  print_int((int) distance);
  print_str("dm RSSI=");
  print_uint((unsigned) rssi);
  print_str(" filter=");
  print_str(filter == 0 ? "OFF" :
            filter == 1 ? "ON" :
                          "AS-IS");
  print_eol();
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_RANGING
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_FLRC
//-----------------------------------------------------------------------------
void cli_flrc_mod(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // flrc mod [BR CR BT]
  if (argc > 0)
  { // set modulation params in FLRC mode
    int8_t retv;
    uint16_t br = Opt.radio.flrc_br; // 260, 325, 520, 650, 1040, 1300 kBit/s
    uint8_t  cr = Opt.radio.flrc_cr; // 1...3 {1, 3/4, 1/2}
    uint8_t  bt = Opt.radio.flrc_bt; // 0...2 {off, 0.5, 1.0}

    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_FLRC)
    { // change mode to FLRC
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_FLRC);
      if (retv != SX128X_ERR_NONE) return;
    }

    br               = (uint16_t) mrl_str2int(argv[0], br, 0);
    if (argc > 1) cr = (uint8_t)  mrl_str2int(argv[1], cr, 0);
    if (argc > 2) bt = (uint8_t)  mrl_str2int(argv[2], bt, 0);

    retv = sx128x_mod_flrc(&Radio, br, cr, bt);
    if (retv != SX128X_ERR_NONE) return;
    print_str("flrc mod: set");
  }
  else
    print_str("flrc mod:");

  print_str(" BR=");       print_uint(Opt.radio.flrc_br);
  print_str("kbit/s CR="); print_uint(Opt.radio.flrc_cr);
  print_str(" BT=");       print_uint(Opt.radio.flrc_bt);
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_flrc_packet(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // flrc packet [PR SW SWM CRC]
  if (argc > 0)
  { // set packet params in FLRC mode
    int8_t retv;
    uint16_t preamble  = Opt.radio.flrc_preamble;  // 4, 8, 12, 16, 20, 24, 28, 32
    uint8_t  sw_on     = Opt.radio.flrc_sw_on;     // 0/1 {no sync, 32-bits SW}
    uint8_t  sw_mode   = Opt.radio.flrc_sw_mode;   // 0..7 {off, sw1, sw2, sw12, sw3, sw13, sw23, sw123}
    uint8_t  crc       = Opt.radio.flrc_crc;       // 0..3 {off, 2 bytes, 3 bytes, 4 bytes}

    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_FLRC)
    { // change mode to FLRC
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_FLRC);
      if (retv != SX128X_ERR_NONE) return;
    }

    preamble                = (uint16_t)   mrl_str2int(argv[0], preamble,  0);
    if (argc > 1) sw_on     = (uint8_t)  !!mrl_str2int(argv[1], sw_on,     0);
    if (argc > 2) sw_mode   = (uint8_t)    mrl_str2int(argv[2], sw_mode,   0);
    if (argc > 3) crc       = (uint8_t)    mrl_str2int(argv[3], crc,       0);

    retv = sx128x_packet_flrc(&Radio, preamble, sw_on, sw_mode, crc,
                              Opt.radio.fixed, Opt.radio.payload_size);

    if (retv != SX128X_ERR_NONE) return;
    print_str("flrc packet: set");
  }
  else
    print_str("flrc packet:");

  print_str(" PR=");  print_uint(Opt.radio.flrc_preamble);
  print_str(" SW=");  print_uint(Opt.radio.flrc_sw_on);
  print_str(" SWM="); print_uint(Opt.radio.flrc_sw_mode);
  print_str(" CRC="); print_uint(Opt.radio.flrc_crc);
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_flrc_swt(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // flrc swt [0...15]
  int8_t retv;
  uint8_t tolerance = sx128x_get_swt_flrc(&Radio);
  if (argc > 0)
  {
    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_FLRC)
    { // change mode to FLRC
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_FLRC);
      if (retv != SX128X_ERR_NONE) return;
    }

    tolerance = mrl_str2int(argv[0], tolerance, 0);
    retv = sx128x_set_swt_flrc(&Radio, tolerance);
    if (retv != SX128X_ERR_NONE) return;

    tolerance = sx128x_get_swt_flrc(&Radio);
    print_str("set ");
  }
  print_ival("flrc_swt=", (int) tolerance);
}
//-----------------------------------------------------------------------------
void cli_flrc_sw(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // flrc sw [SW1 SW2 SW3]
  int i;
  int8_t retv;
  if (argc > 0)
  {
    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_FLRC)
    { // change mode to FLRC
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_FLRC);
      if (retv != SX128X_ERR_NONE) return;
    }

    for (i = 0; i < argc; i++)
    {
      uint32_t sw = (uint32_t) mrl_str2int(argv[i], Opt.radio.flrc_sw[i], 0);
      retv = sx128x_set_sw_flrc(&Radio, i, sw);
      if (retv != SX128X_ERR_NONE) return;
    }
  }

  for (i = 0; i < 3; i++)
  {
    print_str("flrc_SW"); print_int(i + 1); print_str("=0x");
    print_hex(Opt.radio.flrc_sw[i], 8); print_eol();
  }
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_FLRC
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_GFSK
//-----------------------------------------------------------------------------
void cli_gfsk_mod(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // gfsk mode [BR DSB MI BT]
  if (argc > 0)
  {
    int8_t retv;
    uint16_t br  = Opt.radio.gfsk_br;  // 125, 250, 400, 500, 800, 1000, 1600, 2000 kb/s
    uint8_t  dsb = Opt.radio.gfsk_dsb; // 0 or 1 (BW = BR * (DSB + 1))
    uint8_t  mi  = Opt.radio.gfsk_mi;  // 0...15 {0.35...4.0}
    uint8_t  bt  = Opt.radio.gfsk_bt;  // 0...2 {off, 0.5, 1.0}

    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_GFSK)
    { // change mode to GFSK
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_GFSK);
      if (retv != SX128X_ERR_NONE) return;
    }

    br                = (uint16_t)   mrl_str2int(argv[0], br,  10);
    if (argc > 1) dsb = (uint8_t)  !!mrl_str2int(argv[1], dsb, 10);
    if (argc > 2) mi  = (uint8_t)    mrl_str2int(argv[2], mi,  10);
    if (argc > 3) bt  = (uint8_t)    mrl_str2int(argv[3], bt,  10);

    retv = sx128x_mod_gfsk(&Radio, br, dsb, mi, bt);
    if (retv != SX128X_ERR_NONE) return;
    print_str("gfsk mod: set");
  }
  else
    print_str("gfsk mod:");

  print_str(" BR=");        print_uint(Opt.radio.gfsk_br);
  print_str("kbit/s DSB="); print_uint(Opt.radio.gfsk_dsb);
  print_str(" MI=");        print_uint(Opt.radio.gfsk_mi);
  print_str(" BT=");        print_uint(Opt.radio.gfsk_bt);
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_gfsk_packet(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // gfsk packet [PR SWL SWM CRC W]
  if (argc > 0)
  { // set packet params in FLRC mode
    int8_t retv;
    uint16_t preamble  = Opt.radio.gfsk_preamble;  // 4, 8, 12, 16, 20, 24, 28, 32
    uint8_t  sw_len    = Opt.radio.gfsk_sw_len;    // 1...5
    uint8_t  sw_mode   = Opt.radio.gfsk_sw_mode;   // 0...7 {off, 1->1, 2->2, 1->12, 3->3, 1->13, 1->23, 1->123}
    uint8_t  crc       = Opt.radio.gfsk_crc;       // 0...2 {off, 1 byte, 2 bytes}
    uint8_t  whitening = Opt.radio.gfsk_whitening; // 0-disable, 1-enable

    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_GFSK)
    { // change mode to GFSK
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_GFSK);
      if (retv != SX128X_ERR_NONE) return;
    }

    preamble                = (uint16_t)   mrl_str2int(argv[0], preamble,  0);
    if (argc > 1) sw_len    = (uint8_t)    mrl_str2int(argv[1], sw_len,    0);
    if (argc > 2) sw_mode   = (uint8_t)    mrl_str2int(argv[2], sw_mode,   0);
    if (argc > 3) crc       = (uint8_t)    mrl_str2int(argv[3], crc,       0);
    if (argc > 4) whitening = (uint8_t)  !!mrl_str2int(argv[4], whitening, 0);

    retv = sx128x_packet_gfsk(&Radio, preamble, sw_len, sw_mode, crc, whitening,
                              Opt.radio.fixed, Opt.radio.payload_size);

    if (retv != SX128X_ERR_NONE) return;
    print_str("gfsk packet: set");
  }
  else
    print_str("gfsk packet:");

  print_str(" PR=");  print_uint(Opt.radio.gfsk_preamble);
  print_str(" SWL="); print_uint(Opt.radio.gfsk_sw_len);
  print_str(" SWM="); print_uint(Opt.radio.gfsk_sw_mode);
  print_str(" CRC="); print_uint(Opt.radio.gfsk_crc);
  print_str(" W=");   print_uint(Opt.radio.gfsk_whitening);
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_gfsk_swt(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // gfsk swt [0...15]
  int8_t retv;
  uint8_t tolerance = sx128x_get_swt_gfsk(&Radio);
  if (argc > 0)
  {
    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_GFSK)
    { // change mode to GFSK
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_GFSK);
      if (retv != SX128X_ERR_NONE) return;
    }

    tolerance = mrl_str2int(argv[0], tolerance, 0);
    retv = sx128x_set_swt_gfsk(&Radio, tolerance);
    if (retv != SX128X_ERR_NONE) return;

    tolerance = sx128x_get_swt_gfsk(&Radio);
    print_str("set ");
  }
  print_ival("gfsk_swt=", (int) tolerance);
}
//-----------------------------------------------------------------------------
// convert hex string like "01:A5:E0:11:7C" to byte array
int hex2bytes(const char *hex, uint8_t *bytes, int num)
{
  int i;
  for (i = 0; i < num; i++) bytes[i] = 0; // fill zero by default

  for (i = 0; i < num;)
  {
    uint8_t b;
    char c;
    c = *hex++;
    if      (c >= '0' && c <= '9') b = c - '0';
    else if (c >= 'a' && c <= 'f') b = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') b = c - 'A' + 10;
    else break;
    b <<= 4;
    c = *hex++;
    if      (c >= '0' && c <= '9') b |= c - '0';
    else if (c >= 'a' && c <= 'f') b |= c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') b |= c - 'A' + 10;
    else break;
    *bytes++ = b;
    i++;
    c = *hex++;
    if (c != ':' && c != '.' && c != '-' &&
        c != ';' && c != ',' && c != '_') break;
  }

  return i;
}
//-----------------------------------------------------------------------------
void cli_gfsk_sw(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // gfsk [SW1 SW2 SW3]
  int i;
  int8_t retv;
  if (argc > 0)
  {
    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_GFSK)
    { // change mode to GFSK
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_GFSK);
      if (retv != SX128X_ERR_NONE) return;
    }

    for (i = 0; i < argc; i++)
    {
      uint8_t sw[5];
      hex2bytes(argv[i], sw, sizeof(sw));
      retv = sx128x_set_sw_gfsk(&Radio, i, sw);
      if (retv != SX128X_ERR_NONE) return;
    }
  }

  for (i = 0; i < 3; i++)
  {
    print_str("gfsk_SW"); print_int(i+1); print_str("=");
    print_hex(Opt.radio.gfsk_sw[i][0], 2); print_chr(':');
    print_hex(Opt.radio.gfsk_sw[i][1], 2); print_chr(':');
    print_hex(Opt.radio.gfsk_sw[i][2], 2); print_chr(':');
    print_hex(Opt.radio.gfsk_sw[i][3], 2); print_chr(':');
    print_hex(Opt.radio.gfsk_sw[i][4], 2); print_eol();
  }
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_GFSK
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_BLE
//-----------------------------------------------------------------------------
void cli_ble(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // ble [ST TST CRC W]
  if (argc > 0)
  { // set BLE modulation/packet params
    int8_t retv;
    uint8_t state     = Opt.radio.ble_state;     // 0...3
    uint8_t test      = Opt.radio.ble_test;      // 0...7
    uint8_t crc       = Opt.radio.ble_crc;       // 0-off, 1-on
    uint8_t whitening = Opt.radio.ble_whitening; // 0-disable, 1-enable

    uint8_t mode = sx128x_get_mode(&Radio);

    if (mode != SX128X_PACKET_TYPE_BLE)
    { // change mode to BLE
      retv = sx128x_mode(&Radio, SX128X_PACKET_TYPE_BLE);
      if (retv != SX128X_ERR_NONE) return;
    }

    state                   = (uint8_t)   mrl_str2int(argv[0], state,     0);
    if (argc > 1) test      = (uint8_t)   mrl_str2int(argv[1], test,      0);
    if (argc > 2) crc       = (uint8_t)   mrl_str2int(argv[2], crc,       0);
    if (argc > 3) whitening = (uint8_t) !!mrl_str2int(argv[3], whitening, 0);

    // set modulation params in BLE mode
    retv = sx128x_mod_ble(&Radio);
    if (retv != SX128X_ERR_NONE) return;

    // set packet params in BLE mode
    retv = sx128x_packet_ble(&Radio, state, test, crc, whitening);
    if (retv != SX128X_ERR_NONE) return;
  }

  print_str(" ST=");  print_uint(Opt.radio.ble_state);
  print_str(" TST="); print_uint(Opt.radio.ble_test);
  print_str(" CRC="); print_uint(Opt.radio.ble_crc);
  print_str(" W=");   print_uint(Opt.radio.ble_whitening);
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_ble_auto_tx(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // ble auto_tx
  int8_t retv;
  uint16_t time = 0; // auto TX off by default
  if (argc > 0) time = (uint16_t) mrl_str2int(argv[0], time, 0);
  
  retv = sx128x_auto_tx_ble(&Radio, time);
  if (retv != SX128X_ERR_NONE) return;
  
  print_uval("set auto tx: time=", time);
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_BLE
//-----------------------------------------------------------------------------
void cli_buffer_base(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // buffer base TxAd RxAd
  if (argc == 2)
  {
    uint8_t tx_base_addr = (uint8_t) mrl_str2int(argv[0], 0, 10);
    uint8_t rx_base_addr = (uint8_t) mrl_str2int(argv[1], 0, 10);

    int8_t retv = sx128x_set_buffer(&Radio, tx_base_addr, rx_base_addr);
    if (retv != SX128X_ERR_NONE) return;

    print_str("base: TxAddr="); print_uint(tx_base_addr);
    print_str(" RxAddr=");      print_uint(rx_base_addr);
    print_eol();
  }
}
//-----------------------------------------------------------------------------
void cli_buffer_read(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // buffer read Ad [Num]
  uint8_t offset = 0, *data;
  uint16_t count = 1;
  if (argc > 0) offset = (uint8_t)  mrl_str2int(argv[0], 0, 10);
  if (argc > 1) count  = (uint16_t) mrl_str2int(argv[1], 1, 10);

  data = (uint8_t*) malloc(count * sizeof (uint8_t));
  if (data != (uint8_t*) NULL)
  {
    int i;
    int8_t retv = sx128x_buf_read(&Radio, offset, data, count);
    if (retv == SX128X_ERR_NONE)
    {
      print_str("buffer read:\r\n");
      for (i = 0; i < count; i++)
      {
        print_str(" 0x");
        print_hex(((offset + i) & 0xFF), 2);
        print_hval("=>0x", data[i], 2);
      }
    }
    free((void*) data);
  }
}
//-----------------------------------------------------------------------------
void cli_buffer_write(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // buffer write Ad [b0 b1..]
  if (argc >= 2)
  {
    uint16_t i, count = argc - 1;
    uint8_t offset = (uint8_t) mrl_str2int(argv[0], 0, 10);
    uint8_t *data = (uint8_t*) malloc(count * sizeof(uint8_t));
    if (data != (uint8_t*) NULL)
    {
      int8_t retv;
      for (i = 0; i < count; i++)
        data[i] = (uint8_t) mrl_str2int(argv[i + 1], 0, 0);

      retv = sx128x_buf_write(&Radio, offset, data, count);
      if (retv == SX128X_ERR_NONE)
      {
        print_uval("buffer write: count=", count);
      }
      free((void*) data);
    }
  }
}
//-----------------------------------------------------------------------------
void cli_tx_timeout(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // tx_timeout [ms]
  if (argc > 0) Opt.tx_timeout = mrl_str2int(argv[0], 0, 0);
  print_ival("tx_timeout=", Opt.tx_timeout);
}
//-----------------------------------------------------------------------------
void cli_fixed(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // fixed [0|1]
  if (argc > 0) Opt.radio.fixed = !!mrl_str2int(argv[0], 0, 10);
  print_ival("fixed: ", Opt.radio.fixed);
}
//-----------------------------------------------------------------------------
void cli_data(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // data [b0 b1..]
  int i;
  if (argc)
  { // set TX data
    if (argc > OPT_DATA_SIZE) argc = OPT_DATA_SIZE;
    for (i = 0; i < argc; i++)
      Opt.data[i] = (char) mrl_str2int(argv[i], 0, 0);
    Opt.data_size = argc;
    Opt.radio.payload_size = argc;
  }

  print_str("data:");
  for (i = 0; i < Opt.data_size; i++)
  {
    print_str(" 0x");
    print_hex(Opt.data[i], 2);
  }

  print_ival(" size=", Opt.data_size);
}
//-----------------------------------------------------------------------------
void cli_data_fill(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // data fill [size value]
  int i, value = 0;
  if (argc > 0) Opt.data_size = mrl_str2int(argv[0], Opt.data_size, 10) & 0xFF;
  if (argc > 1) value         = mrl_str2int(argv[1], 0,             10) & 0xFF;
  if (Opt.data_size > OPT_DATA_SIZE) Opt.data_size = OPT_DATA_SIZE;
  Opt.radio.payload_size = Opt.data_size;
  for (i = 0; i < Opt.data_size; i++) Opt.data[i] = value;
  print_str("data: size=");
  print_int(Opt.data_size);
  print_hval(" value=0x", value, 2);
}
//-----------------------------------------------------------------------------
void cli_data_size(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // data size [size]
  if (argc > 0) Opt.data_size = mrl_str2int(argv[0], Opt.data_size, 10) & 0xFF;
  if (Opt.data_size > OPT_DATA_SIZE) Opt.data_size = OPT_DATA_SIZE;
  Opt.radio.payload_size = Opt.data_size;
  print_uval("data size=", Opt.data_size);
}
//-----------------------------------------------------------------------------
void cli_code(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // code [101001]
  if (argc > 0)
  { // set OOK code
    int i, size = strlen(argv[0]);
    if (size > sizeof(Opt.code) - 1) size = sizeof(Opt.code) - 1; 
    memcpy((void*) Opt.code, (const void*) argv[0], size);
    Opt.code[size] = '\0';
    Opt.code_size = size;
    for (i = 0; i < size; i++) if (Opt.code[i] != '0') Opt.code[i] = '1';
  }
  print_str("code: ");
  print_str(Opt.code);
  print_str("\r\n");
}
//-----------------------------------------------------------------------------
void cli_status(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // status
  int8_t retv;
  uint8_t mode = sx128x_get_mode(&Radio);

#if  defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
  if (mode == SX128X_PACKET_TYPE_LORA ||
      mode == SX128X_PACKET_TYPE_RANGING)
  { // LoRa or Ranging
    uint8_t status = 0, mode, stat;
    uint8_t rssi = 0, rssi_inst = 0;
    int8_t snr = 0;
    int32_t fei = 0;
    sx128x_lora_hdr_t hdr;

    retv = sx128x_packet_status_lora(&Radio, &status, &rssi, &snr);
    if (retv != SX128X_ERR_NONE) return;

    sx128x_status_unpack(status, &mode, &stat);

    retv = sx128x_get_hdr_lora(&Radio, &hdr);
    if (retv != SX128X_ERR_NONE) return;

    retv = sx128x_rssi_lora(&Radio, &rssi_inst);
    if (retv != SX128X_ERR_NONE) return;

    retv = sx128x_fei_lora(&Radio, &fei);
    if (retv != SX128X_ERR_NONE) return;

    print_str("packet status LoRa/Ranging: Status=0x"); print_hex(status, 2);
#ifdef SX128X_USE_EXTRA
    print_str(" (ChipMode="); print_str(sx128x_status_mode_string[mode]);
    print_str(" CmdStatus="); print_str(sx128x_status_cmd_string[stat]);
#else
    print_str(" (ChipMode="); print_uint((unsigned) mode);
    print_str(" CmdStatus="); print_uint((unsigned) stat);
#endif
    print_str("):\r\n");

    if (hdr.fixed)
    { // Implicit header
      print_str("  HeaderType=Implicit");
    }
    else
    { // Explicit header
      print_str("  HeaderType=Explicit: CR="); print_int((int) hdr.cr);
      print_str(" CRC="); print_str(hdr.crc ? "On" : "Off");
    }
    print_eol();
    print_str("  RSSI=");      print_rssi(rssi);      print_str("dB\r\n");
    print_str("  RSSI_inst="); print_rssi(rssi_inst); print_str("dB\r\n");
    print_str("  SNR=");       print_snr(snr);        print_str("dB\r\n");
    print_str("  FEI=");       print_fei(fei);        print_str("kHz\r\n");
  }
#endif // SX128X_USE_LORA || SX128X_USE_RANGING

#if defined(SX128X_USE_FLRC) || defined(SX128X_USE_GFSK) || defined(SX128X_USE_BLE)
  if (mode == SX128X_PACKET_TYPE_FLRC ||
      mode == SX128X_PACKET_TYPE_GFSK ||
      mode == SX128X_PACKET_TYPE_BLE)
  { // GFSK/FLRC/BLE
    uint8_t status = 0, mode, stat;
    uint8_t rssi = 0;
    uint8_t pkt_status; // status packet status (Table 11-67)
    uint8_t pkt_errors; // error packet status  (Table 11-68)
    uint8_t pkt_sync;   // sync packet status   (Table 11-69)

    retv = sx128x_packet_status(&Radio, &status, &rssi,
                                &pkt_status, &pkt_errors, &pkt_sync);
    if (retv != SX128X_ERR_NONE) return;

    sx128x_status_unpack(status, &mode, &stat);

    print_str("packet status GFSK/FLRC/BLE: Status=0x"); print_hex(status, 2);
#ifdef SX128X_USE_EXTRA
    print_str(" (ChipMode="); print_str(sx128x_status_mode_string[mode]);
    print_str(" CmdStatus="); print_str(sx128x_status_cmd_string[stat]);
#else
    print_str(" (ChipMode="); print_uint((unsigned) mode);
    print_str(" CmdStatus="); print_uint((unsigned) stat);
#endif
    print_str("):\r\n");

    print_ival("  PktSent=", (int) (pkt_status >> 0) & 0x1); // Table 11-67
    print_ival("  rxNoAck=", (int) (pkt_status >> 5) & 0x1); // (page 94)

    print_hval("  pktErrors=0x", pkt_errors, 2); // Table 11-68, page 94
    print_ival("    pktCtrlBusy=", (int) (pkt_errors >> 0) & 0x1);
    print_ival("        pktRecv=", (int) (pkt_errors >> 1) & 0x1);
    print_ival("        hdrRecv=", (int) (pkt_errors >> 2) & 0x1);
    print_ival("       AbortErr=", (int) (pkt_errors >> 3) & 0x1);
    print_ival("         crcErr=", (int) (pkt_errors >> 4) & 0x1);
    print_ival("         LenErr=", (int) (pkt_errors >> 5) & 0x1);
    print_ival("        SyncErr=", (int) (pkt_errors >> 6) & 0x1);

    print_bval("  sync_addrs=0b",  (int) (pkt_sync & 0x07), 3); // Table 11-69

    print_str("  RSSI=");  print_rssi(rssi);  print_str("dB\r\n");
  }
#endif // SX128X_USE_FLRC || SX128X_USE_GFSK || SX128X_USE_BLE
}
//-----------------------------------------------------------------------------
void cli_send(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // send [to]
  int8_t retv;
  uint32_t timeout = Opt.tx_timeout;
  if (argc > 0) timeout = mrl_str2int(argv[0], timeout, 0);

  retv = sx128x_send(&Radio,
                     (const uint8_t*) Opt.data, Opt.data_size,
                     Opt.radio.fixed, timeout, SX128X_TIME_BASE_1MS);
  if (retv != SX128X_ERR_NONE) return;

  Fsm.tx_start(TIME_FUNC());

  print_str("send: size="); print_uint(Opt.data_size);
  print_str(" fixed=");     print_uint(Opt.radio.fixed);
  print_str(" timeout=");   print_uint(timeout);
  print_str("ms\r\n");
  print_flush();
}
//-----------------------------------------------------------------------------
void cli_recv(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // recv [size to]
  int8_t retv;
  int size = Opt.radio.fixed ? Opt.data_size : 0;
  uint32_t timeout = SX128X_RX_TIMEOUT_CONTINUOUS;
  if (argc > 0) size    =            mrl_str2int(argv[0], 1, 0);
  if (argc > 1) timeout = (uint32_t) mrl_str2int(argv[1], timeout, 0);
  if      (size < 0)   size = 0;
  else if (size > 255) size = 255;

  retv = sx128x_recv(&Radio, (uint8_t) size, Opt.radio.fixed,
                     timeout, SX128X_TIME_BASE_1MS);
  if (retv != SX128X_ERR_NONE) return;
  
  print_str("recv: ready to receive size="); print_uint(size);
  print_str(" timeout=");                    print_uint(timeout);
  print_str("ms fixed=");                    print_uint(Opt.radio.fixed);
  print_eol();
  print_flush();
}
//=============================================================================
void cli_mode(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // mode [0..9]
  if (argc > 0) {
    print_str("set ");
    Opt.fsm.mode = (uint8_t) LIMIT(mrl_str2int(argv[0], 0, 10), 0, AFSM_MODES-1);
  }

  print_str("mode=");
  print_uint(Opt.fsm.mode);

  if (Opt.verbose)
  {
    print_str(" (");
    print_str(afsm_mode_string[Opt.fsm.mode]);
    print_str(")\r\nmodes: " AFSM_MODE_HELP);
  }
  print_eol();
}
//-----------------------------------------------------------------------------
void cli_fsm(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // fsm [T dT dC WUT]
  if (argc > 0) Opt.fsm.t   = mrl_str2int(argv[0], Opt.fsm.t,   10);
  if (argc > 1) Opt.fsm.dt  = mrl_str2int(argv[1], Opt.fsm.dt,  10);
  if (argc > 2) Opt.fsm.dc  = mrl_str2int(argv[2], Opt.fsm.dc,  10);
  if (argc > 4) Opt.fsm.wut = mrl_str2int(argv[4], Opt.fsm.wut, 10);

  print_str("fsm: t="); print_uint(Opt.fsm.t);
  print_str("ms dt=");  print_uint(Opt.fsm.dt);
  print_str("ms dc=");  print_uint(Opt.fsm.dc);
  print_str("ms wut="); print_uint(Opt.fsm.wut);
  print_str("ms\r\n");
}
//-----------------------------------------------------------------------------
void cli_sweep(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // sweep [Fmin[kHz] Fmax[kHz] S[kHz/sec]]
  if (argc > 0) Opt.fsm.sweep_min = mrl_str2int(argv[0], AFSM_SWEEP_MIN, 10);
  if (argc > 1) Opt.fsm.sweep_max = mrl_str2int(argv[1], AFSM_SWEEP_MAX, 10);
  if (argc > 2) Opt.fsm.sweep_f   = mrl_str2int(argv[2], AFSM_SWEEP_F,   10);

  print_str("sweep: Fmin="); print_uint(Opt.fsm.sweep_min);
  print_str("kHz Fmax=");    print_uint(Opt.fsm.sweep_max);
  print_str("kHz S=");       print_int( Opt.fsm.sweep_f);
  print_str("kHz/sec\r\n");
}
//-----------------------------------------------------------------------------
void cli_start(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // start
  Fsm.start();
}
//-----------------------------------------------------------------------------
void cli_stop(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // stop
  Autostart = 0;
  Fsm.stop();
}
//-----------------------------------------------------------------------------
void cli_autostart(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // autostart [1|0 delay]
  if (argc > 0) Opt.autostart = !!mrl_str2int(argv[0], 0,         10);
  if (argc > 1) Opt.delay     =   mrl_str2int(argv[1], Opt.delay, 10);

  print_str("autostart=");
  print_uint(Opt.autostart);
  print_str(" delay=");
  print_uint(Opt.delay);
  print_str("sec\r\n");
}
//=============================================================================
void cli_wifi_ssid(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // wifi ssid [SSID]
  if (argc > 0) strncpy(Opt.wifi_ssid, argv[0], OPT_WIFI - 1);
  print_sval("SSID=", Opt.wifi_ssid);
}
//-----------------------------------------------------------------------------
void cli_wifi_passwd(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // wifi passwd [PASSWD]
  if (argc > 0) strncpy(Opt.wifi_passwd, argv[0], OPT_WIFI - 1);
  print_sval("passwd=", Opt.wifi_passwd);
}
//-----------------------------------------------------------------------------
void cli_wifi_disable(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // wifi disable
  strncpy(Opt.wifi_ssid,   "", OPT_WIFI - 1);
  //strncpy(Opt.wifi_passwd, "", OPT_WIFI - 1);
}
//-----------------------------------------------------------------------------
void cli_wifi_connect(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // wifo connect
  wifi_connect(Opt.wifi_ssid, Opt.wifi_passwd, true); // auto_reconnect=true
}
//-----------------------------------------------------------------------------
void cli_wifi_disconnect(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // wifi disconnect
  wifi_disconnect();
}
//-----------------------------------------------------------------------------
void cli_wifi_status(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // wifi status
  wifi_status_print();
}
//=============================================================================
void cli_mqtt_server(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // mqtt server [HOST PORT ID]
  if (argc > 0) strncpy(Opt.mqtt_host, argv[0], OPT_MQTT - 1);
  if (argc > 1) Opt.mqtt_port = mrl_str2int(argv[1], 8883, 10);
  if (argc > 2) Opt.mqtt_id   = mrl_str2int(argv[2], 1, 0);
  
  print_sval("MQTT host: ",      Opt.mqtt_host);
  print_uval("MQTT port: ",      Opt.mqtt_port);
  print_uval("MQTT client ID: ", Opt.mqtt_id);
}
//-----------------------------------------------------------------------------
void cli_mqtt_client(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // mqtt server [USER KEY]
  if (argc > 0) strncpy(Opt.mqtt_user, argv[0], OPT_MQTT - 1);
  if (argc > 1) strncpy(Opt.mqtt_key,  argv[1], OPT_MQTT - 1);
  
  print_sval("MQTT user: ", Opt.mqtt_user);
  print_sval("MQTT key: ",  Opt.mqtt_key);
}
//-----------------------------------------------------------------------------
void cli_mqtt_connect(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // mqtt connect
  mqtt_connect(Opt.mqtt_host, Opt.mqtt_port,
               Opt.mqtt_user, Opt.mqtt_key);
}
//-----------------------------------------------------------------------------
void cli_mqtt_disconnect(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // mqtt disconnect
  mqtt_disconnect();
}
//-----------------------------------------------------------------------------
void cli_mqtt_status(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // mqtt status
  print_sval("MQTT ", mqtt_connected() ? "connected" : "disconnected");
}
//-----------------------------------------------------------------------------
void cli_mqtt_ping(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // mqtt ping [N]
  int n = 1;
  bool ret = false;
  if (argc > 0) n = mrl_str2int(argv[0], 1, 10);
  if (Mqtt != (Adafruit_MQTT_Client*) NULL)
    ret = Mqtt->ping(n);
  print_sval("MQTT ping ", ret ? "SUCCESS" : "FAIL");
}
//-----------------------------------------------------------------------------
void cli_mqtt_pub(int argc, char* const argv[], const cli_cmd_t *cmd)
{ // mqtt pub [Topic MSG QoS]
  if (argc >= 2)
  {
    bool ret = false;
    uint8_t qos = 0; // QoS by default
    const char *topic = argv[0];
    const char *msg   = argv[1];
    if (argc >= 3) qos = mrl_str2int(argv[2], qos, 0);
    if (Mqtt != (Adafruit_MQTT_Client*) NULL)
      ret = Mqtt->publish(topic, msg, qos, true); // retain=true
    print_sval("MQTT publish ", ret ? "SUCCESS" : "FAIL");
  }
}
//=============================================================================
#ifdef MRL_USE_CTRL_C
// Ctrl+C callback
static void cli_sigint_cb()
{
  print_str("\r\n^C\r\n");
  print_str("stop\r\n");
  cli_stop(0, NULL, NULL);
  mrl_prompt(&Mrl);
}
#endif // MRL_USE_CTRL_C
//-----------------------------------------------------------------------------
void cli_init()
{
  // init command parser
  mrl_init(&Mrl, cli_print);
  mrl_set_execute_cb(&Mrl, cli_execute_cb);
  mrl_set_prompt(&Mrl, cli_prompt, cli_prompt_len);

#if defined(MRL_USE_COMPLETE)
  mrl_set_complete_cb(&Mrl, cli_complete_cb); // set callback for completion
#endif // MRL_USE_COMPLETE

#ifdef MRL_USE_CTRL_C
  mrl_set_sigint_cb(&Mrl, cli_sigint_cb);
#endif // MRL_USE_CTRL_C

  // clear input buffer
  while (cli_inkey() != -1);

  // show help
  print_str("Press Ctrl+X for help\r\n");

  // show prompt
  mrl_prompt(&Mrl);
}
//-----------------------------------------------------------------------------
void cli_loop()
{
  int key;

  while ((key = cli_inkey()) != -1)
    if ((key = mrl_insert_char(&Mrl, (char) key)) != 0)
      break;

  // check special keys
  if (key == CLI_KEYCODE_CTRL_S) // Ctrl+S pressed
  {
    print_str("\r\n^S\r\n");
    print_str("start\r\n");
    Fsm.start();
    mrl_refresh(&Mrl);
  }
#ifndef MRL_USE_CTRL_C
  else if (key == CLI_KEYCODE_CTRL_C) // Ctrl+C pressed
  {
    print_str("\r\n^C\r\n");
    print_str("stop\r\n");
    cli_stop(0, NULL, NULL);
    mrl_prompt(&Mrl);
  }
#endif // !MRL_USE_CTRL_C
  else if (key == CLI_KEYCODE_CTRL_T) // Ctrl+T pressed
  {
    print_str("\r\n^T\r\n");
    cli_radio_restore(0, NULL, NULL);
    mrl_refresh(&Mrl);
  }
  else if (key == CLI_KEYCODE_CTRL_D) // Ctrl+D pressed
  {
    print_str("\r\n^D\r\n");
    cli_radio_standby(0, NULL, NULL);
    mrl_refresh(&Mrl);
  }
  else if (key == CLI_KEYCODE_CTRL_Z) // Ctrl+Z pressed
  {
    print_str("\r\n^Z\r\n");
    cli_radio_sleep(0, NULL, NULL);
    mrl_refresh(&Mrl);
  }
  else if (key == CLI_KEYCODE_CTRL_V) // Ctrl+V pressed
  {
    print_str("\r\n^V\r\n");
    //...
    mrl_refresh(&Mrl);
  }
  else if (key == CLI_KEYCODE_CTRL_Y) // Ctrl+Y pressed
  {
    print_str("\r\n^Y\r\n");
    cli_radio_wave(0, NULL, NULL);
    mrl_refresh(&Mrl);
  }
  else if (key == CLI_KEYCODE_CTRL_Q) // Ctrl+Q pressed
  {
    print_str("\r\n^Q\r\n");
    Reset();
    //mrl_refresh(&Mrl);
  }
  else if (key == CLI_KEYCODE_CTRL_W) // Ctrl+W pressed
  {
    print_str("\r\n^W\r\n");
    cli_eeprom_write(0, NULL, NULL);
    mrl_refresh(&Mrl);
  }
#ifdef CLI_HELP
  else if (key == CLI_KEYCODE_CTRL_X) // Ctrl+X pressed
  {
    print_str("\r\n^X\r\n");
    cli_help(0, NULL, &cli_tree[0]);
    mrl_refresh(&Mrl);
  }
#endif // CLI_HELP
}
//-----------------------------------------------------------------------------
// all commands and options tree
#include "cli_tree.h"
//-----------------------------------------------------------------------------

/*** end of "cli.c" file ***/


