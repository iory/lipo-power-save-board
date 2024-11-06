#include <Arduino.h>
#include <WiFi.h>

void killWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

wl_status_t startWiFi(const char* ssid, const char* password, int &wifiRSSI, int wifiTimeout, int txPower) {
  WiFi.mode(WIFI_STA);
  Serial.printf("Connecting to '%s'", ssid);
  WiFi.begin(ssid, password);

  // https://intellectualcuriosity.hatenablog.com/entry/2024/01/30/123336
  WiFi.setTxPower((wifi_power_t)txPower);

  // timeout if WiFi does not connect in WIFI_TIMEOUT ms from now
  unsigned long timeout = millis() + wifiTimeout;
  wl_status_t connection_status = WiFi.status();

  while ((connection_status != WL_CONNECTED) && (millis() < timeout)) {
    Serial.print(".");
    delay(50);
    connection_status = WiFi.status();
  }
  Serial.println();

  if (connection_status == WL_CONNECTED) {
    wifiRSSI = WiFi.RSSI(); // get WiFi signal strength now, because the WiFi
                            // will be turned off to save power!
    Serial.println("IP: " + WiFi.localIP().toString());
  } else{
    Serial.printf("Could not connect to '%s'\n", ssid);
    if (connection_status == WL_NO_SSID_AVAIL) {
      Serial.println("Network Not Available");
    } else {
      Serial.println("WiFi Connection Failed");
    }
    killWiFi();
  }
  return connection_status;
}
