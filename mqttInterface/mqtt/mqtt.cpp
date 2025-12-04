#include "mqtt.h"

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  host  The host IP Address
 * @param[in]  port  The port
 */
mqtt::mqtt(const std::string& host, const std::uint16_t port):
    topicHandlers{}
{
    mosqpp::lib_init();
    connect_async(host.c_str(), port, 60);
    loop_start();
}

/**
 * @brief      Destroys the object.
 */
mqtt::~mqtt()
{
    disconnect();
    loop_stop();
    mosqpp::lib_cleanup();
}

/**
 * @brief      Registers a Topic Handler
 *
 * @param[in]  topic  The topic
 * @param[in]  func   The function
 */
void mqtt::registerTopicHandler(const std::string& topic, msgCallback_t func)
{
    topicHandlers.emplace(topic, func);
    subscribe(topic);
}

/**
 * @brief      Sets the auth.
 *
 * @param[in]  username  The username
 * @param[in]  password  The password
 *
 * @return     True on success, False otherwise
 */
bool mqtt::setAuth(const std::string& username, const std::string& password)
{
    auto result{mosquittopp::username_pw_set(username.c_str(), password.c_str())};
    return (result == MOSQ_ERR_SUCCESS);
}

/**
 * @brief      Publishes a message to a given topic
 *
 * @param[in]  topic    The topic
 * @param[in]  message  The message
 * @param[in]  qos      The qos
 * @param[in]  retain   The retain
 *
 * @return     True on success, False otherwise
 */
bool mqtt::publish(const std::string& topic, const std::string& message, const int qos, const bool retain)
{
    int answer{
        mosqpp::mosquittopp::publish(nullptr, topic.c_str(),
            static_cast<int>(message.size()), message.c_str(), qos, retain)
    };
    return (answer == MOSQ_ERR_SUCCESS);
}

/**
 * @brief      Sets up Last Will and Testament message with the broker
 *
 * @param[in]  topic    The topic
 * @param[in]  message  The message
 * @param[in]  qos      The qos
 * @param[in]  retain   The retain
 */
void mqtt::setupLwt(const std::string& topic, const std::string& message, const int qos, const bool retain)
{
    mosqpp::mosquittopp::will_set(topic.c_str(), static_cast<int>(message.size()), message.c_str(), qos, retain);
}

/**
 * @brief      Subscribes to a given topic
 *
 * @param[in]  topic  The topic
 *
 * @return     True on success, False otherwise
 */
bool mqtt::subscribe(const std::string& topic)
{
    auto result{mosquittopp::subscribe(nullptr, topic.c_str())};
    return (result == MOSQ_ERR_SUCCESS);
}

/**
 * @brief      Called on subscribe.
 *
 * @param[in]  mid          The middle
 * @param[in]  qos_count    The qos count
 * @param[in]  granted_qos  The granted qos
 */
void mqtt::on_subscribe(
    [[maybe_unused]] int mid, [[maybe_unused]] int qos_count, [[maybe_unused]] const int *granted_qos
)
{}

/**
 * @brief      Called on message.
 *
 * @param[in]  message  The message
 */
void mqtt::on_message(const struct mosquitto_message *message)
{
    if(auto it{topicHandlers.find(message->topic)}; it != topicHandlers.end())
    {
        it->second(message);
    }
}

/**
 * @brief      Called on disconnect.
 *
 * @param[in]  rc    The Result Code
 */
void mqtt::on_disconnect([[maybe_unused]] int rc)
{}

/**
 * @brief      Called on connect.
 *
 * @param[in]  rc    The Result Code
 */
void mqtt::on_connect([[maybe_unused]] int rc)
{}

/**
 * @brief      Called on publish.
 *
 * @param[in]  mid   The middle
 */
void mqtt::on_publish([[maybe_unused]] int mid)
{}
