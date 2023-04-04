All CLI comands
===============
```
help - print this help (Ctrl+X)
version - print version
clear - clear screen (Ctrl+L)
verbose [n] - set/get verbose leval 0...3
led {0|1} - 1=on/0=off LED
led blink [N] - blink LED N times
pin gpio [0|1] - read/write digital pin
sys - system information
sys info - print system information
sys time - print system time and ticks
sys reset - full system reset MCU
eeprom - EEPROM commands
eeprom erase - erase options region of EEPROM
eeprom write - write all options to EEPROM (Ctrl+W)
eeprom read - read all options from EEPROM
eeprom delete - delete last options record from EEPROM
eeprom diff - find differece between current options and saved in EEPROM
eeprom dump [offset size] - hex dump region in EEPROM
def - set to default all options (opt_t)
hw - hardware direct/status commands
hw reset [t1 t2] - hardware reset SX128x by NRST
hw rxen [0|1] - 1-set/0-reset RXEN line (on/off LNA)
hw txen [0|1] - 1-set/0-reset TXEN line (on/off PowerAmp)
hw busy - get BUSY status
spi [b0 b1..] - exchange bytes (b0 b1...) by SPI
radio - SX128x radio module commands
radio status - get status
radio status last - get last status
radio ver - get firmware version (16 bits)
radio sleep [config] - set Sleep mode (Ctrl+Z)
radio standby [config] - set Standby mode (Ctrl+D)
radio wakeup [config] - set Standby mode (no BUSY wait)
radio mode [mode] - set/get mode (packet type)
radio save  - save context
radio fs - set FS mode
radio tx [to base] - set TX mode
radio rx [to base] - set RX mode
radio cad - set LoRa CAD mode
radio freq [Hz] - set/get RF frequency SX [Hz]
radio pwr [dBm us] - set/get TX power level [dBm] and rampTime [us]
radio lna [1|0] - on/off/get LNA boost
radio gain [rx_gain] - set RX gain (0-AGC, 1-min, 13-max
radio dcdc [1|0] - DC-DC 0=off/1=on
radio auto_fs {0|1} - set auto FS: 1-enable, 0-disable
radio irq [mask] - get/clear by MASK IRQ status
radio rxdc [rx sleep base] - set RX duty cycle
radio wave - set TX Continuous Wave (RF tone) mode (Ctrl+Y)
radio preamble - set TX Continuous Preamble mode
radio reg [addr val] - read/write register
radio restore - restore all parameters (Ctrl+T)
radio lp {0|1} - set Long Preamble: 1-enable, 0-disable
lora - set/get LoRa params/results
lora mod [BW SF CR ] - set/get LoRa modulation params
lora packet [PR CRC INV] - set/get LoRa packet pars (Preamble, CRC, invertIQ)
lora sw [SW] - set/get LoRa SyncWord (0x12 or 0x34)
lora cad [sym_num] - set LoRa CAD mode sym_num [1...16]
lora fei - get LoRa frequency error indicator [Hz]
lora rssi - get LoRa instantaneous RSSI
ranging [role MA SA SM] - set/get Ranging [Role, MasterAddr, SlaveAddr, SlaveMode]
ranging advanced {1|0} - get/set Advanced Ranging [0-off, 1-on]
ranging calib [calibration] - set/get Ranging calibration
ranging result [filter] - get Ranging result (filter: 0-Raw, 1-Filtered, 2-as-is)
flrc - set/get FLRC params/results
flrc mod [BR CR BT] - set/get FLRC modulation params
flrc packet [PR SW SWM CRC] - set/get FLRC packet pars
flrc swt [0..15] - get/set SyncWord Tolerance in FLRC
flrc sw [SW1 SW2 SW3] - get/set SyncWord 1..3 in FLRC
gfsk - set/get GFSK params/results
gfsk mod [BR DSB MI BT] - set/get GFSK modulation params
gfsk packet [PR SWL SWM CRC W] - set/get GFSK packet pars
gfsk swt [0..15] - get/set SyncWord Tolerance in GFSK
gfsk sw [SW1 SW2 SW3] - get/set SyncWord 1..3 in GFSK
ble [ST TST CRC W] - set/get BLE params
ble auto_tx [delay] - set BLE auto TX delay [us], 0=off
buffer - TX/RX buffer commands
buffer base TxAd RxAd - set TX/RX buffer base addreses
buffer read Ad [Num] - read from RX/TX buffer
buffer write Ad [b0 b1..] - write data to RX/TX buffer
tx_timeout [ms] - get/set TX timeout [ms]
fixed [0|1] - get/set fixed/variable size of send packet
data [b0 b1..] - get/set TX data bytes
data fill [size value] - fill TX data bytes
data size [size] - get/set RX/TX data size
code [101001] - get/set OOK code
status - get packet status
send [to] - send packet [timeout] (Strl+S)
recv [size to] - receive packet [timeout] (Strl+V)
mode [0..9] - get/set FSM mode (0-CW, 1-OOK, 2-TX, 3-RX, 4-RQ, 5-RP, 6-RM, 7-RS, 8-AR, 9-SG)
fsm [T dT dC WUT] - get/set FSM parameters (T-period[ms], dT-CW[ms], dC-code[ms], WUT-wakeup time[ms])
sweep [Fmin Fmax S] - get/set sweep generator pars (Fmin/Fmax - kHz, S - kHz/sec)
start - start FSM loop (Ctrl+S)
stop - stop FSM loop (Ctrl+C)
autostart [1|0 delay] - get/set autostart on reboot flag and delay [sec]
wifi - Wi-Fi options
wifi ssid [SSID] - get/set Wi-Fi SSID
wifi passwd [passwd] - get/set Wi-Fi password
wifi disable - disable Wi-Fi (reset SSID)
wifi connect - Wi-Fi connect
wifi disconnect - Wi-Fi disconnect
wifi status - print Wi-Fi status
mqtt - MQTT options
mqtt server [HOST PORT ID] - get/set MQTT server options
mqtt client [USER KEY] - get/set MQTT client options
mqtt connect - connect to MQTT broker
mqtt disconnect - disconnect from MQTT broker
mqtt status - print MQTT connection status
mqtt state - print MQTT state (integer)
mqtt pub [Topic MSG RTN] - publish message
mqtt sub [Topic QoS] - subscribe topic
mqtt unsub [Topic] - unsubscribe topic
```

