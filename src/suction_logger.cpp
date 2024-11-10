#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include "HTTPSRedirect.h"
#include <OneButton.h>
#include <ArduinoUniqueID.h>

#include "lipo_power_save_board.h"
#include "deep_sleep_utils.h"
#include "wifi_utils.h"
#include "time_utils.h"
#include "config.h"


LipoPowerSaveBoard board;
OneButton btn(board.getButtonPin(), true, true);

struct WiFiTaskParams {
  float V_batt;
  float first_pressure;
  float pressure;
  int dataIndex;
};

enum Mode {
  NOTHING,
  VACCUM_MODE,
  MODE_COUNT
};

constexpr int TIME_TO_SLEEP  = 30;        /* Time ESP32 will go to sleep (in seconds) */
constexpr float pressureThreshold = -5.0;
constexpr float maxPressureThreshold = -15.0;
constexpr int historySize = 5;
RTC_DATA_ATTR int mode = NOTHING;
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR float batteryVoltages[historySize];
RTC_DATA_ATTR float pressures[historySize];
RTC_DATA_ATTR float preSleepPressures[historySize];
RTC_DATA_ATTR time_t timestamps[historySize];
RTC_DATA_ATTR int bootCountList[historySize];
RTC_DATA_ATTR bool timeConfigured = false;
RTC_DATA_ATTR time_t nextTime = 0;
RTC_DATA_ATTR time_t currentTime = 0;
RTC_DATA_ATTR int dataIndex = 0;


TaskHandle_t wifiTaskHandle = NULL;
TaskHandle_t timeSyncTaskHandle = NULL;
bool wifiTaskEnd = false;

const char* host = "script.google.com";
const int httpsPort = 443;

String url = String("/macros/s/") + GScriptId + "/exec";
String payload = "";

bool longPress = false;


void enterTimedDeepSleep() {
  nextTime = currentTime + TIME_TO_SLEEP;
  board.enterDeepSleep(TIME_TO_SLEEP);
}

void enterDeepSleepUntilButtonPress() {
  bootCount = 0;
  timeConfigured = false;
  board.enterDeepSleepUntilButtonPress();
}


static void handleLongPress() {
  USBSerial.println("handleLongPress Entered");
  longPress = true;

  if (timeSyncTaskHandle != NULL) {
    vTaskDelete(timeSyncTaskHandle);
    timeSyncTaskHandle = NULL;
  }
  if (wifiTaskHandle != NULL) {
    vTaskDelete(wifiTaskHandle);
    wifiTaskHandle = NULL;
  }

  board.disableBattery();
  board.disablePump();
  delay(10);
  board.enableBattery();
  // Wait for air relay board wakeup.
  delay(10);

  unsigned long pump_timeout = millis() + 2000;
  unsigned long current_time = millis();
  USBSerial.println("Release Vacuum");
  while (current_time < pump_timeout) {
    board.releaseVacuum();
    delay(10);
    current_time = millis();
  }

  // Disable solenoid.
  board.stopVacuum();
  // Disable Pump Power Board.
  board.disableBattery();

  mode = NOTHING;
  enterDeepSleepUntilButtonPress();
  longPress = false;
}


