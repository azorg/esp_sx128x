/*
 * Simple Arduino blink LED implementration
 * File: "ablink.h"
 * Upadte: 2023.02.28
 */

#pragma once
#ifndef ABLINK_H
#define ABLINK_H
//-----------------------------------------------------------------------------
#include <Arduino.h>
//-----------------------------------------------------------------------------
#define ABLINK_PIN    LED_BUILTIN
#define ABLINK_INVERT false
#define ABLINK_ON     50  // LED on time [ms]
#define ABLINK_OFF    100 // LED off time [ms]
//-----------------------------------------------------------------------------
class ABlink {
private:
  int _pin;           // Arduino digital pin number
  uint8_t _neg;       // 0-positive (HIGH=on), 1-negative (LOW=on)
  uint8_t _state;     // 1-on, 0-off
  uint8_t _run;       // 0-initial, 1-run blink
  unsigned _cnt;      // blink down counter
  unsigned _on, _off; // on/off time [ms]
  unsigned long _t;   // change state next time

public:
  // call from setup()
  void begin(int      pin=ABLINK_PIN,    // GPIO pin number
             bool  invert=ABLINK_INVERT, // negative conect
             unsigned  on=ABLINK_ON,     // on time [ms]
             unsigned off=ABLINK_OFF) {  // off time [ms]
    _pin   = pin;
    _neg   = invert ? 1 : 0;
    _state = _run = 0;
    _cnt   = 0;
    _on    = on;
    _off   = off;
    _t     = 0;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, _neg); // LED off by default
  }

  // set LED state (ON/OFF)
  void set(uint8_t val) { // on/off LED: 0-off, 1-on
    digitalWrite(_pin, (_state = val) ^ _neg);
  }

  void on()  { set(1); } // set LED on
  void off() { set(0); } // set LED off

  // get LED state
  uint8_t get() const { return _state; } // return: 0-off 1-on
  
  // blink LED some times (0 - stop blink)
  void blink(unsigned cnt=1) { if ((_cnt = cnt) == 0) _run = 0; }

  // periodic call from main loop (t = millis())
  void yield(unsigned long t) {
    if (_cnt) {
      if (!_run) {
        _run = 1;
        _t = t + _on;
        on();
      } else {
        signed long dt = (signed long) (_t - t);
        if (dt <= 0) {
          if (get()) {
            _t = t + _off;
            off();
          } else {
            if (--_cnt) {
              _t = t + _on;
              on();
            } else {
              _run = 0;
            }
          }
        }
      }
    }
  }
};
//-----------------------------------------------------------------------------
#endif // ABLINK_H

/*** end of "ablink.h" file ***/

