#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <vector>

struct Request
{
  String method;
  String path;
  JsonDocument (*request_function)(JsonDocument param);
};

class RequestManager
{
  WiFiServer* server;
  std::vector<Request> requests_list;

  String extract_path(String request);
  String read_headers(WiFiClient* client, int* content_length, String* content_type);
  void send_header(WiFiClient* client, bool ok, String content_type);
  JsonDocument parse_parameters(String request, String body, String content_type);

  public:
    RequestManager(WiFiServer* s);
    void add_request(String method, String path, JsonDocument (*request_function)(JsonDocument param));
    void handle_request();
};
