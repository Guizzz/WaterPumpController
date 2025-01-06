#include<request_manager.h>

void RequestManager::web_page(WiFiClient* client)
{   
    File file = SPIFFS.open("/basic.html", "r");
    //TODO: IMPLEMENT SENSOR READING 
    if(!file.available())
    {
        (*client).println("HTTP/1.1 500 OK");
        (*client).println("Content-Type: text/html");
        (*client).println("");
        (*client).println("ERROR");
        return;
    }

    String page = file.readString();

    page.replace("\%status\%", pin_manager.status()?"ON":"OFF");

    (*client).println("HTTP/1.1 200 OK");
    (*client).println("Content-Type: text/html");
    (*client).println("");
    (*client).println(page);

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

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
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
    JsonDocument params = parse_parameters(request);

    if (request.indexOf("/set?") != -1 && params["method"] == "POST") {

        if (params["RELAY"] == "ON") {
            pin_manager.set_relay(true);
        }

        if (params["RELAY"] == "OFF") {
            pin_manager.set_relay(false);
        }
        
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("");

        String status = get_status();
        Serial.println("Setted status: " + status);
        client.println(status);
        return;
    }

    if (request.indexOf("/get_stat") != -1 && params["method"] == "GET")
    {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("");

        String status = get_status();
        Serial.println("Get status: " + status);
        client.println(status);
        return;
    }

    if (request.indexOf(" / ") != -1 && params["method"] == "GET")
        web_page(&client);
}

String RequestManager::get_status()
{
    JsonDocument resp;
    resp["relay_status"] = pin_manager.status();
    String out;
    serializeJson(resp, out);
    return out;
}

JsonDocument RequestManager::parse_parameters(String request)
{
    JsonDocument parsed;
    String method = request.substring(0, request.indexOf(" "));
    parsed["method"] = method;

    if (request.indexOf("?") == -1)
        return parsed; 

    request = request.substring(request.indexOf(" ")+1, request.length());
    String parameters = request.substring(request.indexOf("?")+1, request.indexOf(" "));

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