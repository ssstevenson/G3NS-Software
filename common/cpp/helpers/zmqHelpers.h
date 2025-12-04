#ifndef ZMQ_HELPERS_H_
#define ZMQ_HELPERS_H_

#include <helpers/jsonUpdateHelpers.h>
#include <memory>
#include <zmq.hpp>

namespace empower::helpers
{
    /**
     * @brief      Creates a ZMQ Socket given a ZMQ Context and a Socket Type
     *
     * @param      ctx   The context
     * @param[in]  type  The type
     *
     * @return     unique_ptr owning the socket that was created.
     */
    [[nodiscard]]
    inline std::unique_ptr<zmq::socket_t> getSock(zmq::context_t& ctx, const zmq::socket_type type)
    {
        auto result = std::make_unique<zmq::socket_t>(ctx, type);
        if(zmq::socket_type::sub == type)
        {
            result->setsockopt(ZMQ_LINGER, 25);
            result->setsockopt(ZMQ_SNDTIMEO, 25);
            result->setsockopt(ZMQ_RCVTIMEO, 25);
            result->setsockopt(ZMQ_SUBSCRIBE, "", 0);
            result->setsockopt(ZMQ_CONFLATE, static_cast<std::int32_t>(true));
        }
        else if(zmq::socket_type::req == type)
        {
            result->setsockopt(ZMQ_LINGER, 500);
            result->setsockopt(ZMQ_SNDTIMEO, 500);
            result->setsockopt(ZMQ_RCVTIMEO, 500);
        }
        else if(zmq::socket_type::push == type)
        {
            result->setsockopt(ZMQ_LINGER, 25);
            result->setsockopt(ZMQ_SNDTIMEO, 25);
            result->setsockopt(ZMQ_RCVTIMEO, 25);
        }
        return result;
    }

    /**
     * @brief      Creates a ZMQ Socket, and connects it to a URI
     *
     * @param      ctx   The context
     * @param[in]  type  The type
     * @param[in]  uri   The uri
     *
     * @return     unique_ptr owning the created socket.
     */
    [[nodiscard]]
    inline std::unique_ptr<zmq::socket_t> getSockConnect(zmq::context_t& ctx, const zmq::socket_type type,
        const std::string& uri)
    {
        auto result = getSock(ctx, type);
        result->connect(uri);
        return result;
    }

    /**
     * @brief      Creates a ZMQ Socket, and binds it to a URI
     *
     * @param      ctx   The context
     * @param[in]  type  The type
     * @param[in]  uri   The uri
     *
     * @return     unique_ptr owning the created socket.
     */
    [[nodiscard]]
    inline std::unique_ptr<zmq::socket_t> getSockBind(zmq::context_t& ctx, const zmq::socket_type type,
        const std::string& uri)
    {
        auto result = getSock(ctx, type);
        result->bind(uri);
        return result;
    }

    /**
     * @brief      Sends a json document.
     *
     * @param      jsonDoc  The json document
     * @param      sock     The sock
     *
     * @return     True if the document is sucesfully sent, False otherwise.
     */
    inline bool sendJsonDoc(rapidjson::Document& jsonDoc, zmq::socket_t* sock) noexcept
    {
        try
        {
            return sock->send(jsonToMsg(jsonDoc), zmq::send_flags::none).has_value();
        }
        catch(...)
        {}

        return false;
    }

    /**
     * @brief      Sends a string.
     *
     * @param[in]  str   The string
     * @param      sock  The sock
     *
     * @return     True if the string is sucessfully sent, False otherwise.
     */
    inline bool sendString(const std::string& str, zmq::socket_t* sock) noexcept
    {
        try
        {
            return sock->send(zmq::message_t(str.c_str(), str.size()), zmq::send_flags::none).has_value();
        }
        catch(...)
        {}

        return false;
    }

    /**
     * @brief      Sends a string to a Request Reply ZMQ Socket
     *
     * @param[in]  msgStr    The message string
     * @param      sock      The socket
     * @param[in]  reset     Socket reset callback function
     * @param[in]  retryCnt  The retry count
     *
     * @return     The response string it sucessful, std::nullopt otherwise.
     */
    [[nodiscard]] inline std::optional<std::string> sendReqRep(const std::string& msgStr, zmq::socket_t* sock,
        std::function<void(void)> reset, const std::size_t retryCnt = 3) noexcept
    {
        for(std::size_t i = 0; i < retryCnt; ++i)
        {
            try
            {
                zmq::message_t msg(msgStr.begin(), msgStr.end());
                if(sock->send(msg, zmq::send_flags::none).has_value())
                {
                    msg.rebuild();
                    if(sock->recv(msg))
                    {
                        return std::string(msg.data<const char>(), msg.size());
                    }
                }
            }
            catch(...)
            {}

            reset();
        }

        return std::nullopt;
    }

    /**
     * @brief      Sends a Json Doc to a Request Reply ZMQ Socket
     *
     * @param      doc       The document
     * @param      sock      The sock
     * @param[in]  reset     Socket reset callback function
     * @param[in]  retryCnt  The retry count
     *
     * @return     True if there is a valid response to the message, False otherwise
     */
    inline bool sendReqRep(rapidjson::Document& doc, zmq::socket_t* sock,
        std::function<void(void)> reset, const std::size_t retryCnt = 3) noexcept
    {
        try
        {
            if(auto resp{sendReqRep(jsonToStr(doc), sock, reset, retryCnt)}; resp.has_value())
            {
                return !doc.Parse(resp.value().data(), resp.value().size()).HasParseError();
            }
        } catch(...){}

        return false;
    }
}

#endif
