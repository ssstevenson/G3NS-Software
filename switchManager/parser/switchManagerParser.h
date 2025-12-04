#ifndef SWITCH_MANAGER_PARSER_H_
#define SWITCH_MANAGER_PARSER_H_

#include <controller/switchManagerController.h>
#include <helpers/mainHelper.h>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <parser/parserInterface.h>

namespace empower
{
    /**
     * @brief      This class describes a switch manager parser.
     */
    class switchManagerParser: protected parserInterface
    {
    public:
        using source_select_t = switchManagerController::source_select_t;

    private:
        switchManagerController ctrl;
        messageFactoryConnection msgIf;

    public:
        switchManagerParser(zmq::context_t& zmqCtx, helpers::poller& poller, configManagerConnection& config);

    private:
        void getBand(rapidjson::Document& jsonDoc);
        void setBand(rapidjson::Document& jsonDoc);
        void getBandSourceSelect(rapidjson::Document& jsonDoc);
        void setBandSourceSelect(rapidjson::Document& jsonDoc, const source_select_t src);
        void setTrSwitchTransmit(rapidjson::Document& jsonDoc);
        void setTrSwitchReceive(rapidjson::Document& jsonDoc);
        void getTrSwitchState(rapidjson::Document& jsonDoc);
        void setDummySwitchTransmit(rapidjson::Document& jsonDoc);
        void setDummySwitchLoad(rapidjson::Document& jsonDoc);
        void getDummySwitchState(rapidjson::Document& jsonDoc);
    };
}

#endif
