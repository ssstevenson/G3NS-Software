#ifndef PARSER_INTERFACE_H_
#define PARSER_INTERFACE_H_

#include <array>
#include <configManagerConnection/configManagerConnection.h>
#include <helpers/mainHelper.h>
#include <helpers/zmqConnectionNames.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace empower
{
    /**
     * @brief      This class describes a parser interface.
     */
    class parserInterface
    {
    public:
        using keySet_t = configManagerConnection::keySet_t;
        using parserMapType = std::unordered_map<std::string, std::function<void(rapidjson::Document&)> >;
        enum class tickers: std::uint32_t
        {
            ticker100ms = 0,
            ticker500ms,
            ticker1000ms,
            ticker5000ms,
            ticker30000ms,
            MAX
        };

    protected:
        configManagerConnection& configIf;

    private:
        zmq::context_t& zmqCtx;
        helpers::poller& poll;
        parserMapType parserMap;
        std::array<helpers::socket_handle_t,
            static_cast<std::underlying_type<tickers>::type>(tickers::MAX)> tickersSetup;
        std::unordered_map<tickers, std::vector<std::function<void(void)>>> tickerCallbacks;
        std::unordered_map<tickers, std::unique_ptr<zmq::socket_t> > tickerSocks;
        std::unordered_map<tickers, std::string> tickConnNames;
        std::unique_ptr<zmq::socket_t> inputSock;
        helpers::socket_handle_t inputSockHandle;
        std::unique_ptr<zmq::socket_t> outputSock;
        const std::unordered_map<tickers, std::string> tickerConnMap{
            {tickers::ticker100ms, enums::zmqConnections::getSocket("getTicker100ms")},
            {tickers::ticker500ms, enums::zmqConnections::getSocket("getTicker500ms")},
            {tickers::ticker1000ms, enums::zmqConnections::getSocket("getTicker1000ms")},
            {tickers::ticker5000ms, enums::zmqConnections::getSocket("getTicker5000ms")},
            {tickers::ticker30000ms, enums::zmqConnections::getSocket("getTicker30000ms")}
        };
        bool zmqCleanupSet;

    public:
        parserInterface(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& conf);
        virtual ~parserInterface();

        void process(const bool responseNeeded) noexcept;
        void registerParser(const std::string& type, std::function<void(rapidjson::Document&)> func) noexcept;
        void deregisterParser(const std::string& type) noexcept;
        void registerZmqSockets(const zmq::socket_type inType, const zmq::socket_type outType,
            const std::string& in, const std::string& out) noexcept;
        void registerZmqSockets(const zmq::socket_type type, const std::string& bidir) noexcept;
        [[nodiscard]] helpers::socket_handle_t registerRawSockets(int sd, std::function<void()> in) noexcept;
        void registerCmdSock() noexcept;
        void registerConfigUpdateNotifySock(std::function<void(void)> f) noexcept;
        void registerTicker(const tickers t, std::function<void(void)> f);
        void registerConfigUpdate(std::function<void(void)> f) noexcept;
        void registerConfigSet(const keySet_t& keys, const std::function<void(void)>& f) noexcept;
        void unregisterSocketHandle(helpers::socket_handle_t& handle) noexcept;
        [[nodiscard]] zmq::socket_t* getResponseSockRef() noexcept;

    private:
        void setupZmqCleanup() noexcept;
        void callGetSockOptsZmqEvents() const noexcept;
    };
}

#endif
