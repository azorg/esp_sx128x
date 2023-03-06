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

  AFSM_SWEEP_MIN, // sweep_min: minimal frequency [kHz]
  AFSM_SWEEP_MAX, // sweep_max: maximal frequency [kHz]
  AFSM_SWEEP_F    // sweep_f: sweep factor [kHz/sec = kHz/ms]
};
//-----------------------------------------------------------------------------
// FSM period timer (form txrx_start periodic rise)
void AFsm::start_fsm(unsigned long t)
{
  uint8_t restore = 0;

  if (_run)
  {
    if (_stop && !txrx && !wus)
    { // stop period timer
      _stop      = 0;
      _run       = 0;
      txrx_start = 0;
      restore    = 1;
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
  else if (_stop)
  {
    _stop = 0;
    restore = 1;
  }
      
  if (restore) sx128x_restore(radio);
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
        freq += dt * pars->sweep_f; // ms * Hz/ms = Hz
        if (freq < (pars->sweep_min * 1000) ||
            freq > (pars->sweep_max * 1000)) // FIXME
        { // sweep finish => go to state 1
          sx128x_set_frequency(radio, sweep_save); // restore frequency
          retv = sleep();
        }
        else
        { // set next frequency
#if 0     // FIXME: debug print
          if (Opt.verbose)
          {
            mrl_clear(&Mrl);
            print_uval("freq=", freq);
            mrl_refresh(&Mrl);
          }
#endif
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

        freq *= 1000; // kHz -> Hz
        dt = 1; // 1 ms FIXME!
        
        sweep_save = sx128x_get_frequency(radio); // save frequency
#if 0   // FIXME: debug print
        if (Opt.verbose)
        {
          mrl_clear(&Mrl);
          print_uval("freq=", freq);
          mrl_refresh(&Mrl);
        }
#endif
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
void AFsm::tx_done()
{
  int8_t retv = SX128X_ERR_NONE;
  
  led->off();
  power = 0;

  if (pars->mode == AFSM_TX && _run)
  { // TX mode => go to state 1 and sleep
    retv = sleep();
  }
  else if (pars->mode == AFSM_RQ && _run)
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
  else if (pars->mode == AFSM_RS && _run)
  { // ranging slave => go to state 1 and RX
    print_str("!!! Delete THIS CODE !!!\r\n"); // FIXME
    txrx = 0;
    _run = 0;
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }
  else if (pars->mode == AFSM_AR)
  { // advanced ranging (?!) => go to state 1 and RX
    print_str("!!! Delete THIS CODE !!!\r\n"); // FIXME
    txrx = 0;
    _run = 0;
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }

  if (retv != SX128X_ERR_NONE) print_ival("error in AFsm::tx_done(): err=", retv);
}
//-----------------------------------------------------------------------------
// RX done by RxDone interrupt
void AFsm::rx_done()
{
  int8_t retv = SX128X_ERR_NONE;
  
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
  else if (pars->mode == AFSM_RQ && _run)
  { // requester mode => go to state 1 and sleep
    txrx = 0;
    retv = sleep();
  }
  else if (pars->mode == AFSM_RP)
  { // responder mode => go to TX
    //t_tx_start = t_rx_done;
    t_tx_start = TIME_FUNC(); // FIXME
    power = 1;
    setRXEN(0);
    setTXEN(1);
    retv = sx128x_send(radio, data, *data_size, *fixed,
                       SX128X_TX_TIMEOUT_SINGLE, SX128X_TIME_BASE_1MS);
  }
  else if (pars->mode == AFSM_RM && _run)
  { // ranging master mode => go to state 1 and sleep
    print_str("!!! Delete THIS CODE !!!\r\n"); // FIXME
    retv = sleep();
  }
  else if (pars->mode == AFSM_RS || pars->mode == AFSM_AR)
  { // ranging slave or advanced ranging mode => next RX
    print_str("!!! Delete THIS CODE !!!\r\n"); // FIXME
    _run = txrx = 0;
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }

  if (retv != SX128X_ERR_NONE) print_ival("error in AFsm::rx_done(): err=", retv);
}
//-----------------------------------------------------------------------------
// ranging done interrupt
void AFsm::ranging_done()
{
  int8_t retv = SX128X_ERR_NONE;
  
  if (pars->mode == AFSM_RM && _run)
  { // ranging master done => 
    retv = sleep();
  }
  else if (pars->mode == AFSM_RS)
  { // ranging  => next RX
    print_str("!!! Delete THIS CODE !!!\r\n"); // FIXME
    _run = txrx = 0;
    led->blink();
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }
  else if (pars->mode == AFSM_AR)
  { // advanced ranging mode => next RX
    _run = txrx = 0;
    led->blink();
    setRXEN(1);
    setTXEN(0);
    retv = sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_1MS);
  }
  
  if (retv != SX128X_ERR_NONE) print_ival("error in AFsm::ranging_done(): err=", retv);
}
//-----------------------------------------------------------------------------

/*** end of "afsm.cpp" file ***/

