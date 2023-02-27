Install Arduino IDE + ESP32
===========================

# 1. Install Arduino IDE from
Go to https://www.arduino.cc/

# 2. Add ESP32 support to Arduino IDE
Loock https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html

# 3. Install some additional packages (Debian/Ubuntu) 
```bash
sudo apt install git git-doc gitk qgit git-gui meld #<- git "must have"
sudo apt install minicom screen picocom

sudo apt install python3-pip
sudo pip install esptool
```
# 4. Add linux (Debian/Ubuntu) user to `dialout` group
```bash
sudo adduser $USER dialout
```
## Connect and check ESP32 Chip ID
```bash
esptool.py -p /dev/ttyUSB0 chip_id`
```

# 5. Open sketch `esp32_sx128x.ino`, compile and upload to flash ESP32

# 6. Run minicom or picocom
```bash
minicom -c on -D /dev/ttuUSB0 -b 115200

OR

picocom -b 115200 /dev/ttyUSB0

```

