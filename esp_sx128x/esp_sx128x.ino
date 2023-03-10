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
#include "sx128x_irq.h"
#include "sx128x.h"
#include "eeprom.h"
#include "tfs.h"
//-----------------------------------------------------------------------------
void ticker_callback() {

  if ((Ticks % TICKER_HZ) == 0 && Ticks >= TICKER_HZ) {
    Seconds++;
  }

  // TODO: reset WDT every N seconds
  //...

  Ticks++;
}
//-----------------------------------------------------------------------------
void setup() {
  // setup UART
  Serial.begin(BAUDRATE);
  
  // print hello message
  print_str("\r\n\r\n" HOST_NAME " started\r\n");

  // set EEPROM
  EEPROM_BEGIN(TFS_PAGE_SIZE * TFS_PAGE_NUM);

  // init TFS structure (initial set EEPROM space)
  tfs_init(&Tfs, TFS_PAGE_NUM, 0, TFS_PAGE_SIZE);

  // restore Opt from EEPROM
  opt_default(&Opt); // set to default all options
  opt_read_from_flash(&Opt, &Tfs);

  // init SPI and SPI pins
  print_str("SX128X_SPI_CLOCK=");  
  print_dint(SX128X_SPI_CLOCK / 100000);  
  print_str("MHz\r\n");  
  sx128x_hw_begin();
  
  // reset RXEN/TXEN by default
  setRXEN(0);
  setTXEN(0);

  // hardware reset SX128x
  delay(100); // FIXME: magic
  sx128x_hw_reset(10, 10, NULL);
  
  // init sx128x_t object (Radio) by default pars
  print_str("init SX128x\r\n");
  int8_t retv = sx128x_init(&Radio,
                            sx128x_hw_busy_wait, sx128x_hw_exchange,
                            &Opt.radio, NULL);
  print_ival("sx128x_init() return ", retv);
  
  // setup blink LED
  Led.begin(LED_PIN, LED_INVERT, LED_BLINK_ON, LED_BLINK_OFF);

  // setup onboard button
  pinMode(BUTTON_PIN, INPUT);
  Button = digitalRead(BUTTON_PIN);

  // setup ticker
  Ticker.begin(ticker_callback, TICKER_MS, true, millis());
  Ticks = 0;
  
  // init FSM
  Fsm.begin(&Led,             // on board LED
            &Opt.fsm,         // FSM options
            Opt.data,         // RX/TX packet data
            &Opt.data_size,   // RX/TX packet data size (bytes)
            &Opt.radio.fixed, // 1-fixed packet size, 0-variable packet size 
            Opt.code,         // OOK code (like "100101")
            &Opt.code_size,   // OOK code size (chips) = strlen(code)
            &Radio,           // SX128x object
            setRXEN,          // set RXEN or NULL
            setTXEN);         // set TXEN or NULL
  
  Seconds = 0;
  print_uval("autostart=", Autostart = Opt.autostart);
  print_uval("verbose=", Opt.verbose);

  if (Autostart) {
    retv = sx128x_sleep(&Radio, SX128X_SLEEP_OFF_RETENTION);
    print_ival("sx128x_sleep() return ", retv);
  } else {
    // set RXEN and TXEN from FLASH
    setRXEN(Opt.rxen);
    setTXEN(Opt.txen);
    print_ival("set RXEN=", RXEN);
    print_ival("set TXEN=", TXEN);
  }
  
  // init CLI (MicroRL)
  cli_init();
  print_flush();
}
//-----------------------------------------------------------------------------
void loop() {
  unsigned long ms = millis();
  unsigned long t = TIME_FUNC();
  Led.yield(ms);
  Ticker.yield(ms);
  Fsm.yield(t);

#ifndef USE_DIO1_INTERRUPT
  // periodic check IRQ (DIO1)
  sx128x_hw_check_dio1();
#endif // !USE_DIO1_INTERRUPT
 
  // check SX128x IRQ (DIO1) flag
  sx128x_irq();
  
  // check user CLI commands
  cli_loop();
  
  // check onboard button
  uint8_t btn = digitalRead(BUTTON_PIN);
  if (btn == 0 && Button == 1) {
    print_str("\r\nButton BOOT pressed (ms=");
    print_uint(ms);
    print_str(")\r\n");
    mrl_refresh(&Mrl);
  }
  Button = btn;
  
  // check autostart
  if (Autostart && Seconds >= Opt.delay) {
    Autostart = 0;
    print_str("\r\nautostart: ms=");
    print_uint(ms);
    print_str(" seconds=");
    print_uint(Seconds);
    print_eol();
    Fsm.start();
    mrl_refresh(&Mrl);
  }

}
//-----------------------------------------------------------------------------
/*** end of "esp32_sx128x.ino" file ***/

