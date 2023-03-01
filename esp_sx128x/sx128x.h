/*
 * Yet another Semtech SX128x famaly chips driver
 * File: "sx128x.h"
 */

#pragma once
#ifndef SX128X_H
#define SX128X_H
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdint.h>
#include "config.h"
#include "sx128x_def.h" // SX128x define's
//-----------------------------------------------------------------------------
#ifndef INLINE
#  define INLINE static inline
#endif
//-----------------------------------------------------------------------------
//#define SX128X_USE_LORA    // use LoRa mode
//#define SX128X_USE_RANGING // use LoRa Ranging mode
//#define SX128X_USE_FLRC    // use FLRC mode
//#define SX128X_USE_GFSK    // use GFSK mode
//#define SX128X_USE_BLE     // use BLE mode
//-----------------------------------------------------------------------------
//#define SX128X_USE_EXTRA   // use some extra functions
//#define SX128X_USE_BUGFIX  // use bug fix of known limitations
//-----------------------------------------------------------------------------
//#define SX128X_DEBUG       // debug print
//#define SX128X_DEBUG_IRQ   // debug verbose IRQ print
//#define SX128X_DEBUG_EXTRA // extra debug verbose print
//-----------------------------------------------------------------------------
// BUSY down timeout for SPI exchange (FIXME: why 10ms?)
#ifndef SX128X_TIMEOUT
#  define SX128X_TIMEOUT 10 // ms
#endif
//-----------------------------------------------------------------------------
// limit arguments
#define SX128X_LIMIT(x, min, max) \
  ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))

#define SX128X_MIN(x, y) ((x) < (y) ? (x) : (y))

