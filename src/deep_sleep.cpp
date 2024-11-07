#include "Arduino.h"

constexpr int uS_TO_S_FACTOR = 1000000ULL;  /* Conversion factor for micro seconds to seconds */
constexpr int TIME_TO_SLEEP  = 100000;        /* Time ESP32 will go to sleep (in seconds) */
constexpr int EN_BATT = 45;
constexpr int EN_VCC33 = 35;

void setup() {
  pinMode(EN_VCC33, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_VCC33, LOW);

  pinMode(EN_BATT, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_BATT, LOW);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
}
