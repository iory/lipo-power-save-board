#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include "HTTPSRedirect.h"
#include <OneButton.h>
#include <ArduinoUniqueID.h>
#include <Adafruit_SGP30.h>

#include "lipo_power_save_board.h"
#include "deep_sleep_utils.h"
#include "wifi_utils.h"
#include "time_utils.h"
#include "config.h"


Adafruit_SGP30 sgp;
LipoPowerSaveBoard board;
OneButton btn(board.getButtonPin(), true, true);

struct WiFiTaskParams {
  int dataIndex;
};

enum Mode {
  NOTHING,
  VACCUM_MODE,
  MODE_COUNT
};

RTC_DATA_ATTR int mode = NOTHING;
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR float batteryVoltages[historySize];
RTC_DATA_ATTR float pressures[historySize];
RTC_DATA_ATTR float preSleepPressures[historySize];
RTC_DATA_ATTR time_t timestamps[historySize];
RTC_DATA_ATTR int bootCountList[historySize];
RTC_DATA_ATTR int TVOCList[historySize];
RTC_DATA_ATTR int eCO2List[historySize];
RTC_DATA_ATTR int H2List[historySize];
RTC_DATA_ATTR int EthanolList[historySize];
RTC_DATA_ATTR bool timeConfigured = false;
RTC_DATA_ATTR bool sgpConfigured = false;
RTC_DATA_ATTR time_t nextTime = 0;
RTC_DATA_ATTR time_t currentTime = 0;
RTC_DATA_ATTR int dataIndex = 0;
RTC_DATA_ATTR uint16_t TVOC_base;
RTC_DATA_ATTR uint16_t eCO2_base;

TaskHandle_t wifiTaskHandle = NULL;
TaskHandle_t timeSyncTaskHandle = NULL;
TaskHandle_t co2SensorTaskHandle = NULL;
bool co2TaskEnd = false;
bool wifiTaskEnd = false;

const char* host = "script.google.com";
const int httpsPort = 443;

String url = String("/macros/s/") + GScriptId + "/exec";
String payload = "";

bool longPress = false;

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
  return absoluteHumidityScaled;
}


void enterTimedDeepSleep() {
  nextTime = currentTime + TIME_TO_SLEEP;
  board.disableBattery();
  board.disableVCC33();
  board.enterDeepSleep(TIME_TO_SLEEP);
}

void enterDeepSleepUntilButtonPress() {
  bootCount = 0;
  timeConfigured = false;
  board.disableBattery();
  board.disableVCC33();
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
  if (co2SensorTaskHandle != NULL) {
    vTaskDelete(co2SensorTaskHandle);
    co2SensorTaskHandle = NULL;
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


void co2SensorTask(void *parameter) {
  board.enableVCC33();
  board.enableI2C();

  Wire.begin(46, 45); // (SDA, SCL)
  delay(100);
  if (!sgp.begin()){
    USBSerial.println("Sensor not found :(");
    while (1) {
      USBSerial.println("Sensor not found :(");
    }
  }
  float temperature = 22.1; // [°C]
  float humidity = 45.2; // [%RH]
  sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

  int sgp_init_counter = 0;
  while (sgp_init_counter < 15) {
    if (!sgp.IAQmeasure()) {
      USBSerial.println("Measurement failed");
      continue;
    }
    USBSerial.print("Waiting for ");
    USBSerial.print(15 - sgp_init_counter);
    USBSerial.println("[s] to initialize air quality..");
    delay(1000);
    sgp_init_counter++;
  }
  if (!sgpConfigured) {
    if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      USBSerial.println("Failed to get baseline readings");
    }
    USBSerial.print("****Baseline values: eCO2: 0x"); USBSerial.print(eCO2_base, HEX);
    USBSerial.print(" & TVOC: 0x"); USBSerial.println(TVOC_base, HEX);
    USBSerial.println("");
    sgpConfigured = true;
  } else {
    sgp.setIAQBaseline(eCO2_base, TVOC_base);
  }

  if (!sgp.IAQmeasure()) {
    USBSerial.println("[CO2 Sensor] Measurement failed");
  }
  USBSerial.print("TVOC "); USBSerial.print(sgp.TVOC); USBSerial.print(" ppb\t");
  USBSerial.print("eCO2 "); USBSerial.print(sgp.eCO2); USBSerial.println(" ppm");
  if (!sgp.IAQmeasureRaw()) {
    USBSerial.println("Raw Measurement failed");
  }

  board.disableVCC33();
  board.disableI2C();

  co2TaskEnd = true;
  co2SensorTaskHandle = NULL;
  vTaskDelete(NULL);
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


void uploadData(int index) {
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
  fullUrl += "]&tvoc=[";
  for (int i = 0; i < index; i++) {
    fullUrl += String(TVOCList[i]);
    if (i < index - 1) fullUrl += ",";
  }
  fullUrl += "]&eco2=[";
  for (int i = 0; i < index; i++) {
    fullUrl += String(eCO2List[i]);
    if (i < index - 1) fullUrl += ",";
  }
  fullUrl += "]&h2=[";
  for (int i = 0; i < index; i++) {
    fullUrl += String(H2List[i]);
    if (i < index - 1) fullUrl += ",";
  }
  fullUrl += "]&ethanol=[";
  for (int i = 0; i < index; i++) {
    fullUrl += String(EthanolList[i]);
    if (i < index - 1) fullUrl += ",";
  }

  fullUrl += "]&unique_ids=[";
  for (int i = 0; i < index; i++) {
    fullUrl += "\"";
    for(size_t i = 0; i < UniqueIDsize; i++) {
      if (UniqueID[i] < 0x10) fullUrl += "0";
      fullUrl += String(UniqueID[i], HEX);
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
  int index = params->dataIndex;
  uploadData(index);
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
    board.stopVacuum();

    xTaskCreatePinnedToCore(co2SensorTask, "co2Sensor Task", 2048, NULL, 1, &co2SensorTaskHandle, 1);

    uint32_t vin = 0;
    for(int i = 0; i < 16; i++) {
      vin = vin + analogReadMilliVolts(A0); // ADC with correction
    }
    float V_batt = (vin / 16.0) / 1000.0;

    float first_pressure = board.getPressure();
    USBSerial.printf("pressure: %f kPa\n", first_pressure);
    float pressure = pumpControl(first_pressure);

    USBSerial.printf("boot Count: %d dataIndex: %d, historySize: %d\n", bootCount, dataIndex, historySize);

    if (longPress) {
      USBSerial.printf("Shutting Down\n");
      return;
    }

    if (mode == NOTHING) {
      enterDeepSleepUntilButtonPress();
    }

    if (!timeConfigured) {
      while (timeSyncTaskHandle != NULL) {
        delay(10);
      }
    }

    while (co2TaskEnd == false) {
      delay(10);
    }

    if (dataIndex < historySize) {
      timestamps[dataIndex] = currentTime;
      batteryVoltages[dataIndex] = V_batt;
      pressures[dataIndex] = first_pressure;
      preSleepPressures[dataIndex] = pressure;
      bootCountList[dataIndex] = bootCount;
      TVOCList[dataIndex] = sgp.TVOC;
      eCO2List[dataIndex] = sgp.eCO2;
      H2List[dataIndex] = sgp.rawH2;
      EthanolList[dataIndex] = sgp.rawEthanol;
      dataIndex++;
    }

    if (dataIndex >= historySize || bootCount == historySize) {
      USBSerial.printf("Enter uploading");
      unsigned long start_time = millis();

      WiFiTaskParams *params = (WiFiTaskParams *)malloc(sizeof(WiFiTaskParams));
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
