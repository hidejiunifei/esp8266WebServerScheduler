#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "Gsender.h"
#include <Wire.h>

ESP8266WebServer server(80);
StaticJsonDocument<1200> doc;
JsonArray arrAlarm = doc.to<JsonArray>();
char Time[] = "TIME:  :  :  ";
char Calendar[] = "DATE:  /  /20  ";
bool debug = false;
Gsender *gsender = Gsender::Instance();

void setup(void){
  
  server.on("/", HTTP_GET, handleRoot);
  server.on("/RTC", HTTP_GET, handleGetRTC);
  server.on("/RTC", HTTP_POST, handlePostRTC);
  server.on("/ALM", HTTP_GET, handleGetAlarm);
  server.on("/ALM", HTTP_POST, handlePostAlarm);
  server.on("/DBG", HTTP_GET, handleGetDebug);
  server.on("/DBG", HTTP_POST, handlePostDebug);
  server.onNotFound(handleNotFound);
  
  Wire.begin(0,2);
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
}

void connectWifi()
{
  WiFi.begin("your ssid", "your wifi password");

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  server.begin();
}

void loop(void){
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  server.handleClient();
  String debugStr = "";

  byte minute, hour, day;
  Wire.beginTransmission(0x68);
  Wire.write(0);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 7);
  Wire.read();
  minute = Wire.read();
  hour = Wire.read();
  day = Wire.read();
  Wire.read();
  Wire.read();
  minute = (minute >> 4) * 10 + (minute & 0x0F);
  hour = (hour >> 4) * 10 + (hour & 0x0F);
  day = (day >> 4) * 10 + (day & 0x0F);
  for(int i=arrAlarm.size()-1; i >= 0; i--)
  {
    if (arrAlarm[i]["day"].as<byte>() == day &&
		arrAlarm[i]["hour"].as<byte>() == hour &&
        arrAlarm[i]["minute"].as<byte>() == minute)
    {
      digitalWrite(arrAlarm[i]["pin"].as<int>(), HIGH);
      delay(arrAlarm[i]["delay"].as<long>());
      digitalWrite(arrAlarm[i]["pin"].as<int>(), LOW);
      arrAlarm.remove(i);
    }
  }
  if (debug && debugStr.length() > 0)
    gsender->Subject("loop")->Send("youremail@gmail.com", debugStr);
}

void handleRoot() {
  server.send(200, "text/html", "Available routes GET /RTC POST /RTC GET /ALM POST /ALM GET /DBG POST /DBG");
}

void handleGetRTC() {
  String debugStr = "";

  Wire.beginTransmission(0x68);
  Wire.write(0);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 7);
  byte second, minute, hour, day, month, year;
  second = Wire.read();
  debugStr += "second: " + String(second);
  minute = Wire.read();
  debugStr += " minute:" + String(minute);
  hour = Wire.read();
  debugStr += " hour:" + String(hour);
  Wire.read();
  day = Wire.read();
  debugStr += " day:" + String(day);
  month = Wire.read();
  debugStr += " month:" + String(month);
  year = Wire.read();
  debugStr += " year:" + String(year);
  second = (second >> 4) * 10 + (second & 0x0F);
  debugStr += " " + String(second);
  minute = (minute >> 4) * 10 + (minute & 0x0F);
  debugStr += " " + String(minute);
  hour = (hour >> 4) * 10 + (hour & 0x0F);
  debugStr += " " + String(hour);
  day = (day >> 4) * 10 + (day & 0x0F);
  debugStr += " " + String(day);
  month = (month >> 4) * 10 + (month & 0x0F);
  debugStr += " " + String(month);
  year = (year >> 4) * 10 + (year & 0x0F);
  debugStr += " " + String(year);
  Time[12] = second % 10 + 48;
  Time[11] = second / 10 + 48;
  Time[9] = minute % 10 + 48;
  Time[8] = minute / 10 + 48;
  Time[6] = hour % 10 + 48;
  Time[5] = hour / 10 + 48;
  Calendar[14] = year % 10 + 48;
  Calendar[13] = year / 10 + 48;
  Calendar[9] = month % 10 + 48;
  Calendar[8]= month / 10 + 48;
  Calendar[6]= day % 10 + 48;
  Calendar[5]= day / 10 + 48;

  server.send(200, "text/html", String(Calendar) + " "  + String(Time));
  if (debug && debugStr.length() > 0)
    gsender->Subject("currentTime")->Send("youremail@gmail.com", debugStr);
}

void handlePostRTC() {
  JsonArray arrRTC = doc.to<JsonArray>();
  deserializeJson(doc, server.arg("plain"));
  arrRTC = doc.as<JsonArray>();
  
  if (arrRTC.size() > 0)
  {
    byte minute, hour, day, month, year;
    
    minute = arrRTC[0]["minute"].as<byte>();
    hour = arrRTC[0]["hour"].as<byte>();
    day = arrRTC[0]["day"].as<byte>();
    month = arrRTC[0]["month"].as<byte>();
    year = arrRTC[0]["year"].as<byte>();
    
    minute = (minute/10 << 4) + minute % 10;
    hour = (hour/10 << 4) + hour % 10;
    day = (day/10 << 4) + day % 10;
    month = (month/10 << 4) + month % 10;
    year = (year/10 << 4) + year % 10;
    if (year > 0 && month > 0 && day > 0)
    {
      Wire.beginTransmission(0x68);
      Wire.write(0);
      Wire.write(0);
      Wire.write(minute);
      Wire.write(hour);
      Wire.write(1);
      Wire.write(day);
      Wire.write(month);
      Wire.write(year);
      Wire.endTransmission();
      delay(200);
    }
  }
  server.send(200, "text/html", server.arg("plain"));
}

void handleGetAlarm()
{
  String aux;
  serializeJson(arrAlarm, aux);
  server.send(200, "text/html", aux);
}

void handlePostAlarm()
{
  String debugStr = "";
  deserializeJson(doc, server.arg("plain"));
  arrAlarm = doc.as<JsonArray>();

  debugStr += "size: " + String(arrAlarm.size());
  for(int i=arrAlarm.size()-1; i >= 0; i--)
  {
   if(arrAlarm[i]["pin"].as<int>() != 1 && arrAlarm[i]["pin"].as<int>() != 3 ||
      arrAlarm[i]["delay"].as<long>() < 1)
    {
      debugStr += " pin: " + arrAlarm[i]["pin"].as<String>();
      debugStr += " delay: " + arrAlarm[i]["delay"].as<String>();
      arrAlarm.remove(i);
    }
  }
  
  server.send(200, "text/html", server.arg("plain"));

  if (debug && debugStr.length() > 0)
    gsender->Subject("postAlarm")->Send("youremail@gmail.com", debugStr);
}

void handleGetDebug()
{
  server.send(200, "text/html", String(debug));
}

void handlePostDebug()
{
  debug = !debug;
  server.send(200, "text/html", String(debug));
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}