#define SX128X_MAX(x, y) ((x) > (y) ? (x) : (y))
//-----------------------------------------------------------------------------
// common error codes (return values)
#define SX128X_ERR_NONE      0 // no error, success
#define SX128X_ERR_BUSY     -1 // wait BUSY down timeout
#define SX128X_ERR_SPI      -2 // SPI exchange error (timeout)
#define SX128X_ERR_STATUS   -3 // bad chip status
#define SX128X_ERR_BAD_CALL -4 // bad call
#define SX128X_ERR_BAD_ARG  -5 // bad argument
//-----------------------------------------------------------------------------
//#define SX128X_DEBUG
#ifdef SX128X_DEBUG
#  include <stdio.h> // printf()
#  define SX128X_DBG(fmt, arg...) printf("SX128x: " fmt "\r\n", ## arg)
#else
#  define SX128X_DBG(fmt, ...) // debug output off
#endif // SX128X_DEBUG
//-----------------------------------------------------------------------------
// RX/TX SPI buffer size
#ifndef SX128X_SPI_BUF_SIZE
#  define SX128X_SPI_BUF_SIZE 16
#endif // SX128X_SPI_BUF_SIZE
//-----------------------------------------------------------------------------
// TX SPI buffer size for PacketParams command
#define SX128X_PKT_PARS_BUF_SIZE 8
//-----------------------------------------------------------------------------
// default TX/RX base addresses by sx128x_init()
#define SX128X_FIFO_TX_BASE_ADDR 0x00
#define SX128X_FIFO_RX_BASE_ADDR 0x80
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_EXTRA
extern const char * const sx128x_status_mode_string[8];
extern const char * const sx128x_status_cmd_string[8];
extern const char * const sx128x_packet_type_string[6];
extern const char * const sx128x_time_base_string[5];
extern const char * const sx128x_gain_string[14];
#endif // SX128X_USE_EXTRA
//-----------------------------------------------------------------------------
// SX128x radio mode (packet type)
typedef enum {
  SX128X_GFSK    = 0x00, // GFSK mode
  SX128X_LORA    = 0x01, // LoRa(R) mode
  SX128X_RANGING = 0x02, // Ranging Engine mode (LoRa)
  SX128X_FLRC    = 0x03, // FLRC mode
  SX128X_BLE     = 0x04  // BLE mode
} sx128x_mode_t;
//-----------------------------------------------------------------------------
// SX128x configuration structure
typedef struct sx128x_pars_ {
  // mode:
  uint8_t mode; // packet type: 0-GFSK, 1-LoRa, 2-Ranging, 3-FLRC, 4-BLE

  // common options:
  uint32_t freq;         // frequency: 2400000000...2500000000 Hz
  int8_t   power;        // TX output power: -18...+13 dBm (note: 13 ~ 12.5 dBm)
  uint8_t  ramp;         // ramp time: 2, 4, 6, 8, 10, 12, 16, or 20 us (Table 11-49, page 88)
  uint8_t  lna_boost;    // LNA boost 1-on/0-off (RX gain: +3dB -> +0.7mA)
  uint8_t  gain;         // RX gain: 0 - AGC on, 1 - min gain, 13 - max gain
  uint8_t  dcdc;         // DC-DC: 0-off/1-on
  uint8_t  auto_fs;      // auto FS: 0 - disable, 1 - enable
  uint16_t irq_mask;     // IRQ mask
  uint16_t dio1_mask;    // DIO1 IRQ mask
  uint16_t dio2_mask;    // DIO2 IRQ mask
  uint16_t dio3_mask;    // DIO3 IRQ mask
  uint8_t  fixed;        // 1: implicit header (LoRa) / fixed packet size (FLRC/GFSK)
                         // 0: explicit header (LoRa) / variable packet size (FLRC/GFSK)
  uint8_t  payload_size; // payload size 0...255 bytes

#if defined(SX128X_USE_LORA) || defined(SX128X_RANGING)
  // LoRa and Ranging options:
  uint16_t bw;          // Bandwith: 203, 406, 812, 1625 kHz (>=406 for Ranging)
  uint8_t  sf;          // Spreading Factor: 5...12 (5...10 for Ranging)
  uint8_t  cr;          // Code Rate: 1...7 {4/5, 4/6, 4/7, 4/8, 4/5*, 4/6*, 4/8*}
  uint32_t preamble;    // Preamble length in symbols: 1..491520 (12 recomented)
  uint8_t  crc;         // CRC: 0-off, 1-on, 2-software (8 bit)
  uint8_t  invert_iq;   // invert IQ 1-on/0-off
  uint8_t  lora_sw;     // Sync Word (0x34-public, 0x12-private by default)
  uint8_t  cad_sym_num; // number of symbols on which CAD operates: 1, 2, 4, 8 or 16 (Table 11-52)
#endif

#ifdef SX128X_USE_RANGING
  // Ranging options:
  uint8_t  role;             // Ranging role (0x01-Master, 0x00-Slave)
  uint32_t master_address;   // master request address
  uint32_t slave_address;    // slave respond address
  uint8_t  slave_mode;       // bits check mode: 0...3 {8, 16, 24, 32 bits}
  uint32_t calibration;      // Ranging calibration value (24 bit), by reset 0x005FD2
  uint8_t  advanced_ranging; // Advanced Ranging activate (1-on, 0-off)
#endif

#ifdef SX128X_USE_FLRC
  // FLRC options:
  uint16_t flrc_br;        // Bitrate (BR): 260, 325, 520, 650, 1040, 1300 kBit/s
                           // BW:             300       600       1200     kHz
  uint8_t  flrc_cr;        // Code Rate: 1...3 {1, 3/4, 1/2}
  uint8_t  flrc_bt;        // Gaussian Filtering: 0...2 {off, 0.5, 1.0}
  uint16_t flrc_preamble;  // Preamble length: 4, 8, 12, 16, 20, 24, 28, 32
  uint8_t  flrc_sw_on;     // SyncWord: 0/1 {no sync, 32-bits SW}
  uint8_t  flrc_sw_mode;   // SyncWord mode: 0...7 {off, sw1, sw2, sw12, sw3, sw13, sw23, sw123}
  uint8_t  flrc_crc;       // CRC mode: 0...3 {off, 2 bytes, 3 bytes, 4 bytes}
  uint8_t  flrc_swt;       // SyncWords Tolerance [0..15], 16-use reset value
  uint32_t flrc_sw[3];     // SyncWord1, SyncWord2, SyncWord3 - each 4 bytes
#endif

#ifdef SX128X_USE_GFSK
  // GFSK options:
  uint16_t gfsk_br;        // Bitrate (BR): 125, 250, 400, 500, 800, 1000, 1600, 2000 kb/s
  uint8_t  gfsk_dsb;       // Double Side Band: 0 or 1 (BW = BR * (DSB + 1))
  uint8_t  gfsk_mi;        // Modulation Index: 0...15 {0.35...4.0}
  uint8_t  gfsk_bt;        // Shaping Filtering: 0...2 {off, 0.5, 1.0}
  uint16_t gfsk_preamble;  // Preamble length: 4, 8, 12, 16, 20, 24, 28, 32
  uint8_t  gfsk_sw_len;    // SyncWord length: 1...5
  uint8_t  gfsk_sw_mode;   // SyncWord mode: 0...7 {off, 1->1, 2->2, 1->12, 3->3, 1->13, 1->23, 1->123}
  uint8_t  gfsk_crc;       // CRC mode: 0...3 {off, 1 byte, 2 bytes, 3 bytes}
  uint8_t  gfsk_whitening; // Whitening: 0-disable, 1-enable
  uint8_t  gfsk_swt;       // SyncWords Tolerance [0..15], 16-use reset value
  uint8_t  gfsk_sw[3][5];  // SyncWord1, SyncWord2, SyncWord3 - each 5 bytes
#endif

#if defined(SX128X_USE_LORA) || defined(SX128X_USE_GFSK)
  uint8_t  lp; // Long Preamble: 0 - disable, 1 - enable
#endif

#ifdef SX128X_USE_BLE
  // BLE options:
  uint8_t  ble_state;     // ConnectionState: 0...3
  uint8_t  ble_test;      // TestPacket: 0...7
  uint8_t  ble_crc;       // CRC: 0-off (no Bluetooth compatibility), 1-on (3 bytes)
  uint8_t  ble_whitening; // Whitening: 0-disable, 1-enable
  uint32_t ble_address;   // access address
  uint32_t ble_crc_init;  // BLE CRC initialization (24 bit)
  uint16_t ble_auto_tx;   // auto TX delay: 0 - off, real delay = 33 + time us
#endif
} sx128x_pars_t;
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_RANGING)
// Explicit LoRa header (received)
typedef struct sx128x_lora_hdr_ {
  uint8_t fixed;        // 1: Implicit header, 0: Explicit header
  uint8_t cr;           // Code Rate: 1...7 {4/5, 4/6, 4/7, 4/8, 4/5*, 4/6*, 4/8*}
  uint8_t crc;          // CRC: 0-off, 1-on (hardware)
} sx128x_lora_hdr_t;
#endif // SX128X_USE_LORA || SX128X_RANGING
//-----------------------------------------------------------------------------
// SX128x common packet RX status structure
typedef struct sx128x_rx_ {
  uint8_t status; // SX128x chip Status (Chip Mode + Command Status)
  uint8_t mode;   // Chip Mode: 0:Reserved, 1:Reserved (RFU), 2:STDBY_RC,
                  //            3:STDBY_XOSC, 4:FS, 5:Rx, 6:Tx, 7:Reserved
  uint8_t stat;   // Command Status: 0:Reserved, 1:Tx start, 2: Rx done, 3:Timeout,
                  //                 4:Error, 5:Fail, 6:Tx done, 7:Reserved
  uint8_t crc_ok; // CRC: 1-ok, 0-false
  uint8_t rssi;   // RSSI = -rssi/2 [dBm]
  uint8_t lora;   // flag: 1-LoRa/Ranging, 0-GFSK/FLRC/BLE
  union {
    struct { // LoRa/Ranging status only
#if defined(SX128X_USE_LORA) || defined(SX128X_RANGING)
      sx128x_lora_hdr_t hdr; // LoRa received Explicit header
#endif // SX128X_USE_LORA || SX128X_RANGING
      uint8_t  rssi_inst; // RSSI of signal = -rssi_inst/2 [dBm]
      int8_t  snr;       // SNR of packet = snr/4 [dB]
      int32_t fei;       // FEI value [Hz]
    };
    struct { // GFSK/FLRC/BLE status only
      uint8_t pkt_status; // bit 0 - pktSent, bit 6 - rxNoAck
      uint8_t pkt_errors; // bit 0...bit 6: pktCtrlBusy, pkrRecv, hdrRecv,
                          //                AbortErr, CrcErr, LenErr, SyncErr
      uint8_t sync_addrs; // syncAddrs code (0-error, 1-adrs1, 2-adrs2, 4-adrs3)
    };
  };
} sx128x_rx_t;
//-----------------------------------------------------------------------------
// SX128x class pivate data
typedef struct sx128x_ sx128x_t;
struct sx128x_ {
  sx128x_pars_t *pars;      // all parameters for save and restore
  uint8_t sleep;            // 0 - stadby mode, 1 - sleep mode
  uint8_t status;           // last status
  uint8_t advanced_ranging; // Advanced Rangig 0-off/1-on

