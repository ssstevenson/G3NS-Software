#include "mqttInterfaceParser.h"

#include <helpers/jsonUpdateHelpers.h>
#include <helpers/zmqConnectionNames.h>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      ctx     The context
 * @param      poller  The poller
 * @param      config  The configuration
 */
mqttInterfaceParser::mqttInterfaceParser(zmq::context_t& ctx, helpers::poller& poller,
        configManagerConnection& config):
    parserInterface{ctx, poller, config},
    zmqCtx{ctx},
    ctrl{},
    msgIf{ctx},
    defaultQos{0},
    setupConfigTrue{"{\"setup\": true}"},
    setupConfigFalse{"{\"setup\": false}"},
    configMap{},
    commandMap{},
    mqttRunning{false},
    statusSocket{helpers::getSockConnect(ctx, zmq::socket_type::push,
        enums::zmqConnections::getSocket("getStatusUpdate"))},
    configSocket{helpers::getSockConnect(ctx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getConfigurationManagerAPI"))}
{
    registerCmdSock();

    registerParser("configChangeNotify",
        std::bind(&mqttInterfaceParser::configChangeNotifyHandler, this, std::placeholders::_1));

    ctrl.registerTopicHandler("empower/status",
        std::bind(&mqttInterfaceParser::mqttStatusHandler, this, std::placeholders::_1));
    ctrl.registerTopicHandler("empower/log",
        std::bind(&mqttInterfaceParser::mqttLoggerHandler, this, std::placeholders::_1));
    ctrl.registerTopicHandler("empower/config/request",
        std::bind(&mqttInterfaceParser::mqttConfigHandler, this, std::placeholders::_1));
    ctrl.registerTopicHandler("empower/config/deregister",
        std::bind(&mqttInterfaceParser::mqttConfigDeregisterHandler, this, std::placeholders::_1));

    ctrl.registerTopicHandler("empower/cmd/result",
        std::bind(&mqttInterfaceParser::mqttCmdResultHandler, this, std::placeholders::_1));
    ctrl.registerTopicHandler("empower/cmd/register",
        std::bind(&mqttInterfaceParser::mqttCmdRegisterHandler, this, std::placeholders::_1));

    ctrl.setupLwt("empower/setup", setupConfigFalse, 0, true);

    registerConfigSet({"MQTT_INTERFACE_ENABLED"}, std::bind(&mqttInterfaceParser::setup, this));
}

/**
 * @brief      Destroys the object.
 */
mqttInterfaceParser::~mqttInterfaceParser()
{
    ctrl.publish("empower/cmd/setup", setupConfigFalse, defaultQos, true);
}

/**
 * @brief      Sets up the MQTT Interface based on configuration parameter
 */
void mqttInterfaceParser::setup() noexcept
{
    mqttRunning = configIf.getParamSet<bool>("MQTT_INTERFACE_ENABLED").value_or(false);

    if(mqttRunning)
    {
        ctrl.publish("empower/setup", setupConfigTrue, defaultQos, true);
    }
    else
    {
        ctrl.publish("empower/setup", setupConfigFalse, defaultQos, true);
    }
}

/**
 * @brief      MQTT Status Message Handler
 *
 * @param[in]  msg   The message
 */
void mqttInterfaceParser::mqttStatusHandler(const struct mosquitto_message *msg) noexcept
{
    if(!mqttRunning)
    {
        return;
    }

    helpers::sendString(mqttMsgToStr(msg), statusSocket.get());
}

/**
 * @brief      MQTT Configuration Message Handler
 *
 * @param[in]  msg   The message
 */
void mqttInterfaceParser::mqttConfigHandler(const struct mosquitto_message *msg) noexcept
{
    if(!mqttRunning)
    {
        return;
    }

    const std::string msgPayloadStr{mqttMsgToStr(msg)};

    if(rapidjson::Document jsonDoc; !jsonDoc.Parse(msgPayloadStr.c_str()).HasParseError())
    {
        auto topic{helpers::jsonGet<std::string>(jsonDoc, "resultTopic")};
        auto configKey{helpers::jsonGet<std::string>(jsonDoc, "name")};
        if(topic.has_value() && configKey.has_value())
        {
            if(auto confIt{configMap.find(configKey.value())}; confIt != configMap.end())
            {
                confIt->second.emplace(msgPayloadStr);
            }
            else
            {
                configMap.emplace(configKey.value(), std::set<std::string>{msgPayloadStr});
            }

            pushConfigMessage(msgPayloadStr);
        }
    }
}

/**
 * @brief      Called from Last Will and Testiment from peripheral device to remove its IP Address from monitoring
 *
 * @param[in]  msg   The message
 */
void mqttInterfaceParser::mqttConfigDeregisterHandler(const struct mosquitto_message *msg) noexcept
{
    const std::string msgPayloadStr{mqttMsgToStr(msg)};
    if(rapidjson::Document jsonDoc; !jsonDoc.Parse(msgPayloadStr.c_str()).HasParseError())
    {
        if(auto removeIp{helpers::jsonGet<std::string>(jsonDoc, "deregisterIpAddr")}; removeIp.has_value())
        {
            // For Configuration Keys
            for(auto& [key, set]: configMap)
            {
                for(auto setIt{set.begin()}; setIt != set.end(); setIt++)
                {
                    // create JSON from set message
                    if(rapidjson::Document setJson; !setJson.Parse(setIt->c_str()).HasParseError())
                    {
                        // if resultTopic contains IP address, remove this from the set
                        if(const auto topic{helpers::jsonGet<std::string>(jsonDoc, "resultTopic")}; topic.has_value())
                        {
                            if(topic.value().find(removeIp.value()) != std::string::npos)
                            {
                                set.erase(setIt);
                                break;
                            }
                        }
                    }
                }

                // if the set is now empty, remove this whole key
                if(set.empty())
                {
                    configMap.erase(key);
                }
            }

            // For Commands
            for(const auto& [key, set]: commandMap)
            {
                if(key.find(removeIp.value()) != std::string::npos)
                {
                    for(const auto& type: set)
                    {
                        deregisterParser(type);
                    }

                    commandMap.erase(key);
                    break;
                }
            }
        }
    }
}

/**
 * @brief      Pushes a configuration message.
 *
 * @param[in]  msgPayloadStr  The message payload string
 */
void mqttInterfaceParser::pushConfigMessage(const std::string& msgPayloadStr) noexcept
{
    if(rapidjson::Document jsonDoc; !jsonDoc.Parse(msgPayloadStr.c_str()).HasParseError())
    {
        if(auto topic{helpers::jsonGet<std::string>(jsonDoc, "resultTopic")}; topic.has_value())
        {
            if(helpers::sendReqRep(jsonDoc, configSocket.get(),
                std::bind(&mqttInterfaceParser::resetConfigSocket, this)))
            {
                ctrl.publish(topic.value(), helpers::jsonToStr(jsonDoc), defaultQos, false);
            }
        }
    }
}

/**
 * @brief      Handler method for Configuration Change Notify messages
 *
 * @param      jsonDoc  The json document
 */
void mqttInterfaceParser::configChangeNotifyHandler(rapidjson::Document& jsonDoc) noexcept
{
    if(configIf.updateNeeded(jsonDoc))
    {
        setup();
    }

    if(mqttRunning)
    {
        if(auto keyStr{helpers::jsonGet<std::string>(jsonDoc, "item", "name")}; keyStr.has_value())
        {
            if(auto confIt{configMap.find(keyStr.value())}; configMap.end() != confIt)
            {
                for(const auto& msg: confIt->second)
                {
                    pushConfigMessage(msg);
                }
            }

            return;
        }
    }

    const std::string errMsg{"Failed to execute configChangeNotifyHandler"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      MQTT Command Result Message Handler
 *
 * @param[in]  msg   The message
 */
void mqttInterfaceParser::mqttCmdResultHandler(const struct mosquitto_message *msg) noexcept
{
    if(!mqttRunning)
    {
        return;
    }

    helpers::sendString(mqttMsgToStr(msg), getResponseSockRef());
}

/**
 * @brief      MQTT Command Register Message Handler
 *
 * @param[in]  msg   The message
 */
void mqttInterfaceParser::mqttCmdRegisterHandler(const struct mosquitto_message *msg) noexcept
{
    if(!mqttRunning)
    {
        return;
    }

    if(rapidjson::Document jsonDoc;
        !jsonDoc.Parse(static_cast<char*>(msg->payload), static_cast<std::size_t>(msg->payloadlen)).HasParseError())
    {
        if(auto topic{helpers::jsonGet<std::string>(jsonDoc, "topic")}; topic.has_value())
        {
            if(auto commands{helpers::jsonGetVec<std::string>(jsonDoc, "commands")}; commands.has_value())
            {
                auto [cmdMapIt, ignore] = commandMap.try_emplace(topic.value(), std::set<std::string>{});

                for(const auto& cmd: commands.value())
                {
                    cmdMapIt->second.emplace(cmd);
                    registerParser(cmd, [&, topicStr=topic.value()](const auto& doc){
                        ctrl.publish(topicStr, helpers::jsonToStr(doc), defaultQos, false);
                    });
                }
            }
        }
    }
}

/**
 * @brief      Sends a pre-configured JSON String to the logger
 *
 * @param[in]  msg   The message
 */
void mqttInterfaceParser::mqttLoggerHandler(const struct mosquitto_message *msg) noexcept
{
    if(!mqttRunning)
    {
        return;
    }

    logger::genLogRaw(mqttMsgToStr(msg));
}

/**
 * @brief      Converts an MQTT Message to a String
 *
 * @param[in]  msg   The message
 *
 * @return     The string representation of the MQTT message payload
 */
[[nodiscard]]
std::string mqttInterfaceParser::mqttMsgToStr(const struct mosquitto_message *msg) noexcept
{
    std::string msgStr{static_cast<char*>(msg->payload), static_cast<std::string::size_type>(msg->payloadlen)};
    return msgStr;
}

/**
 * @brief      Resets the Configuration Socket connection
 */
void mqttInterfaceParser::resetConfigSocket() noexcept
{
    if(configSocket)
    {
        configSocket.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    configSocket = helpers::getSockConnect(zmqCtx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getConfigurationManagerAPI"));
}
