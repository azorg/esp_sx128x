ESP32 + SX128x test application
===============================

## You need hardware
1. ESP32 module (like "DOIT ESP32 KIT v1" or "Wemos S2 Mini")
2. SX128x module (like E28-2G4M27S or E28-2G4M12S)

## You need software
* Arduino IDE
* MQTT broker with TLS/SSL support (mosquitto recomented)

## How to install
Look `INSTALL.md` file

## Files:
 - `esp_sx128x` - all Arduino sources directory
 - `esp_sx128x.ino` - Arduino sketch file
 - `doc` - documents and help information, pinouts (ESP32, SX128x, E28) 
 - `lib` - library notes
 - `scrips` - help scripts

## Connect LoRa E28 (SX128x) module to ESP32 (30-pin) development board
| ESP32  | ESP32 board  | E28        | Comment (in/ou} on MCU)  | pin name    | Color   |
|:------ |:------------ |:---------- |:------------------------ |:----------- |:------- |
| 3V3    | XSL:1  (3V3) | `+3.3V`    | Vcc (power after LDO)    | -           | Red     |
| GND    | XSL:2  (GND) | `GND`      | GND                      | -           | Black   |
| GPIO4  | XSL:5  (D4)  | `NRESET`   | neg. reset (out)         | `NRST`      | Magenta |
| GPIO16 | XSL:6  (D16) | `DIO1`     | interrupt (in)           | `DIO1`      | Yellow  |
|  -     | -            | `DIO2`     | -                        | -           | -       |
|  -     | -            | `DIO3`     | -                        | -           | -       |
| GPIO17 | XSL:7  (D17) | `BUSY`     | BUSY (in)                | `BUSY`      | Orange  |
| GPIO5  | XSL:8  (D5)  | `NSS_CTS`  | SPI NSS (out)            | `NSS`       | Gray    |
| GPIO18 | XSL:9  (D18) | `SCK_RTSN` | SPI SCK (out)            | `SCK`       | White   |
| GPIO19 | XSL:10 (D19) | `MISO_TX`  | SPI MISO (in)            | `MISO`      | Blue    |
| GPIO23 | XSL:15 (D23) | `MOSI_RX`  | SPI MOSI (out)           | `MOSI`      | Green   |
| GND    | XSR:2  (GND) | `GND`      | GND                      | -           | -       |
| GPIO33 | XSR:9  (D33) | `RX_EN`    | LNA on (out)             | `RXEN`      | Brown   |
| GPIO32 | XSR:10 (D32) | `TX_EN`    | PowerAmp on (out)        | `TXEN`      | Gray2   |
| GPIO0  |              | -          | button BOOT (in)         | `BUTTON`    | -       |
| GPIO2  | XSL:4  (D4)  | -          | on board LED (out)       | `LED`       | -       |

Note:
 1. XSL - on left side; XSR - on right side
 2. ESP32 connected to PC by USB type-C or MicroUSB cable
 3. `TX_EN` and `RX_EN` only for 27dBm version E28 nodule (E28-2G4M27S)

## Connect LoRa E28 (SX128x) module to "Wemos S2 Mini" board
Look `doc/s2mini+e28-2g4m12s.pdf`

## Run `minicom`
```bash
minicom -c on -b 115200 -D /dev/ttyUSB0
```

## Available CLI commands
look `COMMANDS.md`

