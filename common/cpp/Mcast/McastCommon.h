#pragma once

#include <chrono>
#include <cstring>
#include <iostream>
#include <set>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <string>
#include <functional>
#include <thread>
#include "mcastProtocol.h"
#include "AckSocket.h"

using namespace std;

class Mcast  {
public:
    Mcast() = delete;
    Mcast(const std::string& mcat_addr, unsigned short mcastport, unsigned short replyPort,  bool isController, int NbBoosters);
    // form  packet and send message
    virtual ~Mcast();
    virtual CommandPacket formMcastCommand( CommandID  cmdID, unsigned int value );
    // packet is already formatted, send Mcast Message to all boosters over Mcast socket
    virtual int sendMcastMessage( const void *cmd, int cmdlen );
    // Controller call action before sending command over mcasr socket
    int  ControllerBcastCommand( void *cmd, int cmdLen ) ;
    //

    // This is specifc implementaion of what booster sould for each command
    //
    void boosterStart();
    void boosterStop();
    virtual void boosterProcessCommand(CommandID cmdID, unsigned int val );

    //####################################################################
    // This callback is called when MCAST message either pass or timeout
    // the boolean = true means pass  --- false=fail
    // the int in the number of boosters that have executed the message
   // ################################################################
    using CallbackType = std::function<void(bool, int)>;
    void setCallBack(CallbackType call) { callback= std::move(call);}
    //########################################################
    // This Action is called prior to Sending MCAST Message
    //##########################################################
    using Action = std::function<void()>;
    void setAction(Action act) { action= std::move(act);}

    //
    AckSocket & getAckSocket() { return ackSock;};
protected:
    AckSocket  ackSock;
    const int maxRetries = MAX_TRANSMIT_RETIES;
    //
    virtual int ProcessMcastCommand( Mcast &mcastScok, AckSocket &ackScok );
    unsigned short getAckPort() { return ackPort;};
    //
private: // Methods
     int  joinGroup();
     int  leaveGroup();
     int  createSocket();
     int createListnerSocket();
     int createCtrlSocket();
     void Close();
     void join() {
         if (myThread.joinable()) {
             myThread.join();
         }
     }
     int waitForMcastCommand( CommandPacket &cmdMessage, long int &recvBytes);
     unsigned int getMyIP() { return myIP;};
     int processReply(AckPacket &reply);

     //
     //
     void convertPacketFromNetwork(CommandPacket &pkt);
     void convertPacketToNetwork(CommandPacket &pkt);

 private:  // data
    int mcast_socket;
    string mcat_address;
    unsigned  short mcast_port;
    bool  controller;
    sockaddr_in multicast_addr;
    struct ip_mreq multicastRequest;
    bool joined;
    // Used By Controller only
    int numberOfBoosters;
    static unsigned  int sequenceNumber;
    string  interface;
    unsigned int myIP;
    unsigned short ackPort;
    //
    CallbackType callback;
    Action    action;
    std::thread myThread;
    bool running;
    uint32_t  lastSequence;
    uint32_t lastCmd;
    // Contains booster that responded to last commands
    std::set<uint32_t> boostersIP;

};

