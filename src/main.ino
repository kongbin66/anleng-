#include "config.h"

TaskHandle_t task1 = NULL; //第二核创建一个任务句柄
TaskHandle_t ds_task = NULL;
TaskHandle_t xieyi_task = NULL;

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

  if (oledState == OLED_ON)
  {
    showWelcome();
    postMsgId = 0; //清记录条数
  }
  else
  {
    Serial.printf("NO,BUYAOJINRU CONGXINKAISHI ");
    if (workingState == WORKING && (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)) //不是开机，是定时唤醒。
    {
      send_Msg_var_GSM_while_OLED_on(0); //上传

      go_sleep_a_while_with_ext0(); //休眠
    }
  }
}

void loop()
{
  if (POWER_warning_flag)
  {
    Serial.println("diaodianle!");
  }

  if (oledState == OLED_ON)
  {
    sht20getTempAndHumi();
    screen_loop(); //展示和滚屏
    key_loop();
    screen_show(); //OLED最终显示
    send_Msg_var_GSM_while_OLED_on(1);
  }
  oled_on_off_switch();
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

void send_Msg_var_GSM_while_OLED_off()
{

  Serial.printf("Jinru off2 fasong chengxv!!OK!");
  if (f_Flight_Mode == 0) //飞行模式未打开
  {
    setupModem();  //SIM800L物理开机
    modemToGPRS(); //modem连接GPRS
    //確定网络通畅
    bool i = getLBSLocation();
    if (i == 0) //网络通畅
    {

      Serial.println("wangluo tongchang ok");
      //确定有无漏发文件
      if (f_lose == 0) //没有
      {
        Serial.println("Wu LOU FA! ok");
        //发送数据
        sht20getTempAndHumi(); //获取温湿度数据
        onenet_connect();
        if (client.connected())
          sendTempAndHumi();
        //保存本条数据
        alFFS_addlist();
        alFFS_readlist();
      }
      else //有
      {
        Serial.println("YOU lou fa!");
        //保存本条信息
        alFFS_addlose();
        alFFS_addlist();

        alFFS_readlist();
        alFFS_readlose();
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
      alFFS_addlose();
      alFFS_addlist();

      alFFS_readlist();
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

void test100(bool a)
{
  Serial.printf("goto testx!");
  if (workingState == WORKING)
  {
    now_rec_stamp = unixtime();

    Serial.println("GSM transmission will start at:" + (String)(sleeptime - (now_rec_stamp - last_rec_stamp)));
    // if (now_rec_stamp - last_rec_stamp > sleeptime) //发送间隔
    {
      Serial.println("now_rec_stamp:" + (String)now_rec_stamp);
      Serial.println("last_rec_stamp:" + (String)last_rec_stamp);

      screen_loopEnabled = false;
      key_attach_null();
      //上传
      if (a)
      {
        display.clear();
        display.setFont(Roboto_Condensed_12);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 5, "Initializing modem...");
        display.drawProgressBar(5, 50, 118, 8, 5);
        display.display();
      }
      setupModem(); //SIM800L物理开机
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "Waiting for network...");
        display.drawProgressBar(5, 50, 118, 8, 40);
        display.display();
      }

      modemToGPRS(); //modem连接GPRS
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "getting LBS...");
        display.drawProgressBar(5, 50, 118, 8, 70);
        display.display();
      }
      getLBSLocation(); //获取定位信息
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "connecting to OneNet");
        display.drawProgressBar(5, 50, 118, 8, 90);
        display.display();
      }
      sht20getTempAndHumi(); //获取温湿度数据
      onenet_connect();
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "uploading...");
      }

      if (client.connected())
      {
        sendTempAndHumi();
      }
      else
        Serial.printf("sendTempAndHumi() eer!\n");
      if (a)
      {
        display.drawProgressBar(5, 50, 118, 8, 100);
        display.display();
        delay(200);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
      }

      key_init();
      last_rec_stamp = unixtime();
      screen_loopEnabled = true;
      screen_On_Start = sys_sec;
      screen_On_now = sys_sec;

      alFFS_addlist();
      // alFFS_readRecing();

      postMsgId++;
    }
  }
  digitalWrite(MODEM_POWER_ON, LOW);
}

