---
name: arduino-json-v7
description: Use when reading, writing, or refactoring ArduinoJson code in this project. Covers v7 API (JsonDocument, serialization, deserialization) and the patterns used throughout the codebase.
---

# ArduinoJson v7

This project uses **ArduinoJson ≥ 7.3.0**. The v7 API differs from v6 in several important ways.

## Key differences from v6

- **`JsonDocument` replaces `DynamicJsonDocument`** — `StaticJsonDocument` and `DynamicJsonDocument` are gone. Use `JsonDocument` for everything.
- **No `doc.to<JsonObject>()` needed** — assign directly: `doc["key"] = value`.
- **`doc.as<T>()` returns a copy** in v7 (previously a reference). For mutable references, use `doc.as<JsonObject>()`.
- **`serializeJson(doc, output)`** signature is unchanged.

## Patterns used in this codebase

### Creating a response document
```cpp
JsonDocument resp;
resp["temperature"] = temp;
resp["humidity"] = hum;
return resp;
```

### Checking key existence / type
```cpp
if (params["RELAY"] != "ON" && params["RELAY"] != "OFF")  // implicit string compare
if (!params["TIMER"].is<String>())                          // type check
if (resp.containsKey("error"))                              // key existence
```

### Serializing to String
```cpp
String out;
serializeJson(resp, out);
client.print(out);
```

### Parsing is NOT used in this codebase
Parameters are extracted from the raw HTTP query string via `RequestManager::parse_parameters()` (manual string splitting), not via `deserializeJson()`. If you add JSON body parsing, use `deserializeJson(doc, input)`.

### Returning from handler functions
All REST handler functions return `JsonDocument` by value:
```cpp
JsonDocument get_status(JsonDocument param)
```

## Best practices

- Keep `JsonDocument` objects local / short-lived — they allocate on the heap.
- Do **not** store `JsonDocument` in static/global storage across `loop()` iterations unless necessary (the sole exception is `curr_status` in `main.cpp`).
- Use `doc["key"]` for writing; use `doc["key"].as<T>()` for reading with explicit type conversion.
- Avoid deeply nesting `JsonDocument` — create separate documents and merge if needed.
