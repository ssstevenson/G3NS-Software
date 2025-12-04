#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"

#include "McastCommon.h"


unsigned  int Mcast::sequenceNumber = 1;

Mcast::Mcast(const std::string& mcat_addr, unsigned short mcastport, unsigned short ackport,  bool isController, int NbBoosters):
    ackSock(isController, ackport),
    mcast_socket{-1},
    mcat_address{mcat_addr},
    mcast_port{mcastport},
    controller{isController},
    multicast_addr{},
    multicastRequest{},
    joined{false},
    numberOfBoosters{NbBoosters},
    interface{IFACE},
    myIP{0},
    ackPort(ackport),
    callback{}, action{}, myThread{},
    running{true},
    lastSequence{0},
    lastCmd{0},
    boostersIP{}


{
   int ret = controller ? createCtrlSocket() : createListnerSocket();

   if (ret > 0 ) {
       if  ( mcast_socket > 0 ) {
           leaveGroup();
           close(mcast_socket);
           mcast_socket = -1;
       }
   }

   myIP = ackSock.getMyIP();
}

Mcast::~Mcast() {
    if  ( mcast_socket > 0 ) {
        leaveGroup();
        close(mcast_socket);
        mcast_socket = -1;
    }
    running = false;
}

void Mcast::boosterStart()
{
    if (!controller)
    {
        // Start a thread for Booster that listn to Mcast packet and respond
        if (myThread.joinable())
        {
            myThread.detach();
        }
        myThread = std::thread(
        [this]() { this->ProcessMcastCommand(*this, this->getAckSocket());}
                   );
        myThread.join();
        myThread = std::thread();
    }
}

void Mcast::boosterStop()
{
    if (!controller)
    {
        running = false;
        if (myThread.joinable())
        {
            myThread.detach();
        }
    }
    myThread = std::thread();
}

int Mcast::createSocket()
{
    mcast_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mcast_socket < 0) {
        syslog(LOG_ERR, "[%s] Error Creating Socket", __FUNCTION__);
        return -1;
    }
    int val = 1;
    if ( setsockopt(mcast_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
        syslog(LOG_ERR, "[%s]  Socket option  SO_REUSEADDR  Failed ", __FUNCTION__);
    }

    if ( setsockopt(mcast_socket, SOL_SOCKET, SO_BINDTODEVICE, interface.c_str(), static_cast<socklen_t> ( interface.size()) ) < 0) {
        syslog(LOG_ERR, "[%s]  Socket option  SO_BINDTODEVICE  Failed ", __FUNCTION__);
    }

#ifdef TEST_ON_SAME_HOST

    if ( setsockopt(mcast_socket, SOL_SOCKET, IP_MULTICAST_LOOP, &val, sizeof(val)) < 0) {
        syslog(LOG_ERR, "[%s]  Socket option  IP_MULTICAST_LOOP  Failed ", __FUNCTION__);
    }
#endif
    return 0;
}

int Mcast::createCtrlSocket()
{

    if ( createSocket() < 0) {
        return -1;
    }
    // Enable multicast TTL = 1 (same subnet) or 2 if multiple hops
    uint8_t ttl = 2;
    if (setsockopt(mcast_socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        syslog(LOG_ERR, "[%s] Error Setting TTL", __FUNCTION__);
        return 1;
    }

    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_port = htons(mcast_port);
    inet_pton(AF_INET, MULTICAST_GROUP, &multicast_addr.sin_addr);

    return 0;
}

int Mcast::createListnerSocket()
{
    int ret = 0;
    ret = createSocket() ;
    if (ret < 0 ) {
        return -1;
    }

    if (!ret) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(mcast_port);
        addr.sin_addr.s_addr = INADDR_ANY;

        ret = bind(mcast_socket, (sockaddr*)&addr, sizeof(addr));
    }

    if ( ret == 0 ) {
        ret = joinGroup();
    }
    else {
        Close();
        syslog(LOG_ERR, "[%s] Error Creating Socket", __FUNCTION__);
    }

  return ret;
}


int  Mcast::joinGroup()
{
    // used by Listner
    inet_pton(AF_INET, mcat_address.c_str(), &multicastRequest.imr_multiaddr);
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

    int ret = setsockopt(mcast_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*)&multicastRequest, sizeof(multicastRequest));
    if (ret == 0 ) {
        joined = true;
    }
    else {
        syslog(LOG_ERR, "[%s] Error Joining Mcasr Group %s", __FUNCTION__, mcat_address.c_str() );
    }

    return ret;
}

