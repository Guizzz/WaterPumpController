#include "mqtt_manager.h"
#include <Arduino.h>

MqttManager* MqttManager::_instance = nullptr;

MqttManager::MqttManager(
    const char* device_id,
    const char* device_name,
    const char* device_type,
    const char* wifi_ssid,
    const char* wifi_psw,
    const char* mqtt_host,
    uint16_t mqtt_port,
    unsigned long status_interval
)
    : _device_id(device_id)
    , _device_name(device_name)
    , _device_type(device_type)
    , _wifi_ssid(wifi_ssid)
    , _wifi_psw(wifi_psw)
    , _mqtt_host(mqtt_host)
    , _mqtt_port(mqtt_port)
    , _status_interval(status_interval)
    , _mqtt_client(_wifi_client)
{
    _instance = this;
}

void MqttManager::begin()
{
    connect_wifi();

    _mqtt_client.setServer(_mqtt_host.c_str(), _mqtt_port);
    _mqtt_client.setCallback(mqtt_callback_static);

    connect_mqtt();
}

void MqttManager::loop()
{
    if (!_mqtt_client.connected())
    {
        unsigned long now = millis();
        if (now - _last_reconnect_attempt > 5000)
        {
            _last_reconnect_attempt = now;
            connect_mqtt();
        }
        return;
    }

    _mqtt_client.loop();

    unsigned long now = millis();
    if (now - _last_status > _status_interval * 1000UL)
    {
        publish_status();
        _last_status = now;
    }
}

void MqttManager::on_status(JsonDocument (*status_builder)())
{
    _status_builder = status_builder;
}

void MqttManager::on_command(const char* cmd, JsonDocument (*handler)(JsonDocument param))
{
    CommandHandler ch;
    ch.cmd = cmd;
    ch.handler = handler;
    _command_handlers.push_back(ch);
}

void MqttManager::publish_status()
{
    if (!_mqtt_client.connected()) return;
    if (!_status_builder) return;

    JsonDocument doc = _status_builder();

    char topic[64];
    snprintf(topic, sizeof(topic), "guiver/%s/status", _device_id.c_str());
    publish_json(topic, doc, false);
}

void MqttManager::publish_response(const char* status, JsonDocument* state)
{
    if (!_mqtt_client.connected()) return;

    JsonDocument doc;
    doc["status"] = status;
    if (state)
        doc["state"] = *state;

    char topic[64];
    snprintf(topic, sizeof(topic), "guiver/%s/response", _device_id.c_str());
    publish_json(topic, doc, false);
}

bool MqttManager::is_connected()
{
    return _mqtt_client.connected();
}

const char* MqttManager::get_device_id() const
{
    return _device_id.c_str();
}

void MqttManager::connect_wifi()
{
    Serial.print("Connessione WiFi a ");
    Serial.println(_wifi_ssid);

    WiFi.begin(_wifi_ssid, _wifi_psw);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("WiFi connesso. IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println();
        Serial.println("WiFi NON connesso. Continuo senza rete.");
    }
}

void MqttManager::connect_mqtt()
{
    if (WiFi.status() != WL_CONNECTED) return;

    Serial.print("Connection to MQTT at ");
    Serial.print(_mqtt_host);
    Serial.print(":");
    Serial.println(_mqtt_port);

    char will_topic[64];
    snprintf(will_topic, sizeof(will_topic), "guiver/%s/online", _device_id.c_str());

    bool connected = _mqtt_client.connect(
        _device_id.c_str(),
        will_topic,
        1,
        true,
        "0"
    );

    if (!connected)
    {
        Serial.print("MQTT Connection Error, rc=");
        Serial.println(_mqtt_client.state());
        return;
    }

    Serial.println("MQTT Connected!");

    if (_first_connect)
    {
        publish_announce();
        publish_online();
        _first_connect = false;
    }

    if (!_command_handlers.empty())
    {
        char cmd_topic[64];
        snprintf(cmd_topic, sizeof(cmd_topic), "guiver/%s/command", _device_id.c_str());
        _mqtt_client.subscribe(cmd_topic, 1);
        Serial.print("Sottoscritto a ");
        Serial.println(cmd_topic);
    }

    _subscribed = true;
}

void MqttManager::publish_announce()
{
    JsonDocument doc;
    doc["type"] = _device_type;
    doc["name"] = _device_name;

    JsonArray sensors = doc["sensors"].to<JsonArray>();
    sensors.add("temperature");
    sensors.add("humidity");

    if (!_command_handlers.empty())
    {
        JsonArray actuators = doc["actuators"].to<JsonArray>();
        for (const auto& ch : _command_handlers)
        {
            JsonObject act = actuators.add<JsonObject>();
            act["name"] = ch.cmd;
            act["label"] = ch.cmd;
        }
    }

    doc["interval"] = _status_interval;

    char topic[64];
    snprintf(topic, sizeof(topic), "guiver/%s/announce", _device_id.c_str());
    publish_json(topic, doc, true);

    Serial.println("Announce pubblicato");
}

void MqttManager::publish_online()
{
    char topic[64];
    snprintf(topic, sizeof(topic), "guiver/%s/online", _device_id.c_str());
    _mqtt_client.publish(topic, "1", true);

    Serial.println("Online = 1 pubblicato");
}

void MqttManager::mqtt_callback_static(char* topic, byte* payload, unsigned int length)
{
    if (_instance)
        _instance->on_mqtt_message(topic, payload, length);
}

void MqttManager::on_mqtt_message(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Messaggio MQTT ricevuto su ");
    Serial.println(topic);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err)
    {
        Serial.print("Errore deserializzazione JSON: ");
        Serial.println(err.c_str());
        return;
    }

    const char* cmd = doc["cmd"];
    if (!cmd)
    {
        Serial.println("Payload senza campo cmd");
        return;
    }

    Serial.print("Comando ricevuto: ");
    Serial.println(cmd);

    for (const auto& ch : _command_handlers)
    {
        if (ch.cmd == cmd)
        {
            JsonDocument result = ch.handler(doc);
            const char* status = result["error"].isNull() ? "ok" : "error";

            JsonDocument state;
            for (auto kv : result.as<JsonObject>())
                state[kv.key().c_str()] = kv.value();

            publish_response(status, &state);
            return;
        }
    }

    Serial.println("Nessun handler trovato per il comando");
    publish_response("error");
}

void MqttManager::publish_json(const char* topic, JsonDocument& doc, bool retained)
{
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    _mqtt_client.publish(topic, (const uint8_t*)buffer, n, retained);

    Serial.print("Pubblicato su ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(buffer);
}
