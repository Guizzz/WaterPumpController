#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include <pin_manager.h>


class RequestManager
{
  PinManager pin_manager;
  String ssid, psw;
  WiFiServer* server;

  void send_header(WiFiClient* client, bool ok, String content_type);
  
  void web_page(WiFiClient* client);
  void get_status(WiFiClient* client);

  JsonDocument parse_parameters(String request);

  public:
    RequestManager(String ssid, String psw, WiFiServer* s);
    void init_request();
    void handle_request();
};