int  Mcast::leaveGroup()
{
    int ret = 0;
    inet_pton(AF_INET, mcat_address.c_str(), &multicastRequest.imr_multiaddr);
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
   if (joined) {
       ret =  setsockopt(mcast_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,  (const void*) &multicastRequest,sizeof(multicastRequest) );
       joined = false;
   }
   return ret;

}
CommandPacket Mcast::formMcastCommand( CommandID  cmd, unsigned int value )
{
    sequenceNumber++;
    CommandPacket cmdMessage{};
    // form message
    cmdMessage.commandId = cmd;
    cmdMessage.value =  value;
    cmdMessage.ctrlIP = getMyIP();
    cmdMessage.ctrlPort = getAckPort();
    cmdMessage.seqNumber =  sequenceNumber;
    // send message
    convertPacketToNetwork(cmdMessage);
   return cmdMessage;
}

//
//
//
int Mcast::sendMcastMessage( const void *cmd, int cmdlen )
{
    long int ret = -1;
    if (mcast_socket)
    {
       ret = sendto(mcast_socket, cmd, static_cast <size_t> (cmdlen), 0, (sockaddr*)&multicast_addr, sizeof(multicast_addr));
    }
    return ret;
}
//
// This is called by Booster to process Mcast message
// return 0 is a message is received
// return  -1 if timed out or an error
int Mcast::waitForMcastCommand( CommandPacket &cmdMessage, long int &recvBytes)
{
    int ret = 0;
    struct pollfd mcastFd{mcast_socket, POLLIN, 0};
    const int timeout_ms = POLL_WAIT_IN_MS;

   recvBytes = 0;
    int status = poll(&mcastFd, 1, timeout_ms);

    if (status < 0 )  {
        syslog(LOG_ERR, "[%s] Poll Failed ", __FUNCTION__ );
        Close();
        // recreate Socket
        createListnerSocket();
        ret = -1;
    }
    else if (status > 0 )
    {
        // Got an a Message, Check if it MCAST
        if (mcastFd.revents & POLLIN)
        {
            sockaddr_in from{};
            socklen_t fromlen = sizeof(from);

            long int rc = recvfrom(mcast_socket, &cmdMessage, sizeof(cmdMessage), 0, (sockaddr*)&from, &fromlen);
            if (rc < 0)
            {
                Close();
                // recreate Socket
                createListnerSocket();
                ret = -1;
            }
            else {
                recvBytes = rc;
            }
        }
    }
    else {
        // No message, timed out
        ret = -1;
    }

    return ret;

}

//  Booster method only
//  This is the main entry point for Booster to wait and process Multicast messages
//   THIS IS A BLOCKIG CALL and SHOULD BE called from a thread

int Mcast::ProcessMcastCommand( Mcast &mcastScok, AckSocket &ackScok )
{
    CommandPacket cmdMessage{};
    long int   cmdlen = 0;
    int ret = 0;

    while ( running ) {
        cmdlen = 0;
        ret = mcastScok.waitForMcastCommand( cmdMessage, cmdlen);
        if ( ( ret == 0 ) && (cmdlen > 0 ) )
        {
            //
            if (cmdlen >= static_cast < long int> ( sizeof(cmdMessage)) )
            {
                convertPacketFromNetwork(cmdMessage);
                // Prepare reply
                AckPacket ack{};
                // Form response Ack Message
                ack.commandId = cmdMessage.commandId;
                ack.boosterIP = ackScok.getMyIP();

#ifdef TEST_ON_SAME_HOST
                // Add random number to IP
                srand(static_cast<unsigned int>(time(NULL)));
                int randomNumber = rand() % 100 + 1;
                ack.boosterIP &= 0xFFFFFF00;
                ack.boosterIP += randomNumber;
#endif
                ack.seqNumber = cmdMessage.seqNumber;
                // get sender IP and Port from the message
                unsigned int  senderPort  = cmdMessage.ctrlPort;
                unsigned int senderIp    = cmdMessage.ctrlIP;
                //
                // Process the command and Send a reply over ACK socket
                // Check if we already processed this command
                // The lastCmd and lastSequence are initialized to zero
                // In case of Controller, they are set when it send a new command
                // in case of booster, it is set here when the command is received
                //
                // Check if this booster already received same comamnd before
                //
                if ( (ack.commandId != lastCmd) ||  (ack.seqNumber != lastSequence))
                {
                    // We got new CMD to process
                    mcastScok.boosterProcessCommand( static_cast <CommandID> (ack.commandId), cmdMessage.value);
#ifdef DEBUG
                     printf("[%s] ---BoosterIP = %u received Command ...CommandID = %d  -- Sequence Number=%d \n",
                            __FUNCTION__, ack.boosterIP,  ack.commandId, ack.seqNumber );
#endif
                    // send a reply
                    ackScok.convertPacketToNetwork(ack);
                    ackScok.reply( &ack, sizeof(ack), senderIp,senderPort);
                    // Set last command and sequence number  received
                    lastCmd = ack.commandId;
                    lastSequence = ack.seqNumber;
                }
                else {
                    // send a reply again anyway, in case it is dropped
                    // Won/t hurt because Controller can receive duplicate acks from same booster
                    ackScok.reply( &ack, sizeof(ack), senderIp, static_cast <uint16_t> (senderPort));
                }

            }

        }
    }
    return ret;
}

