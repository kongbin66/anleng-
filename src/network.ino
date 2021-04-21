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
  SerialMon.print("Initializing modem...");
  modem.init(); //开机后modem初始化一下
  SerialMon.println("OK!");
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
  SerialMon.println(" OK!");
  //连接GPRS接入点
  SerialMon.print(F("Connecting to APN: "));
  SerialMon.print(apn);
  while (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    SerialMon.print(".");
    delay(10000);
  }
  SerialMon.println(" OK!");
  SerialMon.print("signalQuality:");
  Serial.println(modem.getSignalQuality());
}
/*-------------------------------获取位置信息-------------------------------------*/
//获取时间和位置，更新ds1302
bool getLBSLocation()
{

  Serial.print("getting LBS...");
  float _locationE = 0, _locationN = 0, _locationA = 0; //地理位置,经度纬度
  int _timeLastNTP_Y = 0, _timeLastNTP_M = 0, _timeLastNTP_D = 0, _timeLastNTP_h = 0, _timeLastNTP_m = 0, _timeLastNTP_s = 0;
  

  while(_locationE==0&&_timeLastNTP_Y==0) //获取位置和时间
  {
    modem.getGsmLocation(&_locationE, &_locationN, &_locationA, &_timeLastNTP_Y, &_timeLastNTP_M, &_timeLastNTP_D, &_timeLastNTP_h, &_timeLastNTP_m, &_timeLastNTP_s);
    delay(1000);
  }
  Serial.println("OK!\n");
    locationE = _locationE;
    locationN = _locationN;
    locationA = _locationA;
    timeLastNTP_Y = _timeLastNTP_Y;
    timeLastNTP_M = _timeLastNTP_M;
    timeLastNTP_D = _timeLastNTP_D;
    timeLastNTP_h = _timeLastNTP_h;
    timeLastNTP_m = _timeLastNTP_m;
    timeLastNTP_s = _timeLastNTP_s;
    Serial.printf("time GSM: %d-%d-%d %d:%d:%d\r\n", timeLastNTP_Y, timeLastNTP_M,timeLastNTP_D,timeLastNTP_h,timeLastNTP_m,timeLastNTP_s);
    //对时 
    now1.year = (_timeLastNTP_Y - 2000);
    now1.month = _timeLastNTP_M;
    now1.day = _timeLastNTP_D;
    now1.hour =_timeLastNTP_h;
    now1.minute =_timeLastNTP_m;
    now1.second =_timeLastNTP_s;
    ds_rtc.setDateTime(&now1);
    Serial.printf("ds1302 time now: %d-%d-%d %d:%d:%d\r\n", (now1.year+2000), now1.month, now1.day,now1.hour, now1.minute, now1.second);
    Serial.println("ds1302 duishi ok!\n");
     now_unixtime = unixtime();
    time_last_async_stamp = unixtime();
    return 0;
    // timeNow_Y = timeLastNTP_Y;
    // timeNow_M = timeLastNTP_M;
    // timeNow_D = timeLastNTP_D;
    // timeNow_h = timeLastNTP_h;
    // timeNow_m = timeLastNTP_m;
    // timeNow_s = timeLastNTP_s;
}
