#!/bin/sh

WDIR=`dirname $0`
OLDPWD=`pwd`
cd "$WDIR"

. ./mqtt_conf.sh

TOPIC="temp"
MSG="99.9"

[ "$MQTT_CLIENT_ID" ] && MQTT_CLIENT_ID="-i ${MQTT_CLIENT_ID}_pub"

if [ "$MQTT_INSECURE" = "yes" ]
then
  mosquitto_pub --cafile "$TLS/ca/ca.crt" \
                --insecure \
                -d \
                -k $MQTT_KEEPALIVE \
                -h $MQTT_HOST -p $MQTT_PORT -q $MQTT_QOS \
                -u $MQTT_USER -P "$MQTT_PASSWD" \
                $MQTT_CLIENT_ID \
                -t "$TOPIC" -m "$MSG"
else
  mosquitto_pub --cafile "$TLS/ca/ca.crt" \
                --cert   "$TLS/client/client.crt" \
                --key    "$TLS/client/client.key" \
                -d \
                -k $MQTT_KEEPALIVE \
                -h $MQTT_HOST -p $MQTT_PORT -q $MQTT_QOS \
                -u $MQTT_USER -P "$MQTT_PASSWD" \
                $MQTT_CLIENT_ID \
                -t "$TOPIC" -m "$MSG"

fi

cd "$OLDPWD"

