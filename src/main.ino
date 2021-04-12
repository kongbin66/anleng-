#include "config.h"

TaskHandle_t task1; //第二核创建一个任务句柄
TaskHandle_t ds_task;
TaskHandle_t xieyi_task;

int rollback = 0;

//第二核创建任务代码
void codeForTask1(void *parameter)
{

  while (1) //这是核1 的loop
  {
    vTaskDelay(10);
    button.tick(); //扫描按键
  }
   vTaskDelete(NULL);
}

void ds1302_task(void *parameter)
{  uint8_t sec=0;
   while(1)
   {
     ds_rtc.getDateTime(&now1);//读取时间参数到NOW
     
     if(now1.second==sec+1)
     {
        sys_sec++;
       // Serial.printf("sec:%d\n",sys_sec);
     }
     sec=now1.second;
     digitalWrite(LED,HIGH); 
     vTaskDelay(250);
     digitalWrite(34,LOW); 
     vTaskDelay(250);
   }
   vTaskDelete(NULL);
}

void xieyi_Task(void *parameter)
{
  while (1) //这是核1 的loop
  {
    xieyi_scan();
    vTaskDelay(100);
  }
   vTaskDelete(NULL);
}




void setup()
{ 
  gpio_hold_dis(GPIO_NUM_32);//解锁电源引脚
  gpio_deep_sleep_hold_dis();
  
  hardware_init(); //硬件初始化
  software_init(); //软件初始化
  
  
   xTaskCreate( xieyi_Task,"xieyi_Task",3000,NULL,2,&xieyi_task);//创建DS1302任务
   xTaskCreate( ds1302_task,"ds1302_task",2000,NULL,2,&ds_task);//创建DS1302任务
   xTaskCreatePinnedToCore(codeForTask1,"task1",1000,NULL,2, &task1,0);  
  if (rollback)
  {
    /*************如果rollback置1, 会恢复出厂设置,数据全清***********/
    Serial.println("clean EEPROM");
    EEPROM.write(1, 0);
    EEPROM.commit();
    Serial.println("OK");
    ESP.deepSleep(300000000);
    modem.sleepEnable();
  }
  else
  {         
    get_eeprom_firstBootFlag();           //获取EEPROM第1位,判断是否是初次开机
    alFFS_init();                         //初始化FFS
    eeprom_config_init();                 //初始化EEPROM
    wakeup_init_time();
  }

  if (oledState == OLED_ON)
   { 
    showWelcome();
    postMsgId=0;//清记录条数
   }
  else 
  {
    if (workingState == WORKING && (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)) //不是开机，是定时唤醒。
    {
      postMsgId++;
      send_Msg_var_GSM_while_OLED_off(); //上传
      go_sleep_a_while_with_ext0();      //休眠
    }
  }
}




void loop()
{
  if(POWER_warning_flag)
  {
    Serial.println("dianliaodi!");
  }
  if (oledState == OLED_ON)
  {
    sht20getTempAndHumi();
    screen_loop(); //展示和滚屏
    key_loop();
    screen_show(); //OLED最终显示
    send_Msg_var_GSM_while_OLED_on();

  }
  oled_on_off_switch();
}








void send_Msg_var_GSM_while_OLED_off()
{
  setupModem();          //SIM800L物理开机
  modemToGPRS();         //modem连接GPRS
  getLBSLocation();      //获取定位信息
  sht20getTempAndHumi(); //获取温湿度数据
  onenet_connect();
  if (client.connected())
  {
  
    char subscribeTopic[75];//订阅主题
    char topicTemplate[] = "$sys/%s/%s/cmd/request/#"; //信息模板
    snprintf(subscribeTopic, sizeof(subscribeTopic), topicTemplate, mqtt_pubid, mqtt_devid);
    client.subscribe(subscribeTopic); //订阅命令下发主题
    sendTempAndHumi();
  }
  alFFS_addRec();
  alFFS_readRecing();
  
  delay(1000);
  digitalWrite(MODEM_POWER_ON, LOW); //关断800C电源
  last_rec_stamp =unixtime();
}








void send_Msg_var_GSM_while_OLED_on()
{
 
  if (workingState == WORKING)
  {
      now_rec_stamp = unixtime();

       
      Serial.println("GSM transmission will start at:"+(String)(sleeptime-(now_rec_stamp - last_rec_stamp)) );
    if (now_rec_stamp - last_rec_stamp > sleeptime )//发送间隔
    {
      Serial.println("now_rec_stamp:"+(String)now_rec_stamp);
      Serial.println("last_rec_stamp:"+(String)last_rec_stamp);
      
      screen_loopEnabled = false;
      key_attach_null();
      //上传
      display.clear();
      display.setFont(Roboto_Condensed_12);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.drawString(64, 5, "Initializing modem...");
      display.drawProgressBar(5, 50, 118, 8, 5);
      display.display();
      setupModem(); //SIM800L物理开机

      display.clear();
      display.drawString(64, 5, "Waiting for network...");
      display.drawProgressBar(5, 50, 118, 8, 40);
      display.display();
      modemToGPRS(); //modem连接GPRS

      display.clear();
      display.drawString(64, 5, "getting LBS...");
      display.drawProgressBar(5, 50, 118, 8, 70);
      display.display();
      getLBSLocation(); //获取定位信息

      display.clear();
      display.drawString(64, 5, "connecting to OneNet");
      display.drawProgressBar(5, 50, 118, 8, 90);
      display.display();
      sht20getTempAndHumi(); //获取温湿度数据
      onenet_connect();

      display.clear();
      display.drawString(64, 5, "uploading...");
      if (client.connected())
      {
        char subscribeTopic[75];
        char topicTemplate[] = "$sys/%s/%s/cmd/request/#"; //信息模板
        snprintf(subscribeTopic, 75, topicTemplate, mqtt_pubid, mqtt_devid);
        client.subscribe(subscribeTopic); //订阅命令下发主题
        sendTempAndHumi();
      }
      display.drawProgressBar(5, 50, 118, 8, 100);
      display.display();
      delay(200);
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      key_init();
      last_rec_stamp =unixtime();
      screen_loopEnabled = true;
      screen_On_Start = sys_sec;
      screen_On_now = sys_sec;
      
      alFFS_addRec();
      alFFS_readRecing();
     
      postMsgId++;
    }
    
  }
  digitalWrite(MODEM_POWER_ON, LOW);
  
}
//设置休眠时间：（S）
void SET_SLEEPTIME(time_t t)
{
    sleeptime= t;
    eeprom_config_save_parameter();
    sleeptime = (time_t)EEPROM.readLong(2);  Serial.printf("sleeptime:%ld\r\n", sleeptime);
}
//设置亮屏时间和息屏到休眠时间
void SET_Last_span_Sleep_span(int x,int y)
{
    screen_On_last_span=x;
    screen_Off_to_sleep_span=y;
    eeprom_config_save_parameter();
    screen_On_last_span = (time_t)EEPROM.readInt(43);      Serial.printf("screen_On_last_span:%ld\r\n", screen_On_last_span);
    screen_Off_to_sleep_span = (time_t)EEPROM.readInt(47); Serial.printf("screen_Off_to_sleep_span:%ld\r\n", screen_Off_to_sleep_span);
}









