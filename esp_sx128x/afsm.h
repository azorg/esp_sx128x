/*
 * File: "afsm.h" (Finite-State Machine class)
 */
#pragma once
#ifndef AFSM_H
#define AFSM_H
//-----------------------------------------------------------------------------
#include <stdint.h>
#include "ablink.h"
#include "sx128x.h"
//-----------------------------------------------------------------------------
// sweep generator default parameters (Wi-Fi channel #6 +/- 7 MHz)
#define AFSM_SWEEP_MIN 2430000 // minimal frequency [kHz]
#define AFSM_SWEEP_MAX 2444000 // maximal frequency [kHz]
#define AFSM_SWEEP_F      7000 // sweep factor [kHz/sec]
//-----------------------------------------------------------------------------
typedef enum {
  AFSM_CW = 0, // periodic continuous wave (CW) beeper
  AFSM_OOK,    // periodic on-off keying (OOK) transmitter
  AFSM_TX,     // periodic transmitter (TX)
  AFSM_RX,     // continuous receiver (RX)
  AFSM_RQ,     // periodic requester (RQ)
  AFSM_RP,     // continuous responder (RP)
  AFSM_RM,     // periodic ranging master (RM)
  AFSM_RS,     // continuous ranging slave (RS)
  AFSM_AR,     // continuous advanced ranging (AR)
  AFSM_SG,     // sweep generator (SG)
  AFSM_MODES   // number of FSM modes 
} afsm_mode_t; // 0...AFSM_MODES-1
//-----------------------------------------------------------------------------
#define AFSM_MODE_STRING { \
  "CW - periodic continuous wave beeper",     \
  "OOK - periodic on-off keying transmitter", \
  "TX - periodic transmitter",                \
  "RX - continuous receiver",                 \
  "RQ - periodic requester",                  \
  "RP - continuous responder",                \
  "RM - periodic ranging master",             \
  "RS - continuous ranging slave",            \
  "AR - continuous advanced ranging",         \
  "SG - Sweep Generator" };
//-----------------------------------------------------------------------------
#define AFSM_MODE_HELP "0:CW 1:OOK 2:TX 3:RX 4:RQ 5:RP 6:RM 7:RS 8:AR 9:SW"
//-----------------------------------------------------------------------------
extern const char * const afsm_mode_string[AFSM_MODES];
//-----------------------------------------------------------------------------
// options for FSM
typedef struct {
  uint8_t  mode;  // AFSM_CW, AFSM_OOK, AFSM_TX, AFSM_RX,
                  // AFSM_RQ, AFSM_RP, AFSM_RM, AFSM_RS, AFSM_AD, AFSM_SG

  uint32_t t;     // TX period [ms]
  uint32_t dt;    // CW time [ms]
  uint32_t dc;    // OOK code chip time [ms]
  uint32_t wut;   // radio wakeup time [ms]

  // sweep generator (SG)
  uint32_t sweep_min; // minimal frequency [kHz]
  uint32_t sweep_max; // maximal frequency [kHz]
  int32_t  sweep_f;   // sweep factor [kHz/s]
} afsm_pars_t;
//-----------------------------------------------------------------------------
// FSM default options
extern const afsm_pars_t afsm_pars_default;
//-----------------------------------------------------------------------------
// FSM class
class AFsm {
private:
  // pointers to external (global) objects
  ABlink      *led;       // onboard LED
  afsm_pars_t *pars;      // FSM options
  uint8_t     *data;      // RX/TX packet data
  uint8_t     *data_size; // RX/TX packet data size (bytes)
  uint8_t     *fixed;     // 1-fixed packet size, 0-variable packet size 
  const char  *code;      // CW code (like "100101")
  uint8_t     *code_size; // CW code size (chips) = strlen(code)
  sx128x_t    *radio;     // SX128x object

  // period timer
  uint8_t  _start;  // start period timer command {0|1}
  int8_t   _stop;   // stop period timer command {0|1}
  uint8_t  _run;    // state (0 - initial (TX off), 1 - run (TX on))
  unsigned long _t; // last call for periodic timer

  // TX/RX FSM
  uint8_t txrx_start; // start TX timer {0|1}
  uint8_t txrx;       // TX/RX state {0-RX, 1-TX}
  uint8_t power;      // power 1-on / 0-off
  uint8_t wus;        // wakeup state (wut or 0)
  uint16_t code_cnt;  // code counter 0...code_size-1 
  unsigned long dt;   // CW/chip time (dt or dc) [ms]
  unsigned long t;    // last call for TXRX FSM

  // sweep generator
  uint32_t freq; // current frequency [Hz]
  uint32_t sweep_save; // saved frequency to restore [Hz]

  uint8_t sleep_ready; // ready to sleep flag {0|1}

  unsigned long t_tx_start;  // TX start time
  unsigned long t_tx_done;   // TX done time
  unsigned long t_rx_done;   // RX done time
  unsigned long t_rx_done_p; // RX done time (previous)

  void (*_setRXEN)(uint8_t rxen); // set RXEN or NULL
  void (*_setTXEN)(uint8_t txen); // set TXEN or NULL

  // TX wave ON/OFF
  int8_t wave(uint8_t on) {
    int8_t retv = on ? sx128x_tx_wave(radio) :
                       sx128x_standby(radio, SX128X_STANDBY_RC);
    led->set(power = (retv == SX128X_ERR_NONE) ? on : 0);
    return retv;
  }

