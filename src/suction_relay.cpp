#include "Arduino.h"

#include "lipo_power_save_board.h"

LipoPowerSaveBoard board;

unsigned long suc_start_time = 0;
bool suc_set_time = false;
float pressure;
bool suc = false;

void setup()
{
  board.enableBattery();
  board.showStartupSequence();

  USBSerial.begin(115200);
  while (!USBSerial) {
    delay(1); // Wait for serial to connect
  }
  USBSerial.println("Hello!");
}

void loop()
{
  if (suc == false && pressure > -5) {
    board.startVacuum();
    suc = true;
    suc_set_time = true; // Start the 5-second timer
    suc_start_time = millis(); // Record the current time
  } else if (suc == true && pressure < -14) {
    board.stopVacuum();
    suc = false;
  }

  // Check if 5 seconds have passed since setting suc == true
  if (suc_set_time && millis() - suc_start_time >= 5000) {
    board.releaseVacuum();
    suc = false;
    delay(2000);
    suc_set_time = false; // Reset the flag
  }

  pressure = board.getPressure();
  USBSerial.print(pressure);
  USBSerial.print(' ');
  USBSerial.println(suc);
  delay(10);

}
