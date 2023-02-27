/*
 * Simple Arduino ticker implementration
 * File: "aticker.h"
 */

#pragma once
#ifndef ATICKER_H
#define ATICKER_H
//-----------------------------------------------------------------------------
class ATicker {
private:
  void (*_callback)(void);
  bool _run;
  unsigned long _dt;
  unsigned long _t;

public:
  // call from setup(): init and start ticker
  void begin(void (*callback)(void), // call back function
             unsigned long dt=1000,  // period of ticker
             bool start=true,        // start by begin
             unsigned long t=0)      // first callback time 
  {
    _callback = callback;
    _run      = start;
    _dt       = dt;
    _t        = t;
  }

  // periodic call from main loop (t = millis())
  void yield(unsigned long t)
  {
    if (_run) {
        for (;;) {
          signed long dt = (signed long) (_t - t);
          if (dt > 0) break;
          _t += _dt;
          _callback();
      }
    }
  }

  // (re)start ticker (t = millis() - start time)
  void start(unsigned long t)
  {
    _run = true;
    _t = t;
  }

  // stop ticker
  void stop() { _run = false; }
  
  // get next tick time
  unsigned long next_t() const { return _t; }
};
//-----------------------------------------------------------------------------
#endif // ATICKER_H

/*** end of "aticker.h" file ***/

