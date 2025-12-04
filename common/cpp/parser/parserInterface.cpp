#include "parserInterface.h"

#include <helpers/zmqConnectionNames.h>
#include <helpers/zmqHelpers.h>
#include <logger/logger.h>
#include <malloc.h>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      ctx     The context
 * @param      poller  The poller
 * @param      conf    The conf
 */
parserInterface::parserInterface(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& conf):
    configIf(conf),
    zmqCtx(ctx),
    poll{poller},
    parserMap{},
    tickersSetup{{0}},
    tickerCallbacks{
        { tickers::ticker100ms, {}},
        { tickers::ticker500ms, {}},
        { tickers::ticker1000ms, {}},
        { tickers::ticker5000ms, {}},
        { tickers::ticker30000ms, {}}
    },
    tickerSocks{},
    tickConnNames{},
    inputSock{},
    inputSockHandle{0},
    outputSock{},
    zmqCleanupSet{false}
{
    for(auto& cb: tickerCallbacks)
    {
        tickerSocks.insert(std::make_pair(cb.first, std::move(
            helpers::getSockConnect(zmqCtx, zmq::socket_type::sub, tickerConnMap.at(cb.first)))));
    }
}

/**
 * @brief      Destroys the object.
 */
parserInterface::~parserInterface()
{
    unregisterSocketHandle(inputSockHandle);
    for(auto h: tickersSetup)
    {
        unregisterSocketHandle(h);
    }
}

/**
 * @brief      Processes a ingressive ZMQ message
 *
 * @param[in]  responseNeeded  True if a response is needed, False otherwise.
 */
