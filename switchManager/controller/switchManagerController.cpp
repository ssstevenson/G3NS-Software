#include "switchManagerController.h"

#include <logger/logger.h>

#include <chrono>
#include <thread>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      zmqCtx  The zmq context
 * @param      config  The configuration
 */
switchManagerController::switchManagerController(zmq::context_t& zmqCtx, configManagerConnection& config):
    configIf{config},
    fpgaIf{zmqCtx},
    updateIf{zmqCtx},
    statusUpdateData{
        updateIfData_t{"BS_CONFIG",
            "Switch Configuration", "", defaultTimeout},
        updateIfData_t{"BS_CURRENT_BAND_OFFSET",
            "Current Band / Offset", "", defaultTimeout},
        updateIfData_t{"BS_SRC_CTRL_SW",
            "Software Source Control", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_ENABLED",
            "Switch 1 Enabled State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_ENABLED",
            "Switch 2 Enabled State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_FB_IND_HIGH_A_IN",
            "Local Switch 1 Dual Coil High Feedback Indicator A Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_FB_IND_HIGH_B_IN",
            "Local Switch 1 Dual Coil High Feedback Indicator B Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_FB_IND_HIGH_A_STATE",
            "Local Switch 1 Dual Coil High Feedback Indicator A State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_FB_IND_HIGH_B_STATE",
            "Local Switch 1 Dual Coil High Feedback Indicator B State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_HIGH_FAULT",
            "Local Switch 1 Dual Coil High Fault", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_CONTROL_HIGH_OUT",
            "Local Switch 1 Dual Coil High Output", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_FB_IND_LOW_A_IN",
            "Local Switch 1 Dual Coil Low Feedback Indicator A Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_FB_IND_LOW_B_IN",
            "Local Switch 1 Dual Coil Low Feedback Indicator B Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_FB_IND_LOW_A_STATE",
            "Local Switch 1 Dual Coil Low Feedback Indicator A State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_FB_IND_LOW_B_STATE",
            "Local Switch 1 Dual Coil Low Feedback Indicator B State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_LOW_FAULT",
            "Local Switch 1 Dual Coil Low Fault", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_CONTROL_LOW_OUT",
            "Local Switch 1 Dual Coil Low Output", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_STEADY_FB_IND_A_IN",
            "Local Switch 1 Dual Coil Steady Feedback Indicator A Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_STEADY_FB_IND_B_IN",
            "Local Switch 1 Dual Coil Steady Feedback Indicator B Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_STEADY_FB_IND_A_STATE",
            "Local Switch 1 Dual Coil Steady Feedback Indicator A State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_STEADY_FB_IND_B_STATE",
            "Local Switch 1 Dual Coil Steady Feedback Indicator B State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_STEADY_FAULT",
            "Local Switch 1 Dual Coil Steady Fault", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_STEADY_CONTROL_OUT",
            "Local Switch 1 Dual Coil Steady Output", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_CONFIG_FAULT",
            "Local Switch 1 Configuration Fault", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_FAULT_SUMMARY",
            "Local Switch 1 Fault Summary", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_HW_CONTROL_IN",
            "Local Switch 1 Hardware Control Input", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_SYSTEM_GUARD_SIGNAL",
            "Local Switch 1 is indicating it is muting the system for a change", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_RELAY_CONTROL_OUT",
            "Local Switch 1 Relay Control Output signal", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_STATE_MACHINE_STATE",
            "Local Switch 1 State Machine State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_RELAY_CTRL_STR",
            "Local Switch 1 Relay Control String", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_1_DUAL_SINGLE_CONFIG",
            "Local Switch 1 Dual or Single Coil Configuration", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_FB_IND_HIGH_A_IN",
            "Local Switch 2 Dual Coil High Feedback Indicator A Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_FB_IND_HIGH_B_IN",
            "Local Switch 2 Dual Coil High Feedback Indicator B Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_FB_IND_HIGH_A_STATE",
            "Local Switch 2 Dual Coil High Feedback Indicator A State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_FB_IND_HIGH_B_STATE",
            "Local Switch 2 Dual Coil High Feedback Indicator B State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_HIGH_FAULT",
            "Local Switch 2 Dual Coil High Fault", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_CONTROL_HIGH_OUT",
            "Local Switch 2 Dual Coil High Output", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_FB_IND_LOW_A_IN",
            "Local Switch 2 Dual Coil Low Feedback Indicator A Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_FB_IND_LOW_B_IN",
            "Local Switch 2 Dual Coil Low Feedback Indicator B Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_FB_IND_LOW_A_STATE",
            "Local Switch 2 Dual Coil Low Feedback Indicator A State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_FB_IND_LOW_B_STATE",
            "Local Switch 2 Dual Coil Low Feedback Indicator B State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_LOW_FAULT",
            "Local Switch 2 Dual Coil Low Fault", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_CONTROL_LOW_OUT",
            "Local Switch 2 Dual Coil Low Output", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_STEADY_FB_IND_A_IN",
            "Local Switch 2 Dual Coil Steady Feedback Indicator A Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_STEADY_FB_IND_B_IN",
            "Local Switch 2 Dual Coil Steady Feedback Indicator B Hardware Line Input Value", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_STEADY_FB_IND_A_STATE",
            "Local Switch 2 Dual Coil Steady Feedback Indicator A State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_STEADY_FB_IND_B_STATE",
            "Local Switch 2 Dual Coil Steady Feedback Indicator B State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_STEADY_FAULT",
            "Local Switch 2 Dual Coil Steady Fault", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_STEADY_CONTROL_OUT",
            "Local Switch 2 Dual Coil Steady Output", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_CONFIG_FAULT",
            "Local Switch 2 Configuration Fault", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_FAULT_SUMMARY",
            "Local Switch 2 Fault Summary", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_HW_CONTROL_IN",
            "Local Switch 2 Relay Control", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_SYSTEM_GUARD_SIGNAL",
            "Local Switch 2 is indicating it is muting the system for a change", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_RELAY_CONTROL_OUT",
            "Local Switch 2 Relay Control Output signal", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_STATE_MACHINE_STATE",
            "Local Switch 2 State Machine State", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_RELAY_CTRL_STR",
            "Local Switch 2 Hardware Control Input String", "", defaultTimeout},
        updateIfData_t{"SWITCH_LOCAL_2_DUAL_SINGLE_CONFIG",
            "Local Switch 2 Dual or Single Coil Configuration", "", defaultTimeout}
    },
    bands{},
    curBand{bands_t::A},
    curOff{0},
    switchFunctionMap{{switch_t::SWITCH_1, switch_function_t::TR}, {switch_t::SWITCH_2, switch_function_t::DUMMY}},
    switchTxRegValMap{{switch_t::SWITCH_1, 0}, {switch_t::SWITCH_2, 0}},
    switchTxAltRegValMap{{switch_t::SWITCH_1, 1}, {switch_t::SWITCH_2, 1}},
    switchRegValNameMap{
        {switch_t::SWITCH_1, {{0, "Receive"}, {1, "Transmit"}}},
        {switch_t::SWITCH_2, {{0, "Load"}, {1, "Transmit"}}}
    },
    switchManagerModName{"FILTER"},
    systemControllerModName{"SYS_CTRL"}
{}

