#!/usr/bin/env python3

import json
import paho.mqtt.client as mqtt

def registerCmdCallback(client, userdata, message):
    msgJson = json.loads(message.payload)
    if msgJson['setup'] == True:
        registerJson = {
            'topic': 'empower/cmd/test/request',
            'commands': ['testMqtt']
        }

        registerJsonStr = json.dumps(registerJson)
        print(registerJsonStr)
        client.publish("empower/cmd/register", registerJsonStr)

def on_message_print(client, userdata, message):
    print("%s: %s" % (message.topic, message.payload))
    msgJson = json.loads(message.payload)

    respJson = {
        'messageType': 'commandComplete',
        'result': 'success',
        'resultMessage': 'Command completed.',
        'request': msgJson,
        'sequenceNumber': msgJson['sequenceNumber']
    }

    respStr = json.dumps(respJson)
    print("Resp: %s" % (respStr))
    client.publish("empower/cmd/result", respStr)

def run():
    client = mqtt.Client()
    client.message_callback_add("empower/cmd/setup", registerCmdCallback)
    client.message_callback_add("empower/cmd/test/request", on_message_print)
    client.connect("10.10.129.2", 1883, 30)
    client.subscribe("empower/cmd/#")

    client.loop_forever();

if __name__ == '__main__':
    run()
