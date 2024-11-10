#include "Arduino.h"

#include "lipo_power_save_board.h"


LipoPowerSaveBoard board;

void setup() {
  USBSerial.begin(115200);
  while (!USBSerial) {
    delay(10);
  }
  USBSerial.println("start");

  board.enableBattery();
  board.showStartupSequence();
}

void loop() {
  USBSerial.println(analogRead(A0));
}
