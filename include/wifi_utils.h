#ifndef __WIFI_UTILS_H__
#define __WIFI_UTILS_H__

#include <Arduino.h>

void killWiFi();
wl_status_t startWiFi(const char* ssid, const char* password, int &wifiRSSI, int wifiTimeout = 5000, int txPower = 60);

#endif
