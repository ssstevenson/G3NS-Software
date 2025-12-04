#include "rfManagerParser.h"

#include <chrono>
#include <cmath>
#include <helpers/types.h>
#include <helpers/zmqConnectionNames.h>
#include <instrumentation/siggen.h>
#include <numeric>
#include <thread>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      ctx     The context
 * @param      poller  The poller
 * @param      conf    The conf
 */
rfManagerParser::rfManagerParser(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& conf):
    parserInterface{ctx, poller, conf},
    ctrl{ctx, conf},
    msgIf{ctx}
{
    registerCmdSock();
    registerConfigUpdate([this](){ ctrl.initializeData(); });
    registerConfigSet(
        { "RF_PHASOR_INPHASE", "RF_PHASOR_QUADRATURE", "RF_PHASE_DWELL_NS", "RF_PHASE_TARGET" },
        [this](){ ctrl.initializeDsaStepTables(); }
    );
    registerConfigSet(
        {
            "SYS_CTRL_XBAR_GPO_1", "SYS_CTRL_XBAR_GPO_2", "SYS_CTRL_XBAR_GPO_3", "SYS_CTRL_XBAR_GPO_4",
            "SYS_CTRL_XBAR_GPO_5", "SYS_CTRL_XBAR_GPO_6", "SYS_CTRL_XBAR_GPO_7", "SYS_CTRL_XBAR_GPO_8",
            "SYS_CTRL_XBAR_GPO_9", "SYS_CTRL_XBAR_GPO_10", "SYS_CTRL_XBAR_GPO_11", "SYS_CTRL_XBAR_GPI_1",
            "SYS_CTRL_XBAR_GPI_2", "SYS_CTRL_XBAR_GPI_3", "SYS_CTRL_XBAR_GPI_4", "SYS_CTRL_XBAR_GPI_5",
            "SYS_CTRL_XBAR_GPI_6", "SYS_CTRL_XBAR_GPI_7", "SYS_CTRL_XBAR_GPI_8", "SYS_CTRL_XBAR_GPI_9",
            "SYS_CTRL_XBAR_GPI_10", "SYS_CTRL_XBAR_GPI_11", "SYS_CTRL_XBAR_GPI_12", "SYS_CTRL_XBAR_GPI_13"
        },
        [this](){ ctrl.initializeGpioCrossbar(); }
    );
    registerConfigSet(
        {
            "RF_MOD_PWR_CONV_TABLE_SIZE", "FHI_MOD_PWR_CONV_BASE_HEX_ADDRESS", "RF_CAL_CURVE_INPUT_RMS",
            "RF_CAL_CURVE_INPUT_PEAK", "RF_CAL_CURVE_INPUT_ENVELOPE", "RF_CAL_CURVE_FORWARD_RMS",
            "RF_CAL_CURVE_FORWARD_PEAK", "RF_CAL_CURVE_FORWARD_ENVELOPE", "RF_CAL_CURVE_REVERSE_RMS",
            "RF_CAL_CURVE_REVERSE_PEAK", "RF_CAL_CURVE_REVERSE_ENVELOPE", "RF_CAL_CURVE_UNBALANCED_RMS",
            "RF_CAL_CURVE_UNBALANCED_PEAK", "RF_CAL_CURVE_UNBALANCED_ENVELOPE"
        },
        [this](){ ctrl.initializePowerConversionTables(); }
    );
    registerConfigSet(
        {
            "RF_CF_FWD", "RF_CF_REV", "RF_CF_INP", "RF_CF_UNB", "STARTUP_RF_MODE", "RF_DEFAULT_BLANKING_POL",
            "RF_DEFAULT_SHUTDOWN_POL", "RF_INP_PRES_THOLD", "RF_INP_PRES_HYST_INC", "RF_INP_PRES_HYST_DEC",
            "RF_INP_PRES_HYST_THOLD", "RF_FWD_PRES_THOLD", "RF_FWD_PRES_HYST_INC", "RF_FWD_PRES_HYST_DEC",
            "RF_FWD_PRES_HYST_THOLD", "RF_INPUT_SWITCH_SEQUENCING_MUTE_TIME", "RF_DEFAULT_BLANKING_OUT_POL"
        },
        [this](){ ctrl.initializeFPGA(); }
    );
    registerConfigSet(
        {
            "RF_PID_KP_LG_P", "RF_PID_KI_LG_P", "RF_PID_KD_LG_P", "RF_PID_KP_LG_N", "RF_PID_KI_LG_N", "RF_PID_KD_LG_N",
            "RF_PID_KP_SM_P", "RF_PID_KI_SM_P", "RF_PID_KD_SM_P", "RF_PID_KP_SM_N", "RF_PID_KI_SM_N", "RF_PID_KD_SM_N",
            "RF_ALC_RANGE", "RF_DEFAULT_DETECTOR", "RF_PID_CLK_PERIOD", "RF_PID_ALC_OVERSHOOT_DROP",
            "RF_PID_ALC_OVERSHOOT_INTG_DROP", "RF_PID_ALC_OVERSHOOT_THRESHOLD", "RF_PID_ALC_OVERSHOOT_DETECTOR",
            "RF_PID_DAC_LOW_LMT", "RF_PID_DAC_HIGH_LMT", "RF_PID_SUM_FAULT_DELAY", "RF_PID_RF_MODE", "RF_PID_OSCK", 
            "RF_ALC_HW_MODE", "RF_PID_SUSPEND_LEVEL", "RF_PID_BLANKING_START_DLY", "RF_PID_BLANKING_SUSPEND_DLY", 
            "RF_PID_BLANKING_TIMEOUT_DLY", "RF_PID_INP_SUSPEND_DLY", "RF_PID_INP_TIMEOUT_DLY", "RF_PID_INP_CHAN_MARGIN_HIGH", 
            "RF_PID_INP_CHAN_MARGIN_LOW"
        },
        [this](){ ctrl.initializePowerRegulation(); }
    );
    registerConfigSet(
        {
            "RF_RMS_TMR_EN", "RF_RMS_TMR_PWR_THRESHOLD", "RF_RMS_TMR_CNT_TOP", "RF_RMS_TMR_CNT_THRESHOLD",
            "RF_INPUT_RMS_FILTER_ENABLE", "RF_INPUT_RMS_FILTER_FORCE_CW", "RF_INPUT_RMS_FILTER_FORCE_PULSE",
            "RF_INPUT_RMS_FILTER_OFFSET", "RF_FORWARD_RMS_FILTER_ENABLE", "RF_FORWARD_RMS_FILTER_FORCE_CW",
            "RF_FORWARD_RMS_FILTER_FORCE_PULSE", "RF_FORWARD_RMS_FILTER_OFFSET", "RF_REVERSE_RMS_FILTER_ENABLE",
            "RF_REVERSE_RMS_FILTER_FORCE_CW", "RF_REVERSE_RMS_FILTER_FORCE_PULSE", "RF_REVERSE_RMS_FILTER_OFFSET",
            "RF_UNBALANCED_RMS_FILTER_ENABLE", "RF_UNBALANCED_RMS_FILTER_FORCE_CW",
            "RF_UNBALANCED_RMS_FILTER_FORCE_PULSE", "RF_UNBALANCED_RMS_FILTER_OFFSET"
        },
        [this](){ ctrl.initializePowerConversionFiltering(); }
    );

    registerConfigSet(
        {
            "RF_DAC_DROOP_CORRECTION_LINE1_ENABLE", "RF_DAC_DROOP_CORRECTION_LINE1_DROP_COUNT", 
            "RF_DAC_DROOP_CORRECTION_LINE1_CLOCK_COUNT", "RF_DAC_DROOP_CORRECTION_LINE1_RISE_COUNT",
            "RF_DAC_DROOP_CORRECTION_LINE2_ENABLE", "RF_DAC_DROOP_CORRECTION_LINE2_DROP_COUNT",
            "RF_DAC_DROOP_CORRECTION_LINE2_CLOCK_COUNT", "RF_DAC_DROOP_CORRECTION_LINE2_RISE_COUNT"
        },
        [this](){ ctrl.initializeDroopCorrection(); }
    );

    registerParser("internalApplyProfile", [this](auto& doc){ setProfile(doc); });

    registerParser("getRfState", [this](auto& doc){ getRfState(doc); });
    registerParser("setRfStateOn", [this](auto& doc){ setRfState(doc, online_state_t::ON); });
    registerParser("setRfStateOff", [this](auto& doc){ setRfState(doc, online_state_t::HOT); });
    registerParser("setRfStateHot", [this](auto& doc){ setRfState(doc, online_state_t::HOT); });
    registerParser("setRfStateCold", [this](auto& doc){ setRfState(doc, online_state_t::COLD); });

    registerParser("getSafeStateOnOff", [this](auto& doc){ getSafeState(doc); });
    registerParser("setSafeStateOn", [this](auto& doc){ setSafeState(doc, safe_state_t::SAFE); });
    registerParser("setSafeStateOff", [this](auto& doc){ setSafeState(doc, safe_state_t::RUNNING); });

    registerParser("getGainMode", [this](auto& doc){ getGainMode(doc); });
    registerParser("setGainModeAlc", [this](auto& doc){ setGainMode(doc, gain_mode_t::ALC); });
    registerParser("setGainModeAgc", [this](auto& doc){ setGainMode(doc, gain_mode_t::AGC); });
    registerParser("setGainModeMgc", [this](auto& doc){ setGainMode(doc, gain_mode_t::MGC); });

    registerParser("getOperatingMode", [this](auto& doc){ getOperatingMode(doc); });

    registerParser("getMgcLevelPercentage", [this](auto& doc){ getMgcSetpointPercentage(doc); });
    registerParser("setMgcLevelPercentage", [this](auto& doc){ setMgcSetpointPercentage(doc); });
    registerParser("getMgcLevelGain", [this](auto& doc){ getMgcSetpointGain(doc); });
    registerParser("setMgcLevelGain", [this](auto& doc){ setMgcSetpointGain(doc); });
    registerParser("getMgcLevelVva", [this](auto& doc){ getMgcSetpointVva(doc); });
    registerParser("setMgcLevelVva", [this](auto& doc){ setMgcSetpointVva(doc); });
    registerParser("getAlcLevel", [this](auto& doc){ getAlcSetpoint(doc); });
    registerParser("setAlcLevel", [this](auto& doc){ setAlcSetpoint(doc); });
    registerParser("getAgcLevel", [this](auto& doc){ getAgcSetpoint(doc); });
    registerParser("setAgcLevel", [this](auto& doc){ setAgcSetpoint(doc); });

    registerParser("getInputSelectState", [this](auto& doc){ getInputSelect(doc); });
    registerParser("setInputSelectBit", [this](auto& doc){ setInputSelect(doc, input_select_t::BIT); });
    registerParser("setInputSelectInput", [this](auto& doc){ setInputSelect(doc, input_select_t::INPUT); });
    registerParser("getInputSwitchState", [this](auto& doc){ getInputSwitch(doc); });

    registerParser("getBlankingInput", [this](auto& doc){ getBlankingInput(doc); });
    registerParser("getBlankingState", [this](auto& doc){ getBlankingState(doc); });
    registerParser("getBlankingPolarity", [this](auto& doc){ getBlankingPolarity(doc); });
    registerParser("setBlankingPolarityActiveHigh", [this](auto& doc){
        setBlankingPolarity(doc, polarity_t::ACTIVE_HIGH); });
    registerParser("setBlankingPolarityActiveLow", [this](auto& doc){
        setBlankingPolarity(doc, polarity_t::ACTIVE_LOW); });
    registerParser("getShutdownInput", [this](auto& doc){ getShutdownInput(doc); });
    registerParser("getShutdownState", [this](auto& doc){ getShutdownState(doc); });
    registerParser("getShutdownPolarity", [this](auto& doc){ getShutdownPolarity(doc); });
    registerParser("setShutdownPolarityActiveHigh", [this](auto& doc){
        setShutdownPolarity(doc, polarity_t::ACTIVE_HIGH); });
    registerParser("setShutdownPolarityActiveLow", [this](auto& doc){
        setShutdownPolarity(doc, polarity_t::ACTIVE_LOW); });

    registerParser("setDroopCorrectionLine1Enable", [this](auto& doc){ 
        setDroopCorrectionEnable(doc, droopLine_t::LINE_1); });
    registerParser("setDroopCorrectionLine2Enable", [this](auto& doc){ 
        setDroopCorrectionEnable(doc, droopLine_t::LINE_2); });

    registerParser("setBitEnableSelect", [this](auto& doc){ setBitEnableSelect(doc); });
    registerParser("setBitEnableParameters", [this](auto& doc){ setBitEnableParameters(doc); });
    registerParser("setBitEnableGuardTimeNs", [this](auto& doc){ setBitEnableGuardTimeNs(doc); });
    registerParser("setBitRfModulationParameters", [this](auto& doc){ setBitRfModulationParameters(doc); });

    registerParser("setRfDroopCorrectionLine1", [this](auto& doc){ setRfDroopCorrection(doc, droopLine_t::LINE_1); });
    registerParser("setRfDroopCorrectionLine2", [this](auto& doc){ setRfDroopCorrection(doc, droopLine_t::LINE_2); });

    registerParser("getPhase", [this](auto& doc){ getPhase(doc); });
    registerParser("setPhase", [this](auto& doc){ setPhase(doc); });

    registerParser("getPowerLevels", [this](auto& doc){ getPowerLevels(doc); });

    registerParser("calibrateRequest", [this](auto& doc){ calRunStep(doc); });
    registerParser("calibrateRequestReset", [this](auto& doc){ calReset(doc); });
    registerParser("calibrateRequestId", [this](auto& doc){ calGetId(doc); });
    registerParser("calibrateRequestDisableSigGen", [this](auto& doc){ calDisableSigGen(doc); });

    registerParser("getManualInputSwitch", [this](auto& doc){ getManualInputSwitch(doc); });
    registerParser("setManualInputSwitchInput", [this](auto& doc){
        setManualInputSwitch(doc, manual_input_switch_t::INPUT); });
    registerParser("setManualInputSwitchLoad", [this](auto& doc){
        setManualInputSwitch(doc, manual_input_switch_t::LOAD); });
    registerParser("setManualInputSwitchNone", [this](auto& doc){
        setManualInputSwitch(doc, manual_input_switch_t::NONE); });

    registerTicker(tickers::ticker500ms, [this](){ ctrl.pollPowerLevels(); });
    registerTicker(tickers::ticker1000ms, [this](){ ctrl.pollFpgaData(); });
}

