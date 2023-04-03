# root CA
CA_NAME="MQTT_ROOT_CA"
CA_DAYS="3653" # 10 years
CA_CN="noflood.ru Root CA"
CA_O="Noflood"
CA_OU="noflood.ru"
CA_C="RU"
CA_EMAIL="mail@noflood.ru"

# server CA
SERVER_DAYS="3653" # 10 years
SERVER_CN="mqtt.noflood.ru" # look `man mosquitto-tls`
SERVER_O="Noflood MQTT Server"
SERVER_OU="MQTT noflood.ru broker"
SERVER_C="RU"
SERVER_EMAIL="mqtt@noflood.ru"

# client CA
CLIENT_DAYS="3653" # 10 years
CLIENT_CN="mqtt.noflood.ru"
CLIENT_O="Noflood MQTT Client"
CLIENT_OU="MQTT noflood.ru client"
CLIENT_C="RU"


