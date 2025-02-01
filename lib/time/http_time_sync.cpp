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
    hours = resp["hour"];
    minutes = resp["minute"];
    seconds = resp["seconds"];
    Serial.print("h: "); Serial.print(hours);
    Serial.print(" m: "); Serial.print(minutes);
    Serial.print(" s: "); Serial.println(seconds);

    daily_sec = (hours * 3600) + (minutes * 60) + seconds;
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
  
  daily_sec++;
  daily_sec = daily_sec % 86400;

  if(daily_sec == 0)
    syncTime();
  last_time = curr_time;

  hours = daily_sec/3600;
  minutes = (daily_sec%3600)/60;
  seconds = (daily_sec%3600)%60;

  Serial.print("h: "); Serial.print(hours);
  Serial.print(" m: "); Serial.print(minutes);
  Serial.print(" s: "); Serial.println(seconds);
}

int ClockTime::get_dailySec()
{
    return daily_sec;
}
