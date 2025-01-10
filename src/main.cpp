#include <Arduino.h>
#include<request_manager.h>
#include <pin_manager.h>

#include "config.h"


WiFiServer server(80);
PinManager pin_manager;
RequestManager request_manager(WIFI_SSID, WIFI_PASSWORD, &server, &pin_manager);

unsigned long curr_time;
unsigned long last_time;



void setup() {
  Serial.begin(9600);
  pin_manager.init_pin();
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

  pin_manager.manage_timer(curr_time/1000);
  request_manager.handle_request();
}
