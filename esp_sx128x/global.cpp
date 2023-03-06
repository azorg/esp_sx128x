/*
 * File: "global.cpp"
 */

//-----------------------------------------------------------------------------
#include "global.h"
#include "print.h"
//-----------------------------------------------------------------------------
ABlink Led;              // LED blinker
uint8_t Button;          // onboart button EN state {0|1}
ATicker Ticker;          // periodical ticker
unsigned long Ticks = 0; // tick counter
uint32_t Seconds = 0;    // secondr from start
tfs_t Tfs;               // Trivial EEPROM "File" System
mrl_t Mrl;               // MicroRL object
sx128x_t Radio;          // SX128x object
opt_t Opt;               // All options (saved to FLASH)
uint8_t RXEN;            // RXEN state {0|1}
uint8_t TXEN;            // TXEN state {0|1}
AFsm Fsm;                // FSM
uint8_t Autostart = 0;   // auto start flag
//-----------------------------------------------------------------------------
// print SX128x RSSI [dBm]
void print_rssi(uint8_t rssi)
{
  static char const *d[] = {".0", ".5"};
  print_chr('-');
  print_int(rssi >> 1);
  print_str(d[rssi & 1]);
}
//-----------------------------------------------------------------------------
// print SX128x SNR [dB]
void print_snr(int8_t snr)
{
  static char const *q[] = {".00", ".25", ".50", ".75"};
  if (snr < 0)
  {
    snr = -snr;
    print_chr('-');
  }
  print_int(snr >> 2);
  print_str(q[snr & 3]);
}
//-----------------------------------------------------------------------------
// print SX128x FEI [kHz]
void print_fei(int32_t fei)
{
  if (fei < 0)
  {
    fei = -fei;
    print_chr('-');
  }
  print_uint(((unsigned) fei) / 1000); // kHz
  print_chr('.');
  print_uint_ex(((unsigned) fei) % 1000, 3); // Hz
}
//-----------------------------------------------------------------------------
// print Distance [dm -> m]
void print_distance(int32_t dist)
{
  if (dist < 0)
  {
    dist = -dist;
    print_chr('-');
  }
  print_uint(((unsigned) dist) / 10); // m
  print_chr('.');
  print_uint_ex(((unsigned) dist) % 10, 1); // dm
}
//-----------------------------------------------------------------------------

/*** end of "global.cpp" file ***/


