#include "serialInterfaceConnection.h"

#include <helpers/jsonUpdateHelpers.h>
#include <helpers/types.h>
#include <helpers/zmqConnectionNames.h>
#include <helpers/zmqHelpers.h>
#include <regex>
#include <thread>
#include <zmq.hpp>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      zmqCtx    The zmq context
 * @param[in]  connName  The connection name
 */
serialInterfaceConnection::serialInterfaceConnection(zmq::context_t& zmqCtx, const std::string& connName):
    ctx{zmqCtx},
    connectionName{enums::zmqConnections::getSocket(connName)},
    reqRepSock{helpers::getSockConnect(zmqCtx, zmq::socket_type::req, connectionName)},
    msgIf{zmqCtx},
    serialRequestJsonStr{}
{
    serialRequestJsonStr = msgIf.getMessageStr("getSerialHardwareRequest", true).value_or("");
}

/**
 * @brief      Writes a byte to the UART
 *
 * @param[in]  datum  The data to be written
 *
 * @return     True if the data was written sucessfully, False otherwise.
 */
bool serialInterfaceConnection::write(const ser_msg_t& datum, const bool kissEncoding)
{
    return read(datum, timeout_t{0}, timeout_t{0}, kissEncoding).has_value();
}

/**
 * @brief      Reads data from the UART Hardware
 *
 * @param[in]  datum         The data
 * @param[in]  startTimeout  The start timeout
 * @param[in]  charTimeout   The character timeout
 *
 * @return     The read data if sucessful, std::nullopt otherwise.
 */
std::optional<serialInterfaceConnection::ser_msg_t> serialInterfaceConnection::read(const ser_msg_t& datum,
    timeout_t startTimeout, timeout_t charTimeout, const bool kissEncoding)
{
    using std::string_literals::operator""s;

    try
    {
        if(auto json{helpers::sendReqRep(setupJson(datum, startTimeout, charTimeout, kissEncoding).value_or(""), reqRepSock.get(),
            [this](){ setupSocket(); })}; json.has_value())
        {
            if(rapidjson::Document workingDoc;
                !workingDoc.Parse(json.value().c_str(), json.value().size()).HasParseError())
            {
                if(auto dataStr{helpers::jsonGet<std::string>(workingDoc, "response", "data")}; dataStr.has_value())
                {
                    return helpers::types::dataTokenizer<ser_data_t>(dataStr.value());
                }

                if(auto dataStr{helpers::jsonGet<std::string>(workingDoc, "response", "error")}; dataStr.has_value())
                {
                    logger::warn(__FILE__, __FUNCTION__, "SHI Error - "s + dataStr.value());
                }
            }
        }
    }
    catch(const zmq::error_t& ze)
    {
        logger::warn(__FILE__, __FUNCTION__, "Caught a ZMQ Error - "s + std::string(ze.what()));
    }
    catch(const std::exception& e)
    {
        logger::warn(__FILE__, __FUNCTION__, "Unexpected Error - "s + std::string(e.what()));
    }
    catch(...)
    {
        logger::warn(__FILE__, __FUNCTION__, "Unexpected Non-Standard Error");
    }

    return std::nullopt;
}

/**
 * @brief      Sets up a Json Document to be sent to the SHI
 *
 * @param[in]  datum         The data
 * @param[in]  startTimeout  The start timeout
 * @param[in]  charTimeout   The character timeout
 *
 * @return     Stringified Json if sucessful, std::nullopt otherwise.
 */
std::optional<std::string> serialInterfaceConnection::setupJson(const ser_msg_t& datum, timeout_t startTimeout,
    timeout_t charTimeout, const bool kissEncoding)
{
    if(rapidjson::Document workingDoc;
        !workingDoc.Parse(serialRequestJsonStr.c_str(), serialRequestJsonStr.size()).HasParseError())
    {
        if(helpers::jsonSet(workingDoc, helpers::types::vecToDataStr(datum), "data") &&
            helpers::jsonSet(workingDoc, startTimeout.count(), "timeoutMessageMs") &&
            helpers::jsonSet(workingDoc, charTimeout.count(), "timeoutByteMs") &&
            helpers::jsonSet(workingDoc, kissEncoding, "kissEncoding"))
        {
            return helpers::jsonToStr(workingDoc);
        }
    }

    return std::nullopt;
}

/**
 * @brief      Sets up the ZMQ Socket.
 */
void serialInterfaceConnection::setupSocket()
{
    if(reqRepSock)
    {
        reqRepSock.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    reqRepSock = helpers::getSockConnect(ctx, zmq::socket_type::req, connectionName);
}
