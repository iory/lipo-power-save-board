/**
*	@file IcsBaseClass.cpp
*	@brief ICS3.5/3.6 base library
*	@author Kondo Kagaku Co.,Ltd.
*	@date	2020/02/20
*	@version 2.1.0
*	@copyright &copy; Kondo Kagaku Co.,Ltd. 2017
**/

#include "IcsBaseClass.h"

//サーボID範囲 /////////////////////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボIDが範囲内か見る
* @param[in] id
* @return 送られてきたid
* @retval 0xFF 範囲外
**/
byte IcsBaseClass::idMax(byte id)
{
  if ((char)id < MIN_ID) {
    id = 0xFF;
  }
  if (id > MAX_ID) {
    id = 0xFF;
  }
  return id;
}



//サーボ可動範囲　パラメータ範囲　リミット設定 //////////////////////////////////////////////////////////////
/**
* @brief ポジションデータの最大、最小値の範囲に収める
* @param[in] maxPos 最大値
* @param[in] minPos 最小値
* @param[in,out] val 現在値
* @return リミットがかかったポジションデータ
**/
bool IcsBaseClass::maxMin(int maxPos, int minPos, int val)
{
  if (val > maxPos) {
    return false;
  }
  if (val < minPos) {
    return false;
  }
  return true;
}



//角度変換　角度からPOSへ////////////////////////////////////////////////////////////////////////////////////
/**
* @brief 角度データ(float型)をポジションデータに変換
* @param[in] deg 角度(deg)(float型)
* @return ポジションデータ
* @retval -1 範囲外
**/
int IcsBaseClass::degPos(float deg)
{
  if (deg > MAX_DEG) {
    return -1;
  }
  if (deg < MIN_DEG) {
    return -1;
  }
  int pos = deg * 29.633;
  pos = pos + 7500;
  return pos;
}


//角度変換　POSから角度へ////////////////////////////////////////////////////////////////////////////////////
/**
* @brief ポジションデータを角度データ(float型)に変換
* @param[in] pos ポジションデータ
* @return 角度(deg)(float型)
* @retval #ANGLE_F_FALSE  正方向範囲外
* @retval -#ANGLE_F_FALSE (0x8000) 負方向範囲外
**/
float IcsBaseClass::posDeg(int pos)
{
  pos = pos - 7500;
  float deg = pos  / 29.633;

  if (deg > MAX_DEG) {
    return ANGLE_F_FALSE;
  }
  if (deg < MIN_DEG) {
    return -ANGLE_F_FALSE;
  }

  return deg;
}


//角度変換　x100 角度からPOSへ///////////////////////////////////////////////////////////////////////////////
/**
* @brief 角度データx100(int型)をポジションデータに変換
* @param[in] deg 角度(deg x100)(int型)
* @return 変換されたポジションデータ
* @retval -1 範囲外
**/
int IcsBaseClass::degPos100(int deg)
{
  if (deg > MAX_100DEG) {
    return -1 ;
  }
  if (deg < MIN_100DEG) {
    return -1;
  }
  long a  = ((long)deg * 2963) / 10000;
  int pos = a + 7500;
  return pos;
}


//角度変換　x100 POSから角度へ///////////////////////////////////////////////////////////////////////////////
/**
* @brief ポジションデータを角度データ(int型)に変換
* @param[in] pos ポジションデータ
* @return 角度(deg x100)(int型)
* @retval #ANGLE_I_FALSE 正方向範囲外
* @retval -#ANGLE_I_FALSE 負方向範囲外
**/
int IcsBaseClass::posDeg100(int pos)
{
  long a = pos - 7500;
  int deg = (a * 1000) / 296;

  if (deg > MAX_100DEG) {
    return ANGLE_I_FALSE;
  }
  if (deg < MIN_100DEG) {
    return -ANGLE_I_FALSE;
  }
  return deg;
}



