#include<request_manager.h>

void RequestManager::web_page(WiFiClient* client)
{
    (*client).println("HTTP/1.1 200 OK");
    (*client).println("Content-Type: text/html");
    (*client).println("");
    (*client).println("<!DOCTYPE HTML>");
    (*client).println("<html>");

    (*client).print("CONTROL LED: ");

    if (pin_manager.status()) {
        (*client).print("ON");
    } else {
        (*client).print("OFF");
    }

    (*client).println("<br><br>");
    (*client).println("<a href=\"?LED=ON\"\"><button>ON</button></a>");
    (*client).println("<a href=\"?LED=OFF\"\"><button>OFF</button></a><br />");
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
        if (request.indexOf("LED=ON") != -1) {
            pin_manager.set_relay(true);
        }

        if (request.indexOf("LED=OFF") != -1) {
            pin_manager.set_relay(false);
        }
    }
    
    web_page(&client);
}