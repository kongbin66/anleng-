#include "config.h"
/*-------------------------------APN相关定义-------------------------------------*/
const char apn[] = "CMNET"; // Your APN
const char gprsUser[] = ""; // User
const char gprsPass[] = ""; // Password
const char simPIN[] = "";   // SIM card PIN code, if any

/*-------------------------------初始化SIM800L-------------------------------------*/
void setupModem()
{
  
 // pinMode(MODEM_PWRKEY, OUTPUT);   //开关机键引脚
  // 先打开SIM800L的电源
  digitalWrite(MODEM_POWER_ON, HIGH);
  //根据手册要求拉下PWRKEY 1秒钟以上 可以开机
 // digitalWrite(MODEM_PWRKEY, HIGH);
  delay(100);
//  digitalWrite(MODEM_PWRKEY, LOW);
  delay(1000);
 // digitalWrite(MODEM_PWRKEY, HIGH);
  SerialMon.println("Initializing modem...");
  modem.init(); //开机后modem初始化一下
}
/*-------------------------------SIM800L连接GPRS-------------------------------------*/
void modemToGPRS()
{
  //连接网络
  SerialMon.print("Waiting for network...");
  while (!modem.waitForNetwork(240000L))
  {
    SerialMon.print(".");
    delay(500);
  }
  SerialMon.println(" OK");
  //连接GPRS接入点
  SerialMon.print(F("Connecting to APN: "));
  SerialMon.print(apn);
  while (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    SerialMon.print(".");
    delay(10000);
  }
  SerialMon.println(" OK");
  SerialMon.println(" signalQuality:");
  Serial.println(modem.getSignalQuality());
}
/*-------------------------------获取位置信息-------------------------------------*/
void getLBSLocation()
{
  int i;
  Serial.println("getting LBS...");
  float _locationE = 0, _locationN = 0, _locationA = 0; //地理位置,经度纬度
  int _timeLastNTP_Y = 0, _timeLastNTP_M = 0, _timeLastNTP_D = 0, _timeLastNTP_h = 0, _timeLastNTP_m = 0, _timeLastNTP_s = 0;
  modem.getGsmLocation(&_locationE, &_locationN, &_locationA, &_timeLastNTP_Y, &_timeLastNTP_M, &_timeLastNTP_D, &_timeLastNTP_h, &_timeLastNTP_m, &_timeLastNTP_s);
  if (_locationE > 0.1)
  {
    locationE = _locationE;
    locationN = _locationN;
    locationA = _locationA;
    timeLastNTP_Y = _timeLastNTP_Y;
    timeLastNTP_M = _timeLastNTP_M;
    timeLastNTP_D = _timeLastNTP_D;
    timeLastNTP_h = _timeLastNTP_h;
    timeLastNTP_m = _timeLastNTP_m;
    timeLastNTP_s = _timeLastNTP_s;
    timeNow_Y = timeLastNTP_Y;
    timeNow_M = timeLastNTP_M;
    timeNow_D = timeLastNTP_D;
    timeNow_h = timeLastNTP_h;
    timeNow_m = timeLastNTP_m;
    timeNow_s = timeLastNTP_s;
  }
  rtc.adjust(DateTime(timeNow_Y, timeNow_M, timeNow_D, timeNow_h, timeNow_m, timeNow_s));
  DateTime now = rtc.now();
  now_unixtime = now.unixtime();
  time_last_async_stamp = millis();
 
  //对时
   if(timeNow_Y!=0&&timeNow_M!=0&&timeNow_D!=0)//数据读出错误
    now1.year= timeNow_Y;
    now1.month=timeNow_M;
    now1.day = timeNow_D;
    now1.hour =timeNow_h;
    now1.minute = timeNow_m;
    now1.second = timeNow_s;
    rtc1.setDateTime(&now1);

  EEPROM.writeULong(39, now_unixtime);
  EEPROM.commit();
}
