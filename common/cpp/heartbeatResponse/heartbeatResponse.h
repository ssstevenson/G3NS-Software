#ifndef HEARBEAT_RESPONSE_H_
#define HEARBEAT_RESPONSE_H_

#include <messageFactoryConnection/messageFactoryConnection.h>
#include <parser/parserInterface.h>

namespace empower
{
    /**
     * @brief      This class describes the heartbeat Response module.
     */
    class heartbeatResponse final: protected parserInterface
    {
    private:
        messageFactoryConnection msgIf;
        std::string responseJsonStr;

    public:
        heartbeatResponse(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& conf,
            const std::string& pName);
        void processHeartbeat(rapidjson::Document& jsonDoc);
    };
}

#endif
