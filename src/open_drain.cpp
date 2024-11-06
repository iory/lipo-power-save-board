#include <Arduino.h>

constexpr int RGB_LED_PIN = 21;
constexpr int EN_RGB_LED_PIN = 33;
constexpr int EN_BATT = 2;
constexpr int NUMPIXELS = 1;

constexpr int EN_TXRX1 = 14;
constexpr int EN_TXRX2 = 4;
constexpr int EN_I2C = 4;
constexpr int EN_VCC33 = 4;

void setup() {
  USBSerial.begin(115200);
  pinMode(EN_BATT, OUTPUT_OPEN_DRAIN);
  // pinMode(EN_BATT, INPUT);
  digitalWrite(EN_BATT, LOW);
  pinMode(EN_RGB_LED_PIN, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_RGB_LED_PIN, LOW);

  pinMode(EN_TXRX2, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_TXRX2, HIGH);
  pinMode(EN_TXRX1, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_TXRX1, HIGH);

  pinMode(EN_VCC33, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_VCC33, LOW);

  pinMode(EN_I2C, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_I2C, HIGH);

  USBSerial.println("start");
}


bool currentState = false;

void loop() {
  USBSerial.println("loop");
  if (currentState) {
    digitalWrite(EN_BATT, LOW);
    USBSerial.println("digitalWrite(EN_BATT, LOW);");
  } else {
    digitalWrite(EN_BATT, HIGH);
    USBSerial.println("digitalWrite(EN_BATT, HIGH);");
  }
  currentState = !currentState;
  delay(3000);
}
