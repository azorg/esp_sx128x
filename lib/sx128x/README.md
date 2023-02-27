Yet another Semtech SX128x famaly chips driver
==============================================
Look "sx128x.h" header file for details.

# Init and set configuration functions
* `sx128x_init()` - call at once
* `sx128x_free()` - free radio (go to sleep)
* `sx128x_set_pars()` - set all parameters from `sx128x_pars_t` structure

# Sleep/Standby mode functions
* `sx128x_sleep()` - go to Sleep mode
* `sx128x_get_sleep()` - get Sleep state
* `sx128x_wakeup()` - set Standby mode from Sleep mode (no wait BUSY down)
* `sx128x_standby()` - go to Standby mode (from Sleep too)

# Common control functions
* `sx128x_mode()` - set mode (packet type)
* `sx128x_get_mode()` - get mode (packet type)
* `sx128x_read_mode()` - read mode (packet type) from chip
* `sx128x_status()` - get status (read by SPI)
* `sx128x_last_status()` - return last status (don't read by SPI)
* `sx128x_cad()` - set CAD (Channel Activity Detection) mode
* `sx128x_tx()` - set TX mode
* `sx128x_rx()` - set RX mode
* `sx128x_set_buffer()` - set buffer(s) base address (TX/RX)
* `sx128x_get_rx_buffer()` - get RX buffer status

# IRQ funcions
* `sx128x_irq_dio_mask()` - set IRQ and DIO mask
* `sx128x_get_irq()` - get IRQ status
* `sx128x_clear_irq()` - clear IRQ status by mask

# Common configuration functions
* `sx128x_set_frequency()` - set RF frequency
* `sx128x_get_frequency()` get RF frequency
* `sx128x_set_power()` - set TX power level and ramp time
* `sx128x_get_power()` - get TX power level and ramp time
* `sx128x_set_lna_boost()` - set LNA boost on/off
* `sx128x_get_lna_boost()` - get LNA boost
* `sx128x_set_gain()` - set RX gain (AGC or manual gain)
* `sx128x_get_gain()` - get RX gain (AGC or manual gain)
* `sx128x_set_dcdc()` - set on/off DC-DC (regulator mode)
* `sx128x_get_dcdc()` - get on/off DC-DC (regulator mode)

# Miscellaneous functions
* `sx128x_fw_version()` - get firmware version (16 bits)
* `sx128x_status_unpack()` - unpack status
* `sx128x_reg_write()` - writes register(s)
* `sx128x_reg_read()` - read register(s)
* `sx128x_buf_write()` - write data to buffer
* `sx128x_buf_read()` - read data from buffer
* `sx128x_rx_duty_cycle()` - set RX duty cycle
* `sx128x_long_preamble()` - set long preamble
* `sx128x_save_context()` - set save context
* `sx128x_auto_fs()` - set auto FS
* `sx128x_set_swt()` set the number of bit-errors Sync Word tolerance (GFSK/FLRC)

# Service functions
* `sx128x_fs()` - set FS mode
* `sx128x_tx_wave()` - set TX continuous wave (RF tone) mode
* `sx128x_tx_preamble()` - set TX continuous preamble

# LoRa and Ranging operation funcions
* `sx128x_mod_lora()` - set modulation params in LoRa mode or Ranging mode
* `sx128x_packet_lora()` - set packet params in LoRa mode or Ranging mode
* `sx128x_set_sw_lora()` - set Sync Word in LoRa mode or Ranging mode
* `sx128x_get_sw_lora()` - get Sync Word in LoRa mode or Ranging mode
* `sx128x_set_cad_lora()` - set LoRa CAD param's
* `sx128x_get_cad_lora()` - get LoRa CAD param's

## RX functions for LoRa
* `sx128x_packet_status_lora()` - get packet status (LoRa and Ranging)
* `sx128x_get_hdr_lora()` - get LoRa Explicit Header (unpacked)
* `sx128x_fei_lora()` - get LoRa frequency error indicator (FEI)
* `sx128x_rssi_lora()` - get instantaneous RSSI during reception

# Ranging operation functions
* `sx128x_ranging_role()` - set Ranging role (master/slave)
* `sx128x_ranging_master_address()` - set master request address
* `sx128x_ranging_slave_address()` - set slave respond address and slave mode
* `sx128x_ranging_set_calibration()` - set ranging callibration register value (24 bit)
* `sx128x_ranging_get_calibration()` - get ranging callibration register value (24 bit)
* `sx128x_tx()` - set TX mode (for Master)
* `sx128x_rx()` - set RX mode (for Slave)
* `sx128x_ranging_result()` - get ranging result

# Advanced Ranging operation functions
* `sx128x_set_advanced_ranging()` - on/off Advanced Ranging (0-off, 1-on)
* `sx128x_get_advanced_ranging()` - get Advanced Ranging state (0-off, 1-on)
* `sx128x_rx()` - set RX mode
* `sx128x_advanced_ranging_address()` - get Advanced Ranging Address Received

# FLRC operation funcions
* `sx128x_mod_flrc()` - set modulation params in FLRC mode
* `sx128x_packet_flrc()` - set packet params in FLRC mode
* `sx128x_set_sw_flrc()` - set Sync Word[1...3] in FLRC mode
* `sx128x_get_sw_flrc()` - get Sync Word[1...3] in FLRC mode
* `sx128x_set_swt_flrc()` - set Sync Words Tolerance in FLRC mode
* `sx128x_get_swt_flrc()` - get Sync Words Tolerance in FLRC mode

# GFSK operation funcions
* `sx128x_mod_gfsk()` - set modulation params in GFSK mode
* `sx128x_packet_gfsk()` - set packet params in GFSK mode
* `sx128x_set_sw_gfsk()` - set Sync Word[1...3] in GFSK mode
* `sx128x_get_sw_gfsk()` - get Sync Word[1...3] in GFSK mode
* `sx128x_set_swt_gfsk()` set the number of bit-errors Sync Word tolerance (GFSK)

# BLE operation funcions
* `sx128x_mod_ble()` - set modulation params in BLE mode
* `sx128x_packet_ble()` - set packet params in BLE mode
* `sx128x_address_ble()` - set BLE access address
* `sx128x_auto_tx_ble()` - set auto TX in BLE mode

## RX functions for BLE/GFSK/FLRC
* `sx128x_packet_status()` - get packet status (FLRC/GFSK/BLE)

# Common TX functions
* `sx128x_send()` - send packet (help _mega_ function)

# Common RX functions
* `sx128x_recv()` - go to RX mode; wait callback by interrupt (help function)
* `sx128x_get_rx_status()` - get RX status from chip (help function)
* `sx128x_get_rx_data()` - get RX data from chip (help function)
* `sx128x_get_recv()` - get RX data and RX status from chip (help _mega_ function)

# LoRa CRC8 additional mode
If crc=2 then used software CRC8 mode (crc8.c/crc8.h) in LoRa mode,
Real payload size = user payload size + 1.
Look source of `sx128x_send()` and `sx128x_get_recv()` for detailes.





