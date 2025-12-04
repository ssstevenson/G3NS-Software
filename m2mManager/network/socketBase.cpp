#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
typedef void raw_type;       // Type used for raw data on this platform

#include <errno.h>             // For errno

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wfloat-conversion"

#include "socketBase.h"

using namespace std;



// Function to fill in address structure given an address and port
void BaseSocket::fillAddr(const string &address, unsigned short port,  sockaddr_in &addr) noexcept
{
  memset(&addr, 0, sizeof(addr));  // Zero out address structure
  addr.sin_family = AF_INET;       // Internet address

  hostent *host;  // Resolve name
  if ((host = gethostbyname(address.c_str())) == NULL)
  {
      printf ("\n Error in %s, Failed to resolve hsotname for address=%s, port =%d (socket=%d) \n",
              __FUNCTION__, address.c_str(), port, sockDesc);
     addr.sin_addr.s_addr = inet_addr(address.c_str());

  }
  else
  {
        addr.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
  }

  addr.sin_port = htons(port);     // Assign port in network byte order
}

// BaseSocket Code

BaseSocket::BaseSocket(int type, int protocol) noexcept:sockDesc(-1)
{

  // Make a new socket
  if ((sockDesc = socket(PF_INET, type, protocol)) < 0)
  {
    printf ("Socket creation failed (socket()) \n");
  }

}

BaseSocket::BaseSocket(int sock) noexcept :sockDesc(sock)
{
}

BaseSocket::~BaseSocket()
{

   ::close(sockDesc);

  sockDesc = -1;
}

string BaseSocket::getLocalAddress() noexcept
{
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getsockname(sockDesc, (sockaddr *) &addr,  &addr_len) < 0)
  {
    printf("Fetch of local address failed (getsockname())\n");
  }
  return inet_ntoa(addr.sin_addr);
}

unsigned short BaseSocket::getLocalPort() noexcept
{
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getsockname(sockDesc, (sockaddr *) &addr,  &addr_len) < 0)
  {
    printf("Fetch of local port failed (getsockname())\n");
  }
  return ntohs(addr.sin_port);
}

void BaseSocket::setLocalPort(unsigned short localPort) noexcept
{
  // Bind the socket to its port
  sockaddr_in localAddr;
  memset(&localAddr, 0, sizeof(localAddr));
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(localPort);

  // We need to close the socket and open new one
  if (sockDesc > 0)
  {
      int type = checkSocketType(sockDesc);
      int protocol = IPPROTO_TCP;
        close (sockDesc);
      // mobbad
      if (type == SOCK_DGRAM )
      {
          protocol = IPPROTO_UDP;
      }
      if ( (sockDesc = socket(PF_INET, type, protocol)) < 0 )
      {
         printf("Set of local port ---- Unable to create socket(tye=%d, protocol=%d)\n",type,protocol );
      }

  }

  if (bind(sockDesc, (sockaddr *) &localAddr, sizeof(sockaddr_in)) < 0)
  {
    printf("Set of local port failed (bind()) \n");
  }
}

void BaseSocket::setLocalAddressAndPort(const string &localAddress,  unsigned short localPort) noexcept
{
    // Get the address of the requested host
    sockaddr_in localAddr;
    fillAddr(localAddress, localPort, localAddr);

    if (bind(sockDesc, (sockaddr *) &localAddr, sizeof(sockaddr_in)) < 0)
    {
        printf("Set of local address and port failed (bind()) \n");
    }
}

void BaseSocket::setLocalInterface(const string &ifaceName) noexcept
{
    // Get the address of the requested host
    dbgprintf("%s()-->Binding to interface %s \n", __FUNCTION__,ifaceName.c_str() );
    if (setsockopt(sockDesc, SOL_SOCKET, SO_BINDTODEVICE, ifaceName.c_str(), ifaceName.size()) < 0 )
    {
        printf("Unable to Bind To interface (bind())\n");
    }
}
void BaseSocket::cleanUp() noexcept
{
}

unsigned short BaseSocket::resolveService(const string &service, const string &protocol) noexcept
{
  struct servent *serv;        /* Structure containing service information */

  if ((serv = getservbyname(service.c_str(), protocol.c_str())) == NULL)
    return atoi(service.c_str());  /* Service is port number */
  else
    return ntohs(serv->s_port);    /* Found port (network byte order) by name */
}

int BaseSocket::checkSocketType(int sockfd)
{
    int type;
    socklen_t len = sizeof(type);

    if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &type, &len) == -1)
    {
        return -1;
    }
    return type;
}
//
// CommunicatingSocket Code. This is the socket used for communication
// Like the one created by ::accept
//

CommunicatingSocket::CommunicatingSocket(int type, int protocol)
                    : BaseSocket(type, protocol)
{
}

CommunicatingSocket::CommunicatingSocket(int newConnSD) : BaseSocket(newConnSD)
{
}

int CommunicatingSocket::connect(const string &foreignAddress, unsigned short foreignPort) noexcept
{
    // Get the address of the requested host
    sockaddr_in destAddr;
    fillAddr(foreignAddress, foreignPort, destAddr);

    // Try to connect to the given port
    int ret = ::connect(sockDesc, (sockaddr *) &destAddr, sizeof(destAddr));

    return ret;
}

int CommunicatingSocket::send(const void *buffer, long unsigned int bufferLen, int flag)   noexcept
{
     int rtn = 0;
    rtn = ::send(sockDesc, (raw_type *) buffer, bufferLen, flag) ;
    return rtn;

}

int CommunicatingSocket::recv(void *buffer, int bufferLen, int flag )   noexcept
{
    int rtn;
    rtn = ::recv(sockDesc,  buffer, bufferLen, flag);
    return rtn;
}

