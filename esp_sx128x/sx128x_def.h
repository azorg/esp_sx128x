/*
 * Yet another Semtech SX128x famaly chips driver
 * File: "sx128x_def.h"
 */

#pragma once
#ifndef SX128X_DEF_H
#define SX128X_DEF_H
//-----------------------------------------------------------------------------
// Table 11-5: Status Byte Definition (page 73)
// bits 7...5  of Status (CircuitMode):
#define SX128X_STATUS_MODE_STRING { \
  "0 (Unused)",     \
  "1 (RFU)",        \
  "2 (STDBY_RC)",   \
  "3 (STDBY_XOSC)", \
  "4 (FS)",         \
  "5 (RX)",         \
  "6 (TX)",         \
  "7 (Unknown)" };
//-----------------------------------------------------------------------------
// bits 4...2  of Status (CommandStatus):
#define SX128X_STATUS_CMD_STRING { \
  "0 (Reserved)",   \
  "1 (OK)",         \
  "2 (RX done)",    \
  "3 (Timeout)",    \
  "4 (CmdError)",   \
  "5 (CmdFail)",    \
  "6 (TX done)",    \
  "7 (Unknown)" };
//-----------------------------------------------------------------------------
#define SX128X_GAIN_STRING { "Auto", "-54dB", "-48dB", "-42dB", "-36dB", \
  "-30dB", "-24dB", "-18dB", "-12dB", "-8dB", "-6dB", "-4dB", "-2dB", "0dB" }
//-----------------------------------------------------------------------------
// packet type strings [0-4]:
#define SX128X_PACKET_TYPE_STRING { "GFSK", "LoRa", "Ranging", "FLRC", "BLE", "Reserved" }
#define SX128X_PACKET_TYPE_HELP "0:GFSK 1:LoRa 2:Ranging 3:FLRC 4:BLE"
//-----------------------------------------------------------------------------
// time base strings [0-3]:
#define SX128X_TIME_BASE_STRING { "15.625us", "62.5us", "1ms", "4ms", "Unknown" }
//-----------------------------------------------------------------------------
// SX128X SPI commands
// operational codes (opcodes)
#define SX128X_CMD_GET_STATUS                  0xC0 // GetStatus()
#define SX128X_CMD_SET_SLEEP                   0x84 // SetSleep()
#define SX128X_CMD_SET_STANDBY                 0x80 // SetStandby()
#define SX128X_CMD_SET_PACKET_TYPE             0x8A // SetPacketType()
#define SX128X_CMD_GET_PACKET_TYPE             0x03 // GetPacketType()
#define SX128X_CMD_SET_FS                      0xC1 // SetFs()
#define SX128X_CMD_SET_TX                      0x83 // SetTx()
#define SX128X_CMD_SET_RX                      0x82 // SetRx()
#define SX128X_CMD_SET_RX_DUTY_CYCLE           0x94 // SetRxDutyCycle()
#define SX128X_CMD_SET_LONG_PREAMBLE           0x9B // SetLongPreamble()
#define SX128X_CMD_SET_CAD                     0xC5 // SetCAD()
#define SX128X_CMD_SET_TX_CONTINUOUS_WAVE      0xD1 // SetTxContinuousWave()
#define SX128X_CMD_SET_TX_CONTINUOUS_PREAMBLE  0xD2 // SetTxContinuousPreamble()
#define SX128X_CMD_SET_AUTO_TX                 0x98 // SetAutoTx()
#define SX128X_CMD_SET_AUTO_FS                 0x9E // SetAutoFs()
#define SX128X_CMD_SET_RF_FREQUENCY            0x86 // SetRfFrequency()
#define SX128X_CMD_SET_TX_PARAMS               0x8E // SetTxParams()
#define SX128X_CMD_SET_CAD_PARAMS              0x88 // SetCadParams()
#define SX128X_CMD_SET_BUFFER_BASE_ADDRESS     0x8F // SetBufferBaseAddress()
#define SX128X_CMD_SET_MODULATION_PARAMS       0x8B // SetModulationParams()
#define SX128X_CMD_SET_PACKET_PARAMS           0x8C // SetPacketParams()
#define SX128X_CMD_SET_RANGING_ROLE            0xA3 // SetRangingRole()
#define SX128X_CMD_SET_ADVANCED_RANGING        0x9A // SetAdvancedRanging()
#define SX128X_CMD_GET_RX_BUFFER_STATUS        0x17 // GetRxBufferStatus()
#define SX128X_CMD_GET_PACKET_STATUS           0x1D // GetPacketStatus()
#define SX128X_CMD_GET_RSSI_INST               0x1F // GetRssiInst()
#define SX128X_CMD_SET_DIO_IRQ_PARAMS          0x8D // SetDioIrqParams()
#define SX128X_CMD_GET_IRQ_STATUS              0x15 // GetIrqStatus()
#define SX128X_CMD_CLR_IRQ_STATUS              0x97 // ClrIrqStatus()
#define SX128X_CMD_SET_REGULATOR_MODE          0x96 // SetRegulatorMode()
#define SX128X_CMD_SET_SAVE_CONTEXT            0xD5 // SetSaveContext()

