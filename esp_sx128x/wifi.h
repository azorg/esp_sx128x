/*
 * Wi-Fi wrappers
 * File: "wifi.h"
 */

#pragma once
#ifndef WIFI_H
#define WIFI_H
//-----------------------------------------------------------------------------
#include <WiFi.h>
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
bool wifi_connect(const char *ssid, const char *passwd, bool auto_reconnect);
bool wifi_reconnect();
bool wifi_connected();
void wifi_disconnect();
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // WIFI_H

/*** end of "wifi.h" file ***/