  uint8_t (*busy_wait)(     // BYSY=0 wait function with timeout (return BUSY state)
    uint32_t timeout,       // wait BUSY down timeout [ms]
    void *dev_context);     // optional device context or NULL

  uint8_t (*spi_exchange)( // SPI exchange function (return: 1-succes, 0-error)
    uint8_t       *rx_buf, // RX buffer
    const uint8_t *tx_buf, // TX buffer
    uint16_t len,          // number of bites
    void *dev_context);    // optional device context or NULL

  void *dev_context; // optional device context or NULL

  uint8_t txbuf[SX128X_SPI_BUF_SIZE];
  uint8_t rxbuf[SX128X_SPI_BUF_SIZE];
  uint8_t pktpars[SX128X_PKT_PARS_BUF_SIZE];

  // TX/RX buffer base addresses
  uint8_t tx_addr;
  uint8_t rx_addr; // realy don't used
};
//-----------------------------------------------------------------------------
// default SX128x radio module configuration
extern const sx128x_pars_t sx128x_pars_default;
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
// write SX128x register(s) to SPI
int8_t sx128x_reg_write(sx128x_t *self,
                      uint16_t address, const uint8_t *data, uint8_t nbytes);
//-----------------------------------------------------------------------------
// read SX128x register(s) from SPI
int8_t sx128x_reg_read(sx128x_t *self,
                     uint16_t address, uint8_t *data, uint8_t nbytes);
//-----------------------------------------------------------------------------
// write data to buffer
int8_t sx128x_buf_write(sx128x_t *self,
                      uint8_t offset, const uint8_t *data, uint16_t nbytes);
//-----------------------------------------------------------------------------
// read data from buffer
int8_t sx128x_buf_read(sx128x_t *self,
                     uint8_t offset, uint8_t *data, uint16_t nbytes);
//-----------------------------------------------------------------------------
// init SX128x radio module
int8_t sx128x_init(
  sx128x_t *self,

  uint8_t (*busy_wait)(    // BYSY=0 wait function with timeout (return BUSY state)
    uint32_t timeout,      // wait BUSY down timeout [ms]
    void *dev_context),    // optional device context or NULL

  uint8_t (*spi_exchange)( // SPI exchange function (return: 1-succes, 0-error)
    uint8_t       *rx_buf, // RX buffer
    const uint8_t *tx_buf, // TX buffer
    uint16_t len,          // number of bytes
    void *dev_context),    // optional device context or NULL

  sx128x_pars_t *pars, // configuration parameters for save and restore
  void *dev_context);  // optional device context or NULL
//-----------------------------------------------------------------------------
// free SX128x radio module (go to sleep)
int8_t sx128x_free(sx128x_t *self);
//-----------------------------------------------------------------------------
// setup SX128x radio module (uses from sx128x_init() too)
int8_t sx128x_set_pars(
  sx128x_t *self,
  sx128x_pars_t *pars); // configuration parameters or NULL (use old)
