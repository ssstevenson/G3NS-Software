#ifndef SWITCH_NAMAGER_CONTROLLER_H_
#define SWITCH_NAMAGER_CONTROLLER_H_

#include <array>
#include <configManagerConnection/configManagerConnection.h>
#include <fpgaInterfaceConnection/fpgaInterfaceConnection.h>
#include <helpers/rfLayoutTypes.h>
#include <optional>
#include <statusUpdateInterface/statusUpdateInterface.h>
#include <string>
#include <tuple>
#include <utility>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      Controls the data flow into a switch manager object and updates the view whenever data changes.
     */
    class switchManagerController
    {
    public:
        using updateIfData_t = helpers::types::updateIfData_t;
        using timeout_t = helpers::types::timeout_t;
        using time_duration_t = helpers::types::time_duration_t;
        using fpga_clock_rate_t = helpers::types::fpga_clock_rate_t;
        using rf_bands_t = helpers::types::rf_bands_t;
        using bands_t = helpers::types::bands_t;
        using pa_enable_t = helpers::types::pa_enable_t;
        using reg_t = fpgaInterfaceConnection::reg_t;
        using mmData_t = fpgaInterfaceConnection::mmData_t;
        using updateArray_t = std::array<updateIfData_t, 57>;
        using source_select_t = helpers::types::source_select_t;
        using signal_t = helpers::types::signal_t;
        using switch_function_t = helpers::types::switch_function_t;
        using switchStatus_t = std::tuple<
        //  Dual FB HA IO   Dual FB HB IO   Dual FB HA      Dual FB HB      Dual H Fault    Dual Ctrl H Drive
            signal_t,       signal_t,       signal_t,       signal_t,       signal_t,       signal_t,
        //  Dual FB LA IO   Dual FB LB IO   Dual FB LA      Dual FB LB      Dual L Fault    Dual Ctrl L Drive
            signal_t,       signal_t,       signal_t,       signal_t,       signal_t,       signal_t,
        //  Steady FB A IO  Steady FB B IO  Steady FB A     Steady FB B     Steady Fault    Steady Ctrl Drive
            signal_t,       signal_t,       signal_t,       signal_t,       signal_t,       signal_t,
        //  Config Fault    Summary Fault   HW Ctrl IO      Sys Guard       Relay Ctrl      SM State
            signal_t,       signal_t,       signal_t,       signal_t,       signal_t,       std::string>;

        enum class switch_t: std::size_t
        {
            SWITCH_1 = 0,
            SWITCH_2 = 1
        };
        static constexpr std::size_t switchControllerCount = 2;

        using switchFunctionMap_t = std::unordered_map<switch_t, switch_function_t>;
        using switchFunctionRegValMap_t = std::unordered_map<switch_t, mmData_t>;
        using switchRegValNameMap_t = std::unordered_map<switch_t, std::unordered_map<mmData_t, std::string>>;

    private:
        /**
         * @brief      Switch Manager Register Map
         */
        struct switchManagerRegMap
        {
            static constexpr reg_t stat = 0x0000;
            static constexpr reg_t srcSel = 0x0004;
            static constexpr reg_t bandSel = 0x0008;
            static constexpr reg_t swMute = 0x000C;
            static constexpr reg_t fwdLutStart = 0x0400;
            static constexpr reg_t revLutStart = 0x0800;
            static constexpr reg_t inpLutStart = 0x0C00;
            static constexpr reg_t unbLutStart = 0x1000;
            static constexpr reg_t paEnableStart = 0x1400;
        };

        /**
         * @brief      System Controller Register Map
         */
        struct systemControllerRegMap
        {
            static constexpr reg_t switchCtrl1Start = 0x0200;
            static constexpr reg_t switchCtrl2Start = 0x0280;

            static constexpr reg_t switchCtrlStatusOff = 0x0000;
            static constexpr reg_t switchCtrlSoftwareControlOff = 0x0004;
            static constexpr reg_t switchCtrlEnableOff = 0x0008;
            static constexpr reg_t switchCtrlControlSourceOff = 0x000C;
            static constexpr reg_t switchCtrlDualEnOff = 0x0010;
            static constexpr reg_t switchCtrlInputPolarityOff = 0x0014;
            static constexpr reg_t switchCtrlFeedbackPolarityOff = 0x0018;
            static constexpr reg_t switchCtrlSteadyAFeedbackEnOff = 0x001C;
            static constexpr reg_t switchCtrlSteadyBFeedbackEnOff = 0x0020;
            static constexpr reg_t switchCtrlDualAHighFeedbackOff = 0x0024;
            static constexpr reg_t switchCtrlDualBHighFeedbackOff = 0x0028;
            static constexpr reg_t switchCtrlDualALowFeedbackOff = 0x002C;
            static constexpr reg_t switchCtrlDualBLowFeedbackOff = 0x0030;
            static constexpr reg_t switchCtrlGuardTimeOff = 0x0034;
            static constexpr reg_t switchCtrlDualTimeOff = 0x0038;
            static constexpr reg_t switchCtrlDischargeTimeOff = 0x003C;

            static constexpr reg_t switch1CtrlStatus = switchCtrl1Start + switchCtrlStatusOff;
            static constexpr reg_t switch1CtrlSoftwareControl = switchCtrl1Start + switchCtrlSoftwareControlOff;
            static constexpr reg_t switch1CtrlEnable = switchCtrl1Start + switchCtrlEnableOff;
            static constexpr reg_t switch1CtrlControlSource = switchCtrl1Start + switchCtrlControlSourceOff;
            static constexpr reg_t switch1CtrlDualEn = switchCtrl1Start + switchCtrlDualEnOff;
            static constexpr reg_t switch1CtrlInputPolarity = switchCtrl1Start + switchCtrlInputPolarityOff;
            static constexpr reg_t switch1CtrlFeedbackPolarity = switchCtrl1Start + switchCtrlFeedbackPolarityOff;
            static constexpr reg_t switch1CtrlSteadyAFeedbackEn = switchCtrl1Start + switchCtrlSteadyAFeedbackEnOff;
            static constexpr reg_t switch1CtrlSteadyBFeedbackEn = switchCtrl1Start + switchCtrlSteadyBFeedbackEnOff;
            static constexpr reg_t switch1CtrlDualAHighFeedback = switchCtrl1Start + switchCtrlDualAHighFeedbackOff;
            static constexpr reg_t switch1CtrlDualBHighFeedback = switchCtrl1Start + switchCtrlDualBHighFeedbackOff;
            static constexpr reg_t switch1CtrlDualALowFeedback = switchCtrl1Start + switchCtrlDualALowFeedbackOff;
            static constexpr reg_t switch1CtrlDualBLowFeedback = switchCtrl1Start + switchCtrlDualBLowFeedbackOff;
            static constexpr reg_t switch1CtrlGuardTime = switchCtrl1Start + switchCtrlGuardTimeOff;
            static constexpr reg_t switch1CtrlDualTime = switchCtrl1Start + switchCtrlDualTimeOff;
            static constexpr reg_t switch1CtrlDischargeTime = switchCtrl1Start + switchCtrlDischargeTimeOff;

            static constexpr reg_t switch2CtrlStatus = switchCtrl2Start + switchCtrlStatusOff;
            static constexpr reg_t switch2CtrlSoftwareControl = switchCtrl2Start + switchCtrlSoftwareControlOff;
            static constexpr reg_t switch2CtrlEnable = switchCtrl2Start + switchCtrlEnableOff;
            static constexpr reg_t switch2CtrlControlSource = switchCtrl2Start + switchCtrlControlSourceOff;
            static constexpr reg_t switch2CtrlDualEn = switchCtrl2Start + switchCtrlDualEnOff;
            static constexpr reg_t switch2CtrlInputPolarity = switchCtrl2Start + switchCtrlInputPolarityOff;
            static constexpr reg_t switch2CtrlFeedbackPolarity = switchCtrl2Start + switchCtrlFeedbackPolarityOff;
            static constexpr reg_t switch2CtrlSteadyAFeedbackEn = switchCtrl2Start + switchCtrlSteadyAFeedbackEnOff;
            static constexpr reg_t switch2CtrlSteadyBFeedbackEn = switchCtrl2Start + switchCtrlSteadyBFeedbackEnOff;
            static constexpr reg_t switch2CtrlDualAHighFeedback = switchCtrl2Start + switchCtrlDualAHighFeedbackOff;
            static constexpr reg_t switch2CtrlDualBHighFeedback = switchCtrl2Start + switchCtrlDualBHighFeedbackOff;
            static constexpr reg_t switch2CtrlDualALowFeedback = switchCtrl2Start + switchCtrlDualALowFeedbackOff;
            static constexpr reg_t switch2CtrlDualBLowFeedback = switchCtrl2Start + switchCtrlDualBLowFeedbackOff;
            static constexpr reg_t switch2CtrlGuardTime = switchCtrl2Start + switchCtrlGuardTimeOff;
            static constexpr reg_t switch2CtrlDualTime = switchCtrl2Start + switchCtrlDualTimeOff;
            static constexpr reg_t switch2CtrlDischargeTime = switchCtrl2Start + switchCtrlDischargeTimeOff;
        };

        static constexpr auto defaultTimeout = timeout_t{(5000 * 3)};

        static constexpr updateArray_t::size_type configIdx = 0;
        static constexpr updateArray_t::size_type bandOffIdx = 1;
        static constexpr updateArray_t::size_type srcSelIdx = 2;

        static constexpr updateArray_t::size_type localSwitch1EnabledIdx = 3;
        static constexpr updateArray_t::size_type localSwitch2EnabledIdx = 4;

        static constexpr updateArray_t::size_type localSwitch1DualFbIndHighAIdx = 5;
        static constexpr updateArray_t::size_type localSwitch1DualFbIndHighBIdx = 6;
        static constexpr updateArray_t::size_type localSwitch1DualFbIndHighAStateIdx = 7;
        static constexpr updateArray_t::size_type localSwitch1DualFbIndHighBStateIdx = 8;
        static constexpr updateArray_t::size_type localSwitch1DualHighFaultIdx = 9;
        static constexpr updateArray_t::size_type localSwitch1DualControlHighOutIdx = 10;
        static constexpr updateArray_t::size_type localSwitch1DualFbIndLowAIdx = 11;
        static constexpr updateArray_t::size_type localSwitch1DualFbIndLowBIdx = 12;
        static constexpr updateArray_t::size_type localSwitch1DualFbIndLowAStateIdx = 13;
        static constexpr updateArray_t::size_type localSwitch1DualFbIndLowBStateIdx = 14;
        static constexpr updateArray_t::size_type localSwitch1DualLowFaultIdx = 15;
        static constexpr updateArray_t::size_type localSwitch1DualControlLowOutIdx = 16;
        static constexpr updateArray_t::size_type localSwitch1FbIndSteadyAIdx = 17;
        static constexpr updateArray_t::size_type localSwitch1FbIndSteadyBIdx = 18;
        static constexpr updateArray_t::size_type localSwitch1FbIndSteadyAStateIdx = 19;
        static constexpr updateArray_t::size_type localSwitch1FbIndSteadyBStateIdx = 20;
        static constexpr updateArray_t::size_type localSwitch1SteadyFaultIdx = 21;
        static constexpr updateArray_t::size_type localSwitch1ControlSteadyOutIdx = 22;
        static constexpr updateArray_t::size_type localSwitch1ConfigFaultIdx = 23;
        static constexpr updateArray_t::size_type localSwitch1FaultSummaryIdx = 24;
        static constexpr updateArray_t::size_type localSwitch1HardwareControlInIdx = 25;
        static constexpr updateArray_t::size_type localSwitch1SystemGuardSignalIdx = 26;
        static constexpr updateArray_t::size_type localSwitch1RelayControlOutIdx = 27;
        static constexpr updateArray_t::size_type localSwitch1StateMachineStateIdx = 28;
        static constexpr updateArray_t::size_type localSwitch1RelayControlStringIdx = 29;
        static constexpr updateArray_t::size_type localSwitch1DualSingleConfig = 30;

        static constexpr updateArray_t::size_type localSwitch2DualFbIndHighAIdx = 31;
        static constexpr updateArray_t::size_type localSwitch2DualFbIndHighBIdx = 32;
        static constexpr updateArray_t::size_type localSwitch2DualFbIndHighAStateIdx = 33;
        static constexpr updateArray_t::size_type localSwitch2DualFbIndHighBStateIdx = 34;
        static constexpr updateArray_t::size_type localSwitch2DualHighFaultIdx = 35;
        static constexpr updateArray_t::size_type localSwitch2DualControlHighOutIdx = 36;
        static constexpr updateArray_t::size_type localSwitch2DualFbIndLowAIdx = 37;
        static constexpr updateArray_t::size_type localSwitch2DualFbIndLowBIdx = 38;
        static constexpr updateArray_t::size_type localSwitch2DualFbIndLowAStateIdx = 39;
        static constexpr updateArray_t::size_type localSwitch2DualFbIndLowBStateIdx = 40;
        static constexpr updateArray_t::size_type localSwitch2DualLowFaultIdx = 41;
        static constexpr updateArray_t::size_type localSwitch2DualControlLowOutIdx = 42;
        static constexpr updateArray_t::size_type localSwitch2FbIndSteadyAIdx = 43;
        static constexpr updateArray_t::size_type localSwitch2FbIndSteadyBIdx = 44;
        static constexpr updateArray_t::size_type localSwitch2FbIndSteadyAStateIdx = 45;
        static constexpr updateArray_t::size_type localSwitch2FbIndSteadyBStateIdx = 46;
        static constexpr updateArray_t::size_type localSwitch2SteadyFaultIdx = 47;
        static constexpr updateArray_t::size_type localSwitch2ControlSteadyOutIdx = 48;
        static constexpr updateArray_t::size_type localSwitch2ConfigFaultIdx = 49;
        static constexpr updateArray_t::size_type localSwitch2FaultSummaryIdx = 50;
        static constexpr updateArray_t::size_type localSwitch2HardwareControlInIdx = 51;
        static constexpr updateArray_t::size_type localSwitch2SystemGuardSignalIdx = 52;
        static constexpr updateArray_t::size_type localSwitch2RelayControlOutIdx = 53;
        static constexpr updateArray_t::size_type localSwitch2StateMachineStateIdx = 54;
        static constexpr updateArray_t::size_type localSwitch2RelayControlStringIdx = 55;
        static constexpr updateArray_t::size_type localSwitch2DualSingleConfig = 56;

        configManagerConnection& configIf;
        fpgaInterfaceConnection fpgaIf;
        statusUpdateInterface updateIf;
        updateArray_t statusUpdateData;
        rf_bands_t bands;
        bands_t curBand;
        std::size_t curOff;
        switchFunctionMap_t switchFunctionMap;
        switchFunctionRegValMap_t switchTxRegValMap;
        switchFunctionRegValMap_t switchTxAltRegValMap;
        switchRegValNameMap_t switchRegValNameMap;

        const std::string switchManagerModName;
        const std::string systemControllerModName;

    public:
        switchManagerController(zmq::context_t& zmqCtx, configManagerConnection& config);
        void initializeData();

        [[nodiscard]] std::optional<mmData_t> getBandRaw() noexcept;
        [[nodiscard]] std::optional<std::pair<bands_t, std::size_t>> getBand() noexcept;
        bool setBand(const bands_t b, const std::size_t off) noexcept;
        [[nodiscard]] std::optional<source_select_t> getBandSourceSelect() noexcept;
        bool setBandSourceSelect(const source_select_t src) noexcept;
        [[nodiscard]] std::optional<switchStatus_t> getSwitchStatus(const switch_t& s) noexcept;
        [[nodiscard]] std::optional<signal_t> getSoftwareControl(const switch_t& s) noexcept;
        bool setSoftwareControl(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchEnabled(const switch_t& s) noexcept;
        bool setSwitchEnabled(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<source_select_t> getSwitchControlSource(const switch_t& s) noexcept;
        bool setSwitchControlSource(const switch_t& s, const source_select_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchDualEnable(const switch_t& s) noexcept;
        bool setSwitchDualEnable(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchInputInvert(const switch_t& s) noexcept;
        bool setSwitchInputInvert(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchFeedbackInvert(const switch_t& s) noexcept;
        bool setSwitchFeedbackInvert(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchSteadyFeedbackAEnable(const switch_t& s) noexcept;
        bool setSwitchSteadyFeedbackAEnable(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchSteadyFeedbackBEnable(const switch_t& s) noexcept;
        bool setSwitchSteadyFeedbackBEnable(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchDualHighFeedbackAEnable(const switch_t& s) noexcept;
        bool setSwitchDualHighFeedbackAEnable(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchDualHighFeedbackBEnable(const switch_t& s) noexcept;
        bool setSwitchDualHighFeedbackBEnable(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchDualLowFeedbackAEnable(const switch_t& s) noexcept;
        bool setSwitchDualLowFeedbackAEnable(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<signal_t> getSwitchDualLowFeedbackBEnable(const switch_t& s) noexcept;
        bool setSwitchDualLowFeedbackBEnable(const switch_t& s, const signal_t& val) noexcept;
        [[nodiscard]] std::optional<time_duration_t> getSwitchGuardTime(const switch_t& s) noexcept;
        bool setSwitchGuardTime(const switch_t& s, const time_duration_t& val) noexcept;
        [[nodiscard]] std::optional<time_duration_t> getSwitchDualTime(const switch_t& s) noexcept;
        bool setSwitchDualTime(const switch_t& s, const time_duration_t& val) noexcept;
        [[nodiscard]] std::optional<time_duration_t> getSwitchDischargeTime(const switch_t& s) noexcept;
        bool setSwitchDischargeTime(const switch_t& s, const time_duration_t& val) noexcept;
        [[nodiscard]] std::optional<std::string> getTrSwitchState() noexcept;
        bool setTrSwitchTransmit() noexcept;
        bool setTrSwitchReceive() noexcept;
        [[nodiscard]] std::optional<std::string> getDummySwitchState() noexcept;
        bool setDummySwitchTransmit() noexcept;
        bool setDummySwitchLoad() noexcept;

        void pollSwitchState();

    private:
        void setupOffsetTables();
        [[nodiscard]] constexpr reg_t getSwitchBase(const switch_t& s) noexcept;
        [[nodiscard]] constexpr reg_t getSwitchAddr(const switch_t& s, const reg_t& off) noexcept;
        [[nodiscard]] std::optional<switch_t> getTrSwitch() noexcept;
        [[nodiscard]] std::optional<switch_t> getDummySwitch() noexcept;
    };
}

#endif
