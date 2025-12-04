#include "serialHardwareInterfaceParser.h"

#include <helpers/jsonUpdateHelpers.h>
#include <helpers/types.h>
#include <helpers/zmqConnectionNames.h>
#include <iomanip>
#include <logger/logger.h>
#include <regex>
#include <sstream>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      ctx          The context
 * @param      poller       The poller
 * @param      conf         The conf
 * @param[in]  sockNameStr  The sock name string
 * @param[in]  baseAddr     The base address
 */
serialHardwareInterfaceParser::serialHardwareInterfaceParser(zmq::context_t& ctx, helpers::poller& poller,
        configManagerConnection& conf, const std::string& sockNameStr, const ser_reg_t baseAddr):
    parserInterface{ctx, poller, conf},
    serIf{baseAddr},
    msgIf{ctx},
    serialResponseSuccessJsonStr{},
    serialResponseErrorJsonStr{}
{
    registerZmqSockets(zmq::socket_type::rep, enums::zmqConnections::getSocket(sockNameStr));

    registerParser("serialRequest", [this](rapidjson::Document& doc){ jsonResponse(doc); });

    serialResponseSuccessJsonStr = msgIf.getMessageStr("getSerialHardwareResponseSuccess", true).value_or("");
    serialResponseErrorJsonStr = msgIf.getMessageStr("getSerialHardwareResponseError", true).value_or("");
}

/**
 * @brief      Processes a Json command
 *
 * @param      jsonDoc  The json document
 */
void serialHardwareInterfaceParser::jsonResponse(rapidjson::Document& jsonDoc)
{
    using namespace helpers;

    if(auto dataStr{jsonGet<std::string>(jsonDoc, "data")}; dataStr.has_value())
    {
        auto data = types::dataTokenizer<ser_data_t>(dataStr.value());

        logger::debug(__FILE__, __FUNCTION__, "Reading Message...");

        auto parityStr{jsonGet<std::string>(jsonDoc, "parity")};
        auto stopStr{jsonGet<std::string>(jsonDoc, "stopBits")};
        auto startTimeout{jsonGet<std::uint32_t>(jsonDoc, "timeoutMessageMs")};
        auto charTimeout{jsonGet<std::uint32_t>(jsonDoc, "timeoutByteMs")};
        auto baud{jsonGet<std::uint32_t>(jsonDoc, "baudRate")};
        auto width{jsonGet<std::uint32_t>(jsonDoc, "symbolBitWidth")};
        auto kissEncoding{jsonGet<bool>(jsonDoc, "kissEncoding").value_or(false)};

        if(kissEncoding)
        {
            data = kissEncode(data);
        }

        if(parityStr.has_value() && stopStr.has_value() && startTimeout.has_value() && charTimeout.has_value() &&
            baud.has_value() && width.has_value())
        {
            if(serIf.setBaud(baud.value()) && serIf.setBits(width.value()) &&
                serIf.setStop(serIf.strToStop(stopStr.value())) &&
                serIf.setParity(serIf.strToParity(parityStr.value())))
            {
                std::string error{"Timeout"};

                try
                {
                    if(auto readData{serIf.read(data, std::chrono::milliseconds(startTimeout.value()),
                        std::chrono::milliseconds(charTimeout.value()))}; readData.has_value())
                    {
                        setupJsonResponse(jsonDoc, serialResponseSuccessJsonStr);
                        std::string responseStr;

                        if(kissEncoding)
                        {
                            responseStr = types::vecToDataStr(kissDecode(readData.value()));
                        }
                        else
                        {
                            responseStr = types::vecToDataStr(readData.value());
                        }

                        jsonSet(jsonDoc, responseStr, "response", "data");
                        return;
                    }
                }
                catch(const std::exception& e)
                {
                    error = e.what();
                }

                setupJsonResponse(jsonDoc, serialResponseErrorJsonStr);
                jsonSet(jsonDoc, error, "response", "error");
            }
        }
        else
        {
            logger::debug(__FILE__, __FUNCTION__, "Writing Message...");
            serIf.write(data);
        }
    }
}

/**
 * @brief      Kiss encodes a message.
 *
 * @param[in]  in    The initial message
 *
 * @return     Kiss encoded message.
 */
std::vector<serialHardwareInterfaceParser::ser_data_t> serialHardwareInterfaceParser::kissEncode(
    const std::vector<ser_data_t>& in)
{
    std::vector<ser_data_t> output{ FEND };

    for(const auto & b: in)
    {
        if(FEND == b)
        {
            output.push_back(FESC);
            output.push_back(TFEND);
        }
        else if(FESC == b)
        {
            output.push_back(FESC);
            output.push_back(TFESC);
        }
        else
        {
            output.push_back(b);
        }
    }

    output.push_back(FEND);

    return output;
}

/**
 * @brief      Kiss decodes a message.
 *
 * @param[in]  in    The message to decode
 *
 * @return     The decoded message.
 */
std::vector<serialHardwareInterfaceParser::ser_data_t> serialHardwareInterfaceParser::kissDecode(
    const std::vector<ser_data_t>& in)
{
    std::vector<ser_data_t> output;

    if((FEND == in.front()) && (FEND == in.back()))
    {
        for(std::vector<ser_data_t>::const_iterator it = std::next(std::begin(in));
            it != std::prev(std::end(in)); it++)
        {
            if(FESC == *it)
            {
                it++;

                if(TFEND == *it)
                {
                    output.push_back(FEND);
                }
                else if(TFESC == *it)
                {
                    output.push_back(FESC);
                }
            }
            else
            {
                output.push_back(*it);
            }
        }
    }

    return output;
}