/**
 * @brief      Sets the profile.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setProfile(rapidjson::Document& jsonDoc)
{
    using namespace helpers;

    logger::verbose(__FILE__, __FUNCTION__, "Attempting to process Profile Setup request");

    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());
        auto modulationMode{jsonGet<std::string>(jsonDoc, "request", "commandParameters", "rfModulationMode")}; // cw, pulse, etc.
        auto detectorMode{jsonGet<std::string>(jsonDoc, "request", "commandParameters", "rfDetectorMode")}; // peakfast, peakslow, peak, rms
        auto peakTauMs{jsonGet<types::decay_rate_t::rep>(jsonDoc, "request", "commandParameters", "peakTauMs")};
        auto operatingMode{jsonGet<std::string>(jsonDoc, "request", "commandParameters", "operatingMode")};
        auto powerMode{jsonGet<std::string>(jsonDoc, "request", "commandParameters", "rfPowerMode")};
        auto paprLevel{jsonGet<types::papr_t>(jsonDoc, "request", "commandParameters", "rfPeakToAverageRatio")};
        auto carriers{jsonGet<uint32_t>(jsonDoc, "request", "commandParameters", "rfNumberOfCarriers")};

        if(modulationMode.has_value() && detectorMode.has_value() && operatingMode.has_value() && powerMode.has_value())
        {
            bool settingGood{true};
            const auto opMode{types::strToOnlineState(operatingMode.value())};

            std::string upperPowerModeStr;
            std::transform(std::begin(powerMode.value()), std::end(powerMode.value()),
                std::back_inserter(upperPowerModeStr), [](const auto& ch){ return std::toupper(ch); });
            auto gainMode{types::strToGainMode(upperPowerModeStr)};
            auto useDetector{types::strToDetector(detectorMode.value())};

            if((types::detector_t::PEAK == useDetector) && peakTauMs.has_value())
            {
                settingGood = settingGood && ctrl.setTauDecayRate(types::decay_rate_t{peakTauMs.value()});
            }

            settingGood = settingGood && ctrl.setRegulationDet(useDetector);
            settingGood = settingGood && ctrl.setGainMode(gainMode);

            switch(gainMode)
            {
            case gain_mode_t::MGC:
                if(auto setpoint{jsonGet<types::percent_t>(jsonDoc, "request", "commandParameters",
                    "rfDefaultOutputLevel")}; setpoint.has_value())
                {
                    settingGood = settingGood && ctrl.setMgcSetpointPercentage(setpoint.value());
                }
                else
                {
                    settingGood = false;
                }
                break;
            case gain_mode_t::ALC:
                if(auto setpoint{jsonGet<types::dbm_t>(jsonDoc, "request", "commandParameters",
                    "rfDefaultOutputLevel")}; setpoint.has_value())
                {
                    settingGood = settingGood && ctrl.setAlcSetpoint(setpoint.value());
                }
                else
                {
                    settingGood = false;
                }
                break;
            case gain_mode_t::AGC:
                if(auto setpoint{jsonGet<types::dbm_t>(jsonDoc, "request", "commandParameters",
                    "rfDefaultOutputLevel")}; setpoint.has_value())
                {
                    settingGood = settingGood && ctrl.setAgcSetpoint(setpoint.value());
                }
                else
                {
                    settingGood = false;
                }
                break;
            default:
                settingGood = false;
                break;
            }

            std::string& modMode{modulationMode.value()};
            if(modMode.compare("dm") == 0)
            {
                settingGood = settingGood && ctrl.setPaprBackoff(std::clamp(paprLevel.value_or(13.0), 0.0, 13.0));
            }
            else if(modMode.compare("agwn") == 0)
            {
                settingGood = settingGood && ctrl.setPaprBackoff(9.0);
            }
            else if(modMode.compare("mcp") == 0)
            {
                settingGood = settingGood && ctrl.setPaprBackoff(3.0);
            }
            else if(modMode.compare("b") == 0)
            {
                settingGood = settingGood && ctrl.setPaprBackoff(std::clamp(paprLevel.value_or(9.0), 3.0, 9.0));
            }
            else if(modMode.compare("mci") == 0)
            {
                uint32_t carrierCount{carriers.value_or(16)};
                dbm_t backoff{7.0};

                if(carrierCount == 2)
                {
                    backoff = 3.0;
                }
                else if((carrierCount > 2) && (carrierCount <= 4))
                {
                    backoff = 5.0;
                }
                else if((carrierCount > 4) && (carrierCount <= 8))
                {
                    backoff = 6.0;
                }

                settingGood = settingGood && ctrl.setPaprBackoff(backoff);
            }

            if(settingGood)
            {
                if(ctrl.setRfState(opMode))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to setup Profile"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the rf state.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getRfState(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getRfState()}; resp.has_value())
        {
            if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
            {
                if(helpers::jsonSet(jsonDoc, helpers::types::onlineStateToStr(resp.value()), "resultPayload"))
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
 * @brief      Sets the rf state.
 *
 * @param      jsonDoc  The json document
 * @param[in]  online   The online
 */
