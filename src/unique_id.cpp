#include "Arduino.h"
#include <ArduinoUniqueID.h>

void setup() {
  USBSerial.begin(115200);
  while (!USBSerial) {
    delay(10);
  }
  USBSerial.println("start");
}

void loop() {
  String unique_id = "";
  for(size_t i = 0; i < UniqueIDsize; i++) {
    if (UniqueID[i] < 0x10) unique_id += "0";
    unique_id += String(UniqueID[i], HEX);
  }
  USBSerial.println(unique_id);
  delay(1000);
}
