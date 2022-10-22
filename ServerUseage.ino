#include <ESP8266WiFi.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include <iostream>
#include <string.h>

const char* ssid = "Network";
const char* password = "12345689";
const char* mqttServer = "api.ikanned.com";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

Adafruit_SSD1306 oled(128, 64, &Wire,-1);

void setup() {
  // put your setup code here, to run once:
  delay(200);
  pinMode(D3,OUTPUT);
  delay(500);
  digitalWrite(D3,LOW);
  pinMode(LED_BUILTIN, OUTPUT);     // 设置板上LED引脚为输出模式
  digitalWrite(LED_BUILTIN, HIGH);  // 启动后关闭LED
  Serial.begin(9600);   //开启串口通讯
  WiFi.mode(WIFI_STA);  //设置终端模式
  connectWifi();  // 连接WiFi
  
  mqttClient.setServer(mqttServer, 1884);  // 设置MQTT服务器和端口号
  mqttClient.setCallback(receiveCallback);  // 设置MQTT订阅回调函数
  connectMQTTserver();  // 连接MQTT服务器
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(500);
  //digitalWrite(D3,LOW);
  if (mqttClient.connected()) { // 如果开发板成功连接服务器
    mqttClient.loop();          // 处理信息以及心跳
  } else {    // 如果开发板未能成功连接服务器
    connectMQTTserver();   // 重新连接    
  }

}

void connectMQTTserver(){
  // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
  String clientId = "esp8266-" + WiFi.macAddress();
 
  // 连接MQTT服务器
  if (mqttClient.connect(clientId.c_str())) { 
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address:");
    Serial.println(mqttServer);
    Serial.println("ClientId: ");
    Serial.println(clientId);
    subscribeTopic(); // 订阅指定主题
  } else {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    delay(5000);
  }   
}
 
// 收到信息后的回调函数
void receiveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");

  String s;
  String cpu_useage;
  String mem_useage;
  
  int i;
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    
    s += (char)payload[i]; 
    
    if (i <= 4){ //分离出CPU使用率
      cpu_useage += (char)payload[i]; //char -> String
      }
      else
      {
        mem_useage +=(char)payload[i];
        }
  }
  Serial.println("  " + cpu_useage + mem_useage);
  int cpu_useage_int = atoi(cpu_useage.c_str()); //CPU使用率的String转int
  int mem_useage_int = atoi(mem_useage.c_str()); //Memery使用率的String转int
  //Serial.println("   Cpu: ");
  //Serial.println(cpu_useage_int);

  if(cpu_useage_int >= 90){
    digitalWrite(D3, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("LED ON");
    delay(500);
    digitalWrite(D3, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    }

  int countCPUTime = countHighUseage();
  Serial.println(countCPUTime);
    
  Serial.println("\n"+cpu_useage_int);
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);
  
  oled.begin(SSD1306_SWITCHCAPVCC,0x3C);
  oled.setTextColor(WHITE);//开像素点发光
  oled.clearDisplay();//清屏

  oled.setTextSize(2);//设置字体大小  
  oled.setCursor(5, 10);//设置显示位置
  oled.println("Cpu: ");

  oled.setTextSize(2);//设置字体大小  
  oled.setCursor(5, 38);//设置显示位置
  oled.println("Mem: ");

  oled.setTextSize(2);//设置字体大小  
  oled.setCursor(55, 10);//设置显示位置
  oled.println(cpu_useage);

  oled.setTextSize(2);//设置字体大小  
  oled.setCursor(55, 38);//设置显示位置
  oled.println(mem_useage_int);
  
  oled.display(); //开启显示
  delay(200);
 
}
 
// 订阅指定主题
void subscribeTopic(){
 
  // 建立订阅主题。主题名称ss,防止混乱
  String topicString = "ss";
  char subTopic[topicString.length() + 1];  
  strcpy(subTopic, topicString.c_str());
  
  // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
  if(mqttClient.subscribe(subTopic)){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic);
  } else {
    Serial.print("Subscribe Fail...");
  }  
}
 
// ESP8266连接wifi
void connectWifi(){
 
  WiFi.begin(ssid, password);
 
  //等待WiFi连接,成功连接后输出成功信息
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");  
}

int countHighUseage(){
  int time;
  time ++;
  return time;
  }
