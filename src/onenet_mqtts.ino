#include "config.h"

/*-------------------------------连接平台-------------------------------*/
void onenet_connect()
{
  //连接OneNet并上传数据
  Serial.println("connecting to OneNet IOT...");
  client.setServer(mqtt_server, port);                   //设置客户端连接的服务器,连接Onenet服务器, 使用6002端口
  client.connect(mqtt_devid, mqtt_pubid, mqtt_password); //客户端连接到指定的产品的指定设备.同时输入鉴权信息
}
/*-------------------------------向云平台发送温湿度数据-------------------------------*/
void sendTempAndHumi()
{
  
  if (client.connected())
  {
    //拼接主题
    char subscribeTopic[75];//订阅主题
    char topicTemplate[] = "$sys/%s/%s/cmd/request/#"; //信息模板
    snprintf(subscribeTopic, sizeof(subscribeTopic), topicTemplate, mqtt_pubid, mqtt_devid);
    client.subscribe(subscribeTopic); //订阅命令下发主题
    Serial.println("pinjiezhuti ok");
    
    //先拼接出json字符串
    char param[256];
    char jsonBuf[256];
    //char gs[]="{\"temp\":{\"value\":%.2f},\"humi\":{\"value\":%.2f},\"le\":{\"value\":%.2f},\"ln\":{\"value\":%.2f},\"last_time\":{\"value\":%u000}}";
    char gs1[]="{\"temp\":{\"value\":%.2f,\"time\":%u000},\"humi\":{\"value\":%.2f,\"time\":%u000},\"le\":{\"value\":%.2f,\"time\":%u000},\"ln\":{\"value\":%.2f,\"time\":%u000}}";
    //下面需要确定一下发送格式 2021-4-20 20:53:58
    //sprintf(param, gs, currentTemp, currentHumi,  locationE, locationN,now_unixtime);
    sprintf(param, gs1, currentTemp,now_unixtime, currentHumi,now_unixtime,  locationE,now_unixtime, locationN,now_unixtime);

    sprintf(jsonBuf, ONENET_POST_BODY_FORMAT,param);
    Serial.println("zhengli  data! ok");
    //打开可以验证数组的大小和内容
    // #if  1
    //     Serial.printf("param:%d\n",strnlen(param,256)); 
    //     Serial.printf("jsonBuf:%d\n",strnlen(jsonBuf,256)); 
    //     //Serial.println(param); 
    //     Serial.println(jsonBuf);
    //     #endif 
   //下面直接验证发送的内容 
   // char gs2[]="{\"temp\":{\"value\":22.22},\"humi\":{\"value\":45.45},\"le\":{\"value\":117.09},\"ln\":{\"value\":36.11},\"start_time\":{\"value\":1618929513000}}";
   // char gs2[]="{\"temp\":{\"value\":45.22,\"time\":1618929513000},\"humi\":{\"value\":55.45,\"time\":1618929513000},\"le\":{\"value\":117.09,\"time\":1618929513000},\"ln\":{\"value\":36.11,\"time\":1618929513000}}";
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

    digitalWrite(MODEM_POWER_ON, LOW);
    // snprintf(msgJson, 256, dataTemplate, currentTemp, currentHumi, locationE, locationN); //将模拟温湿度数据套入dataTemplate模板中, 生成的字符串传给msgJson
    // Serial.print("public the data:");
    // Serial.println(msgJson);
    // char publicTopic[75];
    // char topicTemplate[] = "$sys/%s/%s/thing/property/post"; //主题模板
    // snprintf(publicTopic, 75, topicTemplate, mqtt_pubid, mqtt_devid);
    // Serial.println("publicTopic");
    // Serial.println(publicTopic);
    // if (client.publish(publicTopic, (uint8_t *)msgJson, strlen(msgJson)))
    // {
    //   Serial.print("Post message to cloud: ");
    //   Serial.println(msgJson);
    // }

    // Serial.println("Publish message to cloud failed!");
  }
}