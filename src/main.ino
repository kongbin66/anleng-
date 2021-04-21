#include "config.h"

TaskHandle_t task1=NULL; //第二核创建一个任务句柄
TaskHandle_t ds_task=NULL;
TaskHandle_t xieyi_task=NULL;

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
{
  uint8_t sec = 0;
  while (1)
  {
    ds_rtc.getDateTime(&now1); //读取时间参数到NOW

    if (now1.second == sec + 1)
    {
      sys_sec++;
      // Serial.printf("sec:%d\n",sys_sec);
    }
    sec = now1.second;
    digitalWrite(LED, HIGH);
    vTaskDelay(250);
    digitalWrite(34, LOW);
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
  gpio_hold_dis(GPIO_NUM_32); //解锁电源引脚
  gpio_deep_sleep_hold_dis();

  hardware_init(); //硬件初始化
  software_init(); //软件初始化

  xTaskCreate(xieyi_Task, "xieyi_Task", 4000, NULL, 2, &xieyi_task); //创建DS1302任务
  xTaskCreate(ds1302_task, "ds1302_task", 3000, NULL, 2, &ds_task);  //创建DS1302任务
  xTaskCreatePinnedToCore(codeForTask1, "task1", 1000, NULL, 2, &task1, 0);
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
    get_eeprom_firstBootFlag(); //获取EEPROM第1位,判断是否是初次开机
    alFFS_init();               //初始化FFS
    eeprom_config_init();       //初始化EEPROM
    wakeup_init_time();
  }

  // if (oledState == OLED_ON)
  // {
  //   showWelcome();
  //   postMsgId = 0; //清记录条数
  // }
  // else
  // {
  //   Serial.printf("NO,BUYAOJINRU CONGXINKAISHI ");
  //   // if (workingState == WORKING && (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)) //不是开机，是定时唤醒。
  //   // {
  //   //   //send_Msg_var_GSM_while_OLED_off(); //上传
  //   //   //testx();
  //   //   go_sleep_a_while_with_ext0();      //休眠
  //   // }
  // }
}

void loop()
{
  // if (POWER_warning_flag)
  // {
  //   Serial.println("diaodianle!");
  // }

  // if (oledState == OLED_ON)
  // {
  //   sht20getTempAndHumi();
  //   screen_loop(); //展示和滚屏
  //   key_loop();
  //   screen_show(); //OLED最终显示
  //   //send_Msg_var_GSM_while_OLED_off2();
  // }
  // oled_on_off_switch();
}


uint8_t i;
void send_Msg_var_GSM_while_OLED_off2()
{
  Serial.printf("Jinru off2 fasong chengxv!!OK!");
  if (f_Flight_Mode == 0) //飞行模式未打开
  {
    setupModem();         //SIM800L物理开机
    modemToGPRS();        //modem连接GPRS
    //確定网络通畅
    if (0==getLBSLocation()) //网络通畅
    {
      Serial.println("wangluo tongchang ok");
      //确定有无漏发文件
      if (f_lose == 0) //没有
      {
        Serial.println("Wu LOU FA! ok");
        //发送数据
        sht20getTempAndHumi(); //获取温湿度数据
        onenet_connect();
        if (client.connected()) sendTempAndHumi();
        //保存本条数据
        alFFS_addRec();
        alFFS_readRecing();
      }
      else //有
      {
        //保存本条信息
        alFFS_addRec();
        alFFS_readRecing();
        //读漏发文件
        //逐条发送
        //发送本条信息
        //发送完成后请漏发文件
        f_lose = 0;
      }
    }
    else //网络不通
    {
      //本条信息保存文件系统
      Serial.println("wangluo_butong ! err");
      alFFS_addRec();
      alFFS_readRecing();
      //本条信息保存漏发文件，并使能漏发标志
      f_lose = 1;
    }
  }
  else //飞行模式打开了
  {
    Serial.println("feihangmoshi oN");
    //本条信息保存文件系统
    //本条信息保存漏发文件，并使能漏发标志
    f_lose = 1;
  }
}

