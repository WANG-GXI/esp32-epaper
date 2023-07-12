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
#include "image.h"
#include <WebServer.h>

#define baise  GxEPD_WHITE  //白色
#define heise  GxEPD_BLACK  //黑色
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;
const char* apSSID = "ESP32_AP"; // 设置ESP32的热点名称
const char* apPassword = "password"; // 设置ESP32的热点密码
const int daylightOffset_sec = 0;
int currentHour = 0,lastcurrentHour=0;                 //时
int currentMinute = 0,lastcurrentMinute=0,weekend;               //分
int time_cnt = 0,time_add=0,wifi_time_flag=0;           //分
String timedata="";
const int httpPort = 80; //端口号
const char* host = "api.seniverse.com"; //服务器地址
String reqUserKey = "SymMAw7VsJE2mire4";//知心天气API私钥
String reqLocation = "太原";//地址
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
WebServer  server(80);
void showPartialUpdate();
void showpicture();
void printLocalTime();
void showtime(GxEPD2_GFX& display);
void display_partialLine(uint8_t line, String zf);
void BWClearScreen();
void httpRequest();
void parseJson(WiFiClient client);
void handleConfig();
void handleRoot();
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  // 配置ESP32为AP模式
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, apPassword);
  // 设置处理根路径和配置路径的回调函数
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  // 启动Web服务器
  server.begin();
  // WiFi.mode(WIFI_STA);//开启网络 
  // WiFi.begin("GXI","66666667");//填写自己的wifi账号密码
  // while (WiFi.status() != WL_CONNECTED) 
  // {
  //     Serial.print(".");
  //     vTaskDelay(200);
  // }
  // delay(100);
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
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
  // printLocalTime();
  // httpRequest();
}

void loop()
{
  //printLocalTime();
  // 处理Web请求
  server.handleClient();
  if(wifi_time_flag==1)
  {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    wifi_time_flag=2;
  }
  if(wifi_time_flag==2) printLocalTime();
}

