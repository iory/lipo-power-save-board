
/** 
*  @file IcsHardSerialClass.h
* @brief ICS3.5/3.6 arduino library use HardwareSeria header file
* @author Kondo Kagaku Co.,Ltd.
* @date 2017/12/27
* @version 2.0.0
* @copyright Kondo Kagaku Co.,Ltd. 2017

**/

#ifndef _ics_HardSerial_Servo_h_
#define _ics_HardSerial_Servo_h_

#include <Arduino.h>
#include <IcsBaseClass.h>

//IcsHardSerialClassクラス///////////////////////////////////////////////////
/**
* @class IcsHardSerialClass
* @brief 近藤科学のKRSサーボをArduinoのHardwareSerialからアクセスできるようにしたクラス
* @brief IcsBaseClassからの派生して作られている
**/
class IcsHardSerialClass : public IcsBaseClass
{
  //クラス内の型定義
  public:

  //コンストラクタ、デストラクタ
  public:
    //コンストラクタ(construncor)
	
	IcsHardSerialClass();
	
	IcsHardSerialClass(HardwareSerial* icsSerial);
	IcsHardSerialClass(HardwareSerial* icsSerial,long baudrate,int timeout);


      
      //デストラクタ(destruntor)
	~IcsHardSerialClass();
  
  //変数
  public:



  protected: 
	 
	HardwareSerial *icsHardSerial;  ///<arudinoのシリアル型のポインタを格納
	long baudRate;     ///<ICSの通信速度を格納しておく変数
	int timeOut;               ///<通信のタイムアウト(ms)を格納しておく変数



  //関数

  //通信初期化
  public:
      bool begin();
      bool begin(long baudrate,int timeout);
      bool begin(HardwareSerial *serial,long baudrate,int timeout);

  //データ送受信
  public :
      virtual bool synchronize(byte *txBuf, byte txLen, byte *rxBuf, byte rxLen);
   
  //servo関連	//すべていっしょ
  public:


};

#endif
