/*
 * File: "global.h"
 */

#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H
//-----------------------------------------------------------------------------
#include <stdint.h>
#include "config.h"
#include "ablink.h"
#include "aticker.h"
#include "tfs.h"
#include "mrl.h"
#include "sx128x.h"
#include "opt.h"
//-----------------------------------------------------------------------------
extern int Verbose;         // verbose level (0, 1, 2 or 3)
extern ABlink Led;          // LED blinker
extern uint8_t Button;      // onboart button EN state {0|1}
extern ATicker Ticker;      // periodical ticker
extern unsigned long Ticks; // tick counter
extern tfs_t Tfs;           // Trivial EEPROM "File" System
extern mrl_t Mrl;           // MicroRL object
extern sx128x_t Radio;      // SX128x object
extern opt_t Opt;           // All options (saved to FLASH)
//-----------------------------------------------------------------------------
#endif // GLOBAL_H

/*** end of "global.h" file ***/


