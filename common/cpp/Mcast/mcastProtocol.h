#pragma once
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <poll.h>

constexpr const char* MULTICAST_GROUP = "239.0.0.1";
constexpr int  COMMAND_PORT   = 5001;
constexpr int  RESPONSE_PORT  = 5002;
constexpr int POLL_WAIT_IN_MS  = 1000;  
constexpr int MAX_TRANSMIT_RETIES = 30;
#ifdef TEST_ON_SAME_HOST
constexpr const char* IFACE = "ens33";  // Ubuntu
#else
constexpr const char* IFACE = "rear";
#endif

struct CommandPacket {
    uint32_t commandId;     
    uint32_t  value;
    // Send Controller IP and Port to use for response
    uint32_t  ctrlIP;
    uint32_t  ctrlPort;
    // sequence number to validate
    uint32_t seqNumber;   
} __attribute__((packed));

struct AckPacket {
    uint32_t commandId;
    uint32_t  boosterIP; 
    uint32_t seqNumber;
} __attribute__((packed));


// Here are some commands
enum CommandID {
    ALC_MODE = 1,
    AGC_MODE,
    MGC_MODE,
    TARGET_POWER,
    DETECTION_MODE,
};
