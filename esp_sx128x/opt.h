/*
 * File: "opt.h"
 */
#pragma once
#ifndef OPT_H
#define OPT_H
//-----------------------------------------------------------------------------
#include "sx128x.h"
#include "tfs.h"
//----------------------------------------------------------------------------
#ifndef OPT_DATA_SIZE
#  define OPT_DATA_SIZE 64
#endif
#define OPT_DATA_DEFAULT { 0x01, 0x02, 0x03, 0x04 }
//-----------------------------------------------------------------------------
// all options (saved to / restore from FLASH)
typedef struct {
  uint8_t verbose;          // verbose level (0, 1, 2 or 3)
  uint8_t rxen;             // RXEN state {0|1}
  uint8_t txen;             // TXEN state {0|1}
  sx128x_pars_t radio;      // radio options of SX128x
  char data[OPT_DATA_SIZE]; // packet data
  uint8_t data_size;        // RX/TX packet data size (bytes)
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

