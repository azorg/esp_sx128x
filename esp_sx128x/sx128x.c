/*
 * Yet another Semtech SX128x famaly chips driver
 * File: "sx128x.c"
 */

//-----------------------------------------------------------------------------
#include <stdio.h>  // NULL
#include <string.h> // memset(), memcpy()
#include <stdlib.h> // abs()
#include <limits.h> // INT_MAX, INT_MIN
#include "sx128x.h" // `sx128x_t` class
#include "crc8.h"   // CRC8 function
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_EXTRA
const char * const sx128x_status_mode_string[8] = SX128X_STATUS_MODE_STRING;
const char * const sx128x_status_cmd_string[8]  = SX128X_STATUS_CMD_STRING;
const char * const sx128x_packet_type_string[6] = SX128X_PACKET_TYPE_STRING;
const char * const sx128x_time_base_string[5]   = SX128X_TIME_BASE_STRING;
const char * const sx128x_gain_string[14]       = SX128X_GAIN_STRING;
#endif // SX128X_USE_EXTRA
//-----------------------------------------------------------------------------
// default SX128x radio module configuration
const sx128x_pars_t sx128x_pars_default = {
  // mode:
  1, // packet type: 0-GFSK, 1-LoRa, 2-Ranging, 3-FLRC, 4-BLE

  // common settings:
  2450000000, // frequency: 2400000000...2500000000 Hz
  10,         // TX output power: -18...+13 dBm (note: 13 ~ 12.5 dBm)
  2,          // ramp time: 2, 4, 6, 8, 10, 12, 16, or 20 us (Table 11-49, page 88)
  0,          // LNA boost 1-on/0-off (RX gain: +3dB -> +0.7mA)
  0,          // RX gain: 0 - AGC on, 1 - min gain, 13 - max gain
  0,          // DC-DC: 0-off/1-on
  0,          // auto FS: 0 - disable, 1 - enable
  SX128X_IRQ_ALL & (~SX128X_IRQ_PREAMBLE_DETECTED), // IRQ mask
  // DIO1 IRQ mask:
  SX128X_IRQ_TX_DONE | SX128X_IRQ_RX_DONE | SX128X_IRQ_RX_TX_TIMEOUT | // RX/TX
  SX128X_IRQ_HEADER_ERROR | // Errata 16.2 (page 150)
  SX128X_IRQ_SLAVE_REQUEST_VALID | SX128X_IRQ_SLAVE_REQUEST_DISCARD | // slave request
  SX128X_IRQ_SLAVE_RESPONSE_DONE | // slave response
  SX128X_IRQ_MASTER_RESULT_VALID | SX128X_IRQ_MASTER_TIMEOUT | // master
  SX128X_IRQ_CAD_DONE | SX128X_IRQ_CAD_DETECTED | // CAD
  SX128X_IRQ_ADVANCED_RANGING_DONE, // Advanced Ranging
  SX128X_IRQ_NONE, // DIO2 IRQ mask
  SX128X_IRQ_NONE, // DIO3 IRQ mask
  1,          // 1: implicit header (LoRa) / fixed packet size (FLRC/GFSK)
              // 0: explicit header (LoRa) / variable packet size (FLRC/GFSK)
  4,          // payload size 0...255 bytes

#if defined(SX128X_USE_LORA) || defined(SX128X_RANGING)
  // LoRa and Ranging options:
  1625, // Bandwith: 203, 406, 812, 1625 kHz (>=406 for Ranging)
  10,   // Spreading Factor: 5...12 (5...10 for Ranging)
  4,    // Code Rate: 1...7 {4/5, 4/6, 4/7, 4/8, 4/5*, 4/6*, 4/8*}
  12,   // Preamble Length in symbols: 1..491520 (12 recomented)
  1,    // CRC: 0-off, 1-on, 2-software (8 bit)
  0,    // invert IQ 1-on/0-off
  0x12, // Sync Word (0x34-public, 0x12-private by default)
  16,   // number of symbols on which CAD operates: 1, 2, 4, 8 or 16 (Table 11-52)
#endif

#ifdef SX128X_USE_RANGING
  // Ranging options:
  0x00,       // Ranging role (0x01-Master, 0x00-Slave)
  0xDEADBEEF, // master request address (0x19 by reset)
  0xDEADBEEF, // slave respond address (0x19 by reset)
  3,          // bits check mode: 0...3 {8, 16, 24, 32 bits}
  13315,      // Ranging calibration value (24 bit), by reset 0x005FD2
  0,          // Advanced Ranging activate (1-on, 0-off)
#endif

#ifdef SX128X_USE_FLRC
  // FLRC options:
  1040, // Bitrate (BR): 260, 325, 520, 650, 1040, 1300 kBit/s
        // BW:             300       600       1200     kHz
  3,    // Code Rate: 1...3 {1, 3/4, 1/2}
  1,    // Gaussian Filtering: 0...2 {off, 0.5, 1.0}
  28,   // Preamble length: 8, 12, 16, 20, 24, 28, 32
  1,    // SyncWord: 0/1 {no sync, 32-bits SW}
  1,    // SyncWord mode: 0..7 {off, sw1, sw2, sw12, sw3, sw13, sw23, sw123}
  1,    // CRC mode: 0..3 {off, 2 bytes, 3 bytes, 4 bytes}
  4,    // SyncWords Tolerance [0..15], 16-use reset value (4)
  { 0x12345678, 0x87654321, 0x55AA0FF0 }, // SW1, SW2, SW3
#endif

#ifdef SX128X_USE_GFSK
  // GFSK options:
  1600, // Bitrate (BR): 125, 250, 400, 500, 800, 1000, 1600, 2000 kb/s
  0,    // Double Side Band: 0 or 1 (BW = BR * (DSB + 1))
  2,    // Modulation Index: 0...15 {0.35...4.0}
  1,    // Shaping Filtering: 0...2 {off, 0.5, 1.0}
  28,   // Preamble length: 4, 8, 12, 16, 20, 24, 28, 32
  3,    // SyncWord length: 1...5
  1,    // SyncWord mode: 0..7 {off, 1->1, 2->2, 1->12, 3->3, 1->13, 1->23, 1->123}
  2,    // CRC mode: 0...3 {off, 1 byte, 2 bytes, 3 bytes}
  1,    // Whitening: 0-disable, 1-enable
  0,    // SyncWords Tolerance [0..15], 16-use reset value (0)
  { { 0xAA, 0x13, 0x57, 0x9B, 0xDF },   // SyncWord1
    { 0x33, 0xFD, 0xB9, 0x75, 0x31 },   // SyncWord2
    { 0x55, 0xAA, 0x0F, 0xF0, 0x3C } }, // SyncWord3
#endif

#if defined(SX128X_USE_LORA) || defined(SX128X_USE_GFSK)
  0, // Long Preamble: 0 - disable, 1 - enable
#endif

#ifdef SX128X_USE_BLE
  // BLE options:
  1,          // ConnectionState: 0...3
  1,          // TestPacket: 0...7
  1,          // CRC: 0-off (no Bluetooth compatibility), 1-on (3 bytes)
  1,          // Whitening: 0-disable, 1-enable
  0x55AAFF00, // access address
  0xFFFFFF,   // BLE CRC initialization (24 bit)
  0,          // auto TX delay: 0 - off, real delay = 33 + time us
#endif
};
//-----------------------------------------------------------------------------
// SPI exchange wrapper (+check BUSY with timeout)
static int8_t sx128x_spi(sx128x_t *self, int nbytes)
{
  // wait BUSY down
  if (self->busy_wait(SX128X_TIMEOUT, self->dev_context)) return SX128X_ERR_BUSY;

  // SPI echange (rxbuf, txbuf)
  if (!self->spi_exchange(self->rxbuf, self->txbuf, nbytes,
                          self->dev_context)) return SX128X_ERR_SPI;

  self->status = self->rxbuf[0]; // save last status
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// SPI exchange wrapper - send pktpars[]
static int8_t sx128x_spi_pktpars(sx128x_t *self)
{
  // wait BUSY down
  if (self->busy_wait(SX128X_TIMEOUT, self->dev_context)) return SX128X_ERR_BUSY;

  // SPI exchange (rxbuf, pktpars)
  if (!self->spi_exchange(self->rxbuf, self->pktpars,
                          SX128X_PKT_PARS_BUF_SIZE,
                          self->dev_context)) return SX128X_ERR_SPI;

  self->status = self->rxbuf[0]; // save last status
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// run command without any params (write 1 byte of opcode)
static int8_t sx128x_cmd(sx128x_t *self, uint8_t opcode)
{
  self->txbuf[0] = opcode;
  return sx128x_spi(self, 1);
}
//-----------------------------------------------------------------------------
// run command and write 1 byte parameter
static int8_t sx128x_cmd_write(sx128x_t *self, uint8_t opcode, uint8_t param)
{
  self->txbuf[0] = opcode;
  self->txbuf[1] = param;
  return sx128x_spi(self, 2);
}
//-----------------------------------------------------------------------------
// run command and read 1 byte parameter
static int8_t sx128x_cmd_read(sx128x_t *self, uint8_t opcode, uint8_t *param)
{
  int8_t retv;
  self->txbuf[0] = opcode;
  self->txbuf[1] = 0;
  self->txbuf[2] = 0;
  self->rxbuf[2] = 0;
  retv = sx128x_spi(self, 3);
  *param = self->rxbuf[2];
  return retv;
}
//-----------------------------------------------------------------------------
// write SX128x register(s) to SPI (nbytes <= SX128X_SPI_BUF_SIZE - 3)
int8_t sx128x_reg_write(sx128x_t *self,
                        uint16_t address, const uint8_t *data, uint8_t nbytes)
{
  self->txbuf[0] = SX128X_CMD_WRITE_REGISTER;
  self->txbuf[1] = (address >> 8) & 0xFF;
  self->txbuf[2] = (address     ) & 0xFF;
  memcpy((void*) &self->txbuf[3], (const void*) data, nbytes);
  return sx128x_spi(self, nbytes + 3);
}
//-----------------------------------------------------------------------------
// read SX128x register(s) from SPI (nbytes <= SX128X_SPI_BUF_SIZE - 4)
int8_t sx128x_reg_read(sx128x_t *self,
                       uint16_t address, uint8_t *data, uint8_t nbytes)
{
  int8_t retv;
  self->txbuf[0] = SX128X_CMD_READ_REGISTER;
  self->txbuf[1] = (address >> 8) & 0xFF;
  self->txbuf[2] = (address     ) & 0xFF;
  memset((void*) &self->txbuf[3], 0, nbytes + 1);
  memset((void*) data, 0, nbytes);
  retv = sx128x_spi(self, nbytes + 4);
  memcpy((void*) data, (const void*) &self->rxbuf[4], nbytes);
  return retv;
}
//-----------------------------------------------------------------------------
// write data to buffer
int8_t sx128x_buf_write(sx128x_t *self,
                        uint8_t offset, const uint8_t *data, uint16_t nbytes)
{
  while (nbytes)
  {
    int8_t retv;
    uint8_t part = SX128X_MIN(nbytes, SX128X_SPI_BUF_SIZE - 2);
    self->txbuf[0] = SX128X_CMD_WRITE_BUFFER;
    self->txbuf[1] = offset;
    memcpy((void*) &self->txbuf[2], (const void*) data, part);
    retv = sx128x_spi(self, part + 2);
    if (retv != SX128X_ERR_NONE) return retv;
    offset += part;
    data   += part;
    nbytes -= part;
  }
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// read data from buffer
int8_t sx128x_buf_read(sx128x_t *self,
                       uint8_t offset, uint8_t *data, uint16_t nbytes)
{
  memset((void*) data, 0, nbytes);
  while (nbytes)
  {
    int8_t retv;
    uint8_t part = SX128X_MIN(nbytes, SX128X_SPI_BUF_SIZE - 3);
    self->txbuf[0] = SX128X_CMD_READ_BUFFER;
    self->txbuf[1] = offset;
    memset((void*) &self->txbuf[2], 0, part + 1);
    retv = sx128x_spi(self, part + 3);
    if (retv != SX128X_ERR_NONE) return retv;
    memcpy((void*) data, (const void*) &self->rxbuf[3], part);
    offset += part;
    data   += part;
    nbytes -= part;
  }
  return SX128X_ERR_NONE;
}
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

  sx128x_pars_t *pars,     // configuration parameters for save and restore
  void *dev_context)       // optional device context or NULL
{
  uint8_t status, mode, stat;
  int8_t retv;

  self->pars         = pars;
  self->busy_wait    = busy_wait;
  self->spi_exchange = spi_exchange;
  self->dev_context  = dev_context;
  self->sleep        = 0;
  self->status       = 0;

  SX128X_DBG("init radio module");

  // wakeup and standby FIXME: magic!
  sx128x_wakeup(self, SX128X_STANDBY_RC);
  sx128x_status(self, &status);
  sx128x_standby(self, SX128X_STANDBY_XOSC);
  sx128x_status(self, &status);
  sx128x_standby(self, SX128X_STANDBY_RC);
  sx128x_status(self, &status);

  // get status
  retv = sx128x_status(self, &status);
  if (retv != SX128X_ERR_NONE) return retv;

  sx128x_status_unpack(status, &mode, &stat);

  // check status of standby in RC
  if (mode != 2) // FIXME: magic (2-RC, 3-XOSC)
  {
    SX128X_DBG("error: bad status=0x%02X (ChipMode=%i, but must be 2-RC or 3-XOSC)",
               (unsigned) status, (int) mode);
    SX128X_DBG("       may be no SPI connection or need hard reset...");
    return SX128X_ERR_STATUS;
  }

  // set parameters
  return sx128x_set_pars(self, pars);
}
//-----------------------------------------------------------------------------
// free SX128x radio module (go to sleep)
int8_t sx128x_free(sx128x_t *self)
{ // do nothing -> go to sleep
  return sx128x_sleep(self, SX128X_SLEEP_OFF_RETENTION);
}
//-----------------------------------------------------------------------------
// setup SX128x radio module (uses from sx128x_init() too)
int8_t sx128x_set_pars(
  sx128x_t *self,
  sx128x_pars_t *pars) // configuration parameters or NULL (use old)
{
  int8_t retv;
  int i;
  uint16_t irq_ard = 0;

  if (pars != (sx128x_pars_t*) NULL)
    self->pars = pars;
  else
    pars = self->pars;

  // set mode (set packet type)
  retv = sx128x_mode(self, pars->mode);
  if (retv != SX128X_ERR_NONE) return retv;

  // define RF frequency [Hz]
  retv = sx128x_set_frequency(self, pars->freq);
  if (retv != SX128X_ERR_NONE) return retv;

  // config output power [dBm] and ramp time [us]
  retv = sx128x_set_power(self, pars->power, pars->ramp);
  if (retv != SX128X_ERR_NONE) return retv;

  // set LNA boost on/off
  retv = sx128x_set_lna_boost(self, pars->lna_boost);
  if (retv != SX128X_ERR_NONE) return retv;

  // set RX gain (AGC or manual gain)
  retv = sx128x_set_gain(self, pars->gain);
  if (retv != SX128X_ERR_NONE) return retv;

  // config DC-DC/LDO
  retv = sx128x_set_dcdc(self, pars->dcdc);
  if (retv != SX128X_ERR_NONE) return retv;

  // set auto FS
  retv = sx128x_auto_fs(self, pars->auto_fs);
  if (retv != SX128X_ERR_NONE) return retv;

  // config IRQ
#ifdef SX128X_USE_RANGING
  if (pars->advanced_ranging) irq_ard = SX128X_IRQ_ADVANCED_RANGING_DONE;
#endif
  retv = sx128x_irq_dio_mask(self,
                             pars->irq_mask | irq_ard, // interrupts enabled
                             pars->dio1_mask,          // interrupts on DIO1
                             pars->dio2_mask,          // interrupts on DIO2
                             pars->dio3_mask);         // interrupts on DIO3
  if (retv != SX128X_ERR_NONE) return retv;

  // clear all IRQ status
  retv = sx128x_clear_irq(self, SX128X_IRQ_ALL);
  if (retv != SX128X_ERR_NONE) return retv;

  // set TX/RX base address by default
  retv = sx128x_set_buffer(self, SX128X_FIFO_TX_BASE_ADDR,
                                 SX128X_FIFO_RX_BASE_ADDR);
  if (retv != SX128X_ERR_NONE) return retv;

#if defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
  if (pars->mode == SX128X_PACKET_TYPE_LORA ||
      pars->mode == SX128X_PACKET_TYPE_RANGING)
  { // set LoRa and Ranging options

    // set BW, SF, CR
    retv = sx128x_mod_lora(self, pars->bw, pars->sf, pars->cr);
    if (retv != SX128X_ERR_NONE) return retv;

    // set Preamble, CRC, InvertIQ, HDR, PayloadSize
    retv = sx128x_packet_lora(self,
                              pars->preamble, pars->crc, pars->invert_iq,
                              pars->fixed, pars->payload_size);
    if (retv != SX128X_ERR_NONE) return retv;

    // set LoRa Sync Word
    retv = sx128x_set_sw_lora(self, pars->lora_sw);
    if (retv != SX128X_ERR_NONE) return retv;

    // set LoRa CAD param's
    retv = sx128x_set_cad_lora(self, pars->cad_sym_num);
    if (retv != SX128X_ERR_NONE) return retv;

#ifdef SX128X_USE_RANGING
    if (pars->mode == SX128X_PACKET_TYPE_RANGING)
    {
      // set Ranging role (master/slave)
      retv = sx128x_ranging_role(self,
                                 pars->advanced_ranging ?
                                 0x00 : self->pars->role);
      if (retv != SX128X_ERR_NONE) return retv;

      // set Ranging master request address
      sx128x_ranging_master_address(self, pars->master_address);
      if (retv != SX128X_ERR_NONE) return retv;

      // set Ranging slave respond address and bits check mode
      sx128x_ranging_slave_address(self, pars->slave_address, pars->slave_mode);
      if (retv != SX128X_ERR_NONE) return retv;

      // set ranging callibration register value (24 bit)
      retv = sx128x_ranging_set_calibration(self, pars->calibration);
      if (retv != SX128X_ERR_NONE) return retv;

      if (pars->advanced_ranging)
      { // set Advanced Ranging (by writing 0x01 to opcode 0x9A)
        retv = sx128x_cmd_write(self, SX128X_CMD_SET_ADVANCED_RANGING, 0x01);
        if (retv != SX128X_ERR_NONE) return retv;
        self->pars->advanced_ranging = 1;
      }
      else
      { // deactivate Advanced Ranging mode: write 0x00 to opcode 0x9A
        retv = sx128x_cmd_write(self, SX128X_CMD_SET_ADVANCED_RANGING, 0x00);
        if (retv != SX128X_ERR_NONE) return retv;
        self->pars->advanced_ranging = 0;
      }
    }
#endif // SX128X_USE_RANGING
  }
#endif // SX128X_USE_LORA || SX128X_USE_RANGING

#ifdef SX128X_USE_FLRC
  if (pars->mode == SX128X_PACKET_TYPE_FLRC)
  { // set FLRC options

    // set DR, CR, BT
    retv = sx128x_mod_flrc(self, pars->flrc_br, pars->flrc_cr, pars->flrc_bt);
    if (retv != SX128X_ERR_NONE) return retv;

    // set Preamble, SW & Mode, CRC, Whitening, Fixed, PayloadSize
    retv = sx128x_packet_flrc(self, pars->flrc_preamble,
                              pars->flrc_sw_on, pars->flrc_sw_mode,
                              pars->flrc_crc, pars->fixed, pars->payload_size);
    if (retv != SX128X_ERR_NONE) return retv;

    // set Sync Words Tolerance in FLRC mode [0...15] or 16 for save default
    retv = sx128x_set_swt_flrc(self, pars->flrc_swt);
    if (retv != SX128X_ERR_NONE) return retv;

    // set Sync Word[1...3] in FLRC
    for (i = 0; i < 3; i++)
    {
       retv = sx128x_set_sw_flrc(self, i, pars->flrc_sw[i]);
       if (retv != SX128X_ERR_NONE) return retv;
    }
  }
#endif // SX128X_USE_FLRC

#ifdef SX128X_USE_GFSK
  if (pars->mode == SX128X_PACKET_TYPE_GFSK)
  { // set GFSK options

    // set BR, DSB, MI, BT
    retv = sx128x_mod_gfsk(self, pars->gfsk_br, pars->gfsk_dsb,
                           pars->gfsk_mi, pars->gfsk_bt);
    if (retv != SX128X_ERR_NONE) return retv;

    // set Preamble, SW length & mode, CRC, Whitening, Fixed, PayloadSize
    retv = sx128x_packet_gfsk(self, pars->gfsk_preamble,
                              pars->gfsk_sw_len, pars->gfsk_sw_mode,
                              pars->gfsk_crc, pars->gfsk_whitening,
                              pars->fixed, pars->payload_size);
    if (retv != SX128X_ERR_NONE) return retv;

    // set Sync Words Tolerance in GFSK mode [0...15] or 16 for save default
    retv = sx128x_set_swt_gfsk(self, pars->gfsk_swt);
    if (retv != SX128X_ERR_NONE) return retv;

    // set Sync Word[1...3] in GFSK
    for (i = 0; i < 3; i++)
    {
       retv = sx128x_set_sw_gfsk(self, i, pars->gfsk_sw[i]);
       if (retv != SX128X_ERR_NONE) return retv;
    }
  }
#endif // SX128X_USE_GFSK

#if defined(SX128X_USE_LORA) || defined(SX128X_USE_GFSK)
  if (pars->mode == SX128X_PACKET_TYPE_LORA ||
      pars->mode == SX128X_PACKET_TYPE_GFSK)
  { // set Long Preamble (LoRa/GFSK)
    retv = sx128x_long_preamble(self, pars->lp);
    if (retv != SX128X_ERR_NONE) return retv;
  }
#endif // SX128X_USE_LORA || SX128X_USE_GFSK

#ifdef SX128X_USE_BLE
  if (pars->mode == SX128X_PACKET_TYPE_BLE)
  { // set BLE options

    // set modulation params in BLE mode
    retv = sx128x_mod_ble(self);
    if (retv != SX128X_ERR_NONE) return retv;

    // ConnectionState, TestPacket, CRC, Whitening
    retv = sx128x_packet_ble(self, pars->ble_state, pars->ble_test,
                             pars->ble_crc, pars->ble_whitening);
    if (retv != SX128X_ERR_NONE) return retv;

    // set BLE access address
    retv = sx128x_address_ble(self, pars->ble_address);
    if (retv != SX128X_ERR_NONE) return retv;

    // set BLE CRC initialization (24 bit)
    retv = sx128x_crc_init_ble(self, pars->ble_crc_init);
    if (retv != SX128X_ERR_NONE) return retv;

    // set auto TX delay in BLE
    retv = sx128x_auto_tx_ble(self, pars->ble_auto_tx);
    if (retv != SX128X_ERR_NONE) return retv;
  }
#endif // SX128X_USE_BLE

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// set Sleep mode
// config: SX128X_SLEEP_OFF_RETENTION = 0,
//         SX128X_SLEEP_RAM_RETENTION = 1 | SX128X_SLEEP_BUF_RETENTION = 2
int8_t sx128x_sleep(sx128x_t *self, uint8_t config)
{
  int8_t retv;
  if (self->sleep)
  {
    SX128X_DBG("already sleep: do nothing");
    return SX128X_ERR_BAD_CALL;
  }

  retv = sx128x_cmd_write(self, SX128X_CMD_SET_SLEEP, config);
  if (retv != SX128X_ERR_NONE) return retv;

  self->sleep = 1;

#ifdef SX128X_DEBUG_EXTRA
  SX128X_DBG("set Sleep mode (config=0x%02X: "
             "config_retention=%i buffer_retention=%i)",
             (unsigned) config,
             !!(config & SX128X_SLEEP_RAM_RETENTION),
             !!(config & SX128X_SLEEP_BUF_RETENTION));
#else
  SX128X_DBG("set Sleep mode (config=0x%02X)", (unsigned) config);
#endif // SX128X_DEBUG_EXTRA

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// set Standby mode from Sleep mode (no wait BUSY down)
// config = SX128X_STANDBY_RC (0x00) or SX128X_STANDBY_XOSC (0x01)
int8_t sx128x_wakeup(sx128x_t *self, uint8_t config)
{
  int8_t retv;

  self->txbuf[0] = SX128X_CMD_SET_STANDBY;
  self->txbuf[1] = config;

  // SPI echange (rxbuf, txbuf)
  if (!self->spi_exchange(self->rxbuf, self->txbuf, 2,
                          self->dev_context)) return SX128X_ERR_SPI;

  self->status = self->rxbuf[0]; // save last status
  self->sleep = 0;

#ifdef SX128X_DEBUG_EXTRA
  SX128X_DBG("wakeup: set Standby mode (config=0x%02X => %s)",
             (unsigned) config,
             config == SX128X_STANDBY_XOSC ? "XOSC" : "RC13M");
#else
  SX128X_DBG("wakeup: set Standby mode (config=0x%02X)",
             (unsigned) config);
#endif // SX128X_DEBUG_EXTRA

#ifdef SX128X_USE_BUGFIX
  if (1)
  { // bug fix
    uint8_t mode;
    static const uint8_t blob[] = SX128X_REG_RSSI_SNR_BUGFIX_BLOB;

#if 1 // FIXME: test it
    mode = self->pars->mode;
#else // FIXME: test it
    retv = sx128x_read_mode(self, &mode);
    if (retv != SX128X_ERR_NONE) return retv;
#endif

    if (mode == SX128X_PACKET_TYPE_LORA ||
        mode == SX128X_PACKET_TYPE_RANGING)
    {
      retv = sx128x_reg_write(self,
                              SX128X_REG_RSSI_SNR_BUGFIX_ADDRESS,
                              blob, sizeof(blob));
      if (retv != SX128X_ERR_NONE) return retv;
      SX128X_DBG("fix RSSI=0 bug (write blob to 0x%03X register)",
                 (unsigned) SX128X_REG_RSSI_SNR_BUGFIX_ADDRESS);
    }
  }
#endif // SX128X_USE_BUGFIX

#if 0
  // restore all settings
  retv = sx128x_set_pars(self, NULL);
  if (retv != SX128X_ERR_NONE) return retv;
#endif
  (void) retv;
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// set Standby mode (from Sleep too)
// config = SX128X_STANDBY_RC (0x00) or SX128X_STANDBY_XOSC (0x01)
int8_t sx128x_standby(sx128x_t *self, uint8_t config)
{
  int8_t retv;
  if (self->sleep) return sx128x_wakeup(self, config);

  retv = sx128x_cmd_write(self, SX128X_CMD_SET_STANDBY, config);
  if (retv == SX128X_ERR_BUSY) return sx128x_wakeup(self, config);
#ifdef SX128X_DEBUG
  if (retv == SX128X_ERR_NONE)
  {
#ifdef SX128X_DEBUG_EXTRA
    SX128X_DBG("set Standby mode (config=0x%02X => %s)",
               (unsigned) config,
               config == SX128X_STANDBY_XOSC ? "XOSC" : "RC13M");
#else
    SX128X_DBG("set Standby mode (config=0x%02X)", (unsigned) config);
#endif // SX128X_DEBUG_EXTRA
  }
#endif // SX128X_DEBUG
  return retv;
}
//-----------------------------------------------------------------------------
// get status command
int8_t sx128x_status(sx128x_t *self, uint8_t *status)
{
  int8_t retv = sx128x_cmd(self, SX128X_CMD_GET_STATUS);

  *status = self->status;

  SX128X_DBG("get status=0x%02X (retv=%i):", (int) *status, (int) retv);

#ifdef SX128X_DEBUG
  if (retv == SX128X_ERR_NONE)
  {
    uint8_t mode, stat;
    sx128x_status_unpack(*status, &mode, &stat);
#ifdef SX128X_USE_EXTRA
    SX128X_DBG("  ChipMode=%s", sx128x_status_mode_string[mode]);
    SX128X_DBG(" CmdStatus=%s", sx128x_status_cmd_string[stat]);
#else
    SX128X_DBG("  ChipMode=%i", (int) mode);
    SX128X_DBG(" CmdStatus=%i", (int) stat);
#endif
  }
#endif // SX128X_DEBUG

  return retv;
}
//-----------------------------------------------------------------------------
// set mode (packet type: 0-GFSK, 1-LoRa, 2-Ranging, 3-FLRC, 4-BLE)
uint8_t sx128x_mode(sx128x_t *self, uint8_t mode)
{
  if (mode >= SX128X_PACKET_TYPE_RESERVED) mode = SX128X_PACKET_TYPE_LORA;
#ifdef SX128X_USE_EXTRA
  SX128X_DBG("set mode=%i (%s)", (int) mode, sx128x_packet_type_string[mode]);
#else
  SX128X_DBG("set mode=%i", (int) mode);
#endif // SX128X_USE_EXTRA

  self->pars->mode = mode;

  return sx128x_cmd_write(self, SX128X_CMD_SET_PACKET_TYPE, mode);
}
//-----------------------------------------------------------------------------
// read mode from chip (packet type: 0-GFSK, 1-LoRa, 2-Ranging, 3-FLRC, 4-BLE)
uint8_t sx128x_read_mode(sx128x_t *self, uint8_t *mode)
{
  int8_t retv = sx128x_cmd_read(self, SX128X_CMD_GET_PACKET_TYPE, mode);

#ifdef SX128X_DEBUG
  if (retv == SX128X_ERR_NONE)
  {
#ifdef SX128X_USE_EXTRA
    if (*mode > SX128X_PACKET_TYPE_RESERVED) *mode = SX128X_PACKET_TYPE_RESERVED;
    SX128X_DBG("get mode=%i (%s)", (int) *mode, sx128x_packet_type_string[*mode]);
#else
    SX128X_DBG("get mode=%i", (int) *mode);
#endif // SX128X_USE_EXTRA
  }
#endif // SX128X_DEBUG

  return retv;
}
//-----------------------------------------------------------------------------
// set FS mode
int8_t sx128x_fs(sx128x_t *self)
{
  int8_t retv = sx128x_cmd(self, SX128X_CMD_SET_FS);
#ifdef SX128X_DEBUG_EXTRA
  SX128X_DBG("set FS mode: status=0x%02X (retv=%i)",
             (int) self->status, (int) retv);
#else
  SX128X_DBG("set FS mode");
#endif // SX128X_DEBUG_EXTRA
  return retv;
}
//-----------------------------------------------------------------------------
// set TX mode (clear IRQ status before using)
// Note: timeout = 0 (SX128X_TX_TIMEOUT_SINGLE) - timeout disable (TX Single mode)
//       timeout_base = 0x00 (SX128X_TIME_BASE_15_625US) => 15.625 us
//       timeout_base = 0x01 (SX128X_TIME_BASE_62_5US)   => 62.5 us
//       timeout_base = 0x02 (SX128X_TIME_BASE_1MS)      => 1 ms
//       timeout_base = 0x03 (SX128X_TIME_BASE_4MS)      => 4 ms
int8_t sx128x_tx(sx128x_t *self, uint16_t timeout, uint8_t timeout_base)
{
#if defined(SX128X_DEBUG) && defined(SX128X_USE_EXTRA)
  int base_ix = (timeout_base > 0x04) ? 0x04 : (int) timeout_base;
  SX128X_DBG("set TX mode timeout=0x%04X base=0x%02X (%s)",
             (unsigned) timeout, (unsigned) timeout_base,
             sx128x_time_base_string[base_ix]);
#else
  SX128X_DBG("set TX mode timeout=0x%04X base=0x%02X",
             (unsigned) timeout, (unsigned) timeout_base);
#endif

  self->txbuf[0] = SX128X_CMD_SET_TX;
  self->txbuf[1] = timeout ? timeout_base : 0;
  self->txbuf[2] = (timeout >> 8) & 0xFF;
  self->txbuf[3] = (timeout     ) & 0xFF;

  return sx128x_spi(self, 4);
}
//-----------------------------------------------------------------------------
// set RX mode (clear IRQ status before using)
// Note: timeout = 0x0000 (SX128X_RX_TIMEOUT_SINGLE) - timeout disable (RX Single mode)
//       timeout = 0xFFFF (SX128X_RX_TIMEOUT_CONTINUOUS) - RX Continuous mode
//       timeout_base = 0x00 (SX128X_TIME_BASE_15_625US) => 15.625 us
//       timeout_base = 0x01 (SX128X_TIME_BASE_62_5US)   => 62.5 us
//       timeout_base = 0x02 (SX128X_TIME_BASE_1MS)      => 1 ms
//       timeout_base = 0x03 (SX128X_TIME_BASE_4MS)      => 4 ms
int8_t sx128x_rx(sx128x_t *self, uint16_t timeout, uint8_t timeout_base)
{
#if defined(SX128X_DEBUG) && defined(SX128X_USE_EXTRA)
  int base_ix = (timeout_base > 0x04) ? 0x04 : (int) timeout_base;
  SX128X_DBG("set RX mode timeout=0x%04X base=0x%02X (%s)",
             (unsigned) timeout, (unsigned) timeout_base,
             sx128x_time_base_string[base_ix]);
#else
  SX128X_DBG("set RX mode timeout=0x%04X base=0x%02X",
             (unsigned) timeout, (unsigned) timeout_base);
#endif

  self->txbuf[0] = SX128X_CMD_SET_RX;
  self->txbuf[1] = timeout ? timeout_base : 0;
  self->txbuf[2] = (timeout >> 8) & 0xFF;
  self->txbuf[3] = (timeout     ) & 0xFF;

  return sx128x_spi(self, 4);
}
//-----------------------------------------------------------------------------
// set RX duty cycle
// Note: rx    - RX window time
//       sleep - sleep window time
//       base = 0x00 (SX128X_TIME_BASE_15_625US) => 15.625 us
//       base = 0x01 (SX128X_TIME_BASE_62_5US)   => 62.5 us
//       base = 0x02 (SX128X_TIME_BASE_1MS)      => 1 ms
//       base = 0x03 (SX128X_TIME_BASE_4MS)      => 4 ms
int8_t sx128x_rx_duty_cycle(sx128x_t *self, uint16_t rx, uint16_t sleep, uint8_t base)
{
#if defined(SX128X_DEBUG) && defined(SX128X_USE_EXTRA)
  int base_ix = (base > 0x04) ? 0x04 : (int) base;
  SX128X_DBG("set RX duty cycle rx=0x%04X sleep=0x%04X base=0x%02X (%s)",
             (unsigned) rx, (unsigned) sleep, (unsigned) base,
             sx128x_time_base_string[base_ix]);
#else
  SX128X_DBG("set RX duty cycle rx=0x%04X sleep=0x%04X base=0x%02X",
             (unsigned) rx, (unsigned) sleep, (unsigned) base);
#endif // SX128X_DEBUG && SX128X_USE_EXTRA

  self->txbuf[0] = SX128X_CMD_SET_RX_DUTY_CYCLE;
  self->txbuf[1] = base;
  self->txbuf[2] = (rx    >> 8) & 0xFF;
  self->txbuf[3] = (rx        ) & 0xFF;
  self->txbuf[4] = (sleep >> 8) & 0xFF;
  self->txbuf[5] = (sleep     ) & 0xFF;

  return sx128x_spi(self, 6);
}
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_USE_GFSK)
// set long preamble (enable = 0 or 1)
// (used with LoRa and GFSK)
int8_t sx128x_long_preamble(sx128x_t *self, uint8_t enable)
{
  SX128X_DBG("set long preamble: enable=%i (LoRa/GFSK)", (int) enable);
  self->pars->lp = enable;
  return sx128x_cmd_write(self, SX128X_CMD_SET_LONG_PREAMBLE, enable);
}
#endif // SX128X_USE_LORA || SX128X_USE_GFSK
//-----------------------------------------------------------------------------
// set CAD (Channel Activity Detection) mode
int8_t sx128x_cad(sx128x_t *self)
{
  SX128X_DBG("set CAD mode");
  return sx128x_cmd(self, SX128X_CMD_SET_CAD);
}
//-----------------------------------------------------------------------------
// set TX continuous wave (RF tone) mode
int8_t sx128x_tx_wave(sx128x_t *self)
{
  SX128X_DBG("set TX continuous wave mode");
  return sx128x_cmd(self, SX128X_CMD_SET_TX_CONTINUOUS_WAVE);
}
//-----------------------------------------------------------------------------
// set TX continuous preamble
int8_t sx128x_tx_preamble(sx128x_t *self)
{
  SX128X_DBG("set TX continuous preamble mode");
  return sx128x_cmd(self, SX128X_CMD_SET_TX_CONTINUOUS_PREAMBLE);
}
//-----------------------------------------------------------------------------
// set save context
int8_t sx128x_save_context(sx128x_t *self)
{
  SX128X_DBG("set save context");
  return sx128x_cmd(self, SX128X_CMD_SET_SAVE_CONTEXT);
}
//-----------------------------------------------------------------------------
// convert RF frequency from Hz to code (code = freq * 2**10 / (5**6 * 13))
// note: rf_step = 56 MHz / 2**18 = 13 * 5**6 / 2**10 = 198.3642578125 Hz
static uint32_t sx128x_freq2code(uint32_t freq)
{
  uint32_t code = (freq / (13UL * 15625UL)) << 11UL;
  freq       = (freq % (13UL * 15625UL)) << 11UL;
  code += freq / (13UL * 15625UL);
  return (code >> 1) + (code & 1);
}
//-----------------------------------------------------------------------------
// convert RF frequency code to Hz (freq = code * 13 * 5**6 / 2**10)
// note: rf_step = 56 MHz / 2**18 = 13 * 5**6 / 2**10 = 198.3642578125 Hz
static uint32_t sx128x_code2freq(uint32_t code)
{ // code = 0...21651921
  uint32_t cl = (code >>  0) & 0x3FFF; // 0...16383
  uint32_t ch = (code >> 14) & 0x7FF;  // 0...1321
  cl *= 13UL * 15625UL; // 0...3327796875
  ch *= 13UL * 15625UL; // 0...268328125
  cl >>= 9;
  return (cl >> 1) + (cl & 1) + (ch << 4); // 0...4296499801 Hz
}
//-----------------------------------------------------------------------------
// set RF frequency [Hz]
int8_t sx128x_set_frequency(sx128x_t *self, uint32_t freq)
{
  uint32_t code    = sx128x_freq2code(freq);
  self->pars->freq = sx128x_code2freq(code);

  SX128X_DBG("set RF frequency to %uHz (code=%u)",
             (unsigned) self->pars->freq, (unsigned) code);

  self->txbuf[0] = SX128X_CMD_SET_RF_FREQUENCY;
  self->txbuf[1] = (code >> 16) & 0xFF;
  self->txbuf[2] = (code >>  8) & 0xFF;
  self->txbuf[3] = (code      ) & 0xFF;

  return sx128x_spi(self, 4);
}
//-----------------------------------------------------------------------------
// set TX power level [dBm] and ramp time [us]
//   power: -18...+13 dBm
//   ramp: 2, 4, 6, 8, 10, 12, 16, or 20 us
//   (look Table 11-49 at page 88 datasheet)
int8_t sx128x_set_power(sx128x_t *self,
                        int8_t power, // -18...+13 dBm
                        uint8_t ramp)  // 2...20 us
{
  uint8_t value;

  // Table 11-49: RampTime Definition (page 88)
  if      (ramp <=  2) { ramp =  2; value = SX128X_RADIO_RAMP_02_US; }
  else if (ramp <=  4) { ramp =  4; value = SX128X_RADIO_RAMP_04_US; }
  else if (ramp <=  6) { ramp =  6; value = SX128X_RADIO_RAMP_06_US; }
  else if (ramp <=  8) { ramp =  8; value = SX128X_RADIO_RAMP_08_US; }
  else if (ramp <= 10) { ramp = 10; value = SX128X_RADIO_RAMP_10_US; }
  else if (ramp <= 12) { ramp = 12; value = SX128X_RADIO_RAMP_12_US; }
  else if (ramp <= 16) { ramp = 16; value = SX128X_RADIO_RAMP_16_US; }
  else                 { ramp = 20; value = SX128X_RADIO_RAMP_20_US; }

  self->pars->power = power = SX128X_LIMIT(power, -18, 13); // dBm
  self->pars->ramp = ramp;

  // Pout[dB] = -18 + power (page 88)
  SX128X_DBG("set TX power=%idBm rampTime=%ius", (int) power, (int) ramp);

  // set TX params
  self->txbuf[0] = SX128X_CMD_SET_TX_PARAMS;
  self->txbuf[1] = (uint8_t) (power + 18); // 0...31
  self->txbuf[2] = value;               // 0x00...0xE0

  return sx128x_spi(self, 3);
}
//-----------------------------------------------------------------------------
// set LNA boost on/off
// LNA boost increase current by ~0.7mA for around ~3dB in sensivity
int8_t sx128x_set_lna_boost(sx128x_t *self, uint8_t lna_boost)
{ // read 4.2.1 on page 30
  uint8_t boost;
  int8_t retv = sx128x_reg_read(self, SX128X_REG_RX_GAIN, &boost, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  if (lna_boost) boost |= 0xC0; // set bits 7:6
  else           boost &= 0x3F; // reset bits 7:6

  retv = sx128x_reg_write(self, SX128X_REG_RX_GAIN, &boost, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  self->pars->lna_boost = lna_boost;

  SX128X_DBG("set LNA boost %s", lna_boost ? "on" : "off");

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// set RX gain (AGC or manual gain)
int8_t sx128x_set_gain(
  sx128x_t *self,
  uint8_t gain) // RX gain: 0 - AGC on, 1 - min gain (max - 54dB), 13 - max gain
{ // read 4.2 (Table 4.1 and 4.2) on page 29
  int8_t retv;
  uint8_t gain_control; // reg[0x89F]
  uint8_t manual_gain;  // reg[0x895]

  if (gain)
  { // manual gain controll (AGC off)
    gain_control = 0x80; // bit 7 <= 1
    manual_gain  = 0x00; // bit 0 <= 0
    gain = SX128X_LIMIT(gain, 1, 13);

    retv = sx128x_reg_write(self, SX128X_REG_LNA_GAIN_VALUE, &gain, 1);
    if (retv != SX128X_ERR_NONE) return retv;
  }
  else
  { // AGC on
    gain_control = 0x00; // bit 7 <= 0
    manual_gain  = 0x01; // bit 0 <= 1
  }

  retv = sx128x_reg_write(self, SX128X_REG_LNA_GAIN_CONTROL, &gain_control, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  retv = sx128x_reg_write(self, SX128X_REG_MANUAL_GAIN, &manual_gain, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  self->pars->gain = gain;

#ifdef SX128X_USE_EXTRA
  SX128X_DBG("set RX gain=%i (%s, AGC %s)",
             (int) gain, sx128x_gain_string[gain], gain ? "off" : "on");
#else
  SX128X_DBG("set RX gain=%i (AGC %s)", (int) gain, gain ? "off" : "on");
#endif
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// set on/off DC-DC (regulator mode)
int8_t sx128x_set_dcdc(sx128x_t *self, uint8_t dcdc) // 0-off/1-on
{
  SX128X_DBG("set DC-DC %s", dcdc ? "on" : "off");
  self->pars->dcdc = dcdc;
  return sx128x_cmd_write(self,
                          SX128X_CMD_SET_REGULATOR_MODE,
                          dcdc ? SX128X_REGULATOR_DC_DC : SX128X_REGULATOR_LDO);
}
//-----------------------------------------------------------------------------
// set auto FS (enable = 0 or 1)
int8_t sx128x_auto_fs(sx128x_t *self, uint8_t enable)
{
  SX128X_DBG("set auto FS: enable=%i", (int) enable);
  self->pars->auto_fs = enable;
  return sx128x_cmd_write(self, SX128X_CMD_SET_AUTO_FS, enable);
}
//-----------------------------------------------------------------------------
// set buffer(s) base address (TX/RX)
int8_t sx128x_set_buffer(sx128x_t *self, uint8_t tx_base_addr, uint8_t rx_base_addr)
{
  self->txbuf[0] = SX128X_CMD_SET_BUFFER_BASE_ADDRESS;
  self->txbuf[1] = tx_base_addr;
  self->txbuf[2] = rx_base_addr;

  self->tx_addr = tx_base_addr;
  self->rx_addr = rx_base_addr;

  SX128X_DBG("set txBaseAddr=0x%02X rxBaseAddr=0x%02X",
             (unsigned) tx_base_addr, (unsigned) rx_base_addr);

  return sx128x_spi(self, 3);
}
//-----------------------------------------------------------------------------
// get RX buffer status
int8_t sx128x_get_rx_buffer(
  sx128x_t *self,
  uint8_t *status,          // SX128x status
  uint8_t *rx_payload_size, // rxPayloadLength
  uint8_t *rx_start_addr,   // rxStartBufferPointer
  uint8_t fixed)            // 1 - if LoRa Implicit Header, else 0
{
  int8_t retv;

  self->txbuf[0] = SX128X_CMD_GET_RX_BUFFER_STATUS;
  self->txbuf[1] = self->txbuf[2] = self->txbuf[3] = 0;

  *status = *rx_payload_size = *rx_start_addr = 0;

  retv = sx128x_spi(self, 4);
  if (retv != SX128X_ERR_NONE) return retv;

  *status          = self->rxbuf[1];
  *rx_payload_size = self->rxbuf[2];
  *rx_start_addr   = self->rxbuf[3];

#ifdef SX128X_USE_LORA
  if (self->pars->mode == SX128X_PACKET_TYPE_LORA && fixed)
  { // LoRa fixed packet size (implicit header) - note at page 92
    retv = sx128x_reg_read(self, SX128X_REG_LORA_PAYLOAD_LENGTH,
                           rx_payload_size, 1);
    if (retv != SX128X_ERR_NONE)
    {
      *rx_payload_size = 0;
      return retv;
    }
  }
#endif // SX128X_USE_LORA

  SX128X_DBG("get RxStartAddr=0x%02X RxPayloadSize=%u status=0x%02X",
             (unsigned) *rx_start_addr, (unsigned) *rx_payload_size,
             (unsigned) *status);

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
//-----------------------------------------------------------------------------
// get packet status (LoRa and Ranging)
int8_t sx128x_packet_status_lora(
  sx128x_t *self,
  uint8_t *status, // status
  uint8_t *rssi,   // RSSI of sync = -rssi/2 [dBm]
  int8_t *snr)     // SNR of packet = snr/4 [dB]
{
  int8_t retv;

  *status = *rssi = *snr = 0;

  if (self->pars->mode != SX128X_PACKET_TYPE_LORA &&
      self->pars->mode != SX128X_PACKET_TYPE_RANGING)
    return SX128X_ERR_BAD_CALL;

  self->txbuf[0] = SX128X_CMD_GET_PACKET_STATUS;
  self->txbuf[1] = self->txbuf[2] = self->txbuf[3] = 0;
  self->txbuf[4] = self->txbuf[5] = self->txbuf[6] = 0;

  retv = sx128x_spi(self, 7);
  if (retv != SX128X_ERR_NONE) return retv;

  *status = self->rxbuf[1];

  // Table 11-65: packetStatus Definition (page 93)
  // Note: In the case of LoRa and/or Ranging Engine,
  // there are only 2 bytes returned by the command
  // Table 11-66: RSSI and SNR Packet Status (page 93)

  *rssi =        self->rxbuf[2];
  *snr  = (int8_t) self->rxbuf[3];

#if 1 // FIXME
  if (*snr < 0)
    *rssi = *rssi - *snr * 2;
#endif

#ifdef SX128X_DEBUG_EXTRA
  if (1)
  {
    uint8_t mode, cmd;
    sx128x_status_unpack(*status, &mode, &cmd);

#ifdef SX128X_USE_EXTRA
    SX128X_DBG("packet Status=0x%02X: ChipMode=%s CmdStatus=%s",
               (int) *status,
               sx128x_status_mode_string[mode],
               sx128x_status_cmd_string[cmd]);
#else
    SX128X_DBG("packet Status=0x%02X: ChipMode=%i CmdStatus=%i",
               (unsigned) *status, (int) mode, (int) cmd);
#endif // SX128X_USE_EXTRA
  }
#endif // SX128X_DEBUG_EXTRA

  SX128X_DBG("packet RSSI=-%idBm/2 SNR=%idB/4", (int) *rssi, (int) *snr);
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// get LoRa Explicit Header (unpacked)
int8_t sx128x_get_hdr_lora(sx128x_t *self, sx128x_lora_hdr_t *hdr)
{
  int8_t retv;
  uint8_t reg = 0;

  hdr->fixed = 1;
  hdr->cr = 4;
  hdr->crc = 0;

  retv = sx128x_reg_read(self, SX128X_REG_LORA_HEADER_MODE, &reg, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  //SX128X_DBG("Header mode: reg[0x%03X]=0x%02X", SX128X_REG_LORA_HEADER_MODE, reg);

  if (!(reg & SX128X_LORA_IMPLICIT_HEADER))
  { // Explicit header
    hdr->fixed = 0;
    retv = sx128x_reg_read(self, SX128X_REG_LORA_INCOMING_CR, &reg, 1);
    if (retv != SX128X_ERR_NONE) return retv;
    hdr->cr = (reg >> 4) & 0x07; // 0x01...0x07

    retv = sx128x_reg_read(self, SX128X_REG_LORA_INCOMING_CRC, &reg, 1);
    if (retv != SX128X_ERR_NONE) return retv;
    hdr->crc = (reg >> 4) & 0x01;

    SX128X_DBG("LoRa Explicit header: CR=%i CRC=%s",
               (int) hdr->cr, hdr->crc ? "on" : "off");
  }

  return retv;
}
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
  uint8_t *pkt_sync)   // sync packet status   (Table 11-69: Sync Packet Status Byte)
{
  int8_t retv;

  *status = *rssi = 0;
  *pkt_status = *pkt_errors = *pkt_sync = 0;

  if (self->pars->mode != SX128X_PACKET_TYPE_FLRC &&
      self->pars->mode != SX128X_PACKET_TYPE_GFSK &&
      self->pars->mode != SX128X_PACKET_TYPE_BLE)
    return SX128X_ERR_BAD_CALL;

  self->txbuf[0] = SX128X_CMD_GET_PACKET_STATUS;
  self->txbuf[1] = self->txbuf[2] = self->txbuf[3] = 0;
  self->txbuf[4] = self->txbuf[5] = self->txbuf[6] = 0;

  retv = sx128x_spi(self, 7);
  if (retv != SX128X_ERR_NONE) return retv;

  // Table 11-65: packetStatus Definition (page 93)
  *status     = self->rxbuf[1];
  *rssi       = self->rxbuf[3];
  *pkt_errors = self->rxbuf[4];
  *pkt_status = self->rxbuf[5];
  *pkt_sync   = self->rxbuf[6];

#ifdef SX128X_DEBUG_EXTRA
  if (1)
  {
    uint8_t mode, cmd;
    sx128x_status_unpack(*status, &mode, &cmd);

#ifdef SX128X_USE_EXTRA
    SX128X_DBG("packet Status=0x%02X: ChipMode=%s CmdStatus=%s",
               (int) *status,
               sx128x_status_mode_string[mode],
               sx128x_status_cmd_string[cmd]);
#else
    SX128X_DBG("packet Status=0x%02X: ChipMode=%i CmdStatus=%i",
               (unsigned) *status, (int) mode, (int) cmd);
#endif // SX128X_USE_EXTRA
  }
#endif // SX128X_DEBUG_EXTRA

  SX128X_DBG("packet RSSI=-%idBm/2", (int) *rssi);
  SX128X_DBG("packet status=0x%02X pktSent=%i rxNoAck=%i",
             *pkt_status, !!(*pkt_status & (1<<0)),
                          !!(*pkt_status & (1<<5)));
  SX128X_DBG("packet errors=0x%02X AbortErr=%i CrcErr=%i LenErr=%i SyncErr=%i",
             *pkt_errors, !!(*pkt_errors & (1<<3)),
                          !!(*pkt_errors & (1<<4)),
                          !!(*pkt_errors & (1<<5)),
                          !!(*pkt_errors & (1<<6)));
  SX128X_DBG("packet sync_addrs=%i", ((int) *pkt_sync) & 0x7);
  return SX128X_ERR_NONE;
}
#endif //SX128X_USE_FLRC || SX128X_USE_GFSK || SX128X_USE_BLE
//-----------------------------------------------------------------------------
// set IRQ and DIO mask
int8_t sx128x_irq_dio_mask(sx128x_t *self,
                           uint16_t irq_mask,
                           uint16_t dio1_mask, uint16_t dio2_mask, uint16_t dio3_mask)
{
  SX128X_DBG("set IRQ and DIO mask: "
             "IRQm=0x%04X DIO1m=0x%04X DIO2m=0x%04X DIO3m=0x%04X",
             (unsigned) irq_mask,  (unsigned) dio1_mask,
             (unsigned) dio2_mask, (unsigned) dio3_mask);

  self->pars->irq_mask  = irq_mask;
  self->pars->dio1_mask = dio1_mask;
  self->pars->dio2_mask = dio2_mask;
  self->pars->dio2_mask = dio2_mask;

  self->txbuf[0] = SX128X_CMD_SET_DIO_IRQ_PARAMS;
  self->txbuf[1] = (irq_mask >>  8) & 0xFF;
  self->txbuf[2] = (irq_mask      ) & 0xFF;
  self->txbuf[3] = (dio1_mask >> 8) & 0xFF;
  self->txbuf[4] = (dio1_mask     ) & 0xFF;
  self->txbuf[5] = (dio2_mask >> 8) & 0xFF;
  self->txbuf[6] = (dio2_mask     ) & 0xFF;
  self->txbuf[7] = (dio3_mask >> 8) & 0xFF;
  self->txbuf[8] = (dio3_mask     ) & 0xFF;

  return sx128x_spi(self, 9);
}
//-----------------------------------------------------------------------------
// get IRQ status
int8_t sx128x_get_irq(sx128x_t *self, uint16_t *irq)
{
  int8_t retv;

  *irq = 0;
  self->txbuf[0] = SX128X_CMD_GET_IRQ_STATUS;
  self->txbuf[1] = self->txbuf[2] = self->txbuf[3] = 0;

  retv = sx128x_spi(self, 4);
  if (retv != SX128X_ERR_NONE) return retv;

  *irq = (((uint16_t) self->rxbuf[2]) << 8) |
          ((uint16_t) self->rxbuf[3]);

#ifdef SX128X_DEBUG_IRQ
  SX128X_DBG("get irqStatus=0x%04X:", (unsigned) *irq);
  SX128X_DBG("              TxDone=%i", !!(*irq & SX128X_IRQ_TX_DONE));
  SX128X_DBG("              RxDone=%i", !!(*irq & SX128X_IRQ_RX_DONE));
  SX128X_DBG("       SyncWordValid=%i", !!(*irq & SX128X_IRQ_SYNC_WORD_VALID));
  SX128X_DBG("       SyncWordError=%i", !!(*irq & SX128X_IRQ_SYNC_WORD_ERROR));
  SX128X_DBG("         HeaderValid=%i", !!(*irq & SX128X_IRQ_HEADER_VALID));
  SX128X_DBG("         HeaderError=%i", !!(*irq & SX128X_IRQ_HEADER_ERROR));
  SX128X_DBG("              CrcErr=%i", !!(*irq & SX128X_IRQ_CRC_ERROR));
  SX128X_DBG("   SlaveResponseDone=%i", !!(*irq & SX128X_IRQ_SLAVE_RESPONSE_DONE));
  SX128X_DBG(" SlaveRequestDiscard=%i", !!(*irq & SX128X_IRQ_SLAVE_REQUEST_DISCARD));
  SX128X_DBG("   MasterResultValid=%i", !!(*irq & SX128X_IRQ_MASTER_RESULT_VALID));
  SX128X_DBG("       MasterTimeout=%i", !!(*irq & SX128X_IRQ_MASTER_TIMEOUT));
  SX128X_DBG("   SlaveRequestValid=%i", !!(*irq & SX128X_IRQ_SLAVE_REQUEST_VALID));
  SX128X_DBG("             CadDone=%i", !!(*irq & SX128X_IRQ_CAD_DONE));
  SX128X_DBG("         CadDetected=%i", !!(*irq & SX128X_IRQ_CAD_DETECTED));
  SX128X_DBG("         RxTxTimeout=%i", !!(*irq & SX128X_IRQ_RX_TX_TIMEOUT));
  SX128X_DBG("    PreambleDetected=%i", !!(*irq & SX128X_IRQ_PREAMBLE_DETECTED));
//SX128X_DBG(" AdvancedRangingDone=%i", !!(*irq & SX128X_IRQ_ADVANCED_RANGING_DONE));
#else
  SX128X_DBG("get irqStatus=0x%04X", (unsigned) *irq);
#endif // SX128X_DEBUG_IRQ

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// clear IRQ status by mask
int8_t sx128x_clear_irq(sx128x_t *self, uint16_t mask)
{
#ifdef SX128X_DEBUG
#ifdef SX128X_DEBUG_IRQ
  SX128X_DBG("clear IRQ status by mask=0x%04X:", mask);
  if (mask & SX128X_IRQ_TX_DONE)               SX128X_DBG("  - TxDone");
  if (mask & SX128X_IRQ_RX_DONE)               SX128X_DBG("  - RxDone");
  if (mask & SX128X_IRQ_SYNC_WORD_VALID)       SX128X_DBG("  - SyncWordValid");
  if (mask & SX128X_IRQ_SYNC_WORD_ERROR)       SX128X_DBG("  - SyncWordError");
  if (mask & SX128X_IRQ_HEADER_VALID)          SX128X_DBG("  - HeaderValid");
  if (mask & SX128X_IRQ_HEADER_ERROR)          SX128X_DBG("  - HeaderError");
  if (mask & SX128X_IRQ_CRC_ERROR)             SX128X_DBG("  - CrcErr");
  if (mask & SX128X_IRQ_SLAVE_RESPONSE_DONE)   SX128X_DBG("  - SlaveResponseDone");
  if (mask & SX128X_IRQ_SLAVE_REQUEST_DISCARD) SX128X_DBG("  - SlaveRequestDiscard");
  if (mask & SX128X_IRQ_MASTER_RESULT_VALID)   SX128X_DBG("  - MasterResultValid");
  if (mask & SX128X_IRQ_MASTER_TIMEOUT)        SX128X_DBG("  - MasterTimeout");
  if (mask & SX128X_IRQ_SLAVE_REQUEST_VALID)   SX128X_DBG("  - SlaveRequestValid");
  if (mask & SX128X_IRQ_CAD_DONE)              SX128X_DBG("  - CadDone");
  if (mask & SX128X_IRQ_CAD_DETECTED)          SX128X_DBG("  - CadDetected");
  if (mask & SX128X_IRQ_RX_TX_TIMEOUT)         SX128X_DBG("  - RxTxTimeout");
  if (mask & SX128X_IRQ_PREAMBLE_DETECTED)     SX128X_DBG("  - PreambleDetected");
//if (mask & SX128X_IRQ_ADVANCED_RANGING_DONE) SX128X_DBG("  - AdvancedRangingDone");
#else
  SX128X_DBG("clear IRQ status by mask=0x%04X", mask);
#endif // SX128X_DEBUG_IRQ
#endif // SX128X_DEBUG

  self->txbuf[0] = SX128X_CMD_CLR_IRQ_STATUS;
  self->txbuf[1] = (mask >> 8) & 0xFF;
  self->txbuf[2] = (mask     ) & 0xFF;

  return sx128x_spi(self, 3);
}
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
//-----------------------------------------------------------------------------
// set modulation params in LoRa or Ranging mode
int8_t sx128x_mod_lora(
  sx128x_t *self,
  uint16_t bw, // Bandwith: 203, 406, 812, 1625 kHz (>=406 for Ranging)
  uint8_t  sf, // Spreading Factor: 5...12 (5...10 for Ranging)
  uint8_t  cr) // Code Rate: 1...7 {4/5, 4/6, 4/7, 4/8, 4/5*, 4/6*, 4/8*}
{
  static uint8_t sx128x_lora_sf_param[] = {
    SX128X_MOD_PARAM1_LORA_SF_5,
    SX128X_MOD_PARAM1_LORA_SF_6,
    SX128X_MOD_PARAM1_LORA_SF_7,
    SX128X_MOD_PARAM1_LORA_SF_8,
    SX128X_MOD_PARAM1_LORA_SF_9,
    SX128X_MOD_PARAM1_LORA_SF_10,
    SX128X_MOD_PARAM1_LORA_SF_11,
    SX128X_MOD_PARAM1_LORA_SF_12 };

  uint8_t sf_param, bw_param;
  int ranging = (self->pars->mode == SX128X_PACKET_TYPE_RANGING);

  // Table 14-48: Modulation Parameters in LoRa(R) Mode (page 131)
  if      (bw <=  300) { bw =  203; bw_param = SX128X_MOD_PARAM2_LORA_BW_200;  }
  else if (bw <=  600) { bw =  406; bw_param = SX128X_MOD_PARAM2_LORA_BW_400;  }
  else if (bw <= 1200) { bw =  812; bw_param = SX128X_MOD_PARAM2_LORA_BW_800;  }
  else                 { bw = 1625; bw_param = SX128X_MOD_PARAM2_LORA_BW_1600; }

  sf = SX128X_LIMIT(sf, 5, 12);

  // Table 14-49: Modulation Parameters in LoRa(R) Mode (page 131)
  cr = SX128X_LIMIT(cr, 1, 7);

  self->pars->bw = bw;
  self->pars->sf = sf;
  self->pars->cr = cr;

#ifdef SX128X_USE_RANGING
  if (ranging)
  { // Table 14-56: Ranging Device Modulation Parameters (page 136)
    if (bw < 406) { bw = 406; bw_param = SX128X_MOD_PARAM2_LORA_BW_400; }
    if (sf > 10) sf = 10;
    if      (cr == 5) cr = 1; // 4/5
    else if (cr == 6) cr = 2; // 4/6
    else              cr = 4; // 4/8
  }
#endif

  // Table 14-47: Modulation Parameters in LoRa(R) Mode (page 130)
  sf_param = sx128x_lora_sf_param[sf - 5];

  SX128X_DBG("set LoRa%s ModParam's: BW=%ikHz SF=%i CR=%i",
             ranging ? "-Ranging" : "",
             (int) bw, (int) sf, (int) cr);

  self->txbuf[0] = SX128X_CMD_SET_MODULATION_PARAMS;
  self->txbuf[1] = sf_param;
  self->txbuf[2] = bw_param;
  self->txbuf[3] = cr;

  return sx128x_spi(self, 4);
}
//-----------------------------------------------------------------------------
// pack preamble length to 1 byte (mant * 2^exp)
// (see page 132)
INLINE uint8_t sx128x_preamble_pack(uint32_t *preamble)
{
  uint32_t mant = *preamble;
  uint8_t exp = 0;
#if 0
  mant >>= 1;
  exp++;
#endif
  while (mant > 15)
  {
    mant >>= 1;
    if (++exp >= 15) break;
  }
  if (mant > 15) mant = 15;
  *preamble = mant * (1 << ((uint32_t) exp));
  return (exp << 4) | ((uint8_t) mant);
}
//-----------------------------------------------------------------------------
// set packet params in LoRa mode or Ranging mode
int8_t sx128x_packet_lora(
  sx128x_t *self,
  uint32_t preamble,     // Preamble length in symbols: 1..491520
  uint8_t  crc,          // CRC 0-off, 1-on, 2-software (8 bit)
  uint8_t  invert_iq,    // Invert IQ 1-on/0-off
  uint8_t  impl_hdr,     // 1-Implicit headr, 0-Explicit header
  uint8_t  payload_size) // Payload size 1..255
{
  uint8_t preamble_param = sx128x_preamble_pack(&preamble);

  // Table 14-52: Payload Length Definition in LoRa® Packet (page 132)
  // 1..255
  if      (payload_size == 0)               payload_size = 1;
  else if (payload_size == 255 && crc == 2) payload_size = 254;

  self->pars->preamble     = preamble;
  self->pars->crc          = crc <= 2 ? crc : 1;
  self->pars->invert_iq    = invert_iq;
  self->pars->fixed        = impl_hdr;
  self->pars->payload_size = payload_size;

#ifdef SX128X_USE_RANGING
  if (self->pars->mode == SX128X_PACKET_TYPE_RANGING)
  { // FIXME: why?
    impl_hdr = 0; // Ranging must start with Explicit header
    crc = 1;      // CRC on
  }
#endif

  SX128X_DBG("set LoRa PacketParam's: Preamble=%u CRC=%i "
             "InvertIQ=%i HeaderType=%s PayloadSize=%u",
             (unsigned) preamble, (int) crc,
             (int) invert_iq,
             impl_hdr ? "Implicit" : "Explicit",
             (unsigned) payload_size);

  self->pktpars[0] = SX128X_CMD_SET_PACKET_PARAMS;
  self->pktpars[1] = preamble_param;
  self->pktpars[2] = impl_hdr ? SX128X_LORA_IMPLICIT_HEADER : SX128X_LORA_EXPLICIT_HEADER;
  self->pktpars[3] = (crc == 2) ? payload_size + 1 : payload_size;
  self->pktpars[4] = (crc == 1) ? SX128X_LORA_CRC_ENABLE : SX128X_LORA_CRC_DISABLE;
  self->pktpars[5] = invert_iq ? SX128X_LORA_IQ_INVERTED : SX128X_LORA_IQ_STD;
  self->pktpars[6] = 0;
  self->pktpars[7] = 0;

  return sx128x_spi_pktpars(self);
}
//-----------------------------------------------------------------------------
// set Sync Word in LoRa mode or Ranging mode
//   sw=0x34 - public network
//   sw=0x12 - private network by default
int8_t sx128x_set_sw_lora(sx128x_t *self, uint8_t sw)
{
  int8_t retv;
  uint8_t buf[2];

  self->pars->lora_sw = sw;

  // see page 102 (Table 13-1: List of Registers) and 133
  retv = sx128x_reg_read(self, SX128X_REG_LORA_SYNC_WORD_MSB, buf, 2);
  if (retv != SX128X_ERR_NONE) return retv;

  buf[0] = (buf[0] & 0x0F) | (sw        & 0xF0);
  buf[1] = (buf[1] & 0x0F) | ((sw << 4) & 0xF0);

#if 0
  SX128X_DBG("set LoRa SyncWord=0x%02X%02X",
             (unsigned) buf[0], (unsigned) buf[1]);
#else
  SX128X_DBG("set LoRa SyncWord=0x%02X", (unsigned) sw);
#endif
  return sx128x_reg_write(self, SX128X_REG_LORA_SYNC_WORD_MSB, buf, 2);
}
//-----------------------------------------------------------------------------
// set LoRa CAD param's
// Notice: for symbols 1 & 2, there are higher risks of false detection
int8_t sx128x_set_cad_lora(sx128x_t *self, uint8_t sym_num) // sym_num: 1, 2, 4, 8, 16
{
  uint8_t value;

  // Table 11-52: CadSymbolNum Definition (page 89)
  if      (sym_num <= 1) { sym_num =  1; value = SX128X_LORA_CAD_01_SYMBOL;  }
  else if (sym_num <= 2) { sym_num =  2; value = SX128X_LORA_CAD_02_SYMBOLS; }
  else if (sym_num <= 4) { sym_num =  4; value = SX128X_LORA_CAD_04_SYMBOLS; }
  else if (sym_num <= 8) { sym_num =  8; value = SX128X_LORA_CAD_08_SYMBOLS; }
  else                   { sym_num = 16; value = SX128X_LORA_CAD_16_SYMBOLS; }

  self->pars->cad_sym_num = sym_num;

  SX128X_DBG("set LoRa CAD Param's: SymNum=%u", (unsigned) sym_num);

  return sx128x_cmd_write(self, SX128X_CMD_SET_CAD_PARAMS, value);
}
//-----------------------------------------------------------------------------
// get LoRa frequency error indicator (FEI) value [Hz]
// note: LoRa FEI is reliable only for positive SNR
// see page 135 datashet
int8_t sx128x_fei_lora(sx128x_t *self, int32_t *fei)
{
  int8_t retv;
  uint8_t buf[3];
  uint32_t fei_raw;
  int32_t fei_signed;
  int32_t bw = (int32_t) self->pars->bw; // 203, 406, 812, 1625
  *fei = 0;

  // see page 102 (Table 13-1: List of Registers) and page 135
  retv = sx128x_reg_read(self, SX128X_REG_LORA_FEI_BYTE2, buf, 3);
  if (retv != SX128X_ERR_NONE) return retv;

  fei_raw = (((uint32_t) buf[0]) << 16) |
            (((uint32_t) buf[1]) <<  8) |
             ((uint32_t) buf[2]);

  fei_signed = (int32_t) (fei_raw & 0x000FFFFF);
  if (fei_signed & 0x80000)
  { // FEI is negative
    fei_signed |= (int32_t) 0xFFF00000;
  }

  *fei = (fei_signed * 31) / ((20 * 1625) / (bw ? bw : 1625)); // 31/20 = 1.55

  SX128X_DBG("rawFEI=0x%06X signedFEI=%i FEI=%iHz",
             (unsigned) fei_raw, (int) fei_signed, (int) *fei);

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// get instantaneous RSSI during reception of the LoRa packet
// see page 95 datasheet
int8_t sx128x_rssi_lora(
  sx128x_t *self,
  uint8_t *rssi_inst) // signal power is -rssi_inst/2 dbm
{
  int8_t retv = sx128x_cmd_read(self, SX128X_CMD_GET_RSSI_INST, rssi_inst);
  if (retv != SX128X_ERR_NONE) return retv;

  SX128X_DBG("LoRa RSSI_inst=-%idBm/2", (int) *rssi_inst);
  return retv;
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_LORA || SX128X_USE_RANGING
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_RANGING
//-----------------------------------------------------------------------------
// set Ranging role (0x01-Master, 0x00-Slave)
int8_t sx128x_ranging_role(sx128x_t *self, uint8_t role)
{ // Table 14-61: Ranging Role Value (page 138)
  SX128X_DBG("set Ranging role=%i (%s)",
             (int) role, role ? "Master" : "Slave");
  self->pars->role = role = role ? 0x01 : 0x00;
  return sx128x_cmd_write(self, SX128X_CMD_SET_RANGING_ROLE, role);
}
//-----------------------------------------------------------------------------
// set Ranging master request address
int8_t sx128x_ranging_master_address(sx128x_t *self, uint32_t address)
{
  uint8_t buf[4];

  self->pars->master_address = address;

  SX128X_DBG("set Ranging MasterAddress=0x%08X", (unsigned) address);

  buf[0] = (address >> 24) & 0xFF; // byte 3
  buf[1] = (address >> 16) & 0xFF; // byte 2
  buf[2] = (address >>  8) & 0xFF; // byte 1
  buf[3] = (address      ) & 0xFF; // byte 0

  // Table 14-59: Master Ranging Request Address Definition (page 137)
  return sx128x_reg_write(self, SX128X_REG_RANGING_REQ_ADDR3, buf, 4);
}
//-----------------------------------------------------------------------------
// set Ranging slave respond address and bits check mode
int8_t sx128x_ranging_slave_address(
  sx128x_t *self,
  uint32_t address, // slave respond address
  uint8_t  mode)    // bits check mode: 0...3 {8, 16, 24, 32 bits}
{
  int8_t retv;
  uint8_t buf[4];

  if (mode > 3) mode = 3;

  self->pars->slave_address = address;
  self->pars->slave_mode = mode;

  SX128X_DBG("set Ranging SlaveAddress=0x%08X SlaveMode=%i",
             (unsigned) address, (int) mode);

  buf[0] = (address >> 24) & 0xFF; // byte 3
  buf[1] = (address >> 16) & 0xFF; // byte 2
  buf[2] = (address >>  8) & 0xFF; // byte 1
  buf[3] = (address      ) & 0xFF; // byte 0

  // Table 14-57: Slave Ranging Request Address Definition (page 137)
  retv = sx128x_reg_write(self, SX128X_REG_RANGING_DEV_ADDR3, buf, 4);
  if (retv != SX128X_ERR_NONE) return retv;


  // Table 14-58: Register Address Bit Definition (page 137)
  buf[0] = mode << 6;
  return sx128x_reg_write(self, SX128X_REG_RANGING_ID_CHECK_LEN, buf, 1);
}
//-----------------------------------------------------------------------------
// set ranging callibration register value (24 bit)
int8_t sx128x_ranging_set_calibration(sx128x_t *self, uint32_t callibration)
{ // Table 14-60: Calibration Value in Register (page 138)
  int8_t retv;
  uint8_t buf[3];

  buf[0] = (callibration >> 16) & 0xFF;
  buf[1] = (callibration >>  8) & 0xFF;
  buf[2] = (callibration      ) & 0xFF;

  retv = sx128x_reg_write(self, SX128X_REG_RANGING_CALIB_BYTE2, buf, 3);
  if (retv != SX128X_ERR_NONE) return retv;

  self->pars->calibration = callibration;

  SX128X_DBG("set Ranging Callibration=0x%08X", (unsigned) callibration);

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// get ranging callibration register value (24 bit)
int8_t sx128x_ranging_get_calibration(sx128x_t *self, uint32_t *callibration)
{ // Table 14-60: Calibration Value in Register (page 138)
  uint8_t buf[3];
  int8_t retv = sx128x_reg_read(self, SX128X_REG_RANGING_CALIB_BYTE2, buf, 3);
  if (retv != SX128X_ERR_NONE) return retv;

  *callibration = (((uint32_t) buf[0]) << 16) |
                  (((uint32_t) buf[1]) <<  8) |
                   ((uint32_t) buf[2]);

  SX128X_DBG("get Ranging Callibration=0x%08X", (unsigned) *callibration);

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// get Ranging results
int8_t sx128x_ranging_result(
  sx128x_t *self,
  uint8_t  *filter,   // result type (0-off, 1-on, 2-as-is) - modified
  uint32_t *result,   // raw result
  int32_t *distance, // distance [dm]
  uint8_t  *rssi)     // RSSI of last exchange
{
  uint8_t buf[3], reg;
  int8_t retv;
  uint32_t bw = (uint32_t) self->pars->bw;
  int32_t result_signed;

  if (bw < 400) bw = 406; // kHz

  *result = *distance = 0;
  *rssi = 0;

  // read the ranging resul (page 139)
  // 1. set the radio in Oscillator mode:
  //    SetStandby( STDBY_XOSC)
  retv = sx128x_standby(self, SX128X_STANDBY_XOSC);
  if (retv != SX128X_ERR_NONE) return retv;

  // 2. enable clock in LoRa memory (freeze Ranging result):
  //    WriteRegister(0x97F, ReadRegister(0x97F) | (1 << 1));
  retv = sx128x_reg_read(self, SX128X_REG_FREEZE_RANGING_RESULT, &reg, 1);
  if (retv != SX128X_ERR_NONE) return retv;
  reg |= 1 << 1;
  retv = sx128x_reg_write(self, SX128X_REG_FREEZE_RANGING_RESULT, &reg, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  if (*filter < 2)
  { // filter == 0 or 1
    // 3. set the ranging result type and read the ranging registers as usual:
    //    WriteRegister(0x0924, (ReadRegister(0x0924) & 0xCF) | (((mux & 0x03) << 4));
    retv = sx128x_reg_read(self, SX128X_REG_RANGING_RESULT_MUX, &reg, 1);
    if (retv != SX128X_ERR_NONE) return retv;
    reg = (reg & 0xCF) | (*filter << 4);
    retv = sx128x_reg_write(self, SX128X_REG_RANGING_RESULT_MUX, &reg, 1);
    if (retv != SX128X_ERR_NONE) return retv;
  }

  // 3.1 read ranging result (24 bit value)
  retv = sx128x_reg_read(self, SX128X_REG_RANGING_RESULT_BYTE2, buf, 3);
  if (retv != SX128X_ERR_NONE) return retv;

  *result = (((uint32_t) buf[0]) << 16) |
            (((uint32_t) buf[1]) <<  8) |
             ((uint32_t) buf[2]);

  // 4. set the transceiver to Standby mode:
  //    SetStandby(STDBY_RC)
  retv = sx128x_standby(self, SX128X_STANDBY_RC);
  if (retv != SX128X_ERR_NONE) return retv;

  // 24 bit signed -> 32 bit signed
  result_signed = (int32_t) *result;
  if (result_signed & 0x00800000) result_signed |= 0xFF000000;

  // Table 14-63: Ranging Result Type Selection (page 139)
#if 0 // double - is bad idea
  *distance = (int) (((double) result_signed) * 150. /
                     ((double) (1<<12) * bw * 1e-3) * 10.); // dm
#else // It's work! (only 32-bit integer arithmetic)
  // note: (1625 << 10) / 8000 = 208
  //        150 * 10 / 4 = 375
  if      (result_signed > INT_MAX / 375) result_signed = INT_MAX / 375 - 1;
  else if (result_signed < INT_MIN / 375) result_signed = INT_MAX / 375 + 1;
  *distance = (((result_signed * 375) / 208) * (1625 / (int) bw)) / 8; // dm
#endif

  if (*filter >= 2)
  { // read ranging result MUX
    retv = sx128x_reg_read(self, SX128X_REG_RANGING_RESULT_MUX, &reg, 1);
    if (retv != SX128X_ERR_NONE) return retv;

    *filter = (reg >> 4) & 0x3;
  }

  // read RSSI value of the last Ranging exchange
  retv = sx128x_reg_read(self, SX128X_REG_RANGING_RSSI, rssi, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  SX128X_DBG("get Ranging result: Raw=0x%08X Signed=%i Distance=%idm "
             "Filter=%u RSSI=-%idBm/2",
             (unsigned) *result, (int) result_signed, (int) *distance,
             (unsigned) *filter, (int) *rssi);

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// on/off Advanced Ranging (0-off, 1-on)
int8_t sx128x_set_advanced_ranging(sx128x_t *self, uint8_t on)
{ // 14.6.1 Advanced Ranging Mode Operation (page 142)
  if (on)
  { // enable the advanced ranging mode
    // 1. set Slave mode
    int8_t retv = sx128x_ranging_role(self, 0x00);
    if (retv != SX128X_ERR_NONE) return retv;

    // 2. set Advanced Ranging (by writing 0x01 to opcode 0x9A)
    retv = sx128x_cmd_write(self, SX128X_CMD_SET_ADVANCED_RANGING, 0x01);
    if (retv != SX128X_ERR_NONE) return retv;

    // [save activated state]
    self->pars->advanced_ranging = 1;

    // 3. enable the AdvancedRangingDone interrupt
    retv = sx128x_irq_dio_mask(
             self,
             self->pars->irq_mask | SX128X_IRQ_ADVANCED_RANGING_DONE,
             //!!!SX128X_IRQ_ADVANCED_RANGING_DONE,
             self->pars->dio1_mask,
             self->pars->dio2_mask,
             self->pars->dio3_mask);
    if (retv != SX128X_ERR_NONE) return retv;

    // 3.1 clear AdvancedRanging IRQ status by mask (clear All)
    retv = sx128x_clear_irq(self, SX128X_IRQ_ALL);
    if (retv != SX128X_ERR_NONE) return retv;

    // 4. set continuous RX mode
    retv = sx128x_rx(self, SX128X_RX_TIMEOUT_CONTINUOUS,
                           SX128X_TIME_BASE_15_625US);
    if (retv != SX128X_ERR_NONE) return retv;

    // 5. return and wait AdvancedRangingDone interrupt is raised
  }
  else
  { // exiting advanced ranging mode
    // 1. exit advanced ranging monitoring: SetStandby(STDBY_RC)
    int8_t retv = sx128x_standby(self, SX128X_STANDBY_RC);
    if (retv != SX128X_ERR_NONE) return retv;

    // 2. Deactivate advanced ranging mode: write 0x00 to opcode 0x9A
    retv = sx128x_cmd_write(self, SX128X_CMD_SET_ADVANCED_RANGING, 0x00);
    if (retv != SX128X_ERR_NONE) return retv;

    // [save deactivated state]
    self->pars->advanced_ranging = 0;

    // 2.1 restore interrupt mask
    retv = sx128x_irq_dio_mask(self,
                               self->pars->irq_mask & (~SX128X_IRQ_ADVANCED_RANGING_DONE),
                               //SX128X_IRQ_ALL & (~SX128X_IRQ_ADVANCED_RANGING_DONE),
                               self->pars->dio1_mask,
                               self->pars->dio2_mask,
                               self->pars->dio3_mask);
    if (retv != SX128X_ERR_NONE) return retv;

    // 2.2 clear IRQ status by mask (clear All)
    retv = sx128x_clear_irq(self, SX128X_IRQ_ALL);
    if (retv != SX128X_ERR_NONE) return retv;

    // 2.3 restore Ranging role
    retv = sx128x_ranging_role(self, self->pars->role);
    if (retv != SX128X_ERR_NONE) return retv;
  }

  SX128X_DBG("set Advanced Ranging %s", on ? "On" : "Off");
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// get Advanced Ranging Address Received
int8_t sx128x_advanced_ranging_address(sx128x_t *self, uint32_t *address)
{ // 5. To read the RangingAddressReceived of the exchange,
  // the 4 byte result must be multiplexed out from registers 0x960 and 0x95F
  // (page 142)
  int8_t retv;
  uint8_t reg0x927, reg0x960, reg0x95F;
  *address = 0;

  // WriteRegister(0x927, ReadRegister(0x927) & 0xFC)
  retv = sx128x_reg_read(self, 0x927, &reg0x927, 1);
  if (retv != SX128X_ERR_NONE) return retv;
  reg0x927 &= 0xFC;
  retv = sx128x_reg_write(self, 0x927, &reg0x927, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  // uint32_t RangingAddressReceived = ReadRegister(0x960)
  retv = sx128x_reg_read(self, 0x960, &reg0x960, 1);
  if (retv != SX128X_ERR_NONE) return retv;
  *address = (uint32_t) reg0x960;

  // RangingAddressReceived |= (ReadRegister(0x95F) << 8)
  retv = sx128x_reg_read(self, 0x95F, &reg0x95F, 1);
  if (retv != SX128X_ERR_NONE) return retv;
  *address |= ((uint32_t) reg0x95F) << 8;

  // WriteRegister(0x927, (ReadRegister(0x927) & 0xFC) | 0x01)
  retv = sx128x_reg_read(self, 0x927, &reg0x927, 1);
  if (retv != SX128X_ERR_NONE) return retv;
  reg0x927 = (reg0x927 & 0xFC) | 0x01;
  retv = sx128x_reg_write(self, 0x927, &reg0x927, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  // RangingAddressReceived |= (ReadRegister(0x960) << 16)
  retv = sx128x_reg_read(self, 0x960, &reg0x960, 1);
  if (retv != SX128X_ERR_NONE) return retv;
  *address |= ((uint32_t) reg0x960) << 16;

  // RangingAddressReceived |= (ReadRegister(0x95F) << 24)
  retv = sx128x_reg_read(self, 0x95F, &reg0x95F, 1);
  if (retv != SX128X_ERR_NONE) return retv;
  *address |= ((uint32_t) reg0x95F) << 24;

  SX128X_DBG("get Advanced Ranging received Address=0x%08X",
             (unsigned) *address);
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_RANGING
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_FLRC
//-----------------------------------------------------------------------------
// set modulation params in FLRC mode
int8_t sx128x_mod_flrc(
  sx128x_t *self,
  uint16_t br, // Bitrate (BR): 260, 325, 520, 650, 1040, 1300 kBit/s
  uint8_t  cr, // Code Rate: 1...3 {1, 3/4, 1/2}
  uint8_t  bt) // Gaussian Filtering: 0...2 {off, 0.5, 1.0}
{
  uint8_t br_param, cr_param, bt_param;

  // Table 14-31: Modulation Parameters in FLRC Mode: Bandwidth and Bit Rate
  if      (br <=  290) { br =  260; br_param = SX128X_FLRC_BR_0_260_BW_0_3; }
  else if (br <=  420) { br =  325; br_param = SX128X_FLRC_BR_0_325_BW_0_3; }
  else if (br <=  585) { br =  520; br_param = SX128X_FLRC_BR_0_520_BW_0_6; }
  else if (br <=  850) { br =  650; br_param = SX128X_FLRC_BR_0_650_BW_0_6; }
  else if (br <= 1170) { br = 1040; br_param = SX128X_FLRC_BR_1_040_BW_1_2; }
  else                 { br = 1300; br_param = SX128X_FLRC_BR_1_300_BW_1_2; }

  // Table 14-32: Modulation Parameters in FLRC Mode: Coding Rate (page 122)
  cr = SX128X_LIMIT(cr, 1, 3);
  if      (cr == 1) cr_param = SX128X_FLRC_CR_1_1; // CR=1
  else if (cr == 2) cr_param = SX128X_FLRC_CR_3_4; // CR=3/4
  else              cr_param = SX128X_FLRC_CR_1_2; // CR=1/2

  // Table 14-33: Modulation Parameters in FLRC Mode: BT (page 122)
  if (bt > 2) bt = 1;
  if      (bt == 0) bt_param = SX128X_BT_OFF; // no filtering
  else if (bt == 1) bt_param = SX128X_BT_0_5; // BT=0.5
  else              bt_param = SX128X_BT_1_0; // BT=1

  self->pars->flrc_br = br;
  self->pars->flrc_cr = cr;
  self->pars->flrc_bt = bt;

  SX128X_DBG("set FLRC ModParam's: BR=%ukb/s CR=%u BT=%u",
             (unsigned) br, (unsigned) cr, (unsigned) bt);

  self->txbuf[0] = SX128X_CMD_SET_MODULATION_PARAMS;
  self->txbuf[1] = br_param;
  self->txbuf[2] = cr_param;
  self->txbuf[3] = bt_param;

  return sx128x_spi(self, 4);
}
//-----------------------------------------------------------------------------
// set packet params in FLRC mode
int8_t sx128x_packet_flrc(
  sx128x_t *self,
  uint16_t preamble,     // Preamble length: 8, 12, 16, 20, 24, 28, 32
  uint8_t  sw_on,        // SyncWord: 0/1 {no sync, 32-bits SW}
  uint8_t  sw_mode,      // SyncWord mode: 0..7 {off, sw1, sw2, sw12, sw3, sw13, sw23, sw123}
  uint8_t  crc,          // CRC mode: 0..3 {off, 2 bytes, 3 bytes, 4 bytes}
  uint8_t  fixed,        // 1-fixed length mode, 0-variable length mode
  uint8_t  payload_size) // Payload size 6..127
{
  static uint8_t sx128x_packet_flrc_sw_mode[] = {
    SX128X_RX_DISABLE_SYNC_WORD,
    SX128X_RX_MATCH_SYNC_WORD_1,
    SX128X_RX_MATCH_SYNC_WORD_2,
    SX128X_RX_MATCH_SYNC_WORD_1_2,
    SX128X_RX_MATCH_SYNC_WORD_3,
    SX128X_RX_MATCH_SYNC_WORD_1_3,
    SX128X_RX_MATCH_SYNC_WORD_2_3,
    SX128X_RX_MATCH_SYNC_WORD_1_2_3 };

  static uint8_t sx128x_packet_flrc_crc[] = {
    SX128X_FLRC_CRC_OFF,
    SX128X_FLRC_CRC_2_BYTE,
    SX128X_FLRC_CRC_3_BYTE,
    SX128X_FLRC_CRC_4_BYTE };

  uint8_t preamble_param, sw_param, sw_mode_param, crc_param;

  // Table 14-34: AGC Preamble Length Definition in FLRC Packet (page 122, 123)
  if      (preamble <=  9) { preamble =  8; preamble_param = SX128X_PREAMBLE_LENGTH_8_BITS;  }
  else if (preamble <= 14) { preamble = 12; preamble_param = SX128X_PREAMBLE_LENGTH_12_BITS; }
  else if (preamble <= 18) { preamble = 16; preamble_param = SX128X_PREAMBLE_LENGTH_16_BITS; }
  else if (preamble <= 22) { preamble = 20; preamble_param = SX128X_PREAMBLE_LENGTH_20_BITS; }
  else if (preamble <= 26) { preamble = 24; preamble_param = SX128X_PREAMBLE_LENGTH_24_BITS; }
  else if (preamble <= 30) { preamble = 28; preamble_param = SX128X_PREAMBLE_LENGTH_28_BITS; }
  else                     { preamble = 32; preamble_param = SX128X_PREAMBLE_LENGTH_32_BITS; }

  // Table 14-35: Sync Word Length Definition in FLRC Packet (page 123)
  sw_param = sw_on ? SX128X_FLRC_SYNC_WORD_LEN_P32S : SX128X_FLRC_SYNC_NOSYNC;

  // Table 14-37: Packet Type Definition in FLRC Packet (page 124)
  if (sw_mode > 7) sw_mode = 7;
  sw_mode_param = sx128x_packet_flrc_sw_mode[sw_mode];

  // Table 14-39: CRC Definition in FLRC Packet (page 124)
  if (crc > 3) crc = 3;
  crc_param = sx128x_packet_flrc_crc[crc];

  // Table 14-38: Payload Length Definition in FLRC Packet (page 124)
  // 6...127
  payload_size = SX128X_LIMIT(payload_size, 6, 127);

  self->pars->flrc_preamble  = preamble;
  self->pars->flrc_sw_on     = sw_on;
  self->pars->flrc_sw_mode   = sw_mode;
  self->pars->flrc_crc       = crc;
  self->pars->fixed          = fixed;
  self->pars->payload_size   = payload_size;

  SX128X_DBG("set FLRC PacketParam's: Preamble=%u SW_on=%i SW_mode=%i "
             "CRC=%i Fixed=%i PayloadSize=%u",
             (unsigned) preamble, (int) sw_on, (int) sw_mode,
             (int) crc, (int) fixed, (unsigned) payload_size);

  self->pktpars[0] = SX128X_CMD_SET_PACKET_PARAMS;
  self->pktpars[1] = preamble_param;
  self->pktpars[2] = sw_param;
  self->pktpars[3] = sw_mode_param;
  self->pktpars[4] = fixed ? SX128X_PACKET_FIXED_LENGTH :
                             SX128X_PACKET_VARIABLE_LENGTH;
  self->pktpars[5] = payload_size;
  self->pktpars[6] = crc_param;
  self->pktpars[7] = SX128X_WHITENING_OFF; // Table 14-41 (page 125)

  return sx128x_spi_pktpars(self);
}
//-----------------------------------------------------------------------------
// set Sync Word[1...3] in FLRC mode
int8_t sx128x_set_sw_flrc(
  sx128x_t *self,
  uint8_t  ix, // SyncWord index 0...2
  uint32_t sw) // SyncWord value [4 bytes]
{
  // Table 14-42: Sync Word Definition in FLRC Packet (page 125, 126)
  static uint16_t addr[] = { SX128X_REG_SYNC_ADDRESS1_BYTE3,
                             SX128X_REG_SYNC_ADDRESS2_BYTE3,
                             SX128X_REG_SYNC_ADDRESS3_BYTE3 };
  uint8_t buf[4];
  if (ix > 2) ix = 0;

  buf[0] = (sw >> 24) & 0xFF; // byte 3
  buf[1] = (sw >> 16) & 0xFF; // byte 2
  buf[2] = (sw >>  8) & 0xFF; // byte 1
  buf[3] = (sw      ) & 0xFF; // byte 0

  self->pars->flrc_sw[ix] = sw;

  SX128X_DBG("set FLRC SyncWord%i=0x%08X", (int) (ix + 1), (unsigned) sw);

  return sx128x_reg_write(self, addr[ix], buf, 4);
}
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
  uint8_t  bt)  // Shaping Filtering: 0...2 {off, 0.5, 1.0}
{
  uint8_t ix, bt_param;

  // Table 14-1: Modulation Parameters in GFSK Mode (page 105)
  static uint8_t sx128x_mod_gfsk_br[][2] = {
    { SX128X_GFSK_BLE_BR_0_125_BW_0_3, SX128X_GFSK_BLE_BR_0_125_BW_0_3 }, // 125kb/s
    { SX128X_GFSK_BLE_BR_0_250_BW_0_3, SX128X_GFSK_BLE_BR_0_250_BW_0_6 }, // 250kb/s
    { SX128X_GFSK_BLE_BR_0_400_BW_0_6, SX128X_GFSK_BLE_BR_0_400_BW_1_2 }, // 400kb/s
    { SX128X_GFSK_BLE_BR_0_500_BW_0_6, SX128X_GFSK_BLE_BR_0_500_BW_1_2 }, // 500kb/s
    { SX128X_GFSK_BLE_BR_0_800_BW_1_2, SX128X_GFSK_BLE_BR_0_800_BW_2_4 }, // 800kb/s
    { SX128X_GFSK_BLE_BR_1_000_BW_1_2, SX128X_GFSK_BLE_BR_1_000_BW_2_4 }, // 1000kb/s
    { SX128X_GFSK_BLE_BR_1_600_BW_2_4, SX128X_GFSK_BLE_BR_1_600_BW_2_4 }, // 1600kb/s
    { SX128X_GFSK_BLE_BR_2_000_BW_2_4, SX128X_GFSK_BLE_BR_2_000_BW_2_4 }  // 2000kb/s
  };

  if      (br <  190) { br =  125; ix = 0; }
  else if (br <  325) { br =  250; ix = 1; }
  else if (br <  450) { br =  400; ix = 2; }
  else if (br <  650) { br =  500; ix = 3; }
  else if (br <  900) { br =  800; ix = 4; }
  else if (br < 1300) { br = 1000; ix = 5; }
  else if (br < 1800) { br = 1600; ix = 6; }
  else                { br = 2000; ix = 7; }

  // Table 14-2: Modulation Index Parameters in GFSK Mode (page 105)
  if (mi > 15) mi = 15;

  // Table 14-3: Modulation Shaping Parameters in GFSK Mode (page 106)
  if (bt > 2) bt = 1;
  if      (bt == 0) bt_param = SX128X_BT_OFF; // no filtering
  else if (bt == 1) bt_param = SX128X_BT_0_5; // BT=0.5
  else              bt_param = SX128X_BT_1_0; // BT=1

  self->pars->gfsk_br  = br;
  self->pars->gfsk_dsb = dsb = !!dsb;
  self->pars->gfsk_mi  = mi;
  self->pars->gfsk_bt  = bt;

  SX128X_DBG("set GFSK Param's: BR=%ukb/s DSB=%u MI=%i BT=%u",
             (unsigned) br, (unsigned) dsb, (unsigned) mi, (unsigned) bt);

  self->txbuf[0] = SX128X_CMD_SET_MODULATION_PARAMS;
  self->txbuf[1] = sx128x_mod_gfsk_br[ix][dsb];
  self->txbuf[2] = mi;
  self->txbuf[3] = bt_param;

  return sx128x_spi(self, 4);
}
//-----------------------------------------------------------------------------
// set packet params in GFSK mode
int8_t sx128x_packet_gfsk(
  sx128x_t *self,
  uint16_t preamble,      // Preamble length: 4, 8, 12, 16, 20, 24, 28, 32
  uint8_t  sw_len,        // SyncWord length: 1...5
  uint8_t  sw_mode,       // SyncWord mode: 0..7 {off, 1->1, 2->2, 1->12, 3->3, 1->13, 1->23, 1->123}
  uint8_t  crc,           // CRC mode: 0...3 {off, 1 byte, 2 bytes, 3 bytes}
  uint8_t  whitening,     // Whitening: 0-disable, 1-enable
  uint8_t  fixed,         // 1-fixed length mode, 0-variable length mode
  uint8_t  payload_size)  // Payload size 0..255
{
  static uint8_t sx128x_packet_gfsk_sw_len[] = {
    SX128X_GFSK_SYNC_WORD_LEN_1_B,   // 1 byte SyncWord
    SX128X_GFSK_SYNC_WORD_LEN_2_B,   // 2 bytes SyncWord
    SX128X_GFSK_SYNC_WORD_LEN_3_B,   // 3 bytes SyncWord
    SX128X_GFSK_SYNC_WORD_LEN_4_B,   // 4 bytes SyncWord
    SX128X_GFSK_SYNC_WORD_LEN_5_B }; // 5 bytes SyncWoed

  static uint8_t sx128x_packet_gfsk_sw_mode[] = {
    SX128X_GFSK_SELECT_SYNCWORD_OFF,     // off
    SX128X_GFSK_SELECT_SYNCWORD_1,       // 1->1
    SX128X_GFSK_SELECT_SYNCWORD_2,       // 2->2
    SX128X_GFSK_SELECT_SYNCWORD_1_2,     // 1->12
    SX128X_GFSK_SELECT_SYNCWORD_3,       // 3->3
    SX128X_GFSK_SELECT_SYNCWORD_1_3,     // 1->13
    SX128X_GFSK_SELECT_SYNCWORD_2_3,     // 1->23
    SX128X_GFSK_SELECT_SYNCWORD_1_2_3 }; // 1->123

  static uint8_t sx128x_packet_gfsk_crc[] = {
    SX128X_GFSK_CRC_OFF,       // CRC off
    SX128X_GFSK_CRC_1_BYTE,    // CRC 1 byte
    SX128X_GFSK_CRC_2_BYTES,   // CRC 2 bytes
    SX128X_GFSK_CRC_3_BYTES }; // CRC 3 bytes (undocumented)

  uint8_t preamble_param, sw_param, sw_mode_param, crc_param;

  // Table 14-4: Preamble Length Definition in GFSK Packet (page 106)
  if      (preamble <=  6) { preamble =  4; preamble_param = SX128X_PREAMBLE_LENGTH_4_BITS;  }
  else if (preamble <=  9) { preamble =  8; preamble_param = SX128X_PREAMBLE_LENGTH_8_BITS;  }
  else if (preamble <= 14) { preamble = 12; preamble_param = SX128X_PREAMBLE_LENGTH_12_BITS; }
  else if (preamble <= 18) { preamble = 16; preamble_param = SX128X_PREAMBLE_LENGTH_16_BITS; }
  else if (preamble <= 22) { preamble = 20; preamble_param = SX128X_PREAMBLE_LENGTH_20_BITS; }
  else if (preamble <= 26) { preamble = 24; preamble_param = SX128X_PREAMBLE_LENGTH_24_BITS; }
  else if (preamble <= 30) { preamble = 28; preamble_param = SX128X_PREAMBLE_LENGTH_28_BITS; }
  else                     { preamble = 32; preamble_param = SX128X_PREAMBLE_LENGTH_32_BITS; }

  // Table 14-5: Sync Word Length Definition in GFSK Packet (page 107)
  sw_len = SX128X_LIMIT(sw_len, 1, 5);
  sw_param = sx128x_packet_gfsk_sw_len[sw_len - 1];

  // Table 14-6: Sync Word Combination in GFSK Packet (page 107)
  if (sw_mode > 7) sw_mode = 0;
  sw_mode_param = sx128x_packet_gfsk_sw_mode[sw_mode];

  // Table 14-9: CRC Definition in GFSK Packet
  if (crc > 3) crc = 3;
  crc_param = sx128x_packet_gfsk_crc[crc];

  self->pars->gfsk_preamble  = preamble;
  self->pars->gfsk_sw_len    = sw_len;
  self->pars->gfsk_sw_mode   = sw_mode;
  self->pars->gfsk_crc       = crc;
  self->pars->gfsk_whitening = whitening;
  self->pars->fixed          = fixed;
  self->pars->payload_size   = payload_size;

  SX128X_DBG("set GFSK PacketParam's: Preamble=%u SW_len=%i SW_mode=%i "
             "CRC=%i Whitening=%i Fixed=%i PayloadSize=%u",
             (unsigned) preamble, (int) sw_len, (int) sw_mode,
             (int) crc, (int) whitening,
             (int) fixed, (unsigned) payload_size);

  self->pktpars[0] = SX128X_CMD_SET_PACKET_PARAMS;
  self->pktpars[1] = preamble_param;
  self->pktpars[2] = sw_param;
  self->pktpars[3] = sw_mode_param;
  self->pktpars[4] = fixed ? SX128X_PACKET_FIXED_LENGTH :
                             SX128X_PACKET_VARIABLE_LENGTH;
  self->pktpars[5] = payload_size;
  self->pktpars[6] = crc_param;
  self->pktpars[7] = whitening ? SX128X_WHITENING_ON : SX128X_WHITENING_OFF;

  return sx128x_spi_pktpars(self);
}
//-----------------------------------------------------------------------------
// set Sync Word[1...3] in GFSK mode
int8_t sx128x_set_sw_gfsk(
  sx128x_t *self,
  uint8_t ix,          // SyncWord index 0...2
  const uint8_t sw[5]) // SyncWord value [5 bytes]
{
  // Table 14-11: Sync Word Definition in GFSK Packet (page 108, 109)
  static uint16_t addr[] = { SX128X_REG_SYNC_ADDRESS1_BYTE4,
                          SX128X_REG_SYNC_ADDRESS2_BYTE4,
                          SX128X_REG_SYNC_ADDRESS3_BYTE4 };
  int i;
  if (ix > 2) ix = 0;

  for (i = 0; i < 5; i++) self->pars->gfsk_sw[ix][i] = sw[i];

  SX128X_DBG("set GFSK SyncWord%i=%02X:%02X:%02X:%02X:%02X",
             (int) (ix + 1),
             (int) sw[0], (int) sw[1], (int) sw[2], (int) sw[3], (int) sw[4]);

  return sx128x_reg_write(self, addr[ix], sw, 5);
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_GFSK
//-----------------------------------------------------------------------------
#if defined(SX128X_USE_FLRC) || defined(SX128X_USE_GFSK)
//-----------------------------------------------------------------------------
// set Sync Words Tolerance (GFSK/FLRC)
int8_t sx128x_set_swt(sx128x_t *self, uint8_t tolerance) // 0...15
{
  uint8_t reg;
  int8_t retv;
  if (tolerance > 0xF) return SX128X_ERR_BAD_ARG;

  retv = sx128x_reg_read(self, SX128X_REG_SYNCH_ADDRESS_CONTROL, &reg, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  SX128X_DBG("old FLRC/GFSK SyncWords Tolerance=%i", (int) (reg & 0x0F));

  reg = (reg & 0xF0) | tolerance;

  retv = sx128x_reg_write(self, SX128X_REG_SYNCH_ADDRESS_CONTROL, &reg, 1);
  if (retv != SX128X_ERR_NONE) return retv;

  SX128X_DBG("set FLRC/GFSK SyncWords Tolerance=%i", (int) tolerance);
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_FLRC || SX128X_USE_GFSK
//-----------------------------------------------------------------------------
#ifdef SX128X_USE_BLE
//-----------------------------------------------------------------------------
// set modulation params in BLE mode
int8_t sx128x_mod_ble(sx128x_t *self)
{
  SX128X_DBG("set BLE Param's to standart values: BR=1000kb/s");
  self->txbuf[0] = SX128X_CMD_SET_MODULATION_PARAMS;
  self->txbuf[1] = SX128X_GFSK_BLE_BR_1_000_BW_1_2; // BR=1000kb/s BW=1200kHz
  self->txbuf[2] = SX128X_MOD_IND_0_50;             // ModIndex=0.5
  self->txbuf[3] = SX128X_BT_0_5;                   // BT=0.5
  return sx128x_spi(self, 4);
}
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
  uint8_t state,     // ConnectionState: 0...3
  uint8_t test,      // TestPacket: 0...7
  uint8_t crc,       // CRC: 0-off (no Bluetooth compatibility), 1-on (3 bytes)
  uint8_t whitening) // Whitening: 0-disable, 1-enable
{
  static uint8_t sx128x_packet_ble_state[] = {
    SX128X_BLE_PAYLOAD_LENGTH_MAX_31_BYTES,    // payload_size <=  31 (4.1)
    SX128X_BLE_PAYLOAD_LENGTH_MAX_37_BYTES,    // payload_size <=  37 (4.1)
    SX128X_BLE_TX_TEST_MODE,                   // payload_size <=  63 (4.1)
    SX128X_BLE_PAYLOAD_LENGTH_MAX_255_BYTES }; // payload_size <= 255 (4.2)

  static uint8_t sx128x_packet_ble_test[] = {
    SX128X_BLE_PRBS_9,         // pseudo random based on 9th polynominal
    SX128X_BLE_EYELONG_1_0,    // '11110000’
    SX128X_BLE_EYESHORT_1_0,   // ‘10101010'
    SX128X_BLE_PRBS_15,        // pseudo random based on 15th polynominal
    SX128X_BLE_ALL_1,          // '11111111'
    SX128X_BLE_ALL_0,          // '00000000'
    SX128X_BLE_EYELONG_0_1,    // '00001111'
    SX128X_BLE_EYESHORT_0_1 }; // '01010101'

  if (state > 3) state = 3;
  if (test  > 7) test  = 0;

  self->pars->ble_state     = state;
  self->pars->ble_test      = test;
  self->pars->ble_crc       = crc;
  self->pars->ble_whitening = whitening;

  SX128X_DBG("set BLE PacketParam's: "
             "ConnactionState=%i TestPacket=%i CRC=%i Whitening=%i",
             (int) state, (int) test, (int) crc, (int) whitening);

  self->pktpars[0] = SX128X_CMD_SET_PACKET_PARAMS;
  self->pktpars[1] = sx128x_packet_ble_state[state];
  self->pktpars[2] = crc ? SX128X_BLE_CRC_3B : SX128X_BLE_CRC_OFF;
  self->pktpars[3] = sx128x_packet_ble_test[test];
  self->pktpars[4] = whitening ? SX128X_WHITENING_ON : SX128X_WHITENING_OFF;
  self->pktpars[5] = 0;
  self->pktpars[6] = 0;
  self->pktpars[7] = 0;

  return sx128x_spi_pktpars(self);
}
//-----------------------------------------------------------------------------
// set BLE access address
int8_t sx128x_address_ble(sx128x_t *self, uint32_t address)
{ // Table 14-24: Access Address Definition in BLE Packet (page 116)
  uint8_t buf[4];
  buf[0] = (address >> 24) & 0xFF; // byte 3
  buf[1] = (address >> 16) & 0xFF; // byte 2
  buf[2] = (address >>  8) & 0xFF; // byte 1
  buf[3] = (address      ) & 0xFF; // byte 0
  self->pars->ble_address = address;
  SX128X_DBG("set BLE AccessAddress=0x%08X", (unsigned) address);
  return sx128x_reg_write(self, SX128X_REG_SYNC_ADDRESS1_BYTE3, buf, 4);
}
//-----------------------------------------------------------------------------
// set BLE CRC initialization (24 bit)
int8_t sx128x_crc_init_ble(sx128x_t *self, uint32_t crc_init)
{ // Table 14-25: CRC Initialization Registers
  uint8_t buf[3];
  buf[0] = (crc_init >> 16) & 0xFF; // byte 2
  buf[1] = (crc_init >>  8) & 0xFF; // byte 1
  buf[2] = (crc_init      ) & 0xFF; // byte 0
  self->pars->ble_crc_init = crc_init;
  SX128X_DBG("set BLE CRC_init=0x%06X", (unsigned) crc_init);
  return sx128x_reg_write(self, SX128X_REG_CRC_SEED_BYTE2, buf, 3);
}
//-----------------------------------------------------------------------------
// set auto TX in BLE mode
// Note: real delay will be 33+time [us]
//       if time=0 then auto TX off
int8_t sx128x_auto_tx_ble(sx128x_t *self, uint16_t time)
{
  SX128X_DBG("set BLE auto TX time=%i", (int) time);
  self->pars->ble_auto_tx = time;
  self->txbuf[0] = SX128X_CMD_SET_AUTO_TX;
  self->txbuf[1] = (time >> 8) & 0xFF;
  self->txbuf[2] = (time     ) & 0xFF;
  return sx128x_spi(self, 3);
}
//-----------------------------------------------------------------------------
#endif // SX128X_USE_BLE
//-----------------------------------------------------------------------------
// prepare data to send (help funcion)
int8_t sx128x_to_send(
  sx128x_t *self,
  const uint8_t *payload, // payload to send
  uint8_t  payload_size,  // payload size [bytes]
  uint8_t  fixed)         // 1-fixed packet size, 0-variable packet size
{
  int8_t retv;
  payload_size = sx128x_limit_payload_size(self, payload_size);

  if (self->sleep)
  { // go to standby mode
    retv = sx128x_wakeup(self, SX128X_STANDBY_XOSC);
    if (retv != SX128X_ERR_NONE) return retv;
  }

  // clear IRQ status by mask (clear TxDone/RxDone/RxTxTimeout)
  retv = sx128x_clear_irq(self,
                          SX128X_IRQ_TX_DONE             |
                          SX128X_IRQ_RX_DONE             |
                          SX128X_IRQ_SLAVE_RESPONSE_DONE |
                          SX128X_IRQ_MASTER_RESULT_VALID |
                          SX128X_IRQ_MASTER_TIMEOUT      |
                          SX128X_IRQ_RX_TX_TIMEOUT);
  if (retv != SX128X_ERR_NONE) return retv;

  // restore TxDataPointer, RxDataPoiner
  retv = sx128x_set_buffer(self, self->tx_addr, self->rx_addr);
  if (retv != SX128X_ERR_NONE) return retv;

  // write output data to buffer
  retv = sx128x_buf_write(self,
                          self->tx_addr, payload, payload_size); // offset, data, nbytes
  if (retv != SX128X_ERR_NONE) return retv;

#if  defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
  if (self->pars->mode == SX128X_PACKET_TYPE_LORA ||
      self->pars->mode == SX128X_PACKET_TYPE_RANGING)
  {
    if (self->pars->crc == 2)
    { // add software short CRC8
      uint8_t crc;
      crc = crc8((const uint8_t*) payload, payload_size);
      retv = sx128x_buf_write(self,
                              payload_size, &crc, 1); // offset, data, nbytes
      if (retv != SX128X_ERR_NONE) return retv;
    }

    SX128X_DBG("send LoRa%s packet with %s header (payload_size=%u)",
               self->pars->mode == SX128X_PACKET_TYPE_RANGING ? "-Ranging" : "",
               fixed ? "Implicit" : "Explicit",
               (unsigned) payload_size);

    self->pktpars[2] = fixed ? SX128X_LORA_IMPLICIT_HEADER : SX128X_LORA_EXPLICIT_HEADER;
    self->pktpars[3] = self->pars->crc == 2 ? payload_size + 1 : payload_size;
  }
#endif // SX128X_USE_LORA || SX128X_USE_RANGING

#if defined(SX128X_USE_FLRC) || defined(SX128X_USE_GFSK)
  if (self->pars->mode == SX128X_PACKET_TYPE_FLRC ||
      self->pars->mode == SX128X_PACKET_TYPE_GFSK)
  {
    SX128X_DBG("send %s packet with %s size (payload_size=%u)",
               self->pars->mode == SX128X_PACKET_TYPE_FLRC ? "FLRC" : "GFSK",
               fixed ? "fixed" : "variable", (unsigned) payload_size);

    self->pktpars[4] = fixed ? SX128X_PACKET_FIXED_LENGTH :
                       SX128X_PACKET_VARIABLE_LENGTH;
    self->pktpars[5] = payload_size;
  }
#endif // SX128X_USE_FLRC || defined(SX128X_USE_GFSK)

#ifdef SX128X_USE_BLE
  if (self->pars->mode == SX128X_PACKET_TYPE_BLE)
  {
    SX128X_DBG("send BLE packet");
    //...
    // FIXME
  }
#endif // SX128X_USE_BLE

  // update packet params (HeaderType and PayloadLength)
  return sx128x_spi_pktpars(self);
}
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
  uint8_t  timeout_base)  // TX timeout base (15.625us, 62.5us, 1ms, 4ms)
{
  int8_t retv = sx128x_to_send(self, payload, payload_size, fixed);
  if (retv != SX128X_ERR_NONE) return retv;

  // set TX mode
  return sx128x_tx(self, timeout, timeout_base);
}
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
  uint8_t  payload_size, // payload size
  uint8_t  fixed,        // 1-fixed packet size, 0-variable packet size
  uint16_t timeout,      // RX timeout (0x0 - single mode, 0xFFFF - continuous mode)
  uint8_t  timeout_base) // RX timeout base (15.625us, 62.5us, 1ms, 4ms)
{
  int8_t retv;
  uint8_t max_packet_length = sx128x_limit_payload_size(self, SX128X_MAX_PACKET_LENGTH);
  payload_size = sx128x_limit_payload_size(self, payload_size);

  if (self->sleep)
  { // go to standby mode
    retv = sx128x_wakeup(self, SX128X_STANDBY_XOSC);
    if (retv != SX128X_ERR_NONE) return retv;
  }

  // clear IRQ status by mask (clear RxDone/RxTxTimeout etc)
  retv = sx128x_clear_irq(self,
                          SX128X_IRQ_RX_DONE |
                          SX128X_IRQ_SYNC_WORD_VALID |
                          SX128X_IRQ_SYNC_WORD_ERROR |
                          SX128X_IRQ_HEADER_VALID |
                          SX128X_IRQ_HEADER_ERROR |
                          SX128X_IRQ_CRC_ERROR |
                          SX128X_IRQ_SLAVE_REQUEST_DISCARD |
                          SX128X_IRQ_SLAVE_REQUEST_VALID |
                          SX128X_IRQ_CAD_DONE |
                          SX128X_IRQ_CAD_DETECTED |
                          SX128X_IRQ_RX_TX_TIMEOUT |
                          SX128X_IRQ_PREAMBLE_DETECTED);
  if (retv != SX128X_ERR_NONE) return retv;

#if defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
  if (self->pars->mode == SX128X_PACKET_TYPE_LORA ||
      self->pars->mode == SX128X_PACKET_TYPE_RANGING)
  {
    if (fixed && payload_size)
    { // implicit header
      SX128X_DBG("wait receive LoRa%s packet with Implicit header (payload_size=%u)",
                 self->pars->mode == SX128X_PACKET_TYPE_RANGING ? "-Ranging" : "",
                 (unsigned) payload_size);

      self->pktpars[2] = SX128X_LORA_IMPLICIT_HEADER;
      self->pktpars[3] = self->pars->crc == 2 ? payload_size + 1 : payload_size;
    }
    else
    { // explicit header
      SX128X_DBG("wait receive LoRa%s packet with Explicit header (payload_size=%u)",
                 self->pars->mode == SX128X_PACKET_TYPE_RANGING ? "-Ranging" : "",
                 (unsigned) max_packet_length);

      self->pktpars[2] = SX128X_LORA_EXPLICIT_HEADER;
      self->pktpars[3] = max_packet_length;
    }
  }
#endif // SX128X_USE_LORA || SX128X_USE_RANGING

#if defined(SX128X_USE_FLRC) || defined(SX128X_USE_GFSK)
  if (self->pars->mode == SX128X_PACKET_TYPE_FLRC ||
      self->pars->mode == SX128X_PACKET_TYPE_GFSK)
  {
    if (fixed && payload_size)
    { // fixed packet size
      SX128X_DBG("wait receive %s packet with Fixed size (payload_size=%u)",
                 self->pars->mode == SX128X_PACKET_TYPE_FLRC ? "FLRC" : "GFSK",
                 (unsigned) payload_size);
      self->pktpars[4] = SX128X_PACKET_FIXED_LENGTH;
      self->pktpars[5] = payload_size;
    }
    else
    { // variable packet size
      SX128X_DBG("wait receive %s packet with Variable size (payload_size=%u)",
                 self->pars->mode == SX128X_PACKET_TYPE_FLRC ? "FLRC" : "GFSK",
                 (unsigned) max_packet_length);
      self->pktpars[4] = SX128X_PACKET_VARIABLE_LENGTH;
      self->pktpars[5] = max_packet_length;
    }
  }
#endif // SX128X_USE_FLRC || SX128X_USE_GFSK

#ifdef SX128X_USE_BLE
  if (self->pars->mode == SX128X_PACKET_TYPE_BLE)
  {
    SX128X_DBG("wait receive BLE packet");
  }
#endif // SX128X_USE_BLE

  // update packet params (HeaderType and PayloadLength)
  retv = sx128x_spi_pktpars(self);
  if (retv != SX128X_ERR_NONE) return retv;

  // set RX mode
  return sx128x_rx(self, timeout, timeout_base);
}
//-----------------------------------------------------------------------------
// get RX status from chip (help function)
int8_t sx128x_get_rx_status(
  sx128x_t *self,
  uint16_t irq,       // IRQ status from sx128x_get_irq()
  sx128x_rx_t *rx) // RX status
{
  int8_t retv;

  rx->status = rx->mode = rx->stat = rx->rssi = 0;
  rx->crc_ok = !(irq & SX128X_IRQ_CRC_ERROR);

#if  defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
  if (self->pars->mode == SX128X_PACKET_TYPE_LORA ||
      self->pars->mode == SX128X_PACKET_TYPE_RANGING)
  { // LoRa or Ranging
    rx->lora = 1;
    rx->rssi_inst = rx->snr = 0;
    rx->fei = 0;

    retv = sx128x_rssi_lora(self, &rx->rssi_inst); // RSSI inst first!
    if (retv != SX128X_ERR_NONE) return retv;

    retv = sx128x_packet_status_lora(self, &rx->status, &rx->rssi, &rx->snr);
    if (retv != SX128X_ERR_NONE) return retv;

    sx128x_status_unpack(rx->status, &rx->mode, &rx->stat);

    retv = sx128x_get_hdr_lora(self, &rx->hdr);
    if (retv != SX128X_ERR_NONE) return retv;

    retv = sx128x_fei_lora(self, &rx->fei);
    if (retv != SX128X_ERR_NONE) return retv;

    SX128X_DBG("RX LoRa/Ranging status=0x%02X HeaderType=%s "
               "crc_ok=%i RSSI=-%idBm/2 "
               "RSSI_inst=-%idBm/2 SNR=%idB/4 FEI=%iHz",
               (unsigned) rx->status,
               rx->hdr.fixed ? "Implicit" : "Explicit",
               (int) rx->crc_ok, (int) rx->rssi,
               (int) rx->rssi_inst, (int) rx->snr, (int) rx->fei);
  }
#endif // SX128X_USE_LORA || SX128X_USE_RANGING

#if defined(SX128X_USE_FLRC) || defined(SX128X_USE_GFSK) || defined(SX128X_USE_BLE)
  if (self->pars->mode == SX128X_PACKET_TYPE_FLRC ||
      self->pars->mode == SX128X_PACKET_TYPE_GFSK ||
      self->pars->mode == SX128X_PACKET_TYPE_BLE)
  { // GFSK/FLRC/BLE
    uint8_t pkt_sync;   // sync packet status   (Table 11-69)

    rx->lora = 0;
    rx->sync_addrs = 0;

    retv = sx128x_packet_status(self, &rx->status, &rx->rssi,
                                &rx->pkt_status, &rx->pkt_errors, &pkt_sync);
    if (retv != SX128X_ERR_NONE) return retv;

    rx->sync_addrs = pkt_sync & 0x07; // Table 11-69, page 94

    if (rx->pkt_errors & (1 << 4)) rx->crc_ok = 0; // Table 11-68, page 94

    SX128X_DBG("RX GFSK/FLRC/BLE status=0x%02X crc_ok=%i RSSI=-%idBm/2 "
               "sync_addrs=%i",
               (unsigned) rx->status, (int) rx->crc_ok, (int) rx->rssi,
               (int) rx->sync_addrs);
  }
#endif // SX128X_USE_FLRC || SX128X_USE_GFSK || SX128X_USE_BLE

  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------
// get RX data from chip (help function)
// (call sx128x_get_rx_buffer() & sx128x_buf_read())
int8_t sx128x_get_rx_data(
  sx128x_t *self,
  uint8_t *payload,         // buffer for RX payload data
  uint8_t *payload_size,    // real RX payload data size
  uint8_t max_payload_size, // maximal RX data buffer size
  uint8_t fixed)            // 1 - if LoRa Implicit Header, else 0
{
  int8_t retv;
  uint8_t status, rx_start_addr;

  *payload_size = 0;

  // get RX buffer status
  retv = sx128x_get_rx_buffer(self, &status,
                              payload_size, &rx_start_addr, fixed);
  if (retv != SX128X_ERR_NONE) return retv;

  // limit payload_size
  if (*payload_size > max_payload_size) *payload_size = max_payload_size;

  SX128X_DBG("get RX data: RxStartAddr=0x%02X RxPayloadSize=%u (of %u)",
             (unsigned) rx_start_addr, (unsigned) *payload_size,
             (unsigned) max_payload_size);

  // read received data from buffer
  return sx128x_buf_read(self, rx_start_addr, payload, *payload_size);
}
//-----------------------------------------------------------------------------
// get RX data and RX status from chip (help mega function)
// (call sx128x_get_rx_status() & call sx128x_get_rx_buffer() & sx128x_buf_read())
int8_t sx128x_get_recv(
  sx128x_t *self,           // pointer to `sx128x_t` object
  // input:
  uint16_t irq,             // IRQ status from sx128x_get_irq()
  uint8_t max_payload_size, // RX data buffer size
  // output:
  sx128x_rx_t *rx,          // RX status
  uint8_t *payload,         // buffer for RX payload data
  uint8_t *payload_size)    // real RX payload data size
{
  int8_t retv;
  uint8_t status, rx_start_addr, payload_recv;
  uint8_t crc = 0;

  *payload_size = 0;

  // get RX status from chip
  retv = sx128x_get_rx_status(self, irq, rx);
  if (retv != SX128X_ERR_NONE) return retv;

  // get RX buffer status
  retv = sx128x_get_rx_buffer(self, &status,
                              &payload_recv, &rx_start_addr,
#if defined(SX128X_USE_LORA) || defined(SX128X_RANGING)
                              rx->hdr.fixed
#else
                              0
#endif
                             );
  if (retv != SX128X_ERR_NONE) return retv;

#if  defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
  if (rx->lora && self->pars->crc == 2)
  { // LoRa CRC8 software mode
    payload_recv--;
    crc = 1;
  }
#endif // SX128X_USE_LORA || SX128X_USE_RANGING

  // limit payload_size
  if (payload_recv > max_payload_size) payload_recv = max_payload_size;

  // read received data from buffer
  retv = sx128x_buf_read(self, rx_start_addr, payload, payload_recv);
  if (retv != SX128X_ERR_NONE) return retv;

#if  defined(SX128X_USE_LORA) || defined(SX128X_USE_RANGING)
  if (crc)
  { // get and check software CRC8
    retv = sx128x_buf_read(self, rx_start_addr + payload_recv, &crc, 1);
    if (retv != SX128X_ERR_NONE) return retv;
    rx->crc_ok = (crc == crc8((const uint8_t*) payload, payload_recv));
  }
#endif // SX128X_USE_LORA || SX128X_USE_RANGING

  // restore TxDataPointer, RxDataPoiner
  retv = sx128x_set_buffer(self, self->tx_addr, self->rx_addr);
  if (retv != SX128X_ERR_NONE) return retv;

  *payload_size = payload_recv;

#ifdef SX128X_DEBUG
  if (rx->lora)
  { // LoRa/Ranging
    SX128X_DBG("recv LoRa CRC_ok=%i RSSI=-%idBm/2 "
               "RSSI_inst=-%idB/2 SNR=%idB/4 FEI=%iHz "
               "size=%i",
               (int) rx->crc_ok, (int) rx->rssi,
               (int) rx->rssi_inst, (int) rx->snr, (int) rx->fei,
	       (int) payload_recv);
  }
  else
  { // GFSK/FLRC/BLE
    SX128X_DBG("recv GFSK/FLRC CRC_ok=%i RSSI=-%idBm/2 "
               "sync_addrs=%i "
               "size=%i",
               (int) rx->crc_ok, (int) rx->rssi,
               (int) rx->sync_addrs,
               (int) payload_recv);

  }
#endif // SX128X_DEBUG
  return SX128X_ERR_NONE;
}
//-----------------------------------------------------------------------------

/*** end of "sx128x.c" file ***/

