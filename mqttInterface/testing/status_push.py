#!/usr/bin/env python3

import json
import paho.mqtt.publish as publish

data = {
    'messageType': 'statusUpdate',
    'fields': [
        { 'key': 'MQTT_TEST', 'label': 'MQTT Push Test', 'value': True, 'timeoutMs': 60000 }
    ]
}

dataStr = json.dumps(data)

print(dataStr)

publish.single("empower/status", dataStr, hostname="10.10.129.2")
