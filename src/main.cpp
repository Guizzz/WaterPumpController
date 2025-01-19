#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include <request_manager.h>
#include <pin_manager.h>

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

JsonDocument get_temp(void *manager, JsonDocument param)
{
  GY21 *s = (GY21*)manager;
  float temp = (*s).GY21_Temperature();
  float hum = (*s).GY21_Humidity();
  Serial.print("temp: ");
  Serial.print(temp);
  Serial.print("\t");
  Serial.print("Hum: ");
  Serial.println(hum);

  JsonDocument resp;
  resp["temperature"] = temp;
  resp["humidity"] = hum;
  return resp;
}

JsonDocument get_status(void *manager, JsonDocument param)
{
  PinManager *pin_manager = (PinManager*)manager;
  JsonDocument s = (*pin_manager).status();
  return s;
}

JsonDocument manage_relay(void *manager, JsonDocument params)
{   
  PinManager *pin_manager = (PinManager*)manager;
  JsonDocument resp;
  if(params["RELAY"] != "ON" && params["RELAY"] != "OFF")
  {   
    Serial.println("Invalid type: can only be ON or OFF");
    resp["error"]["RELAY"] = "Invalid type: can only be ON or OFF";
    return resp;
  }

  bool action = params["RELAY"] == "ON";

  if (!params.containsKey("TIMER"))
      (*pin_manager).set_relay(action);
  else
      (*pin_manager).create_timer(String(params["TIMER"]).toInt(), action);
  return get_status(pin_manager, params);
}


void setup() {
  Serial.begin(9600);
  Wire.begin(14,12); // SDA, SCL

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  pin_manager.init_pin();
  request_manager.init_request();

  request_manager.add_request("GET","/get_temp", &get_temp, &sensor);
  request_manager.add_request("GET","/get_status", &get_status, &pin_manager);
  request_manager.add_request("POST","/set?", &manage_relay, &pin_manager);
}

void loop() {
  curr_time = millis();
  if (curr_time % SENSOR_READING_TIME == 0 && curr_time != last_time)
  {
    Serial.print("Read temp ");
    Serial.println(curr_time);

    display.clearDisplay();
  
    // display temperature
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Temperature: ");

    last_time = curr_time;
  }

  pin_manager.manage_timer(curr_time/1000);

  request_manager.handle_request();
}
