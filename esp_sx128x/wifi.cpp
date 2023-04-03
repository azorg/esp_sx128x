/*
 * Wi-Fi wrappers
 * File: "wifi.cpp"
 */

//-----------------------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include "WiFiClientSecure.h"
//-----------------------------------------------------------------------------
#include "config.h"
#include "wifi.h"
//-----------------------------------------------------------------------------
static const char *wifi_ssid   = "";
static const char *wifi_passwd = "";
static bool wifi_auto_reconnetc = true;
//-----------------------------------------------------------------------------
bool wifi_connect(const char *ssid, const char *passwd, bool auto_reconnect)
{
  wifi_ssid   = ssid;
  wifi_passwd = passwd;
  bool connected = wifi_reconnect();
  if (connected) WiFi.setAutoReconnect(auto_reconnect);
  return connected;
}
//-----------------------------------------------------------------------------
bool wifi_reconnect()
{
  unsigned cnt = 0;
  unsigned timeout = WIFI_TIMEOUT * 2;

  Serial.print("Trying Wi-Fi connection to ESSID:\"");
  Serial.print(wifi_ssid);
  Serial.print("\" in ");
  Serial.print(WIFI_TIMEOUT);
  Serial.println(" second(s)");

  WiFi.begin(wifi_ssid, wifi_passwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
    cnt++;
    if (timeout && cnt > timeout) break;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wi-Fi connected in ");
    Serial.print((cnt + 1) / 2);
    Serial.println(" second(s)");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("Wi-Fi connection timeout");
    return false;
  }
}
//-----------------------------------------------------------------------------
bool wifi_connected()
{
  return WiFi.status() == WL_CONNECTED;
}
//-----------------------------------------------------------------------------
void wifi_disconnect()
{
  WiFi.disconnect(true);
}
//-----------------------------------------------------------------------------

/*** end of "wifi.cpp" file ***/


