#!/usr/bin/env python3

import zmq, time, numpy, pwd, os

DWELL_TIME = 0.05

if __name__ == '__main__':
    context = zmq.Context()

    #  Socket to talk to server
    socket = context.socket(zmq.REQ)
    socket.setsockopt(zmq.LINGER, 10)
    socket.setsockopt(zmq.SNDTIMEO, 10)
    socket.setsockopt(zmq.RCVTIMEO, 10)
    socket.connect("ipc:///tmp/" + pwd.getpwuid(os.getuid()).pw_name + "-fpgaInterfaceReqRep")

    msgR = '{' \
        '"messageType":"fpgaRequestRead",' \
        '"hexAddress":"0000000z"}'

    msgRa = '{' \
        '"messageType":"fpgaRequestReadRange",' \
        '"startHexfAddress":"00000000",' \
        '"endHexAddress":"00000007"}'

    msgRb = '{' \
        '"messageTypey":"fpgaRequestReadBatch",' \
        '"hexAddresses": [' \
        '"00000000",' \
        '"00000004",' \
        '"00000010"' \
        ']}'

    msgW = '{' \
        '"messageType":"fpgaRequestWrite",' \
        '"hexAddress":"00000000",' \
        '"daqta":"2000"}'

    print(msgR)
    timeR = time.time()
    socket.send_string(msgR)
    resp = socket.recv()
    timeRE = time.time() - timeR
    print(resp)
    print(timeRE)

    time.sleep(DWELL_TIME)

    print(msgRa)
    timeRa = time.time()
    socket.send_string(msgRa)
    resp = socket.recv()
    timeRaE = time.time() - timeRa
    print(resp)
    print(timeRaE)

    time.sleep(DWELL_TIME)

    print(msgRb)
    timeRb = time.time()
    socket.send_string(msgRb)
    resp = socket.recv()
    timeRbE = time.time() - timeRb
    print(resp)
    print(timeRbE)

    time.sleep(DWELL_TIME)

    print(msgW)
    timeW = time.time()
    socket.send_string(msgW)
    resp = socket.recv()
    timeWE = time.time() - timeW
    print(resp)
    print(timeWE)

    time.sleep(DWELL_TIME)

