#ifndef MQTT_INTERFACE_PARSER_H_
#define MQTT_INTERFACE_PARSER_H_

#include <helpers/mainHelper.h>
#include <map>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <mqtt/mqtt.h>
#include <parser/parserInterface.h>
#include <set>

namespace empower
{
    /**
     * @brief      This class describes a switch manager parser.
     */
    class mqttInterfaceParser: protected parserInterface
    {
    public:
        using configUpdateMap = std::map<const std::string, std::set<std::string>>; // Key -> Request JSON

    private:
        zmq::context_t& zmqCtx;
        mqtt ctrl;
        messageFactoryConnection msgIf;
        int defaultQos;
        const std::string setupConfigTrue;
        const std::string setupConfigFalse;
        configUpdateMap configMap;
        configUpdateMap commandMap;
        bool mqttRunning;

        std::unique_ptr<zmq::socket_t> statusSocket;
        std::unique_ptr<zmq::socket_t> configSocket;

    public:
        mqttInterfaceParser(zmq::context_t& zmqCtx, helpers::poller& poller, configManagerConnection& config);
        ~mqttInterfaceParser();

    private:
        void setup() noexcept;
        void mqttStatusHandler(const struct mosquitto_message *msg) noexcept;
        void mqttConfigHandler(const struct mosquitto_message *msg) noexcept;
        void mqttConfigDeregisterHandler(const struct mosquitto_message *msg) noexcept;
        void pushConfigMessage(const std::string& msgPayloadStr) noexcept;
        void configChangeNotifyHandler(rapidjson::Document& jsonDoc) noexcept;
        void mqttCmdResultHandler(const struct mosquitto_message *msg) noexcept;
        void mqttCmdRegisterHandler(const struct mosquitto_message *msg) noexcept;
        void mqttLoggerHandler(const struct mosquitto_message *msg) noexcept;

        [[nodiscard]] std::string mqttMsgToStr(const struct mosquitto_message *msg) noexcept;
        void resetConfigSocket() noexcept;
    };
}

#endif
