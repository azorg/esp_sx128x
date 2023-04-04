/*
 * MQTT wrappers
 * File: "mqtt.cpp"
 */

//-----------------------------------------------------------------------------
//#include <PubSubClient.h>
#include <WiFiClientSecure.h>
//-----------------------------------------------------------------------------
#include "config.h"
#include "wifi.h"
#include "mqtt.h"
#include "ca_root.h" // MQTT_ROOT_CA
//-----------------------------------------------------------------------------
// Possible values for PubSubClient state [-4...6] (look "PubSubClient.h")
const char * const mqtt_state_string[10] = {
  "CONNECTION_TIMEOUT",
  "CONNECTION_LOST",
  "CONNECT_FAILED",
  "DISCONNECTED",
  "CONNECTED",
  "CONNECT_BAD_PROTOCOL",
  "CONNECT_BAD_CLIENT_ID",
  "CONNECT_UNAVAILABLE",
  "CONNECT_BAD_CREDENTIALS",
  "CONNECT_UNAUTHORIZED"
};
//-----------------------------------------------------------------------------
static const char *mqtt_root_ca = MQTT_ROOT_CA;
//-----------------------------------------------------------------------------
// WiFiFlientSecure for SSL/TLS support
static WiFiClientSecure mqtt_wifi;
//-----------------------------------------------------------------------------
// MQTT client class
PubSubClient Mqtt;
//-----------------------------------------------------------------------------
// return state as string
const char *mqtt_state(int state)
{
  if      (state < -4) state = -4;
  else if (state >  6) state =  6;
  return mqtt_state_string[state + 4];
}
//-----------------------------------------------------------------------------
bool mqtt_connect(const char *host, uint16_t port,
                  const char *id, const char *user, const char *key)
{
  if (Mqtt.connected()) Mqtt.disconnect();
  
  // set root CA
  mqtt_wifi.setCACert(mqtt_root_ca);
  
  Serial.print("Trying MQTT connection to mqtts://");
  Serial.print(user);
  Serial.print(':');
  Serial.print(key);  
  Serial.print('@');
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  Mqtt.setServer(host, port);
  //Mqtt.setCallback(mqtt_callback);
  Mqtt.setClient(mqtt_wifi);
  
  //!!!Mqtt.setKeepAlive(60);
  //!!!Mqtt.setSocketTimeout(180)

  boolean ret = mqtt_reconnect(id, user, key);
  if (ret) Serial.println("MQTT connection SUCCESS");
  return ret;
}
//-----------------------------------------------------------------------------
bool mqtt_reconnect(const char *id, const char *user, const char *key)
{
  uint8_t retries = MQTT_RETRIES;
  while (!Mqtt.connect(id, user, key)) {
    Serial.print("MQTT connection FAIL; state=");
    Serial.print(Mqtt.state());
    Serial.print(" [");
    Serial.print(mqtt_state(Mqtt.state()));
    Serial.print(']');
    //Mqtt.disconnect();
    if (retries-- == 0) {
      Serial.println("; break");
      return false;
    }
    Serial.println("; try again in 3 seconds");
    delay(3000); // wait 3 seconds FIXME: bad idea
  }
  return true;
}
//-----------------------------------------------------------------------------
bool mqtt_connected()
{
  return Mqtt.connected();
}
//-----------------------------------------------------------------------------
void mqtt_disconnect()
{
  Mqtt.disconnect();
}
//-----------------------------------------------------------------------------

/*** end of "mqtt.cpp" file ***/