/**
 * @brief      Initializes the data.
 */
void switchManagerController::initializeData()
{
    using namespace helpers::types;
    using std::literals::string_literals::operator""s;

    fpgaIf.write(switchManagerModName, switchManagerRegMap::swMute, 1);

    bands = getRfLayoutStructure(configIf);
    setupOffsetTables();
    setBand(strToBand(configIf.getParam<std::string>("SW_DEFAULT_BAND").value_or("")),
        configIf.getParam<std::size_t>("SW_DEFAULT_BAND_OFF").value_or(0));
    setBandSourceSelect(strToSrcSel(configIf.getParam<std::string>("SW_DEFAULT_SRC_SEL").value_or("")));

    for(std::size_t i = 0; i < switchControllerCount; ++i)
    {
        const auto sw{static_cast<switch_t>(i)};
        const auto preKey{"SW_LOCAL_"s + std::to_string(i + 1) + "_"s};

        setSwitchEnabled(sw, boolToSig(configIf.getParam<bool>(preKey + "ENABLED"s).value_or(false)));
        setSwitchControlSource(sw, strToSrcSel(configIf.getParam<std::string>(preKey + "CTRL_SRC"s).value_or("SW")));
        setSoftwareControl(sw, boolToSig(configIf.getParam<bool>(preKey + "SW_CTRL_STATE"s).value_or(false)));
        setSwitchDualEnable(sw, boolToSig(configIf.getParam<bool>(preKey + "DUAL_EN"s).value_or(false)));
        setSwitchInputInvert(sw, boolToSig(configIf.getParam<bool>(preKey + "INPUT_INV"s).value_or(false)));
        setSwitchFeedbackInvert(sw, boolToSig(configIf.getParam<bool>(preKey + "FB_INV"s).value_or(false)));
        setSwitchSteadyFeedbackAEnable(sw,
            boolToSig(configIf.getParam<bool>(preKey + "STDY_FB_A_EN"s).value_or(false)));
        setSwitchSteadyFeedbackBEnable(sw,
            boolToSig(configIf.getParam<bool>(preKey + "STDY_FB_B_EN"s).value_or(false)));
        setSwitchDualHighFeedbackAEnable(sw,
            boolToSig(configIf.getParam<bool>(preKey + "DUAL_HIGH_A_EN"s).value_or(false)));
        setSwitchDualHighFeedbackBEnable(sw,
            boolToSig(configIf.getParam<bool>(preKey + "DUAL_HIGH_B_EN"s).value_or(false)));
        setSwitchDualLowFeedbackAEnable(sw,
            boolToSig(configIf.getParam<bool>(preKey + "DUAL_LOW_A_EN"s).value_or(false)));
        setSwitchDualLowFeedbackBEnable(sw,
            boolToSig(configIf.getParam<bool>(preKey + "DUAL_LOW_B_EN"s).value_or(false)));
        setSwitchGuardTime(sw,
            time_duration_t{configIf.getParam<time_duration_t::rep>(preKey + "GUARD_TIME"s).value_or(0.03)});
        setSwitchDualTime(sw,
            time_duration_t{configIf.getParam<time_duration_t::rep>(preKey + "DUAL_TIME"s).value_or(0.03)});
        setSwitchDischargeTime(sw,
            time_duration_t{configIf.getParam<time_duration_t::rep>(preKey + "DISCHARGE_TIME"s).value_or(0.00001)});
        switchFunctionMap[sw] = strToSwitchFunction(
            configIf.getParam<std::string>(preKey + "FUNCTION"s).value_or("TR"));
        switchTxRegValMap[sw] = configIf.getParam<mmData_t>(preKey + "TRANSMIT_REG_VAL"s).value_or(0);
        switchTxAltRegValMap[sw] = configIf.getParam<mmData_t>(preKey + "TX_ALT_REG_VAL"s).value_or(1);

        const auto regValStateNameConfig{configIf.getParam<std::string>(preKey + "REG_VAL_STATE_NAME"s).value_or("")};
        const auto pairs{dataTokenizer<std::string>(regValStateNameConfig, ",")};
        for(const auto& p: pairs)
        {
            const auto data{dataTokenizer<std::string>(p, "=")};
            if(data.size() == 2)
            {
                switchRegValNameMap[sw][strToNum<mmData_t>(data.at(0)).value_or(0)] = data.at(1);
            }
        }
    }

    fpgaIf.write(switchManagerModName, switchManagerRegMap::swMute, 0);
}

