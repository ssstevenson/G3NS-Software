
#include <sys/socket.h>
#include <cstring>  // ✅ Required for memset
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

pthread_t   newConnThread;
M2MTCP *M2MTCP::myInstance = NULL;



#define NEW_LINE 0x0A
#define NEW_CR  0x0D

// Desttructor..
M2MTCP::~M2MTCP()
{
    delete myInstance;
    myInstance = NULL;
}


M2MTCP *M2MTCP::getMyInstance()
{
    if (myInstance == NULL )
    {
        myInstance = new M2MTCP;
    }

    return myInstance;
}
//Ctors
M2MTCP::M2MTCP():TCPServerSocket(M2M_TCP_PORT),
    isrunning(true),
    m2m_tcp_server_port( M2M_TCP_PORT),
    remoteIP(),
    remotePort(0),
    connected(false),
    newConn(NULL)
{
    SetupM2MTCPServer();
}

void M2MTCP::startThread()
{
    printf(".......starting M2M  TCP  Server  Thread .......\n");
    pthread_create (&this->myThread, NULL,  M2MTCPMain, (void *) this);
    //
}
//
//  Restart Socket Server
//

void M2MTCP::SetupM2MTCPServer()
{
    int sfd =  getSocketDesc();
    int optval = 1;
    int status = 0;

    status = setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    status    += setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    status    += setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

    if (status < 0 )
    {
        printf("\n!!!ERROR !!! in [%s]---> Unable to set socket option \n", __FUNCTION__);
    }

}
//
//
void M2MTCP::restartM2MTCP()
{
    int socketFD =  getSocketDesc();
    if (socketFD > 0 )
    {
        ::close(socketFD);
    }
    socketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setSocketDesc( socketFD);
    remoteIP.clear();
    //
    SetupM2MTCPServer();
}
//
//
//

void  *M2MTCP::M2MTCPMain ( void *ptr)
{

    //
    TCPSocket  *conn;
    dbgprintf(".......TCP Server  Thread Main.......\n");
    M2MTCP *m2mTcp = reinterpret_cast < M2MTCP * >  (ptr);

    if (!m2mTcp)
    {
        printf(" M2MTCP has not been created yet \n");
        return NULL;
    }

    while (m2mTcp->isRunning())
    {
        conn = m2mTcp->accept();
        if (conn )
        {
            if ( !m2mTcp->isConnected () )
            {
                m2mTcp->setNewConnection (conn);
                m2mTcp->setConnected(true);
                pthread_create (&newConnThread, NULL, newConnMain,ptr);
            }

        }
        sleep(1);
    }

    return NULL;
}


//
//
//

void  *M2MTCP::newConnMain ( void *ptr)
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
    printf(".......TCP Server New Connection Thread Main.......\n");
    M2MTCP *m2mTcp = reinterpret_cast < M2MTCP * >  (ptr);
    TCPSocket *tcpConn = m2mTcp->getTcpConn();



    if (!tcpConn)
    {
        printf(" M2M TCP  has not been created yet \n");
        return NULL;
    }

    m2mTcp->sysMsgCtrl.setUiM2mControl();

    long unsigned int dataOffset = 0;
    memset(DataBuffer,0, static_cast <size_t> (dataMaxLen));

    clSocket = tcpConn->getSocketDesc();

    while (m2mTcp->isRunning() )
    {
        auto processTimeST = m2mTcp->getCurrentTime();
        if ( (m2mTcp->isTimeOutUsed() ) && ( m2mTcp->getTimeout()  != 0 ) )
        {
            timeout = m2mTcp->getTimeout()*1000;
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
                processTimeST = m2mTcp->getCurrentTime();
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

#ifdef DEBUG
                    dbgprintf("%s():M2M recv TCP Message  =[%s] --->len=[%d]\n", __FUNCTION__, DataBuffer, dataOffset);
                    printf("CMD in Hex: ");
                    m2mTcp->print_hex(DataBuffer,dataOffset );
                      printf("\n");
#endif
                    // We could have "MO\n\r" or "MO\n" or "MO\r" or "MO\r\n"
                    if ( int bufx= dataOffset-1;   m2mTcp->is_delimiter(DataBuffer[bufx -1]) ||  m2mTcp->is_delimiter(DataBuffer[bufx]) )
                    {
                        std::string message{DataBuffer,dataOffset};
#ifdef DEBUG
                        auto alphanumeric = []( string &input)  -> std::string {
                            string str{input};
                            for (auto it = str.begin(); it != str.end(); ) {
                                if (*it == '\n' || *it == '\r') {
                                    it = str.erase(it);  // Erase and update iterator
                                } else {
                                    ++it;
                                }
                            }
                            return str;
                        };
                        string temp = alphanumeric(message);
                        dbgprintf("%s():M2M TCP processing Command   =[%s]-->len=[%d]\n", __FUNCTION__, temp.c_str(), temp.size());
#endif
                        syslog(LOG_INFO,"%s():Received User M2M Message: %s:", __FUNCTION__, DataBuffer);

                        reply.clear();
                        m2mTcp->processM2MMessage(message, reply);
                        if ( (reply.size() > 0) )
                        {
                            if ( ( reply != stop ) && (reply.substr(0, 2) != std::string("NA") ) )
                            {
                                dbgprintf("%s():Sending Response (%d bytes):%s\n", __FUNCTION__, (int ) (reply.size()) , reply.c_str());
                                syslog(LOG_INFO,"%s():Sending Response to User:%s", __FUNCTION__, reply.c_str() );
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
            auto ProcessTimeEnd = m2mTcp->getCurrentTime();
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
                if  ( (m2mTcp-> isTimeOutUsed()) && (m2mTcp->getTimeout() != 0) )
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

     m2mTcp->sysMsgCtrl.clearUiM2mControl();
    m2mTcp->setConnected(false);
    close (clSocket);
    dbgprintf("\n !!!!!%s(): TCP Connection Ended..!!!!!!!!\n ", __FUNCTION__);
    syslog(LOG_INFO,"\n !!!!!%s(): Closing M2M Connection !!!!!!!!\n ", __FUNCTION__);
    delete tcpConn;

    sleep(1);
    return NULL;
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
                syslog(LOG_INFO,"Sending/Forwarding response to user: %s\n ", reply.c_str());
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
        close (clSocket);
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

#pragma GCC diagnostic pop
