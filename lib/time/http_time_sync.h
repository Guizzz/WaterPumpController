#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

class HTTPTimeSync
{
    int hours;
    int minutes;
    int seconds;

    int daily_sec = 0;

    unsigned long curr_time;
    unsigned long last_time;

public:
    void syncTime();
    void update_time();
    int get_dailySec();
};