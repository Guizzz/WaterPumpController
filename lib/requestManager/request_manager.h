#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include <pin_manager.h>


class RequestManager
{
  PinManager pin_manager;
  String ssid, psw;
  WiFiServer* server;

  void web_page(WiFiClient* client);

  public:
    RequestManager(String ssid, String psw, WiFiServer* s);
    void handle_request();
    void init_request();
};