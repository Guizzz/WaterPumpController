#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include "pin_manager.h"

WiFiServer server(80);
PinManager pin_manager;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pin_manager.init_pin();
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("Server started");
  server.begin();

  Serial.print("IP Address of network: ");
  Serial.println(WiFi.localIP());
  Serial.print("Copy and paste the following URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}

void loop() {
  WiFiClient client = server.available();

  if (!client) {
    return;
  }

  Serial.println("Waiting for new client");

  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println(request);
  
  client.flush();


  if (request.indexOf("/LED_ON") != -1) {
    pin_manager.set_relay(true);
  }

  if (request.indexOf("/LED_OFF") != -1) {
    pin_manager.set_relay(false);
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("CONTROL LED: ");

  if (pin_manager.status()) {
    client.print("ON");
  } else {
    client.print("OFF");
  }

  client.println("<br><br>");
  client.println("<a href=\"/LED_ON\"\"><button>ON</button></a>");
  client.println("<a href=\"/LED_OFF\"\"><button>OFF</button></a><br />");
  client.println("</html>");

  delay(1);

  Serial.println("Client disconnected");
  Serial.println("");
}
