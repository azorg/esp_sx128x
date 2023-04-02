/*
 * MQTT wrappers
 * File: "mqtt.pp"
 */

//-----------------------------------------------------------------------------
#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
//-----------------------------------------------------------------------------
#include "mqtt.h"
#include "ca_root.h" // MQTT_ROOT_CA
//-----------------------------------------------------------------------------
const char *mqtt_root_ca = MQTT_ROOT_CA;

//-----------------------------------------------------------------------------

/*** end of "mqtt.h" file ***/


