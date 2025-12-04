#ifndef ENVIRONMENT_MANAGER_PARSER_H_
#define ENVIRONMENT_MANAGER_PARSER_H_

#include <helpers/mainHelper.h>
#include <parser/parserInterface.h>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include "../controller/envMgrController.h"

namespace empower
{
    class envMgrParser: protected parserInterface
    {
    private:
        envMgrController         ctrl;
        messageFactoryConnection    msgIf;

        void parseConfigChangeNotify(rapidjson::Document& jsonDoc);
        void parseLoadPumpConfig(rapidjson::Document& jsonDoc);
        void parseGetFanEnable(rapidjson::Document& jsonDoc);
        void parseSetFanEnable(rapidjson::Document& jsonDoc, bool enabled);

        //----------------------------------------------------------------------
        // DEBUG / MESSAGE VARS
        std::string msgHdr;
        std::string msgData;

    public:
        envMgrParser(zmq::context_t& zmqCtx, helpers::poller& poller, configManagerConnection& config);
    };
}

#endif
