# WaterPumpController — Agent Guide

## Project
PlatformIO project for **ESP8266** (board `espino`, Arduino framework).  
Controls a water pump relay + reads GY-21 temp/humidity sensor + SSD1306 OLED display, all via WiFi with a REST API.

## First-time setup
- Create `include/config.h` (gitignored) from the template in `README.md`. Must define `WIFI_SSID`, `WIFI_PASSWORD`, `PORT`, `SENSOR_READING_TIME`, `TIME_OUT_SCREEN`, `REALY_PIN`, `BUTTON_PIN`, `SDA`, `SCL`.

## Commands
| Action | Command |
|---|---|
| Build | `pio run` |
| Upload | `pio run -t upload` |
| Upload SPIFFS (HTML) | `pio run -t uploadfs` |
| Monitor serial | `pio device monitor` |
| Clean | `pio run -t clean` |
| Test | `pio test` (no tests currently) |

## Architecture
- **`src/main.cpp`** — entry point, runs `setup()` → `loop()`, connects WiFi, routes REST endpoints
- **`lib/`** — 5 custom libraries: `displayManager/`, `pinManager/`, `requestManager/`, `time/`, `mqttManager/`
- **`include/gy_21.h`** — GY-21 sensor driver (I2C, 0x40), template class
- **`data/basic.html`** — web UI served from SPIFFS
- **`include/config.h`** — all user configuration (gitignored)

## Hardware pinout (I2C)
- SDA = 14, SCL = 12
- Relay = 5, Button = 15 (INPUT_PULLUP)
- OLED addr = 0x3C, GY-21 addr = 0x40

## Key gotchas
- Time sync uses `timeapi.io` (Europe/Amsterdam hardcoded) via insecure HTTPS (`client->setInsecure()`)
- `ArduinoJson` v7: uses `JsonDocument` (not `DynamicJsonDocument`)
- SPIFFS serves `basic.html` — after changing it, run `pio run -t uploadfs`
- Relay is active-HIGH. Button is pullup, reads HIGH when pressed.
- Timer array is fixed at 10 slots. Timers run in daily seconds (resets at midnight).
- Known typos in source: `create_rutine` (function + route missing 'o'), `...already one setted` in error message.
- Serial monitor speed: 9600 baud.

## MQTT (Guiver protocol)
- **`lib/mqttManager/`** — copiata da `Guizzz/TempStation`, gestisce WiFi internamente
- Topic: `guiver/<DEVICE_ID>/` (announce, online, status, command, response)
- `begin()` → WiFi + MQTT connect con LWT (`guiver/<id>/online` = "0")
- `on_status(builder)` → callback che restituisce `JsonDocument` per publish periodico
- `on_command(cmd, handler)` → registra handler per comando MQTT
- L'announce è automatico al boot (type, name, sensors, actuators, interval)
