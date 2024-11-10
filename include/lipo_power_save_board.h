#ifndef __LIPO_POWER_SAVE_BOARD_UTILS_H__
#define __LIPO_POWER_SAVE_BOARD_UTILS_H__

#include <Adafruit_NeoPixel.h>
#include "driver/gpio.h"
#include <driver/rtc_io.h>


constexpr int uS_TO_S_FACTOR = 1000000ULL;  /* Conversion factor for micro seconds to seconds */

class LipoPowerSaveBoard {
 public:
  LipoPowerSaveBoard()
    : _pixels(1, 21, NEO_GRB + NEO_KHZ800), _brightness(10) {
    pinMode(_enableBatteryPin, OUTPUT);
    pinMode(_enablePumpPin, OUTPUT);
    pinMode(_enableVCC33Pin, OUTPUT_OPEN_DRAIN);
    pinMode(_enableRGBPin, OUTPUT_OPEN_DRAIN);
    _pixels.begin();
    _pixels.setBrightness(_brightness);
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

 private:
  int _enableVCC33Pin = 35;
  int _enablePumpPin = 13;
  int _enableBatteryPin = 11;
  int _enableRGBPin = 33;

  Adafruit_NeoPixel _pixels;
  int _brightness;
};



#endif // __LIPO_POWER_SAVE_BOARD_UTILS_H__
