#!/bin/sh

sudo systemctl stop   mosquitto
sudo systemctl start  mosquitto
sudo systemctl status mosquitto