// register and buffer access commands
#define SX128X_CMD_WRITE_REGISTER              0x18 // WriteRegister()
#define SX128X_CMD_READ_REGISTER               0x19 // ReadRegister()
#define SX128X_CMD_WRITE_BUFFER                0x1A // WriteBuffer()
#define SX128X_CMD_READ_BUFFER                 0x1B // ReadBuffer()

// SX128X SPI command variables
// SX128X_CMD_SET_SLEEP:
#define SX128X_SLEEP_OFF_RETENTION 0x0 // Data RAM and buffer is flushed during Sleep Mode
#define SX128X_SLEEP_RAM_RETENTION 0x1 // Data RAM in retention mode
#define SX128X_SLEEP_BUF_RETENTION 0x2 // Data buffer in retention mode
#define SX128X_SLEEP_ALL_RETENTION 0x3 // Data RAM and buffer in retention mode

// SX128X_CMD_SET_STANDBY:
#define SX128X_STANDBY_RC         0x00 // standby mode: 13 MHz RC oscillator
#define SX128X_STANDBY_XOSC       0x01 //               52 MHz crystal oscillator

// SX128X_CMD_SET_PACKET_TYPE, SX128X_CMD_GET_PACKET_TYPE:
#define SX128X_PACKET_TYPE_GFSK     0x00 // GFSK mode
#define SX128X_PACKET_TYPE_LORA     0x01 // LoRa(R) mode
#define SX128X_PACKET_TYPE_RANGING  0x02 // Ranging Engine mode (LoRa)
#define SX128X_PACKET_TYPE_FLRC     0x03 // FLRC mode
#define SX128X_PACKET_TYPE_BLE      0x04 // BLE mode
#define SX128X_PACKET_TYPE_RESERVED 0x05 // Reserved

// SX128X_CMD_SET_TX, SX128X_CMD_SET_RX, SX128X_CMD_SET_RX_DUTY_CYCLE:
#define SX128X_TIME_BASE_15_625US    0x00 // time base = 15.625 us
#define SX128X_TIME_BASE_62_5US      0x01 // time base = 62.5 us
#define SX128X_TIME_BASE_1MS         0x02 // time base = 1 ms
#define SX128X_TIME_BASE_4MS         0x03 // time base = 1 ms
#define SX128X_TX_TIMEOUT_SINGLE     0x0000 // no time-out, TX single mode
#define SX128X_RX_TIMEOUT_SINGLE     0x0000 // no time-out, RX single mode
#define SX128X_RX_TIMEOUT_CONTINUOUS 0xFFFF // Rx continuous mode

// SX128X_CMD_SET_TX_PARAMS:
// Table 11-49: RampTime Definition (page 88)
#define SX128X_RADIO_RAMP_02_US 0x00 // RampTime=2 us
#define SX128X_RADIO_RAMP_04_US 0x20 // RampTime=4 us
#define SX128X_RADIO_RAMP_06_US 0x40 // RampTime=6 us
#define SX128X_RADIO_RAMP_08_US 0x60 // RampTime=8 us
#define SX128X_RADIO_RAMP_10_US 0x80 // RampTime=10 us
#define SX128X_RADIO_RAMP_12_US 0xA0 // RampTime=12 us
#define SX128X_RADIO_RAMP_16_US 0xC0 // RampTime=16 us
#define SX128X_RADIO_RAMP_20_US 0xE0 // RampTime=20 us

// SX128X_CMD_SET_CAD_PARAMS:
// Table 11-52: CadSymbolNum Definition (page 89)
#define SX128X_LORA_CAD_01_SYMBOL  0x00 // sym_num=1
#define SX128X_LORA_CAD_02_SYMBOLS 0x20 // sym_num=2
#define SX128X_LORA_CAD_04_SYMBOLS 0x40 // sym_num=4
#define SX128X_LORA_CAD_08_SYMBOLS 0x60 // sym_num=8
#define SX128X_LORA_CAD_16_SYMBOLS 0x80 // sym_num=16

