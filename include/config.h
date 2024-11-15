const char* ssid = "";   //your network SSID (2.4G)
const char* password = "";   //your network password

char* getGScriptId() {
  static char unique_id[UniqueIDsize * 2 + 1]; // 16進文字列＋終端文字
  static char result[128]; // 結果文字列の静的メモリ

  // UniqueIDを16進数文字列に変換
  for (size_t i = 0; i < UniqueIDsize; i++) {
    snprintf(&unique_id[i * 2], 3, "%02x", UniqueID[i]);
  }

  // UniqueIDに応じた文字列を返す
  if (strcmp(unique_id, "MAC_ADDRESS2") == 0) {
    strcpy(result, "GScriptId_1");
  } else if (strcmp(unique_id, "MAC_ADDRESS2") == 0) {
    strcpy(result, "GScriptId_2");
  } else {
    strcpy(result, "GScriptId_3");
  }

  return result;
}

constexpr int TIME_TO_SLEEP  = 180;        /* Time ESP32 will go to sleep (in seconds) */
constexpr float pressureThreshold = -8.0;
// Set low threshold for bad pumps to work.
// Instead of low threshold, try vacuuming for 1[s] after reaching threshold
constexpr float maxPressureThreshold = -12.0;
constexpr int historySize = 10;
