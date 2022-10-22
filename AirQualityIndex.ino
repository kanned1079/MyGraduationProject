#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQ135.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

Adafruit_SSD1306 display(128, 32, &Wire, -1);
const char* ssid = "Network";
const char* password = "12345689";

char ThingsboardHost[] = "api.ikanned.com";a
#define TOKEN "KANNA_ESP_TOKEN_AIR" //Access token of device Display


WiFiClient wifiClient; //创建对象wifiClient
PubSubClient client(wifiClient);
int status = WL_IDLE_STATUS;
float air_quality;
String payload;

void setup()
{
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();
  delay(10);

  display.setTextSize(1); //设置字体大小，下同
  display.setCursor(0,0); //设置显示位置，数值为左上角X,Y坐标
  display.setTextColor(WHITE);
  display.println("Connecting Wifi: ");
  display.setTextSize(1); //设置字体大小，下同
  display.setCursor(0,18); //设置显示位置，数值为左上角X,Y坐标
  display.print(ssid);
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {  //Wifi没有连接
    delay(200);
    Serial.print(".");
    
    
    display.setTextSize(1); //设置字体大小，下同
    display.setCursor(0,0); //设置显示位置，数值为左上角X,Y坐标
    display.println("Connecting Wifi: ");
    display.setTextSize(1); //设置字体大小，下同
    display.setCursor(0,18); //设置显示位置，数值为左上角X,Y坐标
    display.print(ssid);
    display.display();
  }
  Serial.println("");
  Serial.print("connected to");
  Serial.println(ssid);
  
  //delay(1000);
  client.setServer( ThingsboardHost, 1883 );
  //MQ135 gasSensor = MQ135(A0);
}

void loop()
{
  if ( !client.connected() ) 
  {
    reconnect();
  }

  GetAndSendData();
  delay(2000);

}

void GetAndSendData()
{
  MQ135 gasSensor = MQ135(A0);
  air_quality = gasSensor.getPPM();
  
  payload = "{";
  payload += "\"AirQuality\":";payload += air_quality; 
  payload += "}";

  char attributes[1000];
  payload.toCharArray( attributes, 1000 );
  client.publish( "v1/devices/me/telemetry",attributes);
  Serial.println( attributes );  

  display.clearDisplay();
  display.setCursor(0,0);  //oled display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Air Quality Index");

  display.setCursor(0,16);  //oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(air_quality);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(" PPM");
  display.display();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
      display.clearDisplay();
      display.setTextSize(1); //设置字体大小，下同
      display.setCursor(0,0); //设置显示位置，数值为左上角X,Y坐标
      display.println("Connecting Wifi: ");
      display.setTextSize(1); //设置字体大小，下同
      display.setCursor(0,18); //设置显示位置，数值为左上角X,Y坐标
      display.print(ssid);
      display.display();
    }              
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("Esp8266", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
      display.clearDisplay();
      display.setTextSize(1); //设置字体大小，下同
      display.setCursor(0,0); //设置显示位置，数值为左上角X,Y坐标
      display.println("Server connected");
      display.display();
      delay(2000);
      display.clearDisplay();      

    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.println( " : retrying in 5 seconds]" );
      delay( 500 );
      display.clearDisplay();
      display.setTextSize(1); //设置字体大小，下同
      display.setCursor(0,0); //设置显示位置，数值为左上角X,Y坐标
      display.println("Connecting server:");
      display.setTextSize(1); //设置字体大小，下同
      display.setCursor(0,18); //设置显示位置，数值为左上角X,Y坐标
      display.print(ThingsboardHost);
      display.display();
    }
  }
}