void rfManagerParser::setRfState(rapidjson::Document& jsonDoc, const online_state_t online)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setRfState(online))
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the safe state.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getSafeState(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getSafeState()}; resp.has_value())
        {
            if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
            {
                if(helpers::jsonSet(jsonDoc, helpers::types::safeStateToStr(resp.value()), "resultPayload"))
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
 * @brief      Sets the safe state.
 *
 * @param      jsonDoc  The json document
 * @param[in]  safe     The safe
 */
void rfManagerParser::setSafeState(rapidjson::Document& jsonDoc, const safe_state_t safe)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setSafeState(safe))
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the gain mode.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getGainMode(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getGainMode()}; resp.has_value())
        {
            if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
            {
                if(helpers::jsonSet(jsonDoc, helpers::types::gainModeToStr(resp.value()), "resultPayload"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to Get the Amplifier Gain Mode"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the gain mode.
 *
 * @param      jsonDoc  The json document
 * @param[in]  mode     The mode
 */
void rfManagerParser::setGainMode(rapidjson::Document& jsonDoc, const gain_mode_t mode)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setGainMode(mode))
        {
            return;
        }
    }

    const std::string errMsg{"Failed to Set the Amplifier Gain Mode"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Enables or disables droop correction by updating configuration values
 *
 * @param      jsonDoc  The json document
 * @param[in]  line     The droop correction line
 */
void rfManagerParser::setDroopCorrectionEnable(rapidjson::Document& jsonDoc, const droopLine_t line)
{
    using namespace helpers;

    std::string errMsg{"Failed to set Droop Correction Enable state"};

    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());
        
        if(auto enableValue{jsonGet<bool>(jsonDoc, "request", "commandValue")}; enableValue.has_value())
        {
            // Directly control the hardware via controller
            bool success = false;
            std::string lineName;
            
            if(line == droopLine_t::LINE_1)
            {
                lineName = "Line 1";
                success = ctrl.setDroopCorrectionLine1Enable(enableValue.value());
            }
            else if(line == droopLine_t::LINE_2)
            {
                lineName = "Line 2"; 
                success = ctrl.setDroopCorrectionLine2Enable(enableValue.value());
            }
            
            if(success)
            {
                logger::info(__FILE__, __FUNCTION__, 
                    std::string("Droop Correction ") + lineName +
                    std::string(enableValue.value() ? " Enabled" : " Disabled"));
                
                // Set response payload
                if(helpers::jsonSet(jsonDoc, enableValue.value(), "resultPayload", "enabled") &&
                   helpers::jsonSet<std::string>(jsonDoc, lineName, "resultPayload", "line"))
                {
                    return;
                }
            }
            else
            {
                errMsg = "Failed to set droop correction " + lineName + " hardware state";
            }
        }
        else
        {
            errMsg = "No enable/disable value provided in commandValue";
        }
    }

    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the operating mode.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getOperatingMode(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto gain{ctrl.getGainMode()}; gain.has_value())
        {
            if(auto state{ctrl.getRfState()}; state.has_value())
            {
                if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
                {
                    if(helpers::jsonSet(jsonDoc, helpers::types::gainModeToCmdStr(gain.value()),
                            "resultPayload", "rfGainMode") &&
                        helpers::jsonSet(jsonDoc, helpers::types::onlineStateToCmdStr(state.value()),
                            "resultPayload", "operatingMode")
                    )
                    {
                        return;
                    }
                }
            }
        }
    }

    const std::string errMsg{"Failed to Get the Amplifier Operation Mode"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the mgc setpoint percentage.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getMgcSetpointPercentage(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getMgcSetpointPercentage()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, static_cast<unsigned>(std::round(resp.value())), "resultPayload", "value") &&
                helpers::jsonSet<std::string>(jsonDoc, "MGC Level", "resultPayload", "label") &&
                helpers::jsonSet<std::string>(jsonDoc, "%", "resultPayload", "units"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the MGC Percentage Setpoint..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the mgc setpoint percentage.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setMgcSetpointPercentage(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto val{helpers::jsonGet<helpers::types::percent_t>(jsonDoc, "request", "commandValue")}; val.has_value())
        {
            if(ctrl.setMgcSetpointPercentage(val.value()))
            {
                if(helpers::jsonSet(jsonDoc, val.value(), "resultPayload", "value") &&
                    helpers::jsonSet<std::string>(jsonDoc, "MGC Level", "resultPayload", "label") &&
                    helpers::jsonSet<std::string>(jsonDoc, "%", "resultPayload", "units"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to complete setting the MGC Percentage Setpoint..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the mgc setpoint gain.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getMgcSetpointGain(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getMgcSetpointGain()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, resp.value(), "resultPayload", "value") &&
                helpers::jsonSet<std::string>(jsonDoc, "MGC Gain", "resultPayload", "label") &&
                helpers::jsonSet<std::string>(jsonDoc, "dB", "resultPayload", "units"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the MGC Gain Setpoint..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the mgc setpoint gain.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setMgcSetpointGain(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto val{helpers::jsonGet<helpers::types::dbm_t>(jsonDoc, "request", "commandValue")}; val.has_value())
        {
            if(ctrl.setMgcSetpointGain(val.value()))
            {
                if(helpers::jsonSet(jsonDoc, val.value(), "resultPayload", "value") &&
                    helpers::jsonSet<std::string>(jsonDoc, "MGC Gain", "resultPayload", "label") &&
                    helpers::jsonSet<std::string>(jsonDoc, "dB", "resultPayload", "units"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to complete setting the MGC Gain Setpoint..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the mgc setpoint vva.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getMgcSetpointVva(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getMgcSetpointVva()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, resp.value(), "resultPayload", "value") &&
                helpers::jsonSet<std::string>(jsonDoc, "MGC VVA Attenuation", "resultPayload", "label") &&
                helpers::jsonSet<std::string>(jsonDoc, "dB", "resultPayload", "units"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the MGC VVA Attenuation Setpoint..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the mgc setpoint vva.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setMgcSetpointVva(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto val{helpers::jsonGet<helpers::types::mmData_t>(jsonDoc, "request", "commandValue")}; val.has_value())
        {
            if(ctrl.setMgcSetpointVva(val.value()))
            {
                if(helpers::jsonSet(jsonDoc, val.value(), "resultPayload", "value") &&
                    helpers::jsonSet<std::string>(jsonDoc, "MGC VVA Attenuation", "resultPayload", "label") &&
                    helpers::jsonSet<std::string>(jsonDoc, "dB", "resultPayload", "units"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to complete setting the MGC VVA Attenuation Setpoint..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the alc setpoint.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getAlcSetpoint(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getAlcSetpoint()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, resp.value(), "resultPayload", "value") &&
                helpers::jsonSet<std::string>(jsonDoc, "ALC Level", "resultPayload", "label") &&
                helpers::jsonSet<std::string>(jsonDoc, "dBm", "resultPayload", "units"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the ALC Setpoint..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the alc setpoint.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setAlcSetpoint(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto val{helpers::jsonGet<helpers::types::dbm_t>(jsonDoc, "request", "commandValue")}; val.has_value())
        {
            if(ctrl.setAlcSetpoint(val.value()))
            {
                if(helpers::jsonSet(jsonDoc, val.value(), "resultPayload", "value") &&
                    helpers::jsonSet<std::string>(jsonDoc, "ALC Level", "resultPayload", "label") &&
                    helpers::jsonSet<std::string>(jsonDoc, "dBm", "resultPayload", "units"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to complete setting the ALC Level Value..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the agc setpoint.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getAgcSetpoint(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getAgcSetpoint()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, resp.value(), "resultPayload", "value") &&
                helpers::jsonSet<std::string>(jsonDoc, "AGC Level", "resultPayload", "label") &&
                helpers::jsonSet<std::string>(jsonDoc, "dB", "resultPayload", "units"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the AGC Setpoint..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the agc setpoint.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setAgcSetpoint(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto val{helpers::jsonGet<helpers::types::dbm_t>(jsonDoc, "request", "commandValue")}; val.has_value())
        {
            if(ctrl.setAgcSetpoint(val.value()))
            {
                if(helpers::jsonSet(jsonDoc, val.value(), "resultPayload", "value") &&
                    helpers::jsonSet<std::string>(jsonDoc, "AGC Level", "resultPayload", "label") &&
                    helpers::jsonSet<std::string>(jsonDoc, "dB", "resultPayload", "units"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to complete setting the AGC Level Value..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the input select.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getInputSelect(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getInputSelect()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, helpers::types::inpSelToStr(resp.value()), "resultPayload"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the Input Select State..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the input select.
 *
 * @param      jsonDoc  The json document
 * @param[in]  sw       The new value
 */
void rfManagerParser::setInputSelect(rapidjson::Document& jsonDoc, const input_select_t sw)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setInputSelect(sw))
        {
            return;
        }
    }

    const std::string errMsg{"Failed to Set the Amplifier Input Select state"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the input switch.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getInputSwitch(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getInputSwitch()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, helpers::types::inpSwitchToStr(resp.value()), "resultPayload"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the Input Switch State..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the phase.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getPhase(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getPhaseCur()}; resp.has_value())
        {
            if(auto tgtPhase{ctrl.getPhaseTgt()}; tgtPhase.has_value())
            {
                const bool walking{(resp.value() & 0x80000000) == 0x80000000};
                const std::uint32_t val{resp.value() & 0x7F};
                if(helpers::jsonSet(jsonDoc, val, "resultPayload", "value") &&
                   helpers::jsonSet(jsonDoc, tgtPhase.value(), "resultPayload", "target") &&
                   helpers::jsonSet<bool>(jsonDoc, walking, "resultPayload", "walking") &&
                   helpers::jsonSet<std::string>(jsonDoc, "Amplifier Phase", "resultPayload", "label") &&
                   helpers::jsonSet<std::string>(jsonDoc, "Deg", "resultPayload", "units"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to get the Input Switch State..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the phase.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setPhase(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto val{helpers::jsonGet<helpers::types::phase_t>(jsonDoc, "request", "commandValue")}; val.has_value())
        {
            if(ctrl.setPhaseTgt(val.value()))
            {
                if(helpers::jsonSet(jsonDoc, val.value(), "resultPayload", "value") &&
                   helpers::jsonSet<std::string>(jsonDoc, "Phase", "resultPayload", "label") &&
                   helpers::jsonSet<std::string>(jsonDoc, "Deg", "resultPayload", "units"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to complete setting the AGC Level Value..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}


/**
 * @brief      Gets the phase.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getRfBand(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getRfBandIndex()}; resp.has_value())
        {

            if(helpers::jsonSet(jsonDoc, std::to_string(resp.value()), "resultPayload"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the RF Band Index..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the phase.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setRfBand(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

//        if(auto val{helpers::jsonGet<helpers::types::rf_band_index_t>(jsonDoc, "request", "commandValue")}; val.has_value())
//        {
//            if(ctrl.setRfBandIndex(val.value()))
//            {
//                if(helpers::jsonSet(jsonDoc, val.value(), "resultPayload", "value"))
//                {
//                    return;
//                }
//            }
//        }
    }
    return;
    
    const std::string errMsg{"Failed to complete setting the RF Band Index..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the blanking input.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getBlankingInput(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getBlankingInput()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, helpers::types::sigToStrHigh(resp.value()), "resultPayload"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the Blanking Input signal..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the blanking state.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getBlankingState(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getBlankingState()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, helpers::types::sigToStrActive(resp.value()), "resultPayload"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the Blanking State signal..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the blanking polarity.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getBlankingPolarity(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getBlankingPol()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, helpers::types::polToStr(resp.value()), "resultPayload"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the Blanking Polarity..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the blanking polarity.
 *
 * @param      jsonDoc  The json document
 * @param[in]  pol      The new value
 */
void rfManagerParser::setBlankingPolarity(rapidjson::Document& jsonDoc, const polarity_t pol)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setBlankingPol(pol))
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the shutdown input.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getShutdownInput(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getShutdownInput()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, helpers::types::sigToStrHigh(resp.value()), "resultPayload"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the Shutdown Input signal..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the shutdown state.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getShutdownState(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getShutdownState()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, helpers::types::sigToStrActive(resp.value()), "resultPayload"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the Shutdown State signal..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the shutdown polarity.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getShutdownPolarity(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getShutdownPol()}; resp.has_value())
        {
            if(helpers::jsonSet(jsonDoc, helpers::types::polToStr(resp.value()), "resultPayload"))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to get the Shutdown Polarity..."};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the shutdown polarity.
 *
 * @param      jsonDoc  The json document
 * @param[in]  pol      The new value
 */
void rfManagerParser::setShutdownPolarity(rapidjson::Document& jsonDoc, const polarity_t pol)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setShutdownPol(pol))
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the Enable/Disables the BIT oscillator
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setBitEnableSelect(rapidjson::Document& jsonDoc)
{
    using namespace helpers;

    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto val{jsonGet<std::string>(jsonDoc, "request", "commandValue")}; val.has_value())
        {
            std::string upperVal;
            std::transform(std::begin(val.value()), std::end(val.value()), std::back_inserter(upperVal),
                [](const auto& c){ return std::toupper(c); });

            if(ctrl.setBitFuncGenEnableSelect(types::strToSigBitIntExt(upperVal)))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to set BIT Enable Bit.  Configuration parameter BIT_ENABLE_FUNCTION_GENERATOR " \
        "is likely set to False.  BIT operation is not permitted!"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the bit enable parameters of Pulse Width and Duty Cycle.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setBitEnableParameters(rapidjson::Document& jsonDoc)
{
    using namespace helpers;

    std::string errMsg{"Failed to get message from FPGA Hardware Interface"};

    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());
        if(auto params{jsonGetVec<std::string>(jsonDoc, "request", "commandParameters")}; params.has_value())
        {
            std::optional<double> pulseWidthSeconds;
            std::optional<percent_t> dutyCyclePercentage;

            for(const auto& str: params.value())
            {
                auto kv{types::dataTokenizer<std::string>(str, "=")};
                if(kv.size() >= 2)
                {
                    if(kv.at(0).compare("pulseWidthSeconds") == 0)
                    {
                        pulseWidthSeconds = types::strToNum<double>(kv.at(1));
                    }
                    else if(kv.at(0).compare("dutyCyclePercentage") == 0)
                    {
                        dutyCyclePercentage = types::strToNum<percent_t>(kv.at(1));
                    }
                }
            }

            if(pulseWidthSeconds.has_value() && dutyCyclePercentage.has_value())
            {
                auto pulseWidthNano{std::chrono::duration<double>{pulseWidthSeconds.value()}};
                if(ctrl.setBitEnablePulseParams(std::chrono::duration_cast<fpga_clock_rate_t>(pulseWidthNano),
                    dutyCyclePercentage.value()))
                {
                    return;
                }

                errMsg = "Failed to set BIT Pulse Parameters.  Either configuration parameter " \
                    "BIT_ENABLE_FUNCTION_GENERATOR is not set to True, or the given Pulse Width and Duty Cycle " \
                    "were out of range.";
            }

            errMsg = "Failed to set BIT Pulse Parameters.  Either parameter for Pulse Width (pulseWidthSeconds) or " \
                "Duty Cycle (dutyCyclePercentage) were not provided in the JSON message properly.";
        }
        else
        {
            errMsg = "Failed to set BIT Pulse Parameters.  " \
                "The Parameters Pulse Width and/or Duty Cycle values were not given.";
        }
    }
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the bit enable guard time in nanoseconds.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setBitEnableGuardTimeNs(rapidjson::Document& jsonDoc)
{
    using namespace helpers;

    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(const auto val{jsonGet<std::uint64_t>(jsonDoc, "request", "commandValue")}; val.has_value())
        {
            const auto setVal{std::chrono::duration<fpga_clock_rate_t::rep, std::nano>(val.value())};
            if(ctrl.setBitRfGuardTime(std::chrono::duration_cast<fpga_clock_rate_t>(setVal)))
            {
                return;
            }
        }
    }

    const std::string errMsg{"Failed to set BIT Enable Bit.  Configuration parameter BIT_ENABLE_FUNCTION_GENERATOR " \
        "is likely set to False.  BIT operation is not permitted!"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the bit rf modulation parameters.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::setBitRfModulationParameters(rapidjson::Document& jsonDoc)
{
    using namespace helpers;

    std::string errMsg{"Failed to get message from FPGA Hardware Interface"};

    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());
        if(auto params{jsonGetVec<std::string>(jsonDoc, "request", "commandParameters")}; params.has_value())
        {
            std::optional<double> activeTimeSeconds;
            std::optional<percent_t> dutyCyclePercentage;

            for(const auto& str: params.value())
            {
                auto kv{types::dataTokenizer<std::string>(str, "=")};
                if(kv.size() >= 2)
                {
                    if(kv.at(0).compare("activeTimeSeconds") == 0)
                    {
                        activeTimeSeconds = types::strToNum<double>(kv.at(1));
                    }
                    else if(kv.at(0).compare("dutyCyclePercentage") == 0)
                    {
                        dutyCyclePercentage = types::strToNum<percent_t>(kv.at(1));
                    }
                }
            }

            if(activeTimeSeconds.has_value() && dutyCyclePercentage.has_value())
            {
                auto activeTimeNano{std::chrono::duration<double>{activeTimeSeconds.value()}};

                if(ctrl.setBitRfModulationParams(std::chrono::duration_cast<fpga_clock_rate_t>(activeTimeNano),
                    dutyCyclePercentage.value()))
                {
                    return;
                }

                errMsg = "Failed to set BIT Pulse Parameters.  Either configuration parameter " \
                    "BIT_ENABLE_FUNCTION_GENERATOR is not set to True, or the given Duty Cycle was out of range.";
            }
            else
            {
                errMsg = "Failed to set BIT Pulse Parameters.  " \
                    "The Parameters Active Time and/or Duty Cycle values were not given.";
            }
        }
        else
        {
            errMsg = "Failed to set BIT Pulse Parameters.  " \
                "The Parameters Active Time and/or Duty Cycle values were not given.";
        }
    }

    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Sets the rf droop correction.
 *
 * @param      jsonDoc  The json document
 * @param[in]  line     The line
 */
void rfManagerParser::setRfDroopCorrection(rapidjson::Document& jsonDoc, const droopLine_t line)
{
    using namespace helpers;

    std::string errMsg{"Failed to get message from FPGA Hardware Interface"};

    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());
        if(auto params{jsonGetVec<std::string>(jsonDoc, "request", "commandParameters")}; params.has_value())
        {
            std::optional<mmData_t> runCount;
            std::optional<mmData_t> dropCount;
            std::optional<mmData_t> riseCount;

            for(const auto& str: params.value())
            {
                auto kv{types::dataTokenizer<std::string>(str, "=")};
                if(kv.size() >= 2)
                {
                    if(kv.at(0).compare("dropCount") == 0)
                    {
                        dropCount = types::strToNum<mmData_t>(kv.at(1));
                    }
                    else if(kv.at(0).compare("runCount") == 0)
                    {
                        runCount = types::strToNum<mmData_t>(kv.at(1));
                    }
                    else if(kv.at(0).compare("riseCount") == 0)
                    {
                        riseCount = types::strToNum<mmData_t>(kv.at(1));
                    }
                }
            }

            if(dropCount.has_value() && runCount.has_value() && riseCount.has_value())
            {
                if(ctrl.setRfDroopCorrectionRise(line, riseCount.value()) &&
                   ctrl.setRfDroopCorrectionRun(line, runCount.value()) &&
                   ctrl.setRfDroopCorrectionDrop(line, dropCount.value()))
                {
                    return;
                }

                errMsg = "Failed to set RF Droop Correction Parameters.  FPGA values not set correctly.";
            }
            else
            {
                errMsg = "Failed to set RF Droop Correction Parameters.  " \
                    "The Parameters dropCount, runCount, and/or riseCount values were not given.";
            }
        }
        else
        {
            errMsg = "Failed to set RF Droop Correction Parameters.  " \
                "The Parameters dropCount, runCount, and/or riseCount values were not given.";
        }
    }

    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

void rfManagerParser::getManualInputSwitch(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto resp{ctrl.getManualInputSwitch()}; resp.has_value())
        {
            if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
            {
                if(helpers::jsonSet(jsonDoc, helpers::types::manualInpSwitchToStr(resp.value()), "resultPayload"))
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

void rfManagerParser::setManualInputSwitch(rapidjson::Document& jsonDoc, const manual_input_switch_t sw)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(ctrl.setManualInputSwitch(sw))
        {
            return;
        }
    }

    const std::string errMsg{"Failed to get message from FPGA Hardware Interface"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the power levels.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::getPowerLevels(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(helpers::jsonSet<std::string>(jsonDoc, "success", "result"))
        {
            if(auto det{ctrl.getRegulationDet()}; det.has_value())
            {
                auto data{ctrl.pollPowerLevels()};
                rfManagerController::dbm_t vswr, fwd, rev, inp, unb;

                switch(det.value())
                {
                case helpers::types::detector_t::RMS:
                    vswr = rfManagerController::getTupVswrRms(data);
                    fwd = rfManagerController::getTupFwdRms(data);
                    rev = rfManagerController::getTupRevRms(data);
                    inp = rfManagerController::getTupInpRms(data);
                    unb = rfManagerController::getTupUnbRms(data);
                    break;

                case helpers::types::detector_t::PEAK:
                    vswr = rfManagerController::getTupVswrPeak(data);
                    fwd = rfManagerController::getTupFwdPeak(data);
                    rev = rfManagerController::getTupRevPeak(data);
                    inp = rfManagerController::getTupInpPeak(data);
                    unb = rfManagerController::getTupUnbPeak(data);
                    break;

                default: // helpers::types::detector_t::ENV
                    vswr = rfManagerController::getTupVswrEnv(data);
                    fwd = rfManagerController::getTupFwdEnv(data);
                    rev = rfManagerController::getTupRevEnv(data);
                    inp = rfManagerController::getTupInpEnv(data);
                    unb = rfManagerController::getTupUnbEnv(data);
                    break;
                }

                if(helpers::jsonSet(jsonDoc, rfManagerController::getTupFwdRms(data), "resultPayload", "FwdRms") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupFwdEnv(data), "resultPayload", "FwdEnv") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupFwdPeak(data), "resultPayload", "FwdPeak") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupRevRms(data), "resultPayload", "RevRms") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupRevEnv(data), "resultPayload", "RevEnv") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupRevPeak(data), "resultPayload", "RevPeak") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupInpRms(data), "resultPayload", "InpRms") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupInpEnv(data), "resultPayload", "InpEnv") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupInpPeak(data), "resultPayload", "InpPeak") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupUnbRms(data), "resultPayload", "UnbRms") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupUnbEnv(data), "resultPayload", "UnbEnv") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupUnbPeak(data), "resultPayload", "UnbPeak") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupVswrRms(data), "resultPayload", "VswrRms") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupVswrEnv(data), "resultPayload", "VswrEnv") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupVswrPeak(data), "resultPayload", "VswrPeak") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupFwdPapr(data), "resultPayload", "FwdPapr") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupRevPapr(data), "resultPayload", "RevPapr") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupInpPapr(data), "resultPayload", "InpPapr") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupUnbPapr(data), "resultPayload", "UnbPapr") &&
                    helpers::jsonSet(jsonDoc, rfManagerController::getTupCompr(data), "resultPayload", "Compression") &&
                    helpers::jsonSet(jsonDoc, vswr, "resultPayload", "vswr") &&
                    helpers::jsonSet(jsonDoc, fwd, "resultPayload", "forward") &&
                    helpers::jsonSet(jsonDoc, rev, "resultPayload", "reverse") &&
                    helpers::jsonSet(jsonDoc, inp, "resultPayload", "input") &&
                    helpers::jsonSet(jsonDoc, unb, "resultPayload", "unbalanced") &&
                    helpers::jsonSet(jsonDoc, helpers::types::detectorToStr(det.value()), "resultPayload", "detector"))
                {
                    return;
                }
            }
        }
    }

    const std::string errMsg{"Failed to read Power Level data"};
    msgIf.convertSuccessToErrorResponse(jsonDoc, errMsg);
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Runs a calibration step.
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::calRunStep(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        auto chan{helpers::jsonGet<std::string>(jsonDoc, "request", "channel")};
        auto sigGenIpAddr{helpers::jsonGet<std::string>(jsonDoc, "request", "signalGeneratorIp")};
        auto freqMHz{helpers::jsonGet<std::uint32_t>(jsonDoc, "request", "frequencyMHz")};
        auto powerLeveldBm{helpers::jsonGet<helpers::types::dbm_t>(jsonDoc, "request", "powerLeveldBm")};
        auto numberOfSamples{helpers::jsonGet<std::uint32_t>(jsonDoc, "request", "numberOfSamples")};
        auto disableSigGen{helpers::jsonGet<bool>(jsonDoc, "request", "disableSigGen")};
        auto calDecayRate{helpers::jsonGet<helpers::types::decay_rate_t::rep>(jsonDoc, "request", "decayRate")};

        auto origDecayRate{ctrl.getPeakDecayRate()};
        bool setDecayRate{calDecayRate.has_value() && origDecayRate.has_value()};

        if(setDecayRate)
        {
            setDecayRate &= ctrl.setPeakDecayRate(helpers::types::decay_rate_t{calDecayRate.value()});
        }

        if(chan.has_value() && sigGenIpAddr.has_value() && freqMHz.has_value() &&
            powerLeveldBm.has_value() && numberOfSamples.has_value())
        {
            if(auto inpSwitchState{ctrl.getInputSwitch()}; inpSwitchState.has_value())
            {
                using std::string_literals::operator""s;
                using namespace std::chrono_literals;
                using namespace helpers::types;

                logger::verbose(__FILE__, __FUNCTION__,
                    "Running Calibration Step"s +
                    "\n\tChannel: "s  + chan.value() +
                    "\n\tFrequency (MHz): "s + std::to_string(freqMHz.value()) +
                    "\n\tPower Level (dBm): "s + std::to_string(powerLeveldBm.value()) +
                    "\n\tNumber of Samples: "s + std::to_string(numberOfSamples.value()));

                std::vector<adc_raw_t> rawEnv, rawPeak, rawRms;
                siggen sigSrc(sigGenIpAddr.value());

                sigSrc.setFreq(freqMHz.value(), "MHz");
                sigSrc.setAmp(powerLeveldBm.value());
                sigSrc.outputEnable(true);

                std::this_thread::sleep_for(200ms);
                ctrl.setInputSelect(input_select_t::INPUT);
                std::this_thread::sleep_for(50ms);

                for(std::size_t idx = 0; idx < numberOfSamples.value(); ++idx)
                {
                    std::this_thread::sleep_for(10ms);
                    rawEnv.emplace_back(ctrl.readRawAdc({strToPort(chan.value()), detector_t::ENV}).value_or(0));
                    rawPeak.emplace_back(ctrl.readRawAdc({strToPort(chan.value()), detector_t::PEAK}).value_or(0));
                    rawRms.emplace_back(ctrl.readRawAdc({strToPort(chan.value()), detector_t::RMS}).value_or(0));
                }

                if (disableSigGen.value_or(false))
                {
                    sigSrc.outputEnable(false);
                }

                const double numSampF{static_cast<double>(numberOfSamples.value())};

                std::vector<double> diffEnv;
                double meanEnv = std::accumulate(rawEnv.begin(), rawEnv.end(), 0.0) / numSampF;
                std::transform(rawEnv.begin(), rawEnv.end(), std::back_inserter(diffEnv),
                    [meanEnv](double x) {return x - meanEnv;});
                double stdDevEnv = std::sqrt(std::inner_product(diffEnv.begin(), diffEnv.end(), diffEnv.begin(), 0.0) /
                    numSampF);

                std::vector<double> diffPeak;
                double meanPeak = std::accumulate(rawPeak.begin(), rawPeak.end(), 0.0) / numSampF;
                std::transform(rawPeak.begin(), rawPeak.end(), std::back_inserter(diffPeak),
                    [meanPeak](double x) {return x - meanPeak;});
                double stdDevPeak = std::sqrt(std::inner_product(diffPeak.begin(), diffPeak.end(), diffPeak.begin(), 0.0) /
                    numSampF);

                std::vector<double> diffRms;
                double meanRms = std::accumulate(rawRms.begin(), rawRms.end(), 0.0) / numSampF;
                std::transform(rawRms.begin(), rawRms.end(), std::back_inserter(diffRms),
                    [meanRms](double x) {return x - meanRms;});
                double stdDevRms = std::sqrt(std::inner_product(diffRms.begin(), diffRms.end(), diffRms.begin(), 0.0) /
                    numSampF);

                helpers::jsonSet(jsonDoc, meanEnv, "resultPayload", "sampleEnvMean");
                helpers::jsonSet(jsonDoc, stdDevEnv, "resultPayload", "sampleEnvStdDev");
                helpers::jsonSetVec(jsonDoc, rawEnv, "resultPayload", "sampleEnvRaw");
                helpers::jsonSet(jsonDoc, meanPeak, "resultPayload", "samplePeakMean");
                helpers::jsonSet(jsonDoc, stdDevPeak, "resultPayload", "samplePeakStdDev");
                helpers::jsonSetVec(jsonDoc, rawPeak, "resultPayload", "samplePeakRaw");
                helpers::jsonSet(jsonDoc, meanRms, "resultPayload", "sampleRmsMean");
                helpers::jsonSet(jsonDoc, stdDevRms, "resultPayload", "sampleRmsStdDev");
                helpers::jsonSetVec(jsonDoc, rawRms, "resultPayload", "sampleRmsRaw");

                return;
            }
        }

        if(setDecayRate)
        {
            ctrl.setPeakDecayRate(origDecayRate.value());
        }
    }

    logger::warn(__FILE__, __FUNCTION__, "Failed to execute calibration step");
}

/**
 * @brief      Resets the calibration setup and equipment
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::calReset(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto sigGenIpAddr{helpers::jsonGet<std::string>(jsonDoc, "request", "signalGeneratorIp")};
            sigGenIpAddr.has_value())
        {
            if(siggen::reset(sigGenIpAddr.value()))
            {
                return;
            }
        }
    }

    logger::warn(__FILE__, __FUNCTION__, "Failed to Reset Calibration Instrument");
}

/**
 * @brief      Gets the Signal Generator ID
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::calGetId(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto sigGenIpAddr{helpers::jsonGet<std::string>(jsonDoc, "request", "signalGeneratorIp")};
            sigGenIpAddr.has_value())
        {
            const auto idStr{siggen::getId(sigGenIpAddr.value())};
            helpers::jsonSet(jsonDoc, idStr.value_or(""), "resultPayload", "id");
            return;
        }
    }

    logger::warn(__FILE__, __FUNCTION__, "Failed to aquire Calibration Instrument ID");
}

/**
 * @brief      Disables the calibration Signal Generator
 *
 * @param      jsonDoc  The json document
 */
void rfManagerParser::calDisableSigGen(rapidjson::Document& jsonDoc)
{
    if(auto jsonResp{msgIf.getMessageResponse(jsonDoc, "getCommandComplete")}; jsonResp.has_value())
    {
        jsonDoc.Swap(jsonResp.value());

        if(auto sigGenIpAddr{helpers::jsonGet<std::string>(jsonDoc, "request", "signalGeneratorIp")};
            sigGenIpAddr.has_value())
        {
            siggen sigSrc(sigGenIpAddr.value());
            sigSrc.outputEnable(false);
            return;
        }
    }

    logger::warn(__FILE__, __FUNCTION__, "Failed to disable Signal Generator");
}