void  Mcast::boosterProcessCommand(CommandID cmdID, [[maybe_unused]] unsigned int val )
{
    switch (cmdID) {
        case ALC_MODE:
            // RF manager switch ALC mode
            break;
        case AGC_MODE:
            // RF manager switch AGC mode
            break;
        case MGC_MODE:
             // RF manager switch MGC mode
            break;
        case TARGET_POWER:
              // RF manager set target Power
              // Val is target power in % or Dbm
              // float fval = static_cast  < float > (val );
            break;
        case DETECTION_MODE:
              // RF manager change Detection Mode
              // Val  can indicate the Mode  0:RMS, 1: PEAK 2:FAST PEAK, ...5: Envelope

            break;
    }
}

//
// This is the main call to broadcast a message fro the Controller to all Booster
//
//  THIS IS A Blocking call

int Mcast::ControllerBcastCommand( void *cmd, int cmdLen )
{
    if ( !cmd )  return 0;

    CommandPacket *pCmd = static_cast<CommandPacket *> (cmd);

    // Set last commandId and last SequenceNumber from packet
    // This needed to check response from Booster
    lastSequence = (*pCmd).seqNumber;
    lastCmd = (*pCmd).commandId;

    if (action) {
        action();   // Action could be like blanking
    }
    boostersIP.clear();
    int retries = 0;
    int boosterReplied = 0;
    // Start a thread to send Mcat Connad and wait for response
    // Re-send command till all booster respond or time out
    // Make sure a call back is registered

    sendMcastMessage(cmd,cmdLen);
    while (true)
    {
        AckPacket ack{};
        long int rcvBytes = 0;
        // Wait for a reply from booster
        // We are reading one ACK at a time..
        // So rcvBytes should not have more than sizeof (AckPacket)
        int ret =  ackSock.waitForReply(ack, rcvBytes);

        if ( (ret == 0 ) &&  ( rcvBytes >= sizeof (ack) )  )
        {
            ackSock.convertPacketFromNetwork(ack);
            boosterReplied = processReply(ack);
            if ( boosterReplied == numberOfBoosters )
            {
                // All boosterd have executed last command
                if (callback)
                {
                    callback(true, boosterReplied);
                }
                break;
            }

        }
        else {
            // try sending Mcast Message again
            sendMcastMessage(cmd,cmdLen);
            if (retries++ > maxRetries ) {
                if (callback)
                {
                    callback(false, boosterReplied);
                }
                break;
            }

        }

    }


    return boosterReplied;

}

// Controller process a reply
int Mcast::processReply(AckPacket &reply)
{
// If command is like command and
//
 if ( ( reply.commandId == lastCmd) && (reply.seqNumber == lastSequence))
 {
    boostersIP.insert(reply.boosterIP) ;
 }
 return boostersIP.size();
}
void Mcast::Close() {
    if  ( mcast_socket > 0 ) {
        leaveGroup();
        ::close(mcast_socket);
        mcast_socket = -1;
    }
}


void Mcast::convertPacketFromNetwork(CommandPacket &pkt) {
    uint32_t *p = reinterpret_cast<uint32_t*>(&pkt);
    size_t n = sizeof(CommandPacket) / sizeof(uint32_t);

    for (size_t i = 0; i < n; i++) {
        p[i] = ntohl(p[i]);
    }
}

void Mcast::convertPacketToNetwork(CommandPacket &pkt) {
    uint32_t *p = reinterpret_cast<uint32_t*>(&pkt);
    size_t n = sizeof(CommandPacket) / sizeof(uint32_t);

    for (size_t i = 0; i < n; i++) {
        p[i] = htonl(p[i]);
    }
}

#pragma GCC diagnostic pop
