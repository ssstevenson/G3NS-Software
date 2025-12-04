#!/usr/bin/env python3

import zmq, time, pwd, os

if __name__ == '__main__':
    context = zmq.Context()

    #  Socket to talk to server
    socket = context.socket(zmq.REQ)
    socket.setsockopt(zmq.LINGER, 25)
    socket.setsockopt(zmq.SNDTIMEO, 25)
    socket.setsockopt(zmq.RCVTIMEO, 25)
    socket.connect("ipc:///tmp/" + pwd.getpwuid(os.getuid()).pw_name + "-serialInterfaceAPI")

    msg = '{' \
        '"messageType":"serialRequest",' \
        '"data": "48,65,6c,6c,6f,20,57,6f,72,6c,64,21,21,21"' \
        '}'

    socket.send_string(msg)
