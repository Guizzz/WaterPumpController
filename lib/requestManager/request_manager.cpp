#include<request_manager.h>

void RequestManager::web_page(WiFiClient* client)
{
    (*client).println("HTTP/1.1 200 OK");
    (*client).println("Content-Type: text/html");
    (*client).println("");
    (*client).println("<!DOCTYPE HTML>");
    (*client).println("<html>");

    (*client).print("CONTROL RELAY: ");

    if (pin_manager.status()) {
        (*client).print("ON");
    } else {
        (*client).print("OFF");
    }

    (*client).println("<br><br>");
    (*client).println("<a href=\"/set?RELAY=ON\"\"><button>ON</button></a>");
    (*client).println("<a href=\"/set?RELAY=OFF\"\"><button>OFF</button></a><br />");
    (*client).println("</html>");

    delay(1);

    Serial.println("Client disconnected");
    Serial.println("");
}

RequestManager::RequestManager(String ssid, String psw, WiFiServer *s)
{
    this->ssid=ssid;
    this->psw=psw;
    this->server=s;
}

void RequestManager::init_request()
{
  Serial.println("Starting...");
  Serial.println("Inizializing request manager");
  pin_manager.init_pin();
  WiFi.begin(ssid, psw);
  Serial.print("Connecting to Wifi:");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("Server started");
  (*server).begin();

  Serial.print("IP Address of network: ");
  Serial.println(WiFi.localIP());
  Serial.print("Copy and paste the following URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void RequestManager::handle_request()
{
    WiFiClient client;

    client = (*server).accept();
    if (!client)
        return;

    Serial.println("Waiting for new client");

    while (!client.available()) {
        delay(1);
    }
    
    String request = client.readStringUntil('\r');

    client.flush();
    Serial.println(request);

    if (request.indexOf("/set?") != -1) {

        JsonDocument params = parse_parameters(request);
        if (params["RELAY"] == "ON") {
            pin_manager.set_relay(true);
        }

        if (params["RELAY"] == "OFF") {
            pin_manager.set_relay(false);
        }
    }
    
    web_page(&client);
}

JsonDocument RequestManager::parse_parameters(String request)
{
    JsonDocument parsed;
    if (request.indexOf("?") == -1)
        return parsed; 
    String parameters = request.substring(request.indexOf("?")+1, request.indexOf(" ", request.indexOf("?")));

    Serial.println("Parameters: " + parameters);
 
    while (parameters.indexOf(",") != -1)
    {
        
        String param_name = parameters.substring(0, parameters.indexOf("="));
        parsed[param_name] = parameters.substring(parameters.indexOf("=")+1, parameters.indexOf(","));

        Serial.print("Param name: ");
        Serial.print(param_name);
        Serial.print(" value: ");
        String s = parsed[param_name];
        Serial.println(s);

        parameters = parameters.substring(parameters.indexOf(",")+1, parameters.length());
    }

    String param_name = parameters.substring(0, parameters.indexOf("="));
    parsed[param_name] = parameters.substring(parameters.indexOf("=")+1, parameters.length());

    Serial.print("Param name: ");
    Serial.print(param_name);
    Serial.print(" value: ");
    String s = parsed[param_name];
    Serial.println(s);

    return parsed;
}