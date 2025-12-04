#ifndef MESSAGE_FACTORY_CONNECTION_H_
#define MESSAGE_FACTORY_CONNECTION_H_

#include <helpers/jsonUpdateHelpers.h>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      This class describes a message factory connection.
     */
    class messageFactoryConnection
    {
    private:
        zmq::context_t& ctx;
        std::unique_ptr<zmq::socket_t> reqRepSock;
        std::unordered_map<std::string, std::string> cache;

    public:
        messageFactoryConnection(zmq::context_t& zmqCtx);

        [[nodiscard]] std::optional<rapidjson::Document> getMessageResponse(const rapidjson::Document& orig,
            const std::string& msgStr);
        [[nodiscard]] std::optional<std::string> getMessageStr(const std::string& msgStr, bool removeSeqNum);
        [[nodiscard]] std::optional<rapidjson::Document> getMessageJson(const std::string& msgStr, bool removeSeqNum);

        bool convertSuccessToErrorResponse(rapidjson::Document& jsonDoc, const std::string& desc);

    private:
        void setupSocket();
    };
}

#endif