//-----------------------------------------------------------------------------
// set Sleep mode
// config: SX128X_SLEEP_OFF_RETENTION = 0,
//         SX128X_SLEEP_RAM_RETENTION = 1 | SX128X_SLEEP_BUF_RETENTION = 2
int8_t sx128x_sleep(sx128x_t *self, uint8_t config);
//-----------------------------------------------------------------------------
// get Sleep state (return: 0 - stadby mode, 1 - sleep mode)
INLINE uint8_t sx128x_get_sleep(const sx128x_t *self) { return self->sleep; }
//-----------------------------------------------------------------------------
// set Standby mode from Sleep mode (no wait BUSY down)
// config = SX128X_STANDBY_RC (0x00) or SX128X_STANDBY_XOSC (0x01)
int8_t sx128x_wakeup(sx128x_t *self, uint8_t config);
//-----------------------------------------------------------------------------
// set Standby mode (from Sleep too)
// config = SX128X_STANDBY_RC (0x00) or SX128X_STANDBY_XOSC (0x01)
int8_t sx128x_standby(sx128x_t *self, uint8_t config);
//-----------------------------------------------------------------------------
// get status command
int8_t sx128x_status(sx128x_t *self, uint8_t *status);
//-----------------------------------------------------------------------------
// get last status
INLINE uint8_t sx128x_last_status(const sx128x_t *self) { return self->status; }
//-----------------------------------------------------------------------------
// unpack status to circuit mode and command status
//   return:
//     mode (Chip Mode):
//       0x0: Reserved
//       0x1: Reserved (RFU)
//       0x2: STDBY_RC
//       0x3: STDBY_XOSC
//       0x4: FS
//       0x5: Rx
//       0x6: Tx
//       0x7: Reserved (Unknown)
//     stat (Command Status):
//       0x0: Reserved
//       0x1: Tx start
//       0x2: Rx done
//       0x3: Timeout
//       0x4: Error
//       0x5: Fail
//       0x6: Tx done
//       0x7: Reserved (Unknown)
INLINE void sx128x_status_unpack(uint8_t status, uint8_t *mode, uint8_t *stat)
{
  *mode = (status >> 5) & 0x7; // 0...7
  *stat = (status >> 2) & 0x7; // 0...7
}
//-----------------------------------------------------------------------------
// get firmware version (16 bits)
INLINE int8_t sx128x_fw_version(sx128x_t *self, uint16_t *fw_version)
{
  int8_t retv;
  uint8_t buf[2];
  retv = sx128x_reg_read(self, SX128X_REG_FW_VERSION, buf, 2);
  *fw_version = ((uint16_t) buf[0]) << 8;
  *fw_version |= (uint16_t) buf[1];
  return retv;
}
//-----------------------------------------------------------------------------
// restore config after sleep mode or hard reset
INLINE int8_t sx128x_restore(sx128x_t *self)
{
  int8_t retv = sx128x_standby(self, SX128X_STANDBY_RC);
  if (retv != SX128X_ERR_NONE) return retv;
  return sx128x_set_pars(self, NULL);
}
//-----------------------------------------------------------------------------
// set mode (packet type: 0-GFSK, 1-LoRa, 2-Ranging, 3-FLRC, 4-BLE)
uint8_t sx128x_mode(sx128x_t *self, uint8_t mode);
//-----------------------------------------------------------------------------
// get current mode (packet type: 0-GFSK, 1-LoRa, 2-Ranging, 3-FLRC, 4-BLE)
INLINE uint8_t sx128x_get_mode(const sx128x_t *self) { return self->pars->mode; }
//-----------------------------------------------------------------------------
// read mode from chip (packet type: 0-GFSK, 1-LoRa, 2-Ranging, 3-FLRC, 4-BLE)
uint8_t sx128x_read_mode(sx128x_t *self, uint8_t *mode);
//-----------------------------------------------------------------------------
// set FS mode
int8_t sx128x_fs(sx128x_t *self);
//-----------------------------------------------------------------------------
// set TX mode (clear IRQ status before using)
// Note: timeout = 0 (SX128X_TX_TIMEOUT_SINGLE) - timeout disable (TX Single mode)
//       timeout_base = 0x00 (SX128X_TIME_BASE_15_625US) => 15.625 us
//       timeout_base = 0x01 (SX128X_TIME_BASE_62_5US)   => 62.5 us
//       timeout_base = 0x02 (SX128X_TIME_BASE_1MS)      => 1 ms
//       timeout_base = 0x03 (SX128X_TIME_BASE_4MS)      => 4 ms
int8_t sx128x_tx(sx128x_t *self, uint16_t timeout, uint8_t timeout_base);
//-----------------------------------------------------------------------------
// set RX mode (clear IRQ status before using)
// Note: timeout = 0x0000 (SX128X_RX_TIMEOUT_SINGLE) - timeout disable (RX Single mode)
//       timeout = 0xFFFF (SX128X_RX_TIMEOUT_CONTINUOUS) - RX Continuous mode
//       timeout_base = 0x00 (SX128X_TIME_BASE_15_625US) => 15.625 us
//       timeout_base = 0x01 (SX128X_TIME_BASE_62_5US)   => 62.5 us
//       timeout_base = 0x02 (SX128X_TIME_BASE_1MS)      => 1 ms
//       timeout_base = 0x03 (SX128X_TIME_BASE_4MS)      => 4 ms
int8_t sx128x_rx(sx128x_t *self, uint16_t timeout, uint8_t timeout_base);
//-----------------------------------------------------------------------------
// set RX duty cycle
// Note: rx    - RX window time
//       sleep - sleep window time
//       base = 0x00 (SX128X_TIME_BASE_15_625US) => 15.625 us
//       base = 0x01 (SX128X_TIME_BASE_62_5US)   => 62.5 us
//       base = 0x02 (SX128X_TIME_BASE_1MS)      => 1 ms
//       base = 0x03 (SX128X_TIME_BASE_4MS)      => 4 ms
int8_t sx128x_rx_duty_cycle(sx128x_t *self, uint16_t rx, uint16_t sleep, uint8_t base);
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_USE_GFSK)
// set long preamble (enable = 0 or 1)
// (used with LoRa and GFSK)
int8_t sx128x_long_preamble(sx128x_t *self, uint8_t enable);
#endif // SX128X_USE_LORA || SX128X_USE_GFSK
//-----------------------------------------------------------------------------
// set CAD (Channel Activity Detection) mode
int8_t sx128x_cad(sx128x_t *self);
//-----------------------------------------------------------------------------
// set TX continuous wave (RF tone) mode
int8_t sx128x_tx_wave(sx128x_t *self);
//-----------------------------------------------------------------------------
// set TX continuous preamble
int8_t sx128x_tx_preamble(sx128x_t *self);
//-----------------------------------------------------------------------------
// set save context
int8_t sx128x_save_context(sx128x_t *self);
//-----------------------------------------------------------------------------
// set RF frequency [Hz]
int8_t sx128x_set_frequency(sx128x_t *self, uint32_t freq);
//-----------------------------------------------------------------------------
// get RF frequency [Hz]
INLINE uint32_t sx128x_get_frequency(sx128x_t *self) { return self->pars->freq; }
//-----------------------------------------------------------------------------
// set TX power level [dBm] and ramp time [us]
//   power: -18...+13 dBm
//   ramp: 2, 4, 6, 8, 10, 12, 16, or 20 us
//   (look Table 11-49 at page 88)
int8_t sx128x_set_power(sx128x_t *self,
                      int8_t power, // -18...+13 dBm
                      uint8_t ramp); // 2...20 us
