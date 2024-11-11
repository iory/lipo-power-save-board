const char* ssid = "";   //your network SSID
const char* password = "";   //your network password
const char *GScriptId = "";

constexpr int TIME_TO_SLEEP  = 10;        /* Time ESP32 will go to sleep (in seconds) */
constexpr float pressureThreshold = -7.0;
constexpr float maxPressureThreshold = -15.0;
constexpr int historySize = 10;
