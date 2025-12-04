/*
*
 */

#ifndef __SOCKET_BASE__
#define __SOCKET_BASE__

#include <string>            // For string
#include <exception>         // For exception class
#include <netinet/in.h>      // For sockaddr_in
#include <cstring>
#include <helpers/debug.h>
using namespace std;

#pragma GCC diagnostic ignored "-Wdeprecated"

/**
 *   Base class representing basic communication endpoint
 */
class BaseSocket
{
    public:
      
        virtual ~BaseSocket();
        string getLocalAddress()   noexcept;
        unsigned short getLocalPort()  noexcept;
        void setLocalPort(unsigned short localPort)  noexcept;
        void setLocalAddressAndPort(const string &localAddress,
        unsigned short localPort = 0)  noexcept;
        static void cleanUp()  noexcept;
        static unsigned short resolveService(const string &service,  const string &protocol = "tcp") noexcept;
        int getSocketDesc() { return sockDesc; };
        void setSocketDesc( int sock) { sockDesc = sock; };

        void setLocalInterface(const string &ifaceName)   noexcept ;
        void fillAddr(const string &address, unsigned short port,  sockaddr_in &addr) noexcept;
    private:
        // Prevent the user from trying to use value semantics on this object
        BaseSocket(const BaseSocket &sock);
        void operator=(const BaseSocket &sock);

    protected:
        int sockDesc;              // Socket descriptor
        BaseSocket(int type, int protocol)  noexcept;
        BaseSocket(int sockDesc)  noexcept;
        int checkSocketType(int sockfd);
};

/**
 *   Socket which is able to connect, send, and receive
 */
class CommunicatingSocket : public BaseSocket
{
    public:
        /**
        *   Establish a socket connection with the given foreign
        *   address and port
        *   @param foreignAddress foreign address (IP address or name)
        *   @param foreignPort foreign port
        *   @exception BaseSocketException thrown if unable to establish connection
        */
        int connect(const string &foreignAddress, unsigned short foreignPort)
         noexcept;

        /**
        *   Write the given buffer to this socket.  Call connect() before
        *   calling send()
        *   @param buffer buffer to be written
        *   @param bufferLen number of bytes from buffer to be written
        *   @exception BaseSocketException thrown if unable to send data
        */
        int send(const void *buffer, long unsigned int bufferLen, int flag=0 )  noexcept;

        /**
        *   Read into the given buffer up to bufferLen bytes data from this
        *   socket.  Call connect() before calling recv()
        *   @param buffer buffer to receive the data
        *   @param bufferLen maximum number of bytes to read into buffer
        *   @return number of bytes read, 0 for EOF, and -1 for error
        *   @exception BaseSocketException thrown if unable to receive data
        */
        int recv(void *buffer, int bufferLen, int flag=0 )  noexcept;

        /**
        *   Get the foreign address.  Call connect() before calling recv()
        *   @return foreign address
        *   @exception BaseSocketException thrown if unable to fetch foreign address
        */
        string getForeignAddress()  noexcept;

        /**
        *   Get the foreign port.  Call connect() before calling recv()
        *   @return foreign port
        *   @exception BaseSocketException thrown if unable to fetch foreign port
        */
        unsigned short getForeignPort()  noexcept;

    protected:
        CommunicatingSocket(int type, int protocol);
        CommunicatingSocket(int newConnSD);
};

/**
 *   TCP socket for communication with other TCP sockets
 */
class TCPSocket : public CommunicatingSocket
{
    public:
        /**
        *   Construct a TCP socket with no connection
        *   @exception BaseSocketException thrown if unable to create TCP socket
        */
        TCPSocket() ;

        /**
        *   Construct a TCP socket with a connection to the given foreign address
        *   and port
        *   @param foreignAddress foreign address (IP address or name)
        *   @param foreignPort foreign port
        *   @exception BaseSocketException thrown if unable to create TCP socket
        */
        TCPSocket(const string &foreignAddress, unsigned short foreignPort);

    private:
        // Access for TCPServerSocket::accept() connection creation
        friend class TCPServerSocket;
        TCPSocket(int newConnSD);
};

