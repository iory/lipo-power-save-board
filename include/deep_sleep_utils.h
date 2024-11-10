#ifndef __DEEP_SLEEP_UTILS_H__
#define __DEEP_SLEEP_UTILS_H__

#include "esp_sleep.h"


esp_sleep_wakeup_cause_t manageWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  // Check if ESP_SLEEP_WAKEUP_EXT0 was the wakeup cause by directly comparing it here.
  // This first USBSerial output is added to verify that output is working correctly before the switch statement.
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    USBSerial.printf("[ESP_SLEEP_WAKEUP_EXT0] Woken up by external interrupt (RTC_IO)\n");
  }

  switch(wakeup_reason){
  case ESP_SLEEP_WAKEUP_EXT0:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_EXT0] Woken up by external interrupt (RTC_IO)\n");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_EXT1] Woken up by external interrupt (RTC_CNTL) IO=%llX\n", esp_sleep_get_ext1_wakeup_status());
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_TIMER] Woken up by timer interrupt\n");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_TOUCHPAD] Woken up by touch interrupt PAD=%d\n", esp_sleep_get_touchpad_wakeup_status());
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_ULP] Woken up by ULP program\n");
    break;
  case ESP_SLEEP_WAKEUP_GPIO:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_GPIO] Woken up from light sleep by GPIO interrupt\n");
    break;
  case ESP_SLEEP_WAKEUP_UART:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_UART] Woken up from light sleep by UART interrupt\n");
    break;
  case ESP_SLEEP_WAKEUP_WIFI:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_WIFI] Woken up from light sleep by WiFi\n");
    break;
  case ESP_SLEEP_WAKEUP_COCPU:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_COCPU] Woken up by COCPU interrupt\n");
    break;
  case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG] Woken up by COCPU crash\n");
    break;
  case ESP_SLEEP_WAKEUP_BT:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_BT] Woken up from light sleep by Bluetooth\n");
    break;
  case ESP_SLEEP_WAKEUP_UNDEFINED:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_UNDEFINED] Wakeup reason undefined (not from deep sleep)\n");
    break;
  case ESP_SLEEP_WAKEUP_ALL:
    USBSerial.printf("[ESP_SLEEP_WAKEUP_ALL] All wakeup sources are disabled\n");
    break;
  default:
    USBSerial.printf("[ESP_SLEEP_WAKEUP] Woken up from non-sleep mode or unknown reason\n");
    break;
  }
  return wakeup_reason;
}


#endif // __DEEP_SLEEP_UTILS_H__
