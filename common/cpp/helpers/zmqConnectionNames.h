#ifndef ZMQ_CONNECTION_NAMES_H_
#define ZMQ_CONNECTION_NAMES_H_

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

namespace empower::enums
{
    /**
     * @brief      This class describes zmq connections.
     */
    class zmqConnections
    {
    private:
        std::string username;
        std::unordered_map<std::string, std::pair<std::string, std::string>> mapping = { \
            {"getTicker100ms", {"ipc:///tmp/", "ticker100ms"}}, \
            {"getTicker500ms", {"ipc:///tmp/", "ticker500ms"}}, \
            {"getTicker1000ms", {"ipc:///tmp/", "ticker1000ms"}}, \
            {"getTicker5000ms", {"ipc:///tmp/", "ticker5000ms"}}, \
            {"getTicker30000ms", {"ipc:///tmp/", "ticker30000ms"}}, \
            {"getLogger", {"ipc:///tmp/", "logger"}}, \
            {"getLoggerAPI", {"ipc:///tmp/", "loggerAPI"}}, \
            {"getFpgaHardwareInterfaceAPI", {"ipc:///tmp/", "fpgaHardwareInterfaceAPI"}}, \
            {"getSerialPmodInterfaceAPI", {"ipc:///tmp/", "serialPmodInterfaceAPI"}}, \
            {"getSerialPsuInterfaceAPI", {"ipc:///tmp/", "serialPsuInterfaceAPI"}}, \
            {"getSerialRmtInterfaceAPI", {"ipc:///tmp/", "serialRmtInterfaceAPI"}}, \
            {"getSerialFPInterfaceAPI", {"ipc:///tmp/", "serialFPInterfaceAPI"}}, \
            {"getSerialRPInterfaceAPI", {"ipc:///tmp/", "serialRPInterfaceAPI"}}, \
            {"getHeartbeatRequest", {"ipc:///tmp/", "heartbeatRequest"}}, \
            {"getHeartbeatResponse", {"ipc:///tmp/", "heartbeatResponse"}}, \
            {"getHeartbeatAPI", {"ipc:///tmp/", "heartbeatAPI"}}, \
            {"getMessageFactoryAPI", {"ipc:///tmp/", "messageFactoryAPI"}}, \
            {"getStatusUpdateBroadcast", {"ipc:///tmp/", "statusUpdateBroadcast"}}, \
            {"getStatusUpdate", {"ipc:///tmp/", "statusUpdate"}}, \
            {"getStatusProcessorAPI", {"ipc:///tmp/", "statusProcessorAPI"}}, \
            {"getWorkerCommandRequest", {"ipc:///tmp/", "workerCommandRequest"}}, \
            {"getWorkerCommandResponse", {"ipc:///tmp/", "workerCommandResponse"}}, \
            {"getConfigChangeNotify", {"ipc:///tmp/", "configChangeNotify"}}, \
            {"getConfigurationManagerAPI", {"ipc:///tmp/", "configurationManagerAPI"}}, \
            {"getCommandProcessorInternal", {"ipc:///tmp/", "commandProcessorInternal"}}, \
            {"getCommandProcessorInternalAPI", {"ipc:///tmp/", "commandProcessorInternalAPI"}}, \
            {"getCommandProcessorM2MTcpAPI", {"ipc:///tmp/", "commandProcessorM2MTcpAPI"}}, \
            {"getCommandProcessorM2MUdpAPI", {"ipc:///tmp/", "commandProcessorM2MUdpAPI"}}, \
            {"getCommandProcessorM2MSerialAPI", {"ipc:///tmp/", "commandProcessorM2MSerialAPI"}}, \
            {"getCommandProcessorWebRemoteAPI", {"ipc:///tmp/", "commandProcessorWebRemoteAPI"}}, \
            {"getCommandProcessorWebLocalAPI", {"ipc:///tmp/", "commandProcessorWebLocalAPI"}}, \
            {"getCommandCompleteBroadcast", {"ipc:///tmp/", "commandCompleteBroadcast"}}, \
            {"getProcessManagerAPI", {"ipc:///tmp/", "processManagerAPI"}}, \
            {"getPsuManagerAPI", {"ipc:///tmp/", "psuManagerAPI"}}, \
            {"getSystemManagerAPI", {"ipc:///tmp/", "systemManagerAPI"}}, \
            {"getFaultManagerAPI", {"ipc:///tmp/", "faultManagerAPI"}}, \
            {"getProfileManagerAPI", {"ipc:///tmp/", "profileManagerAPI"}}, \
        };

        /**
         * @brief      Constructs a new instance.
         */
        zmqConnections():
            username{"root"}
        {
            uid_t uid = ::getuid();
            struct passwd* pw = ::getpwuid(uid);
            if(pw)
            {
                username = std::string(pw->pw_name);
            }
        };

    public:
        /**
         * @brief      Gets the socket.
         *
         * @param[in]  infName  The interface name
         *
         * @return     The socket.
         */
        [[nodiscard]] static const std::string getSocket(const std::string& infName)
        {
            static zmqConnections instance;
            return instance.mapping[infName].first + instance.username + std::string("-") + instance.mapping[infName].second;
        }
    };
}

#endif
