#include "config.h"

void sleep_update_time()//更改1
{
  sleep_start_time = sys_sec;              //system_get_time()/1000000;//获取睡眠开始时间 ns-s

 // waking_update_time();
  Serial.printf("sleep at : %d:%d:%d\r\n", now1.hour, now1.minute, now1.second);
 // EEPROM.writeULong(39, now_unixtime + (sys_sec - time_last_async_stamp) / 1000);
  //Serial.println("now_unixtime:"+(String)now_unixtime);
  Serial.println("time_last_async_stamp:"+(String)time_last_async_stamp);
  //Serial.println("EEPROM.writeULong:"+(String)(now_unixtime + (sys_sec - time_last_async_stamp) / 1000));
  EEPROM.commit();
}
void wakeup_init_time()
{
 
  sleep_end_time = sys_sec;//system_get_time()/1000000;//ns-s
  sleep_time_count = sleep_end_time - sleep_start_time;
  Serial.printf("sleep count: %ld second\r\n", sleep_time_count );//gg1不能摘除1000000
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0 && workingState == WORKING)
  {
    reduce_sleeptime += sleep_time_count;
  }
}




#define SECONDS_FROM_1970_TO_2000                                              \
  946684800 
#define pgm_read_byte(addr)   (*(const unsigned char *)(addr))
const uint8_t daysInMonth[] PROGMEM = {31, 28, 31, 30, 31, 30,
                                       31, 31, 30, 31, 30};
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
  if (y >= 2000U)
    y -= 2000U;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
  if (m > 2 && y % 4 == 0)
    ++days;
  return days + 365 * y + (y + 3) / 4 - 1;
}
static uint32_t time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
  return ((days * 24UL + h) * 60 + m) * 60 + s;
}
uint32_t unixtime() 
{

  uint32_t t;
  uint16_t days = date2days(now1.year, now1.month, now1.day);
  t = time2ulong(days, now1.hour, now1.minute, now1.second);
  t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000
  //Serial.println("32wweishijian:"+(String)t);
  return t;
}