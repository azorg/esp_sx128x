#!/bin/sh

WDIR=`dirname $0`
OLDPWD=`pwd`
cd "$WDIR"

. ./mqtt_conf.sh

TOPIC="topic"

[ "$MQTT_CLIENT_ID" ] && MQTT_CLIENT_ID="-i ${MQTT_CLIENT_ID}_sub"

if [ "$MQTT_INSECURE" = "yes" ]
then
  mosquitto_sub --cafile "$TLS/ca/ca.crt" \
                --insecure \
                -d -v \
                -k $MQTT_KEEPALIVE \
                -h $MQTT_HOST -p $MQTT_PORT -q $MQTT_QOS \
                -u $MQTT_USER -P "$MQTT_PASSWD" \
                $MQTT_CLIENT_ID \
                -t "$TOPIC" 
else
  mosquitto_sub --cafile "$TLS/ca/ca.crt" \
                --cert   "$TLS/client/client.crt" \
                --key    "$TLS/client/client.key" \
                -d -v \
                -k $MQTT_KEEPALIVE \
                -h $MQTT_HOST -p $MQTT_PORT -q $MQTT_QOS \
                -u $MQTT_USER -P "$MQTT_PASSWD" \
                $MQTT_CLIENT_ID \
                -t "$TOPIC" 
fi

cd "$OLDPWD"

