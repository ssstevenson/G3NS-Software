#!/usr/bin/env python3

import paho.mqtt.subscribe as subscribe

def on_message_print(client, userdata, message):
    print("%s: %s" % (message.topic, message.payload))

subscribe.callback(on_message_print, "empower/config/test/result", hostname="10.10.129.2")
