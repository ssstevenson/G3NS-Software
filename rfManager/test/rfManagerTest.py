#!/usr/bin/env python3

import zmq, time, pwd, os

PRINT_COUNT = 10001

if __name__ == '__main__':
    context = zmq.Context()

    cmdInSock = context.socket(zmq.PUB)
    cmdInSock.connect("ipc:///tmp/" + pwd.getpwuid(os.getuid()).pw_name + "-workerCommandRequest")

    cmdOutSock = context.socket(zmq.PULL)
    cmdOutSock.bind("ipc:///tmp/" + pwd.getpwuid(os.getuid()).pw_name + "-workerCommandResponse")

    statusSock = context.socket(zmq.PULL)
    statusSock.bind("ipc:///tmp/" + pwd.getpwuid(os.getuid()).pw_name + "-statusUpdate")

    msgGetRfOnline = '{"messageType":"rfManagerGetOnline"}'
    msgSetRfOnlineTrue = '{"messageType":"rfManagerSetOnline","isOnline":true}'
    msgSetRfOnlineFalse = '{"messageType":"rfManagerSetOnline","isOnline":false}'

    poller = zmq.Poller()
    poller.register(cmdOutSock, zmq.POLLIN)
    poller.register(statusSock, zmq.POLLIN)

    counter = 0

    while True:
        socks = dict(poller.poll(1000))
        if cmdOutSock in socks and socks[cmdOutSock] == zmq.POLLIN:
            message = cmdOutSock.recv()
            if(counter % PRINT_COUNT == 0):
                print("cmdOutSock Rcv (", time.time(), "): ", message)

        if statusSock in socks and socks[statusSock] == zmq.POLLIN:
            message = statusSock.recv()
            if(counter % PRINT_COUNT == 0):
                print("statusSock Rcv (", time.time(), "): ", message)

        if(counter % PRINT_COUNT == 0):
            print("Sending Get RF Online (", time.time(), ")...")
        cmdInSock.send_string(msgGetRfOnline)

        counter += 1
        if(counter % 20 == 0):
            cmdInSock.send_string(msgSetRfOnlineTrue)
        if(counter % 20 == 10):
            cmdInSock.send_string(msgSetRfOnlineFalse)