// SX128X_CMD_SET_MODULATION_PARAMS:
// Table 14-47: Modulation Parameters in LoRa Mode (page 130)
#define SX128X_MOD_PARAM1_LORA_SF_5  0x50 // SF=5
#define SX128X_MOD_PARAM1_LORA_SF_6  0x60 // SF=6
#define SX128X_MOD_PARAM1_LORA_SF_7  0x70 // SF=7
#define SX128X_MOD_PARAM1_LORA_SF_8  0x80 // SF=8
#define SX128X_MOD_PARAM1_LORA_SF_9  0x90 // SF=9
#define SX128X_MOD_PARAM1_LORA_SF_10 0xA0 // SF=10
#define SX128X_MOD_PARAM1_LORA_SF_11 0xB0 // SF=11
#define SX128X_MOD_PARAM1_LORA_SF_12 0xC0 // SF=12

// Table 14-48: Modulation Parameters in LoRa Mode (page 131)
#define SX128X_MOD_PARAM2_LORA_BW_1600 0x0A // BW=1625.0kHz
#define SX128X_MOD_PARAM2_LORA_BW_800  0x18 // BW=812.5kHz
#define SX128X_MOD_PARAM2_LORA_BW_400  0x26 // BW=406.25kHz
#define SX128X_MOD_PARAM2_LORA_BW_200  0x34 // BW=203.125kHz

// Table 14-49: Modulation Parameters in LoRa Mode (page 131)
#define SX128X_MOD_PARAM3_LORA_CR_4_5    0x01 // CR=4/5
#define SX128X_MOD_PARAM3_LORA_CR_4_6    0x02 // CR=4/6
#define SX128X_MOD_PARAM3_LORA_CR_4_7    0x03 // CR=4/7
#define SX128X_MOD_PARAM3_LORA_CR_4_8    0x04 // CR=4/8
#define SX128X_MOD_PARAM3_LORA_CR_LI_4_5 0x05 // CR=4/5*
#define SX128X_MOD_PARAM3_LORA_CR_LI_4_6 0x06 // CR=4/6*
#define SX128X_MOD_PARAM3_LORA_CR_LI_4_8 0x07 // CR=4/8*
// * A new interleaving scheme has been implemented to increase robustness
//   to burst interference and/or strong Doppler events.

// Table 14-31: Modulation Parameters in FLRC Mode: Bandwidth and Bit Rate (page 121)
#define SX128X_FLRC_BR_1_300_BW_1_2  0x45 // dr=1300kb/s BW=1200kHz
#define SX128X_FLRC_BR_1_040_BW_1_2  0x69 // dr=1040kb/s BW=1200kHz
#define SX128X_FLRC_BR_0_650_BW_0_6  0x86 // dr=650kb/s  BW=600kHz
#define SX128X_FLRC_BR_0_520_BW_0_6  0xAA // dr=520kb/s  BW=600kHz
#define SX128X_FLRC_BR_0_325_BW_0_3  0xC7 // dr=325kb/s  BW=300kHz
#define SX128X_FLRC_BR_0_260_BW_0_3  0xEB // dr=260kb/s  BW=300kHz

// Table 14-32: Modulation Parameters in FLRC Mode: Coding Rate (page 122)
#define SX128X_FLRC_CR_1_2 0x00 // fcr=3 (CR=1/2)
#define SX128X_FLRC_CR_3_4 0x02 // fcr=2 (CR=3/4)
#define SX128X_FLRC_CR_1_1 0x04 // fcr=1 (CR=1)

// Table 14-33: Modulation Parameters in FLRC Mode: BT (page 122)
// Table 14-3: Modulation Shaping Parameters in GFSK Mode (page 106)
#define SX128X_BT_OFF 0x00 // bt=0 (no filtering)
#define SX128X_BT_1_0 0x10 // bt=2 (BT=1)
#define SX128X_BT_0_5 0x20 // bt=1 (BT=0.5)

