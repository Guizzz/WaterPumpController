#include <Arduino.h>

#include <display_manager.h>
#include <request_manager.h>
#include <pin_manager.h>
#include <http_time_sync.h>

#include "config.h"
#include "gy_21.h"

#include <mqtt_manager.h>
#include <thread_manager.h>

WiFiServer server(80);

PinManager pin_manager;
DisplayManager display_manager(TIME_OUT_SCREEN * 1000UL);
RequestManager request_manager(&server);

GY21 sensor;
ClockTime t;
JsonDocument curr_status;

MqttManager mqtt(DEVICE_ID, "Pompa Acqua", "pump",
                 WIFI_SSID, WIFI_PASSWORD,
                 MQTT_HOST, MQTT_PORT,
                 SENSOR_READING_TIME);

ThreadManager thread_manager;

int current_page = 0;

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
  bool ret = pin_manager.create_routine(
    String(params["start_hour"]).toInt(),
    String(params["start_minute"]).toInt(),
    String(params["stop_hour"]).toInt(),
    String(params["stop_minute"]).toInt());

  if (ret)
    return get_status((JsonDocument)nullptr);

  JsonDocument resp;
  resp["relay_info"]["error"] = "Fail to crate new routine, there is already one setted";
  return resp;
}

JsonDocument delete_rutine(JsonDocument params)
{
  pin_manager.delete_routine();
  return get_status((JsonDocument)nullptr);
}

JsonDocument mqtt_status_builder()
{
  JsonDocument doc;
  doc["temperature"] = curr_status["temperature"];
  doc["humidity"] = curr_status["humidity"];
  doc["running"] = curr_status["relay_info"]["relay_status"];
  return doc;
}

JsonDocument handle_set_pump(JsonDocument params)
{
  bool value = params["value"];
  pin_manager.set_relay(value);
  return pin_manager.status();
}

void read_sensor_task()
{
  float temp = sensor.GY21_Temperature();
  float hum = sensor.GY21_Humidity();

  curr_status["temperature"] = temp;
  curr_status["humidity"] = hum;

  JsonDocument s = pin_manager.status();
  curr_status["relay_info"]["relay_status"] = s["relay_status"];
  curr_status["relay_info"]["active_timer"] = s["active_timer"];
  curr_status["relay_info"]["active_routine"] = s["active_routine"];
}

void manage_timer_task()
{
  pin_manager.manage_timer(t);
}

void handle_button_task()
{
  if (pin_manager.isButtonPressed())
  {
    display_manager.activity();
  }
}

void update_time_task()
{
  t.update_time();
  ClockData time = t.get_time();
  curr_status["time"]["seconds"] = time.seconds;
  curr_status["time"]["minutes"] = time.minutes;
  curr_status["time"]["hours"] = time.hours;
}

void update_display_task()
{
  display_manager.update();

  if (!display_manager.is_on()) return;

  Info info;
  info.temp           = curr_status["temperature"];
  info.humidity       = curr_status["humidity"];
  info.relay          = curr_status["relay_info"]["relay_status"];
  info.active_timer   = curr_status["relay_info"]["active_timer"];
  info.active_routine = curr_status["relay_info"]["active_routine"];
  info.seconds        = curr_status["time"]["seconds"];
  info.minutes        = curr_status["time"]["minutes"];
  info.hours          = curr_status["time"]["hours"];
  info.ssid           = WiFi.SSID();
  info.ip             = WiFi.localIP().toString();

  display_manager.display_info(info);
}

void setup()
{
  Serial.begin(9600);
  Wire.begin(SDA, SCL);

  display_manager.init_display();
  display_manager.fast_write("Connessione WiFi...");

  mqtt.begin();
  server.begin();

  pin_manager.init_pin(REALY_PIN, BUTTON_PIN);

  display_manager.fast_write("Sincronizzo ora...");
  t.syncTime();

  request_manager.add_request("GET","/get_temp", &get_temp);
  request_manager.add_request("GET","/get_status", &get_status);
  request_manager.add_request("POST","/set", &manage_relay);
  request_manager.add_request("POST","/create_timer", &create_timer);
  request_manager.add_request("POST","/create_routine", &create_rutine);
  request_manager.add_request("DELETE","/delete_routine", &delete_rutine);

  mqtt.on_status(mqtt_status_builder);
  mqtt.on_command("set_pump", handle_set_pump);

  thread_manager.add_method(update_time_task, 1000);
  thread_manager.add_method(manage_timer_task, 1000);
  thread_manager.add_method(handle_button_task, 25);
  thread_manager.add_method(read_sensor_task, SENSOR_READING_TIME * 1000UL);
  thread_manager.add_method(update_display_task, 500);

  curr_status = get_status((JsonDocument)nullptr);

  display_manager.fast_write("Pronto!");
}

void loop()
{
  thread_manager.thread_loop();
  request_manager.handle_request();
  mqtt.loop();
}
