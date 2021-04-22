#include "config.h"

void alFFS_init()
{
  if (firstBootFlag)
  {//格式化文件系统
     Serial.printf("format FFS:%d", SPIFFS.format());
     //挂载文件系统
     if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
     {
         Serial.println("SPIFFS Mount Failed");
         f_ffsok=0;
         return;
     }
     else f_ffsok=1;
     //创建两个文件
     if(f_ffsok)
     {
        writeFile(SPIFFS, "/list.json", "");
        writeFile(SPIFFS, "/lose.json", "");
     }  
  } 
  else
    Serial.printf("FFS size:%d,used:%d\r\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());
}

// alFFS_addloseRec()
// {

// }
void alFFS_addlist()
{
   Serial.println("in alffs_addlist");
//    //时间日期
    char tempStr[18];
    char tempStrtemplate[] = "%d%02d%02d %02d:%02d:%02d";
    snprintf(tempStr, sizeof(tempStr), tempStrtemplate, (now1.year+2000),now1.month,now1.day,now1.hour,now1.minute,now1.second);
   Serial.printf("DATE:%d\n",strlen( tempStr));
    sht20getTempAndHumi(); //读取温湿度，
    //连不上网没有坐标位置


//  Serial.println(tempStr);
 // Serial.print("now the alFFS_thisRec_firstData_flag value is :");
  Serial.printf("firstDdata=%d\n",alFFS_thisRec_firstData_flag);
  if (alFFS_thisRec_firstData_flag)
  {
      Serial.println("first rec, so create a file named:");

    // char tempPathtemplate[] = "/R%d%02d%02d_%02d%02d.json";
    // snprintf(nowREC_filepath, sizeof(nowREC_filepath), tempPathtemplate, now1.year,now1.month,now1.day,now1.hour,now1.minute);
 //   Serial.println(nowREC_filepath);
 //   Serial.println("now first write content to it");
    File f = SPIFFS.open("/list.json", FILE_WRITE);
    String strtemp = "{\"st\":\"" + (String)tempStr +
                     "\",\"data\": [{\"tm\":\"" + (String)tempStr +
                     "\",\"tmsp\":" + (String)(unixtime() ) +
                     ",\"tp\":" + (String)currentTemp +
                     ",\"h\":" + (String)currentHumi +
                     ",\"E\":" + (String)locationE +
                     ",\"N\":" + (String)locationN +
                     "}]}";
    f.println(strtemp);
   // Serial.println("ADD:" + strtemp);
    f.close();
    alFFS_thisRec_firstData_flag = 0;
  }
  else
  {
    Serial.println("not the first rec, so i can just append some content in to the file:");
    
    File f = SPIFFS.open("/list.json", FILE_APPEND);
        String strtemp = ",{\"tm\":\"" + (String)tempStr +
                     "\",\"data\": [{\"tm\":\"" + (String)tempStr +
                     "\",\"tmsp\":" + (String)(unixtime() ) +
                     ",\"tp\":" + (String)currentTemp +
                     ",\"h\":" + (String)currentHumi +
                     ",\"E\":" + (String)locationE +
                     ",\"N\":" + (String)locationN +
                     "}]}";
    f.println(strtemp);
//    Serial.println("ADD:" + strtemp);
    f.close();
  }
}

void alFFS_readlist()
{
  File f = SPIFFS.open("/list.json", FILE_READ);
  // String strtemp;
  // strtemp = f.readString();
  Serial.println("file size:"+(String)f.size());
  Serial.println("read out the file:");
  Serial.print(f.readString());
  Serial.println("Size of json_file :" + (String)(f.size()) + "B");
  Serial.println("Size of json_file :" + (String)(f.size() / 1024.0) + "KB");
  Serial.println("Size of json_file :" + (String)((f.size() / 1024.0) / 1024.0) + "MB");
}
void alFFS_addlose()
{

    Serial.println("in alffs_lose");
//    //时间日期
    char tempStr[18];
    char tempStrtemplate[] = "%d%02d%02d %02d:%02d:%02d";
    snprintf(tempStr, sizeof(tempStr), tempStrtemplate, (now1.year+2000),now1.month,now1.day,now1.hour,now1.minute,now1.second);
   Serial.printf("DATE:%d\n",strlen( tempStr));
   sht20getTempAndHumi(); //获取温湿度数据
 

//  Serial.println(tempStr);
 // Serial.print("now the alFFS_thisRec_firstData_flag value is :");
  Serial.printf("firstDdata=%d\n",alFFS_thisRec_firstData_flag);
  if (alFFS_thisRec_firstData_flag)
  {
      Serial.println("first rec, so create a file named:");

    // char tempPathtemplate[] = "/R%d%02d%02d_%02d%02d.json";
    // snprintf(nowREC_filepath, sizeof(nowREC_filepath), tempPathtemplate, now1.year,now1.month,now1.day,now1.hour,now1.minute);
 //   Serial.println(nowREC_filepath);
 //   Serial.println("now first write content to it");
    File f = SPIFFS.open("/lose.json", FILE_WRITE);
    String strtemp ="{\"temp\":{\"value\":"+ (String)currentTemp +
                     ",\"time\":"+ (String)(unixtime() ) +
                     "},\"humi\":{\"value\":"+ (String)currentHumi +
                     ",\"time\":"+ (String)(unixtime() ) +
                     "},\"le\":{\"value\":"+ (String)locationE +
                     ",\"time\":"+ (String)(unixtime() ) +
                     "},\"ln\":{\"value\":"+ (String)locationN +
                     ",\"time\":"+ (String)(unixtime() ) +
                     "}}";
    f.println(strtemp);
   // Serial.println("ADD:" + strtemp);
    f.close();
  }
  else
  {
    Serial.println("not the first rec, so i can just append some content in to the file:");
    
      File f = SPIFFS.open("/lose.json", FILE_APPEND);
    String strtemp ="{\"temp\":{\"value\":"+ (String)currentTemp +
                     ",\"time\":"+ (String)(unixtime() ) +
                     "},\"humi\":{\"value\":"+ (String)currentHumi +
                     ",\"time\":"+ (String)(unixtime() ) +
                     "},\"le\":{\"value\":"+ (String)locationE +
                     ",\"time\":"+ (String)(unixtime() ) +
                     "},\"ln\":{\"value\":"+ (String)locationN +
                     ",\"time\":"+ (String)(unixtime() ) +
                     "}}";
    f.println(strtemp);
   // Serial.println("ADD:" + strtemp);
    f.close();
  }

}
void alFFS_readlose()
{
  File f = SPIFFS.open("/lose.json", FILE_READ);
  Serial.println("lose file size:"+(String)f.size());
  Serial.print(f.readString());
}




void alFFS_endRec()
{
  char tempStr[15];
  char tempStrtemplate[] = "%d%02d%02d %02d:%02d";
  snprintf(tempStr, 15, tempStrtemplate, now1.year,now1.month,now1.day,now1.hour,now1.minute);
  Serial.print("DATE:");
  Serial.println(tempStr);
  File f = SPIFFS.open(nowREC_filepath, FILE_APPEND);
  String strtemp = "],\"et\":\"" + (String)tempStr + "\"}";
  f.println(strtemp);
  Serial.println("ADD:" + strtemp);
  f.close();
  //alFFS_readRecing();
}



//json格式:
// String strtemp = {
//                       "st": "1",
//                       "data": 
//                       [
//                         {
//                           "tm":" 2",
//                           "tmsp": 3,
//                           "tp": 4,
//                           "h": 5,
//                           "E": 6,
//                           "N": 7
//                         },
//                         {
//                           "tm":"1",
//                           "tmsp": 2 ,
//                           "tp": 3 ,
//                           "h": 4 ,
//                           "E": 5 ,
//                           "N": 6 
//                         }



/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true
//显示所有文件
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}
//读文件
void readFile(fs::FS &fs, const char * path){
    //Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }
    int s = file.size();
    char aa[s];
    Serial.printf("Size=%d\r\n", s);
    //读取index.html的文本内容
    String data = file.readString();
    data.toCharArray(aa,s+1);
    Serial.printf(aa);
    Serial.printf("]}");

   // Serial.println(data);
   // Serial.println("- read from file:");
    // while(file.available()){
    //     Serial.write(file.read());
    // }
    file.close();
}
//写文件
int writeFile(fs::FS &fs, const char * path, String message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return 0;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    int i=file.size();
    file.close();
    return i;
}
//添写文件
void appendFile(fs::FS &fs, const char * path, String message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}
//文件重命名
void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("- file renamed");
    } else {
        Serial.println("- rename failed");
    }
}
//删除文件
void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}
//测试函数
void testFileIO(fs::FS &fs, const char * path){
    Serial.printf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }

    size_t i;
    Serial.print("- writing" );
    uint32_t start = millis();
    for(i=0; i<2048; i++){
        if ((i & 0x001F) == 0x001F){
          Serial.print(".");
        }
        file.write(buf, 512);
    }
    Serial.println("");
    uint32_t end = millis() - start;
    Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
    file.close();

    file = fs.open(path);
    start = millis();
    end = start;
    i = 0;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        start = millis();
        Serial.print("- reading" );
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F){
              Serial.print(".");
            }
            len -= toRead;
        }
        Serial.println("");
        end = millis() - start;
        Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
        file.close();
    } else {
        Serial.println("- failed to open file for reading");
    }
}

