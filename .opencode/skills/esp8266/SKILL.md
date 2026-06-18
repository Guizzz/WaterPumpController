---
name: esp8266
description: Use when dealing with ESP8266-specific behaviour — pin layout, WiFi, I2C, timing, memory constraints, and known quirks of this board.
---

# ESP8266 — Platform Gotchas

This project targets the **ESP8266** (board `espino`) via the Arduino framework.

## Hardware pinout (this project)

| Signal | GPIO | Notes                       |
|--------|------|-----------------------------|
| SDA    | 14   | I2C data line               |
| SCL    | 12   | I2C clock line              |
| Relay  | 5    | Active-HIGH (HIGH = pump ON)|
| Button | 15   | INPUT_PULLUP, HIGH = pressed|

I2C addresses: OLED = `0x3C`, GY-21 = `0x40`.

## Key constraints

- **GPIO 15** (button pin) has a built-in pull-down on some ESP8266 boards. The code uses `INPUT_PULLUP` — if it doesn't work, an external 10kΩ pull-up resistor may be needed.
- **WiFiClient vs WiFiClientSecure**: The time sync uses `WiFiClientSecureBearSSL` with `client->setInsecure()` (no cert validation) because it calls `timeapi.io` over HTTPS. MQTT runs on the local LAN without TLS, so use plain `WiFiClient`.
- **Memory**: ESP8266 has ~40-50 kB free heap after WiFi + ArduinoJson use. Avoid allocating large buffers. Keep `JsonDocument` sizes small.
- **Watchdog**: The ESP8266 SDK includes a hardware watchdog. Long `delay()` calls (> a few seconds) may trigger a reset. Prefer non-blocking patterns with `millis()`.
- **WiFi persistence**: `WiFi.begin()` writes credentials to flash by default. Call `WiFi.persistent(false)` before `WiFi.begin()` if you want to avoid flash wear during testing.

## I2C notes

- Use `Wire.begin(SDA, SCL)` to set custom pins (GPIO 14 and 12).
- Both OLED and GY-21 share the same I2C bus. The GY-21 template class `GY21_<TwoWire>` hardcodes address `0x40`.
- If `Wire.requestFrom()` blocks, the sensor code has a 100-iteration timeout and returns the last known value.
