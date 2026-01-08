#include <sys/socket.h>
#include <cstring>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sstream>
#include <stdlib.h>
#include "Timer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored  "-Wreorder"
#pragma GCC diagnostic ignored  "-Wnarrowing"
#pragma GCC diagnostic ignored  "-Wconversion"

#include "m2mUDP.h"

// Define Static parameters
UDPServer *UDPServer::myInstance = nullptr;
std::mutex UDPServer::m2m_mutex;

// Desttructor..
UDPServer::~UDPServer()
{
    if (userThread.joinable()) {
        userThread.join();
    }
    myInstance = nullptr;
}


UDPServer *UDPServer::getMyInstance( unsigned short port )
{
    std::lock_guard<std::mutex> lock(m2m_mutex);

    if (myInstance == nullptr )
    {
        myInstance = new UDPServer( port);
        syslog(LOG_DEBUG, "  [%s]-->  First Instance  port  = %d ", __FUNCTION__, port);
    }
    else
    {
        unsigned short currPort = myInstance->getServerPort();
        if (currPort != port )
        {
            syslog(LOG_DEBUG, "  [%s]-->  New port  %d  --- current Port =%d ", __FUNCTION__, port, currPort);
            myInstance->setRunning(false);
            myInstance->stopNotificationThread();
            if (myInstance->userThread.joinable() )
            {
                myInstance->userThread.join();
            }

            if (myInstance)
            {
                delete myInstance;
            }
            myInstance = new UDPServer( port);
        }
    }

    return myInstance;
}
//Ctors
UDPServer::UDPServer( unsigned short port):UDPSocket(port),
    isrunning(false),
    sourceAddress{},
    sourcePort(0),
    serverPort(port)
{
    myInstance = this;
}
// Thread
void UDPServer::startThread()
{
    printf(".......starting UDP Socket  Thread .......\n");

    if (userThread.joinable()) {
        userThread.join();
    }
    userThread =  std::thread(&UDPServer::udpServertMain, this);


}
//
//  Restart Socket Server
//

void UDPServer::SetupServerSocket()
{
    int sfd =  getSocketDesc();
    int optval = 1;
    int status = 0;

    status = setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    status    += setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (status < 0 )
    {
        printf("\n!!!ERROR !!! in [%s]---> UDP::Unable to set socket option for socket %d \n", __FUNCTION__, sfd );
    }
    setLocalInterface(ETH_EXTERNAL);
}
//
//
void UDPServer::restartServer()
{
    int socketFD =  getSocketDesc();
    if (socketFD > 0 )
    {
        closeSock(socketFD);
    }
    socketFD = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    setSocketDesc( socketFD);
    //
    SetupServerSocket();
    setLocalPort(serverPort);
}

//
//
//

