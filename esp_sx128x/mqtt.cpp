/*
 * MQTT wrappers
 * File: "mqtt.cpp"
 */

//-----------------------------------------------------------------------------
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <WiFiClientSecure.h>
//-----------------------------------------------------------------------------
#include "config.h"
#include "wifi.h"
#include "mqtt.h"
#include "ca_root.h" // MQTT_ROOT_CA
//-----------------------------------------------------------------------------
#define MQTT_EXTREME_EMBEDED_PROGRAMMING
//-----------------------------------------------------------------------------
static const char *mqtt_root_ca = MQTT_ROOT_CA;
//-----------------------------------------------------------------------------
// WiFiFlientSecure for SSL/TLS support
static WiFiClientSecure mqtt_wifi;
//-----------------------------------------------------------------------------
// MQTT client class by passing in the WiFi client
Adafruit_MQTT_Client *Mqtt = (Adafruit_MQTT_Client*) NULL;
//-----------------------------------------------------------------------------
bool mqtt_connect(const char *host, uint16_t port,
                  const char *user, const char *key)
{
  Serial.print("Trying MQTT connection to mqtts://");
  Serial.print(user);
  Serial.print(':');
  Serial.print(key);
  Serial.print('@');
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);
  
  // set root CA
  mqtt_wifi.setCACert(mqtt_root_ca);

  if (Mqtt != (Adafruit_MQTT_Client*) NULL) {
    Mqtt->disconnect();
#ifdef MQTT_EXTREME_EMBEDED_PROGRAMMING
    delete Mqtt;
    Mqtt = new Adafruit_MQTT_Client(&mqtt_wifi, host, port, user, key);
#endif
  } else Mqtt = new Adafruit_MQTT_Client(&mqtt_wifi, host, port, user, key);

  if (Mqtt == (Adafruit_MQTT_Client*) NULL)
  {
    Serial.println("MQTT error: `new` return NULL");
    return false;
  }
  
  uint8_t retries = MQTT_RETRIES;
  int8_t ret;
  while (ret = Mqtt->connect() != 0) { // connect() will return 0 for connected
    Serial.println(Mqtt->connectErrorString(ret));
    if (retries-- == 0) {
#ifdef MQTT_EXTREME_EMBEDED_PROGRAMMING
      delete Mqtt;
      Mqtt = (Adafruit_MQTT_Client*) NULL;
#endif
      Serial.println("MQTT connection FAIL");
      return false;
    }
    Serial.println("Retrying MQTT connection in 3 seconds...");
    Mqtt->disconnect();
    delay(3000); // wait 3 seconds
  }

  Serial.println("MQTT connection SUCCESS");
  return true;
}
//-----------------------------------------------------------------------------
bool mqtt_connected()
{
  if (Mqtt == (Adafruit_MQTT_Client*) NULL) return false;
  return Mqtt->connected();
}
//-----------------------------------------------------------------------------
void mqtt_disconnect()
{
  if (Mqtt == (Adafruit_MQTT_Client*) NULL) return;
  Mqtt->disconnect();
#ifdef MQTT_EXTREME_EMBEDED_PROGRAMMING
  delete Mqtt;
  Mqtt = (Adafruit_MQTT_Client*) NULL;
#endif
}
//-----------------------------------------------------------------------------

/*** end of "mqtt.cpp" file ***/


