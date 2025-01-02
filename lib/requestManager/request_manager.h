#include <ESP8266WiFi.h>
#include <pin_manager.h>


class RequestManager
{
  PinManager pin_manager;
  String ssid, psw;

  public:
    RequestManager(String ssid, String psw);
    void handle_request(WiFiServer* s);
    void init_request(WiFiServer* s);
};