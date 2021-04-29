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

