# WaterPumpControler
 Esp32 project for control house's water pump and its temperature over wifi

## SetUp
Create a 'config.h' file inside 'include' folder
``` c++
#define WIFI_SSID            "wifi-name"
#define WIFI_PASSWORD        "wifi-psw"
#define SENSOR_READING_TIME  5
```
# API Endpoints Documentation

## Endpoints

### GET `/get_temp`
- **Method:** GET
- **Description:** Returns the current temperature and humidity.
- **Parameters:** None
- **Response:**
    - **Type**: JSON
    - **Schema**:
    ```json
    {
        "type": "object",
        "properties": {
            "temperature": {
                "type": "number",
                "format": "float",
                "description": "The current temperature reading."
            },
            "humidity": {
                "type": "number",
                "format": "float",
                "description": "The current humidity reading."
            }
        }
    }
    ```

---

### GET `/get_status`
- **Method:** GET
- **Description:** Returns the system status.
- **Parameters:** None
- **Response:**
    - **Type**: JSON
    - **Schema**:
    ```json
    {
        "type": "object",
        "properties": {
            "relay_status": {
                "type": "boolean",
                "description": "Indicates if the relay is active (true) or inactive (false)"
            },
            "active_timer": {
                "type": "integer",
                "description": "The number of active timers"
            },
            "active_routine": {
                "type": "boolean",
                "description": "Indicates if a routine is currently running (true) or not (false)"
            }
        }
    }
    ```
---

### POST `/set`
- **Method:** POST
- **Description:** Handles setting the relay.
- **Parameters:**
    - **type**: query string
    - **schema**:
    ```json
    {
        "type": "object",
        "properties": {
            "RELAY": {
            "type": "string",
            "enum": [
                "ON",
                "OFF"
            ],
            "description": "Command for the relay. Can be 'ON' or 'OFF'."
            }
        }
    }
    ```
- **Response:** Same as `/get_status`

---

### POST `/create_timer`
- **Method:** POST
- **Description:** Creates a new timer. When the timer ends, it sets the relay to the given value.
- **Parameters:**
    - **type**: query string
    - **schema**:
    ```json
    {
        "type": "object",
        "properties": {
            "RELAY": {
            "type": "string",
            "enum": [
                "ON",
                "OFF"
            ],
            "description": "Command for the relay. Can be 'ON' or 'OFF'."
            },
            "TIMER": {
                "type": [
                    "integer"
                ],
                "description": "Timer value in seconds"
                }
        }
    }
    ```
- **Response:** Same as `/get_status`

---

### POST `/create_routine`
- **Method:** POST
- **Description:** Creates a new daily routine to turn the relay on and off.
- **Parameters:**
    - **type**: query string
    - **schema**:
    ```json
    {
        "type": "object",
        "properties": {
            "start_hour": {
                "type": "string",
                "description": "The starting hour of a time interval, provided as a string that will be converted to an integer."
            },
            "start_minute": {
                "type": "string",
                "description": "The starting minute of a time interval, provided as a string that will be converted to an integer."
            },
            "stop_hour": {
                "type": "string",
                "description": "The ending hour of a time interval, provided as a string that will be converted to an integer."
            },
            "stop_minute": {
                "type": "string",
                "description": "The ending minute of a time interval, provided as a string that will be converted to an integer."
            }
        }
    }
    ```
- **Response:** Same as `/get_status`

---

### DELETE `/delete_routine`
- **Method:** DELETE
- **Description:** Deletes the current routine.
- **Parameters:** None
- **Response:** Same as `/get_status`
