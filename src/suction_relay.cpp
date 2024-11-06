#include "Arduino.h"
#include <IcsHardSerialClass.h>

// === Servo === //
const byte PSEUDO_EN_PIN = 7;

constexpr int EN_TXRX1 = 14;
constexpr int EN_TXRX2 = 4;
constexpr int EN_BATT = 2;
constexpr int EN_5V = 10;

const byte TX_PIN1 = 17;
const byte RX_PIN1 = 18;

const long BAUDRATE = 1250000;
const int TIMEOUT = 20;
const int KJS_ID = 20;
IcsHardSerialClass *krs;

unsigned long suc_start_time = 0;
bool suc_set_time = false;

void setup()
{
  pinMode(EN_BATT, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_BATT, LOW);
  pinMode(EN_TXRX1, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_TXRX1, LOW);

  pinMode(EN_TXRX2, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_TXRX1, HIGH);

  pinMode(EN_5V, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_5V, LOW);

  USBSerial.begin(115200);
  while (!USBSerial) {
    delay(1); // Wait for serial to connect
  }
  USBSerial.println("Hello!");

  // === Servo === //
  pinMode(TX_PIN1, OUTPUT_OPEN_DRAIN);
  Serial.begin(BAUDRATE, SERIAL_8E1, RX_PIN1, TX_PIN1, false, TIMEOUT);
  krs = new IcsHardSerialClass(&Serial, PSEUDO_EN_PIN, BAUDRATE, TIMEOUT);
  krs->begin();
}

float suction() {
  float pressure = -1000.0;
  while (pressure < -900) {
    std::vector<byte> rx_buff = krs->getSubcommandPacket(KJS_ID);
    for (int i = 0; i < rx_buff.size(); ++i) {
      USBSerial.print(rx_buff[i]);
      USBSerial.print(' ');
    }
    USBSerial.println("");

    if (rx_buff.size() > 0) {
      // 先頭2byte分捨てる
      std::vector<byte> partial_buff(rx_buff.begin() + 2, rx_buff.end());
      pressure = krs->getPressureFromPacket(partial_buff);
      char log_msg[50];
      sprintf(log_msg, "pressure: %f kPa", pressure);
      USBSerial.println(log_msg);
    }
  }
  return pressure;
}

float pressure = 10000;
bool suc = false;

void loop()
{
  if (suc == false && pressure > -12) {
    krs->setGPIO(KJS_ID, 1, 0);
    suc = true;
    suc_set_time = true; // Start the 5-second timer
    suc_start_time = millis(); // Record the current time
  } else if (suc == true && pressure < -17) {
    suc = false;
    krs->setGPIO(KJS_ID, 0, 0);
  }

  // Check if 5 seconds have passed since setting suc == true
  if (suc_set_time && millis() - suc_start_time >= 5000) {
    krs->setGPIO(KJS_ID, 0, 1); // Set GPIO after 5 seconds
    delay(2000);
    suc_set_time = false; // Reset the flag
  }

  pressure = suction();
  delay(10);
}
