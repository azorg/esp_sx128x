/*
 * File: "afsm.cpp" (Finite-State Machine class)
 */
//-----------------------------------------------------------------------------
#include "afsm.h"
#include "print.h"
#include "global.h" // FIXME
//-----------------------------------------------------------------------------
const char * const afsm_mode_string[AFSM_MODES] = AFSM_MODE_STRING;
//-----------------------------------------------------------------------------
// FSM default options
const afsm_pars_t afsm_pars_default = {
  AFSM_CW, // mode: AFSM_CW, AFSM_OOK, AFSM_TX, AFSM_RX,
           //       AFSM_RQ, AFSM_RP, AFSM_RM, AFSM_RS, AFSM_AD, AFSM_SG

  4000,   // t: TX period [ms]
  
  100,    // dt: CW time [ms]
  50,     // dc: OOK code chip time [ms]
  2,      // wut: radio wakeup time [ms]

  AFSM_SWEEP_MIN * 1000, // sweep_min: minimal frequency [Hz]
  AFSM_SWEEP_MAX * 1000, // sweep_max: maximal frequency [Hz]
  AFSM_SWEEP_F           // sweep_f: sweep factor [kHz/sec = Hz/ms]
};
//-----------------------------------------------------------------------------
// FSM period timer (form txrx_start periodic rise)
void AFsm::start_fsm(unsigned long t)
{
  if (_run)
  {
    if (_stop && !txrx && !wus)
    { // stop period timer
      _stop      = 0;
      _run       = 0;
      txrx_start = 0;
    }
    else if (((long)(t - _t)) >= pars->t * TIME_FACTOR)
    {
      _t += pars->t * TIME_FACTOR;
      txrx_start = 1;
    }
  }
  else if (_start)
  { // start period timer
    _start     = 0;
    _run       = 1;
    _t         = t;
    txrx_start = 1;
  }
}
//-----------------------------------------------------------------------------
// FSM TX/RX timer has 3 state:
//  1. initial state (wait  txrx_start): txrx=0, wus=0
//  2. Wakeup radio pause: txrx=0, wus=wut
//  3. TX/RX data/code: txrx=1, wus=0
void AFsm::txrx_fsm(unsigned long t)
{
  int8_t retv = SX128X_ERR_NONE;

  if (txrx)
  { // state 3
    if (((long)(t - this->t)) >= dt * TIME_FACTOR)
    { // CW/OOK/SG interval finish
      if (pars->mode == AFSM_CW)
      { // CW beep finish => go to state 1
        retv = sleep();
      }
      else if (pars->mode == AFSM_OOK)
      { // OOK code beeper
        if (++code_cnt >= *code_size)
        { // TX OOK code finish => go to state 1
          retv = sleep();
        }
        else
        { // next OOK chip
          uint8_t next_chip = code[code_cnt] != '0' ? 1 : 0;
          if (power != next_chip) retv = wave(next_chip);
        }
      }
      else if (pars->mode == AFSM_SG)
      { // sweep generator
        freq += ((long) (t - this->t) / TIME_FACTOR) * pars->sweep_f;
        if (freq < pars->sweep_min ||
            freq > pars->sweep_max)
        { // sweep finish => go to state 1
          sx128x_set_frequency(radio, sweep_save); // restore frequency
          retv = sleep();
        }
        else
        { // set next frequency
          retv = sx128x_set_frequency(radio, freq);
        }
      }
      else
      {
        // do nothing
      }
      this->t += dt * TIME_FACTOR;
    } // if ((long)(t - this->t) >= dt * TIME_FACTOR)
  }
  else if (!txrx && wus)
  { // state 2
    if (((long)(t - this->t)) >= wus * TIME_FACTOR)
    { // goto TX/RX (state 3)
      this->t = t;
      wus     = 0;
      txrx    = 1;
      if (pars->mode == AFSM_CW)
      { // CW mode start
        dt = pars->dt;
        retv = wave(1);
      }
      else if (pars->mode == AFSM_OOK)
      { // OOK mode start
        dt = pars->dc;
        code_cnt = 0;
        retv = wave(code[code_cnt] != '0');
      }
      else if (pars->mode == AFSM_TX || pars->mode == AFSM_RQ)
      { // TX or requester -> send packet)
        t_tx_start = t;
        power = 1;
        led->on();
        setRXEN(0);
        setTXEN(1);
        retv = sx128x_send(radio, data, *data_size, *fixed,
                           SX128X_TX_TIMEOUT_SINGLE, SX128X_TIME_BASE_1MS);
      }
      else if (pars->mode == AFSM_RM)
      { // ranging master
        t_tx_start = t;
        power = 1;
        led->on();
        setRXEN(0);
        setTXEN(1);
        retv = sx128x_tx(radio, SX128X_TX_TIMEOUT_SINGLE, SX128X_TIME_BASE_1MS);
      }
      else if (pars->mode == AFSM_RS)
      { // ranging slave
        _run = 0; // stop periodic timer
        txrx = 0; // stop TX/RX timer
        setRXEN(1);
        setTXEN(0);
        retv = sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
      }
      else if (pars->mode == AFSM_RX || pars->mode == AFSM_RP)
      { // RX or responder mode -> go to continous receive mode
        uint8_t size = *fixed ? *data_size : 0;
        _run = 0; // continous receive mode => stop periodic timer
        txrx = 0; // stop TX/RX timer
        setRXEN(1);
        setTXEN(0);
        retv = sx128x_recv(radio, size, *fixed,
                           SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
      }
      else if (pars->mode == AFSM_SG)
      { // start sweep generator
        if (pars->sweep_f > 0)
          freq = pars->sweep_min;
        else if (pars->sweep_f < 0)
          freq = pars->sweep_max;
        else // pars->sweep == 0 (?!)
          freq = (pars->sweep_min + pars->sweep_max) / 2;
          
        dt = pars->dc; // FIXME: check it
        //dt = 1; // 1 ms
        
        sweep_save = sx128x_get_frequency(radio); // save frequency
        sx128x_set_frequency(radio, freq);
        retv = wave(1);
      }
    } // if (((long)(t - this->t)) >= wus * TIME_FACTOR)
  }
  else if (txrx_start)
  { // state 1 -> goto state 2 (wakeup radio)
    if (Opt.verbose >= 3)
    {
      print_uval("\rtxrx_start: t=", t);
      mrl_refresh(&Mrl);
    }
    txrx_start  = 0;
    wus         = pars->wut ? pars->wut : 1;
    txrx        = 0;
    this->t     = t;
    retv = wakeup();
  }

  if (retv != SX128X_ERR_NONE) {
    mrl_clear(&Mrl);
    print_ival("error in AFsm::txrx_fsm(): err=", retv);
    mrl_refresh(&Mrl);
  }
}
//-----------------------------------------------------------------------------
// TX done by TxDone interrupt
unsigned long AFsm::tx_done(unsigned long irq_t)
{
  int8_t retv = SX128X_ERR_NONE;
  unsigned long dt = (t_tx_done = irq_t) - t_tx_start;
 
  led->off();
  power = 0;

  if (!_run) return dt;

  if (pars->mode == AFSM_TX)
  { // TX mode => go to state 1 and sleep
    txrx = 0;
    retv = sleep();
  }
  else if (pars->mode == AFSM_RQ)
  { // requester mode => go to state 1 and RX after TX
    txrx = 0;
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_recv(radio, *fixed ? *data_size : 0, *fixed,
                       pars->t / 2, SX128X_TIME_BASE_1MS); // FIXME: timeout = period / 2 [ms]
  }
  else if (pars->mode == AFSM_RP)
  { // responder mode => goto state 1 and RX after TX
    _run = 0;
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_recv(radio, *fixed ? *data_size : 0, *fixed,
                       SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }
  else if (pars->mode == AFSM_RS)
  { // ranging slave => go to state 1 and RX
    txrx = 0;
    _run = 0;
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }
  else if (pars->mode == AFSM_AR)
  { // advanced ranging (?!) => go to state 1 and RX
    txrx = 0;
    _run = 0;
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }

  if (retv != SX128X_ERR_NONE) print_ival("error in AFsm::tx_done(): err=", retv);
  return dt;
}
//-----------------------------------------------------------------------------
// RX done by RxDone interrupt
unsigned long AFsm::rx_done(unsigned long irq_t)
{
  int8_t retv = SX128X_ERR_NONE;
  unsigned long dt;
  t_rx_done_p = t_rx_done;
  t_rx_done = irq_t;
  dt = t_rx_done - t_rx_done_p;

  if (!_run) {
    led->blink();
    return dt;
  }
  
  if (pars->mode == AFSM_RP) led->on();
  else                       led->blink();

  if (pars->mode == AFSM_RX)
  { // RX mode => next RX
    _run = txrx = 0;
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_recv(radio, *fixed ? *data_size : 0, *fixed,
                       SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }
  else if (pars->mode == AFSM_RQ)
  { // requester mode => go to state 1 and sleep
    txrx = 0;
    retv = sleep();
  }
  else if (pars->mode == AFSM_RP)
  { // responder mode => go to TX
    t_tx_start = t;
    power = 1;
    setRXEN(0);
    setTXEN(1);
    retv = sx128x_send(radio, data, *data_size, *fixed,
                       SX128X_TX_TIMEOUT_SINGLE, SX128X_TIME_BASE_1MS);
  }
  else if (pars->mode == AFSM_RM)
  { // ranging master mode => go to state 1 and sleep
    txrx = 0;
    retv = sleep();
  }
  else if (pars->mode == AFSM_RS || pars->mode == AFSM_AR)
  { // ranging slave or advanced ranging mode => next RX
    _run = txrx = 0;
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }

  if (retv != SX128X_ERR_NONE) print_ival("error in AFsm::rx_done(): err=", retv);
  return dt;
}
//-----------------------------------------------------------------------------

/*** end of "afsm.cpp" file ***/

