
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

#include "m2mTCP.h"
// Define Static parameters

M2MTCP* M2MTCP::myInstance = nullptr;
std::mutex M2MTCP::m2m_mutex;


#define NEW_LINE 0x0A
#define NEW_CR  0x0D

// Desttructor..
M2MTCP::~M2MTCP()
{
    syslog(LOG_DEBUG, "  [%s]-->  M2MTCP::~M2MTCP() called  ", __FUNCTION__);
    // Join threads here if they are still running
    if (userThread.joinable()) {
        userThread.join();
    }

    if (clientThread.joinable()) {
        clientThread.join();
    }
    myInstance = nullptr;
}


M2MTCP *M2MTCP::getMyInstance( unsigned short port )
{
    std::lock_guard<std::mutex> lock(m2m_mutex);

    if (myInstance == nullptr )
    {
        myInstance = new M2MTCP( port);
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
            myInstance->m2mCloseServerConnection();


            if (myInstance->clientThread.joinable()) {
                myInstance->clientThread.join();
            }

            if (myInstance)  {
                delete myInstance;
            }
            myInstance = new M2MTCP( port);
        }
    }

    return myInstance;
}
//Ctors
M2MTCP::M2MTCP( unsigned short port):TCPServerSocket(port),
    isrunning(false),
    remoteIP(),
    remotePort(0),
    connected(false),
    newConn(NULL),
    serverSocket(-1),
    clientScoket(-1),
    serverPort(port),
    clientThread{}

{
    serverSocket = getSocketDesc();
    syslog(LOG_DEBUG, "  [%s]--> M2M TCP Ctor --Server Socket = %d ", __FUNCTION__, serverSocket);

}

void M2MTCP::startThread()
{
    userThread =  std::thread(&M2MTCP::M2MTCPMain, this);

}
//
//  Restart Socket Server
//

void M2MTCP::SetupServerSocket()
{
    int sfd =  getSocketDesc();
    int optval = 1;
    int status = 0;

    if (sfd > 0)
    {
        status = setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
        status += setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        status += setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    }

    if (status < 0 )
    {
        printf("\n!!!ERROR !!! in [%s]---> Unable to set socket option \n", __FUNCTION__);
    }

}
//
//
void M2MTCP::restartServer()
{
    int socketFD =  getSocketDesc();

    if (socketFD > 0 )
    {
        closeSock(socketFD);
    }
    socketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setSocketDesc( socketFD);
    remoteIP.clear();
    //
    SetupServerSocket();
}
//
//
//

void  M2MTCP::M2MTCPMain ()
{

    //
    TCPSocket  *conn;
    dbgprintf(".......TCP Main   Thread .......\n");
    syslog(LOG_INFO, "[%s] .......starting M2M TCP Main  Thread .......", __FUNCTION__);

    struct pollfd fds[1];
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN;

    while (isRunning())
    {
        // Poll the server socket
        int ret = poll(fds, 1, 500);

        if (ret < 0)
        {
            if (errno == EINTR) continue;
            syslog(LOG_ERR, "Poll error: %s", strerror(errno));
            break;
        }

        if (ret == 0) {
            continue;
        }

        if (fds[0].revents & POLLIN)
        {
            conn = accept(); // This will now return immediately
            if (conn)
            {
                if (!isConnected())
                {
                    setNewConnection(conn);
                    setConnected(true);
                    clientThread = std::thread(&M2MTCP::newConnMain, this);
                }
                else {
                    // Only one connection allowed, reject the new one
                    int tempSk = conn->getSocketDesc();
                    closeSock(tempSk);
                    delete conn;
                }
            }
        }
    }


    // Close Main TCP Socket
    closeSock(serverSocket);
    int srvSock = getSocketDesc();
    syslog(LOG_INFO, "[%s] ..TCP Main Thread is Terminated ( Server Socket =%d  Closed ) ......", __FUNCTION__, srvSock );
    if (serverSocket != srvSock )
        closeSock(srvSock);

}



//
//
//

