#include <Arduino.h>

#include <display_manager.h>
#include <request_manager.h>
#include <pin_manager.h>

#include <http_time_sync.h>

#include "config.h"
#include "gy_21.h"

WiFiServer server(80);

PinManager pin_manager;
DisplayManager display_manager;
RequestManager request_manager(WIFI_SSID, WIFI_PASSWORD, &server);

unsigned long curr_time;
unsigned long last_time;

GY21 sensor;
ClockTime t;
JsonDocument curr_status;

JsonDocument get_temp(JsonDocument param)
{
  float temp = sensor.GY21_Temperature();
  float hum = sensor.GY21_Humidity();

  JsonDocument resp;
  resp["temperature"] = temp;
  resp["humidity"] = hum;
  return resp;
}

JsonDocument get_status(JsonDocument param)
{
  JsonDocument status = get_temp((JsonDocument)nullptr);
  status["relay_info"] = pin_manager.status();
  ClockData time = t.get_time();
  status["time"]["seconds"] = time.seconds;
  status["time"]["minutes"] = time.minutes;
  status["time"]["hours"] = time.hours;

  return status;
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

  if (params["TIMER"].is<String>())
  {   
    Serial.println("Invalid request: set/ request does not have TIMER");
    resp["error"]["TIMER"] = "Invalid type: set/ request does not have TIMER";
    return resp;
  }
  
  pin_manager.set_relay(action);

  return get_status((JsonDocument)nullptr);
}

JsonDocument create_timer(JsonDocument params)
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
  {   
    Serial.println("Invalid request: missing TIMER parameter");
    resp["error"]["TIMER"] = "Invalid request: missing TIMER parameter";
    return resp;
  }

  pin_manager.create_timer(String(params["TIMER"]).toInt(), action, t.get_dailySec());

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

int time_out = TIME_OUT_SCREEN;
int page = 0;

void show_info()
{
  if (time_out == 0)
    return;

  if (curr_time % SENSOR_READING_TIME == 0 && curr_time != last_time)
  {
    Serial.print("Read temp ");
    Serial.println(curr_time);
    curr_status = get_status((JsonDocument)nullptr);

    last_time = curr_time;
    time_out --;
  }

  Info i;
  i.temp           = curr_status["temperature"];
  i.humidity       = curr_status["humidity"];
  i.relay          = curr_status["relay_info"]["relay_status"];
  i.active_timer   = curr_status["relay_info"]["active_timer"];
  i.active_routine = curr_status["relay_info"]["active_routine"];
  i.seconds        = curr_status["time"]["seconds"];
  i.minutes        = curr_status["time"]["minutes"];
  i.hours          = curr_status["time"]["hours"];

  display_manager.display_info(i, page);
}

void setup() {
  Serial.begin(9600);
  Wire.begin(SDA, SCL);

  display_manager.init_display();
  pin_manager.init_pin(REALY_PIN, BUTTON_PIN);
  request_manager.init_request();

  t.syncTime();

  request_manager.add_request("GET","/get_temp", &get_temp);
  request_manager.add_request("GET","/get_status", &get_status);
  request_manager.add_request("POST","/set", &manage_relay);
  request_manager.add_request("POST","/create_timer", &create_timer);
  request_manager.add_request("POST","/create_routine", &create_rutine);
  request_manager.add_request("DELETE","/delete_routine", &delete_rutine);
  
  curr_status = get_status((JsonDocument)nullptr);
}


void loop() {
  curr_time = t.get_dailySec();
  
  show_info();   

  if (time_out == 0)
  {
    display_manager.clear();
    page = 0;
  }
  
  if(pin_manager.isButtonPressed())
  {
    Serial.println("Button pressed");
    time_out = TIME_OUT_SCREEN;
    if (last_time == curr_time)
      page = (++page) % 2;

    Serial.print("page: ");
    Serial.println(page);
  }

  pin_manager.manage_timer(t);
  request_manager.handle_request();

  t.update_time();
}
