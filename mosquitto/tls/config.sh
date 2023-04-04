# root CA
CA_NAME="MQTT_ROOT_CA" # name in *.h file
CA_DAYS="3653" # 10 years
CA_CN="noflood.ru Root CA" # Common Name (eg: the main domain the certificate should cover)
CA_O="Noflood" # Organization
CA_OU="noflood.ru" # Organization Unit
CA_C="RU" # Country
CA_EMAIL="mail@noflood.ru" # main administrative point of contact for the certificate
CA_SAN_DNS="noflood.ru www.noflood.ru ca.noflood.ru" # subjectAltName [DNS.n]

# server CA
SERVER_DAYS="3653" # 10 years
SERVER_CN="mqtt.noflood.ru" # look `man mosquitto-tls`
SERVER_O="Noflood MQTT Server"
SERVER_OU="MQTT noflood.ru broker"
SERVER_C="RU"
SERVER_EMAIL="mqtt@noflood.ru"
SERVER_SAN_DNS="noflood.ru mqtt.noflood.ru io.noflood.ru"

# client CA
CLIENT_DAYS="3653" # 10 years
CLIENT_CN="mqtt.noflood.ru"
CLIENT_O="Noflood MQTT Client"
CLIENT_OU="MQTT noflood.ru client"
CLIENT_C="RU"
#CLIENT_SAN_DNS="client.net"


