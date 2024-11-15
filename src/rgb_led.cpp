#include "Arduino.h"

#include "lipo_power_save_board.h"


LipoPowerSaveBoard board;

void setup() {
  board.enableBattery();
}

void loop() {
  board.showStartupSequence();
}