//-----------------------------------------------------------------------------
// get TX power level [dBm] and ramp time [us]
INLINE void sx128x_get_power(const sx128x_t *self, int8_t *power, uint8_t *ramp)
{
  *power = self->pars->power; // -18...+13 dBm
  *ramp  = self->pars->ramp;  // 2...20 us
}
//-----------------------------------------------------------------------------
// set LNA boost on/off
// LNA boost increase current by ~0.7mA for around ~3dB in sensivity
int8_t sx128x_set_lna_boost(sx128x_t *self, uint8_t lna_boost);
//-----------------------------------------------------------------------------
// get LNA boost
INLINE uint8_t sx128x_get_lna_boost(const sx128x_t *self)
{ return self->pars->lna_boost; }
//-----------------------------------------------------------------------------
// set RX gain (AGC or manual gain)
int8_t sx128x_set_gain(
  sx128x_t *self,
  uint8_t gain); // RX gain: 0 - AGC on, 1 - min gain (max - 54dB), 13 - max gain
//-----------------------------------------------------------------------------
// get  RX gain (AGC or manual)
INLINE uint8_t sx128x_get_gain(const sx128x_t *self) { return self->pars->gain; }
//-----------------------------------------------------------------------------
// set on/off DC-DC (regulator mode)
int8_t sx128x_set_dcdc(sx128x_t *self, uint8_t dcdc); // 0-off/1-on
//-----------------------------------------------------------------------------
// get on/off DC-DC (regulator mode)
INLINE uint8_t sx128x_get_dcdc(const sx128x_t *self) { return self->pars->dcdc; }
//-----------------------------------------------------------------------------
// set auto FS (enable = 0 or 1)
int8_t sx128x_auto_fs(sx128x_t *self, uint8_t enable);
//-----------------------------------------------------------------------------
// set buffer(s) base address (TX/RX)
int8_t sx128x_set_buffer(sx128x_t *self, uint8_t tx_base_addr, uint8_t rx_base_addr);
//-----------------------------------------------------------------------------
// get RX buffer status
int8_t sx128x_get_rx_buffer(
  sx128x_t *self,
  uint8_t *status,          // SX128x status
  uint8_t *rx_payload_size, // rxPayloadLength
  uint8_t *rx_start_addr,   // rxStartBufferPointer
  uint8_t fixed);           // 1 - if LoRa Implicit Header, else 0
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
//-----------------------------------------------------------------------------
// get packet status (LoRa and Ranging)
int8_t sx128x_packet_status_lora(
  sx128x_t *self,
  uint8_t *status, // status
  uint8_t *rssi,   // RSSI of sync = -rssi/2 [dBm]
  int8_t  *snr);   // SNR of packet = snr/4 [dB]
//-----------------------------------------------------------------------------
// get LoRa Explicit Header (unpacked)
int8_t sx128x_get_hdr_lora(sx128x_t *self, sx128x_lora_hdr_t *hdr);
//-----------------------------------------------------------------------------
#endif // SX128X_USE_LORA || SX128X_USE_RANGING
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_FLRC) || defined(SX128X_USE_GFSK) || defined(SX128X_USE_BLE)
// get packet status (FLRC/GFSK/BLE)
// see 11.8.2 GetPacketStatus (page 93)
int8_t sx128x_packet_status(
  sx128x_t *self,
  uint8_t *status,     // status
  uint8_t *rssi,       // RSSI of sync = -rssi/2 [dBm]
  uint8_t *pkt_status, // status packet status (Table 11-67: Status Packet Status Byte)
  uint8_t *pkt_errors, // error packet status  (Table 11-68: Error Packet Status Byte)
  uint8_t *pkt_sync);  // sync packet status   (Table 11-69: Sync Packet Status Byte)
#endif // SX128X_USE_FLRC || SX128X_USE_GFSK || SX128X_USE_BLE
//-----------------------------------------------------------------------------
// set IRQ and DIO mask
int8_t sx128x_irq_dio_mask(sx128x_t *self,
                           uint16_t irq_mask,
                           uint16_t dio1_mask, uint16_t dio2_mask,
                           uint16_t dio3_mask);
