/*
 * File: "global.cpp"
 */

//-----------------------------------------------------------------------------
#include "global.h"
//-----------------------------------------------------------------------------
int Verbose = VERBOSE;   // verbose level (0, 1, 2 or 3)
ABlink Led;              // LED blinker
uint8_t Button;          // onboart button EN state {0|1}
ATicker Ticker;          // periodical ticker
unsigned long Ticks = 0; // tick counter
tfs_t Tfs;               // Trivial EEPROM "File" System
mrl_t Mrl;               // MicroRL object
sx128x_t Radio;          // SX128x object
opt_t Opt;               // All options (saved to FLASH)
//-----------------------------------------------------------------------------

/*** end of "global.cpp" file ***/


