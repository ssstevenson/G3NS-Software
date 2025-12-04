#!/usr/bin/env python3

import serial, time, random

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
                return data
        if len(data) > 1 and (time.time() - readTime) > timeout * 2.1:
            return data

def calcChecksum(data):
    chksum = 0
    for i in data:
        chksum ^= i
    return chksum

def handlePing(ser, packet):
    ser.write(packet)

    return True

def handleExit(ser, packet):
    return False

def splitHeader(packet):
    return (packet[:2], packet[3:-1])

def handleGetStatus(ser, packet):
    (header, packet) = splitHeader(packet)

    # Currents
    for i in range(4):
        packet += random.randint(0, 65536).to_bytes(2, 'big')
    # Total Current
    packet += random.randint(0, 65536).to_bytes(2, 'big')
    # Input Voltage
    packet += random.randint(0, 65536).to_bytes(2, 'big')
    # Temperature
    packet += random.randint(0, 65536).to_bytes(2, 'big')
    # Alarm State
    packet += random.randint(0, 256).to_bytes(1, 'big')
    # High Alarm Flags
    packet += random.randint(0, 65536).to_bytes(2, 'big')
    # High Warning Flags
    packet += random.randint(0, 65536).to_bytes(2, 'big')
    # Low Alarm Flags
    packet += random.randint(0, 65536).to_bytes(2, 'big')
    # Low Warning Flags
    packet += random.randint(0, 65536).to_bytes(2, 'big')

    packet += calcChecksum(packet).to_bytes(1, 'big')

    pktSize = len(packet).to_bytes(1, 'big')

    packet = header + pktSize + packet

    ser.write(packet)

    return True

if __name__ == '__main__':
    funcMap = {
        b'\x00\x00\x03\x00\x00\x03': handlePing,
        b'\x00\x00\x03\x00\x02\x01': handleGetStatus,
        b'exit': handleExit,
    }

    with serial.Serial(port="/dev/ttyS1", baudrate=115200, timeout=TIMEOUT) as ser:
        data = bytes()
        readTime = time.time()
        keepRunning = True
        while keepRunning:
            msg = readSerial(ser, TIMEOUT)
            if msg in funcMap:
                keepRunning = funcMap[msg](ser, msg)
