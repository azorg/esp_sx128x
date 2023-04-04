# cert/key path
TLS="./tls"

# MQTT server
#MQTT_HOST="mqtt.noflood.ru" # from Internet
MQTT_HOST="192.168.0.250" # from local network
MQTT_PORT="8883"
MQTT_QOS="0"

MQTT_KEEPALIVE="60" # by default 60 sec

MQTT_INSECURE="yes" # don't use client TLS cert
MQTT_USER="mqtt"
MQTT_PASSWD="12345678" # secret password of user
MQTT_CLIENT_ID=""

#MQTT_RETAIN="yes"