//-----------------------------------------------------------------------------
// get IRQ status
int8_t sx128x_get_irq(sx128x_t *self, uint16_t *irq);
//-----------------------------------------------------------------------------
// clear IRQ status by mask
int8_t sx128x_clear_irq(sx128x_t *self, uint16_t mask);
//-----------------------------------------------------------------------------
// limit payload size for current mode and packet settings
INLINE uint8_t sx128x_limit_payload_size(const sx128x_t *self, uint8_t size)
{
  if (self->pars->mode == SX128X_PACKET_TYPE_LORA)
  { // 1...255 (Table 14-52)
    if      (size == 0)                           size = 1;
    else if (size == 255 && self->pars->crc == 2) size = 254;
  }
  else if (self->pars->mode == SX128X_PACKET_TYPE_FLRC)
  { // 6...127 (Table 14-38)
    if      (size < 6)   size = 6;
    else if (size > 127) size = 127;
  }
  return size;
}
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
//-----------------------------------------------------------------------------
// set modulation params in LoRa or Ranging mode
int8_t sx128x_mod_lora(
  sx128x_t *self,
  uint16_t bw,  // Bandwith: 203, 406, 812, 1625 kHz (>=406 for Ranging)
  uint8_t  sf,  // Spreading Factor: 5...12 (5...10 for Ranging)
  uint8_t  cr); // Code Rate: 1...7 {4/5, 4/6, 4/7, 4/8, 4/5*, 4/6*, 4/8*}
//-----------------------------------------------------------------------------
// set packet params in LoRa mode or Ranging mode
int8_t sx128x_packet_lora(
  sx128x_t *self,
  uint32_t preamble,     // Preamble length in symbols: 1..491520
  uint8_t crc,           // CRC 0-off, 1-on, 2-software (8 bit)
  uint8_t invert_iq,     // Invert IQ 1-on/0-off
  uint8_t impl_hdr,      // 1-Implicit headr, 0-Explicit header
  uint8_t payload_size); // Payload size 1..255
//-----------------------------------------------------------------------------
// set Sync Word in LoRa mode or Ranging mode
//   sw=0x34 - public network
//   sw=0x12 - private network by default
int8_t sx128x_set_sw_lora(sx128x_t *self, uint8_t sw);
//-----------------------------------------------------------------------------
// get Sync Word in LoRa mode or Ranging mode
INLINE uint8_t sx128x_get_sw_lora(const sx128x_t *self)
{ return self->pars->lora_sw; }
//-----------------------------------------------------------------------------
// set LoRa CAD param's
// Notice: for symbols 1 & 2, there are higher risks of false detection
int8_t sx128x_set_cad_lora(sx128x_t *self, uint8_t sym_num); // sym_num: 1, 2, 4, 8, 16
//-----------------------------------------------------------------------------
// get LoRa CAD param's
INLINE uint8_t sx128x_get_cad_lora(const sx128x_t *self)
{ return self->pars->cad_sym_num; }
//-----------------------------------------------------------------------------
// get LoRa frequency error indicator (FEI) value [Hz]
// note: LoRa FEI is reliable only for positive SNR
// see page 135 datashet
int8_t sx128x_fei_lora(sx128x_t *self, int32_t *fei);
//-----------------------------------------------------------------------------
// get instantaneous RSSI during reception of the LoRa packet
// see page 95 datasheet
int8_t sx128x_rssi_lora(
  sx128x_t *self,
  uint8_t *rssi_inst); // signal power is -rssi_inst/2 dBm
//-----------------------------------------------------------------------------
#endif // SX128X_USE_LORA || SX128X_USE_RANGING
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_RANGING
//-----------------------------------------------------------------------------
// set Ranging role (0x01-Master, 0x00-Slave)
int8_t sx128x_ranging_role(sx128x_t *self, uint8_t role);
//-----------------------------------------------------------------------------
// set Ranging master request address
int8_t sx128x_ranging_master_address(sx128x_t *self, uint32_t address);
//-----------------------------------------------------------------------------
// set Ranging slave respond address and bits check mode
int8_t sx128x_ranging_slave_address(
  sx128x_t *self,
  uint32_t address, // slave respond address
  uint8_t  mode);   // bits check mode: 0...3 {8, 16, 24, 32 bits}
//-----------------------------------------------------------------------------
// set ranging callibration register value (24 bit)
int8_t sx128x_ranging_set_calibration(sx128x_t *self, uint32_t callibration);
//-----------------------------------------------------------------------------
// get ranging callibration register value (24 bit)
int8_t sx128x_ranging_get_calibration(sx128x_t *self, uint32_t *callibration);
//-----------------------------------------------------------------------------
// get Ranging results
int8_t sx128x_ranging_result(
  sx128x_t *self,
  uint8_t  *filter,   // result type (0-off, 1-on, 2-as-is) - modified
  uint32_t *result,   // raw result
  int32_t  *distance, // distance [dm]
  uint8_t  *rssi);    // RSSI of last exchange
//-----------------------------------------------------------------------------
// on/off Advanced Ranging (0-off, 1-on)
int8_t sx128x_set_advanced_ranging(sx128x_t *self, uint8_t on);
//-----------------------------------------------------------------------------
// get Advanced Ranging state (0-off, 1-on)
INLINE uint8_t sx128x_get_advanced_ranging(const sx128x_t *self)
{ return self->pars->advanced_ranging; }
//-----------------------------------------------------------------------------
// get Advanced Ranging Address Received
int8_t sx128x_advanced_ranging_address(sx128x_t *self, uint32_t *address);
//-----------------------------------------------------------------------------
#endif // SX128X_USE_RANGING
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_FLRC) || defined(SX128X_USE_GFSK)
//-----------------------------------------------------------------------------
// set Sync Words Tolerance (GFSK/FLRC)
int8_t sx128x_set_swt(sx128x_t *self, uint8_t tolerance); // 0...15
//-----------------------------------------------------------------------------
#endif // SX128X_USE_FLRC || SX128X_USE_GFSK
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_FLRC
//-----------------------------------------------------------------------------
// set modulation params in FLRC mode
int8_t sx128x_mod_flrc(
  sx128x_t *self,
  uint16_t br,  // Bitrate (BR): 260, 325, 520, 650, 1040, 1300 kBit/s
  uint8_t  cr,  // Code Rate: 1...3 {1, 3/4, 1/2}
  uint8_t  bt); // Gaussian Filtering: 0...2 {off, 0.5, 1.0}
