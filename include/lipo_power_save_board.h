#ifndef __LIPO_POWER_SAVE_BOARD_UTILS_H__
#define __LIPO_POWER_SAVE_BOARD_UTILS_H__

#include <Adafruit_NeoPixel.h>
#include "driver/gpio.h"
#include <driver/rtc_io.h>
#include <ics_air_relay.h>

constexpr int uS_TO_S_FACTOR = 1000000ULL;  /* Conversion factor for micro seconds to seconds */
const long BAUDRATE = 1250000;
const int TIMEOUT = 1000;
const int KJS_ID = 19;

class LipoPowerSaveBoard {
 public:
  LipoPowerSaveBoard()
    : _pixels(1, 21, NEO_GRB + NEO_KHZ800), _brightness(10) {
    pinMode(_enableBatteryPin, OUTPUT);
    pinMode(_enablePumpPin, OUTPUT);
    pinMode(_enableVCC33Pin, OUTPUT_OPEN_DRAIN);
    pinMode(_enableRGBPin, OUTPUT_OPEN_DRAIN);
    pinMode(_enableI2CPin, OUTPUT_OPEN_DRAIN);
    pinMode(_enableTXRX1, OUTPUT_OPEN_DRAIN);
    digitalWrite(_enableTXRX1, LOW);
    pinMode(_enableTXRX2, OUTPUT_OPEN_DRAIN);
    digitalWrite(_enableTXRX2, LOW);
    _pixels.begin();
    _pixels.setBrightness(_brightness);
    initICS();
  }

  void enableBattery() {
    rtc_gpio_hold_dis((gpio_num_t)_enableBatteryPin);
    digitalWrite(_enableBatteryPin, HIGH);
  }
  void disableBattery() {
    digitalWrite(_enableBatteryPin, LOW);
    rtc_gpio_hold_en((gpio_num_t)_enableBatteryPin);
  }

  void enablePump() {
    digitalWrite(_enablePumpPin, HIGH);
  }
  void disablePump() {
    digitalWrite(_enablePumpPin, LOW);
  }

  void enableI2C() {
    digitalWrite(_enableI2CPin, LOW);
  }
  void disableI2C() {
    digitalWrite(_enableI2CPin, HIGH);
  }

  void enableVCC33() {
    digitalWrite(_enableVCC33Pin, LOW);
  }
  void disableVCC33() {
    digitalWrite(_enableVCC33Pin, HIGH);
  }

  void enableRGB() {
    digitalWrite(_enableRGBPin, LOW);
  }
  void disableRGB() {
    digitalWrite(_enableRGBPin, HIGH);
  }

  void enableTXRX1() {
    digitalWrite(_enableTXRX1, LOW);
  }
  void disableTXRX1() {
    digitalWrite(_enableTXRX1, HIGH);
  }
  void enableTXRX2() {
    digitalWrite(_enableTXRX2, LOW);
  }
  void disableTXRX2() {
    digitalWrite(_enableTXRX2, HIGH);
  }

  void setBrightness(int brightness) {
    this->_brightness = brightness;
    _pixels.setBrightness(brightness);
  }

  void setColor(uint16_t hue, uint8_t saturation = 255, uint8_t value = 255) {
    uint32_t color = _pixels.ColorHSV(hue, saturation, value);
    _pixels.setPixelColor(0, color);
    _pixels.show();
  }

  void showStartupSequence() {
    enableRGB();
    uint16_t hue = 0;
    for (int i = 0; i < 256; ++i) {
      hue += 256;
      if (hue >= 65536) {
        hue = 0;
      }
      setColor(hue);
      delay(5);
    }
    disableRGB();
  }

  void enterDeepSleep(int seconds) {
    rtc_gpio_pullup_en((gpio_num_t)GPIO_NUM_12);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, LOW);
    esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }

  void enterDeepSleepUntilButtonPress() {
    rtc_gpio_pullup_en((gpio_num_t)GPIO_NUM_12);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, LOW);
    esp_deep_sleep_start();
  }

  void initICS() {
    enableTXRX1();
    pinMode(_TX_PIN1, OUTPUT_OPEN_DRAIN);
    Serial1.begin(BAUDRATE, SERIAL_8E1, _RX_PIN1, _TX_PIN1, false, TIMEOUT);
    _krs = new IcsAirRelay(&Serial1, BAUDRATE, TIMEOUT);
    _krs->begin();
  }

  void deinitICS() {
    delete _krs;
  }

  float getPressure() {
    float pressure;
    while (_krs->getPressure(KJS_ID, &pressure) == -1) {
      delay(10);
    }
    return pressure;
  }

  float getAveragePressure(int n = 5) {
    float pressure = 0;
    for (int i = 0; i < n; ++i) {
      pressure += getPressure();
    }
    return pressure / n;
  }

  void releaseVacuum() {
    _krs->setGPIO(KJS_ID, 1, 1);
  }

  void startVacuum() {
    enablePump();
    delay(10);
    _krs->setGPIO(KJS_ID, 0, 1);
  }

  void stopVacuum() {
    _krs->setGPIO(KJS_ID, 0, 0);
    delay(10);
    disablePump();
  }

  int getButtonPin() {
    return 12;
  }

 private:
  int _enableVCC33Pin = 35;
  int _enablePumpPin = 13;
  int _enableBatteryPin = 11;
  int _enableRGBPin = 33;
  int _enableTXRX1 = 14;
  int _enableTXRX2 = 4;
  int _enableI2CPin = 34;
  byte _TX_PIN1 = 17;
  byte _RX_PIN1 = 18;

  IcsAirRelay *_krs;

  Adafruit_NeoPixel _pixels;
  int _brightness;
};



#endif // __LIPO_POWER_SAVE_BOARD_UTILS_H__
