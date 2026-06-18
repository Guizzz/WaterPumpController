#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <list>

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

struct CommandHandler
{
    String cmd;
    JsonDocument (*handler)(JsonDocument param);
};

class MqttManager
{
public:
    MqttManager(
        const char* device_id,
        const char* device_name,
        const char* device_type,
        const char* wifi_ssid,
        const char* wifi_psw,
        const char* mqtt_host,
        uint16_t mqtt_port,
        unsigned long status_interval = 60
    );

    void begin();
    void loop();
    void on_status(JsonDocument (*status_builder)());
    void on_command(const char* cmd, JsonDocument (*handler)(JsonDocument param));
    void publish_status();
    void publish_response(const char* status, JsonDocument* state = nullptr);
    bool is_connected();
    const char* get_device_id() const;

private:
    String _device_id;
    String _device_name;
    String _device_type;
    String _wifi_ssid;
    String _wifi_psw;
    String _mqtt_host;
    uint16_t _mqtt_port;
    unsigned long _status_interval;

    WiFiClient _wifi_client;
    PubSubClient _mqtt_client;

    JsonDocument (*_status_builder)() = nullptr;
    std::list<CommandHandler> _command_handlers;

    unsigned long _last_status = 0;
    unsigned long _last_reconnect_attempt = 0;

    bool _first_connect = true;
    bool _subscribed = false;

    void connect_wifi();
    void connect_mqtt();
    void publish_announce();
    void publish_online();

    static MqttManager* _instance;
    static void mqtt_callback_static(char* topic, byte* payload, unsigned int length);
    void on_mqtt_message(char* topic, byte* payload, unsigned int length);

    void publish_json(const char* topic, JsonDocument& doc, bool retained);
};

#endif
