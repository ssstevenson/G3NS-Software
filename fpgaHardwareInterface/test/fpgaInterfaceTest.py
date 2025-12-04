#!/usr/bin/env python3

import zmq, time, numpy, pwd, os

PRINT_ALL = False
QUERY_COUNT = 100
DWELL_TIME = 0.05

def condPrint(str):
    if(PRINT_ALL):
        print(str)

if __name__ == '__main__':
    context = zmq.Context()

    #  Socket to talk to server
    socket = context.socket(zmq.REQ)
    socket.setsockopt(zmq.LINGER, 25)
    socket.setsockopt(zmq.SNDTIMEO, 25)
    socket.setsockopt(zmq.RCVTIMEO, 25)
    socket.connect("ipc:///tmp/" + pwd.getpwuid(os.getuid()).pw_name + "-fpgaInterfaceReqRep")

    msgR = '{' \
        '"messageType":"fpgaRequestRead",' \
        '"hexAddress":"00000000"}'

    msgRa = '{' \
        '"messageType":"fpgaRequestReadRange",' \
        '"startHexAddress":"00000000",' \
        '"endHexAddress":"00000007"}'

    msgRb = '{' \
        '"messageType":"fpgaRequestReadBatch",' \
        '"hexAddresses": [' \
        '"00000000",' \
        '"00000004",' \
        '"00000010"' \
        ']}'

    msgW = '{' \
        '"messageType":"fpgaRequestWrite",' \
        '"hexAddress":"00000000",' \
        '"data":"2000"}'

    readTimes = []
    readArrayTimes = []
    readBatchTimes = []
    writeTimes = []

    readTimesMax = -1
    readArrayTimesMax = -1
    readBatchTimesMax = -1
    writeTimesMax = -1

    for x in range(int(QUERY_COUNT / 4)):
        condPrint(msgR)
        timeR = time.time()
        socket.send_string(msgR)
        resp = socket.recv()
        timeRE = time.time() - timeR
        readTimesMax = max(readTimesMax, timeRE)
        readTimes.append(timeRE)
        condPrint(resp)
        condPrint(timeRE)

        time.sleep(DWELL_TIME)

        condPrint(msgRa)
        timeRa = time.time()
        socket.send_string(msgRa)
        resp = socket.recv()
        timeRaE = time.time() - timeRa
        readArrayTimesMax = max(readArrayTimesMax, timeRaE)
        readArrayTimes.append(timeRaE)
        condPrint(resp)
        condPrint(timeRaE)

        time.sleep(DWELL_TIME)

        condPrint(msgRb)
        timeRb = time.time()
        socket.send_string(msgRb)
        resp = socket.recv()
        timeRbE = time.time() - timeRb
        readBatchTimesMax = max(readBatchTimesMax, timeRbE)
        readBatchTimes.append(timeRbE)
        condPrint(resp)
        condPrint(timeRbE)

        time.sleep(DWELL_TIME)

        condPrint(msgW)
        timeW = time.time()
        socket.send_string(msgW)
        resp = socket.recv()
        timeWE = time.time() - timeW
        writeTimesMax = max(writeTimesMax, timeWE)
        writeTimes.append(timeWE)
        condPrint(resp)
        condPrint(timeWE)

        time.sleep(DWELL_TIME)

    print("Read:      ", int(numpy.average(readTimes, axis = 0) * 1000000), \
        "uS (", int(numpy.std(readTimes, axis = 0) * 1000000), ") <>", \
        int(readTimesMax * 1000000))
    print("Read Array:", int(numpy.average(readArrayTimes, axis = 0) * 1000000), \
        "uS (", int(numpy.std(readArrayTimes, axis = 0) * 1000000), ") <>", \
        int(readArrayTimesMax * 1000000))
    print("Read Batch:", int(numpy.average(readBatchTimes, axis = 0) * 1000000), \
        "uS (", int(numpy.std(readBatchTimes, axis = 0) * 1000000), ") <>", \
        int(readBatchTimesMax * 1000000))
    print("Write:     ", int(numpy.average(writeTimes, axis = 0) * 1000000), \
        "uS (", int(numpy.std(writeTimes, axis = 0) * 1000000), ") <>", \
        int(writeTimesMax * 1000000))
