#include "switchManagerParser.h"

#include <helpers/jsonUpdateHelpers.h>
#include <helpers/zmqConnectionNames.h>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      ctx     The context
 * @param      poller  The poller
 * @param      config  The configuration
 */
switchManagerParser::switchManagerParser(zmq::context_t& ctx, helpers::poller& poller,
        configManagerConnection& config):
    parserInterface{ctx, poller, config},
    ctrl{ctx, config},
    msgIf{ctx}
{
    registerCmdSock();
    registerConfigUpdate([this](){ ctrl.initializeData(); });

    registerParser("getSwitchBand", [this](rapidjson::Document& doc){ getBand(doc); });
    registerParser("setSwitchBand", [this](rapidjson::Document& doc){ setBand(doc); });

    registerParser("getSwitchBandSourceSelect", [this](rapidjson::Document& doc){ getBandSourceSelect(doc); });
    registerParser("setSwitchBandSourceSelectSw",
        [this](rapidjson::Document& doc){ setBandSourceSelect(doc, source_select_t::SW); });
    registerParser("setSwitchBandSourceSelectHw",
        [this](rapidjson::Document& doc){ setBandSourceSelect(doc, source_select_t::HW); });
    registerParser("setTrSwitchTransmit", [this](rapidjson::Document& doc){ setTrSwitchTransmit(doc); });
    registerParser("setTrSwitchReceive", [this](rapidjson::Document& doc){ setTrSwitchReceive(doc); });
    registerParser("getTrSwitchState", [this](rapidjson::Document& doc){ getTrSwitchState(doc); });
    registerParser("setOutputLoadSwitchAntenna", [this](rapidjson::Document& doc){ setDummySwitchTransmit(doc); });
    registerParser("setOutputLoadSwitchDummy", [this](rapidjson::Document& doc){ setDummySwitchLoad(doc); });
    registerParser("getOutputLoadSwitchState", [this](rapidjson::Document& doc){ getDummySwitchState(doc); });

    registerTicker(tickers::ticker5000ms, [this](){ ctrl.pollSwitchState(); });
}

/**
 * @brief      Gets the band.
 *
 * @param      jsonDoc  The json document
 */
void switchManagerParser::getBand(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
        {
            if(auto resp{ctrl.getBand()}; resp.has_value())
            {
                helpers::types::bands_t band;
                std::size_t off;

                std::tie(band, off) = resp.value();

                if(helpers::jsonSet(jsonDoc, helpers::types::bandToStr(band), "resultMessage", "band") &&
                    helpers::jsonSet(jsonDoc, off, "resultMessage", "offset"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the band.
 *
 * @param      jsonDoc  The json document
 */
void switchManagerParser::setBand(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        auto bandStr{helpers::jsonGet<std::string>(jsonDoc, "band")};
        auto off{helpers::jsonGet<std::size_t>(jsonDoc, "offset")};

        if(bandStr.has_value() && off.has_value())
        {
            if(ctrl.setBand(helpers::types::strToBand(bandStr.value()), off.value()))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the source select.
 *
 * @param      jsonDoc  The json document
 */
void switchManagerParser::getBandSourceSelect(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getBandSourceSelect()}; resp.has_value())
        {
            if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
            {
                if(helpers::jsonSet(jsonDoc, helpers::types::srcSelToStr(resp.value()), "resultMessage"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the source select.
 *
 * @param      jsonDoc  The json document
 * @param[in]  src      The new value
 */
void switchManagerParser::setBandSourceSelect(rapidjson::Document& jsonDoc, const source_select_t src)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setBandSourceSelect(src))
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the tr switch transmit.
 *
 * @param      jsonDoc  The json document
 */
void switchManagerParser::setTrSwitchTransmit(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setTrSwitchTransmit())
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the tr switch receive.
 *
 * @param      jsonDoc  The json document
 */
void switchManagerParser::setTrSwitchReceive(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setTrSwitchReceive())
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the tr switch state.
 *
 * @param      jsonDoc  The json document
 */
void switchManagerParser::getTrSwitchState(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getTrSwitchState()}; resp.has_value())
        {
            if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
            {
                if(helpers::jsonSet(jsonDoc, resp.value(), "resultMessage"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the dummy switch transmit.
 *
 * @param      jsonDoc  The json document
 */
void switchManagerParser::setDummySwitchTransmit(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setDummySwitchTransmit())
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the dummy switch load.
 *
 * @param      jsonDoc  The json document
 */
void switchManagerParser::setDummySwitchLoad(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setDummySwitchLoad())
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the dummy switch state.
 *
 * @param      jsonDoc  The json document
 */
void switchManagerParser::getDummySwitchState(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getDummySwitchState()}; resp.has_value())
        {
            if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
            {
                if(helpers::jsonSet(jsonDoc, resp.value(), "resultMessage"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}
