#include <Wire.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

const char* ssid = "Network";
const char* password = "12345689";

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);
//unsigned long delayTime;

#define TOKEN "KANNA_ESP_TOKEN_TEMPHUM" //Access token of device Display
char ThingsboardHost[] = "api.ikanned.com";

WiFiClient wifiClient; //创建对象wifiClient
PubSubClient client(wifiClient);
int status = WL_IDLE_STATUS;

#define DHTTYPE DHT11 //本次选择为DHT11传感器
//#define DHTTYPE DHT21 // DHT 21 (AM2301)
//#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321
uint8_t DHTPin = 12; //GPIO引脚12，等于D6
DHT dht(DHTPin, DHTTYPE); //设置DHT传感器的信号输出引脚
float Temperature;
float Humidity;
float Temp_Fahrenheit;
String HumidifierStatus;
int num = 0;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT); //将板载LED口作为输出口
  digitalWrite(LED_BUILTIN, HIGH); //开机关闭LED
  pinMode(D3, OUTPUT); //将板载D3接口作为输出口，用于继电器连接
  digitalWrite(D3, LOW); //开机关闭继电器
  pinMode(DHTPin, INPUT); //将连接DHT11的数据口作为输入口

  dht.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //初始化IIC使用地址x03c，适用于128x64
  //完成初始化
  //display.display();
  delay(100);
  display.clearDisplay();
  display.setTextSize(1); //设置字体大小，下同
  display.setCursor(0,0); //设置显示位置，数值为左上角X,Y坐标
  display.setTextColor(WHITE);
  display.println("Connecting Wifi: ");
  display.setTextSize(1); //设置字体大小，下同
  display.setCursor(0,18); //设置显示位置，数值为左上角X,Y坐标
  display.print(ssid);
  display.display();

  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("connected to");
  Serial.println(ssid);
  client.setServer( ThingsboardHost, 1883 ); //端口1833

}
void loop() 
{
  delay(100);
  Humidity = dht.readHumidity(); //读取湿度
  Temperature = dht.readTemperature(); //将温度读取为摄氏度
  Temp_Fahrenheit= dht.readTemperature(true); //将温度读取为华氏度

  if(Humidity < 45 && num ==0)
  {
    num += 201;
  }

  if(num != 0)
  {
    digitalWrite(LED_BUILTIN, LOW); //低电平为打开板载LED
    digitalWrite(D3, HIGH); //湿度低于45打开加湿器
    HumidifierStatus = "工作中";
    num--;
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH); //低电平为打开板载LED
    digitalWrite(D3, LOW); //湿度低于45打开加湿器
    HumidifierStatus = "暂停中";
  }

  if (isnan(Humidity) || isnan(Temperature) || isnan(Temp_Fahrenheit)) { //检查传感器是否连接&正常工作
    Serial.println(F("Failed to read from DHT sensor!")); //出错则向串口打印错误信息
    return;
  }

    if ( !client.connected() ) //如果没有连接上服务器
  {
    reconnect(); //重新连接
  }
  getAndSendTemperatureAndHumidityData(); //调用函数处理发送数据到ThingsBoard
  //delay(5000);

  Serial.print(F("Humidity: "));
  Serial.print(Humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(Temperature);
  Serial.print(F("°C "));
  Serial.print(Temp_Fahrenheit);
  Serial.println(F("°F "));

  display.setCursor(0,0);
  display.clearDisplay(); //清屏

  //在第一行开始显示温度
  display.setTextSize(1); //设置字体大小，下同
  display.setCursor(0,0); //设置显示位置，数值为左上角X,Y坐标
  display.print("Temperature: ");

  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(Temperature);
  display.print(" ");

  display.setTextSize(1); //设置字体大小为1来显示o
  display.cp437(true); //为了显示摄氏度单位符号，启用Code Page437 字符
  display.write(167); //左上角的o在Page437中对应于字符167
  display.setTextSize(2); //设置字体大小为1来显示C
  display.print("C");
  
  //在第二行开始显示湿度
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity: ");

  display.setTextSize(1);
  display.setCursor(100, 35);
  display.print(num);

  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(Humidity);
  display.print(" %");
  
  display.display(); //打开显示
  delay(2900); //延时2.9s 加上开始的0.1s，延时3s显示并发送一次
}

void getAndSendTemperatureAndHumidityData()
{
  //JSON
  String payload = "{";  //发送格式开始为{
  payload += "\"Humidity\":";payload += Humidity; payload += ","; //将湿度数据放入payload中
  payload += "\"HumidifierStatus\":";payload += HumidifierStatus; payload += ","; //将湿度数据放入payload中
  payload += "\"Temperature\":";payload += Temperature; //将温度数据放入payload中
  payload += "}"; //发送格式末尾为}
  char attributes[1000];
  payload.toCharArray( attributes, 1000 ); //将payload中的信息放入sttributes
  client.publish( "v1/devices/me/telemetry",attributes);
  Serial.println( attributes ); 
}

void reconnect() {  //除非连接成功，否则一直连接
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print("."); //每间隔500ms向串口打印.......
      }
      Serial.println("Connected to AP");
    }

    Serial.print("Connecting to ThingsBoard node ..."); //尝试连接ThingsBoard节点
    if ( client.connect("Esp8266", TOKEN, NULL) ) { //以提供的服务器地址和TOKEN连接服务器
      Serial.println( "[DONE]" ); //显示连接完成

      //display.clearDisplay();
      display.clearDisplay();
      display.setTextSize(1); //设置字体大小，下同
      display.setCursor(0,0); //设置显示位置，数值为左上角X,Y坐标
      display.setTextColor(WHITE);
      display.println("Server connected");
      display.display();
      delay(1500);

    }
    else
    {
      Serial.print( "[FAILED] [ rc = " );
      Serial.println( " : retrying in 5 seconds]" );
      delay( 500 );
    }
  }
}
