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

  Serial.print("Trying Wi-Fi connection to ESSID: \"");
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
    Serial.print("signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
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
long wifi_rssi()
{
  return WiFi.RSSI();
}
//-----------------------------------------------------------------------------
void wifi_status_print()
{
  bool connected = WiFi.isConnected();
  Serial.print("Wi-Fi is ");
  Serial.println(connected ? "connected" : "disconnected");

  if (connected) {
    Serial.print("AutoReconnect is ");
    Serial.println(WiFi.getAutoReconnect() ? "true" : "false");

    // print the SSID of the network you're attached to:
    Serial.print("SSID: \"");
    Serial.print(WiFi.SSID());
    Serial.println("\"");

    // print your board's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
  }
}
//-----------------------------------------------------------------------------

/*** end of "wifi.cpp" file ***/


