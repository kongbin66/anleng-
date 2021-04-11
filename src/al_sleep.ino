#include "config.h"
void go_sleep_a_while_with_ext0()//进入休眠
{
  uint32_t i;
  //更新睡眠时间
  sleep_update_time();
  //正在记录状态，开启定时唤醒
  if (workingState == WORKING)
  {
     i=sleeptime-(unixtime() - last_rec_stamp);
     if(i<0||i>sleeptime)esp_sleep_enable_timer_wakeup(i+1*1000000);// - reduce_sleeptime);//启用定时唤醒
     else esp_sleep_enable_timer_wakeup(i*1000000);
     Serial.println("sleeptime:"+(String)sleeptime);
     Serial.println("now sleep for " + (String)(sleeptime-(unixtime() - last_rec_stamp))+"seconds!");
  } 
  
  Serial.println("in print last_rec_stamp:"+(String)last_rec_stamp);
      eeprom_config_save_parameter();
  delay(500);
  esp_sleep_enable_ext0_wakeup(WEAKUPKEY1, LOW);//使能按键唤醒
  gpio_hold_en(GPIO_NUM_32);//锁定电源管脚
  gpio_deep_sleep_hold_en();
  esp_deep_sleep_start();
}