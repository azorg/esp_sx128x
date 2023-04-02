/*
 * File: "opt.c"
 */
//-----------------------------------------------------------------------------
#include <string.h>
#include "config.h"
#include "opt.h"
#include "afsm.h"
#include "print.h"
//-----------------------------------------------------------------------------
// set to default all options
void opt_default(opt_t *opt)
{
  const uint8_t data[] = OPT_DATA_DEFAULT;
  const char    code[] = OPT_CODE_DEFAULT;

  opt->verbose = VERBOSE;

  strcpy(opt->wifi_ssid,   "WiFi_SSID");
  strcpy(opt->wifi_passwd, "WiFi_passwd");
  
  strcpy(opt->mqtt_host, "mqtt.server");
  strcpy(opt->mqtt_user, "mqtt_user");
  strcpy(opt->mqtt_key,  "1234");
  opt->mqtt_port = 8883;
  opt->mqtt_id   = 1;

  opt->rxen = opt->txen = 0; // RXEN and TXEN by deafault OFF

  // SX128x pars
  memcpy((void*) &opt->radio, (const void*) &sx128x_pars_default,
         sizeof(sx128x_pars_t));
  
  // TX timeout (0 - disable) [ms]
  opt->tx_timeout = 0;

  // TX/RX packet data
  opt->data_size = sizeof(data);
  memcpy((void*) opt->data, (const void*) data, sizeof(data));
  
  // OOK code
  opt->code_size = sizeof(code);
  memcpy((void*) opt->code, (const void*) code, sizeof(code));
  opt->code[sizeof(code)] = '\0';
  
  // FSM options
  memcpy((void*) &opt->fsm, (const void*) &afsm_pars_default,
         sizeof(afsm_pars_t));
  
  opt->autostart = OPT_AUTOSTART; // auto start FSM TX on reboot
  opt->delay = OPT_AUTOSTART_DELAY; // auto start delay [sec]
}
//-----------------------------------------------------------------------------
// restore options from TFS
void opt_read_from_flash(opt_t *opt, tfs_t *tfs)
{
  uint16_t size, cnt, retv;

  opt_default(opt);
  retv = tfs_read(tfs,
                  (void*) opt, sizeof(opt_t),
                  &size, &cnt);

  // FIXME: set default IRQ mask 
  opt->radio.irq_mask = SX128X_IRQ_ALL & (~SX128X_IRQ_PREAMBLE_DETECTED);

  if ((retv & ~TFS_ERR_DELETED) == TFS_SUCCESS && size == sizeof(opt_t))
  {
    print_str("read config from EEPROM success\r\n");
  }
  else
  {
    print_str("read config from EEPROM fail\r\n"
              "erase EEPROM config area\r\n");

    tfs_erase(tfs);
    opt_default(opt);

    print_str("write default options to EEPROM\r\n");

    tfs_write(tfs, (const void*) opt, sizeof(opt_t));
  }
}
//-----------------------------------------------------------------------------

/*** end of "opt.cpp" file ***/