// Table 14-1: Modulation Parameters in GFSK Mode (page 104)
#define SX128X_GFSK_BLE_BR_2_000_BW_2_4 0x04 // bitrate=2000kb/s BW=2400kHz
#define SX128X_GFSK_BLE_BR_1_600_BW_2_4 0x28 // bitrate=1600kb/s BW=2400kHz
#define SX128X_GFSK_BLE_BR_1_000_BW_2_4 0x4C // bitrate=1000kb/s BW=2400kHz
#define SX128X_GFSK_BLE_BR_1_000_BW_1_2 0x45 // bitrate=1000kb/s BW=1200kHz
#define SX128X_GFSK_BLE_BR_0_800_BW_2_4 0x70 // bitrate=800kb/s  BW=2400kHz
#define SX128X_GFSK_BLE_BR_0_800_BW_1_2 0x69 // bitrate=800kb/s  BW=1200kHz
#define SX128X_GFSK_BLE_BR_0_500_BW_1_2 0x8D // bitrate=500kb/s  BW=1200kHz
#define SX128X_GFSK_BLE_BR_0_500_BW_0_6 0x86 // bitrate=500kb/s  BW=600kHz
#define SX128X_GFSK_BLE_BR_0_400_BW_1_2 0xB1 // bitrate=400kb/s  BW=1200kHz
#define SX128X_GFSK_BLE_BR_0_400_BW_0_6 0xAA // bitrate=400kb/s  BW=600kHz
#define SX128X_GFSK_BLE_BR_0_250_BW_0_6 0xCE // bitrate=250kb/s  BW=600kHz
#define SX128X_GFSK_BLE_BR_0_250_BW_0_3 0xC7 // bitrate=250kb/s  BW=300kHz
#define SX128X_GFSK_BLE_BR_0_125_BW_0_3 0xEF // bitrate=125kb/s  BW=300kHz

// Table 14-2: Modulation Index Parameters in GFSK Mode (page 105)
#define SX128X_MOD_IND_0_35 0x00 // 0.35
#define SX128X_MOD_IND_0_50 0x01 // 0.50
#define SX128X_MOD_IND_0_75 0x02 // 0.75
#define SX128X_MOD_IND_1_00 0x03 // 1.00
#define SX128X_MOD_IND_1_25 0x04 // 1.25
#define SX128X_MOD_IND_1_50 0x05 // 1.50
#define SX128X_MOD_IND_1_75 0x06 // 1.75
#define SX128X_MOD_IND_2_00 0x07 // 2.00
#define SX128X_MOD_IND_2_25 0x08 // 2.25
#define SX128X_MOD_IND_2_50 0x09 // 2.50
#define SX128X_MOD_IND_2_75 0x0A // 2.75
#define SX128X_MOD_IND_3_00 0x0B // 3.00
#define SX128X_MOD_IND_3_25 0x0C // 3.25
#define SX128X_MOD_IND_3_50 0x0D // 3.50
#define SX128X_MOD_IND_3_75 0x0E // 3.75
#define SX128X_MOD_IND_4_00 0x0F // 4.00

// SX128X_CMD_SET_PACKET_PARAMS:
#define SX128X_LORA_EXPLICIT_HEADER 0x00 // LoRa Explicit Header
#define SX128X_LORA_IMPLICIT_HEADER 0x80 // LoRa Implicit Header
#define SX128X_LORA_CRC_ENABLE      0x20 // LoRa hardware CRC enable
#define SX128X_LORA_CRC_DISABLE     0x00 // LoRa hardware CRC disable
#define SX128X_LORA_IQ_INVERTED     0x00 // LoRa IQ swapped
#define SX128X_LORA_IQ_STD          0x40 // LoRa IQ standard

// Table 14-34: AGC Preamble Length Definition in FLRC Packet (page 122)
// Table 14-4: Preamble Length Definition in GFSK Packet (page 106)
#define SX128X_PREAMBLE_LENGTH_4_BITS  0x00 // preamble=4 bits (reserved for FLRC)
#define SX128X_PREAMBLE_LENGTH_8_BITS  0x10 // preamble=8  bits
#define SX128X_PREAMBLE_LENGTH_12_BITS 0x20 // preamble=12 bits
#define SX128X_PREAMBLE_LENGTH_16_BITS 0x30 // preamble=16 bits
#define SX128X_PREAMBLE_LENGTH_20_BITS 0x40 // preamble=20 bits
#define SX128X_PREAMBLE_LENGTH_24_BITS 0x50 // preamble=24 bits
#define SX128X_PREAMBLE_LENGTH_28_BITS 0x60 // preamble=28 bits
#define SX128X_PREAMBLE_LENGTH_32_BITS 0x70 // preamble=32 bits