/**
 * @brief      Sets up the Offset Tables in the FPGA
 */
void switchManagerController::setupOffsetTables()
{
    using namespace helpers::types;
    using std::literals::string_literals::operator""s;

    auto strRemoveLastChar = [](std::string& str){ str.erase(std::prev(std::end(str))); };

    std::string offsetTableConfig;

    for(const auto& [band, path]: bands)
    {
        const auto bandIdx{bandTypeIndex(band)};
        const auto offsets{rfPathToOffsets(path)};
        const auto bandOff{(bandIdx * OFFSET_POINTS_PER_PATH)};
        const auto paEnableRegVal{paEnToRegVal(rfPathToPaEn(path))};
        std::string fwdOffsets, revOffsets, inpOffsets, unbOffsets;

        fpgaIf.write(switchManagerModName, (switchManagerRegMap::paEnableStart + bandIdx),
            paEnableRegVal);

        offsetTableConfig += "<"s + bandToStr(band) + "|"s + toHexString(paEnableRegVal) + "|"s;

        for(std::size_t i = 0; i < OFFSET_POINTS_PER_PATH; ++i)
        {
            const auto offsetSelect{findOffset(offsets, i)};
            const auto indexOff{(bandOff + (i << 2))};

            const auto fwdOff{offsetToFwdOff(offsetSelect)};
            const auto revOff{offsetToRevOff(offsetSelect)};
            const auto inpOff{offsetToInpOff(offsetSelect)};
            const auto unbOff{offsetToUnbOff(offsetSelect)};

            fpgaIf.write(switchManagerModName, (switchManagerRegMap::fwdLutStart + indexOff), dbmToRegVal(fwdOff));
            fpgaIf.write(switchManagerModName, (switchManagerRegMap::revLutStart + indexOff), dbmToRegVal(revOff));
            fpgaIf.write(switchManagerModName, (switchManagerRegMap::inpLutStart + indexOff), dbmToRegVal(inpOff));
            fpgaIf.write(switchManagerModName, (switchManagerRegMap::unbLutStart + indexOff), dbmToRegVal(unbOff));

            fwdOffsets += std::to_string(i) + "-"s + std::to_string(fwdOff) + "/"s;
            revOffsets += std::to_string(i) + "-"s + std::to_string(revOff) + "/"s;
            inpOffsets += std::to_string(i) + "-"s + std::to_string(inpOff) + "/"s;
            unbOffsets += std::to_string(i) + "-"s + std::to_string(unbOff) + "/"s;
        }

        strRemoveLastChar(fwdOffsets);
        strRemoveLastChar(revOffsets);
        strRemoveLastChar(inpOffsets);
        strRemoveLastChar(unbOffsets);

        offsetTableConfig += fwdOffsets + "|"s + revOffsets + "|"s + inpOffsets + "|"s + unbOffsets + ">,"s;
    }

    strRemoveLastChar(offsetTableConfig);

    statusUpdateData.at(configIdx).data = offsetTableConfig;
    statusUpdateData.at(configIdx).valid = true;
}

/**
 * @brief      Gets the band raw.
 *
 * @return     The band raw if sucessful, std::nullopt otherwise.
 */
std::optional<switchManagerController::mmData_t> switchManagerController::getBandRaw() noexcept
{
    if(auto resp{fpgaIf.read(switchManagerModName, switchManagerRegMap::stat)}; resp.has_value())
    {
        return resp.value();
    }

    return std::nullopt;
}

/**
 * @brief      Gets the band.
 *
 * @return     The band if sucessful, std::nullopt otherwise.
 */
