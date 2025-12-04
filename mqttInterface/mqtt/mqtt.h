#ifndef MQTT_H_
#define MQTT_H_

#include <functional>
#include <map>
#include <mosquittopp.h>
#include <string>
#include <vector>

namespace empower
{
    class mqtt:
        public mosqpp::mosquittopp
    {
    public:
        using msgCallback_t = std::function<void(const struct mosquitto_message *)>;

    private:
        std::map<std::string, msgCallback_t> topicHandlers;

    public:
        mqtt(const std::string& host="localhost", const std::uint16_t port=1883);
        ~mqtt();

        void registerTopicHandler(const std::string& topic, msgCallback_t func);
        bool setAuth(const std::string& username, const std::string& password);
        bool publish(const std::string& topic, const std::string& message, const int qos, const bool retain);
        void setupLwt(const std::string& topic, const std::string& message, const int qos, const bool retain);

    private:
        bool subscribe(const std::string& topic);
        void on_connect(int rc);
        void on_disconnect(int rc);
        void on_publish(int mid);
        void on_subscribe(int mid, int qos_count, const int *granted_qos);
        void on_message(const struct mosquitto_message *message);
    };
}

#endif // MQTT_H_
