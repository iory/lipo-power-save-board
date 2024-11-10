#include "Arduino.h"

#include "lipo_power_save_board.h"
#include "deep_sleep_utils.h"


LipoPowerSaveBoard board;

void setup() {
  USBSerial.begin(115200);
  while (!USBSerial) {
    delay(10);
  }
  USBSerial.println("start");
  manageWakeupReason();

  board.enableBattery();
  board.showStartupSequence();
  board.disableBattery();

  board.enterDeepSleep(10);
}

void loop() {
}