//サーボ角度セット //////////////////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータの角度を変更します
* @param[in] id サーボモータのID番号
* @param[in] pos ポジションデータ
* @return ポジションデータ
* @retval -1 範囲外、通信失敗
**/
int IcsBaseClass::setPos(byte id, unsigned int pos)
{
  byte txCmd[3];
  byte rxCmd[3];
  unsigned int rePos;
  bool flg;

  
  if ((id != idMax(id)) || ( ! maxMin(MAX_POS, MIN_POS, pos)) ) //範囲外の時
  {
    return ICS_FALSE;
  }

  txCmd[0] = 0x80 + id;               // CMD
  txCmd[1] = ((pos >> 7) & 0x007F);   // POS_H
  txCmd[2] = (pos & 0x007F);          // POS_L

  //送受信
  	
  
  
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

		
  
  rePos = ((rxCmd[1] << 7) & 0x3F80) + (rxCmd[2] & 0x007F);

  return rePos;

}



//サーボ フリーモード ///////////////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータをフリー(脱力)状態にします
* @param[in] id サーボモータのID番号
* @return ポジションデータ
* @retval -1 範囲外、通信失敗
**/
int IcsBaseClass::setFree(byte id)
{
  byte txCmd[3];
  byte rxCmd[3];
  unsigned int rePos;
  bool flg;

  if (id != idMax(id)) //範囲外の時
  {
    return ICS_FALSE;
  }

  txCmd[0] = 0x80 + id;    // CMD
  txCmd[1] = 0;
  txCmd[2] = 0;

  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  rePos = ((rxCmd[1] << 7) & 0x3F80) + (rxCmd[2] & 0x007F);

  return rePos;

}




