#include "http_time_sync.h"

void ClockTime::syncTime()
{
  if ((WiFi.status() != WL_CONNECTED)) {
    Serial.println("Connection error");
    return;
  }

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  
  HTTPClient https;
  if (!https.begin(*client, "https://timeapi.io/api/time/current/zone?timeZone=Europe%2FAmsterdam")) { 
    Serial.printf("[HTTPS] Unable to connect\n");
    return;
  }

  int httpCode = https.GET();
  if (httpCode <= 0) {
    Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    return;
  }

  // file found at server
  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
    String payload = https.getString();
    JsonDocument resp;
    deserializeJson(resp, payload);
    Serial.println("Syncing time...");
    clock_data.hours = resp["hour"];
    clock_data.minutes = resp["minute"];
    clock_data.seconds = resp["seconds"];
    Serial.print("h: "); Serial.print(clock_data.hours);
    Serial.print(" m: "); Serial.print(clock_data.minutes);
    Serial.print(" s: "); Serial.println(clock_data.seconds);

    clock_data.daily_sec = (clock_data.hours * 3600) + (clock_data.minutes * 60) + clock_data.seconds;
  }
  else{
    Serial.print(https.getString());
  }
  https.end();

}

void ClockTime::update_time()
{
  curr_time = millis()/1000;
  if (curr_time == last_time )
    return;
  
  clock_data.daily_sec++;
  clock_data.daily_sec = clock_data.daily_sec % 86400;

  if(clock_data.daily_sec == 0)
    syncTime();
  last_time = curr_time;

  clock_data.hours = clock_data.daily_sec/3600;
  clock_data.minutes = (clock_data.daily_sec%3600)/60;
  clock_data.seconds = (clock_data.daily_sec%3600)%60;

  Serial.print("h: "); Serial.print(clock_data.hours);
  Serial.print(" m: "); Serial.print(clock_data.minutes);
  Serial.print(" s: "); Serial.println(clock_data.seconds);
}

int ClockTime::get_dailySec()
{
    return clock_data.daily_sec;
}

ClockData ClockTime::get_time()
{
  return clock_data;
}