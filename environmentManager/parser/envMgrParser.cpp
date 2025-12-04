/*!
* \file
* \brief Environmental Manager Parser Support Class
*
-------------------------------------------------------------------------------

 Name        : envMgrParser.cpp

 Author      : Steve Stevenson

 Version     : v 00.90.00 [ Initial Dev Build ]

 Description : Environmental Manager Parser Support Class Definitions

-------------------------------------------------------------------------------


Copyright   : (C) Copyright 2018 - 2020 Empower RF Systems

-------------------------------------------------------------------------------
*
*
*
*
*/

#include "envMgrParser.h"

#include <helpers/jsonUpdateHelpers.h>
#include <helpers/zmqConnectionNames.h>
#include <iostream>

using namespace empower;

envMgrParser::envMgrParser(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& config):
    parserInterface(ctx, poller, config),
    ctrl{ctx, config},
    msgIf{ctx},
    msgHdr{},
    msgData{}
{
    // Inbound Sockets
    //
    registerCmdSock();

    registerTicker(tickers::ticker100ms,   [this](){ ctrl.Ticker100ms_Tasks(); });
    registerTicker(tickers::ticker500ms,   [this](){ ctrl.Ticker500ms_Tasks(); });
    registerTicker(tickers::ticker1000ms,  [this](){ ctrl.Ticker1000ms_Tasks(); });
    registerTicker(tickers::ticker5000ms,  [this](){ ctrl.Ticker5000ms_Tasks(); });
//    registerTicker(tickers::ticker30000ms, [this](){ ctrl.Ticker30000ms_Tasks(); });

    registerParser("getFanEnable", [this](rapidjson::Document& doc){ parseGetFanEnable(doc); });
    registerParser("configChangeNotify", [this](rapidjson::Document& doc){ parseConfigChangeNotify(doc); });
    registerParser("loadCduPumpConfig",  [this](rapidjson::Document& doc){ parseLoadPumpConfig(doc); });

}

// configChangeNotify

void envMgrParser::parseConfigChangeNotify(rapidjson::Document& jsonDoc)
{
    /// Detect Name starts with 'ENV_'
    std::string reply = helpers::jsonToStr(jsonDoc);
    size_t result = reply.find("ENV_");
//    std::cout << "\n\nReceived ConfigChangeNotify Message: " << reply << "\n\n" << std::endl;

    if(result != std::string::npos)
    {
        /// EXTRACT : parameter "name":"ENV_----"
        size_t parmPosStart = reply.find("\"ENV_");
        size_t parmPosEnd = reply.find(",", parmPosStart+3);
        std::string  parmName = reply.substr(parmPosStart+1, ((parmPosEnd-2) - parmPosStart));

        logger::debug(__FILE__, __FUNCTION__, parmName);

        ctrl.processConfigChangeUpdate(parmName);
        return;
    }

    logger::warn(__FILE__, __FUNCTION__, "Failed to get message from Configuration Manager");

}



void envMgrParser::parseLoadPumpConfig(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        std::string request = helpers::jsonToStr(jsonDoc);
        std::string requestType = helpers::jsonGet<std::string> (jsonDoc, "messageType").value_or("");
        jsonDoc.Swap(jsonResp.value());
        std::cout << "\n\nReceived LoadPumpConfig MessageType: " << requestType << "\n\n" << std::endl;

        if(ctrl.loadPumpConfig())
        {
            return;
        }
    }

    // TODO -- MB -- Generate log using MessageFactory::getLogPostRequest message
}

void envMgrParser::parseGetFanEnable(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(bool fanEnable{false}; ctrl.getFanEnableState(fanEnable))
        {
            if (empower::helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
            {
                if (empower::helpers::jsonSet<std::string>(jsonDoc, (fanEnable ? "on" : "off"), "resultMessage"))
                {
                    return;
                }
            }
        }
    }

    logger::warn(__FILE__, __FUNCTION__, "Failed to get message from FPGA Hardware Interface");
}

void envMgrParser::parseSetFanEnable(rapidjson::Document& jsonDoc, bool fanEnable)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setFanEnableState(fanEnable))
        {
//            updateSingleStatus(ctrl.fanEnableStatus, fanEnable);
            return;
        }
    }

    // TODO -- MB -- Generate log using MessageFactory::getLogPostRequest message
}