//-----------------------------------------------------------------------------
// set packet params in FLRC mode
int8_t sx128x_packet_flrc(
  sx128x_t *self,
  uint16_t preamble,      // Preamble length: 8, 12, 16, 20, 24, 28, 32
  uint8_t  sw,            // SyncWord: 0/1 {no sync, 32-bits SW}
  uint8_t  sw_mode,       // SyncWord mode: 0..7 {off, sw1, sw2, sw12, sw3, sw13, sw23, sw123}
  uint8_t  crc,           // CRC mode: 0..3 {off, 2 bytes, 3 bytes, 4 bytes}
  uint8_t  fixed,         // 1-fixed length mode, 0-variable length mode
  uint8_t  payload_size); // Payload size 6..127
//-----------------------------------------------------------------------------
// set Sync Words Tolerance in FLRC mode [0...15] or 16 for save default
INLINE int8_t sx128x_set_swt_flrc(sx128x_t *self, uint8_t tolerance)
{
  if (tolerance > 16) tolerance = 16;
  self->pars->flrc_swt = tolerance;
  if (tolerance > 15) return SX128X_ERR_NONE; // save default
  return sx128x_set_swt(self, tolerance);
}
//-----------------------------------------------------------------------------
// get Sync Words Tolerance in FLRC mode [0...16]
INLINE uint8_t sx128x_get_swt_flrc(const sx128x_t *self)
{ return self->pars->flrc_swt; }
//-----------------------------------------------------------------------------
// set Sync Word[1...3] in FLRC mode
int8_t sx128x_set_sw_flrc(
  sx128x_t *self,
  uint8_t  ix,  // SyncWord index 0...2
  uint32_t sw); // SyncWord value [4 bytes]
//-----------------------------------------------------------------------------
// get Sync Word[1...3] in FLRC mode
INLINE uint32_t sx128x_get_sw_flrc(const sx128x_t *self, uint8_t ix) // index 0...2
{ return self->pars->flrc_sw[ix]; }
//-----------------------------------------------------------------------------
#endif // SX128X_USE_FLRC
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_GFSK
//-----------------------------------------------------------------------------
// set modulation params in GFSK mode
int8_t sx128x_mod_gfsk(
  sx128x_t *self,
  uint16_t br,  // Bitrate (BR): 125, 250, 400, 500, 800, 1000, 1600, 2000 kb/s
  uint8_t  dsb, // Double Side Band: 0 or 1 (BW = BR * (DSB + 1))
  uint8_t  mi,  // Modulation Index: 0...15 {0.35...4.0}
  uint8_t  bt); // Shaping Filtering: 0...2 {off, 0.5, 1.0}
//-----------------------------------------------------------------------------
// set packet params in GFSK mode
int8_t sx128x_packet_gfsk(
  sx128x_t *self,
  uint16_t preamble,      // Preamble length: 4, 8, 12, 16, 20, 24, 28, 32
  uint8_t  sw_len,        // SyncWord length: 1...5
  uint8_t  sw_mode,       // SyncWord mode: 0..7 {off, 1->1, 2->2, 1->12, 3->3, 1->13, 1->23, 1->123}
  uint8_t  crc,           // CRC mode: 0...2 {off, 1 byte, 2 bytes, 3 bytes}
  uint8_t  whitening,     // Whitening: 0-disable, 1-enable
  uint8_t  fixed,         // 1-fixed length mode, 0-variable length mode
  uint8_t  payload_size); // Payload size 0..255
//-----------------------------------------------------------------------------
// set Sync Words Tolerance in GFSK mode [0...15] or 16 for save default
INLINE int8_t sx128x_set_swt_gfsk(sx128x_t *self, uint8_t tolerance)
{
  if (tolerance > 16) tolerance = 16;
  self->pars->gfsk_swt = tolerance;
  if (tolerance > 15) return SX128X_ERR_NONE; // save default
  return sx128x_set_swt(self, tolerance);
}
//-----------------------------------------------------------------------------
// get Sync Words Tolerance in GFSK mode [0...16]
INLINE uint8_t sx128x_get_swt_gfsk(const sx128x_t *self)
{ return self->pars->gfsk_swt; }
//-----------------------------------------------------------------------------
// set Sync Word[1...3] in GFSK mode
int8_t sx128x_set_sw_gfsk(
  sx128x_t *self,
  uint8_t ix,           // SyncWord index 0...2
  const uint8_t sw[5]); // SyncWord value [5 bytes]
