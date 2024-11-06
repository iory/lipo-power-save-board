#include <Adafruit_NeoPixel.h>

constexpr int RGB_LED_PIN = 21;
constexpr int EN_RGB_LED_PIN = 33;
constexpr int EN_BATT = 2;
constexpr int NUMPIXELS = 1;


Adafruit_NeoPixel pixels(NUMPIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
constexpr int DELAYVAL = 20;  // 色変化の速度調整
uint16_t hue = 0;  // 色相を表す変数
constexpr int BRIGHTNESS = 10;  // 明度（0-255で指定、50はかなり暗い）

void setup() {
  USBSerial.begin(115200);
  pinMode(EN_BATT, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_BATT, LOW);
  pinMode(EN_RGB_LED_PIN, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_RGB_LED_PIN, LOW);
  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);  // 明度を設定
  USBSerial.println("start");
}

void loop() {
  USBSerial.println("loop");

  // 色相を徐々に変化させてグラデーションを作成
  hue += 256;  // 色相の増加。小さくすると変化が遅くなる。65536で一周
  if (hue >= 65536) {
    hue = 0;  // 一周したらリセット
  }

  // 現在の色相に基づいて色を設定
  uint32_t color = pixels.ColorHSV(hue, 255, 255);  // 彩度・明度は最大
  pixels.setPixelColor(0, color);
  pixels.show();

  delay(DELAYVAL);  // 次の色に移行する前に少し待つ
}
