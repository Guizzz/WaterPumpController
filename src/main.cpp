#include <Arduino.h>
#include<request_manager.h>

#include "config.h"


WiFiServer server(80);
RequestManager request_manager(WIFI_SSID, WIFI_PASSWORD, &server);

unsigned long curr_time;
unsigned long last_time;



void setup() {
  Serial.begin(9600);
  request_manager.init_request();
}

void loop() {
  curr_time = millis();
  if (curr_time % SENSOR_READING_TIME == 0 && curr_time != last_time)
  {
    Serial.print("Read temp ");
    Serial.println(curr_time);

    last_time = curr_time;
  }

  request_manager.handle_request();
}
