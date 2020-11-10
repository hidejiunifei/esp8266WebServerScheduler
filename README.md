# esp8266WebServerScheduler

esp8266 used esp8266 esp-01
Real time module used Real Time Clock RTC Ds1307

The real time module is not necessary if you will be using an online real time clock web service.
Libraries used:

    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
    #include <ArduinoJson.h>
    #include "Gsender.h"
    #include <Wire.h>
	
The Gsender.h and the following configurations are only necessary to use gmail as debugger.
	
Need to configure \ESP8266_Gmail_Sender\Gsender.h

    const char* EMAILBASE64_LOGIN = "youremail@gmail.com in base64";
    const char* EMAILBASE64_PASSWORD = "your password in base64";
    const char* FROM = "youremail@gmail.com";
	
The esp8266.ino has to be configured as well

    gsender->Subject("currentTime")->Send(

The following line is used by esp8266 to connect to a wifi network and make the routes available:

    WiFi.begin("your ssid", "your wifi password");
	
Routes available:

    server.on("/", HTTP_GET, handleRoot); -- List available routes
    server.on("/RTC", HTTP_GET, handleGetRTC); -- get real time data
    server.on("/RTC", HTTP_POST, handlePostRTC); -- set real time data
    server.on("/ALM", HTTP_GET, handleGetAlarm); -- get alarm data
    server.on("/ALM", HTTP_POST, handlePostAlarm); -- set alarm data
    server.on("/DBG", HTTP_GET, handleGetDebug); -- get debug status
    server.on("/DBG", HTTP_POST, handlePostDebug); -- set debug status
	
The pin configuration is fixed:
    pinMode(1, OUTPUT);
    digitalWrite(1, LOW);
	
Json used to set real time clock:
	[{"day":16,"minute":5,"hour":51,"day":30,"month":12,"year":19}]
	
Json used to set alarm:
    [ { pin: 1, delay: 50, day: 1, hour: 12, minute: 10 } ]
	
For future work:

The alarms should be stored on the flash memory to avoid losing them to power loss.
The current code is always setting the pin from HIGH to LOW, that could be configured on the json to the alarm.