// Table 14-35: Sync Word Length Definition in FLRC Packet (page 122)
#define SX128X_FLRC_SYNC_NOSYNC        0x00 // 21 bits preamble
#define SX128X_FLRC_SYNC_WORD_LEN_P32S 0x04 // 21 bits preamble + 32 bits Sync Word

// Table 14-36: Sync Word Combination in FLRC Packet (page 123)
#define SX128X_RX_DISABLE_SYNC_WORD     0x00 // Disable Sync Word
#define SX128X_RX_MATCH_SYNC_WORD_1     0x10 // SyncWord1
#define SX128X_RX_MATCH_SYNC_WORD_2     0x20 // SyncWord2
#define SX128X_RX_MATCH_SYNC_WORD_1_2   0x30 // SyncWord1 or SyncWord2
#define SX128X_RX_MATCH_SYNC_WORD_3     0x40 // SyncWord3
#define SX128X_RX_MATCH_SYNC_WORD_1_3   0x50 // SyncWord1 or SyncWord3
#define SX128X_RX_MATCH_SYNC_WORD_2_3   0x60 // SyncWord2 or SyncWord3
#define SX128X_RX_MATCH_SYNC_WORD_1_2_3 0x70 // SyncWord1 or SyncWord2 or SyncWord3

// Table 14-39: CRC Definition in FLRC Packet (page 124)
#define SX128X_FLRC_CRC_OFF    0x00 // no CRC
#define SX128X_FLRC_CRC_2_BYTE 0x10 // CRC field uses 2 bytes
#define SX128X_FLRC_CRC_3_BYTE 0x20 // CRC field uses 3 bytes
#define SX128X_FLRC_CRC_4_BYTE 0x30 // CRC field uses 4 bytes

// Table 14-37: Packet Type Definition in FLRC Packet (page 124)
// Table 14-7: Packet Type Definition in GFSK Packet (page 107)
#define SX128X_PACKET_FIXED_LENGTH    0x00 // fixed length mode
#define SX128X_PACKET_VARIABLE_LENGTH 0x20 // variable length mode

// Table 14-41: Whitening Definition in FLRC Packet (page 125)
// Table 14-10: Whitening Enabling in GFSK Packet (page 108)
#define SX128X_WHITENING_ON  0x00 // Whitening enable
#define SX128X_WHITENING_OFF 0x08 // Whitening disabled

// Table 14-5: Sync Word Length Definition in GFSK Packet (page 107)
#define SX128X_GFSK_SYNC_WORD_LEN_1_B 0x00 // 1 byte SyncWord
#define SX128X_GFSK_SYNC_WORD_LEN_2_B 0x02 // 2 bytes SyncWord
#define SX128X_GFSK_SYNC_WORD_LEN_3_B 0x04 // 3 bytes SyncWord
#define SX128X_GFSK_SYNC_WORD_LEN_4_B 0x06 // 4 bytes SyncWord
#define SX128X_GFSK_SYNC_WORD_LEN_5_B 0x08 // 5 bytes SyncWoed

// Table 14-6: Sync Word Combination in GFSK Packet (page 107)
#define SX128X_GFSK_SELECT_SYNCWORD_OFF   0x00 // off
#define SX128X_GFSK_SELECT_SYNCWORD_1     0x10 // 1->1
#define SX128X_GFSK_SELECT_SYNCWORD_2     0x20 // 2->2
#define SX128X_GFSK_SELECT_SYNCWORD_1_2   0x30 // 1->12
#define SX128X_GFSK_SELECT_SYNCWORD_3     0x40 // 3->3
#define SX128X_GFSK_SELECT_SYNCWORD_1_3   0x50 // 1->13
#define SX128X_GFSK_SELECT_SYNCWORD_2_3   0x60 // 1->23
#define SX128X_GFSK_SELECT_SYNCWORD_1_2_3 0x70 // 1->123

// Table 14-9: CRC Definition in GFSK Packet (page 108)
#define SX128X_GFSK_CRC_OFF     0x00 // CRC off
#define SX128X_GFSK_CRC_1_BYTE  0x10 // CRC 1 byte
#define SX128X_GFSK_CRC_2_BYTES 0x20 // CRC 2 bytes
#define SX128X_GFSK_CRC_3_BYTES 0x30 // CRC 3 bytes (undocumented)