void send_Msg_var_GSM_while_OLED_on()
{

  if (workingState == WORKING)
  {
    now_rec_stamp = unixtime();

    Serial.println("GSM transmission will start at:" + (String)(sleeptime - (now_rec_stamp - last_rec_stamp)));
    if (now_rec_stamp - last_rec_stamp > sleeptime) //发送间隔
    {
      Serial.println("now_rec_stamp:" + (String)now_rec_stamp);
      Serial.println("last_rec_stamp:" + (String)last_rec_stamp);

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
      last_rec_stamp = unixtime();
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
  sleeptime = t;
  EEPROM.writeLong(2, sleeptime);
  EEPROM.commit();
  sleeptime = (time_t)EEPROM.readLong(2);
  Serial.printf("sleeptime:%ld\r\n", sleeptime);
}
//设置亮屏时间和息屏到休眠时间
void SET_Last_span_Sleep_span(int x, int y)
{
  screen_On_last_span = x;
  screen_Off_to_sleep_span = y;
  EEPROM.writeInt(43, screen_On_last_span);      //亮屏时间
  EEPROM.writeInt(47, screen_Off_to_sleep_span); // 息屏到休眠时间
  EEPROM.commit();
  screen_On_last_span = (time_t)EEPROM.readInt(43);
  Serial.printf("screen_On_last_span:%ld\r\n", screen_On_last_span);
  screen_Off_to_sleep_span = (time_t)EEPROM.readInt(47);
  Serial.printf("screen_Off_to_sleep_span:%ld\r\n", screen_Off_to_sleep_span);
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
    char subscribeTopic[75];                           //订阅主题
    char topicTemplate[] = "$sys/%s/%s/cmd/request/#"; //信息模板
    snprintf(subscribeTopic, sizeof(subscribeTopic), topicTemplate, mqtt_pubid, mqtt_devid);
    client.subscribe(subscribeTopic); //订阅命令下发主题
    sendTempAndHumi();
  }
  alFFS_addRec();
  alFFS_readRecing();

  delay(1000);
  digitalWrite(MODEM_POWER_ON, LOW); //关断800C电源
  last_rec_stamp = unixtime();
  postMsgId++;
}
int tempStr =0;


//检测网络
bool GSM_starts_networking()
{
    setupModem();     //SIM800L物理开机
    modemToGPRS();    //modem连接GPRS
    //获取定位信息
    return getLBSLocation();
}

// bool GSM_send_data(String x)
// {
//     onenet_connect();
//     if (client.connected())
//     {
//           char subscribeTopic[75];                           //订阅主题
//           char topicTemplate[] = "$sys/%s/%s/cmd/request/#"; //信息模板
//           snprintf(subscribeTopic, sizeof(subscribeTopic), topicTemplate, mqtt_pubid, mqtt_devid);
//           client.subscribe(subscribeTopic); //订阅命令下发主题
//           sendTempAndHumi();
//     }
// }



void send_Msg_var_GSM_while_OLED_on2()
{
  if (f_Flight_Mode == 0) //飞行模式未打开
  {
    
    //確定网络通畅
    if (GSM_starts_networking()) //网络通畅
    {
      //确定有无漏发文件
      if (f_lose == 0) //没有
      {
        sht20getTempAndHumi(); //获取温湿度数据
       // GSM_send_data(String x);
        //保存本条数据
          String strtemp = "{\"st\":\"" + (String)tempStr +
                           "\",\"data\": [{\"tm\":\"" + (String)tempStr +
                           "\",\"tmsp\":" + (String)(unixtime()) + //- 8 * 60 * 60
                           ",\"tp\":" + (String)currentTemp +
                           ",\"h\":" + (String)currentHumi +
                           ",\"E\":" + (String)locationE +
                           ",\"N\":" + (String)locationN +
                           "}";
          appendFile(SPIFFS, "/list.json", strtemp);
          readFile(SPIFFS, "/list.json");
      }
      else //有
      {
        //保存本条信息
        //读漏发文件
        //逐条发送
        //发送本条信息
        //发送完成后请漏发文件
        writeFile(SPIFFS, "/lose.json","");
      }
    }
    else //网络不通
    {
      //本条信息保存文件系统
      //本条信息保存漏发文件，并使能漏发标志
    }
  }
  else //飞行模式打开了
  {
    //关闭GSM电源
    digitalWrite(MODEM_POWER_ON, LOW);
    //本条信息保存文件系统

    //本条信息保存漏发文件，并使能漏发标志
  }
}



//发送格式
// {
//     "id": "123",
//     "version": "1.0",
//     "params": {
//         "Power": {
//             "value": "12345",
//             "time": 1599534283111
//         },
//         "temp": {
//             "value": 23.6,
//             "time": 1599534283111
//         }   
//     }
// }

//漏发文本记录格式
// {
//     "id": "123",
//     "version": "1.0",
//     "params": {
//         "Power": {
//             "value": "12345",
//             "time": 1599534283111
//         },
//         "temp": {
//             "value": 23.6,
//             "time": 1599534283111
//         }   
//     }
// }

//


                           







// currentTemp, currentHumi, locationE, locationN,now_unixtime;
//  String strtempx = "{\"st\":\"" + (String)tempStr +
//                            "\",\"data\": [{\"tm\":\"" + (String)tempStr +
//                            "\",\"tmsp\":" + (String)(unixtime()) + //- 8 * 60 * 60
//                            ",\"tp\":" + (String)currentTemp +
//                            ",\"h\":" + (String)currentHumi +
//                            ",\"E\":" + (String)locationE +
//                            ",\"N\":" + (String)locationN +
//                            "}";
// String strtemp2 = "{\"id\": \"1\",\"params\": {\"temp\":{\"value\":"+(String)currentTemp+
//                                                         "\"time\":" +(String)now_unixtime+
//                                               "},\"humi\":{\"value\":"+(String)currentHumi+
//                                               ",\"time\":"+(String)now_unixtime+
//                                               "},\"le\":{\"value\":"+(String)locationE+
//                                               ",\"time\":" +(String)now_unixtime+
//                                               "},\"ln\":{\"value\":"+(String)locationN+
//                                                ",\"time\": "+(String)now_unixtime+
//                                               "},\"start_time\":{\"value\":"+(String)now_unixtime+
//                                               ",\"time\":"+ (String)now_unixtime+"}}}"






// void send_Msg_var_GSM_while_OLED_off()
// {
  
//   alFFS_addRec();
//   alFFS_readRecing();

//   delay(1000);
//   digitalWrite(MODEM_POWER_ON, LOW); //关断800C电源
//   last_rec_stamp = unixtime();
//   postMsgId++;
// }






void sendTempAndHumi2()
{
  if (client.connected())
  {
    //先拼接出json字符串
    //char param[178];
    char jsonBuf[500];
    String str = "{\"id\":\"1\",\"params\":{\"temp\":{\"value\":"+(String)currentTemp+
                                              
                                              "},\"humi\":{\"value\":"+(String)currentHumi+
                                           
                                              "},\"le\":{\"value\":"+(String)locationE+
                                         
                                              "},\"ln\":{\"value\":"+(String)locationN+
                                            
                                              "},\"start_time\":{\"value\":"+(String)unixtime()+
                                             "}}}";
    int i=writeFile(SPIFFS, "/lose.json",jsonBuf );
    Serial.printf("size:%d\r\n",i);
    //str.toCharArray(jsonBuf,500,0);
    Serial.println(jsonBuf);
    
    //sprintf(param, "{\"temp\":{\"value\":%.2f},\"humi\":{\"value\":%.2f},\"le\":{\"value\":%.2f},\"ln\":{\"value\":%.2f},\"start_time\":{\"value\":%u000}}", currentTemp, currentHumi, locationE, locationN,now_unixtime); //我们把要上传的数据写在param里 

    //sprintf(jsonBuf, ONENET_POST_BODY_FORMAT,param);




    //再从mqtt客户端中发布post消息
    if (client.publish(ONENET_TOPIC_PROP_POST, jsonBuf))
    {
      Serial.print("Post message to cloud: ");
      Serial.println(jsonBuf);
      current_rec_State = KEEP_RECING;
    }
    else
    {
      Serial.println("Publish message to cloud failed!");
    }

  }
}



void test()
{
  String strtemp = "{\"st\":\"" + (String)tempStr +
                           "\",\"data\": [{\"tm\":\"" + (String)tempStr +
                           "\",\"tmsp\":" + (String)(unixtime()) + //- 8 * 60 * 60
                           ",\"tp\":" + (String)currentTemp +
                           ",\"h\":" + (String)currentHumi +
                           ",\"E\":" + (String)locationE +
                           ",\"N\":" + (String)locationN +
                           "}";
         
          appendFile(SPIFFS, "/list.json", strtemp);
          readFile(SPIFFS, "/list.json");
}
void test2()
{
  listDir(SPIFFS, "/", 0);

}
void test3()
{
  Serial.printf("format FFS:%d", SPIFFS.format());

}
void test4()
{
  writeFile(SPIFFS, "/lose.json","");
  writeFile(SPIFFS, "/list.json","");

}

void test5(uint8_t f_one)//读文件辨认条目
{
  if(f_one)
  {
    writeFile(SPIFFS, "/lose.json","");
  }
  else
  {
     appendFile(SPIFFS, "/lose.json","456");
  }
  //读取文件
  readFile(SPIFFS,  "/lose.json");

}



//文本记录格式 
// {
//   "id":"1",
//   "params":
//           {
//           "temp":{"value":34.83},
//           "humi":{"value":15.54},
//           "le":{"value":117.62},
//           "ln":{"value":35.90},
//           "start_time":{"value":1618754516000}
//           }
// }
//  String strtempx = "{\"st\":\"" + (String)tempStr +
//                            "\",\"data\": [{\"tm\":\"" + (String)tempStr +
//                            "\",\"tmsp\":" + (String)(unixtime()) + //- 8 * 60 * 60
//                            ",\"tp\":" + (String)currentTemp +
//                            ",\"h\":" + (String)currentHumi +
//                            ",\"E\":" + (String)locationE +
//                            ",\"N\":" + (String)locationN +
//                            "}";
void testx(bool x)//写漏發文件
{

   
}