//
//
void  UDPServer::udpServertMain ()
{

    int ret = -1;
    struct pollfd fds[1];
    int nfds ;
    int timeout = M2M_POLL_TIMEOUT;
    char  DataBuffer[1024];
    int dataMaxLen = 1024;
    int len, clSocket;
    int socketError = 0;
    // Listen on Socket for messages from UI services
    bool  m2mMode = false;
    string stop("CLOSE");
    //
    syslog(LOG_DEBUG,"....... UDP  Server Thread ......");
    setRunning(true);

    clSocket = getSocketDesc();
    syslog(LOG_DEBUG,"[%s] Server Socket =%d  ", __FUNCTION__, clSocket );
    while (isRunning())
    {

        if ( (isTimeOutUsed() ) && ( getTimeout()  != 0 ) )
        {
            timeout = getTimeout()*1000;
        }
        else
        {
            timeout = M2M_POLL_TIMEOUT;
        }
        clSocket = getSocketDesc();
        socketError = 0;
        if ( clSocket > 0 )
        {
            fds[0].fd = clSocket;
            nfds = 1;
            fds[0].events = POLLIN;
            ret = poll(fds, nfds, timeout);

            if ( ret > 0 )
            {
                if ( (fds[0].revents &  POLLIN ) && (fds[0].fd ==  clSocket) )
                {
                    len = read(DataBuffer, dataMaxLen);
                    if ( len < 0 )
                    {
                        syslog(LOG_ERR,"%s(): UDP Server recvFrom failed ", __FUNCTION__);
                        socketError++;
                    }
                    else if (len > 0 )
                    {
                        if ( !m2mMode)
                        {
                            sysMsgCtrl.setUiM2mControl();
                            m2mMode = true;
                        }

                        if ( int bufx= len -1 ; is_delimiter(DataBuffer[bufx -1]) ||  is_delimiter(DataBuffer[bufx]) )
                        {
                            std::string message{DataBuffer,len};
                            syslog(LOG_DEBUG,"%s():Received M2M Command: %s:", __FUNCTION__, DataBuffer);

                            string reply;
                            processM2MMessage(message, reply);
                            if ( (reply.size() > 0) )
                            {
                                if ( ( reply != stop ) && (reply.substr(0, 2) != std::string("NA") ) )
                                {
                                    dbgprintf("%s():Sending Response (%d bytes):%s\n", __FUNCTION__, (int ) (reply.size()) , reply.c_str());
                                    syslog(LOG_DEBUG,"%s():Sending Response to User:%s", __FUNCTION__, reply.c_str() );
                                    write(reply);
                                }
                            }
                        }

                        memset(DataBuffer,0, static_cast<size_t> (dataMaxLen)) ;
                    }
                }
            }
            else
            {
                if (ret  < 0)
                {
                     syslog( LOG_ERR, "%s()--> Errno=%s -- Socket poll error ",__FUNCTION__ ,strerror(errno) );
                    if (errno != EWOULDBLOCK)
                    {
                        socketError++;
                    }
                }
                else
                {
                    if  ( ( isTimeOutUsed()) && (getTimeout() != 0) )
                    {
                        syslog(LOG_INFO,"%s(): M2M Timed out -- No M2M command received for %d  milli-seconds", __FUNCTION__, timeout);

                        if ( m2mMode)
                        {
                            sysMsgCtrl.clearUiM2mControl();
                            m2mMode = false;
                        }
                    }
                }

            }
            // Check For Socket Errors
            if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL) )
            {
                syslog( LOG_ERR, "%s()--> Errno=%s -- Socket poll Revents  failed ",__FUNCTION__ ,strerror(errno) );
                socketError++;
            }

            if (socketError > 0 )
            {
                syslog(LOG_WARNING,"%s(): UDP Server Socket Error, creating  new one  ", __FUNCTION__);
                restartServer();

                clSocket = getSocketDesc();

            }
        }
        else
        {
            syslog(LOG_WARNING,"%s(): UDP Server Socket closed --creating new one  ", __FUNCTION__);
            restartServer();
            clSocket = getSocketDesc();
        }
    }
    syslog(LOG_WARNING,"%s(): Closing UDP Server   ", __FUNCTION__);
    sysMsgCtrl.clearUiM2mControl();
    closeSock (clSocket);

}


void UDPServer::m2Mrespond( string reply)
{
    struct pollfd fds[1];
    int nfds, ret, clSocket ;
    int timeout = M2M_POLL_TIMEOUT;

    clSocket = getSocketDesc();
    if (clSocket > 0 )
    {
        fds[0].fd = clSocket;
        nfds = 1;
        fds[0].events = (POLLOUT);
        // Wait for Data to be present in Sockets
        ret = poll(fds, nfds, timeout);

        if ( ret > 0 )
        {
            syslog(LOG_DEBUG,"Sending/Forwarding response to user: %s\n ", reply.c_str());
            sendTo(  reply.c_str(), reply.size(), sourceAddress,sourcePort);
        }
    }

}

void  UDPServer::closeConnection()
{

    int clSocket = getSocketDesc();
    closeSock (clSocket);
    setRunning(false);   // This will stop UDP Server

    // Fixme...What we should do if user send M2M command again over UDP
}

void UDPServer::m2mCloseConnection()
{
    closeConnection();
}

void UDPServer::handleExecption()
{
    closeConnection();
}
#pragma GCC diagnostic pop





