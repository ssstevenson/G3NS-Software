#ifndef CONFIG_MANAGER_CONNECTION_H_
#define CONFIG_MANAGER_CONNECTION_H_

#include <functional>
#include <helpers/jsonUpdateHelpers.h>
#include <helpers/zmqConnectionNames.h>
#include <helpers/zmqHelpers.h>
#include <logger/logger.h>
#include <map>
#include <memory>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <set>
#include <string>
#include <thread>
#include <zmq.hpp>

#include <iostream>

namespace empower
{
    /**
     * @brief      This class describes a configuration manager connection.
     */
    class configManagerConnection
    {
    public:
        using keySet_t = std::set<std::string>;

    private:
        zmq::context_t& ctx;
        std::unique_ptr<zmq::socket_t> reqRepSock;
        messageFactoryConnection msgIf;
        std::string requestJsonStr;
        keySet_t keys;
        std::map<keySet_t, std::function<void(void)>> configParamFunctorMap;

        static constexpr std::size_t retryCnt = 5;

    public:
        configManagerConnection(zmq::context_t& zmqCtx);

        template<typename T> [[nodiscard]] std::optional<T> getParam(const std::string& key);
        template<typename T> [[nodiscard]] std::optional<T> getParamSet(const std::string& key);
        [[nodiscard]] bool updateNeeded(const rapidjson::Document& doc);
        void registerConfigFunction(const keySet_t& key, const std::function<void(void)>& func);

    private:
        void setupSocket();
    };

    /**
     * @brief      Gets a parameter and sets the generic keys.
     *
     * @param[in]  key   The key
     *
     * @tparam     T     Parameter Type
     *
     * @return     The parameter if it can, std::nullopt otherwise.
     */
    template<typename T> std::optional<T> configManagerConnection::getParam(const std::string& key)
    {
        keys.insert(key);
        return getParamSet<T>(key);
    }

    /**
     * @brief      Gets a parameter.
     *
     * @param[in]  key   The key
     *
     * @tparam     T     Parameter Type
     *
     * @return     The parameter if it can, std::nullopt otherwise.
     */
    template<typename T> std::optional<T> configManagerConnection::getParamSet(const std::string& key)
    {
        using std::string_literals::operator""s;

        logger::verbose(__FILE__, __FUNCTION__, "Retrieving Configuration Key: "s + key);

        for(std::size_t retryVal{0}; retryVal < retryCnt; ++retryVal)
        {
            try
            {
                if(rapidjson::Document workingDoc;
                    !workingDoc.Parse(requestJsonStr.c_str(), requestJsonStr.size()).HasParseError())
                {
                    if(helpers::jsonSet(workingDoc, key, "name"))
                    {
                        if(helpers::sendReqRep(workingDoc, reqRepSock.get(), [this](){ setupSocket(); }))
                        {
                            auto data{helpers::jsonGet<T>(workingDoc, "response", "value")};

                            if(data)
                            {
                                if constexpr(std::is_same_v<std::string, T>)
                                {
                                    logger::verbose(__FILE__, __FUNCTION__, key + " => "s + data.value());
                                }
                                else if constexpr(std::is_same_v<helpers::types::decay_rate_t, T>)
                                {
                                    logger::verbose(__FILE__, __FUNCTION__, key + " => "s +
                                        std::to_string(data.value().count()));
                                }
                                else
                                {
                                    logger::verbose(__FILE__, __FUNCTION__, key + " => "s + std::to_string(data.value()));
                                }
                            }

                            return data;
                        }
                    }
                }
            }
            catch(...) {}

            logger::warn(__FILE__, __FUNCTION__, "Failed to get parameter from Configuration Manager");

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        const auto errMsg{"Configuration Manager Could Not Get Value for key ( "s + key + " ), Terminating..."s};
        logger::critical(__FILE__, __FUNCTION__, errMsg);
        std::cout << errMsg << std::endl;
        std::terminate();

        return std::nullopt;
    }
}

#endif
