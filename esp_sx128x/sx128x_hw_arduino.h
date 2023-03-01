/*
 * Yet another Semtech SX128x famaly chips hardware wrapper for Arduino
 * File: "sx128x_hw_arduino.h"
 */

#pragma once
#ifndef SX128X_HW_ARDUINO_H
#define SX128X_HW_ARDUINO_H
//-----------------------------------------------------------------------------
#include <stdint.h>
#include "config.h"
//-----------------------------------------------------------------------------
#define SX128X_HW_RESET_T1      10 // ms
#define SX128X_HW_RESET_T2      10 // ms
#ifndef SX128X_HW_SPI
#  define SX128X_HW_SPI       (&SPI)
#endif
//-----------------------------------------------------------------------------
// global variable(s)
extern volatile char     sx128x_hw_irq_state; // GPIO IRQ state (DIO1)
extern volatile char     sx128x_hw_irq_flag;  // interrupt flag by DIO1
extern volatile unsigned sx128x_hw_irq_cnt;   // interrupt counter
extern volatile unsigned long sx128x_hw_irq_time; // time of last interrupt [ms or us]
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
#ifndef USE_DIO1_INTERRUPT
// periodic check IRQ (DIO1)
void sx128x_hw_check_dio1();
#endif // !USE_DIO1_INTERRUPT
//-----------------------------------------------------------------------------
// init SPI/GPIO
void sx128x_hw_begin();
//-----------------------------------------------------------------------------
// hard reset chip by NRST
void sx128x_hw_reset(int t1, int t2, void *context);
//-----------------------------------------------------------------------------
// on/off LNA by RXEN (0-LNA on, 1-LNA off)
void sx128x_hw_rxen(uint8_t rxen, void *context);
//-----------------------------------------------------------------------------
// on/off PowerAmp by TXEN (0-PA on, 1-PA off)
void sx128x_hw_txen(uint8_t txen, void *context);
//-----------------------------------------------------------------------------
// read state of BUSY line
uint8_t sx128x_hw_busy(void *context);
//-----------------------------------------------------------------------------
// busy wait with timeout [ms]
// return: 0 - busy=0 (success)
//         1 - busy=1 and timeout
uint8_t sx128x_hw_busy_wait(uint32_t timeout, void *context);
//-----------------------------------------------------------------------------
// SPI exchange wrapper function
// return: 0 - error (SPI timeout)
//         1 - success
uint8_t sx128x_hw_exchange(
  uint8_t       *rx_buf, // RX buffer
  const uint8_t *tx_buf, // TX buffer
  uint16_t len,          // number of bytes
  void *context);        // optional device context or NULL
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // SX128X_HW_ARDUINO_H

/*** end of "sx128x_hw_arduino.h" file ***/