// Table 14-20: Connection State Definition in BLE Packet (page 114)
#define SX128X_BLE_PAYLOAD_LENGTH_MAX_31_BYTES  0x00 // payload_size <=  31 (4.1)
#define SX128X_BLE_PAYLOAD_LENGTH_MAX_37_BYTES  0x20 // payload_size <=  37 (4.1)
#define SX128X_BLE_TX_TEST_MODE                 0x40 // payload_size <=  63 (4.1) - test mode
#define SX128X_BLE_RX_TEST_MODE                 0x60 // payload_size <=  63 (4.1) - undocumented
#define SX128X_BLE_PAYLOAD_LENGTH_MAX_255_BYTES 0x80 // payload_size <= 255 (4.2)

// Table 14-21: CRC Definition in BLE Packet (page 114)
#define SX128X_BLE_CRC_OFF 0x00 // CRC off - no Bluetooth compatibility
#define SX128X_BLE_CRC_3B  0x10 // CRC on (3 bytes) - Bluetooth compatibility

// Table 14-22: Tx Test Packet Payload in Test Mode for BLE Packet (page 115)
#define SX128X_BLE_PRBS_9       0x00 // pseudo random based on 9th polynominal
#define SX128X_BLE_EYELONG_1_0  0x04 // '11110000’
#define SX128X_BLE_EYESHORT_1_0 0x08 // ‘10101010'
#define SX128X_BLE_PRBS_15      0x0C // pseudo random based on 15th polynominal
#define SX128X_BLE_ALL_1        0x10 // '11111111'
#define SX128X_BLE_ALL_0        0x14 // '00000000'
#define SX128X_BLE_EYELONG_0_1  0x18 // '00001111'
#define SX128X_BLE_EYESHORT_0_1 0x1C // '01010101'

// SX128X_CMD_GET_PACKET_STATUS
// Table 11-67: Status Packet Status Byte (page 94)
#define SX128X_PACKET_STATUS_PKT_SENT  (1 << 0) // pktSent
#define SX128X_PACKET_STATUS_RX_NO_ACK (1 << 5) // rxNoAck

// Table 11-68: Error Packet Status Byte (page 94)
#define SX128X_PACKET_STATUS_ERROR_PKT_CTRL_BUSY (1 << 0) // packetCtrlBusy
#define SX128X_PACKET_STATUS_ERROR_PKT_RX        (1 << 1) // packetReceived
#define SX128X_PACKET_STATUS_ERROR_HEADER_RX     (1 << 2) // headerReceived
#define SX128X_PACKET_STATUS_ERROR_TX_RX_ABORTED (1 << 3) // AbortError
#define SX128X_PACKET_STATUS_ERROR_PKT_CRC       (1 << 4) // CrcError
#define SX128X_PACKET_STATUS_ERROR_PKT_LEN       (1 << 5) // LengthError
#define SX128X_PACKET_STATUS_ERROR_PKT_SYNC      (1 << 6) // SyncError

// Table 11-69: Sync Packet Status Byte (page 94)
#define SX128X_PACKET_STATUS_SYNC_ADDRESS_1 (1 << 0) // sync_adrs_1 detected
#define SX128X_PACKET_STATUS_SYNC_ADDRESS_2 (1 << 1) // sync_adrs_2 detected
#define SX128X_PACKET_STATUS_SYNC_ADDRESS_3 (1 << 2) // sync_adrs_3 detected

// SX128X_CMD_SET_DIO_IRQ_PARAMS, SX128X_CMD_GET_IRQ_STATUS:
// Table 11-73: IRQ Register (page 95)
#define SX128X_IRQ_NONE                  0x0000 // all interrupts off
#define SX128X_IRQ_ALL                   0xFFFF // all interrupts on
#define SX128X_IRQ_TX_DONE               0x0001 // TX complete (All)
#define SX128X_IRQ_RX_DONE               0x0002 // RX complete (All)
#define SX128X_IRQ_SYNC_WORD_VALID       0x0004 // sync word valid (GFSK/BLE/FLRC)
#define SX128X_IRQ_SYNC_WORD_ERROR       0x0008 // sync word error (FLRC)
#define SX128X_IRQ_HEADER_VALID          0x0010 // header valid (LoRa/Ranging)
#define SX128X_IRQ_HEADER_ERROR          0x0020 // header error (LoRa/Ranging)
#define SX128X_IRQ_CRC_ERROR             0x0040 // CRC error (All)
#define SX128X_IRQ_SLAVE_RESPONSE_DONE   0x0080 // ranging slave response complete (Ranging)
#define SX128X_IRQ_SLAVE_REQUEST_DISCARD 0x0100 // ranging slave request discarded (Ranging)
#define SX128X_IRQ_MASTER_RESULT_VALID   0x0200 // ranging master result valid (Ranging)
#define SX128X_IRQ_MASTER_TIMEOUT        0x0400 // ranging master timeout (Ranging)
#define SX128X_IRQ_SLAVE_REQUEST_VALID   0x0800 // ranging slave request valid (Ranging)
#define SX128X_IRQ_CAD_DONE              0x1000 // channel activity check complete (LoRa/Ranging)
#define SX128X_IRQ_CAD_DETECTED          0x2000 // channel activity detected (LoRa/Ranging)
#define SX128X_IRQ_RX_TX_TIMEOUT         0x4000 // RX or TX timeout (All)
#define SX128X_IRQ_PREAMBLE_DETECTED     0x8000 // preamble detected (LoRa/GFSK and BLE if LongPreamble=1)
#define SX128X_IRQ_ADVANCED_RANGING_DONE 0x8000 // advanced ranging completed (Ranging)