//-----------------------------------------------------------------------------
// get Sync Word[1...3] in GFSK mode
INLINE const uint8_t *sx128x_get_sw_gfsk(const sx128x_t *self, uint8_t ix) // index 0...2
{ return self->pars->gfsk_sw[ix]; }
//-----------------------------------------------------------------------------
#endif // SX128X_USE_GFSK
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_BLE
//-----------------------------------------------------------------------------
// set modulation params in BLE mode
int8_t sx128x_mod_ble(sx128x_t *self);
//-----------------------------------------------------------------------------
// set packet params in BLE mode
//   ConnectionState:
//     0 - maximum payload size 31  (Bluetooth 4.1 and above)
//     1 - maximum payload size 37  (Bluetooth 4.1 and above)
//     2 - maximum payload size 63  (Bluetooth 4.1 and above) - test mode
//     3 - maximum payload size 255 (Bluetooth 4.2 and above)
//   TestPacket:
//     0 - Pseudo Random Binary Sequence based on 9th degree polynomial
//     1 - Repeated ‘11110000’
//     2 - Repeated ‘10101010'
//     3 - Pseudo Random Binary Sequence based on 15th degree polynomial
//     4 - Repeated ‘11111111’
//     5 - Repeated ‘00000000’
//     6 - Repeated ‘00001111’
//     7 - Repeated ‘01010101’
int8_t sx128x_packet_ble(
  sx128x_t *self,
  uint8_t state,      // ConnectionState: 0...3
  uint8_t test,       // TestPacket: 0...7
  uint8_t crc,        // CRC: 0-off (no Bluetooth compatibility), 1-on (3 bytes)
  uint8_t whitening); // Whitening: 0-disable, 1-enable
//-----------------------------------------------------------------------------
// set BLE access address
int8_t sx128x_address_ble(sx128x_t *self, uint32_t address);
//-----------------------------------------------------------------------------
// set BLE CRC initialization (24 bit)
int8_t sx128x_crc_init_ble(sx128x_t *self, uint32_t crc_init);
//-----------------------------------------------------------------------------
// set auto TX in BLE mode
// Note: real delay will be 33+time [us]
//       if time=0 then auto TX off
int8_t sx128x_auto_tx_ble(sx128x_t *self, uint16_t time);
//-----------------------------------------------------------------------------
#endif // SX128X_USE_BLE
//-----------------------------------------------------------------------------
// prepare data to send (help funcion)
int8_t sx128x_to_send(
  sx128x_t *self,
  const uint8_t *payload, // payload to send
  uint8_t  payload_size,  // payload size [bytes]
  uint8_t  fixed);        // 1-fixed packet size, 0-variable packet size
//-----------------------------------------------------------------------------
// send packet (help function)
// Note: timeout = 0 (SX128X_TX_TIMEOUT_SINGLE) - timeout disable (TX Single mode)
//       timeout_base = 0x00 (SX128X_TIME_BASE_15_625US) => 15.625 us
//       timeout_base = 0x01 (SX128X_TIME_BASE_62_5US)   => 62.5 us
//       timeout_base = 0x02 (SX128X_TIME_BASE_1MS)      => 1 ms
//       timeout_base = 0x03 (SX128X_TIME_BASE_4MS)      => 4 ms
int8_t sx128x_send(
  sx128x_t *self,
  const uint8_t *payload, // payload to send
  uint8_t  payload_size,  // payload size [bytes]
  uint8_t  fixed,         // 1-fixed packet size, 0-variable packet size
  uint16_t timeout,       // TX timeout (SX128X_TX_TIMEOUT_SINGLE - disable)
  uint8_t  timeout_base); // TX timeout base (15.625us, 62.5us, 1ms, 4ms)
//-----------------------------------------------------------------------------
// go to RX mode; wait callback by interrupt (help function)
// Note: timeout = 0x0000 (SX128X_RX_TIMEOUT_SINGLE) - timeout disable (RX Single mode)
//       timeout = 0xFFFF (SX128X_RX_TIMEOUT_CONTINUOUS) - RX Continuous mode
//       timeout_base = 0x00 (SX128X_TIME_BASE_15_625US) => 15.625 us
//       timeout_base = 0x01 (SX128X_TIME_BASE_62_5US)   => 62.5 us
//       timeout_base = 0x02 (SX128X_TIME_BASE_1MS)      => 1 ms
//       timeout_base = 0x03 (SX128X_TIME_BASE_4MS)      => 4 ms
int8_t sx128x_recv(
  sx128x_t *self,
  uint8_t  payload_size,  // payload size
  uint8_t  fixed,         // 1-fixed packet size, 0-variable packet size
  uint16_t timeout,       // RX timeout (0x0 - single mode, 0xFFFF - continuous mode)
  uint8_t  timeout_base); // RX timeout base (15.625us, 62.5us, 1ms, 4ms)
//-----------------------------------------------------------------------------
// get RX status from chip (help function)
int8_t sx128x_get_rx_status(
  sx128x_t *self,
  uint16_t irq,        // IRQ status from sx128x_get_irq()
  sx128x_rx_t *rx); // RX status
//-----------------------------------------------------------------------------
// check RX data ready and copy received data (help function)
// call sx128x_get_rx_buffer() & sx128x_buf_read()
int8_t sx128x_get_rx_data(
  sx128x_t *self,
  uint8_t *payload,         // buffer for RX payload data
  uint8_t *payload_size,    // real RX payload data size
  uint8_t max_payload_size, // maximal RX data buffer size
  uint8_t fixed);           // 1 - if LoRa Implicit Header, else 0
//-----------------------------------------------------------------------------
// get RX data and RX status from chip (help mega function)
// (call sx128x_get_rx_status() & call sx128x_get_rx_buffer() & sx128x_buf_read())
int8_t sx128x_get_recv(
  sx128x_t *self,         // pointer to `sx128x_t` object
  // input:
  uint16_t irq,              // IRQ status from sx128x_get_irq()
  uint8_t  max_payload_size, // RX data buffer size
  // output:
  sx128x_rx_t *rx,        // RX status
  uint8_t  *payload,         // buffer for RX payload data
  uint8_t  *payload_size);   // real RX payload data size
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // SX128X_H

/*** end of "sx128x.h" file ***/

