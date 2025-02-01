#ifndef TIME_H
#define TIME_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

class ClockTime
{
    int hours;
    int minutes;
    int seconds;

    int daily_sec = 0;

    unsigned long curr_time;
    unsigned long last_time = 0;

public:
    void syncTime();
    void update_time();
    int get_dailySec();
};
#endif