string CommunicatingSocket::getForeignAddress() noexcept
{
    sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    if (getpeername(sockDesc, (sockaddr *) &addr, &addr_len) < 0)
    {
        printf("Fetch of foreign address failed (getpeername())\n");
    }
    return inet_ntoa(addr.sin_addr);
}

unsigned short CommunicatingSocket::getForeignPort() noexcept
{
    sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    if (getpeername(sockDesc, (sockaddr *) &addr,  &addr_len) < 0)
    {
        printf("Fetch of foreign port failed (getpeername())\n");
    }
    return ntohs(addr.sin_port);
}

// TCPSocket Code

TCPSocket::TCPSocket() : CommunicatingSocket(SOCK_STREAM, IPPROTO_TCP)
{
}

TCPSocket::TCPSocket(const string &foreignAddress, unsigned short foreignPort)
    : CommunicatingSocket(SOCK_STREAM, IPPROTO_TCP)
{
    connect(foreignAddress, foreignPort);
}

TCPSocket::TCPSocket(int newConnSD) : CommunicatingSocket(newConnSD)
{
}

// TCPServerSocket Code

TCPServerSocket::TCPServerSocket(unsigned short localPort, int queueLen)
    : BaseSocket(SOCK_STREAM, IPPROTO_TCP)
{
    setLocalPort(localPort);
    setListen(queueLen);
}

TCPServerSocket::TCPServerSocket(const string &localAddress,
    unsigned short localPort, int queueLen)
    : BaseSocket(SOCK_STREAM, IPPROTO_TCP) {
  setLocalAddressAndPort(localAddress, localPort);
  setListen(queueLen);
}

TCPSocket *TCPServerSocket::accept() noexcept
{
    int newConnSD;
    TCPSocket *newConn = NULL;
    if ((newConnSD = ::accept(sockDesc, NULL, 0)) < 0)
    {
        printf("Accept failed (accept()) \n");
    }
    else
    {
        newConn = new TCPSocket(newConnSD);
    }

    return newConn;
}

void TCPServerSocket::setListen(int queueLen) noexcept
{
    if (listen(sockDesc, queueLen) < 0)
    {
        printf("Set listening socket failed (listen()) \n");
    }
}

// UDPSocket Code

UDPSocket::UDPSocket() : CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP)
{
    setBroadcast();
}

UDPSocket::UDPSocket(unsigned short localPort)  :  CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP)
{
    setLocalPort(localPort);
    setBroadcast();
}

UDPSocket::UDPSocket(const string &localAddress, unsigned short localPort)  : CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP)
{
    setLocalAddressAndPort(localAddress, localPort);
    setBroadcast();
}

void UDPSocket::setBroadcast()
{
    // If this fails, we'll hear about it when we try to send.  This will allow
    // system that cannot broadcast to continue if they don't plan to broadcast
    int broadcastPermission = 1;
    setsockopt(sockDesc, SOL_SOCKET, SO_BROADCAST, (raw_type *) &broadcastPermission, sizeof(broadcastPermission));
}

void UDPSocket::disconnect()
{
    sockaddr_in nullAddr;
    memset(&nullAddr, 0, sizeof(nullAddr));
    nullAddr.sin_family = AF_UNSPEC;

    // Try to disconnect
    if (::connect(sockDesc, (sockaddr *) &nullAddr, sizeof(nullAddr)) < 0)
    {
        if (errno != EAFNOSUPPORT)
        {
            printf("Disconnect failed (connect()) \n");
        }
    }
}

int UDPSocket::sendTo(const void *buffer, int bufferLen, const string &foreignAddress, unsigned short foreignPort) noexcept

{
    sockaddr_in destAddr;
    fillAddr(foreignAddress, foreignPort, destAddr);
    int len;

    // Write out the whole buffer as a single message.
    if ( (len=sendto(sockDesc, (raw_type *) buffer, bufferLen, 0,   (sockaddr *) &destAddr, sizeof(destAddr)) )!= bufferLen)
    {
        printf("Send failed (sendto()) \n");
    }

    return len;
}

int UDPSocket::recvFrom(void *buffer, int bufferLen, string &sourceAddress, unsigned short &sourcePort) noexcept
{
    sockaddr_in clntAddr;
    socklen_t addrLen = sizeof(clntAddr);
    int rtn;
    if ((rtn = recvfrom(sockDesc,  buffer, bufferLen, 0, (sockaddr *) &clntAddr, &addrLen)) < 0)
    {
       printf("Receive failed (recvfrom()) \n");
    }
    sourceAddress = inet_ntoa(clntAddr.sin_addr);
    sourcePort = ntohs(clntAddr.sin_port);

    return rtn;
}

void UDPSocket::setMulticastTTL(unsigned char multicastTTL) noexcept
{
    if (setsockopt(sockDesc, IPPROTO_IP, IP_MULTICAST_TTL, (raw_type *) &multicastTTL, sizeof(multicastTTL)) < 0)
    {
        printf("Multicast TTL set failed (setsockopt()) \n");
    }
}

void UDPSocket::joinGroup(const string &multicastGroup)
{
    struct ip_mreq multicastRequest;

    multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockDesc, IPPROTO_IP, IP_ADD_MEMBERSHIP,(raw_type *) &multicastRequest, sizeof(multicastRequest)) < 0)
    {
        printf("Multicast group join failed (setsockopt()) \n");
    }
}

void UDPSocket::leaveGroup(const string &multicastGroup)
{
    struct ip_mreq multicastRequest;

    multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockDesc, IPPROTO_IP, IP_DROP_MEMBERSHIP, (raw_type *) &multicastRequest,sizeof(multicastRequest)) < 0)
    {
        printf("Multicast group leave failed (setsockopt()) \n");
    }
}



#pragma GCC diagnostic pop
