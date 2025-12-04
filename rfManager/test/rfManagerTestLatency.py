#!/usr/bin/env python3

import zmq, time, json, numpy, sys, pwd, os
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from optparse import OptionParser

if __name__ == '__main__':
    doneTimes = []
    outputStr = ""

    parser = OptionParser()
    parser.add_option("-t", "--text", dest="textFilename", help="Output Summary Text File", metavar="FILE", default="summary.txt")
    parser.add_option("-p", "--pic", dest="picFilename", help="Histogram Image File", metavar="FILE", default="hist.png")
    parser.add_option("-c", "--cnt", type="int", dest="count", default=100000)
    (options, args) = parser.parse_args()

    context = zmq.Context()
    with zmq.Context() as context:
        with context.socket(zmq.PUB) as cmdInSock:
            # cmdInSock.connect("ipc:///tmp/" + pwd.getpwuid(os.getuid()).pw_name + "-workerCommandRequest")
            # cmdInSock.connect("tcp://10.10.1.215:15000")
            cmdInSock.bind("tcp://*:15000")
            cmdInSock.setsockopt(zmq.LINGER, 0)

            with context.socket(zmq.PULL) as cmdOutSock:
                # cmdOutSock.bind("ipc:///tmp/" + pwd.getpwuid(os.getuid()).pw_name + "-workerCommandResponse")
                # cmdOutSock.connect("tcp://10.10.1.215:20000")
                cmdOutSock.bind("tcp://*:20000")
                cmdOutSock.setsockopt(zmq.LINGER, 0)

                msgGetRfOnline = '{"messageType":"rfManagerGetOnline","testSequenceNumber":'
                msgSetRfOnlineTrue = '{"messageType":"rfManagerSetOnline","isOnline":true,"testSequenceNumber":-1}'
                msgSetRfOnlineFalse = '{"messageType":"rfManagerSetOnline","isOnline":false,"testSequenceNumber":-1}'

                for i in range(10):
                    print(i)
                    time.sleep(1)
                print("Getting a move on...")

                poller = zmq.Poller()
                poller.register(cmdOutSock, zmq.POLLIN)

                counter = 0
                curTime = time.time()
                waitDoneMap = dict()
                first = True

                for i in range(options.count+1):
                    if i % 10000 == 0:
                        print(i)
                    socks = dict(poller.poll(1000))
                    if cmdOutSock in socks and socks[cmdOutSock] == zmq.POLLIN:
                        message = cmdOutSock.recv()
                        curTime = time.time()
                        data = json.loads(message)
                        seqNum = data["request"]["testSequenceNumber"]
                        if seqNum != -1:
                            if seqNum in waitDoneMap:
                                curTime -= waitDoneMap.pop(seqNum)
                                doneTimes.append(curTime)
                    else:
                        if first:
                            first = False
                        else:
                            print("ERROR!!!")
                            sys.exit(1)

                    curCmd = msgGetRfOnline + str(counter) + '}'
                    curTime = time.time()
                    cmdInSock.send_string(curCmd)
                    waitDoneMap[counter] = curTime

                    counter += 1

    plt.figure(figsize=(16,9), frameon=False)

    doneTimes = sorted(doneTimes)

    outputStr += "Full Data ->" + "\n"
    outputStr += "  Data Length: " + str(len(doneTimes)) + "\n"
    outputStr += "  Average: " + str(numpy.average(doneTimes, axis = 0)) + "\n"
    outputStr += "  Std Dev: " + str(numpy.std(doneTimes, axis = 0)) + "\n"
    outputStr += "  Min Val: " + str(numpy.amin(doneTimes)) + "\n"
    outputStr += "  Max Val: " + str(numpy.amax(doneTimes)) + "\n"
    outputStr += "  Cnt > 1 ms: " + str(sum(i >= 0.001 for i in doneTimes)) + "\n"
    outputStr += "  Cnt > 10 ms: " + str(sum(i >= 0.01 for i in doneTimes)) + "\n"

    plt.subplot(611)
    plt.hist(doneTimes, bins=100)  # arguments are passed to np.histogram
    plt.title("Full Data")

    # 5-9 Data Inclusive
    g = int(0.00001 * len(doneTimes))
    cutData = doneTimes[:-g]

    outputStr += "5-9 Data Inclusive ->" + "\n"
    outputStr += "  Data Length: " + str(len(cutData)) + "\n"
    outputStr += "  Average: " + str(numpy.average(cutData, axis = 0)) + "\n"
    outputStr += "  Std Dev: " + str(numpy.std(cutData, axis = 0)) + "\n"
    outputStr += "  Min Val: " + str(numpy.amin(cutData)) + "\n"
    outputStr += "  Max Val: " + str(numpy.amax(cutData)) + "\n"
    outputStr += "  Cnt > 1 ms: " + str(sum(i >= 0.001 for i in cutData)) + "\n"
    outputStr += "  Cnt > 10 ms: " + str(sum(i >= 0.01 for i in cutData)) + "\n"

    plt.subplot(612)
    plt.hist(cutData, bins=100)  # arguments are passed to np.histogram
    plt.title("5-9 Data Inclusive")

    # 2-9 Data Exclusive
    g = int(0.01 * len(doneTimes))
    cutData = doneTimes[-g:]

    outputStr += "2-9 Data Exclusive ->" + "\n"
    outputStr += "  Data Length: " + str(len(cutData)) + "\n"
    outputStr += "  Average: " + str(numpy.average(cutData, axis = 0)) + "\n"
    outputStr += "  Std Dev: " + str(numpy.std(cutData, axis = 0)) + "\n"
    outputStr += "  Min Val: " + str(numpy.amin(cutData)) + "\n"
    outputStr += "  Max Val: " + str(numpy.amax(cutData)) + "\n"
    outputStr += "  Cnt > 1 ms: " + str(sum(i >= 0.001 for i in cutData)) + "\n"
    outputStr += "  Cnt > 10 ms: " + str(sum(i >= 0.01 for i in cutData)) + "\n"

    plt.subplot(613)
    plt.hist(cutData, bins=100)  # arguments are passed to np.histogram
    plt.title("2-9 Data Exclusive")

    # 3-9 Data Exclusive
    g = int(0.001 * len(doneTimes))
    cutData = doneTimes[-g:]

    outputStr += "3-9 Data Exclusive ->" + "\n"
    outputStr += "  Data Length: " + str(len(cutData)) + "\n"
    outputStr += "  Average: " + str(numpy.average(cutData, axis = 0)) + "\n"
    outputStr += "  Std Dev: " + str(numpy.std(cutData, axis = 0)) + "\n"
    outputStr += "  Min Val: " + str(numpy.amin(cutData)) + "\n"
    outputStr += "  Max Val: " + str(numpy.amax(cutData)) + "\n"
    outputStr += "  Cnt > 1 ms: " + str(sum(i >= 0.001 for i in cutData)) + "\n"
    outputStr += "  Cnt > 10 ms: " + str(sum(i >= 0.01 for i in cutData)) + "\n"

    plt.subplot(614)
    plt.hist(cutData, bins=100)  # arguments are passed to np.histogram
    plt.title("3-9 Data Exclusive")

    # 4-9 Data Exclusive
    g = int(0.0001 * len(doneTimes))
    cutData = doneTimes[-g:]

    outputStr += "4-9 Data Exclusive ->" + "\n"
    outputStr += "  Data Length: " + str(len(cutData)) + "\n"
    outputStr += "  Average: " + str(numpy.average(cutData, axis = 0)) + "\n"
    outputStr += "  Std Dev: " + str(numpy.std(cutData, axis = 0)) + "\n"
    outputStr += "  Min Val: " + str(numpy.amin(cutData)) + "\n"
    outputStr += "  Max Val: " + str(numpy.amax(cutData)) + "\n"
    outputStr += "  Cnt > 1 ms: " + str(sum(i >= 0.001 for i in cutData)) + "\n"
    outputStr += "  Cnt > 10 ms: " + str(sum(i >= 0.01 for i in cutData)) + "\n"

    plt.subplot(615)
    plt.hist(cutData, bins=100)  # arguments are passed to np.histogram
    plt.title("4-9 Data Exclusive")

    # 5-9 Data Exclusive
    g = int(0.00001 * len(doneTimes))
    cutData = doneTimes[-g:]

    outputStr += "5-9 Data Exclusive ->" + "\n"
    outputStr += "  Data Length: " + str(len(cutData)) + "\n"
    outputStr += "  Average: " + str(numpy.average(cutData, axis = 0)) + "\n"
    outputStr += "  Std Dev: " + str(numpy.std(cutData, axis = 0)) + "\n"
    outputStr += "  Min Val: " + str(numpy.amin(cutData)) + "\n"
    outputStr += "  Max Val: " + str(numpy.amax(cutData)) + "\n"
    outputStr += "  Cnt > 1 ms: " + str(sum(i >= 0.001 for i in cutData)) + "\n"
    outputStr += "  Cnt > 10 ms: " + str(sum(i >= 0.01 for i in cutData)) + "\n"

    plt.subplot(616)
    plt.hist(cutData, bins=100)  # arguments are passed to np.histogram
    plt.title("5-9 Data Exclusive")

    plt.subplots_adjust(hspace=0.6)
    plt.savefig(options.picFilename, dpi=100)

    with open(options.textFilename, 'w') as fd:
        fd.write(outputStr)