void send_Msg_var_GSM_while_OLED_on(bool a)
{
  //Serial.printf("goto send_Msg_var_GSM_while_OLED_on(bool a)\n");

  now_rec_stamp = unixtime(); //读现在时间
  screen_loopEnabled = false;
  //key_attach_null();
  //确定进入条件
  if (workingState == WORKING && f_Flight_Mode == false) //工作模式和飞行模式关闭（正常记录）
  {
    //Serial.printf("zhengchang jilu\n ");
    if (now_rec_stamp - last_rec_stamp > sleeptime) //记录间隔到了吗？
    {
      Serial.printf("zhengchang mode time Ok!\n ");
      //1.需要联网测网络
      if (a)
      {
        display.clear();
        display.setFont(Roboto_Condensed_12);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 5, "Initializing modem...");
        display.drawProgressBar(5, 50, 118, 8, 5);
        display.display();
      }
      setupModem();
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "Waiting for network...");
        display.drawProgressBar(5, 50, 118, 8, 40);
        display.display();
      }
      modemToGPRS();
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "getting LBS...");
        display.drawProgressBar(5, 50, 118, 8, 70);
        display.display();
      }
      if (!getLBSLocation()) //检查网络情况
      {
        Serial.printf("zhengchang mode wangluo  OK！\n ");
        //检查有漏发文件和飞行模式标志吗
        if (f_Flight_Mode == true && f_lose == true) //正在飞行模式中
        {
          Serial.printf("feixing mode zhijietuichu ！\n ");
          f_Flight_Mode = true;
          f_lose = true;
        }
        else if (f_Flight_Mode == false && f_lose == false) //正常记录和发送
        {
          Serial.printf("run ！\n ");
          if (a)
          {
            display.clear();
            display.drawString(64, 5, "connecting to OneNet");
            display.drawProgressBar(5, 50, 118, 8, 90);
            display.display();
          }
          //获取时间温度
          sht20getTempAndHumi();
          //1.记录正常文件
          alFFS_addlist();
          alFFS_readlist();
          //2.发送数据
          onenet_connect();
          if (a)
          {
            display.clear();
            display.drawString(64, 5, "uploading...");
          }
          if (client.connected())
          {
            sendTempAndHumi();
            if (a)
            {
              display.drawProgressBar(5, 50, 118, 8, 100);
              display.display();
              delay(200);
              display.setTextAlignment(TEXT_ALIGN_LEFT);
            }

            key_init();
            last_rec_stamp = unixtime();
            screen_loopEnabled = true;
            screen_On_Start = sys_sec;
            screen_On_now = sys_sec;

            postMsgId++;
          }
          else
            Serial.printf("sendTempAndHumi() eer!\n"); //这里应加入F_LOSE=1?
          //3.标志位
        }
        else if (f_Flight_Mode == false && f_lose == true) //有漏发文件非飞行模式
        {
          Serial.printf("fei feixingmoshi youloufa \n");
        
          //调用一个函数
        }
        else
          Serial.printf("qitamoshi zhong tuicu! qingjiancha !!!!!!!\n");
      }
      else
      {
        Serial.printf("zhengchang mode wangluo  eer！\n ");
        //1.直接写漏发文件和正常记录文件
        alFFS_addlist();
        alFFS_addlose();
        alFFS_readlist();
        alFFS_readlose();
        //置位标志位
        f_lose = true;
      }
    }
   // else
     // Serial.printf("zhengchang mode time no!");
  }
  else if (workingState == WORKING && f_Flight_Mode == true) //工作模式和飞行模式关闭（不上传网络）
  {
    Serial.println("jilu no send");
    if (now_rec_stamp - last_rec_stamp > sleeptime) //记录间隔到了吗？
    {
      //1.直接写漏发文件和正常记录文件
      alFFS_addlist();
      alFFS_addlose();
      alFFS_readlist();
      alFFS_readlose();
      //置位标志位
      f_lose = true;

      key_init();
      last_rec_stamp = unixtime();
      screen_loopEnabled = true;
      screen_On_Start = sys_sec;
      screen_On_now = sys_sec;
    }
    else
      Serial.println("feixing mode timer no");
  }
  else //无操作，退出
  {
    //Serial.println("no worke , jump out!");
      if((f_lose==true)&&(workingState ==NOT_WORKING)&&(f_Flight_Mode==false)&&(old_workingstate==0))//真正的状态
      {
          Serial.println("bufa louchuan");
          old_workingstate= workingState;
          workingState = WORKING; 
          key_init();
          last_rec_stamp = unixtime();
 //         screen_loopEnabled = true;
          // screen_On_Start = sys_sec;
          // screen_On_now = sys_sec;       
      }
         
  }
}

