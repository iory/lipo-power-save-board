#ifndef __RGB_LED_UTILS_H__
#define __RGB_LED_UTILS_H__

#include <Adafruit_NeoPixel.h>

class RGBLED {
 public:
  RGBLED(int pin = 21, int numPixels = 1, int brightness = 10, int enablePin = 33);
  void setBrightness(int brightness);
  void setColor(uint16_t hue, uint8_t saturation = 255, uint8_t value = 255);
  void showStartupSequence() {
    enable();
    uint16_t hue = 0;
    for (int i = 0; i < 256; ++i) {
      hue += 256;
      if (hue >= 65536) {
        hue = 0;
      }
      setColor(hue);
      delay(5);
    }
    disable();
  }
  void enable() {
    digitalWrite(_enablePin, LOW);
  }
  void disable() {
    digitalWrite(_enablePin, HIGH);
  }

 private:
  Adafruit_NeoPixel pixels;
  int brightness;
  int _enablePin;
};


RGBLED::RGBLED(int pin, int numPixels, int brightness, int enablePin)
: pixels(numPixels, pin, NEO_GRB + NEO_KHZ800), brightness(brightness), _enablePin(enablePin) {
  pixels.begin();
  pixels.setBrightness(brightness);
  pinMode(_enablePin, OUTPUT_OPEN_DRAIN);
}

void RGBLED::setBrightness(int brightness) {
  this->brightness = brightness;
  pixels.setBrightness(brightness);
}

void RGBLED::setColor(uint16_t hue, uint8_t saturation, uint8_t value) {
  uint32_t color = pixels.ColorHSV(hue, saturation, value);
  pixels.setPixelColor(0, color);
  pixels.show();
}

#endif // __RGB_LED_UTILS_H__