void parserInterface::process(const bool responseNeeded) noexcept
{
    using std::string_literals::operator""s;

    if(!inputSock)
    {
        return;
    }

    zmq::message_t msg;

    try
    {
        if(!inputSock->recv(msg).has_value())
        {
            return;
        }
    }
    catch(const std::exception& e)
    {
        logger::warn(__FILE__, __FUNCTION__, "ZMQ Socket RX Error - "s + std::string(e.what()));
        return;
    }

    try
    {
        if(rapidjson::Document workingDoc; !workingDoc.Parse(msg.data<const char>(), msg.size()).HasParseError())
        {
            if(auto msgStr{helpers::jsonGet<std::string>(workingDoc, "messageType")}; msgStr.has_value())
            {
                if(auto mapIt = parserMap.find(msgStr.value()); mapIt != parserMap.end())
                {
                    mapIt->second(workingDoc);
                    if(responseNeeded)
                    {
                        helpers::sendJsonDoc(workingDoc, getResponseSockRef());
                    }
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        logger::warn(__FILE__, __FUNCTION__, "Issue with Processing Replying to Message - "s + std::string(e.what()));
    }
}

/**
 * @brief      Register Message Parser
 *
 * @param[in]  type  Message Type
 * @param[in]  func  Callback Function
 */
void parserInterface::registerParser(const std::string& type, std::function<void(rapidjson::Document&)> func) noexcept
{
    parserMap.emplace(type, func);
}

/**
 * @brief      Removes a Registered Message parser
 *
 * @param[in]  type  The Message Type
 */
void parserInterface::deregisterParser(const std::string& type) noexcept
{
    parserMap.erase(type);
}

/**
 * @brief      Register ZMQ Socket connection
 *
 * @param[in]  inType   ZMQ Socket Ingressive type
 * @param[in]  outType  ZMQ Socket Egressive type
 * @param[in]  in       Input Socket Name
 * @param[in]  out      Output Socket Name
 */
void parserInterface::registerZmqSockets(const zmq::socket_type inType, const zmq::socket_type outType,
    const std::string& in, const std::string& out) noexcept
{
    if(!inputSock)
    {
        setupZmqCleanup();

        inputSock = helpers::getSockConnect(zmqCtx, inType, in);
        outputSock = helpers::getSockConnect(zmqCtx, outType, out);

        inputSockHandle = helpers::getSocketHandle();
        std::get<0>(poll).push_back(inputSockHandle);
        std::get<1>(poll).push_back({static_cast<void*>(*inputSock.get()), 0, ZMQ_POLLIN, 0});
        std::get<2>(poll).push_back([this](){ process(true); });
    }
}

/**
 * @brief      { function_description }
 *
 * @param[in]  type   ZMQ Socket Type
 * @param[in]  bidir  Bi-Directional Socket Name
 */
void parserInterface::registerZmqSockets(const zmq::socket_type type, const std::string& bidir) noexcept
{
    if(!inputSock)
    {
        setupZmqCleanup();

        inputSock = helpers::getSockBind(zmqCtx, type, bidir);

        inputSockHandle = helpers::getSocketHandle();
        std::get<0>(poll).push_back(inputSockHandle);
        std::get<1>(poll).push_back({static_cast<void*>(*inputSock.get()), 0, ZMQ_POLLIN, 0});
        std::get<2>(poll).push_back([this](){ process(true); });
    }
}

/**
 * @brief      Register UNIX Socket
 *
 * @param[in]  sd    Socket Descriptor
 * @param[in]  in    Callback Function
 *
 * @return     Socket Handle
 */
helpers::socket_handle_t parserInterface::registerRawSockets(int sd, std::function<void()> in) noexcept
{
    setupZmqCleanup();

    auto handle{helpers::getSocketHandle()};
    std::get<0>(poll).push_back(handle);
    std::get<1>(poll).push_back({nullptr, sd, ZMQ_POLLIN, 0});
    std::get<2>(poll).push_back(in);
    return handle;
}

/**
 * @brief      Register default ZMQCommand Socket
 */
void parserInterface::registerCmdSock() noexcept
{
    registerZmqSockets(zmq::socket_type::sub, zmq::socket_type::push,
        enums::zmqConnections::getSocket("getWorkerCommandRequest"),
        enums::zmqConnections::getSocket("getWorkerCommandResponse"));
}

/**
 * @brief      Register Configuration Update Notification Socket
 *
 * @param[in]  f     Setup Callback Function
 */
void parserInterface::registerConfigUpdateNotifySock(std::function<void(void)> f) noexcept
{
    if(!inputSock)
    {
        setupZmqCleanup();

        inputSock = helpers::getSockConnect(zmqCtx, zmq::socket_type::sub,
            enums::zmqConnections::getSocket("getConfigChangeNotify"));

        inputSockHandle = helpers::getSocketHandle();
        std::get<0>(poll).push_back(inputSockHandle);
        std::get<1>(poll).push_back({static_cast<void*>(*inputSock.get()), 0, ZMQ_POLLIN, 0});
        std::get<2>(poll).push_back([this](){ process(false); });

        registerConfigUpdate(f);
    }
}

/**
 * @brief      Register Ticker Socket
 *
 * @param[in]  t     Ticker Time Type
 * @param[in]  f     Callback Function for ticker
 */
void parserInterface::registerTicker(const tickers t, std::function<void(void)> f)
{
    using std::string_literals::operator""s;
    const std::underlying_type<tickers>::type ticker_val{static_cast<std::underlying_type<tickers>::type>(t)};

    if(tickersSetup[ticker_val] == 0)
    {
        auto handle{helpers::getSocketHandle()};

        std::get<0>(poll).push_back(handle);
        std::get<1>(poll).push_back({static_cast<void*>(*tickerSocks.at(t).get()), 0, ZMQ_POLLIN, 0});
        std::get<2>(poll).push_back([this, t](){
            for(auto& fp: tickerCallbacks.at(t))
            {
                fp();
            }

            try{
                zmq::message_t msg;
                if(tickerSocks.at(t))
                {
                    if(auto recvRtn{tickerSocks.at(t)->recv(msg)}; !recvRtn.has_value())
                    {
                        logger::warn(__FILE__, __FUNCTION__, "ZMQ Ticker Socket Recv Error");
                    }
                }
            }
            catch(const std::exception& e)
            {
                logger::warn(__FILE__, __FUNCTION__, "ZMQ Ticker Socket Error - "s + std::string(e.what()));
            }
        });

        tickersSetup.at(ticker_val) = handle;
    }

    tickerCallbacks.at(t).push_back(f);
}

/**
 * @brief      Register Configuration Update Callback
 *
 * @param[in]  f     Callback Function
 */
void parserInterface::registerConfigUpdate(std::function<void(void)> f) noexcept
{
    f();

    registerParser("configChangeNotify",
        [this, f](rapidjson::Document& doc){
            if(configIf.updateNeeded(doc)){
                f();
            }
        }
    );
}

/**
 * @brief      Register Configuration Update Callback
 *
 * @param[in]  keys  The keys
 * @param[in]  f     Callback Function
 */
void parserInterface::registerConfigSet(const keySet_t& keys, const std::function<void(void)>& f) noexcept
{
    f();
    configIf.registerConfigFunction(keys, f);
}

/**
 * @brief      Unregister a Socket Handle
 *
 * @param      handle  The handle
 */
void parserInterface::unregisterSocketHandle(helpers::socket_handle_t& handle) noexcept
{
    if(handle > 0)
    {
        auto pollHandleRef{std::get<0>(poll)};
        if(auto it{std::find(pollHandleRef.begin(), pollHandleRef.end(), handle)}; it != pollHandleRef.end())
        {
            auto idx{std::distance(pollHandleRef.begin(), it)};

            std::get<0>(poll).erase(std::next(std::get<0>(poll).cbegin(), idx));
            std::get<1>(poll).erase(std::next(std::get<1>(poll).cbegin(), idx));
            std::get<2>(poll).erase(std::next(std::get<2>(poll).cbegin(), idx));
            handle = 0;
        }
    }
}

[[nodiscard]]
zmq::socket_t* parserInterface::getResponseSockRef() noexcept
{
    auto resp{((outputSock)?(outputSock.get()):(inputSock.get()))};
    return resp;
}

/**
 * @brief      Setup ZMQ Cleanup for periodic Memory Management
 */
void parserInterface::setupZmqCleanup() noexcept
{
    if(!zmqCleanupSet)
    {
        registerTicker(tickers::ticker30000ms, [&](){ callGetSockOptsZmqEvents(); });
        zmqCleanupSet = true;
    }
}

/**
 * @brief      Periodic Memory Management Cleanup
 */
void parserInterface::callGetSockOptsZmqEvents() const noexcept
{
    if(inputSock)
    {
        inputSock->getsockopt<int>(ZMQ_EVENTS);
    }

    for(const auto& sock: tickerSocks)
    {
        if(sock.second)
        {
            sock.second->getsockopt<int>(ZMQ_EVENTS);
        }
    }

    malloc_trim(1);
}
