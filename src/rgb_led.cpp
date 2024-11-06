#include <Adafruit_NeoPixel.h>

constexpr int RGB_LED_PIN = 21;
constexpr int EN_RGB_LED_PIN = 33;
constexpr int EN_BATT = 2;
constexpr int NUMPIXELS = 1;

constexpr int EN_TXRX1 = 14;
constexpr int EN_TXRX2 = 4;
constexpr int EN_I2C = 4;
constexpr int EN_VCC33 = 4;

Adafruit_NeoPixel pixels(NUMPIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
constexpr int DELAYVAL = 20;  // 色変化の速度調整
uint16_t hue = 0;  // 色相を表す変数
constexpr int BRIGHTNESS = 10;  // 明度（0-255で指定、50はかなり暗い）

void setup() {
  USBSerial.begin(115200);
  pinMode(EN_BATT, OUTPUT_OPEN_DRAIN);
  // pinMode(EN_BATT, INPUT);
  digitalWrite(EN_BATT, HIGH);
  pinMode(EN_RGB_LED_PIN, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_RGB_LED_PIN, LOW);

  pinMode(EN_TXRX2, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_TXRX2, HIGH);
  pinMode(EN_TXRX1, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_TXRX1, HIGH);

  pinMode(EN_VCC33, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_VCC33, HIGH);

  pinMode(EN_I2C, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_I2C, HIGH);

  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);  // 明度を設定
  USBSerial.println("start");
}

float readBatteryVoltage() {
  uint32_t vin = 0;
  for(int i = 0; i < 16; i++) {
    vin = vin + analogReadMilliVolts(A0); // ADC with correction
    USBSerial.println(vin);
  }
  float V_batt = vin / 16.0;
  return V_batt;
}


bool currentState = false;
void loop() {
  USBSerial.println("loop");

  if (currentState) {
    // digitalWrite(EN_RGB_LED_PIN, LOW);
    digitalWrite(EN_BATT, LOW);
    USBSerial.println("digitalWrite(EN_BATT, LOW);");
  } else {
    // digitalWrite(EN_RGB_LED_PIN, HIGH);
    digitalWrite(EN_BATT, HIGH);
    USBSerial.println("digitalWrite(EN_BATT, HIGH);");
  }
  currentState = !currentState;
  delay(3000);


  // 色相を徐々に変化させてグラデーションを作成
  hue += 256;  // 色相の増加。小さくすると変化が遅くなる。65536で一周
  if (hue >= 65536) {
    hue = 0;  // 一周したらリセット
  }

  // 現在の色相に基づいて色を設定
  uint32_t color = pixels.ColorHSV(hue, 255, 255);  // 彩度・明度は最大
  pixels.setPixelColor(0, color);
  pixels.show();

  float volt = readBatteryVoltage();

  delay(DELAYVAL);  // 次の色に移行する前に少し待つ
}