/**
 *   TCP socket class for servers
 */
class TCPServerSocket : public BaseSocket
{
    public:
        /**
        *   Construct a TCP socket for use with a server, accepting connections
        *   on the specified port on any interface
        *   @param localPort local port of server socket, a value of zero will
        *                   give a system-assigned unused port
        *   @param queueLen maximum queue length for outstanding
        *                   connection requests (default 5)
        *   @exception BaseSocketException thrown if unable to create TCP server socket
        */
        TCPServerSocket(unsigned short localPort, int queueLen = 2);
        virtual ~TCPServerSocket() = default; 

        /**
        *   Construct a TCP socket for use with a server, accepting connections
        *   on the specified port on the interface specified by the given address
        *   @param localAddress local interface (address) of server socket
        *   @param localPort local port of server socket
        *   @param queueLen maximum queue length for outstanding
        *                   connection requests (default 5)
        *   @exception BaseSocketException thrown if unable to create TCP server socket
        */
        TCPServerSocket(const string &localAddress, unsigned short localPort,
        int queueLen = 2);

        /**
        *   Blocks until a new connection is established on this socket or error
        *   @return new connection socket
        *   @exception BaseSocketException thrown if attempt to accept a new connection fails
        */
        TCPSocket *accept()  noexcept;

    private:
        void setListen(int queueLen)  noexcept;
};

/**
  *   UDP socket class
  */
class UDPSocket : public CommunicatingSocket
{
    public:
        /**
        *   Construct a UDP socket
        *   @exception BaseSocketException thrown if unable to create UDP socket
        */
        UDPSocket() ;

        /**
        *   Construct a UDP socket with the given local port
        *   @param localPort local port
        *   @exception BaseSocketException thrown if unable to create UDP socket
        */
        UDPSocket(unsigned short localPort) ;

        /**
        *   Construct a UDP socket with the given local port and address
        *   @param localAddress local address
        *   @param localPort local port
        *   @exception BaseSocketException thrown if unable to create UDP socket
        */
        UDPSocket(const string &localAddress, unsigned short localPort);

        /**
        *   Unset foreign address and port
        *   @return true if disassociation is successful
        *   @exception BaseSocketException thrown if unable to disconnect UDP socket
        */
        void disconnect() ;

        /**
        *   Send the given buffer as a UDP datagram to the
        *   specified address/port
        *   @param buffer buffer to be written
        *   @param bufferLen number of bytes to write
        *   @param foreignAddress address (IP address or name) to send to
        *   @param foreignPort port number to send to
        *   @return true if send is successful
        *   @exception BaseSocketException thrown if unable to send datagram
        */
        int sendTo(const void *buffer, int bufferLen, const string &foreignAddress,
        unsigned short foreignPort)  noexcept;

        /**
        *   Read read up to bufferLen bytes data from this socket.  The given buffer
        *   is where the data will be placed
        *   @param buffer buffer to receive data
        *   @param bufferLen maximum number of bytes to receive
        *   @param sourceAddress address of datagram source
        *   @param sourcePort port of data source
        *   @return number of bytes received and -1 for error
        *   @exception BaseSocketException thrown if unable to receive datagram
        */
        int recvFrom(void *buffer, int bufferLen, string &sourceAddress,
        unsigned short &sourcePort)  noexcept;

        /**
        *   Set the multicast TTL
        *   @param multicastTTL multicast TTL
        *   @exception BaseSocketException thrown if unable to set TTL
        */
        void setMulticastTTL(unsigned char multicastTTL)  noexcept;

        /**
        *   Join the specified multicast group
        *   @param multicastGroup multicast group address to join
        *   @exception BaseSocketException thrown if unable to join group
        */
        void joinGroup(const string &multicastGroup) ;

        /**
        *   Leave the specified multicast group
        *   @param multicastGroup multicast group address to leave
        *   @exception BaseSocketException thrown if unable to leave group
        */
        void leaveGroup(const string &multicastGroup) ;

    private:
        void setBroadcast();
};

#endif