std::optional<std::pair<switchManagerController::bands_t, std::size_t>> switchManagerController::getBand() noexcept
{
    if(auto resp{getBandRaw()}; resp.has_value())
    {
        return helpers::types::regValToBandOffset(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the band.
 *
 * @param[in]  b     The new value
 * @param[in]  off   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool switchManagerController::setBand(const bands_t b, const std::size_t off) noexcept
{
    using namespace helpers::types;

    // This will do a lot of other stuff, but for now, this is it...
    curBand = b;
    curOff = (off % OFFSET_POINTS_PER_PATH);

    return fpgaIf.write(switchManagerModName, switchManagerRegMap::bandSel,
        static_cast<mmData_t>(bandTypeIndex(b) + curOff));
}

/**
 * @brief      Gets the band source select.
 *
 * @return     The source select if sucessful, std::nullopt otherwise.
 */
std::optional<switchManagerController::source_select_t> switchManagerController::getBandSourceSelect() noexcept
{
    if(auto resp{fpgaIf.read(switchManagerModName, switchManagerRegMap::srcSel)}; resp.has_value())
    {
        return helpers::types::regValToSrcSel(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the band source select.
 *
 * @param[in]  src   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool switchManagerController::setBandSourceSelect(const source_select_t src) noexcept
{
    return fpgaIf.write(switchManagerModName, switchManagerRegMap::srcSel,
        helpers::types::srcSelToRegVal(src));
}

[[nodiscard]] std::optional<switchManagerController::switchStatus_t>
switchManagerController::getSwitchStatus(const switch_t& s) noexcept
{
    using namespace helpers::types;

    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlStatusOff))}; resp.has_value())
    {
        const auto regVal{resp.value()};

        static const auto bitMaskToSignal = [](const auto reg, const auto off)
        {
            return boolToSig((reg & (1u << off)) != 0);
        };

        static const auto statusBitsToStateMachineState = [](const auto reg){
            std::string str{"WAIT"};

            if(1 == (reg & 0x03))
            {
                str = "DISCHARGE_TIME";
            }
            else if(2 == (reg & 0x03))
            {
                str = "DUAL_TIME";
            }
            else if(3 == (reg & 0x03))
            {
                str = "GUARD_TIME";
            }

            return str;
        };

        return std::make_tuple(
            bitMaskToSignal(regVal, 24), bitMaskToSignal(regVal, 23), bitMaskToSignal(regVal, 22),
            bitMaskToSignal(regVal, 21), bitMaskToSignal(regVal, 20), bitMaskToSignal(regVal, 19),
            bitMaskToSignal(regVal, 18), bitMaskToSignal(regVal, 17), bitMaskToSignal(regVal, 16),
            bitMaskToSignal(regVal, 15), bitMaskToSignal(regVal, 14), bitMaskToSignal(regVal, 13),
            bitMaskToSignal(regVal, 12), bitMaskToSignal(regVal, 11), bitMaskToSignal(regVal, 10),
            bitMaskToSignal(regVal, 9), bitMaskToSignal(regVal, 8), bitMaskToSignal(regVal, 7),
            bitMaskToSignal(regVal, 6), bitMaskToSignal(regVal, 5), bitMaskToSignal(regVal, 4),
            bitMaskToSignal(regVal, 3), bitMaskToSignal(regVal, 2), statusBitsToStateMachineState(regVal)
        );
    }

    return std::nullopt;
}

/**
 * @brief      Gets the software control.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The software control.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSoftwareControl(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlSoftwareControlOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the software control.
 *
 * @param[in]  s     The new value
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSoftwareControl(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlSoftwareControlOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch enabled state.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch enabled state.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchEnabled(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlEnableOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch enabled state.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchEnabled(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlEnableOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch control source.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch control source.
 */
[[nodiscard]] std::optional<switchManagerController::source_select_t>
switchManagerController::getSwitchControlSource(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlControlSourceOff))}; resp.has_value())
    {
        return helpers::types::regValToSwitchSrcSel(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch control source.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise
 */
bool switchManagerController::setSwitchControlSource(const switch_t& s, const source_select_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlControlSourceOff), helpers::types::switchSrcSelToRegVal(val));
}

/**
 * @brief      Gets the switch dual coil enable.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch dual coil enable.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchDualEnable(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlDualEnOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch dual coilenable.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchDualEnable(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlDualEnOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch input invert.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch input invert.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchInputInvert(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlInputPolarityOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch input invert.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchInputInvert(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlInputPolarityOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch feedback invert.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch feedback invert.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchFeedbackInvert(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlFeedbackPolarityOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch feedback invert.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchFeedbackInvert(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlFeedbackPolarityOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch steady feedback a enable.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch steady feedback a enable.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchSteadyFeedbackAEnable(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlSteadyAFeedbackEnOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch steady feedback a enable.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchSteadyFeedbackAEnable(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlSteadyAFeedbackEnOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch steady feedback b enable.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch steady feedback b enable.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchSteadyFeedbackBEnable(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlSteadyBFeedbackEnOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch steady feedback b enable.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchSteadyFeedbackBEnable(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlSteadyBFeedbackEnOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch dual high feedback a enable.
 *
 * @param[in]  s     Switch instance
 *
 * @return     The switch dual high feedback a enable.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchDualHighFeedbackAEnable(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlDualAHighFeedbackOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch dual high feedback a enable.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchDualHighFeedbackAEnable(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlDualAHighFeedbackOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch dual high feedback b enable.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch dual high feedback b enable.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchDualHighFeedbackBEnable(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlDualBHighFeedbackOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch dual high feedback b enable.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchDualHighFeedbackBEnable(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlDualBHighFeedbackOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch dual low feedback a enable.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch dual low feedback a enable.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchDualLowFeedbackAEnable(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlDualALowFeedbackOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch dual low feedback a enable.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchDualLowFeedbackAEnable(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlDualALowFeedbackOff), helpers::types::sigToRegVal(val));
}

/**
 * @brief      Gets the switch dual low feedback b enable.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch dual low feedback b enable.
 */
