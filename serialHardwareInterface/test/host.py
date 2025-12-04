#!/usr/bin/env python3

import serial, time

TIMEOUT=0.01

def readSerial(ser, timeout = 0):
    data = bytes()
    readTime = time.time()
    while True:
        datum = ser.read()
        if len(datum) > 0:
            data += datum
            readTime = time.time()
            if len(data) >= 6:
                if len(data) >= data[2] + 3:
                    return data
        if len(data) > 1 and (time.time() - readTime) > timeout * 2.1:
            return data

if __name__ == '__main__':
    with serial.Serial(port="/dev/ttyS0", baudrate=115200, timeout=TIMEOUT) as ser:
        pingData = bytes("\x00\x00\x03\x00\x00\x03".encode())
        statusData = bytes("\x00\x00\x03\x00\x02\x01".encode())
        exitData = bytes("exit".encode())

        ser.write(pingData)
        print(readSerial(ser, TIMEOUT))
        for i in range(10):
            ser.write(statusData)
            print(readSerial(ser, TIMEOUT))
        ser.write(exitData)