#include "heartbeatResponse.h"

#include <helpers/jsonUpdateHelpers.h>
#include <helpers/zmqConnectionNames.h>
#include <helpers/zmqHelpers.h>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      ctx     The ZMQ Context
 * @param      poller  The Main poller setup
 * @param      conf    The Configuration interface
 * @param[in]  pName   The Process Name
 */
heartbeatResponse::heartbeatResponse(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& conf,
    const std::string& pName):
    parserInterface{ctx, poller, conf},
    msgIf{ctx},
    responseJsonStr{"getHeartbeatStatusResponse"}
{
    registerZmqSockets(zmq::socket_type::sub, zmq::socket_type::push,
        enums::zmqConnections::getSocket("getHeartbeatRequest"),
        enums::zmqConnections::getSocket("getHeartbeatResponse"));

    if(auto jsonDoc{msgIf.getMessageJson(responseJsonStr, true)}; jsonDoc.has_value())
    {
        helpers::jsonSet(jsonDoc.value(), pName, "processName");
        responseJsonStr = helpers::jsonToStr(jsonDoc.value());
    }

    registerParser("heartbeatRequest",
        [this](rapidjson::Document& doc){ processHeartbeat(doc); });
}

/**
 * @brief      Processes heartbeat messages
 *
 * @param      jsonDoc  The json document
 */
void heartbeatResponse::processHeartbeat(rapidjson::Document& jsonDoc)
{
    if(auto time{helpers::jsonGet<double>(jsonDoc, "epochMicroUTC")}; time.has_value())
    {
        jsonDoc.GetAllocator().Clear();
        if(!jsonDoc.Parse(responseJsonStr.c_str()).HasParseError())
        {
            helpers::jsonSet(jsonDoc, time.value(), "epochMicroUTC");
        }
    }
}
