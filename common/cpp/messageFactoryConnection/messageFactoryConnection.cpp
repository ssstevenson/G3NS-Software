#include "messageFactoryConnection.h"

#include <chrono>
#include <helpers/zmqConnectionNames.h>
#include <helpers/zmqHelpers.h>
#include <thread>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      zmqCtx  The zmq context
 */
messageFactoryConnection::messageFactoryConnection(zmq::context_t& zmqCtx):
    ctx(zmqCtx),
    reqRepSock{helpers::getSockConnect(zmqCtx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getMessageFactoryAPI"))},
    cache{}
{
    setupSocket();
}

/**
 * @brief      Gets the message response.
 *
 * @param[in]  orig    The original Json Document
 * @param[in]  msgStr  The message string
 *
 * @return     The message response if sucessful, std::nullopt otherwise.
 */
std::optional<rapidjson::Document> messageFactoryConnection::getMessageResponse(const rapidjson::Document& orig,
    const std::string& msgStr)
{
    if(auto sequenceNumber{helpers::jsonGet<std::uint64_t>(orig, "sequenceNumber")}; sequenceNumber.has_value())
    {
        if(auto resp{getMessageJson(msgStr, true)}; resp.has_value())
        {
            rapidjson::Document& jsonResp{resp.value()};

            jsonResp.AddMember("request", rapidjson::Value().CopyFrom(orig, jsonResp.GetAllocator()).Move(),
                jsonResp.GetAllocator());

            helpers::jsonSet(jsonResp, sequenceNumber.value(), "sequenceNumber");

            return resp;
        }
    }

    return std::nullopt;
}

/**
 * @brief      Gets the message string.
 *
 * @param[in]  msgStr        The message string
 * @param[in]  removeSeqNum  Indicates if the sequence number is removed
 *
 * @return     The message string if sucessful, std::nullopt otherwise.
 */
std::optional<std::string> messageFactoryConnection::getMessageStr(const std::string& msgStr, bool removeSeqNum)
{
    if(std::unordered_map<std::string, std::string>::const_iterator it{cache.find(msgStr)};
        removeSeqNum && (it != cache.end()))
    {
        return it->second;
    }

    if(auto resp{helpers::sendReqRep(msgStr, reqRepSock.get(), [this](){ setupSocket(); })}; resp.has_value())
    {
        std::string result{resp.value()};

        if(rapidjson::Document workingDoc;
            removeSeqNum && !workingDoc.Parse(result.c_str(), result.size()).HasParseError())
        {
            workingDoc.RemoveMember("sequenceNumber");
            result = helpers::jsonToStr(workingDoc);
            cache.insert(std::make_pair(msgStr, result));
        }

        return result;
    }

    logger::warn(__FILE__, __FUNCTION__, "Failed to get message from Message Factory");

    return std::nullopt;
}

/**
 * @brief      Gets the message json.
 *
 * @param[in]  msgStr        The message string
 * @param[in]  removeSeqNum  Indicates if the sequence number is removed
 *
 * @return     The message json if sucessful, std::nullopt otherwise.
 */
std::optional<rapidjson::Document> messageFactoryConnection::getMessageJson(const std::string& msgStr,
    bool removeSeqNum)
{
    if(auto msg{getMessageStr(msgStr, removeSeqNum)}; msg.has_value())
    {
        std::string& tmpStr{msg.value()};
        if(rapidjson::Document response; !response.Parse(tmpStr.c_str(), tmpStr.size()).HasParseError())
        {
            return response;
        }
    }

    return std::nullopt;
}

/**
 * @brief      Converts a Sucess Json Document to an Error Json Document
 *
 * @param      jsonDoc  The json document
 * @param[in]  desc     The description
 *
 * @return     True if the conversion was sucessful, False otherwise.
 */
bool messageFactoryConnection::convertSuccessToErrorResponse(rapidjson::Document& jsonDoc, const std::string& desc)
{
    return helpers::jsonSet<std::string>(jsonDoc, "error", "result") &&
        helpers::jsonSet<std::string>(jsonDoc, "General Error.", "resultMessage") &&
        helpers::jsonSet<std::string>(jsonDoc, desc, "errorDescription");
}

/**
 * @brief      Sets up the ZMQ Socket
 */
void messageFactoryConnection::setupSocket()
{
    if(reqRepSock)
    {
        reqRepSock.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    reqRepSock = helpers::getSockConnect(ctx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getMessageFactoryAPI"));
}
