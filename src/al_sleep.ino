#include "config.h"
void go_sleep_a_while_with_ext0()//进入休眠
{
  esp_sleep_enable_ext0_wakeup(WEAKUPKEY1, LOW);//使能按键唤醒
  sleep_update_time();
  if (workingState == WORKING)
    esp_sleep_enable_timer_wakeup(sleeptime - reduce_sleeptime);
  Serial.println("now sleep for " + (String)((sleeptime - reduce_sleeptime)/1000000)+"seconds!");
  gpio_hold_en(GPIO_NUM_32);//锁定电源管脚
  gpio_deep_sleep_hold_en();
  esp_deep_sleep_start();
}