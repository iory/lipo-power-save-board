#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "wifi_utils.h"
#include "time_utils.h"

bool timeConfigured = false;
const char* ssid     = ""; //your network SSID
const char* password = ""; //your network password
time_t currentTime = 0;

void firstSyncTime() {
  int wifiRSSI = 0; // “Received Signal Strength Indicator"
  wl_status_t wifiStatus = startWiFi(ssid, password, wifiRSSI);
  if (wifiStatus == WL_CONNECTED) {
    configTzTime(TIMEZONE, NTP_SERVER_1, NTP_SERVER_2);
    struct tm timeInfo;
    if (!waitForSNTPSync(&timeInfo)) {
      USBSerial.println("Failed to synchronize time.");
    } else {
      USBSerial.println("Time synchronized successfully.");
      currentTime = mktime(&timeInfo);
      USBSerial.println(currentTime);
      timeConfigured = true;
    }
  }
  killWiFi();
}


void setup() {
  USBSerial.begin(115200);
  firstSyncTime();
}

void loop() {
  firstSyncTime();
  struct tm *timeInfo = localtime(&currentTime);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
  USBSerial.println(buffer);
  delay(1000);
}
