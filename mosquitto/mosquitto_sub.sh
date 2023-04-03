#!/bin/sh

WDIR=`dirname $0`
OLDPWD=`pwd`
cd "$WDIR"

. ./mqtt_conf.sh

TOPIC="sample/TLS"

if [ "$MQTT_INSECURE" = "yes" ]
then
  mosquitto_sub --cafile "$TLS/ca/ca.crt" \
                --insecure \
                -d -v \
                -k $MQTT_KEEPALIVE \
                -h $MQTT_HOST -p $MQTT_PORT -q $MQTT_QOS \
                -i "${MQTT_CLIENT_ID}_sub" \
                -u $MQTT_USER -P "$MQTT_PASSWD" \
                -t "$TOPIC" 
else
  mosquitto_sub --cafile "$TLS/ca/ca.crt" \
                --cert   "$TLS/client/client.crt" \
                --key    "$TLS/client/client.key" \
                -d -v \
                -k $MQTT_KEEPALIVE \
                -h $MQTT_HOST -p $MQTT_PORT -q $MQTT_QOS \
                -i "${MQTT_CLIENT_ID}_sub" \
                -u $MQTT_USER -P "$MQTT_PASSWD" \
                -t "$TOPIC" 
fi

cd "$OLDPWD"

