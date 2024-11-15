#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#include <esp_sntp.h>

// NTPサーバーとタイムゾーン設定
const unsigned long NTP_TIMEOUT_MS = 5000;
const char* TIMEZONE = "JST-9"; // 日本標準時 (UTC+9)
const char* NTP_SERVER_1 = "ntp.nict.jp";
const char* NTP_SERVER_2 = "time.google.com";
const char* NTP_SERVER_3 = "ntp.jst.mfeed.ad.jp";

/* Pure function that waits for NTP server time sync.
 *
 * Returns true if time was set successfully, otherwise false.
 * The result is written into the timeInfo structure.
 *
 * Note: Must be connected to WiFi to get time from NTP server.
 */
bool waitForSNTPSync(struct tm *timeInfo, unsigned long ntpTimeout = NTP_TIMEOUT_MS,
                     const char* timezone = TIMEZONE, const char* ntpServer1 = NTP_SERVER_1, const char* ntpServer2 = NTP_SERVER_2,
                     const char* ntpServer3 = NTP_SERVER_3)
{
  configTzTime(timezone, ntpServer1, ntpServer2);

  unsigned long timeout = millis() + ntpTimeout;
  while ((sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) && (millis() < timeout))
  {
    delay(100); // ms
  }

  return getLocalTime(timeInfo);
}

/* Pure function that formats a time_t value into a string.
 *
 * The formatted time is written into the buffer in UTC (Z) format.
 */
void formatTime(time_t rawTime, char* buffer, size_t bufferSize) {
  struct tm* timeInfo = gmtime(&rawTime);  // Get time in UTC
  strftime(buffer, bufferSize, "%Y-%m-%dT%H:%M:%SZ", timeInfo);
}

#endif // __TIME_UTILS_H__
