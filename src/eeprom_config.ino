#include "config.h"
/************************dataType_bytes************************
 * dataType                 bytes
 * ------------------------------------------------------------
 * char                     1
 * uchar                    1
 * short                    2
 * int                      4
 * time_t                   4
 * int32_t                  4
 * long                     4
 * float                    4
 * double                   8
 * **********************************************************************************************************/

/************************EEPROM_table************************************************************************
 * adress                   length(byte)    type            description
 * ----------------------------------------------------------------------------------------------------------
 * 1                        1                bit           firstLoad_flag
 * 2                        4                long          sleeptime                             休眠时间
 * 6                        1                bool          temp Upper/Lower limit enabled
 * 7                        4                float         temp Upper limit
 * 11                       4                float         temp Lower limit
 * 15                       1                uchar         factory date year
 * 16                       1                uchar         factory date month
 * 17                       1                uchar         factory date day
 * 18                       1                uchar         factory time hour
 * 19                       1                uchar         factory time min
 * 20                       4                time_t(long)  last_rec_stamp                         最后一次发送时间
 * 24                       4                time_t(long)  screen_On_last_span                    亮屏时间
 * 28                       4                time_t(long)  screen_Off_to_sleep_span               息屏到休眠时间
 * ************************************************************************************************************/
void get_eeprom_firstBootFlag()
{
  firstBootFlag = EEPROM.read(1);
  Serial.printf("EEPROM 1: %d \r\n", firstBootFlag);
  
}
void eeprom_config_init()
{
  if (firstBootFlag)
  {
    Serial.println("this is the first load,begin to write default:");
    EEPROM.write(1, 0);
    EEPROM.writeLong(2, FACTORY_SLEEPTIME);
    EEPROM.write(6, FACTORY_TEMP_LIMIT_ENABLE);
    EEPROM.writeFloat(7, FACTORY_TEMP_UPPER_LIMIT);
    EEPROM.writeFloat(11, FACTORY_TEMP_LOWER_LIMIT);
    EEPROM.write(15, FACTORY_DATE_YEAR);//KB类型不对
    EEPROM.write(16, FACTORY_DATE_MONTH);
    EEPROM.write(17, FACTORY_DATE_DAY);
    EEPROM.write(18, FACTORY_TIME_HOUR);
    EEPROM.write(19, FACTORY_TIME_MIN);
   // EEPROM.writeULong(20, 0);
    EEPROM.commit();
    firstBootFlag = 0;

    screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
  }
  else
  {
    Serial.println("this is not the first load");

    sleeptime =         (time_t)EEPROM.readLong(2);        Serial.printf("sleeptime:%ld,byte:%d\r\n", sleeptime,sizeof(sleeptime));
    tempLimit_enable =  EEPROM.read(6) == 0 ? false : true;
    tempUpperLimit =    EEPROM.readFloat(7);              Serial.printf("tempUpperLimit:%.2f,byte:%d\r\n", tempUpperLimit,sizeof(tempUpperLimit));
    tempLowerLimit =    EEPROM.readFloat(11);              Serial.printf("tempLowerLimit:%.2f,byte:%d\r\n", tempLowerLimit,sizeof(tempLowerLimit));
    last_rec_stamp =    (time_t)EEPROM.readLong(20);       Serial.printf("last_rec_stamp:%ld,byte:%d\r\n", last_rec_stamp,sizeof(last_rec_stamp));
    screen_On_last_span=(time_t)EEPROM.readLong(24);        Serial.printf("screen_On_last_span:%ld,byte:%d\r\n", screen_On_last_span,sizeof(screen_On_last_span));
    screen_Off_to_sleep_span=(time_t)EEPROM.readLong(28);        Serial.printf("screen_Off_to_sleep_span:%ld,byte:%d\r\n", screen_Off_to_sleep_span,sizeof(screen_Off_to_sleep_span));
    //处理时间
    Serial.printf("time now: %d-%d-%d %d:%d:%d\r\n", now1.year, now1.month, now1.day,now1.hour, now1.minute, now1.second);
 
  }
}

void eeprom_config_save_parameter(void)
{
   // EEPROM.write(1, 1);
    EEPROM.writeLong(2, sleeptime);
    EEPROM.write(6, FACTORY_TEMP_LIMIT_ENABLE);
    // EEPROM.writeFloat(7, FACTORY_TEMP_UPPER_LIMIT);
    // EEPROM.writeFloat(11, FACTORY_TEMP_LOWER_LIMIT);
    // EEPROM.writeInt(15, FACTORY_DATE_YEAR);
    // EEPROM.writeInt(16, FACTORY_DATE_MONTH);
    // EEPROM.writeInt(17, FACTORY_DATE_DAY);
    // EEPROM.writeInt(18, FACTORY_TIME_HOUR);
    // EEPROM.writeInt(19, FACTORY_TIME_MIN);
    EEPROM.writeULong(20, last_rec_stamp);
    EEPROM.writeInt(24, screen_On_last_span);  //亮屏时间
    EEPROM.writeInt(28, screen_Off_to_sleep_span);  // 息屏到休眠时间
    EEPROM.commit();
}