void ButtonTask(void *parameter) {
  btn.setClickMs(200);  // Timeout used to distinguish single clicks from double clicks. (msec)
  btn.attachLongPressStart(handleLongPress);
  while (true) {
    btn.tick();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}


void syncTime() {
  int wifiRSSI = 0;
  wl_status_t wifiStatus = startWiFi(ssid, password, wifiRSSI);
  if (wifiStatus == WL_CONNECTED) {
    configTzTime(TIMEZONE, NTP_SERVER_1, NTP_SERVER_2);
    struct tm timeInfo;
    if (!waitForSNTPSync(&timeInfo)) {
      USBSerial.println("Failed to synchronize time.");
    } else {
      USBSerial.println("Time synchronized successfully.");
      currentTime = mktime(&timeInfo); // save ntp time
      USBSerial.println(currentTime);
      timeConfigured = true;
    }
  }
  // killWiFi();
}


void uploadData(float V_batt, float first_pressure, float pressure, int index) {
  int wifiRSSI = 0; // “Received Signal Strength Indicator"
  wl_status_t wifiStatus = startWiFi(ssid, password, wifiRSSI);
  if (wifiStatus != WL_CONNECTED) {
    USBSerial.println("Failed to connect Wifi");
    wifiTaskEnd = true;
    return;
  }
  HTTPSRedirect* client = new HTTPSRedirect(httpsPort);
  USBSerial.print("Connecting to ");
  USBSerial.println(host);

  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setTimeout(5000);
  client->connect(host, httpsPort);

  String fullUrl = url + "?timestamps=[";
  char timeString[25];
  for (int i = 0; i < index; i++) {
    formatTime(timestamps[i], timeString, sizeof(timeString));
    fullUrl += "\"" + String(timeString) + "\"";
    if (i < index - 1) fullUrl += ",";
  }
  fullUrl += "]&battery_voltages=[";
  for (int i = 0; i < index; i++) {
    fullUrl += String(batteryVoltages[i]);
    if (i < index - 1) fullUrl += ",";
  }
  fullUrl += "]&pressures=[";
  for (int i = 0; i < index; i++) {
    fullUrl += String(pressures[i]);
    if (i < index - 1) fullUrl += ",";
  }
  fullUrl += "]&pre_sleep_pressures=[";
  for (int i = 0; i < index; i++) {
    fullUrl += String(preSleepPressures[i]);
    if (i < index - 1) fullUrl += ",";
  }
  fullUrl += "]&boot_counts=[";
  for (int i = 0; i < index; i++) {
    fullUrl += String(bootCountList[i]);
    if (i < index - 1) fullUrl += ",";
  }
  fullUrl += "]&unique_ids=[";
  for (int i = 0; i < index; i++) {
    fullUrl += "\"";
    for(size_t i = 0; i < UniqueIDsize; i++) {
      fullUrl += String(UniqueID8[i]);
    }
    fullUrl += "\"";
    if (i < index - 1) fullUrl += ",";
  }
  fullUrl += "]";

  USBSerial.println(fullUrl);

  if (client->GET(fullUrl, host) == 0) {
    USBSerial.println("Data sent successfully");
  } else {
    USBSerial.println("Failed to send data");
  }
  delete client;

  configTzTime(TIMEZONE, NTP_SERVER_1, NTP_SERVER_2);
  struct tm timeInfo;
  if (!waitForSNTPSync(&timeInfo)) {
    USBSerial.println("Failed to synchronize time.");
  } else {
    USBSerial.println("Time synchronized successfully.");
    currentTime = mktime(&timeInfo);
  }
  wifiTaskEnd = true;
}


float pumpControl(float first_pressure) {
  unsigned long pump_timeout = millis() + 5000;
  unsigned long current_time = millis();
  float pressure = first_pressure;
  USBSerial.printf("pumpControl function: pressure: %f kPa long_press: %d \n, ",
                   pressure, longPress);
  if (!longPress && pressure > pressureThreshold) {
    current_time = millis();
    while (!longPress && pressure > maxPressureThreshold && (current_time < pump_timeout)) {
      board.startVacuum();
      current_time = millis();
      pressure = board.getPressure();
      char log_msg[50];
      sprintf(log_msg, "pressure: %f kPa", pressure);
      USBSerial.println(log_msg);
    }
    if (!longPress) {
      for (int i = 0; i < 2; ++i) {
        board.stopVacuum();
      }
    }
  }
  if (current_time >= pump_timeout) {
    mode = NOTHING;
  }
  return pressure;
}

void WiFiTask(void *parameter) {
  WiFiTaskParams *params = (WiFiTaskParams *)parameter;
  float V_batt = params->V_batt;
  float first_pressure = params->first_pressure;
  float pressure = params->pressure;
  int index = params->dataIndex;
  uploadData(V_batt, first_pressure, pressure, index);
  vTaskDelete(NULL);
}


void timeSyncTask(void *parameter) {
  syncTime();
  timeSyncTaskHandle = NULL;
  vTaskDelete(NULL);
}


void setup() {
    USBSerial.begin(115200);
    USBSerial.println("hello");
    bootCount++;

    xTaskCreatePinnedToCore(ButtonTask, "Button Task", 2048, NULL, 24, NULL, 0);

    esp_sleep_wakeup_cause_t wakeup_reason = manageWakeupReason();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
      mode = VACCUM_MODE;
    }
    if (mode == NOTHING) {
      enterDeepSleepUntilButtonPress();
    }

    if (!timeConfigured) {
      xTaskCreatePinnedToCore(timeSyncTask, "Time Sync Task",
                              8192, NULL, 1, &timeSyncTaskHandle, 1);
    } else {
      currentTime = nextTime;
    }

    board.enableBattery();
    delay(20);

    float first_pressure = board.getPressure();
    USBSerial.printf("pressure: %f kPa\n", first_pressure);
    float pressure = pumpControl(first_pressure);

    USBSerial.printf("boot Count: %d dataIndex: %d, historySize: %d\n", bootCount, dataIndex, historySize);

    if (longPress) {
      USBSerial.printf("Shutting Down\n");
      return;
    }

    board.disableBattery();
    board.disableVCC33();

    if (mode == NOTHING) {
      enterDeepSleepUntilButtonPress();
    }

    if (!timeConfigured) {
      while (timeSyncTaskHandle != NULL) {
        delay(10);
      }
    }

    if (dataIndex < historySize) {
      timestamps[dataIndex] = currentTime;
      batteryVoltages[dataIndex] = 0.0;
      pressures[dataIndex] = first_pressure;
      preSleepPressures[dataIndex] = pressure;
      bootCountList[dataIndex] = bootCount;
      dataIndex++;
    }

    if (dataIndex >= historySize || bootCount == historySize) {
      USBSerial.printf("Enter uploading");
      unsigned long start_time = millis();

      WiFiTaskParams *params = (WiFiTaskParams *)malloc(sizeof(WiFiTaskParams));
      params->V_batt = 0;
      params->first_pressure = first_pressure;
      params->pressure = pressure;
      params->dataIndex = dataIndex;

      wifiTaskEnd = false;
      xTaskCreatePinnedToCore(WiFiTask, "WiFi Task", 8192, params, 1, &wifiTaskHandle, 1);
      dataIndex = 0;
      free(params);

      unsigned long elapsed_time = millis() - start_time;
    } else {
      wifiTaskEnd = true;
      if (digitalRead(board.getButtonPin()) == HIGH) {
        delay(2000);
      }
    }

    while ((!wifiTaskEnd) | longPress) {
      delay(10);
      if (mode == NOTHING) {
        enterDeepSleepUntilButtonPress();
      }
    }
    enterTimedDeepSleep();
}

void loop() {
  delay(10);
}
