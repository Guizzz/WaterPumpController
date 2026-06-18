---
name: mqtt-guiver
description: Use when implementing or troubleshooting MQTT integration with the Guiver protocol. Covers topics, payload formats, LWT, QoS, retained flags, and PubSubClient patterns used in this project.
---

# MQTT — Guiver Protocol

This project uses MQTT for remote control and monitoring alongside the existing REST API (hybrid architecture). The broker is a local Mosquitto instance.

## Configuration (from `config.h`)

```cpp
#define MQTT_HOST "192.168.1.109"
#define MQTT_PORT 1883
#define DEVICE_ID "pompa_cantina"
```

## Topic structure

All topics are prefixed with `guiver/<DEVICE_ID>/`:

| Topic                              | Direction     | QoS  | Retained | Description          |
|------------------------------------|---------------|------|----------|----------------------|
| `guiver/<id>/announce`             | ESP → Broker  | 1    | Yes      | Device capabilities  |
| `guiver/<id>/online`               | ESP → Broker  | 1    | Yes      | Presence (1/0)       |
| `guiver/<id>/status`               | ESP → Broker  | 0    | No       | Periodic sensor data |
| `guiver/<id>/command`              | Broker → ESP  | 1    | No       | Incoming commands    |
| `guiver/<id>/response`             | ESP → Broker  | 1    | No       | Command response     |

### LWT (Last Will and Testament)

The `online` topic doubles as the LWT will topic. When the ESP disconnects unexpectedly, the broker publishes `"0"` to `guiver/<id>/online` automatically.

**Connect call with LWT (PubSubClient):**
```cpp
client.connect(DEVICE_ID, "", "", "guiver/" DEVICE_ID "/online", 1, true, "0");
```

## Payload formats

### Announce (at boot)
```json
{
  "type": "pump",
  "name": "Pompa Acqua",
  "sensors": ["temperature", "humidity"],
  "actuators": [{ "name": "pump", "label": "Pompa acqua" }],
  "interval": 30
}
```

### Online (presence)
Plain string: `"1"` (online) or `"0"` (offline via LWT).

### Status (periodic)
```json
{ "temperature": 23.5, "humidity": 62.0, "running": false }
```

### Command (inbound)
```json
{ "cmd": "set_pump", "value": false }
```

### Response (outbound)
```json
{ "status": "ok", "state": { "running": false } }
```

## PubSubClient patterns

### Connection
```cpp
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void connect() {
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(onMqttMessage);
  client.connect(DEVICE_ID, "", "", "guiver/" DEVICE_ID "/online", 1, true, "0");
  client.subscribe("guiver/" DEVICE_ID "/command");
}
```

### Callback
```cpp
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  JsonDocument doc;
  deserializeJson(doc, payload, length);
  String cmd = doc["cmd"];
  // handle command
}
```

### Publishing
```cpp
// String payload (online)
client.publish("guiver/" DEVICE_ID "/online", "1", true);

// JSON payload (status, response, announce)
String out;
serializeJson(doc, out);
client.publish("guiver/" DEVICE_ID "/status", out.c_str());
```

- `publish()` returns `bool` — check it for QoS 1 delivery.
- For retained messages, pass `true` as the 3rd argument to `publish()`.
- Call `client.loop()` frequently (every `loop()` iteration) to keep the connection alive and process inbound messages.