void  M2MTCP::newConnMain ( )
{

    int ret = -1;
    struct pollfd fds[1];
    nfds_t nfds ;
    int timeout = M2M_POLL_TIMEOUT;

    const long unsigned int dataMaxLen = 1024;
    char  DataBuffer[dataMaxLen];
    int len, clSocket;
    int socketError = 0;
    string reply;
    string stop("CLOSE");
    //
    //
    TCPSocket *tcpConn = getTcpConn();



    if (!tcpConn)
    {
        printf(" M2M TCP  has not been created yet \n");
        syslog(LOG_ERR, "[%s] .....M2M TCP Server  has not been created yet.......", __FUNCTION__);
        return ;
    }

    sysMsgCtrl.setUiM2mControl();

    long unsigned int dataOffset = 0;
    memset(DataBuffer,0, static_cast <size_t> (dataMaxLen));

    clSocket = tcpConn->getSocketDesc();
    setRunning(true);
    syslog(LOG_DEBUG, "[%s] .......starting M2M TCP Session  Thread -- Client Socket=%d.......", __FUNCTION__,clSocket);
    while (isRunning() )
    {
        auto processTimeST = getCurrentTime();
        if ( (isTimeOutUsed() ) && ( getTimeout()  != 0 ) )
        {
            timeout = getTimeout()*1000;
        }
        else
        {
            timeout = M2M_POLL_TIMEOUT;
        }
        clSocket = tcpConn->getSocketDesc();
        fds[0].fd = clSocket;
        nfds = 1;
        fds[0].events = (POLLIN | POLLERR | POLLHUP | POLLNVAL);
        // Wait for Data to be present in Sockets
        //cout << " Time Before Poll " << timems << endl;
        ret = poll(fds, nfds, timeout);

        if ( ret > 0 )
        {
            if ( (fds[0].revents &  POLLIN ) && (fds[0].fd ==  clSocket) )
            {
                processTimeST = getCurrentTime();
                if ( dataOffset >= dataMaxLen)
                {

                    memset(DataBuffer,0, static_cast <size_t> (dataMaxLen));
                    dataOffset = 0;
                }
                len = tcpConn->recv(DataBuffer + dataOffset, dataMaxLen -dataOffset);
                if ( len <= 0 )
                {
                    syslog( LOG_ERR, "%s()--> Errno=%s -- Socket Recv failed ",__FUNCTION__ ,strerror(errno) );
                    socketError++;
                }
                else if (len > 0 )
                {
                    dataOffset += len;

                    // We could have "MO\n\r" or "MO\n" or "MO\r" or "MO\r\n"
                    if ( int bufx= dataOffset-1;   is_delimiter(DataBuffer[bufx -1]) ||  is_delimiter(DataBuffer[bufx]) )
                    {
                        std::string message{DataBuffer,dataOffset};
                        syslog(LOG_DEBUG,"%s():Received User M2M Message: %s:", __FUNCTION__, DataBuffer);

                        reply.clear();
                        processM2MMessage(message, reply);
                        if ( (reply.size() > 0) )
                        {
                            if ( ( reply != stop ) && (reply.substr(0, 2) != std::string("NA") ) )
                            {
                                dbgprintf("%s():Sending Response (%d bytes):%s\n", __FUNCTION__, (int ) (reply.size()) , reply.c_str());
                                syslog(LOG_DEBUG,"%s():Sending Response to User:%s", __FUNCTION__, reply.c_str() );
                                tcpConn->send(reply.c_str(), reply.size());
                            }
                        }
                        memset(DataBuffer,0, static_cast<size_t> (dataMaxLen)) ;
                        dataOffset = 0;
                    }

                }

            }
            if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL) )
            {
                syslog( LOG_ERR, "%s()--> Errno=%s -- Socket poll Revents  failed ",__FUNCTION__ ,strerror(errno) );
                socketError++;
            }
            auto ProcessTimeEnd = getCurrentTime();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(ProcessTimeEnd - processTimeST);
            dbgprintf("%s(): Processing time ===>  Time = %lu microseconds \n", __FUNCTION__,duration.count() );
        }
        else
        {
            if (ret  < 0)
            {

                syslog(LOG_ERR,"%s()--> Errno =%s -- M2M  poll failed  !\n ", __FUNCTION__ ,strerror(errno));
                if (errno != EWOULDBLOCK)
                {
                    syslog(LOG_ERR,"%s()--> Errno =%s -- Errno is fatal  Closing Socket.. !\n ", __FUNCTION__ ,strerror(errno));
                    socketError++;
                }
            }
            else
            {
                if  ( ( isTimeOutUsed()) && (getTimeout() != 0) )
                {
                    syslog(LOG_INFO,"\n !!!!!%s(): Connection Times Out after %d !!!!!!!!\n ", __FUNCTION__, timeout);
                    break;
                }
            }
        }
        // Check For Socket Errors
        if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL) )
        {
            syslog(LOG_ERR,"\n !!!!!%s(): M2M Socket failed...closing Connection !!!!!!!!\n ", __FUNCTION__);
            socketError++;
        }

        if (socketError > 0 )
        {
            break;
        }
    }

    sysMsgCtrl.clearUiM2mControl();
    setConnected(false);
    syslog(LOG_INFO,"!!!!!%s(): Closing M2M Client Connection -Client Socket=%d!!!!!!!! ", __FUNCTION__, clSocket);
    closeSock (clSocket);

    delete tcpConn;

}


void M2MTCP::m2Mrespond( string reply)
{
    struct pollfd fds[1];
    int nfds, ret, clSocket ;
    int timeout = M2M_POLL_TIMEOUT;

    if ( newConn != NULL )
    {
        clSocket = newConn->getSocketDesc();
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
                newConn->send(reply.c_str(), reply.size());
            }
        }
    }
}

void  M2MTCP::closeConnection()
{

    setConnected(false);
    if (newConn )
    {
        int clSocket = newConn->getSocketDesc();
        closeSock (clSocket);
    }
}

void M2MTCP::m2mCloseConnection()
{
    closeConnection();
}

void M2MTCP::handleExecption()
{
   stopConnection();
   closeConnection();
}

void M2MTCP::m2mCloseServerConnection()
{
    syslog(LOG_DEBUG," %s()--->Closing Server Connection", __FUNCTION__);
    closeSock (serverSocket);
    closeConnection();
}

#pragma GCC diagnostic pop
