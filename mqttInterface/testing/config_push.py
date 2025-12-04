#!/usr/bin/env python3

import json
import paho.mqtt.publish as publish

data = {
    'messageType': 'configRequestRead',
    'resultTopic': 'empower/config/test/result',
    'name': 'TEST_FACTORY_INT1'
}

dataStr = json.dumps(data)

print(dataStr)

publish.single("empower/config/request", dataStr, hostname="10.10.129.2")
