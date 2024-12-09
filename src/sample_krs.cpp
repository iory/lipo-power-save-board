#include <IcsHardSerialClass.h>

const byte TX_PIN = 17;
const byte RX_PIN = 18;

const long BAUDRATE = 1250000;
const int TIMEOUT = 1000;
IcsHardSerialClass *krs;

void setup()
{
  USBSerial.begin(115200);
  while (!USBSerial) {
    delay(10);
  }
  USBSerial.println("start");

  // === Servo === //
  // ICS : データ長8bit, パリティEVEN, ストップビット1, 極性反転なし
  // pinMode(TX_PIN, OUTPUT_OPEN_DRAIN);
  Serial1.begin(BAUDRATE, SERIAL_8E1, RX_PIN, TX_PIN, false, TIMEOUT);
  krs = new IcsHardSerialClass(&Serial1, BAUDRATE, TIMEOUT);
  krs->begin();
}

float rotations;

void loop()
{
  // krs->setServoPosition(0, 3500);
  // delay(1000);
  USBSerial.println(krs->getPosition(0));
  delay(10);
  // krs->setServoPosition(0, 11500);
  // delay(1000);
  // USBSerial.println(krs->getPosition(0));
  // delay(10);
}
