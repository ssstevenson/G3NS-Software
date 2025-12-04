#include "configManagerConnection.h"

#include <helpers/zmqConnectionNames.h>
#include <thread>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      zmqCtx  The zmq context
 */
configManagerConnection::configManagerConnection(zmq::context_t& zmqCtx):
    ctx{zmqCtx},
    reqRepSock{helpers::getSockConnect(ctx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getConfigurationManagerAPI"))},
    msgIf{zmqCtx},
    requestJsonStr{},
    keys{},
    configParamFunctorMap{}
{
    requestJsonStr = msgIf.getMessageStr("getConfigRequestRead", true).value_or("");
}

/**
 * @brief      Checks if an update is needed from the parameters being updated
 *
 * @param[in]  doc   RapidJson Document
 *
 * @return     True if an internal update is necessary, False otherwise.
 */
bool configManagerConnection::updateNeeded(const rapidjson::Document& doc)
{
    auto singleResult{helpers::jsonGet<std::string>(doc, "item", "name")};
    auto arrayResults{helpers::jsonGetVec<std::string>(doc, "item", "names")};

    // Check the single result
    if (singleResult)
    {
        for (const auto& [key, func] : configParamFunctorMap)
        {
            if (const auto it{key.find(singleResult.value())}; it != key.end())
            {
                func();
            }
        }
    }

    // Check the array results
    if (arrayResults)
    {
        for (const auto& [key, func] : configParamFunctorMap)
        {
            for (const auto& name : arrayResults.value())
            {
                if (const auto it{key.find(name)}; it != key.end())
                {
                    func();
                    break;
                }
            }
        }
    }

    // Return true if either the single result or any of the array results are found in keys
    return (keys.find(singleResult.value_or("")) != keys.end()) ||
           (arrayResults && std::any_of(arrayResults->begin(), arrayResults->end(), 
               [this](const std::string& name) { return keys.find(name) != keys.end(); }));
}

/**
 * @brief      Registers a function to be called with a set of keys
 *
 * @param[in]  key   The key
 * @param      func  The function
 */
void configManagerConnection::registerConfigFunction(const keySet_t& key, const std::function<void(void)>& func)
{
    configParamFunctorMap.emplace(std::make_pair(std::move(key), std::move(func)));
}

/**
 * @brief      Set's up the ZMQ Socket
 */
void configManagerConnection::setupSocket()
{
    if(reqRepSock)
    {
        reqRepSock.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    reqRepSock = helpers::getSockConnect(ctx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getConfigurationManagerAPI"));
}
