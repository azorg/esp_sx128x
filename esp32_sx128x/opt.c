/*
 * File: "opt.c"
 */
//-----------------------------------------------------------------------------
#include <string.h>
#include "opt.h"
//#include "print.h"
//-----------------------------------------------------------------------------
// set to default all options
void opt_default(opt_t *opt)
{
  const uint8_t data[] = OPT_DATA_DEFAULT;

  opt->rxen = opt->txen = 0; // RXEN and TXEN by deafault OFF

  // SX128x pars
  memcpy((void*) &opt->radio, (const void*) &sx128x_pars_default,
         sizeof(sx128x_pars_t));

  // TX/RX packet data
  opt->data_size = sizeof(data);
  memcpy((void*) opt->data, (const void*) data, sizeof(data));
}
//-----------------------------------------------------------------------------
// restore options from TFS
#if 0
void opt_read_from_flash(opt_t *opt, tfs_t *tfs)
{
  uint16_t size, cnt, retv;

#ifdef DEBUG
  print_str("try to read config from FLASH\r\n");
  print_flush();
#endif

  opt_default(opt);
  retv = tfs_read(tfs,
                  (void*) opt, sizeof(opt_t),
                  &size, &cnt);
  if ((retv & ~TFS_ERR_DELETED) == TFS_SUCCESS && size == sizeof(opt_t))
  {
#ifdef DEBUG
    print_str("read config from FLASH success\r\n");
    print_flush();
#endif
  }
  else
  {
#ifdef DEBUG
    print_str("read config from FLASH fail\r\n");
    print_str("erase FLASH config area\r\n");
    print_flush();
#endif

    tfs_erase(tfs);
    opt_default(opt);

#ifdef DEBUG
    print_str("write default options to FLASH\r\n");
    print_flush();
#endif

    tfs_write(tfs, (const void*) opt, sizeof(opt_t));

#ifdef DEBUG
    print_str("write FLASH success\r\n");
    print_flush();
#endif
  }
}
#endif
//-----------------------------------------------------------------------------

/*** end of "opt.c" file ***/

