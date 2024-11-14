const char* ssid = "";   //your network SSID
const char* password = "";   //your network password
const char *GScriptId = "";

constexpr int TIME_TO_SLEEP  = 180;        /* Time ESP32 will go to sleep (in seconds) */
constexpr float pressureThreshold = -8.0;
// Set low threshold for bad pumps to work.
// Instead of low threshold, try vacuuming for 1[s] after reaching threshold
constexpr float maxPressureThreshold = -12.0;
constexpr int historySize = 10;