[[nodiscard]] std::optional<switchManagerController::signal_t>
switchManagerController::getSwitchDualLowFeedbackBEnable(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlDualBLowFeedbackOff))}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch dual low feedback b enable.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchDualLowFeedbackBEnable(const switch_t& s, const signal_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlDualBLowFeedbackOff), helpers::types::sigToRegVal(val));
}



/**
 * @brief      Gets the switch guard time.
 *
 * @param[in]  s     Swtich Instance
 *
 * @return     The switch guard time.
 */
[[nodiscard]] std::optional<switchManagerController::time_duration_t>
switchManagerController::getSwitchGuardTime(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlGuardTimeOff))}; resp.has_value())
    {
        return helpers::types::regMapToTime<time_duration_t>(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch guard time.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchGuardTime(const switch_t& s, const time_duration_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlGuardTimeOff), helpers::types::timeToFpgaRegMap(val));
}

/**
 * @brief      Gets the switch dual time.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch dual time.
 */
[[nodiscard]] std::optional<switchManagerController::time_duration_t>
switchManagerController::getSwitchDualTime(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlDualTimeOff))}; resp.has_value())
    {
        return helpers::types::regMapToTime<time_duration_t>(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch dual time.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchDualTime(const switch_t& s, const time_duration_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlDualTimeOff), helpers::types::timeToFpgaRegMap(val));
}

/**
 * @brief      Gets the switch discharge time.
 *
 * @param[in]  s     Switch Instance
 *
 * @return     The switch discharge time.
 */
