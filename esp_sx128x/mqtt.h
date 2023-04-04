/*
 * MQTT wrappers
 * File: "mqtt.h"
 */

#pragma once
#ifndef MQTT_H
#define NQTT_H
//-----------------------------------------------------------------------------
#ifdef __cplusplus
#include <PubSubClient.h>
extern PubSubClient Mqtt; // global variable
//-----------------------------------------------------------------------------
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
const char *mqtt_state(int state); // return state as string
bool mqtt_connect(const char *host, uint16_t port,
                  const char *id, const char *user, const char *key);
bool mqtt_reconnect(const char *id, const char *user, const char *key);
bool mqtt_connected();
void mqtt_disconnect();
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // MQTT_H

/*** end of "mqtt.h" file ***/