// void setup(){
//     Serial.begin(115200);
//     if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
//         Serial.println("SPIFFS Mount Failed");
//         return;
//     }
    
//     listDir(SPIFFS, "/", 0);
//     writeFile(SPIFFS, "/hello.txt", "Hello ");
//     appendFile(SPIFFS, "/hello.txt", "World!\r\n");
//     readFile(SPIFFS, "/hello.txt");
//     renameFile(SPIFFS, "/hello.txt", "/foo.txt");
//     readFile(SPIFFS, "/foo.txt");
//     deleteFile(SPIFFS, "/foo.txt");
//     testFileIO(SPIFFS, "/test.txt");
//     deleteFile(SPIFFS, "/test.txt");
//     Serial.println( "Test complete" );
// }

// void loop(){

// }
void alFFS_Writelist(bool x)//写正常记录文件
{
   if(!x)//第一次写清除数据发送JSON头，同时能得到占用空间
   {
     uint32_t lenght;
     String txt="{\"id\": \"1\",\"params\":[{\"temp\": { \"value\":"+(String)currentTemp+
     "},\"humi\": {\"value\":"+(String)currentHumi+
     "},\"le\": {\"value\":" +(String)locationE+
     "},\"ln\": {\"value\":" +(String)locationN+
     "},\"time\": {\"value\":" +(String)now_unixtime+
     "}}";     
   
    lenght=writeFile(SPIFFS, "/list.json", txt);
    Serial.printf("TXT size:%d",lenght);
    readFile(SPIFFS, "/list.json");
   }
   else//否则添加文件
   {
     String txt = ",{\"temp\": { \"value\":"+(String)currentTemp+
                  "},\"humi\": {\"value\":" +(String)currentHumi+
                      "},\"le\": {\"value\":" +(String)locationE+
                      "},\"ln\": {\"value\": "+(String)locationN+
                  "},\"time\": {\"value\":"+(String)now_unixtime+
                  "}}";       
     //] }
    appendFile(SPIFFS, "/list.json", txt);
    readFile(SPIFFS, "/list.json");
   }
   
}

