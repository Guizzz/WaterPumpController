#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include <request_manager.h>
#include <pin_manager.h>
#include <http_time_sync.h>

#include "config.h"
#include "gy_21.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiServer server(80);
PinManager pin_manager;
RequestManager request_manager(WIFI_SSID, WIFI_PASSWORD, &server);

unsigned long curr_time;
unsigned long last_time;

GY21 sensor;
ClockTime t;

void init_display()
{
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);

  display.clearDisplay();
  display.setTextColor(WHITE);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Starting...");
  display.display();
}

void display_info(float temp, float humidity, bool relay)
{
  display.clearDisplay();

  // display temperature
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Monitor ambient ");
  display.print(relay?"ON":"OFF");

  display.setCursor(0, 16);
  display.print("Temp: ");
  // display.setCursor(0, 26);
  display.setTextSize(2);
  display.print(temp, 1);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  display.setCursor(0, 42);
  display.setTextSize(1);
  display.print("Hum:  ");
  // display.setCursor(0, 52);
  display.setTextSize(2);
  display.print(humidity, 1);
  display.print(" %");

  display.display(); 
}

JsonDocument get_temp(JsonDocument param)
{
  float temp = sensor.GY21_Temperature();
  float hum = sensor.GY21_Humidity();
  /*Serial.print("temp: ");
  Serial.print(temp);
  Serial.print("\t");
  Serial.print("Hum: ");
  Serial.println(hum);*/

  JsonDocument resp;
  resp["temperature"] = temp;
  resp["humidity"] = hum;
  return resp;
}

JsonDocument get_status(JsonDocument param)
{
  JsonDocument s = pin_manager.status();
  return s;
}

JsonDocument manage_relay(JsonDocument params)
{   
  JsonDocument resp;
  if(params["RELAY"] != "ON" && params["RELAY"] != "OFF")
  {   
    Serial.println("Invalid type: can only be ON or OFF");
    resp["error"]["RELAY"] = "Invalid type: can only be ON or OFF";
    return resp;
  }

  bool action = params["RELAY"] == "ON";

  if (!params["TIMER"].is<String>())
      pin_manager.set_relay(action);
  else
      pin_manager.create_timer(String(params["TIMER"]).toInt(), action);

  
  JsonDocument t_h = get_temp((JsonDocument)nullptr);
  t_h["relay_info"] = get_status((JsonDocument)nullptr);
  display_info(t_h["temperature"], t_h["humidity"], t_h["relay_info"]["relay_status"]); 

  return get_status((JsonDocument)nullptr);
}

JsonDocument create_rutine(JsonDocument params)
{
  pin_manager.create_routine(
    String(params["start_hour"]).toInt(), 
    String(params["start_minute"]).toInt(), 
    String(params["stop_hour"]).toInt(), 
    String(params["stop_minute"]).toInt());
  return get_status((JsonDocument)nullptr);
}

JsonDocument delete_rutine(JsonDocument params)
{
  pin_manager.delete_routine();
  return get_status((JsonDocument)nullptr);
}

void setup() {
  Serial.begin(9600);
  Wire.begin(14,12); // SDA, SCL

  init_display();
  pin_manager.init_pin();
  request_manager.init_request();
  
  t.syncTime();

  request_manager.add_request("GET","/get_temp", &get_temp);
  request_manager.add_request("GET","/get_status", &get_status);
  request_manager.add_request("POST","/set", &manage_relay);
  request_manager.add_request("POST","/create_routine", &create_rutine);
  request_manager.add_request("DELETE","/delete_rutine", &delete_rutine);
}

void loop() {
  curr_time = t.get_dailySec();
  if (curr_time % SENSOR_READING_TIME == 0 && curr_time != last_time)
  {
    Serial.print("Read temp ");
    Serial.println(curr_time);
    JsonDocument t_h = get_temp((JsonDocument)nullptr);
    t_h["relay_info"] = get_status((JsonDocument)nullptr);
    display_info(t_h["temperature"], t_h["humidity"], t_h["relay_info"]["relay_status"]); 
    last_time = curr_time;
  }
  
  pin_manager.manage_timer(t);
  request_manager.handle_request();
  
  t.update_time();
}