void showpicture()
{ 
  // 生成一个 0 到 5 之间的随机数
  int randomIndex = random(6);
  String randomPoem;
  switch (randomIndex) {
    case 0:
      randomPoem = poem1;
      break;
    case 1:
      randomPoem = poem2;
      break;
    case 2:
      randomPoem = poem3;
      break;
    case 3:
      randomPoem = poem4;
      break;
    case 4:
      randomPoem = poem5;
      break;
    case 5:
      randomPoem = poem6;
      break;
    default:
      randomPoem = "未知诗句";
      break;
  }
  display.fillScreen(GxEPD_WHITE);
  display.display(1);         // 显示缓冲内容到屏幕，用于全屏缓冲
  display.drawInvertedBitmap(0, 0, gImage_Todolist, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
  display.display(1);  //gImage_123
  display.setPartialWindow(0, 0, display.width(), 30);   //整行刷新
  display.firstPage();
  do
  {
    u8g2Fonts.setCursor(0, 23);
    u8g2Fonts.print(randomPoem.c_str());
  }
  while (display.nextPage());
  
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
  weekend = timeinfo.tm_wday;
  if(lastcurrentHour!=currentHour ||lastcurrentMinute!=currentMinute ) //只要时或者分发生变化
  {
  if(currentHour<10)timedata+=String("0");
  timedata+=String(currentHour);
  timedata+=String(":");
  if(currentMinute<10)timedata+=String("0");
  timedata+=String(currentMinute);
  //display_partialLine(6,timedata);
  }
  if(lastcurrentMinute!=currentMinute ) //只要分发生变化
  {
  time_add++;
  u8g2Fonts.setFont(u8g2_font_timB18_tn);
  u8g2Fonts.setFontMode(1);
  display.setPartialWindow(120, 172, 30, 30);   //整行刷新
  display.firstPage();
  do
  {
    u8g2Fonts.setCursor(122, 189);
    u8g2Fonts.print(time_add);
  }
  while (display.nextPage());
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
  String zf= "太原";
  int code_int = obj1["now"]["code"].as<int>();
  Serial.println(cityName);
  //Serial.println(code);
  Serial.println(weather);
  Serial.println(temperature);
  const char *character = zf.c_str(); 
  const char *character_temperature = temperature.c_str(); 
  u8g2Fonts.setFont(chinese_gb2312);
  // display.setPartialWindow(160, 10, 50, 35);   //整行刷新
  display.setPartialWindow(77, 10, 135, 38);   //整行刷新
  display.firstPage();
  do
  {
    u8g2Fonts.setCursor(160, 20);
    u8g2Fonts.print(character);
    u8g2Fonts.setCursor(160,45);
    u8g2Fonts.print(character_temperature);
    switch (weekend)
    {
    case 1:
      display.drawInvertedBitmap(78, 30,gImage_weekend1,70, 23, GxEPD_BLACK);
      break;
     case 2:
      display.drawInvertedBitmap(78, 30,gImage_weekend2,70, 23, GxEPD_BLACK);
      break;
     case 3:
      display.drawInvertedBitmap(78, 30,gImage_weekend3,70, 23, GxEPD_BLACK);
      break;
     case 4:
      display.drawInvertedBitmap(78, 30,gImage_weekend4,70, 23, GxEPD_BLACK);
      break;
     case 5:
      display.drawInvertedBitmap(78, 30,gImage_weekend5,70, 23, GxEPD_BLACK);
      break;
     case 6:
      display.drawInvertedBitmap(78, 30,gImage_weekend6,70, 23, GxEPD_BLACK);
      break;
     case 0:
      display.drawInvertedBitmap(78, 30,gImage_weekend7,70, 23, GxEPD_BLACK);
      break;
    default:
      break;
    }

    //display.display(1);  
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
void handleRoot() {
  String htmlContent = "<!DOCTYPE html>";
  htmlContent += "<html>";
  htmlContent += "<head>";
  htmlContent += "<meta charset=\"UTF-8\">";
  htmlContent += "<title>ESP32配置页面</title>";
  htmlContent += "</head>";
  htmlContent += "<body>";
  htmlContent += "<h1>ESP32配置页面</h1>";
  htmlContent += "<form action=\"/config\" method=\"post\">";
  htmlContent += "<label for=\"ssid\">Wi-Fi名称：</label>";
  htmlContent += "<input type=\"text\" id=\"ssid\" name=\"ssid\" placeholder=\"请输入Wi-Fi名称\"><br><br>";
  htmlContent += "<label for=\"password\">Wi-Fi密码：</label>";
  htmlContent += "<input type=\"password\" id=\"password\" name=\"password\" placeholder=\"请输入Wi-Fi密码\"><br><br>";
  
  htmlContent += "<label for=\"plan1\">每日计划1：</label>";
  htmlContent += "<input type=\"plan1\" id=\"plan1\" name=\"plan1\" placeholder=\"请输入每日计划1\"><br><br>";
  htmlContent += "<label for=\"plan2\">每日计划2：</label>";
  htmlContent += "<input type=\"plan2\" id=\"plan2\" name=\"plan2\" placeholder=\"请输入每日计划2\"><br><br>";
  htmlContent += "<label for=\"plan3\">每日计划3：</label>";
  htmlContent += "<input type=\"plan3\" id=\"plan3\" name=\"plan3\" placeholder=\"请输入每日计划3\"><br><br>";
  htmlContent += "<label for=\"plan4\">每日计划4：</label>";
  htmlContent += "<input type=\"plan4\" id=\"plan4\" name=\"plan4\" placeholder=\"请输入每日计划4\"><br><br>";
  htmlContent += "<input type=\"submit\" value=\"提交\">";
  htmlContent += "</form>";
  htmlContent += "</body>";
  htmlContent += "</html>";
  server.send(200, "text/html", htmlContent);
}
void handleConfig() {
  if (server.method() == HTTP_POST) {
    // 读取请求体中的数据
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String plan1 = server.arg("plan1").c_str();
    String plan2 = server.arg("plan2").c_str();
    String plan3 = server.arg("plan3").c_str();
    String plan4 = server.arg("plan4").c_str();
    // 在这里进行相应的处理，如连接到指定的Wi-Fi网络等
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    u8g2Fonts.setFont(chinese_gb2312);
    display.setPartialWindow(40, 50, 150, 100);   //整行刷新
    display.firstPage();
    do
    {
      u8g2Fonts.setCursor(42, 61);
      u8g2Fonts.print(plan1);
      u8g2Fonts.setCursor(42, 90);
      u8g2Fonts.print(plan2);
      u8g2Fonts.setCursor(42, 116);
      u8g2Fonts.print(plan3);
      u8g2Fonts.setCursor(42, 145);
      u8g2Fonts.print(plan4);
    }
    while (display.nextPage());
    Serial.println(plan1);
    server.send(200, "text/plain", "Config received");
    wifi_time_flag=1;
  } 
  else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}