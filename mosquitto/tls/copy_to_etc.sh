#!/bin/bash

sudo chmod 0400 ca/ca.key
sudo chmod 0400 client/client.key
sudo chmod 0400 server/server.key

#sudo cp ca/ca.crt /etc/ssl/certs/ca-fake.pem
#sudo chown root:root /etc/ssl/certs/ca-fake.pem
#sudo chmod 0444      /etc/ssl/certs/ca-fake.pem          
#sudo update-ca-certificates --fresh

sudo cp ca/ca.crt /etc/mosquitto/ca_certificates/

sudo cp server/server.crt /etc/mosquitto/certs/
sudo cp server/server.key /etc/mosquitto/certs/

sudo chown mosquitto:mosquitto /etc/mosquitto/certs/server.key
sudo chmod 0440                /etc/mosquitto/certs/server.key

