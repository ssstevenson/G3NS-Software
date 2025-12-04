#pragma once

#include <unistd.h>
#include <cstring>
#include <chrono>
#include <cstring>
#include <iostream>
#include <set>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <functional>
#include <thread>
#include "mcastProtocol.h"

using namespace std;
class AckSocket {

public:
    AckSocket(  bool isController , int port =0);
    ~AckSocket();
    unsigned int getMyIP() { return myIp;};
    int reply( void *msg, int msgLen,  unsigned int senderIp, uint16_t senderPort);
    int getMyIPAddress();
    int waitForReply( AckPacket &reply, long int &recvBytes);
    //
    // Before sending unsigned short and unsigned 32
    // convert to net or to host endianess
    //
    void convertPacketFromNetwork(AckPacket &pkt);
    void convertPacketToNetwork(AckPacket &pkt);

private:
    int createServerSocket();
    int createReplySocket();
    int openSocket();
    int createSocket();
private:
    int ack_sock;
    unsigned int myIp;
    string myIpStr;
    uint16_t ackport;
    bool isController;
    string interface;


};
