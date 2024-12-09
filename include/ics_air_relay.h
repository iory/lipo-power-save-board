#include <IcsHardSerialClass.h>


class IcsAirRelay : public IcsHardSerialClass {
public:
  IcsAirRelay(HardwareSerial* serial, long baudRate, int timeout, int enPin = -1) : IcsHardSerialClass(serial, baudRate, timeout, enPin) {}
  int setGPIO(byte id, bool gpio0, bool gpio1) {
    const int sub_command_NONE = 0x40;
    const int sub_command_SW_ADC = 0x41;
    const int sub_command_PS = 0x42;
    const int sub_command_IMU = 0x44;
    const int sub_command_MAGENC = 0x48;
    const int sub_command_ALL = 0x90 | (2 * gpio1) | gpio0;
    const int ADC_CHANNEL_NUM = 5;
    byte txCmd[2];
    uint8_t recv_length = 2;
    uint16_t adc_buff[ADC_CHANNEL_NUM] = {};
    uint8_t SC_send = sub_command_ALL; // SC_SW_ADC, SC_PS, SC_IMU, SC_MAGENC;
    byte rx_buff[recv_length];
    unsigned int reData;
    bool flg;

    txCmd[0] = 0xA0 + id;    // CMD
    txCmd[1] = SC_send;
    flg = synchronize(txCmd, sizeof txCmd, rx_buff, sizeof rx_buff);
    if (flg == false) {
      return ICS_FALSE;
    }
    return 0;
  }

  uint16_t getLen(uint8_t sub_command_raw) {
    // SENSOR sub_command
    const int sub_command_NONE = 0x40;
    const int sub_command_SW_ADC = 0x41;
    const int sub_command_PS = 0x42;
    const int sub_command_IMU = 0x44;
    const int sub_command_MAGENC = 0x48;
    const int sub_command_ALL = (sub_command_SW_ADC | sub_command_PS | sub_command_IMU | sub_command_MAGENC);
    const int sub_command_SW_ADC_R = (sub_command_SW_ADC & 0x0F);
    const int sub_command_PS_R = (sub_command_PS & 0x0F);
    const int sub_command_IMU_R = (sub_command_IMU & 0x0F);
    const int sub_command_MAGENC_R = (sub_command_MAGENC & 0x0F);
    const int ADC_CHANNEL_NUM = 5;
    const int FORCE_CHANNEL_NUM = 4;
    const int PS_CHANNEL_NUM = 4;
    const int GYRO_CHANNEL_NUM = 3;
    const int ACC_CHANNEL_NUM = 3;
    const int MAGENC_CHANNEL_NUM = 1;
    const int ADC_LEN = ADC_CHANNEL_NUM * 2;
    const int PS_LEN = PS_CHANNEL_NUM * 3;
    const int GYRO_LEN = GYRO_CHANNEL_NUM * 3;
    const int ACC_LEN = ACC_CHANNEL_NUM * 3;
    const int IMU_LEN = GYRO_LEN + ACC_LEN;
    const int MAGENC_LEN = MAGENC_CHANNEL_NUM * 2;
    const int MAX_BUFFER_LENGTH = 2 + ADC_LEN + PS_LEN + IMU_LEN + MAGENC_LEN;
    uint16_t length = 2;
    uint8_t sub_command = sub_command_raw & 0x0F;
    if ((sub_command & sub_command_SW_ADC_R) == sub_command_SW_ADC_R) {
      length += ADC_LEN;
    }
    if ((sub_command & sub_command_PS_R) == sub_command_PS_R) {
      length += PS_LEN;
    }
    if ((sub_command & sub_command_IMU_R) == sub_command_IMU_R) {
      length += IMU_LEN;
    }
    if ((sub_command & sub_command_MAGENC_R) == sub_command_MAGENC_R) {
      length += MAGENC_LEN;
    }
    return length;
  }

  int getPressure(uint8_t id, float* pressure_out) {
    const uint8_t sub_command_NONE = 0x40;
    const uint8_t sub_command_SW_ADC = 0x41;
    const uint8_t sub_command_PS = 0x42;
    const uint8_t sub_command_IMU = 0x44;
    const uint8_t sub_command_MAGENC = 0x48;
    // const uint8_t sub_command_ALL = sub_command_SW_ADC | sub_command_PS | sub_command_IMU | sub_command_MAGENC;
    const uint8_t sub_command_ALL = sub_command_SW_ADC;

    uint8_t tx_cmd[2] = {static_cast<uint8_t>(0xA0 + id), sub_command_ALL};
    uint8_t recv_length = getLen(sub_command_ALL);
    uint8_t rx_buff[recv_length];
    if (!synchronize(tx_cmd, sizeof(tx_cmd), rx_buff, recv_length)) {
      return ICS_FALSE;
    }
    const int index = 2 + 2 * 3;
    float adc_raw = ((0x1F & rx_buff[index]) << 7) | rx_buff[index + 1];
    const float V_OFFSET = 1.65f;
    const float GAIN = 7.4f;
    const float V_REF = 3.3f;
    const float ADC_MAX = 4095.0f;
    const float SCALE_FACTOR = 5.0f / 4.14f;
    const float PRESSURE_OFFSET = 10.739f;
    const float PRESSURE_SCALE = 3.1395f;
    float v_amplified = V_REF * adc_raw / ADC_MAX;
    float v_diff = (v_amplified - V_OFFSET) / GAIN;
    float pressure = (v_diff * 1000.0f * SCALE_FACTOR - PRESSURE_OFFSET) / PRESSURE_SCALE;
    *pressure_out = pressure;
    return 0;
  }
};