//ストレッチ値書込み　1～127 ////////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータのストレッチ(保持力)の値を書き込みます
* @param[in] id サーボモータのID番号
* @param[in] strc ストレッチのデータ(1～127)
* @return 書き込んだストレッチのデータ
* @retval -1 通信失敗
* @note ストレッチ書込 1～127  1(弱）      127(強）
**/
int IcsBaseClass::setStrc(byte id, unsigned int strc)
{
  byte txCmd[3];
  byte rxCmd[3];
  unsigned int reData;
  bool flg;

  if ((id != idMax(id)) || ( ! maxMin(MAX_127, MIN_1, strc)) ) //範囲外の時
  {
    return ICS_FALSE;
  }

  txCmd[0] = 0xC0 + id;    // CMD
  txCmd[1] = 0x01;         // SC ストレッチ
  txCmd[2] = strc;         // ストレッチ


  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }
  reData = rxCmd[2];

  return reData;

}



//スピード値書込み　1～127 //////////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータのスピード(出力)を変更します
* @param[in] id サーボモータのID番号
* @param[in] spd スピードのデータ(1～127)
* @return 書き込んだスピードのデータ
* @retval -1 通信失敗
* @note スピード書込   1～127  1(遅い)     127(速い)
**/
int IcsBaseClass::setSpd(byte id, unsigned int spd)
{
  byte txCmd[3];
  byte rxCmd[3];
  unsigned int reData;
  bool flg;

  if ((id != idMax(id)) || ( ! maxMin(MAX_127, MIN_1, spd)) ) //範囲外の時
  {
    return ICS_FALSE;
  }


  txCmd[0] = 0xC0 + id;      // CMD
  txCmd[1] = 0x02;           // SC スピード
  txCmd[2] = spd;            // スピード

  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  reData = rxCmd[2];

  return reData;

}




//電流リミット値書込み　1～63 ///////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータの電流リミット（電流制限）値を変更します
* @param[in] id サーボモータのID番号
* @param[in] curlim 電流値のデータ
* @return サーボモータの電流リミット値
* @retval -1 通信失敗
* @note 電流制限値書込 1～63   1(低い)     63 (高い)
**/
int IcsBaseClass::setCur(byte id, unsigned int curlim)
{
  byte txCmd[3];
  byte rxCmd[3];
  unsigned int reData;
  bool flg;

  if ((id != idMax(id)) || ( ! maxMin(MAX_63, MIN_1, curlim)) ) //範囲外の時
  {
    return ICS_FALSE;
  }

  txCmd[0] = 0xC0 + id;                     // CMD
  txCmd[1] = 0x03;                          // SC 電流値
  txCmd[2] = curlim;                        // 電流リミット値


  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  reData = rxCmd[2];

  return reData;

}




//温度リミット値書込み 　1～127 /////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータの温度リミット（温度制限）値を変更します
* @param[in] id サーボモータのID番号
* @param[in] tmplim 温度リミット値のデータ
* @return サーボモータの温度リミット値
* @retval -1 通信失敗
* @note 温度上限書込   1～127  127(低温）  1(高温)
**/
int IcsBaseClass::setTmp(byte id, unsigned int tmplim)
{
  byte txCmd[3];
  byte rxCmd[3];
  unsigned int reData;
  bool flg;
//  if (id != idMax(id)) //範囲外の時
//  {
//    return ICS_FALSE;
//  }
//  tmplim = maxMin(MAX_127, MIN_1, tmplim);   //入力値範囲

  if ((id != idMax(id)) || ( ! maxMin(MAX_127, MIN_1, tmplim)) ) //範囲外の時
  {
    return ICS_FALSE;
  }


  txCmd[0] = 0xC0 + id;                      // CMD
  txCmd[1] = 0x04;                           // SC 温度値
  txCmd[2] = tmplim;                         // 温度リミット値

  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  reData = rxCmd[2];

  return reData;

}


//ストレッチ値読出し　1～127 ////////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータのストレッチ(保持力)の値を読み込みます
* @param[in] id サーボモータのID番号
* @return 読み込んだストレッチのデータ
* @retval -1 通信失敗
* @note ストレッチ読込    1～127  1(弱）      127(強）
**/
int IcsBaseClass::getStrc(byte id)
{
  byte txCmd[2];
  byte rxCmd[3];
  unsigned int reData;
  bool flg;
  //id = idMax(id);          //ID範囲
  if (id != idMax(id)) //範囲外の時
  {
    return ICS_FALSE;
  }
  txCmd[0] = 0xA0 + id;    // CMD
  txCmd[1] = 0x01;         // SC ストレッチ


  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  reData = rxCmd[2];

  return reData;

}




//スピード値読出し　1～127 //////////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータのスピード(出力)を読み込みます
* @param[in] id サーボモータのID番号
* @return 読み込んだスピードのデータ
* @retval -1 通信失敗
* @note スピード読込      1～127  1(遅い)     127(速い)
**/
int IcsBaseClass::getSpd(byte id)
{
  byte txCmd[2];
  byte rxCmd[3];
  unsigned int reData;
  bool flg;
  if (id != idMax(id)) //範囲外の時
  {
    return ICS_FALSE;
  }
  txCmd[0] = 0xA0 + id;    // CMD
  txCmd[1] = 0x02;         // SC スピード


  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  reData = rxCmd[2];

  return reData;

}



//電流値読出し　正転時0～63、逆転時64～127 //////////////////////////////////////////////////////////
/**
* @brief サーボモータの現在の電流値を読み込みます
* @param[in] id サーボモータのID番号
* @return サーボモータの現在の電流値
* @retval -1 通信失敗
* @note 電流値読込      正転時0～63、逆転時64～127
**/
int IcsBaseClass::getCur(byte id)
{
  byte txCmd[2];
  byte rxCmd[3];
  unsigned int reData;
  bool flg;
  if (id != idMax(id)) //範囲外の時
  {
    return ICS_FALSE;
  }
  txCmd[0] = 0xA0 + id;    // CMD
  txCmd[1] = 0x03;         // SC 電流値


  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  reData = rxCmd[2];

  return reData;

}




//温度値読出し　1～127 ///////////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータの現在の温度値を読み込みます
* @param[in] id サーボモータのID番号
* @return サーボモータの現在の温度値
* @retval -1 通信失敗
* @note 現在温度読込	0(高温) ～ 127(低温
**/
int IcsBaseClass::getTmp(byte id)
{
  byte txCmd[2];
  byte rxCmd[3];
  unsigned int reData;
  bool flg;
  if (id != idMax(id)) //範囲外の時
  {
    return ICS_FALSE;
  }
  txCmd[0] = 0xA0 + id;    // CMD
  txCmd[1] = 0x04;         // SC 温度値


  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  reData = rxCmd[2];

  return reData;

}


//現在値読出し　※ICS3.6以降で有効 //////////////////////////////////////////////////////////////////////////
/**
* @brief サーボモータの現在のポジションデータを読み込みます
* @param[in] id サーボモータのID番号
* @return 指定したIDの現在のポジションデータ
* @retval -1 通信失敗
* @attention ICS3.6から有効です。ICS3.5のサーボモータに送った場合は返事を返しません
**/
int IcsBaseClass::getPos(byte id)
{
  byte txCmd[2];
  byte rxCmd[4];
  unsigned int reData;
  bool flg;
  if (id != idMax(id)) //範囲外の時
  {
    return ICS_FALSE;
  }
  txCmd[0] = 0xA0 + id;    // CMD
  txCmd[1] = 0x05;         // 角度読出し


  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  reData = ((rxCmd[2] << 7) & 0x3F80) + (rxCmd[3] & 0x007F);

  return reData;

}


uint16_t IcsBaseClass::getLen(uint8_t sub_command_raw) {
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


int IcsBaseClass::setGPIO(byte id, bool gpio0, bool gpio1) {
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

std::vector<int16_t> IcsBaseClass::getIMUFromPacket(std::vector<byte> rx_buff) {
  const int ADC_CHANNEL_NUM = 5;
  const int GYRO_CHANNEL_NUM = 3;
  const int ACC_CHANNEL_NUM = 3;
  int index = 2 + 2 * ADC_CHANNEL_NUM;

  std::vector<int16_t> buff(GYRO_CHANNEL_NUM + ACC_CHANNEL_NUM);
  for(int i=0; i < GYRO_CHANNEL_NUM; i++) {
    buff[i] = ((0x03 & rx_buff[index]) << 14) | ((0x7f & rx_buff[index + 1]) << 7) | (0x7f & rx_buff[index + 2]);;
    index += 3;
  }
  for(int i=0; i < ACC_CHANNEL_NUM; i++) {
    buff[GYRO_CHANNEL_NUM + i] = ((0x03 & rx_buff[index]) << 14) | ((0x7f & rx_buff[index + 1]) << 7) | (0x7f & rx_buff[index + 2]);
    index += 3;
  }
  return buff;
}


std::vector<byte> IcsBaseClass::getSubcommandPacket(byte id) {
  const uint8_t sub_command_NONE = 0x40;
  const uint8_t sub_command_SW_ADC = 0x41;
  const uint8_t sub_command_PS = 0x42;
  const uint8_t sub_command_IMU = 0x44;
  const uint8_t sub_command_MAGENC = 0x48;
  // const uint8_t sub_command_ALL = sub_command_SW_ADC | sub_command_PS | sub_command_IMU | sub_command_MAGENC;
  const uint8_t sub_command_ALL = sub_command_SW_ADC;

  byte txCmd[2];
  uint8_t recv_length = getLen(sub_command_ALL) + 2;  // 2byte余計にとる
  std::vector<byte> rx_buff(recv_length); // std::vectorで動的配列を確保
  uint8_t SC_send = sub_command_ALL;
  bool flg;

  txCmd[0] = 0xA0 + id;
  txCmd[1] = SC_send;
  flg = synchronize(txCmd, sizeof(txCmd), rx_buff.data(), recv_length);

  // if (flg) {
  //   for (int i = 0; i < recv_length; ++i) {
  //     USBSerial.print(rx_buff[i]);
  //     USBSerial.print(' ');
  //   }
  //   USBSerial.println("");
  // }

  if (!flg) {
    rx_buff.clear();
  }
  return rx_buff;
}


float IcsBaseClass::getPressureFromPacket(std::vector<byte> rx_buff) {
  const int index = 2 + 2 * 3;
  float adc_raw = ((0x1f & rx_buff[index]) << 7) | rx_buff[index + 1];
  const float V_OFFSET = 1.65;
  const float GAIN = 7.4;
  const float v_amplified = 3.3 * adc_raw / 4095;
  const float v_diff = (v_amplified - V_OFFSET) / GAIN;
  const float pressure = (v_diff * 1000 * 5.0 / 4.14 - 10.739) / 3.1395;
  return pressure;
}


float IcsBaseClass::getPressure(byte id) {
  const int sub_command_NONE = 0x40;
  const int sub_command_SW_ADC = 0x41;
  const int sub_command_PS = 0x42;
  const int sub_command_IMU = 0x44;
  const int sub_command_MAGENC = 0x48;
  // const int sub_command_ALL = (sub_command_SW_ADC | sub_command_PS | sub_command_IMU | sub_command_MAGENC);
  const int sub_command_ALL = sub_command_SW_ADC;
  const int ADC_CHANNEL_NUM = 5;
  byte txCmd[2];
  uint8_t recv_length = getLen(sub_command_ALL);
  uint16_t adc_buff[ADC_CHANNEL_NUM] = {};
  uint8_t SC_send = sub_command_ALL; // SC_SW_ADC, SC_PS, SC_IMU, SC_MAGENC;
  byte rx_buff[recv_length];
  unsigned int reData;
  bool flg;
  // if (id != idMax(id)) {
  //   return ICS_FALSE;
  // }
  txCmd[0] = 0xA0 + id;    // CMD
  txCmd[1] = SC_send;

  flg = synchronize(txCmd, sizeof txCmd, rx_buff, sizeof rx_buff);

  // char log_msg[50];
  // sprintf(log_msg, "len: %d", recv_length);
  // M5.Lcd.println(log_msg);

  // for (int i = 0; i < recv_length; ++i) {
  //   M5.Lcd.print(rx_buff[i]);
  //   M5.Lcd.print(' ');
  // }
  // M5.Lcd.println(" ");

  if (flg == false) {
    return ICS_FALSE;
  }
  int index = 2;

  // M5.Lcd.println("Adc buff");
  for(int i=0; i < ADC_CHANNEL_NUM; i++){
    adc_buff[i] = ((0x1f & rx_buff[index]) << 7) | rx_buff[index + 1];
    // M5.Lcd.print(adc_buff[i]);
    // M5.Lcd.print(' ');
    if(i == ADC_CHANNEL_NUM - 1){ //temp sensor
      // Sensor_vector[j].temperature = adc_buff[i];
      continue;
    }else{
      // Sensor_vector[j].adc[i] = adc_buff[i];
    }
    index += 2;
  }
  // M5.Lcd.println(" ");

  return adc_buff[3];
  // const float V_OFFSET = 1.65;
  // const float GAIN = 7.4;
  // uint16_t adc_raw = adc_buff[3];

  // const float v_amplified = 3.3 * adc_raw / 4095;
  // const float v_diff = (v_amplified - V_OFFSET) / GAIN;
  // const float pressure = (v_diff * 1000 * 5.0 / 4.14 - 10.739) / 3.1395;
  // return pressure;
}


//IDの読み込み //////////////////////////////////////////////////////////////////////////////////////////
/**
* @brief ICSデバイスのIDを取得する
* @return 返ってきたIDデータ
* @retval -1 範囲外、通信失敗
* @date 2020/02/20 Ver2.1.0から追加
* @attention IDコマンドは１対１でのみ接続してください。多数つないだ場合は意図しないデータが返って来ます。
**/
int IcsBaseClass::getID()
{
  byte txCmd[4];
  byte rxCmd[1];
  int id;
  bool flg;


  txCmd[0] = 0xFF;               // CMD
  txCmd[1] = 0;          // ID読み込み
  txCmd[2] = 0;          // ID読み込み
  txCmd[3] = 0;          // ID読み込み

  //送受信
  
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

		
  delay(520);	//コマンドが応答するまで500msは最低かかる
  
  id = 0x1F & rxCmd[0]; //データはマスクをかけておくとIDになる

  return id;

}

//IDの書き込み //////////////////////////////////////////////////////////////////////////////////////////
/**
* @brief ICSデバイスのIDを書き込む
* @param[in] id サーボモータのID番号
* @return 返ってきたIDデータ
* @retval -1 範囲外、通信失敗
* @date 2020/02/20 Ver2.1.0から追加
* @attention IDコマンドは１対１でのみ接続してください。多数つないだ場合はすべてのIDが書き変わります。
**/
int IcsBaseClass::setID(byte id)
{
  byte txCmd[4];
  byte rxCmd[1];
  int reID;
  bool flg;


  txCmd[0] = 0xE0 + id;               // CMD
  txCmd[1] = 1;          // ID書き込み
  txCmd[2] = 1;          // ID書き込み
  txCmd[3] = 1;          // ID書き込み

  //送受信
  
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

		
  delay(520);	//コマンドが応答するまで500msは最低かかる
  
  reID = 0x1F & rxCmd[0]; //データはマスクをかけておくとIDになる

  return id;

}


//KRR5全ボタン読出し ////////////////////////////////////////////////////////////////////////////////////////
/**
* @brief KRRが受信しているボタンのデータを取得します
* @return #KRR_BUTTON を参照
* @retval -1 通信失敗
**/
unsigned short IcsBaseClass::getKrrButton()
{
  byte txCmd[4];
  byte rxCmd[8];
  unsigned short btn;
  bool flg;

  txCmd[0] = 0xBF;   // CMD
  txCmd[1] = 0x7F;   // SC 受信データ読込
  txCmd[2] = 0x00;   //ADDR
  txCmd[3] = 0x02;   //BYTE


  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return KRR_BUTTON_FALSE;
  }

  //ボタンデータ
  btn = ((rxCmd[4] << 4) & 0xF0) + (rxCmd[5] & 0x0F);
  btn = (btn << 8 | (((rxCmd[6] << 4) & 0xF0) + (rxCmd[7] & 0x0F)) );

  return btn;

}


//KRR5 PAアナログ読出し /////////////////////////////////////////////////////////////////////////////////////
/**
* @brief KRRが受信している指定したアナログポート(PA)のデータを取得します
* @param[in] paCh アナログポートの番号
* @return 指定したアナログポートのデータ
* @retval -1 通信失敗
* @attention KRC5のアナログポートは1～4になります
**/
int IcsBaseClass::getKrrAnalog(int paCh)
{
  byte txCmd[4];
  byte rxCmd[6];
  int adData;
  bool flg;

  if (1 > paCh || paCh > 4) //ADがない場合は-1を返す
  {
    return false;
  }

  txCmd[0] = 0xBF;   // CMD
  txCmd[1] = 0x7F;   // SC 受信データ読込
  txCmd[2] = (1 + paCh);   //ADDR
  txCmd[3] = 0x01;   //BYTE


  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return ICS_FALSE;
  }

  adData =  ((rxCmd[4] << 4) & 0xF0) + (rxCmd[5] & 0x0F);

  return adData;
}


//KRR5 全読出し /////////////////////////////////////////////////////////////////////////////////////////////
/**
* @brief KRRが受信しているボタンデータおよびアナログポート(PA)のデータをすべて取得します
* @param[out] button ボタンデータ(#KRR_BUTTON を参照)
* @param[out] adData 4ch分のアナログデータ
* @retval ture 通信成功
* @retval false 通信失敗
* @attention KRC5のアナログポートは1～4になります
* @attention 配列サイズは4固定です
**/
bool IcsBaseClass::getKrrAllData(unsigned short *button,int adData[4])
{
  byte txCmd[4];
  byte rxCmd[18];
  unsigned short btn;
  bool flg;

  txCmd[0] = 0xBF;   // CMD
  txCmd[1] = 0x7F;   // SC 受信データ読込
  txCmd[2] = 0x00;   //ADDR
  txCmd[3] = 0x07;   //BYTE
  ;
  //送受信
  flg = synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
  if (flg == false)
  {
    return false;
  }

  //ボタンデータ
  btn = ((rxCmd[4] << 4) & 0xF0) + (rxCmd[5] & 0x0F);
  btn = (btn << 8 | (((rxCmd[6] << 4) & 0xF0) + (rxCmd[7] & 0x0F)) );
  *button = btn;


  //アナログデータ
  for (int i = 0; i < 4; i++)
  {
    adData[i] = ((rxCmd[8 + i * 2] << 4) & 0xF0) + (rxCmd[9 + i * 2] & 0x0F);
  }
  return true;
}