  // finish TX => radio sleep
  int8_t sleep() {
    int8_t retv = sx128x_sleep(radio, SX128X_SLEEP_OFF_RETENTION);
    led->set(txrx = power = 0);
    sleep_ready = 1;
    return retv;
  }

  // wakeup radio
  int8_t wakeup() {
    int8_t retv = sx128x_wakeup(radio, SX128X_STANDBY_XOSC);
    led->off();
    if (retv == SX128X_ERR_NONE) retv = sx128x_set_pars(radio, NULL); // restore
    if (retv == SX128X_ERR_NONE) sleep_ready = 0;
    return retv;
  }

  void start_fsm(unsigned long t); // form txrx_start
  void txrx_fsm(unsigned long t);  // TX/RX FSM after txrx_start

public:
  // set RXEN
  void setRXEN(uint8_t rxen) {
    if (_setRXEN != (void (*)(uint8_t)) NULL) _setRXEN(rxen);
  }

  // set TXEN
  void setTXEN(uint8_t txen) {
    if (_setTXEN != (void (*)(uint8_t)) NULL) _setTXEN(txen);
  }
  
  // call from setup(): init FSM
  void begin(ABlink      *led,          // on board LED
             afsm_pars_t *pars,         // FSM options
             uint8_t     *data,         // RX/TX packet data
             uint8_t     *data_size,    // RX/TX packet data size (bytes)
             uint8_t     *fixed,        // 1-fixed packet size, 0-variable packet size 
             const char  *code,         // OOK code (like "100101")
             uint8_t     *code_size,    // OOK code size (chips) = strlen(code)
             sx128x_t    *radio,        // SX128x object
             void (*rxen)(uint8_t),     // set RXEN or NULL
             void (*txen)(uint8_t))     // set TXEN or NULL
  {
    // save pointers to external (global) objects
    this->led       = led;
    this->pars      = pars;
    this->data      = data;
    this->data_size = data_size;
    this->fixed     = fixed;
    this->code      = code;
    this->code_size = code_size;
    this->radio     = radio;

    _setRXEN = rxen;
    _setTXEN = txen;

    // period timer
    _start = 0; // start timer command
    _stop  = 0; // stop timer command
    _run   = 0; // state (0-initial (TX off), 1-run (TX on))
    _t     = 0;

    // TX/RX FSM
    txrx_start = 0; // start TX timer flag
    txrx       = 0; // TX/RX state 
    power      = 0; // power on/off
    wus        = 0; // wakeup state (wut or 0)
    code_cnt   = 0; // code counter 0...code_size-1 
    dt         = 0; // CW/chip time (dt or dc) [ms]
    t          = 0;
 
    sleep_ready = 1; // ready to sleep
  }

  // FSM start
  void start() {
    if (!_run) {
      _start = 1;
    
#ifdef SX128X_USE_RANGING
      // mega fix - set ranging mode and role
      if (pars->mode == AFSM_RM || pars->mode == AFSM_RS ||
          pars->mode == AFSM_AR)
      {
        if (sx128x_get_mode(radio) != SX128X_PACKET_TYPE_RANGING)
          // change radio mode to Ranging
          sx128x_mode(radio, SX128X_PACKET_TYPE_RANGING);
      
        if (pars->mode == AFSM_AR) { // start advanced ranging
          sx128x_set_advanced_ranging(radio, 1);
        } else { // classic ranging => set role
          sx128x_ranging_role(radio, pars->mode == AFSM_RM ? 0x01 : // master
                                                             0x00); // slave
        }
      }
      else if (pars->mode == AFSM_CW || pars->mode == AFSM_OOK ||
               pars->mode == AFSM_RX || pars->mode == AFSM_TX  ||
               pars->mode == AFSM_RQ || pars->mode == AFSM_RP)
      {
        if (sx128x_get_mode(radio) == SX128X_PACKET_TYPE_RANGING) {
          sx128x_set_advanced_ranging(radio, 0); // off advanced ranging
          sx128x_mode(radio, SX128X_PACKET_TYPE_LORA); // LoRa by default
        }
      }
#endif
    }
  }
  
  // FSM stop
  void stop() {
    _stop = 1;
    led->off();
  }

  // get run state
  uint8_t run() const { return _run; }

  // set TX start time and LED on
  void tx_start(unsigned long t) {
    led->on();
    t_tx_start = t;
  }

  // TX done by TxDone interrupt
  unsigned long tx_done_dt(unsigned long irq_t) {
    return (t_tx_done = irq_t) - t_tx_start;
  }
  void tx_done();
  
  // RX done by RxDone interrupt
  unsigned long rx_done_dt(unsigned long irq_t) {
    t_rx_done_p = t_rx_done;
    t_rx_done = irq_t;
    return t_rx_done - t_rx_done_p;
  }
  void rx_done();

  // ranging done interrupt
  void ranging_done();
  
  // RX/TX timeout interrupt
  void rxtx_timeout() {
    led->off();
    txrx = power = 0;
  }

  // periodic call from main loop (t = millis())
  void yield(unsigned long t) {
    start_fsm(t); // form txrx_start timer
    txrx_fsm(t);  // TX/RX FSM 
  }
};
//-----------------------------------------------------------------------------
#endif // AFSM_H

/*** end of "afsm.h" file ***/

