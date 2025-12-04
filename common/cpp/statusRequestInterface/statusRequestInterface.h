#ifndef STATUS_REQUEST_INTERFACE_H_
#define STATUS_REQUEST_INTERFACE_H_

#include <chrono>
#include <helpers/jsonUpdateHelpers.h>
#include <helpers/zmqConnectionNames.h>
#include <helpers/zmqHelpers.h>
#include <memory>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      This class describes a status request interface.
     */
    class statusRequestInterface
    {
    private:
        zmq::context_t& ctx;
        std::unique_ptr<zmq::socket_t> reqRepSock;
        messageFactoryConnection msgIf;
        std::string statusRequestJsonStr;
        static constexpr std::uint32_t retryCnt = 3;

    public:
        statusRequestInterface(zmq::context_t& zmqCtx);

        template<typename T, typename ...Types> [[nodiscard]] std::optional<T> getStatusParam(const std::string& key,
            const Types... parms);

    private:
        void setupSocket();
    };

    /**
     * @brief      Gets the status parameter.
     *
     * @param[in]  key   The key
     * @param[in]  parm  The parameter
     *
     * @tparam     T     Type to return the parameter as
     *
     * @return     The status parameter.
     */
    template<typename T, typename ...Types> std::optional<T> statusRequestInterface::getStatusParam(
        const std::string& key, const Types... parms)
    {
        for(std::uint32_t i = 0; i < retryCnt; ++i)
        {
            try
            {
                if(rapidjson::Document workingDoc;
                   !workingDoc.Parse(statusRequestJsonStr.c_str(), statusRequestJsonStr.size()).HasParseError())
                {
                    if(helpers::jsonSet(workingDoc, key, "key"))
                    {
                        if(helpers::sendReqRep(workingDoc, reqRepSock.get(), [this](){ setupSocket(); }))
                        {
                            if constexpr(std::is_same_v<rapidjson::Document, T>)
                            {
                                return workingDoc;
                            }
                            else if constexpr(sizeof...(parms) == 0)
                            {
                                return helpers::jsonGet<T>(workingDoc, "resultPayload", "value");
                            }
                            else
                            {
                                return helpers::jsonGet<T>(workingDoc, parms...);
                            }
                        }
                    }
                }
            }
            catch(const std::exception& e)
            {
                logger::warn(__FILE__, __FUNCTION__, "Failed to get parameter from Status Processor");
            }
        }

        return std::nullopt;
    }
}
#endif
