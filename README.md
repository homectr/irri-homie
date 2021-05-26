# Irrigation Controller for ESP8266 

Controller is based on Homie convention for connecting to MQTT brokers.
See https://homieiot.github.io/

Avalable connectors:
* OpenHAB https://www.openhab.org/addons/bindings/mqtt.homie/
* Homeassist - not yet (only using direct MQTT)

# Principles
## Valves
System supports definable number of valves, which can be turned ON and OFF.
Valves are managed by programs or manually.

## Programs
System supports definable number of programs.
Each program defines 
* days of week it is excecuted on
* starting hour & minute
* run times for each valve (0 means valve is OFF)

Program can optionally have name specified.

## System
It is possible to disable scheduling until specific date.

It is possible to set irrigation intensity. Intensity is percentage value applied on valve run time.
If run time is e.g. 10 minutes, then intensity 
* 100% = 10 minutes
* 50% = 5 minutes
* 200% = 20 minutes

## Configuration file example
```
{
    "name": "Irrigation",
    "device_id": "iridevice",
    "device_stats_interval": 60,
    "wifi": {
      "ssid": "my wifi",
      "password": "my psk"
    },
    "mqtt": {
      "host": "openhabian.local",
      "port": 1883
    },
    "ota": {
      "enabled": true
    },
    "settings": {
      "percentage": 55
    },
    "irrigation": {
        "intensity": 100,
        "disabled-til": "2000-01-01",
        "valves":[10,11,12,13,14,15],
        "programs":[
            { "id": 1, "run-times":"10,10,20,0,0,0", "run-days":"10010101", "start-hour": 6, "start-min": 30 },
            { "id": 2, "run-times":"0,0,0,0,0,20", "run-days":"10010101", "start-hour": 5, "start-min": 0 }
        ]
    }
  }

```