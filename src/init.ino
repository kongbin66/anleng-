/*----------------------------------------------------------
                 軟、硬件初始化操作相关
*---------------------------------------------------------*/
#include "config.h"

void hardware_init()//硬件初始化
{
  gpio_hold_dis(GPIO_NUM_32);//解锁电源引脚
  gpio_deep_sleep_hold_dis();
  pinMode     (MODEM_POWER_ON, OUTPUT); //电源引脚
  digitalWrite(MODEM_POWER_ON,LOW);
  Wire.begin();
  SerialMon.begin(115200); //初始化调试串口
  Serial.println("wakeup");
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX); //初始化AT串口
  sht20.begin();//溫濕度傳感器初始化
  EEPROM.begin(4096);//E2PROM初始化
  SPIFFS.begin();
  display.init();
  display.flipScreenVertically();
  key_init();
  adcAttachPin(BATTERY_ADC_PIN); //将引脚连接到ADC
  //adcStart(BATTERY_ADC_PIN);     //在连接的引脚总线上开始ADC转换
  PowerManagment();//保持升压芯片持续工作
}
/*----------------------------------------------------------
                 软件初始化操作相关
*---------------------------------------------------------*/
void software_init()//軟件初始化
{
  tempAndHumi_Ready = false;
  Serial.printf("workingState:%d\r\n", workingState);
  Serial.printf("oledState:%d\r\n", oledState);
  loopStartTime = millis();
  screen_loopEnabled = true;
  show_tip_screen_last = 2500;
  show_BLE_screen_last = 8000;
  show_rec_stop_screen_last = 2000;
  screen_On_last_span = 10000;
  screen_Off_to_sleep_span = 3000;
  screen_On_Start = millis();
  screen_On_now = millis();
  Serial.print("esp_sleep_get_wakeup_cause()");
  Serial.println(esp_sleep_get_wakeup_cause());
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) //如果是系统复位唤醒的, 则停止工作, 亮屏
  {
    workingState = NOT_WORKING;
    oledState = OLED_ON;
  } 
  keyState = NOKEYDOWN;
  screenState = MAIN_TEMP_SCREEN;
  bleState = BLE_OFF;
  lockState = UNLOCKED;
  qualifiedState = QUALITIFY_RIGHT;
  wakeup_init_time();
}