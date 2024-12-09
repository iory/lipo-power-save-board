#include "Arduino.h"

#include "ics_air_relay.h"

const long BAUDRATE = 1250000;
const int TIMEOUT = 20;
const int KJS_ID = 20;
byte TX_PIN1 = 17;
byte RX_PIN1 = 18;
float pressure;

IcsAirRelay ics(&Serial1, BAUDRATE, TIMEOUT);

void setup()
{
  USBSerial.begin(115200);
  while (!USBSerial) {
    delay(1); // Wait for serial to connect
  }
  USBSerial.println("Hello!");
  Serial1.begin(BAUDRATE, SERIAL_8E1, RX_PIN1, TX_PIN1, false, TIMEOUT);
  ics.begin();
}

void loop()
{
  if (ics.getPressure(KJS_ID, &pressure) == -1) {
    USBSerial.println("invalid");
  } else {
    USBSerial.println(pressure);
  }
  delay(10);
}
