#include <Arduino.h>
#include <WiFiClient.h>

class WebPage
{
  public:
    WebPage(WiFiServer server);
    void set_status(bool set);
    String get_request();
    bool client_available();
  private:
    bool pin_status;
    WiFiClient client;
};