void test101(bool a)
{
  Serial.printf("goto testx!");
  workingState = WORKING;
  if (workingState == WORKING)
  {
    now_rec_stamp = unixtime();

    Serial.println("GSM transmission will start at:" + (String)(sleeptime - (now_rec_stamp - last_rec_stamp)));
    // if (now_rec_stamp - last_rec_stamp > sleeptime) //发送间隔
    {
      Serial.println("now_rec_stamp:" + (String)now_rec_stamp);
      Serial.println("last_rec_stamp:" + (String)last_rec_stamp);

      screen_loopEnabled = false;
      key_attach_null();
      //上传
      if (a)
      {
        display.clear();
        display.setFont(Roboto_Condensed_12);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 5, "Initializing modem...");
        display.drawProgressBar(5, 50, 118, 8, 5);
        display.display();
      }
      setupModem(); //SIM800L物理开机
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "Waiting for network...");
        display.drawProgressBar(5, 50, 118, 8, 40);
        display.display();
      }

      modemToGPRS(); //modem连接GPRS
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "getting LBS...");
        display.drawProgressBar(5, 50, 118, 8, 70);
        display.display();
      }
      getLBSLocation(); //获取定位信息
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "connecting to OneNet");
        display.drawProgressBar(5, 50, 118, 8, 90);
        display.display();
      }
      sht20getTempAndHumi(); //获取温湿度数据
      onenet_connect();
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "uploading...");
      }

      if (client.connected())
      {
        sendTempAndHumi();
      }
      else
        Serial.printf("sendTempAndHumi() eer!\n");
      if (a)
      {
        display.drawProgressBar(5, 50, 118, 8, 100);
        display.display();
        delay(200);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
      }

      key_init();
      last_rec_stamp = unixtime();
      screen_loopEnabled = true;
      screen_On_Start = sys_sec;
      screen_On_now = sys_sec;

      alFFS_addlist();
      // alFFS_readRecing();

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

void test2(bool a)
{
  alFFS_thisRec_firstData_flag=a;
  writeFile(SPIFFS, "/list.json", "");
  writeFile(SPIFFS, "/lose.json", "");
    f_lose=0;
  listDir(SPIFFS, "/", 0);
}
void test3(bool a)
{
  f_Flight_Mode = a;
  workingState = a;
  Serial.printf("f_Flight_Mode=%d\n", f_Flight_Mode);
}
void test4(bool a)
{
  f_lose = a;
  Serial.printf("f_lose=%d\n", f_lose);
}

void test5(uint8_t a) //读文件辨认条目
{
  if (a == 0)
  {
    writeFile(SPIFFS, "/list.json", "");
    writeFile(SPIFFS, "/lose.json", "");
    f_lose=0;
  }
  else if (a == 1)
  {
    writeFile(SPIFFS, "/list.json", "");
  }
  else if (a == 2)
  {
    writeFile(SPIFFS, "/lose.json", "");
    f_lose=0;
  }

  //读取文件mulu
  listDir(SPIFFS, "/", 0);
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


void testx ()
{
  uint32_t i;
  //读取漏发文件
  File f = SPIFFS.open("/lose.json", FILE_READ);
  i=f.available();
  char bb[i];
  Serial.println("lose file size:"+(String)i);  
  String aa=f.readString();
  aa.toCharArray(bb,i,0);
  //Serial.println(bb);
  


 //

}


void testy(bool b)
{
  StaticJsonDocument<200> doc;
  doc["id"]   = "1";
  doc["params"] = 1351824120;
}