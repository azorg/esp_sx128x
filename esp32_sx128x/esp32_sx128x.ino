/*
 * ESP32 + SX128x test application
 */

//-----------------------------------------------------------------------------
#include "config.h"
#include "global.h"
#include "cli.h"
#include "opt.h"
#include "print.h"
#include "sx128x_hw_arduino.h"
#include "sx128x.h"
//-----------------------------------------------------------------------------
void ticker_callback() {

  // blink led every 10 seconds
  if ((Ticks % (10 * TICKER_HZ)) == 0) Led.blink(2);

  Ticks++;
}
//-----------------------------------------------------------------------------
void setup() {
  // setup UART
  Serial.begin(BAUDRATE);
  
  // print "Hello"
  print_str("\r\n\r\nArduino started\r\n");

  // restore Opt
  opt_default(&Opt); // set to default all options
  // FIXME: read from FLASH

  // init SPI and SPI pins
  sx128x_hw_begin();

  // init sx128x_t object (Radio) by default pars
  print_str("init SX128x\r\n");
  int8_t retv = sx128x_init(&Radio,
                            sx128x_hw_busy_wait, sx128x_hw_exchange,
                            &Opt.radio, NULL);
  print_ival("sx128x_init() return ", (int) retv);
  
  // init CLI (MicroRL)
  cli_init();
  print_flush();

  // setup blink LED
  Led.begin(LED_PIN, LED_INVERT, LED_BLINK_ON, LED_BLINK_OFF);

  // setup onboard button
  pinMode(BUTTON_PIN, INPUT);
  Button = digitalRead(BUTTON_PIN);

  // setup ticker
  Ticker.begin(ticker_callback, TICKER_MS, true, millis());
}
//-----------------------------------------------------------------------------
void loop() {
  unsigned long t = millis();
  Led.yield(t);
  Ticker.yield(t);
  sx128x_hw_yield(t);
  cli_loop();

  uint8_t btn = digitalRead(BUTTON_PIN);
  if (btn == 0 && Button == 1)
  {
    print_str("\r\nButton BOOT pressed (millis=");
    print_uint(t);
    print_str(")\r\n");
    mrl_refresh(&Mrl);
  }
  Button = btn;
}
//-----------------------------------------------------------------------------
/*** end of "esp32_sx128x.ino" file ***/

