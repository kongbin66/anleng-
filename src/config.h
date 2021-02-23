#if !defined(CONFIG_H)
#define CONFIG_H
#include "Arduino.h"
#include <esp_sleep.h>
#include "Wire.h"
#include "uFire_SHT20.h"
#define TINY_GSM_MODEM_SIM800 // Modem is SIM800
//引入TinyGSM库. 在引入之前要定义好TINY_GSM_MODEM_SIM800,让它知道我们用的模块型号
#include <TinyGsmClient.h>
#include "PubSubClient.h"
#include "SPIFFS.h"
#include "EEPROM.h"
#include "SH1106Wire.h"
#include "images.h"
#include "OneButton.h"
#include "RTClib.h"

RTC_Millis rtc;
/*-------------------------------SIM800L 硬件定义----------------------------------*/
#define MODEM_RST 5      //SIM800L复位引脚接在GPIO5
//#define MODEM_PWRKEY  //SIM800L开关机引脚接在GPIO32
#define MODEM_POWER_ON 32 //SIM800L电源引脚接在GPIO4
#define MODEM_TX 27       //SIM800L串口TX引脚接在GPIO27
#define MODEM_RX 26       //SIM800L串口RX引脚接在GPIO26

/*-------------------------------其他硬件定义-------------------------------------*/
#define SerialMon Serial      //调试串口为UART0
#define SerialAT  Serial1      //AT串口为UART1
#define KEY1      14            //按键1对应引脚
#define WEAKUPKEY1 GPIO_NUM_14 //按键1对应引脚

/*-------------------------------显示/按键相关定义-------------------------------------*/
OneButton button(KEY1, true);
SH1106Wire display(0x3c, 21, 22);
//state of OLED
#define OLED_ON 1
#define OLED_OFF 0
//define state of workingState
#define NOT_WORKING 0
#define WORKING 1
//state of Screens
#define MAIN_TEMP_SCREEN 0
#define MAIN_HUMI_SCREEN 1
#define TIPS_SCREEN 2
#define LOGO_SCREEN 4
#define BLE_SCREEN 5
#define TEMP_HUMI_SCROLL_SCREEN 6
#define HUMI_TEMP_SCROLL_SCREEN 7
#define SETTING_SUCCESS 8
#define REC_START_SCREEN 9
#define REC_STOP_SCREEN 10
#define REC_COUNT_SCREEN 11
//state of key
#define NOKEYDOWN 0
#define CLICK 1
#define DOUBLECLICK 2
#define LONGPRESS_START 3
#define LONGPRESS_END 4
#define LONGPRESS_DURRING 5
//state of rec_State
#define START_RECING 0
#define END_RECING 1
#define KEEP_RECING 2

TaskHandle_t task1; //第二核创建一个任务句柄

RTC_DATA_ATTR int workingState;        //工作状态机
RTC_DATA_ATTR int keyState;            //按键状态机
RTC_DATA_ATTR int oledState;           //OLED工作状态机
RTC_DATA_ATTR int screenState;         //屏幕状态机
RTC_DATA_ATTR bool screen_loopEnabled; //是否允许滚屏
RTC_DATA_ATTR int current_rec_State;   //当前记录状态机 (正在开始记录,正在持续记录,正在停止记录)
time_t loopStartTime;                  //计算主屏幕滚屏的起始时间
time_t loopnowTime;                    //计算主屏幕滚屏的当前时间
time_t looptimeSpan;                   //计算滚屏间隔时间
time_t keyScreen_Start;                //计算按键触发的当前屏的起始时间
time_t keyScreen_Now;                  //计算当前屏的当前时间
time_t screen_On_Start;                //计算息屏的亮屏起点
time_t screen_On_now;                  //计算息屏的当前时间
time_t screen_On_last_span;            //亮屏时间间隔
time_t screen_Off_to_sleep_span;       //息屏到休眠时间间隔
time_t show_tip_screen_last;           //提示界面自动返回的时间
time_t show_BLE_screen_last;           //蓝牙界面自动返回的时间
time_t show_rec_stop_screen_last;      //停止测量界面自动返回的时间
time_t last_rec_stamp;                 //上次记录时间
time_t now_rec_stamp;                  //计算现在记录时间