// SX128X_CMD_SET_REGULATOR_MODE:
#define SX128X_REGULATOR_LDO    0x00       // set regulator mode: LDO (default)
#define SX128X_REGULATOR_DC_DC  0x01       //                     DC-DC
//-----------------------------------------------------------------------------
// Register Map (undocumented)
#define SX128X_REG_FW_VERSION             0x1F0 // Firmware version (16 bits)
#define SX128X_REG_LORA_INCOMING_CR       0x950 // LoRa incoming packet coding rate register [6:4]
#define SX128X_REG_LORA_INCOMING_CRC      0x954 // LoRa incoming packet coding rate register [bit 4]
#define SX128X_REG_LORA_IQ_CONF           0x93B // LoRa incoming packet coding rate register

// The address where the blob SX128X_REG_RSSI_SNR_BUGFIX_BLOB must be written
// after wakeup to fix the "rssi is 0 after a sleep" bug
#define SX128X_REG_RSSI_SNR_BUGFIX_ADDRESS 0x0EF

// The blob that must be written to SX128X_REG_RSSI_SNR_BUGFIX_ADDRESS after
// wakeup to fix the "rssi is 0 after a sleep" bug
#define SX128X_REG_RSSI_SNR_BUGFIX_BLOB { 0x00, 0xAD, 0x08, 0x9A }

// Register Map (Table 13-1: List of Registers)
#define SX128X_REG_RX_GAIN                0x891 // Register determining the LNA gain regime
#define SX128X_REG_MANUAL_GAIN            0x895 // see Section 4.2
#define SX128X_REG_LNA_GAIN_VALUE         0x89E // see Section 4.2
#define SX128X_REG_LNA_GAIN_CONTROL       0x89F // Enable/Disable manual LNA gain control
#define SX128X_REG_SYNCH_PEAK_ATT         0x8C2 // dB Attenuation of the peak power during synch address
#define SX128X_REG_LORA_PAYLOAD_LENGTH    0x901 // The length of the received LoRa payload
#define SX128X_REG_LORA_HEADER_MODE       0x903 // LoRa modem header (bit 7): 1-on, 0-off

#define SX128X_REG_RANGING_REQ_ADDR3      0x912 // Ranging Master: address of the Slave device
#define SX128X_REG_RANGING_REQ_ADDR2      0x913 //                 to which request is sent
#define SX128X_REG_RANGING_REQ_ADDR1      0x914 //
#define SX128X_REG_RANGING_REQ_ADDR0      0x915 //

#define SX128X_REG_RANGING_DEV_ADDR3      0x916 // Ranging Address when used in Slave
#define SX128X_REG_RANGING_DEV_ADDR2      0x917 // and Advanced Ranging mode
#define SX128X_REG_RANGING_DEV_ADDR1      0x918 //
#define SX128X_REG_RANGING_DEV_ADDR0      0x919 //

#define SX128X_REG_RANGING_WIN_SIZE       0x91E // The number of ranging samples over which
                                                // the RSSI evaluated and the results averaged

#define SX128X_REG_RANGING_FILTER         0x923 // Clears the samples stored in the ranging filter
#define SX128X_REG_RANGING_RESULT_MUX     0x924 // Ranging result configuration
#define SX128X_REG_SF_ADDITIONAL_CONF     0x925 // SF range selection in LoRa mode

#define SX128X_REG_RANGING_CALIB_BYTE2    0x92B // The ranging calibration value
#define SX128X_REG_RANGING_CALIB_BYTE1    0x92C //
#define SX128X_REG_RANGING_CALIB_BYTE0    0x92D //

