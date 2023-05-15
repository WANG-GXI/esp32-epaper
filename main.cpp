// Display Library example for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Display Library based on Demo Example from Good Display: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2
//
// Purpose: show uses of GxEPD2_GFX base class for references to a display instance

// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Display ePaper for Arduino: https://forum.arduino.cc/index.php?topic=436411.0

// see GxEPD2_wiring_examples.h for wiring suggestions and examples

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 1

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>
// !!!!  ============================================================================================ !!!!
#include "BitmapDisplay.h"
#include <GxEPD2_BW.h>
// #include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include "bitmaps/Bitmaps200x200.h" // 1.54" b/w
#include "TextDisplay.h"
// select the display constructor line in one of the following files (old style):
#include "GxEPD2_display_selection.h"
#include "GxEPD2_display_selection_added.h"
//#include "GxEPD2_display_selection_more.h" // private
// or select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"
#include <U8g2_for_Adafruit_GFX.h>
#include "gb2312.c"
#include "WiFi.h"
#include <ArduinoJson.h>

#define baise  GxEPD_WHITE  //白色
#define heise  GxEPD_BLACK  //黑色
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;
int currentHour = 0,lastcurrentHour=0;                 //时
int currentMinute = 0,lastcurrentMinute=0;               //分
int time_cnt = 0;               //分
String timedata="";
const int httpPort = 80; //端口号
const char* host = "api.seniverse.com"; //服务器地址
String reqUserKey = "SymMAw7VsJE2mire4";//知心天气API私钥
String reqLocation = "天津";//地址
String reqUnit = "c";//摄氏度
//-------------------http请求-----------------------------//
String reqRes = "/v3/weather/now.json?key=" + reqUserKey +
                + "&location=" + reqLocation +
                "&language=en&unit=" + reqUnit;
String httprequest = String("GET ") + reqRes + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Connection: close\r\n\r\n";
//--------------------------------------------------------//
BitmapDisplay bitmaps(display);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
void showPartialUpdate();
void showpicture();
void printLocalTime();
void showtime(GxEPD2_GFX& display);
void display_partialLine(uint8_t line, String zf);
void BWClearScreen();
void httpRequest();
void parseJson(WiFiClient client);
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  WiFi.mode(WIFI_STA);//开启网络 
  WiFi.begin("GXI","66666666");//填写自己的wifi账号密码
  while (WiFi.status() != WL_CONNECTED) 
  {
      Serial.print(".");
      vTaskDelay(200);
  }
  delay(100);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.setRotation(1);      // 设置旋转方向 0-0° 1-90° 2-180° 3-270°
  u8g2Fonts.begin(display);             // 将u8g2过程连接到Adafruit GFX
  u8g2Fonts.setFontMode(1);             // 使用u8g2透明模式（这是默认设置）
  u8g2Fonts.setFontDirection(0);        // 从左到右（这是默认设置）
  u8g2Fonts.setForegroundColor(heise);  // 设置前景色
  u8g2Fonts.setBackgroundColor(baise);  // 设置背景色
  u8g2Fonts.setFont(chinese_gb2312);    // 设置字体
  // first update should be full refresh
  showpicture();
  httpRequest();
}

void loop()
{
  printLocalTime();
}

void showpicture()
{ 
  display.fillScreen(GxEPD_WHITE);
  display.display(1);         // 显示缓冲内容到屏幕，用于全屏缓冲
  display.drawInvertedBitmap(0, 0, gImage_134, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
  display.display(1);  
  
}
void printLocalTime()
{
  struct tm timeinfo;
  unsigned char testcode[4];
  timedata="";
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  lastcurrentHour=currentHour;
  lastcurrentMinute=currentMinute;
  currentHour=timeinfo.tm_hour;
  currentMinute=timeinfo.tm_min;
  if(lastcurrentHour!=currentHour ||lastcurrentMinute!=currentMinute )
  {
  if(currentHour<10)timedata+=String("0");
  timedata+=String(currentHour);
  timedata+=String(":");
  if(currentMinute<10)timedata+=String("0");
  timedata+=String(currentMinute);
  display_partialLine(6,timedata);
  }
}

void display_partialLine(uint8_t line, String zf) ////发送局部刷新的显示信息到屏幕,带居中
{
  /*
    display_partialLine()
    发送局部刷新的显示信息到屏幕,带居中

    line        行数（0-7）
    zf          字符内容
    lineRefresh 整行刷新 1-是 0-仅刷新字符长度的区域
  */
  u8g2Fonts.setFont(u8g2_font_7Segments_26x42_mn);
   u8g2Fonts.setFontMode(1);
  // display.init(0, 0, 10, 1);
  const char *character = zf.c_str();                            //String转换char
  uint16_t zf_width = u8g2Fonts.getUTF8Width(character);         //获取字符的像素长度
  uint16_t x = (display.width() / 2) - (zf_width / 2);           //计算字符居中的X坐标（屏幕宽度/2-字符宽度/2）
  display.setPartialWindow(0, 125-68, display.width(), 68);   //整行刷新
  display.firstPage();
  do
  {
    u8g2Fonts.setCursor(x, 125);
    u8g2Fonts.print(character);
    //display.drawInvertedBitmap(80, 30, , 45, 45, heise);
  }
  while (display.nextPage());
  //display.powerOff(); //关闭屏幕电源
}
void BWClearScreen()
{
  //display.init(0, 0, 10, 1);   //串口使能 初始化完全刷新使能 复位时间 ret上拉使能
  display.setPartialWindow(0, 0, display.width(), display.height()); //设置局部刷新窗口
  display.firstPage();
  do
  {
    display.fillScreen(heise);  // 填充屏幕
    display.display(1);         // 显示缓冲内容到屏幕，用于全屏缓冲
    display.fillScreen(baise);  // 填充屏幕
    display.display(1);         // 显示缓冲内容到屏幕，用于全屏缓冲
  }
  while (display.nextPage());
}
void parseJson(WiFiClient client) {
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 230;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, client);

  JsonObject obj1 = doc["results"][0];
  String cityName = obj1["location"]["name"].as<String>();
  String weather = obj1["now"]["text"].as<String>();
  String code = obj1["now"]["code"].as<String>();
  String temperature = obj1["now"]["temperature"].as<String>()+"℃";
  String zf= "天津";
  int code_int = obj1["now"]["code"].as<int>();
  Serial.println(cityName);
  //Serial.println(code);
  Serial.println(weather);
  Serial.println(temperature);
  const char *character = zf.c_str(); 
  const char *character_temperature = temperature.c_str(); 
  u8g2Fonts.setFont(chinese_gb2312);
  display.setPartialWindow(160, 10, 50, 35);   //整行刷新
  display.firstPage();
  do
  {
    u8g2Fonts.setCursor(160, 20);
    u8g2Fonts.print(character);
    u8g2Fonts.setCursor(160,45);
    u8g2Fonts.print(character_temperature);
    //display.drawInvertedBitmap(80, 30, , 45, 45, heise);
  }
  while (display.nextPage());
}
void httpRequest() {
  WiFiClient client;
  //1 连接服务器
  if (client.connect(host, httpPort)) {
    Serial.println("连接成功，接下来发送请求");
    client.print(httprequest);//访问API接口
    String response_status = client.readStringUntil('\n');
    Serial.println(response_status);

    if (client.find("\r\n\r\n")) {
      Serial.println("响应报文体找到，开始解析");
    }
    parseJson(client);
  }
  else {
    Serial.println("连接服务器失败");
  }
  client.stop();
}

