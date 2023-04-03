#!/bin/sh

WDIR=`dirname $0`
OLDPWD=`pwd`
cd "$WDIR"

. ./mqtt_conf.sh

TOPIC="sample/TLS"
MSG="Hello from mosquitto_pub!"

if [ "$MQTT_INSECURE" = "yes" ]
then
  mosquitto_pub --cafile "$TLS/ca/ca.crt" \
                --insecure \
                -d \
                -k $MQTT_KEEPALIVE \
                -h $MQTT_HOST -p $MQTT_PORT -q $MQTT_QOS \
                -i "${MQTT_CLIENT_ID}_pub" \
                -u $MQTT_USER -P "$MQTT_PASSWD" \
                -t "$TOPIC" -m "$MSG"
else
  mosquitto_pub --cafile "$TLS/ca/ca.crt" \
                --cert   "$TLS/client/client.crt" \
                --key    "$TLS/client/client.key" \
                -d \
                -k $MQTT_KEEPALIVE \
                -h $MQTT_HOST -p $MQTT_PORT -q $MQTT_QOS \
                -i "${MQTT_CLIENT_ID}_pub" \
                -u $MQTT_USER -P "$MQTT_PASSWD" \
                -t "$TOPIC" -m "$MSG"

fi

cd "$OLDPWD"