#define SX128X_REG_RANGING_ID_CHECK_LEN   0x931 // The number of bytes of the Ranging Slave ID
                                                // that are checked

#define SX128X_REG_FREQ_ERR_CORRECTION    0x93C // Crystal frequency error correction mode

#define SX128X_REG_LORA_SYNC_WORD_MSB     0x944 // 0x14 LoRa synch word value
#define SX128X_REG_LORA_SYNC_WORD_LSB     0x945 // 0x24

#define SX128X_REG_RANGING_RSSI_THRESHOLD 0x953 // RangingFilterRssiThresholdOffset [db], default 0x24

#define SX128X_REG_LORA_FEI_BYTE2         0x954 // LoRa Frequency error indicator (FEI)
#define SX128X_REG_LORA_FEI_BYTE1         0x955 // Note: LoRa FEi is reliable only for positive SNR
#define SX128X_REG_LORA_FEI_BYTE0         0x956 //

#define SX128X_REG_RANGING_RESULT_BYTE2   0x961 // The result of the last ranging exchange
#define SX128X_REG_RANGING_RESULT_BYTE1   0x962 //
#define SX128X_REG_RANGING_RESULT_BYTE0   0x963 //

#define SX128X_REG_RANGING_RSSI           0x964 // The RSSI value of the last ranging exchange
#define SX128X_REG_FREEZE_RANGING_RESULT  0x97F // Set to preserve the ranging result for reading

#define SX128X_REG_PACKET_PREAMBLE_SET    0x9C1 // Preamble length in GFSK and BLE
#define SX128X_REG_WHITENING_INITIAL_VAL  0x9C5 // Data whitening seed for GFSK and BLE modulation

#define SX128X_REG_CRC_POLYNOMIAL_MSB     0x9C6 // CRC Polynomial Definition for GFSK
#define SX128X_REG_CRC_POLYNOMIAL_LSB     0x9C7 //

#define SX128X_REG_CRC_SEED_BYTE2         0x9C7 // CRC Seed for BLE modulation
#define SX128X_REG_CRC_SEED_BYTE1         0x9C8 //
#define SX128X_REG_CRC_SEED_BYTE0         0x9C9 //

#define SX128X_REG_CRC_MSB_INITIAL_VALUE  0x9C8 // CRC Seed used for GFSK and FLRC
#define SX128X_REG_CRC_LSB_INITIAL_VALUE  0x9C9 // modulation

#define SX128X_REG_SYNCH_ADDRESS_CONTROL  0x9CD // The number of synch word bit errors
                                                // tolerated in FLRC and GFSK modes

#define SX128X_REG_SYNC_ADDRESS1_BYTE4    0x9CE // Synch Word 1
#define SX128X_REG_SYNC_ADDRESS1_BYTE3    0x9CF // (Also used as the BLE Access Address)
#define SX128X_REG_SYNC_ADDRESS1_BYTE2    0x9D0 //
#define SX128X_REG_SYNC_ADDRESS1_BYTE1    0x9D1 //
#define SX128X_REG_SYNC_ADDRESS1_BYTE0    0x9D2 //

#define SX128X_REG_SYNC_ADDRESS2_BYTE4    0x9D3 // Synch Word 2
#define SX128X_REG_SYNC_ADDRESS2_BYTE3    0x9D4 //
#define SX128X_REG_SYNC_ADDRESS2_BYTE2    0x9D5 //
#define SX128X_REG_SYNC_ADDRESS2_BYTE1    0x9D6 //
#define SX128X_REG_SYNC_ADDRESS2_BYTE0    0x9D7 //

#define SX128X_REG_SYNC_ADDRESS3_BYTE4    0x9D8 // Synch Word 3
#define SX128X_REG_SYNC_ADDRESS3_BYTE3    0x9D9 //
#define SX128X_REG_SYNC_ADDRESS3_BYTE2    0x9DA //
#define SX128X_REG_SYNC_ADDRESS3_BYTE1    0x9DB //
#define SX128X_REG_SYNC_ADDRESS3_BYTE0    0x9DC //
//-----------------------------------------------------------------------------
// Constants
#define SX128X_FREQ_OSC  52000000       // Hz
#define SX128X_FREQ_STEP 198.3642578125 // 52e6/2**18 Hz

#define SX128X_MAX_PACKET_LENGTH 255
//-----------------------------------------------------------------------------
#endif // SX128X_DEF_H

/*** end of "sx126x_def.h" file ***/


