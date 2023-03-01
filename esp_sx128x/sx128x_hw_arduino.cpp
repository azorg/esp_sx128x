/*
 * Yet another Semtech SX128x famaly chips hardware wrapper for Arduino
 * File: "sx128x_hw_arduino.c"
 */

//-----------------------------------------------------------------------------
#include <Arduino.h>
#include <SPI.h>
#include "sx128x_hw_arduino.h"
#include "print.h"
//-----------------------------------------------------------------------------
// global variable(s)
volatile char     sx128x_hw_irq_flag  = 0; // interrupt flag by DIO1
volatile unsigned sx128x_hw_irq_cnt   = 0; // interrupt counter
volatile unsigned long sx128x_hw_irq_time = 0; // time of last interrupt [ms or us]
//-----------------------------------------------------------------------------
#ifdef USE_DIO1_INTERRUPT
//-----------------------------------------------------------------------------
void
#if defined(ARDUINO_ESP32)
IRAM_ATTR
#elif defined(ARDUINO_ESP8266)
ICACHE_RAM_ATTR
#endif
sx128x_hw_isr()
{
  sx128x_hw_irq_time = TIME_FUNC(); // ms or us
  sx128x_hw_irq_cnt++;
  sx128x_hw_irq_flag = 1;
}
//-----------------------------------------------------------------------------
#else
//-----------------------------------------------------------------------------
// periodic check IRQ (DIO1)
void sx128x_hw_check_dio1()
{ 
  static char old_state = 0;
  char state = digitalRead(SX128X_DIO1_PIN);
  if (state == 1 && old_state == 0)
  { // IRQ DIO1 rise
    sx128x_hw_irq_time = TIME_FUNC();
    sx128x_hw_irq_cnt++;
    sx128x_hw_irq_flag = 1;
  }
  old_state = state;
}
//-----------------------------------------------------------------------------
#endif // USE_DIO1_INTERRUPT
//-----------------------------------------------------------------------------
// init SPI/GPIO
void sx128x_hw_begin()
{
#ifdef SX128X_NRST_PIN
  pinMode(SX128X_NRST_PIN, OUTPUT);
  digitalWrite(SX128X_NRST_PIN, HIGH);
#endif

#ifdef SX128X_RXEN_PIN
  pinMode(SX128X_RXEN_PIN, OUTPUT);
  digitalWrite(SX128X_RXEN_PIN, LOW);
#endif

#ifdef SX128X_TXEN_PIN
  pinMode(SX128X_TXEN_PIN, OUTPUT);
  digitalWrite(SX128X_TXEN_PIN, LOW);
#endif

  pinMode(SX128X_NSS_PIN, OUTPUT);
  pinMode(SX128X_BUSY_PIN, INPUT);
  pinMode(SX128X_DIO1_PIN, INPUT);

#ifdef USE_DIO1_INTERRUPT
#if defined(ARDUINO_ESP32) || defined(ARDUINO_ESP8266)
  attachInterrupt(SX128X_DIO1_PIN, sx128x_hw_isr, RISING);
#else
  attachInterrupt(digitalPinToInterrupt(SX128X_DIO1_PIN), sx128x_hw_isr, RISING);
#endif
#endif

#ifdef SX128X_SPI_ALTERNATE_PINS
  SX128X_HW_SPI->begin(SX128X_SCK_PIN,  SX128X_MISO_PIN,
                       SX128X_MOSI_PIN, SX128X_NSS_PIN); // SCLK, MISO, MOSI, SS
#else
  SX128X_HW_SPI->begin();
#endif
}
//-----------------------------------------------------------------------------
// hard reset chip by NRST
void sx128x_hw_reset(int t1, int t2, void *context)
{
#ifdef SX128X_NRST_PIN
  if (t1 <= 0) t1 = SX128X_HW_RESET_T1;
  if (t2 <= 0) t2 = SX128X_HW_RESET_T2;
  digitalWrite(SX128X_NSS_PIN, HIGH);
  digitalWrite(SX128X_NRST_PIN, HIGH);
  delay(t1);
  digitalWrite(SX128X_NRST_PIN, LOW);
  delay(t1);
  digitalWrite(SX128X_NRST_PIN, HIGH);
  delay(t2);
#endif
}
//-----------------------------------------------------------------------------
// on/off LNA by RXEN (0-LNA on, 1-LNA off)
void sx128x_hw_rxen(uint8_t rxen, void *context)
{ // FIXME: use context to select chip
#ifdef SX128X_RXEN_PIN
  digitalWrite(SX128X_RXEN_PIN, rxen);
#endif
}
//-----------------------------------------------------------------------------
// on/off PowerAmp by TXEN (0-PA on, 1-PA off)
void sx128x_hw_txen(uint8_t txen, void *context)
{ // FIXME: use context to select chip
#ifdef SX128X_TXEN_PIN
  digitalWrite(SX128X_TXEN_PIN, txen);
#endif
}
//-----------------------------------------------------------------------------
// read state of BUSY line
uint8_t sx128x_hw_busy(void *context)
{ // FIXME: use context to select chip
  return digitalRead(SX128X_BUSY_PIN);
}
//-----------------------------------------------------------------------------
// busy wait with timeout [ms]
// return: 0 - busy=0 (success)
//         1 - busy=1 and timeout
uint8_t sx128x_hw_busy_wait(uint32_t timeout, void *context)
{
  if (timeout)
  {
    unsigned long start = millis();
    while (sx128x_hw_busy(context))
    {
      yield(); // FIXME
      if (((unsigned long) millis()) - start >= (unsigned long) timeout)
      {
        DBG("SX128X_HW: wait BUSY=0 timeout=%ums", (unsigned) timeout);
        return 1; // BUSY=1 timeout
      }
    }
  }
  return 0; // BUSY=0
}
//-----------------------------------------------------------------------------
// SPI exchange wrapper function
// return: 0 - error (SPI timeout)
//         1 - success
uint8_t sx128x_hw_exchange(
  uint8_t       *rx_buf, // RX buffer
  const uint8_t *tx_buf, // TX buffer
  uint16_t len,          // number of bytes
  void *context)         // optional device context or NULL
{ // FIXME: use context to select chip
  uint16_t i;

  // NSS down
  digitalWrite(SX128X_NSS_PIN, LOW);

  // SPI exchange
  SX128X_HW_SPI->beginTransaction(SPISettings(SX128X_SPI_CLOCK, MSBFIRST, SPI_MODE0));

  for (i = 0; i < len; i++)
    rx_buf[i] = SX128X_HW_SPI->transfer(tx_buf[i]);

  SX128X_HW_SPI->endTransaction();

  // NSS up
  digitalWrite(SX128X_NSS_PIN, HIGH);

  return 1; // 1-success, 0-error
}
//----------------------------------------------------------------------------

/*** end of "sx128x_hw_arduino.c" file ***/

