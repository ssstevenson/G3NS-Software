#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"

#include "AckSocket.h"

AckSocket::AckSocket(bool controller, int port):
    ack_sock{-1},
    myIp{0},
    myIpStr{},
    ackport{static_cast <uint16_t> (port)},
    isController{controller},
    interface{IFACE}
{
   getMyIPAddress();
   openSocket();
}
//
//
AckSocket::~AckSocket()
{
    if (ack_sock > 0) {
        ::close(ack_sock);
        ack_sock = -1;
    }

}
//
//
int AckSocket::openSocket()
{
    int ret;
    if (ack_sock > 0 ) {
        ::close(ack_sock);
        ack_sock = -1;
    }
    if (isController) {
       ret = createServerSocket();
    } else {
        ret =  createReplySocket();
    }

    return ret;
}
int AckSocket::createServerSocket() {

   int ret = createSocket();
   // Server need to bind to  adress/port
    if (ret == 0 ) {
        sockaddr_in ack_addr{};
        ack_addr.sin_family = AF_INET;
        ack_addr.sin_port = htons(ackport);
        ack_addr.sin_addr.s_addr = INADDR_ANY;

        ret = bind(ack_sock, reinterpret_cast<sockaddr*>(&ack_addr), sizeof(ack_addr));
        if (ret < 0 ) {
           syslog(LOG_ERR, "[%s] Error Binding to Socket", __FUNCTION__);
           ::close(ack_sock);
           ack_sock = -1;

        }
    }
    return ret;
}

int AckSocket::createReplySocket() {

   return  createSocket();
}

int AckSocket::createSocket()
{
    ack_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ack_sock < 0) {
        syslog(LOG_ERR, "[%s] Error Creating Socket", __FUNCTION__);
        return 1;
    }
    int val = 1;
    if ( setsockopt(ack_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
        syslog(LOG_ERR, "[%s]  Socket option  SO_REUSEADDR  Failed ", __FUNCTION__);
    }

    if ( setsockopt(ack_sock, SOL_SOCKET, SO_BINDTODEVICE, interface.c_str(), static_cast <socklen_t> (interface.size())) < 0) {
        syslog(LOG_ERR, "[%s]  Socket option  SO_BINDTODEVICE  Failed ", __FUNCTION__);
    }

#ifdef TEST_ON_SAME_HOST

    if ( setsockopt(ack_sock, SOL_SOCKET, IP_MULTICAST_LOOP, &val, sizeof(val)) < 0) {
        syslog(LOG_ERR, "[%s]  Socket option  IP_MULTICAST_LOOP  Failed ", __FUNCTION__);
    }
#endif
    return 0;
}
//
// return -1 on failure
// return 0  on success
// return number of bytes transmitted if different from requested
//
int AckSocket::reply( void *msg, int msgLen,  unsigned int senderIp, uint16_t senderPort)
{
    long int ret = 0;
    sockaddr_in reply_addr{};
    reply_addr.sin_family = AF_INET;
    reply_addr.sin_port = htons(senderPort);
    reply_addr.sin_addr.s_addr = htonl(senderIp);

    long int nbytes = sendto(ack_sock, msg, static_cast <size_t> ( msgLen) , 0, reinterpret_cast<sockaddr*>(&reply_addr), sizeof(reply_addr));

    if ( nbytes < 0) {
         openSocket();
         ret = -1;
    }
    else if ( nbytes != msgLen ) {
        ret = nbytes;
    }
    return (static_cast < int>  (ret) );
}


int AckSocket::getMyIPAddress()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    std::strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ);

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        perror("ioctl");
        close(fd);
        return 1;
    }

    close(fd);

    struct sockaddr_in* ipaddr = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);

    // IP as 32-bit integer in host byte order
    myIp = ntohl(ipaddr->sin_addr.s_addr);

    // IP as dotted-decimal string
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipaddr->sin_addr, ip_str, sizeof(ip_str));
    myIpStr = ip_str;

    return 0;
}

//
//  return 0 on success or timeout
//  return -1 on failure.
//  recvBytes has the number of bytes received
//
int AckSocket::waitForReply(  AckPacket &reply, long int &recvBytes)
{
    long int ret = 0;
    if (ack_sock < 0)
    {
        openSocket();
    }

    struct pollfd pfd{};
    pfd.fd = ack_sock;
    pfd.events = POLLIN;

    int err = poll(&pfd, 1, POLL_WAIT_IN_MS);
    if (err < 0)
    {
       openSocket();
       ret = err;
    }
    else if (err > 0 )
    {
        // some data, check if for our socket and in input
        if (pfd.revents & POLLIN)
        {
            sockaddr_in senderAddr{};
            socklen_t senderLen = sizeof(senderAddr);

            long int rc = recvfrom(ack_sock, &reply, sizeof(reply) , 0, reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);

            if (rc < 0 )
            {
                openSocket();
                ret = rc;
            }
            else if (rc > 0) {
                recvBytes = rc;
                convertPacketFromNetwork(reply);
            }

        }
    }

    return ret ;
}

void AckSocket::convertPacketFromNetwork(AckPacket &pkt) {
    uint32_t *p = reinterpret_cast<uint32_t*>(&pkt);
    size_t n = sizeof(CommandPacket) / sizeof(uint32_t);

    for (size_t i = 0; i < n; i++) {
        p[i] = ntohl(p[i]);
    }
}

void AckSocket::convertPacketToNetwork(AckPacket &pkt) {
    uint32_t *p = reinterpret_cast<uint32_t*>(&pkt);
    size_t n = sizeof(CommandPacket) / sizeof(uint32_t);

    for (size_t i = 0; i < n; i++) {
        p[i] = htonl(p[i]);
    }
}

#pragma GCC diagnostic pop
