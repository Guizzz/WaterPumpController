---
name: platformio
description: Use when building, uploading, monitoring, or cleaning a PlatformIO project (ESP8266/Arduino). Handles pio run, upload, uploadfs, device monitor, clean, and test commands.
---

# PlatformIO — Build & Deploy

This project uses **PlatformIO** with the `espressif8266` platform and `arduino` framework on the `espino` board.

## Commands

| Action                        | Command                 |
|-------------------------------|-------------------------|
| Build                         | `pio run`               |
| Upload firmware               | `pio run -t upload`     |
| Upload SPIFFS filesystem      | `pio run -t uploadfs`   |
| Open serial monitor (9600)    | `pio device monitor`    |
| Clean build artifacts         | `pio run -t clean`      |
| Run tests                     | `pio test` (none yet)   |

## SPIFFS

The web UI lives in `data/basic.html`. After editing it, **always** run `pio run -t uploadfs` to push the new file to the ESP's SPIFFS partition. The firmware upload (`-t upload`) does NOT include SPIFFS.

## Library dependencies (from `platformio.ini`)

- `bblanchon/ArduinoJson@^7.3.0`
- `robtillaart/SHT2x@^0.5.0`
- `adafruit/Adafruit SSD1306@^2.5.13`
- `adafruit/Adafruit GFX Library@^1.11.11`

When adding a new library dependency, add it under `lib_deps` in `platformio.ini`, then rebuild.

## Serial monitor

Speed is **9600 baud** (`monitor_speed = 9600`). Always pass this baud rate when opening tools like PuTTY or screen, or use `pio device monitor` which reads the setting automatically.