[[nodiscard]] std::optional<switchManagerController::time_duration_t>
switchManagerController::getSwitchDischargeTime(const switch_t& s) noexcept
{
    if(auto resp{fpgaIf.read(systemControllerModName,
        getSwitchAddr(s, systemControllerRegMap::switchCtrlDischargeTimeOff))}; resp.has_value())
    {
        return helpers::types::regMapToTime<time_duration_t>(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the switch discharge time.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool switchManagerController::setSwitchDischargeTime(const switch_t& s, const time_duration_t& val) noexcept
{
    return fpgaIf.write(systemControllerModName, getSwitchAddr(s,
        systemControllerRegMap::switchCtrlDischargeTimeOff), helpers::types::timeToFpgaRegMap(val));
}

/**
 * @brief      Gets the tr switch state.
 *
 * @return     The tr switch state.
 */
[[nodiscard]] std::optional<std::string>
switchManagerController::getTrSwitchState() noexcept
{
    try
    {
        if(auto trSwitch{getTrSwitch()}; trSwitch.has_value())
        {
            if(auto resp{fpgaIf.read(systemControllerModName,
                getSwitchAddr(trSwitch.value(), systemControllerRegMap::switchCtrlSoftwareControlOff))};
                resp.has_value())
            {
                return switchRegValNameMap.at(trSwitch.value()).at(resp.value());
            }
        }
    }
    catch(...)
    {
        return std::nullopt;
    }

    return std::nullopt;
}

/**
 * @brief      Sets the tr switch transmit.
 *
 * @return     True if the operation was successful, False otherwise.
 */
bool switchManagerController::setTrSwitchTransmit() noexcept
{
    if(auto trSwitch{getTrSwitch()}; trSwitch.has_value())
    {
        return fpgaIf.write(systemControllerModName,
            getSwitchAddr(trSwitch.value(), systemControllerRegMap::switchCtrlSoftwareControlOff),
            switchTxRegValMap.at(trSwitch.value()));
    }

    return false;
}

/**
 * @brief      Sets the tr switch receive.
 *
 * @return     True if the operation was successful, False otherwise.
 */
bool switchManagerController::setTrSwitchReceive() noexcept
{
    if(auto trSwitch{getTrSwitch()}; trSwitch.has_value())
    {
        return fpgaIf.write(systemControllerModName,
            getSwitchAddr(trSwitch.value(), systemControllerRegMap::switchCtrlSoftwareControlOff),
            switchTxAltRegValMap.at(trSwitch.value()));
    }

    return false;
}

/**
 * @brief      Gets the dummy switch state.
 *
 * @return     The dummy switch state.
 */
[[nodiscard]] std::optional<std::string>
switchManagerController::getDummySwitchState() noexcept
{
    try
    {
        if(auto trSwitch{getDummySwitch()}; trSwitch.has_value())
        {
            if(auto resp{fpgaIf.read(systemControllerModName,
                getSwitchAddr(trSwitch.value(), systemControllerRegMap::switchCtrlSoftwareControlOff))};
                resp.has_value())
            {
                return switchRegValNameMap.at(trSwitch.value()).at(resp.value());
            }
        }
    }
    catch(...)
    {
        return std::nullopt;
    }

    return std::nullopt;
}

/**
 * @brief      Sets the dummy switch transmit.
 *
 * @return     True if the operation was successful, False otherwise.
 */
bool switchManagerController::setDummySwitchTransmit() noexcept
{
    if(auto trSwitch{getDummySwitch()}; trSwitch.has_value())
    {
        return fpgaIf.write(systemControllerModName,
            getSwitchAddr(trSwitch.value(), systemControllerRegMap::switchCtrlSoftwareControlOff),
            switchTxRegValMap.at(trSwitch.value()));
    }

    return false;
}

/**
 * @brief      Sets the dummy switch load.
 *
 * @return     True if the operation was successful, False otherwise.
 */
bool switchManagerController::setDummySwitchLoad() noexcept
{
    if(auto trSwitch{getDummySwitch()}; trSwitch.has_value())
    {
        return fpgaIf.write(systemControllerModName,
            getSwitchAddr(trSwitch.value(), systemControllerRegMap::switchCtrlSoftwareControlOff),
            switchTxAltRegValMap.at(trSwitch.value()));
    }

    return false;
}

/**
 * @brief      Periodically polls the Switch State and reports it to status
 */
void switchManagerController::pollSwitchState()
{
    using namespace helpers::types;

    static const auto getBandOffData = [&](){
        const auto resp{getBandRaw()};
        return std::make_pair(bandOffsetToStr(resp.value_or(0)), resp.has_value());
    };

    static const auto getSrcSelData = [&](){
        const auto resp{getBandSourceSelect()};
        return std::make_pair(srcSelToStr(resp.value_or(source_select_t::SW)), resp.has_value());
    };

    static const auto getSwitchEnabledData = [&](const switch_t& sw){
        const auto resp{getSwitchEnabled(sw)};
        return std::make_pair(sigToBool(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    static const auto getSwitchStatusData = [&](const switch_t& sw){
        const auto status{getSwitchStatus(sw)};

        if(status.has_value())
        {
            return std::make_tuple(
                sigToBool(std::get<0>(status.value())), true, sigToBool(std::get<1>(status.value())), true,
                sigToBool(std::get<2>(status.value())), true, sigToBool(std::get<3>(status.value())), true,
                sigToBool(std::get<4>(status.value())), true, sigToBool(std::get<5>(status.value())), true,
                sigToBool(std::get<6>(status.value())), true, sigToBool(std::get<7>(status.value())), true,
                sigToBool(std::get<8>(status.value())), true, sigToBool(std::get<9>(status.value())), true,
                sigToBool(std::get<10>(status.value())), true, sigToBool(std::get<11>(status.value())), true,
                sigToBool(std::get<12>(status.value())), true, sigToBool(std::get<13>(status.value())), true,
                sigToBool(std::get<14>(status.value())), true, sigToBool(std::get<15>(status.value())), true,
                sigToBool(std::get<16>(status.value())), true, sigToBool(std::get<17>(status.value())), true,
                sigToBool(std::get<18>(status.value())), true, sigToBool(std::get<19>(status.value())), true,
                sigToBool(std::get<20>(status.value())), true, sigToBool(std::get<21>(status.value())), true,
                sigToBool(std::get<22>(status.value())), true, std::get<23>(status.value()), true,
                switchRegValNameMap.at(sw).at(static_cast<mmData_t>(sigToBool(std::get<22>(status.value())))), true
            );
        }

        return std::make_tuple(
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, std::string("WAIT"), false,
            std::string("Transmit"), false
        );
    };

    static const auto getSwitchDualEnableConfig = [&](const switch_t& sw){
        const auto dualEnConf{getSwitchDualEnable(sw)};
        std::string resp{"Single"};

        if(signal_t::ACTIVE == dualEnConf.value_or(signal_t::INACTIVE))
        {
            resp = std::string("Dual");
        }

        return std::make_pair(resp, dualEnConf.has_value());
    };

    std::tie(statusUpdateData.at(bandOffIdx).data, statusUpdateData.at(bandOffIdx).valid) = getBandOffData();
    std::tie(statusUpdateData.at(srcSelIdx).data, statusUpdateData.at(srcSelIdx).valid) = getSrcSelData();
    std::tie(statusUpdateData.at(localSwitch1EnabledIdx).data, statusUpdateData.at(localSwitch1EnabledIdx).valid) =
        getSwitchEnabledData(switch_t::SWITCH_1);
    std::tie(statusUpdateData.at(localSwitch2EnabledIdx).data, statusUpdateData.at(localSwitch2EnabledIdx).valid) =
        getSwitchEnabledData(switch_t::SWITCH_2);
    std::tie(statusUpdateData.at(localSwitch1DualSingleConfig).data, statusUpdateData.at(localSwitch1DualSingleConfig).valid) =
        getSwitchDualEnableConfig(switch_t::SWITCH_1);
    std::tie(statusUpdateData.at(localSwitch2DualSingleConfig).data, statusUpdateData.at(localSwitch2DualSingleConfig).valid) =
        getSwitchDualEnableConfig(switch_t::SWITCH_2);

    std::tie(
        statusUpdateData.at(localSwitch1DualFbIndHighAIdx).data, statusUpdateData.at(localSwitch1DualFbIndHighAIdx).valid,
        statusUpdateData.at(localSwitch1DualFbIndHighBIdx).data, statusUpdateData.at(localSwitch1DualFbIndHighBIdx).valid,
        statusUpdateData.at(localSwitch1DualFbIndHighAStateIdx).data, statusUpdateData.at(localSwitch1DualFbIndHighAStateIdx).valid,
        statusUpdateData.at(localSwitch1DualFbIndHighBStateIdx).data, statusUpdateData.at(localSwitch1DualFbIndHighBStateIdx).valid,
        statusUpdateData.at(localSwitch1DualHighFaultIdx).data, statusUpdateData.at(localSwitch1DualHighFaultIdx).valid,
        statusUpdateData.at(localSwitch1DualControlHighOutIdx).data, statusUpdateData.at(localSwitch1DualControlHighOutIdx).valid,
        statusUpdateData.at(localSwitch1DualFbIndLowAIdx).data, statusUpdateData.at(localSwitch1DualFbIndLowAIdx).valid,
        statusUpdateData.at(localSwitch1DualFbIndLowBIdx).data, statusUpdateData.at(localSwitch1DualFbIndLowBIdx).valid,
        statusUpdateData.at(localSwitch1DualFbIndLowAStateIdx).data, statusUpdateData.at(localSwitch1DualFbIndLowAStateIdx).valid,
        statusUpdateData.at(localSwitch1DualFbIndLowBStateIdx).data, statusUpdateData.at(localSwitch1DualFbIndLowBStateIdx).valid,
        statusUpdateData.at(localSwitch1DualLowFaultIdx).data, statusUpdateData.at(localSwitch1DualLowFaultIdx).valid,
        statusUpdateData.at(localSwitch1DualControlLowOutIdx).data, statusUpdateData.at(localSwitch1DualControlLowOutIdx).valid,
        statusUpdateData.at(localSwitch1FbIndSteadyAIdx).data, statusUpdateData.at(localSwitch1FbIndSteadyAIdx).valid,
        statusUpdateData.at(localSwitch1FbIndSteadyBIdx).data, statusUpdateData.at(localSwitch1FbIndSteadyBIdx).valid,
        statusUpdateData.at(localSwitch1FbIndSteadyAStateIdx).data, statusUpdateData.at(localSwitch1FbIndSteadyAStateIdx).valid,
        statusUpdateData.at(localSwitch1FbIndSteadyBStateIdx).data, statusUpdateData.at(localSwitch1FbIndSteadyBStateIdx).valid,
        statusUpdateData.at(localSwitch1SteadyFaultIdx).data, statusUpdateData.at(localSwitch1SteadyFaultIdx).valid,
        statusUpdateData.at(localSwitch1ControlSteadyOutIdx).data, statusUpdateData.at(localSwitch1ControlSteadyOutIdx).valid,
        statusUpdateData.at(localSwitch1ConfigFaultIdx).data, statusUpdateData.at(localSwitch1ConfigFaultIdx).valid,
        statusUpdateData.at(localSwitch1FaultSummaryIdx).data, statusUpdateData.at(localSwitch1FaultSummaryIdx).valid,
        statusUpdateData.at(localSwitch1HardwareControlInIdx).data, statusUpdateData.at(localSwitch1HardwareControlInIdx).valid,
        statusUpdateData.at(localSwitch1SystemGuardSignalIdx).data, statusUpdateData.at(localSwitch1SystemGuardSignalIdx).valid,
        statusUpdateData.at(localSwitch1RelayControlOutIdx).data, statusUpdateData.at(localSwitch1RelayControlOutIdx).valid,
        statusUpdateData.at(localSwitch1StateMachineStateIdx).data, statusUpdateData.at(localSwitch1StateMachineStateIdx).valid,
        statusUpdateData.at(localSwitch1RelayControlStringIdx).data, statusUpdateData.at(localSwitch1RelayControlStringIdx).valid
    ) = getSwitchStatusData(switch_t::SWITCH_1);

    std::tie(
        statusUpdateData.at(localSwitch2DualFbIndHighAIdx).data, statusUpdateData.at(localSwitch2DualFbIndHighAIdx).valid,
        statusUpdateData.at(localSwitch2DualFbIndHighBIdx).data, statusUpdateData.at(localSwitch2DualFbIndHighBIdx).valid,
        statusUpdateData.at(localSwitch2DualFbIndHighAStateIdx).data, statusUpdateData.at(localSwitch2DualFbIndHighAStateIdx).valid,
        statusUpdateData.at(localSwitch2DualFbIndHighBStateIdx).data, statusUpdateData.at(localSwitch2DualFbIndHighBStateIdx).valid,
        statusUpdateData.at(localSwitch2DualHighFaultIdx).data, statusUpdateData.at(localSwitch2DualHighFaultIdx).valid,
        statusUpdateData.at(localSwitch2DualControlHighOutIdx).data, statusUpdateData.at(localSwitch2DualControlHighOutIdx).valid,
        statusUpdateData.at(localSwitch2DualFbIndLowAIdx).data, statusUpdateData.at(localSwitch2DualFbIndLowAIdx).valid,
        statusUpdateData.at(localSwitch2DualFbIndLowBIdx).data, statusUpdateData.at(localSwitch2DualFbIndLowBIdx).valid,
        statusUpdateData.at(localSwitch2DualFbIndLowAStateIdx).data, statusUpdateData.at(localSwitch2DualFbIndLowAStateIdx).valid,
        statusUpdateData.at(localSwitch2DualFbIndLowBStateIdx).data, statusUpdateData.at(localSwitch2DualFbIndLowBStateIdx).valid,
        statusUpdateData.at(localSwitch2DualLowFaultIdx).data, statusUpdateData.at(localSwitch2DualLowFaultIdx).valid,
        statusUpdateData.at(localSwitch2DualControlLowOutIdx).data, statusUpdateData.at(localSwitch2DualControlLowOutIdx).valid,
        statusUpdateData.at(localSwitch2FbIndSteadyAIdx).data, statusUpdateData.at(localSwitch2FbIndSteadyAIdx).valid,
        statusUpdateData.at(localSwitch2FbIndSteadyBIdx).data, statusUpdateData.at(localSwitch2FbIndSteadyBIdx).valid,
        statusUpdateData.at(localSwitch2FbIndSteadyAStateIdx).data, statusUpdateData.at(localSwitch2FbIndSteadyAStateIdx).valid,
        statusUpdateData.at(localSwitch2FbIndSteadyBStateIdx).data, statusUpdateData.at(localSwitch2FbIndSteadyBStateIdx).valid,
        statusUpdateData.at(localSwitch2SteadyFaultIdx).data, statusUpdateData.at(localSwitch2SteadyFaultIdx).valid,
        statusUpdateData.at(localSwitch2ControlSteadyOutIdx).data, statusUpdateData.at(localSwitch2ControlSteadyOutIdx).valid,
        statusUpdateData.at(localSwitch2ConfigFaultIdx).data, statusUpdateData.at(localSwitch2ConfigFaultIdx).valid,
        statusUpdateData.at(localSwitch2FaultSummaryIdx).data, statusUpdateData.at(localSwitch2FaultSummaryIdx).valid,
        statusUpdateData.at(localSwitch2HardwareControlInIdx).data, statusUpdateData.at(localSwitch2HardwareControlInIdx).valid,
        statusUpdateData.at(localSwitch2SystemGuardSignalIdx).data, statusUpdateData.at(localSwitch2SystemGuardSignalIdx).valid,
        statusUpdateData.at(localSwitch2RelayControlOutIdx).data, statusUpdateData.at(localSwitch2RelayControlOutIdx).valid,
        statusUpdateData.at(localSwitch2StateMachineStateIdx).data, statusUpdateData.at(localSwitch2StateMachineStateIdx).valid,
        statusUpdateData.at(localSwitch2RelayControlStringIdx).data, statusUpdateData.at(localSwitch2RelayControlStringIdx).valid
    ) = getSwitchStatusData(switch_t::SWITCH_2);

    updateIf.updateData(statusUpdateData);
}

/**
 * @brief      Gets the switch base address from a switch enum.
 *
 * @param[in]  s     Switch Enumeration
 *
 * @return     The switch base address.
 */
[[nodiscard]] constexpr switchManagerController::reg_t
switchManagerController::getSwitchBase(const switch_t& s) noexcept
{
    reg_t result{systemControllerRegMap::switchCtrl1Start};

    if(switch_t::SWITCH_2 == s)
    {
        result = systemControllerRegMap::switchCtrl2Start;
    }

    return result;
}

/**
 * @brief      Gets the switch address given which switch and offset.
 *
 * @param[in]  s     Switch Instance
 * @param[in]  off   Offset
 *
 * @return     The switch address.
 */
[[nodiscard]] constexpr switchManagerController::reg_t
switchManagerController::getSwitchAddr(const switch_t& s, const reg_t& off) noexcept
{
    return getSwitchBase(s) + off;
}

/**
 * @brief      Gets the tr switch.
 *
 * @return     The tr switch.
 */
[[nodiscard]] std::optional<switchManagerController::switch_t>
switchManagerController::getTrSwitch() noexcept
{
    for(const auto& [sw, func]: switchFunctionMap)
    {
        if(helpers::types::switch_function_t::TR == func)
        {
            return sw;
        }
    }

    return std::nullopt;
}

/**
 * @brief      Gets the dummy switch.
 *
 * @return     The dummy switch.
 */
[[nodiscard]] std::optional<switchManagerController::switch_t>
switchManagerController::getDummySwitch() noexcept
{
    for(const auto& [sw, func]: switchFunctionMap)
    {
        if(helpers::types::switch_function_t::DUMMY == func)
        {
            return sw;
        }
    }

    return std::nullopt;
}
