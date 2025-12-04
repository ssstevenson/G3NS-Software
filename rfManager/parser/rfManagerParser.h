#ifndef RF_MANAGER_PARSER_H_
#define RF_MANAGER_PARSER_H_

#include <controller/rfManagerController.h>
#include <helpers/jsonUpdateHelpers.h>
#include <helpers/mainHelper.h>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <parser/parserInterface.h>



namespace empower
{
    /**
     * @brief      This class describes a rf manager parser.
     */
    class rfManagerParser: protected parserInterface
    {
    public:
        using online_state_t = helpers::types::online_state_t;
        using safe_state_t = helpers::types::safe_state_t;
        using gain_mode_t = helpers::types::gain_mode_t;
        using input_select_t = helpers::types::input_select_t;
        using input_switch_t = helpers::types::input_switch_t;
        using phase_t = helpers::types::phase_t;
        using polarity_t = helpers::types::polarity_t;
        using dbm_t = helpers::types::dbm_t;
        using percent_t = helpers::types::percent_t;
        using fpga_clock_rate_t = helpers::types::fpga_clock_rate_t;
        using droopLine_t = helpers::types::droopLine_t;
        using mmData_t = helpers::types::mmData_t;
        using manual_input_switch_t = helpers::types::manual_input_switch_t;
        using rf_band_index_t = helpers::types::rf_band_index_t;

        rfManagerController ctrl;
    private:
        messageFactoryConnection msgIf;

    public:
        rfManagerParser(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& conf);

    private:
        void setProfile(rapidjson::Document& jsonDoc);
        void getRfState(rapidjson::Document& jsonDoc);
        void setRfState(rapidjson::Document& jsonDoc, const online_state_t online);
        void getSafeState(rapidjson::Document& jsonDoc);
        void setSafeState(rapidjson::Document& jsonDoc, const safe_state_t safeState);
        void getGainMode(rapidjson::Document& jsonDoc);
        void setGainMode(rapidjson::Document& jsonDoc, const gain_mode_t mode);
        void getOperatingMode(rapidjson::Document& jsonDoc);
        void getMgcSetpointPercentage(rapidjson::Document& jsonDoc);
        void setMgcSetpointPercentage(rapidjson::Document& jsonDoc);
        void getMgcSetpointGain(rapidjson::Document& jsonDoc);
        void setMgcSetpointGain(rapidjson::Document& jsonDoc);
        void getMgcSetpointVva(rapidjson::Document& jsonDoc);
        void setMgcSetpointVva(rapidjson::Document& jsonDoc);
        void getAlcSetpoint(rapidjson::Document& jsonDoc);
        void setAlcSetpoint(rapidjson::Document& jsonDoc);
        void getAgcSetpoint(rapidjson::Document& jsonDoc);
        void setAgcSetpoint(rapidjson::Document& jsonDoc);
        void getInputSelect(rapidjson::Document& jsonDoc);
        void setInputSelect(rapidjson::Document& jsonDoc, const input_select_t sw);
        void getInputSwitch(rapidjson::Document& jsonDoc);
        void getPhase(rapidjson::Document& jsonDoc);
        void setPhase(rapidjson::Document& jsonDoc);
        void getRfBand(rapidjson::Document& jsonDoc);
        void setRfBand(rapidjson::Document& jsonDoc);
        void getBlankingInput(rapidjson::Document& jsonDoc);
        void getBlankingState(rapidjson::Document& jsonDoc);
        void getBlankingPolarity(rapidjson::Document& jsonDoc);
        void setBlankingPolarity(rapidjson::Document& jsonDoc, const polarity_t pol);
        void getShutdownInput(rapidjson::Document& jsonDoc);
        void getShutdownState(rapidjson::Document& jsonDoc);
        void getShutdownPolarity(rapidjson::Document& jsonDoc);
        void setShutdownPolarity(rapidjson::Document& jsonDoc, const polarity_t pol);
        void setBitEnableSelect(rapidjson::Document& jsonDoc);
        void setBitEnableParameters(rapidjson::Document& jsonDoc);
        void setBitEnableGuardTimeNs(rapidjson::Document& jsonDoc);
        void setBitRfModulationParameters(rapidjson::Document& jsonDoc);
        void setRfDroopCorrection(rapidjson::Document& jsonDoc, const droopLine_t line);
        void getManualInputSwitch(rapidjson::Document& jsonDoc);
        void setManualInputSwitch(rapidjson::Document& jsonDoc, const manual_input_switch_t sw);
        void setDroopCorrectionEnable(rapidjson::Document& jsonDoc, const droopLine_t line);

        void getPowerLevels(rapidjson::Document& jsonDoc);

        void calRunStep(rapidjson::Document& jsonDoc);
        void calReset(rapidjson::Document& jsonDoc);
        void calGetId(rapidjson::Document& jsonDoc);
        void calDisableSigGen(rapidjson::Document& jsonDoc);
    };
}

#endif
