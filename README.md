# Irrigation controller
Irrigation controller - Homie based

### Configuration file example
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