/*-------------------------------出厂设置定义-------------------------------------*/
#define FACTORY_SLEEPTIME 60000000//300000000    //休眠时间
#define FACTORY_TEMP_LIMIT_ENABLE 0    //出厂温度上下限失能
#define FACTORY_TEMP_UPPER_LIMIT 50.0  //出厂温度上限
#define FACTORY_TEMP_LOWER_LIMIT -40.0 //出厂温度下限
#define FACTORY_DATE_YEAR 1970         //出厂默认时间
#define FACTORY_DATE_MONTH 1           //出厂默认时间
#define FACTORY_DATE_DAY 1             //出厂默认时间
#define FACTORY_TIME_HOUR 0            //出厂默认时间
#define FACTORY_TIME_MIN 0             //出厂默认时间

#if 0
const char *mqtt_server = "218.201.45.7"; //onenet 的 IP地址
const int port = 1883;                     //端口号
#define mqtt_devid "al_kh00001_zx_0001"         //设备ID
#define mqtt_pubid "4LwKzUwOpX"                //产品ID
//鉴权信息
#define mqtt_password "version=2018-10-31&res=products%2F4LwKzUwOpX%2Fdevices%2Fal_kh00001_zx_0001&et=4092599349&method=md5&sign=xpaXrOTMJ9WJjOVolwJhWw%3D%3D"
#endif

#if 1
const char *mqtt_server = "218.201.45.7"; //onenet 的 IP地址
const int port = 1883;                     //端口号
#define mqtt_devid "al_kh00001_zx_0002"         //设备ID
#define mqtt_pubid "4LwKzUwOpX"                //产品ID
//鉴权信息
#define mqtt_password "version=2018-10-31&res=products%2F4LwKzUwOpX%2Fdevices%2Fal_kh00001_zx_0002&et=4092599349&method=md5&sign=FxSayE%2BpBzK9L1YgXt8rxA%3D%3D"
#endif

#if 0
const char *mqtt_server = "218.201.45.7"; //onenet 的 IP地址
const int port = 1883;                     //端口号
#define mqtt_devid "al_kh00001_zx_0003"         //设备ID
#define mqtt_pubid "4LwKzUwOpX"                //产品ID
//鉴权信息
#define mqtt_password "version=2018-10-31&res=products%2F4LwKzUwOpX%2Fdevices%2Fal_kh00001_zx_0003&et=4092599349&method=md5&sign=RJjI9dBTNLUXL9rk9zbBtQ%3D%3D"
#endif

/*-------------------------------公共变量,参数定义-------------------------------------*/
//温湿度采集相关
float currentTemp;
float currentHumi;
bool tempAndHumi_Ready;
bool timeNTPdone;
//判断是否第一次启动
bool firstBootFlag; //第一次启动标志位

#define BLE_ON 1
#define BLE_OFF 2
#define LOCKED 1
#define UNLOCKED 2
#define QUALITIFY_RIGHT 1
#define QUALITIFY_WRONG 2

RTC_DATA_ATTR int bleState;       //蓝牙状态机
RTC_DATA_ATTR int lockState;      //开关箱状态机
RTC_DATA_ATTR int qualifiedState; //合格状态机
/*-------------------------------公共变量,参数定义-------------------------------------*/
//以下参数需要休眠RTC记忆
RTC_DATA_ATTR bool tempLimit_enable;                 //温度上下限报警开关
RTC_DATA_ATTR float tempUpperLimit;                  //温度上限设定
RTC_DATA_ATTR float tempLowerLimit;                  //温度下限设定
RTC_DATA_ATTR time_t sleeptime;                      //休眠时间
RTC_DATA_ATTR time_t reduce_sleeptime;               //缩减休眠时间
RTC_DATA_ATTR time_t sleep_start_time;               //休眠开始时间
RTC_DATA_ATTR time_t sleep_end_time;                 //休眠结束时间
RTC_DATA_ATTR time_t sleep_time_count;               //休眠时长时间
RTC_DATA_ATTR int postMsgId = 0;                     //记录已经post了多少条
RTC_DATA_ATTR float locationE, locationN, locationA; //地理位置,经度纬度
RTC_DATA_ATTR int timeNow_Y, timeNow_M, timeNow_D, timeNow_h, timeNow_m, timeNow_s;
RTC_DATA_ATTR int timeLastNTP_Y, timeLastNTP_M, timeLastNTP_D, timeLastNTP_h, timeLastNTP_m, timeLastNTP_s;
/*-------------------------------SPIFFS定义-------------------------------------*/
RTC_DATA_ATTR bool alFFS_thisRec_firstData_flag; //本次记录第一次上传
RTC_DATA_ATTR char nowREC_filepath[21];          //记录文件的路径
/*-------------------------------系统时间定义-------------------------------------*/
RTC_DATA_ATTR uint32_t now_unixtime;
// RTC_DATA_ATTR int64_t now_unixtime64;
time_t time_last_async_stamp;

