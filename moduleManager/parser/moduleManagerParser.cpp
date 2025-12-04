#include "moduleManagerParser.h"

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      ctx     The context
 * @param      poller  The poller
 * @param      config  The configuration
 */
moduleManagerParser::moduleManagerParser(zmq::context_t& ctx, helpers::poller& poller,
        configManagerConnection& config):
    parserInterface{ctx, poller, config},
    ctrl{ctx, config}
{
    registerCmdSock();
    registerParser("moduleRequestSoftReset", [this](rapidjson::Document& doc){
        auto modules{helpers::jsonGet<std::string>(doc, "module")};
        ctrl.reset(modules.value_or("*"));
    });
    registerParser("moduleRequestFaultClear", [this](rapidjson::Document& doc){
        auto modules{helpers::jsonGet<std::string>(doc, "module")};
        ctrl.faultClear(modules.value_or("*"));
    });
    registerConfigUpdate([this](){ ctrl.initializeData(); });
    registerTicker(tickers::ticker500ms, [this](){ ctrl.pollPmod(); });
    registerTicker(tickers::ticker1000ms, [this](){ ctrl.updateSideData(); });
}
