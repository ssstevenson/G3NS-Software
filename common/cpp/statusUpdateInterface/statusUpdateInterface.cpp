#include "statusUpdateInterface.h"

#include <helpers/jsonUpdateHelpers.h>
#include <helpers/zmqConnectionNames.h>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      zmqCtx  The zmq context
 */
statusUpdateInterface::statusUpdateInterface(zmq::context_t& zmqCtx):
    ctx{zmqCtx},
    msgIf{zmqCtx},
    sock{helpers::getSockConnect(zmqCtx, zmq::socket_type::push,
        enums::zmqConnections::getSocket("getStatusUpdate"))},
    statusUpdateJsonStr{}
{
    statusUpdateJsonStr = msgIf.getMessageStr("getStatusUpdateRequest", true).value_or("");
}

/**
 * @brief      Gets the status document.
 *
 * @param      response  The response
 *
 * @return     True if the string was properly formatted, False otherwise.
 */
bool statusUpdateInterface::getStatusDoc(rapidjson::Document& response)
{
    response.GetAllocator().Clear();
    return !response.Parse(statusUpdateJsonStr.c_str(), statusUpdateJsonStr.size()).HasParseError();
}

/**
 * @brief      Sends a status document.
 *
 * @param      jsonDoc  The json document
 *
 * @return     True if the status document was transmitted properly, False otherwise.
 */
bool statusUpdateInterface::sendStatusDoc(rapidjson::Document& jsonDoc)
{
    if(!helpers::sendJsonDoc(jsonDoc, sock.get()))
    {
        logger::warn(__FILE__, __FUNCTION__, "Failure to Send JSON Doc to Status Manager");
        return false;
    }

    return true;
}

/**
 * @brief      Validates that a point in the Json Doc exists and that it is not an array
 *
 * @param      it       The iterator
 * @param[in]  jsonDoc  The json document
 *
 * @return     True if the member exists and it is not an array, False otherwise.
 */
bool statusUpdateInterface::validate(rapidjson::Value::MemberIterator& it, const rapidjson::Document& jsonDoc)
{
    if(it == jsonDoc.MemberEnd())
    {
        return false;
    }

    if(!it->value.IsArray())
    {
        return false;
    }

    return true;
}