/*-------------------------------初始化相关init.ino-------------------------------------*/
void hardware_init();
void software_init();

/*-------------------------------温湿度采集相关al_sht20.ino-----------------------------*/
uFire_SHT20 sht20;
void sht20getTempAndHumi();

/*-------------------------------SIM800相关network.ino---------------------*/
// 创建一个关联到SerialAT的SIM800L模型
TinyGsm modem(SerialAT);
// 创建一个GSM型的网络客户端
TinyGsmClient gsmclient(modem);
PubSubClient client(gsmclient);

void setupModem();
void modemToGPRS();
void getLBSLocation();
// /*-------------------------------ali_mqtt服务相关ali_mqtt.ino---------------------*/
// void ali_mqtt_connect();
// void ali_callback(char *topic, byte *payload, unsigned int length);
// void ali_sendTemp_Humi_LBS();
/*-------------------------------onenet_mqtts服务相关onenet_mqtts.ino---------------------*/

/*-------------------------------云平台相关定义-------------------------------------*/
//设备上传数据的post主题
#define ONENET_TOPIC_PROP_POST "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/post"
//接收下发属性设置主题
#define ONENET_TOPIC_PROP_SET "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/set"
//接收下发属性设置成功的回复主题
#define ONENET_TOPIC_PROP_SET_REPLY "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/set_reply"

//接收设备属性获取命令主题
#define ONENET_TOPIC_PROP_GET "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/get"
//接收设备属性获取命令成功的回复主题
#define ONENET_TOPIC_PROP_GET_REPLY "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/get_reply"

//这是post上传数据使用的模板
#define ONENET_POST_BODY_FORMAT "{\"id\":\"1\",\"params\":%s}"



// char msgJson[256]; //要发送的json格式的数据
// char dataTemplate[] = "{\"id\":\"123\",\"params\":{\"temp\":{\"value\":%.2f},\"humi\":{\"value\":%.2f},\"le\":{\"value\":%.2f},\"ln\":{\"value\":%.2f}}}";
void onenet_connect();
void sendTempAndHumi();
// void onenet_mqtts_connect();
// void onenet_mqtts_sendTemp_Humi_LBS();
/*-------------------------------休眠服务相关al_sleep.ino---------------------*/
void go_sleep_a_while_with_ext0();

/*--------------------------------eeprom相关函数--------------------*/
void get_eeprom_firstBootFlag();
void eeprom_config_init();
void eeprom_config_set_sleeptime(time_t time1);
/*********************************SPIFFS相关函数 al_FFS.ino**********/
void alFFS_init();
void alFFS_addRec();
void alFFS_readRecing();
void alFFS_endRec();
/*********************************显示屏相关函数 al_oled.ino**********/
void showWelcome();
void screen_loop();
void screen_show();
void send_Msg_var_GSM_while_OLED_on();
void oled_on_off_switch();

/*********************************按键相关函数 al_key1.ino***********/

void key_init();
void key_loop();
void key_attach_null();
void oledoff_upload_but_click();
/*********************************对时相关函数 al_time.ino***********/

void sleep_update_time();
void wakeup_init_time();
void waking_update_time();

int bat_mv;//电池电压
#define ADC_BAT 4 // TCALL 35
#define IP5306_ADDR 0x75
#define IP5306_REG_SYS_CTL0 0x00
int8_t getBatteryLevel();//获取电量等级
bool setPowerBoostKeepOn(bool en);
void PowerManagment(uint32_t time_delay);


extern const char *p1;//电量图标显示
extern uint8_t BatteryLevel;//电量等级
void fun_getBatteryLevel();//获取电量等级更改显示图标
#endif // CONFIG_H