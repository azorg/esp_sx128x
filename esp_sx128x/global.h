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
#include "sx128x_hw_arduino.h"
#include "opt.h"
#include "afsm.h"
//-----------------------------------------------------------------------------
#ifndef INLINE
#  define INLINE static inline
#endif
//-----------------------------------------------------------------------------
extern ABlink Led;          // LED blinker
extern uint8_t Button;      // onboart button EN state {0|1}
extern ATicker Ticker;      // periodical ticker
extern unsigned long Ticks; // tick counter
extern uint32_t Seconds;    // seconds from start
extern tfs_t Tfs;           // Trivial EEPROM "File" System
extern mrl_t Mrl;           // MicroRL object
extern sx128x_t Radio;      // SX128x object
extern opt_t Opt;           // All options (saved to EEPROM)
extern uint8_t RXEN;        // RXEN state {0|1}
extern uint8_t TXEN;        // TXEN state {0|1}
extern AFsm Fsm;            // FSM
extern uint8_t Autostart;   // auto start flag

extern unsigned long T_tx_start;  // TX start time
extern unsigned long T_tx_done;   // TX done time
extern unsigned long T_rx_done;   // RX done time
extern unsigned long T_rx_done_p; // RX done time (previous)
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
INLINE void setRXEN(uint8_t rxen) { sx128x_hw_rxen(RXEN = rxen, NULL); }
INLINE void setTXEN(uint8_t txen) { sx128x_hw_txen(TXEN = txen, NULL); }
//-----------------------------------------------------------------------------
// print SX128x RSSI [dBm]
void print_rssi(uint8_t rssi);

// print SX128x SNR [dB]
void print_snr(int8_t snr);

// print SX128x FEI [kHz]
void print_fei(int32_t fei);

// print Distance [dm -> m]
void print_distance(int32_t dist);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // GLOBAL_H

/*** end of "global.h" file ***/


