#include "rfManagerController.h"

#include <hardwareInterface/hardwareInterface.h>
#include <helpers/jsonUpdateHelpers.h>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      zmqCtx  The zmq context
 * @param      conf    The conf
 */
rfManagerController::rfManagerController(zmq::context_t& zmqCtx, configManagerConnection& conf):
    configIf{conf},
    updateIf{zmqCtx},
    fpgaIf{zmqCtx},
    mgcTop{mgc_top_max<mmData_t>},
    openLoopGain{0.0},
    vvaRange{0.0},
    agcRange{0.0},
    agcRef{0.0},
    powerFloorValue{0.0},
    peakFastTimeConst{0.0},
    peakSlowTimeConst{0.0},
    peakTauTimeConst{100.0},
    bitEnableFunctionGeneratorConfig{false},
    bitMaxPulseWidth{std::chrono::duration_cast<time_duration_t>(std::chrono::microseconds{100})},
    bitMaxDutyCycle{5.0},
    bitOscPresentConfig{false},
    channelFloor{
        {{port_t::FWD,   detector_t::RMS},  {200}},
        {{port_t::FWD,   detector_t::ENV},  {200}},
        {{port_t::FWD,   detector_t::PEAK}, {200}},
        {{port_t::REF,   detector_t::RMS},  {200}},
        {{port_t::REF,   detector_t::ENV},  {200}},
        {{port_t::REF,   detector_t::PEAK}, {200}},
        {{port_t::IN,    detector_t::RMS},  {200}},
        {{port_t::IN,    detector_t::ENV},  {200}},
        {{port_t::IN,    detector_t::PEAK}, {200}},
        {{port_t::UNBAL, detector_t::RMS},  {200}},
        {{port_t::UNBAL, detector_t::ENV},  {200}},
        {{port_t::UNBAL, detector_t::PEAK}, {200}},
    },
    channelCf{
        {port_t::FWD, {}},
        {port_t::REF, {}},
        {port_t::IN, {}},
        {port_t::UNBAL, {}},
    },
    powerReportingDataSet{
        updateIfData_t{"RF_POWER_LEVEL_FORWARD_RMS", "Forward RMS", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_FORWARD_ENV", "Forward Envelope", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_FORWARD_PEAK_SNAP", "Forward Peak Snapshot", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_FORWARD_PEAK", "Forward Peak Hold", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_REVERSE_RMS", "Reverse RMS", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_REVERSE_ENV", "Reverse Envelope", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_REVERSE_PEAK_SNAP", "Reverse Peak Snapshot", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_REVERSE_PEAK", "Reverse Peak Hold", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_INPUT_RMS", "Input RMS", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_INPUT_ENV", "Input Envelope", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_INPUT_PEAK_SNAP", "Input Peak Snapshot", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_INPUT_PEAK", "Input Peak Hold", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_UNBALANCED_RMS", "Unbalanced RMS", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_UNBALANCED_ENV", "Unbalanced Envelope", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_UNBALANCED_PEAK_SNAP", "Unbalanced Peak Snapshot", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_LEVEL_UNBALANCED_PEAK", "Unbalanced Peak Hold", "dBm", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_VSWR_RMS", "RMS VSWR", "", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_VSWR_ENV", "Envelope VSWR", "", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_VSWR_PEAK", "Peak VSWR", "", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_FORWARD_PAPR", "Forward Peak to Average Power Ratio", "dB", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_REVERSE_PAPR", "Reverse Peak to Average Power Ratio", "dB", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_INPUT_PAPR", "Input Peak to Average Power Ratio", "dB", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_UNBALANCED_PAPR", "Unbalanced Peak to Average Power Ratio", "dB", defaultPowerTimeout},
        updateIfData_t{"RF_POWER_COMPRESSION", "Compression", "", defaultPowerTimeout},
    },
    slowDataSet{
        updateIfData_t{"RF_INP_SWITCH", "Input Switch State", "", defaultFpgaTimeout},
        updateIfData_t{"RF_INP_SELECT", "Input Select State", "", defaultFpgaTimeout},
        updateIfData_t{"RF_PHASE_CURRENT", "Current Phasor Angle", "Deg", defaultFpgaTimeout},
        updateIfData_t{"RF_ALC_SETPOINT", "ALC Setpoint", "dBm", defaultFpgaTimeout},
        updateIfData_t{"RF_AGC_SETPOINT", "AGC Setpoint", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_MGC_VVA_SETPOINT", "MGC VVA Setpoint", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_MGC_GAIN_SETPOINT", "MGC Gain Setpoint", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_MGC_PERCENTAGE_SETPOINT", "MGC Percentage Setpoint", "%", defaultFpgaTimeout},
        updateIfData_t{"RF_ONLINE_STATE", "Online State", "", defaultFpgaTimeout},
        updateIfData_t{"RF_SAFE_STATE", "Safe State", "", defaultFpgaTimeout},
        updateIfData_t{"RF_OPERATIONAL_MODE", "Operational Mode", "", defaultFpgaTimeout},
        updateIfData_t{"RF_REGULATION_DET", "Power Regulation Detector", "", defaultFpgaTimeout},
        updateIfData_t{"RF_INPUT_PRESENT", "Input Power Present", "", defaultFpgaTimeout},
        updateIfData_t{"RF_FORWARD_PRESENT", "Transmitting RF Power", "", defaultFpgaTimeout},
        updateIfData_t{"RF_DAC_OUT_DATA", "Value written to the DAC", "", defaultFpgaTimeout},
        updateIfData_t{"RF_BLANKING_IN", "Blanking Input", "", defaultFpgaTimeout},
        updateIfData_t{"RF_BLANKING_POL", "Blanking Polarity", "", defaultFpgaTimeout},
        updateIfData_t{"RF_BLANKING_STATE", "Blanking State", "", defaultFpgaTimeout},
        updateIfData_t{"RF_SHUTDOWN_IN", "Shutdown Input", "", defaultFpgaTimeout},
        updateIfData_t{"RF_SHUTDOWN_POL", "Shutdown Polarity", "", defaultFpgaTimeout},
        updateIfData_t{"RF_SHUTDOWN_STATE", "Shutdown State", "", defaultFpgaTimeout},
        updateIfData_t{"RF_POWER_HARDWARE_FAULT_BACKOFF", "Hardware Power Fault Backoff", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_POWER_SOFTWARE_FAULT_BACKOFF", "Software Power Fault Backoff", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_POWER_HARDWARE_AVAILABLE", "Available Hardware Power", "dBm", defaultFpgaTimeout},
        updateIfData_t{"RF_POWER_NOMINAL", "Nominal Power", "dBm", defaultFpgaTimeout},
        updateIfData_t{"RF_POWER_AVAILABLE", "Total Available Power", "dBm", defaultFpgaTimeout},
        updateIfData_t{"RF_MODULATION_PROFILE_BACKOFF", "Modulation Profile Backoff", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_MGC_TOP", "Maximum VVA setting", "", defaultFpgaTimeout},
        updateIfData_t{"RF_OPEN_LOOP_GAIN", "Open Loop Gain of the HPA", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_VVA_RANGE", "VVA Range", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_ALC_RANGE", "ALC Range", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_AGC_RANGE", "AGC Range", "dB", defaultFpgaTimeout},
        updateIfData_t{"RF_AGC_REF", "AGC Reference Power", "dBm", defaultFpgaTimeout},
        updateIfData_t{"BIT_FUNC_GEN_ENABLE_SELECT", "BIT Enabled", "", defaultFpgaTimeout},
        updateIfData_t{"BIT_RF_ENABLE_HIGH_TIME", "BIT RF High Time", "S", defaultFpgaTimeout},
        updateIfData_t{"BIT_RF_ENABLE_LOW_TIME", "BIT RF Low Time", "S", defaultFpgaTimeout},
        updateIfData_t{"BIT_RF_ENABLE_DUTY", "RF Enabled Duty Cycle", "%", defaultFpgaTimeout},
        updateIfData_t{"BIT_RF_GUARD_TIME", "RF Guard Time", "nS", defaultFpgaTimeout},
        updateIfData_t{"BIT_RF_MODULATION_ACTIVE_TIME", "RF Modulation Active Time", "S", defaultFpgaTimeout},
        updateIfData_t{"BIT_RF_MODULATION_INACTIVE_TIME", "RF Modulation Inactive Time", "S", defaultFpgaTimeout},
        updateIfData_t{"BIT_RF_MODULATION_DUTY", "RF Modulation Duty Cycle", "%", defaultFpgaTimeout},
        updateIfData_t{"BIT_RF_OVERALL_DUTY", "RF Overall Duty Cycle", "%", defaultFpgaTimeout},
        updateIfData_t{"BIT_OSC_PWM_HIGH_TIME", "BIT Oscillator PWM High Time", "S", defaultFpgaTimeout},
        updateIfData_t{"BIT_OSC_PWM_LOW_TIME", "BIT Oscillator PWM Low Time", "S", defaultFpgaTimeout},
        updateIfData_t{"RF_MANUAL_INPUT_SWITCH", "Manual Override for Input Switch", "S", defaultFpgaTimeout},
        updateIfData_t{"RF_RMS_TIMER_ANALOG_FILTERING", "ACTIVE = Analog, INACTIVE = Digital", "", defaultFpgaTimeout},
        updateIfData_t{"RF_RMS_FILT_INP_ANALOG_FILTERING", "ACTIVE = Analog, INACTIVE = Digital", "", defaultFpgaTimeout},
        updateIfData_t{"RF_RMS_FILT_FWD_ANALOG_FILTERING", "ACTIVE = Analog, INACTIVE = Digital", "", defaultFpgaTimeout},
        updateIfData_t{"RF_RMS_FILT_REV_ANALOG_FILTERING", "ACTIVE = Analog, INACTIVE = Digital", "", defaultFpgaTimeout},
        updateIfData_t{"RF_RMS_FILT_UNB_ANALOG_FILTERING", "ACTIVE = Analog, INACTIVE = Digital", "", defaultFpgaTimeout},
        updateIfData_t{"RF_DAC_DROOP_CORRECTION_LINE1_ENABLE", "Droop Correction Line 1 Enable Status", "", defaultFpgaTimeout},
        updateIfData_t{"RF_DAC_DROOP_CORRECTION_LINE2_ENABLE", "Droop Correction Line 2 Enable Status", "", defaultFpgaTimeout},
    },
    calibrationPortMapping{
        {{port_t::FWD,   detector_t::RMS},  {}},
        {{port_t::FWD,   detector_t::ENV},  {}},
        {{port_t::FWD,   detector_t::PEAK}, {}},
        {{port_t::REF,   detector_t::RMS},  {}},
        {{port_t::REF,   detector_t::ENV},  {}},
        {{port_t::REF,   detector_t::PEAK}, {}},
        {{port_t::IN,    detector_t::RMS},  {}},
        {{port_t::IN,    detector_t::ENV},  {}},
        {{port_t::IN,    detector_t::PEAK}, {}},
        {{port_t::UNBAL, detector_t::RMS},  {}},
        {{port_t::UNBAL, detector_t::ENV},  {}},
        {{port_t::UNBAL, detector_t::PEAK}, {}},
    },
    powerConvToggleMap{
        {{port_t::FWD,   detector_t::RMS},  ((0x001) << 16)},
        {{port_t::FWD,   detector_t::ENV},  ((0x002) << 16)},
        {{port_t::REF,   detector_t::RMS},  ((0x004) << 16)},
        {{port_t::REF,   detector_t::ENV},  ((0x008) << 16)},
        {{port_t::IN,    detector_t::RMS},  ((0x010) << 16)},
        {{port_t::IN,    detector_t::ENV},  ((0x020) << 16)},
        {{port_t::UNBAL, detector_t::RMS},  ((0x040) << 16)},
        {{port_t::UNBAL, detector_t::ENV},  ((0x080) << 16)},
        {{port_t::FWD,   detector_t::PEAK}, ((0x100) << 16)},
        {{port_t::REF,   detector_t::PEAK}, ((0x200) << 16)},
        {{port_t::IN,    detector_t::PEAK}, ((0x400) << 16)},
        {{port_t::UNBAL, detector_t::PEAK}, ((0x800) << 16)},
    },
    rfManagerModName{"RF"},
    adcModName{"ADC"},
    dacModName{"DAC"},
    peakDetModName{"PEAK_DET"},
    moveToPsModName{"MOVE_TO_PS"},
    pidModName{"PID"},
    pwrConvModName{"PWR_CONV"},
    spiModName{"SPI"},
    psuModName{"PSU"},
    bitModName{"BIT"},
    sysCtrlModName{"SYS_CTRL"},
    channelMap{
        {{port_t::FWD,   detector_t::RMS},  0},
        {{port_t::FWD,   detector_t::ENV},  1},
        {{port_t::REF,   detector_t::RMS},  2},
        {{port_t::REF,   detector_t::ENV},  3},
        {{port_t::IN,    detector_t::RMS},  4},
        {{port_t::IN,    detector_t::ENV},  5},
        {{port_t::UNBAL, detector_t::RMS},  6},
        {{port_t::UNBAL, detector_t::ENV},  7},
        {{port_t::FWD,   detector_t::PEAK}, 8},
        {{port_t::REF,   detector_t::PEAK}, 9},
        {{port_t::IN,    detector_t::PEAK}, 10},
        {{port_t::UNBAL, detector_t::PEAK}, 11},
    },
    gpioKeyArray{
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_1", sysCtrlRegMap::GPO1_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_2", sysCtrlRegMap::GPO2_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_3", sysCtrlRegMap::GPO3_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_4", sysCtrlRegMap::GPO4_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_5", sysCtrlRegMap::GPO5_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_6", sysCtrlRegMap::GPO6_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_7", sysCtrlRegMap::GPO7_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_8", sysCtrlRegMap::GPO8_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_9", sysCtrlRegMap::GPO9_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_10", sysCtrlRegMap::GPO10_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPO_11", sysCtrlRegMap::GPO11_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_1", sysCtrlRegMap::GPI1_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_2", sysCtrlRegMap::GPI2_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_3", sysCtrlRegMap::GPI3_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_4", sysCtrlRegMap::GPI4_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_5", sysCtrlRegMap::GPI5_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_6", sysCtrlRegMap::GPI6_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_7", sysCtrlRegMap::GPI7_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_8", sysCtrlRegMap::GPI8_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_9", sysCtrlRegMap::GPI9_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_10", sysCtrlRegMap::GPI10_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_11", sysCtrlRegMap::GPI11_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_12", sysCtrlRegMap::GPI12_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_13", sysCtrlRegMap::GPI13_CTRL},
        gpioKeyPair_t{"SYS_CTRL_XBAR_GPI_14", sysCtrlRegMap::GPI14_CTRL}
    },
    rfBandSettings{}
{}

/**
 * @brief      Initializes the data.
 */
void rfManagerController::initializeData()
{
    logger::verbose(__FILE__, __FUNCTION__, "Initializing Data...");

    try
    {
        using namespace helpers::types;

        mgcTop = std::min(mgc_top_max<mmData_t>,
            configIf.getParam<mmData_t>("RF_VVA_SCALE_UPPER_LIMIT").value_or(mgc_top_max<mmData_t>));
        openLoopGain = configIf.getParam<dbm_t>("RF_OPEN_LOOP_GAIN").value_or(67.0);
        vvaRange = configIf.getParam<dbm_t>("RF_VVA_RANGE").value_or(31.5);
        agcRange = configIf.getParam<dbm_t>("RF_AGC_RANGE").value_or(15.0);
        agcRef = configIf.getParam<dbm_t>("RF_AGC_REF").value_or(0.0);
        powerFloorValue = configIf.getParam<dbm_t>("RF_MOD_POWER_FLOOR_VALUE").value_or(0.0);
        peakFastTimeConst = decay_rate_t{
            configIf.getParam<decay_rate_t::rep>("RF_PEAK_FAST_TIME_CONSTANT_MS").value_or(500.0)};
        peakSlowTimeConst = decay_rate_t{
            configIf.getParam<decay_rate_t::rep>("RF_PEAK_SLOW_TIME_CONSTANT_MS").value_or(0.1)};

        bitEnableFunctionGeneratorConfig = configIf.getParam<bool>("BIT_ENABLE_FUNCTION_GENERATOR").value_or(false);
        bitMaxPulseWidth = time_duration_t{
            configIf.getParam<time_duration_t::rep>("BIT_FUNC_GEN_MAX_PULSE_WIDTH").value_or(0.0001)};
        bitMaxDutyCycle = configIf.getParam<percent_t>("BIT_FUNC_GEN_MAX_DUTY_PERCENTAGE").value_or(5.0);
        bitOscPresentConfig = configIf.getParam<bool>("BIT_OSC_PRESENT").value_or(false);
    }
    catch(const std::exception& e)
    {
        logger::critical(__FILE__, __FUNCTION__, e.what());
        std::terminate();
    }
}

/*
    Initialize the FPGA.  FPGA initialization is conditional, based on the state of the "FPGA Initialized" register.

    FPGA initialization occurs only when the "FPGA Initialized" register indicates initialization has not been completed.

    FPGA initialization is skipped, when:
        - FPGA initialization has already been performed as part of Fast-Boot sequence
        - RFManager has been killed & restarted, or has crashed and is automatically restarted
*/
/**
 * @brief      Initializes the fpga.
 */
void rfManagerController::initializeFPGA()
{
    using namespace helpers::types;

    logger::verbose(__FILE__, __FUNCTION__, "Starting...");

    // TODO - DM - Check hardware "FPGA Initialization Complete" register
    bool alreadyInitialized{false};

    if(!alreadyInitialized) {
        setSafeState(safe_state_t::SAFE);

        setCfFwd(configIf.getParamSet<dbm_t>("RF_CF_FWD").value_or(0));
        setCfRev(configIf.getParamSet<dbm_t>("RF_CF_REV").value_or(0));
        setCfInp(configIf.getParamSet<dbm_t>("RF_CF_INP").value_or(0));
        setCfUnb(configIf.getParamSet<dbm_t>("RF_CF_UNB").value_or(0));
        setBlankingPol(strToPol(configIf.getParamSet<std::string>("RF_DEFAULT_BLANKING_POL").value_or("")));
        setShutdownPol(strToPol(configIf.getParamSet<std::string>("RF_DEFAULT_SHUTDOWN_POL").value_or("")));
        setBlankingOutputPol(strToPol(configIf.getParamSet<std::string>("RF_DEFAULT_BLANKING_OUT_POL").value_or("")));
        setInputSelect(input_select_t::INPUT);
        //setNominalPwr(configIf.getParamSet<dbm_t>("RF_NOMINAL_POWER").value_or(0));
        setInpPresThold(configIf.getParamSet<dbm_t>("RF_INP_PRES_THOLD").value_or(-18.0));
        setInpPresHystInc(configIf.getParamSet<mmData_t>("RF_INP_PRES_HYST_INC").value_or(5));
        setInpPresHystDec(configIf.getParamSet<mmData_t>("RF_INP_PRES_HYST_DEC").value_or(2));
        setInpPresHystThold(configIf.getParamSet<mmData_t>("RF_INP_PRES_HYST_THOLD").value_or(10));
        setFwdPresThold(configIf.getParamSet<dbm_t>("RF_FWD_PRES_THOLD").value_or(47.0));
        setFwdPresHystInc(configIf.getParamSet<mmData_t>("RF_FWD_PRES_HYST_INC").value_or(5));
        setFwdPresHystDec(configIf.getParamSet<mmData_t>("RF_FWD_PRES_HYST_DEC").value_or(2));
        setFwdPresHystThold(configIf.getParamSet<mmData_t>("RF_FWD_PRES_HYST_THOLD").value_or(10));
        setRfInputSwitchSeqMuteTime(regMapToTime(
            configIf.getParamSet<mmData_t>("RF_INPUT_SWITCH_SEQUENCING_MUTE_TIME").value_or(400000000)));

        // Clear safe state
        setSafeState(safe_state_t::RUNNING);
    }

    logger::verbose(__FILE__, __FUNCTION__, "Complete");
}



/**
 * @brief      Initializes the power regulation.
 */
void rfManagerController::initializePowerRegulation()
{
    static const std::array<std::tuple<std::string, reg_t, double>, 12> pidConfig{
        std::make_tuple("RF_PID_KP_LG_P", pidRegMap::pid_kp_lg_p, 0.68359375),
        std::make_tuple("RF_PID_KI_LG_P", pidRegMap::pid_ki_lg_p, 0.1953125),
        std::make_tuple("RF_PID_KD_LG_P", pidRegMap::pid_kd_lg_p, 0.1220703125),
        std::make_tuple("RF_PID_KP_LG_N", pidRegMap::pid_kp_lg_n, 0.68359375),
        std::make_tuple("RF_PID_KI_LG_N", pidRegMap::pid_ki_lg_n, 0.1953125),
        std::make_tuple("RF_PID_KD_LG_N", pidRegMap::pid_kd_lg_n, 0.1220703125),
        std::make_tuple("RF_PID_KP_SM_P", pidRegMap::pid_kp_sm_p, 0.244140625),
        std::make_tuple("RF_PID_KI_SM_P", pidRegMap::pid_ki_sm_p, 0.048828125),
        std::make_tuple("RF_PID_KD_SM_P", pidRegMap::pid_kd_sm_p, 0.01220703125),
        std::make_tuple("RF_PID_KP_SM_N", pidRegMap::pid_kp_sm_n, 0.244140625),
        std::make_tuple("RF_PID_KI_SM_N", pidRegMap::pid_ki_sm_n, 0.0048828125),
        std::make_tuple("RF_PID_KD_SM_N", pidRegMap::pid_kd_sm_n, 0.1220703125)
    };

    const auto fractionalBits{getPidFractionalBits().value_or(4096)};

    const auto setGain = [&](const std::tuple<std::string, reg_t, double>& dataSet) {
        const auto gainF{configIf.getParamSet<double>(std::get<0>(dataSet)).value_or(std::get<2>(dataSet))};
        const mmData_t gainVal{static_cast<mmData_t>(gainF * std::pow(2, fractionalBits))};
        fpgaIf.write(pidModName, std::get<1>(dataSet), gainVal);
    };

    std::for_each(std::begin(pidConfig), std::end(pidConfig), setGain);

    setAlcDynamicRange(configIf.getParamSet<dbm_t>("RF_ALC_RANGE").value_or(15.0));
    setRegulationDet(helpers::types::strToDetector(
        configIf.getParamSet<std::string>("RF_DEFAULT_DETECTOR").value_or("PEAK")));
    setPidClockEnable(time_duration_t{configIf.getParamSet<time_duration_t::rep>("RF_PID_CLK_PERIOD").value_or(0.001)});
    setAlcOvershootDrop(configIf.getParamSet<mmData_t>("RF_PID_ALC_OVERSHOOT_DROP").value_or(64));
    setAlcOvershootIntegratorDrop(configIf.getParamSet<mmData_t>("RF_PID_ALC_OVERSHOOT_INTG_DROP").value_or(32));
    setAlcOvershootThreshold(configIf.getParamSet<dbm_t>("RF_PID_ALC_OVERSHOOT_THRESHOLD").value_or(0.5));
    setAlcOvershootDet(helpers::types::strToDetector(configIf.getParamSet<std::string>("RF_PID_ALC_OVERSHOOT_DETECTOR").value_or("PEAK")));
    fpgaIf.write(pidModName, pidRegMap::pid_dac_low_limit, configIf.getParamSet<mmData_t>("RF_PID_DAC_LOW_LMT").value_or(0x00000000));
    fpgaIf.write(pidModName, pidRegMap::pid_dac_high_limit, configIf.getParamSet<mmData_t>("RF_PID_DAC_HIGH_LMT").value_or(0x00002A00));
    fpgaIf.write(pidModName, pidRegMap::pid_sum_fault_delay, configIf.getParamSet<mmData_t>("RF_PID_SUM_FAULT_DELAY").value_or(0x000186a0));
    fpgaIf.write(pidModName, pidRegMap::pid_rf_mode, configIf.getParamSet<mmData_t>("RF_PID_RF_MODE").value_or(0x00000002));
    fpgaIf.write(pidModName, pidRegMap::pid_osck, configIf.getParamSet<mmData_t>("RF_PID_OSCK").value_or(0x00000014));
    fpgaIf.write(pidModName, pidRegMap::alc_hw_mode, configIf.getParamSet<mmData_t>("RF_ALC_HW_MODE").value_or(0x00000000));
    fpgaIf.write(pidModName, pidRegMap::pid_suspend_level, configIf.getParamSet<mmData_t>("RF_PID_SUSPEND_LEVEL").value_or(0x000000c8));
    fpgaIf.write(pidModName, pidRegMap::pid_blanking_start_dly, configIf.getParamSet<mmData_t>("RF_PID_BLANKING_START_DLY").value_or(0x00000064));
    fpgaIf.write(pidModName, pidRegMap::pid_blanking_suspend_dly, configIf.getParamSet<mmData_t>("RF_PID_BLANKING_SUSPEND_DLY").value_or(0X493E0));
    fpgaIf.write(pidModName, pidRegMap::pid_blanking_timeout_dly, configIf.getParamSet<mmData_t>("RF_PID_BLANKING_TIMEOUT_DLY").value_or(0x2FAF080));
    fpgaIf.write(pidModName, pidRegMap::pid_inp_suspend_dly, configIf.getParamSet<mmData_t>("RF_PID_INP_SUSPEND_DLY").value_or(0x002625a0));
    fpgaIf.write(pidModName, pidRegMap::pid_inp_timeout_dly, configIf.getParamSet<mmData_t>("RF_PID_INP_TIMEOUT_DLY").value_or(0x004c4b40));
    fpgaIf.write(pidModName, pidRegMap::pid_inp_chan_margin_high, helpers::types::dbmToRegVal(configIf.getParamSet<dbm_t>("RF_PID_INP_CHAN_MARGIN_HIGH").value_or(5)));
    fpgaIf.write(pidModName, pidRegMap::pid_inp_chan_margin_low, helpers::types::dbmToRegVal(configIf.getParamSet<dbm_t>("RF_PID_INP_CHAN_MARGIN_LOW").value_or(-8)));
}

/**
 * @brief      Initializes the droop correction.
 */
void rfManagerController::initializeDroopCorrection()
{
    // Line 1 Configuration
    const bool line1Enable = configIf.getParamSet<bool>("RF_DAC_DROOP_CORRECTION_LINE1_ENABLE").value_or(false);
    
    if (line1Enable) {
        const mmData_t dropCount = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE1_DROP_COUNT").value_or(0);
        const mmData_t clockCount = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE1_CLOCK_COUNT").value_or(1);
        const mmData_t riseCount = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE1_RISE_COUNT").value_or(1);
        
        logger::info(__FILE__, __FUNCTION__, 
            std::string("Droop Line 1: DeltaDrop=") + std::to_string(dropCount) +
            std::string(", DeltaClocks=") + std::to_string(clockCount) +
            std::string(", DeltaRise=") + std::to_string(riseCount));
        
        fpgaIf.write(dacModName, dacRegMap::initDropCnt_1, dropCount);
        fpgaIf.write(dacModName, dacRegMap::riseHoriz_1, clockCount);
        fpgaIf.write(dacModName, dacRegMap::riseVert_1, riseCount);
    } else {
        logger::info(__FILE__, __FUNCTION__, "Disabling Droop Line 1 Correction");
        fpgaIf.write(dacModName, dacRegMap::initDropCnt_1, 0);
        fpgaIf.write(dacModName, dacRegMap::riseHoriz_1, 1);
        fpgaIf.write(dacModName, dacRegMap::riseVert_1, 1);
    }
    
    // Line 2 Configuration
    const bool line2Enable = configIf.getParamSet<bool>("RF_DAC_DROOP_CORRECTION_LINE2_ENABLE").value_or(false);
    
    if (line2Enable) {
        const mmData_t dropCount2 = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE2_DROP_COUNT").value_or(0);
        const mmData_t clockCount2 = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE2_CLOCK_COUNT").value_or(1);
        const mmData_t riseCount2 = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE2_RISE_COUNT").value_or(1);
        
        logger::info(__FILE__, __FUNCTION__, 
            std::string("Droop Line 2: DeltaDrop=") + std::to_string(dropCount2) +
            std::string(", DeltaClocks=") + std::to_string(clockCount2) +
            std::string(", DeltaRise=") + std::to_string(riseCount2));
        
        fpgaIf.write(dacModName, dacRegMap::initDropCnt_2, dropCount2);
        fpgaIf.write(dacModName, dacRegMap::riseHoriz_2, clockCount2);
        fpgaIf.write(dacModName, dacRegMap::riseVert_2, riseCount2);
    } else {
        logger::info(__FILE__, __FUNCTION__, "Disabling Droop Line 2 Correction");
        fpgaIf.write(dacModName, dacRegMap::initDropCnt_2, 0);
        fpgaIf.write(dacModName, dacRegMap::riseHoriz_2, 1);
        fpgaIf.write(dacModName, dacRegMap::riseVert_2, 1);
    }
}

/**
 * @brief      Sets droop correction line 1 enable state.
 *
 * @param[in]  enable  True to enable, false to disable
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setDroopCorrectionLine1Enable(bool enable) noexcept
{
    logger::info(__FILE__, __FUNCTION__, 
        std::string("Setting Droop Correction Line 1 to ") + (enable ? "ENABLED" : "DISABLED"));
        
    if (enable) {
        // When enabling, read current configuration values and apply them
        const mmData_t dropCount = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE1_DROP_COUNT").value_or(1);
        const mmData_t clockCount = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE1_CLOCK_COUNT").value_or(1);
        const mmData_t riseCount = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE1_RISE_COUNT").value_or(1);
        
        return fpgaIf.write(dacModName, dacRegMap::initDropCnt_1, dropCount) &&
               fpgaIf.write(dacModName, dacRegMap::riseHoriz_1, clockCount) &&
               fpgaIf.write(dacModName, dacRegMap::riseVert_1, riseCount);
    } else {
        // When disabling, set drop count to 0 and others to 1
        return fpgaIf.write(dacModName, dacRegMap::initDropCnt_1, 0) &&
               fpgaIf.write(dacModName, dacRegMap::riseHoriz_1, 1) &&
               fpgaIf.write(dacModName, dacRegMap::riseVert_1, 1);
    }
}

/**
 * @brief      Sets droop correction line 2 enable state.
 *
 * @param[in]  enable  True to enable, false to disable
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setDroopCorrectionLine2Enable(bool enable) noexcept
{
    logger::info(__FILE__, __FUNCTION__, 
        std::string("Setting Droop Correction Line 2 to ") + (enable ? "ENABLED" : "DISABLED"));
        
    if (enable) {
        // When enabling, read current configuration values and apply them
        const mmData_t dropCount = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE2_DROP_COUNT").value_or(1);
        const mmData_t clockCount = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE2_CLOCK_COUNT").value_or(1);
        const mmData_t riseCount = configIf.getParamSet<mmData_t>("RF_DAC_DROOP_CORRECTION_LINE2_RISE_COUNT").value_or(1);
        
        return fpgaIf.write(dacModName, dacRegMap::initDropCnt_2, dropCount) &&
               fpgaIf.write(dacModName, dacRegMap::riseHoriz_2, clockCount) &&
               fpgaIf.write(dacModName, dacRegMap::riseVert_2, riseCount);
    } else {
        // When disabling, set drop count to 0 and others to 1
        return fpgaIf.write(dacModName, dacRegMap::initDropCnt_2, 0) &&
               fpgaIf.write(dacModName, dacRegMap::riseHoriz_2, 1) &&
               fpgaIf.write(dacModName, dacRegMap::riseVert_2, 1);
    }
}

/**
 * @brief      Initializes the power conversion filtering.
 */
void rfManagerController::initializePowerConversionFiltering()
{
    using namespace helpers::types;
    using std::string_literals::operator""s;

    setRmsTimerEnable(boolToSig(configIf.getParamSet<bool>("RF_RMS_TMR_EN").value_or(true)));
    setRmsTimerPowerThold(configIf.getParamSet<dbm_t>("RF_RMS_TMR_PWR_THRESHOLD").value_or(47.0));
    setRmsTimerCountTop(configIf.getParamSet<mmData_t>("RF_RMS_TMR_CNT_TOP").value_or(10000000));
    setRmsTimerCountThold(configIf.getParamSet<mmData_t>("RF_RMS_TMR_CNT_THRESHOLD").value_or(20));

    for(const auto& port: {port_t::IN, port_t::FWD, port_t::REF, port_t::UNBAL})
    {
        const auto portStr{helpers::types::portToStr(port)};

        setRmsFilterEnable(port, boolToSig(configIf.getParamSet<bool>(
            "RF_"s + portStr + "RMS_FILTER_ENABLE").value_or(true)));
        setRmsFilterForceCw(port, boolToSig(configIf.getParamSet<bool>(
            "RF_"s + portStr + "RMS_FILTER_FORCE_CW").value_or(false)));
        setRmsFilterForcePulse(port, boolToSig(configIf.getParamSet<bool>(
            "RF_"s + portStr + "RMS_FILTER_FORCE_PULSE").value_or(false)));
        setRmsFilterOffset(port, configIf.getParamSet<dbm_t>("RF_"s + portStr + "RMS_FILTER_OFFSET").value_or(70.0));
    }
}

/**
 * @brief      Initializes the power conversion tables.
 */
void rfManagerController::initializePowerConversionTables()
{
    using namespace helpers::types;
    using std::string_literals::operator""s;

    logger::verbose(__FILE__, __FUNCTION__, "Initializing power conversion tables...");

    const auto calTableSize{configIf.getParamSet<std::size_t>("RF_MOD_PWR_CONV_TABLE_SIZE").value_or(0)};
    const auto powerConvBaseAddr{strToNum<reg_t>(
        configIf.getParamSet<std::string>("FHI_MOD_PWR_CONV_BASE_HEX_ADDRESS").value_or(""), 16).value_or(0x80000000)};
    const auto powerFloorValueRaw{dbmToDbmRaw(powerFloorValue)};

    hardwareInterface rawFpgaIf(powerConvBaseAddr, 0x10000);

    setSafeState(safe_state_t::SAFE);

    for(auto& [chan, vec]: calibrationPortMapping)
    {
        strToCalVec(vec, configIf.getParamSet<std::string>(std::string("RF_CAL_CURVE_") +
            chanToChanStr(chan)).value_or(""));

        if(vec.size() > 2)
        {
            channelFloor.at(chan) = calDataPointToDbmF(vec.at(1));
        }
    }

    // Set the tables for all entries in the Calibration Port Mapping
    for(auto& [chan, data]: calibrationPortMapping)
    {
        logger::verbose(__FILE__, __FUNCTION__, "Calibrating Channel: "s + chanToChanStr(chan));

        if(data.empty())
        {
            logger::warn(__FILE__, __FUNCTION__, "Missing Calibration Data!!!");
            continue;
        }

        // Set the MM IF to the appropriate channel setting
        rawFpgaIf.write(powerConvRegMap::ctrl, channelMap.at(chan));

        dbm_t slope{0};
        dbm_t intercept{0};
        bool top{false};
        dbm_raw_t topValue{};
        calVec_t::iterator prevIt = data.end();

        // Loop over index range setting all values into the table
        for(std::size_t idx = 0; idx < calTableSize; ++idx)
        {
            dbm_raw_t regData{0};

            // Bottom of the table
            if(idx < calDataPointToRawAdc(data.front()))
            {
                regData = powerFloorValueRaw;
            }
            // Top of the table
            else if(top)
            {
                regData = topValue;
            }
            // Peice-wise Linearization in the middle of the table
            else
            {
                // Price is Right type find
                calVec_t::iterator curIt = data.begin();
                for(;curIt != std::prev(data.end()); curIt++)
                {
                    if(idx < calDataPointToRawAdc(*std::next(curIt)))
                    {
                        break;
                    }
                }

                if((curIt == std::prev(data.end())) || (calDataPointToRawAdc(*curIt) >= (calTableSize - 1)))
                {
                    top = true;
                    topValue = calDataPointToDbm(data.back());
                    regData = topValue;
                }
                else
                {
                    // Check if the current index are in the same peice as the previous calculation
                    //      If not, we need to update the slope and intercept for the new segment
                    if(curIt != prevIt)
                    {
                        // Formula Definitions:
                        // X1 = Current Index ADC   static_cast<dbm_t>(calDataPointToRawAdc(*curIt))
                        // Y1 = Current Index dBm   calDataPointToDbmF(*curIt)
                        // X2 = Next Index ADC      static_cast<dbm_t>(calDataPointToRawAdc(*std::next(curIt)))
                        // Y2 = Next Index dBm      calDataPointToDbmF(*std::next(curIt))
                        //
                        // slope = (Y2 - Y1) / (X2 - X1)
                        // intercept = Y1 - (slope * X1)

                        const dbm_t X1 = static_cast<dbm_t>(calDataPointToRawAdc(*curIt));
                        const dbm_t Y1 = calDataPointToDbmF(*curIt);
                        const dbm_t X2 = static_cast<dbm_t>(calDataPointToRawAdc(*std::next(curIt)));
                        const dbm_t Y2 = calDataPointToDbmF(*std::next(curIt));

                        slope = (Y2 - Y1) / (X2 - X1);
                        intercept = Y1 - (slope * X1);

                        logger::debug(__FILE__, __FUNCTION__,
                            "Calculating Slope/Intercept Values for Index "s + std::to_string(idx) +
                            ": X1 = "s + std::to_string(X1) + ", Y1 = "s + std::to_string(Y1) +
                            "| X2 = "s + std::to_string(X2) + ", Y2 = "s + std::to_string(Y2) +
                            "| Slope = "s + std::to_string(slope) + ", Intercept = "s + std::to_string(intercept));

                        prevIt = curIt;
                    }

                    regData = dbmToDbmRaw((static_cast<dbm_t>(idx) * slope) + intercept);
                }
            }

            // Last dummy check to make sure we don't overrun the table
            if(regData >= static_cast<dbm_raw_t>(calTableSize))
            {
                regData = static_cast<dbm_raw_t>(calTableSize - 1);
            }

            if((idx % 32) == 0)
            {
                logger::debug(__FILE__, __FUNCTION__, "Cal Table Generation for Channel "s +
                    helpers::types::chanToChanStr(chan) + " ("s + std::to_string(idx) + "): "s +
                    std::to_string(regData));
            }

            rawFpgaIf.write((powerConvRegMap::tableBase + (idx << 2)), static_cast<mmData_t>(regData));
        }
    }

    setSafeState(safe_state_t::RUNNING);
}

/**
 * @brief      Initializes the dsa step tables.
 */
void rfManagerController::initializeDsaStepTables()
{
    const auto inphaseDsa{helpers::types::dataTokenizer<mmData_t>(
        configIf.getParamSet<std::string>("RF_PHASOR_INPHASE").value_or(""))};
    const auto quadratureDsa{helpers::types::dataTokenizer<mmData_t>(
        configIf.getParamSet<std::string>("RF_PHASOR_QUADRATURE").value_or(""))};

    if((inphaseDsa.size() == spiRegMap::dsa_lut_size) && (quadratureDsa.size() == spiRegMap::dsa_lut_size))
    {
        for(reg_t idx = 0; idx < spiRegMap::dsa_lut_size; ++idx)
        {
            fpgaIf.write(spiModName, (spiRegMap::dsa_lut_0 + (idx << 2)), inphaseDsa.at(idx));
            fpgaIf.write(spiModName, (spiRegMap::dsa_lut_90 + (idx << 2)), quadratureDsa.at(idx));
        }
    }

    setPhaseDwell(std::chrono::nanoseconds{configIf.getParamSet<std::uint64_t>("RF_PHASE_DWELL_NS").value_or(0)});

    // TODO -- MB -- Remove this bandaid after fix in FPGA is implemented
    // setPhaseTgt(static_cast<phase_t>(configIf.getParamSet<std::uint64_t>("RF_PHASE_TARGET").value_or(0)));
    auto phaseTarget{static_cast<phase_t>(configIf.getParamSet<std::uint64_t>("RF_PHASE_TARGET").value_or(0))};
    setPhaseTgt(phaseTarget - 1);
    setPhaseTgt(phaseTarget);
}

/**
 * @brief      Initializes the gpio crossbar.
 */
void rfManagerController::initializeGpioCrossbar()
{
    const auto getFunctionPairs = [&](){
        const auto funcStr{helpers::types::dataTokenizer<std::string>(
            configIf.getParamSet<std::string>("SYS_CTRL_XBAR_FUNCTIONS").value_or(""))};
        std::vector<std::pair<std::string, mmData_t>> result;

        std::transform(std::begin(funcStr), std::end(funcStr), std::back_inserter(result), [](const auto& str){
            const auto splitStr{helpers::types::dataTokenizer<std::string>(str, "=")};
            if(splitStr.size() != 2)
            {
                return std::make_pair(std::string(""), mmData_t{0});
            }

            return std::make_pair(splitStr.at(0), helpers::types::strToNum<mmData_t>(splitStr.at(1)).value_or(mmData_t{0}));
        });

        return result;
    };

    const auto keyToVal = [](const auto& pairs, const auto& key){
        mmData_t result{0};

        for(const auto& [k, v]: pairs)
        {
            if(k == key)
            {
                result = v;
                break;
            }
        }

        return result;
    };

    const auto pairs{getFunctionPairs()};

    for(const auto& [key, addr]: gpioKeyArray)
    {
        if(const auto confVal{configIf.getParamSet<std::string>(key)}; confVal.has_value())
        {
            fpgaIf.write(sysCtrlModName, addr, keyToVal(pairs, confVal.value()));
        }
    }
}

/**
 * @brief      Gets the rf state.
 *
 * @return     The rf state if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::online_state_t> rfManagerController::getRfState() noexcept
{
    if(auto psuState{fpgaIf.read(psuModName, psuRegMap::ctrl)}; psuState.has_value())
    {
        if(auto rfState{fpgaIf.read(rfManagerModName, rfManagerRegMap::rfEnable)}; rfState.has_value())
        {
            return helpers::types::regValToOnlineState(rfState.value(), psuState.value());
        }
    }

    return std::nullopt;
}

/**
 * @brief      Sets the rf state.
 *
 * @param[in]  online  The online
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setRfState(const online_state_t online) noexcept
{
    return fpgaIf.write(rfManagerModName, rfManagerRegMap::rfEnable,
        helpers::types::onlineStateToRfRegVal(online));
}

/**
 * @brief      Gets the input select.
 *
 * @return     The input select if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::input_select_t> rfManagerController::getInputSelect() noexcept
{
    if(auto data{fpgaIf.read(rfManagerModName, rfManagerRegMap::inpSel)}; data.has_value())
    {
        return helpers::types::regValToInpSel(data.value());
    }

    return std::nullopt;
}

std::optional<bool> rfManagerController::getDroopCorrectionLine1Enable() noexcept
{
    if(auto data{fpgaIf.read(dacModName, dacRegMap::initDropCnt_1)}; data.has_value())
    {
        // Droop correction is enabled if drop count is non-zero
        return data.value() != 0;
    }

    return std::nullopt;
}

std::optional<bool> rfManagerController::getDroopCorrectionLine2Enable() noexcept
{
    if(auto data{fpgaIf.read(dacModName, dacRegMap::initDropCnt_2)}; data.has_value())
    {
        // Droop correction is enabled if drop count is non-zero
        return data.value() != 0;
    }

    return std::nullopt;
}



/**
 * @brief      Sets the input select.
 *
 * @param[in]  sw    The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setInputSelect(const input_select_t sw) noexcept
{
    return fpgaIf.write(rfManagerModName, rfManagerRegMap::inpSel,
        helpers::types::inpSelToRegVal(sw));
}

/**
 * @brief      Gets the input switch.
 *
 * @return     The input switch if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::input_switch_t> rfManagerController::getInputSwitch() noexcept
{
    if(auto data{fpgaIf.read(rfManagerModName, rfManagerRegMap::inputSwitchControl)}; data.has_value())
    {
        return helpers::types::regValToInpSwitch(data.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the manual input switch.
 *
 * @return     The manual input switch.
 */
std::optional<rfManagerController::manual_input_switch_t> rfManagerController::getManualInputSwitch() noexcept
{
    if(auto data{fpgaIf.read(rfManagerModName, rfManagerRegMap::manualInputSwitchCtrl)}; data.has_value())
    {
        return helpers::types::regValToManualInpSwitch(data.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the manual input switch.
 *
 * @param[in]  state  The state
 *
 * @return     True if successful, False otherwise
 */
bool rfManagerController::setManualInputSwitch(const manual_input_switch_t state) noexcept
{
    return fpgaIf.write(rfManagerModName, rfManagerRegMap::manualInputSwitchCtrl,
        helpers::types::manualInpSwitchToRegVal(state));
}

/**
 * @brief      Gets the rf input switch sequence mute time.
 *
 * @return     The rf input switch sequence mute time if successful, std::nullopt otherwise
 */
std::optional<rfManagerController::fpga_clock_rate_t>
rfManagerController::getRfInputSwitchSeqMuteTime() noexcept
{
    if(auto resp{fpgaIf.read(rfManagerModName, rfManagerRegMap::rfInpSwitchSeqMuteTime)}; resp.has_value())
    {
        return helpers::types::regMapToTime(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the rf input switch sequence mute time.
 *
 * @param[in]  time  The time
 *
 * @return     True if successful, False otherwise
 */
bool rfManagerController::setRfInputSwitchSeqMuteTime(const fpga_clock_rate_t& time) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting RF Input Switch Sequencing Mute Time to " +
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count()) + " nanoSeconds...");

    return fpgaIf.write(rfManagerModName, rfManagerRegMap::rfInpSwitchSeqMuteTime, helpers::types::timeToFpgaRegMap(time));
}

/**
 * @brief      Gets the safe state.
 *
 * @return     The safe state if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::safe_state_t> rfManagerController::getSafeState() noexcept
{
    if(auto data{fpgaIf.read(rfManagerModName, rfManagerRegMap::safeState)}; data.has_value())
    {
        return helpers::types::regValToSafeState(data.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the safe state.
 *
 * @param[in]  safe  The safe
 *
 * @return     True if sucessful, False otherwise
 */
bool rfManagerController::setSafeState(const safe_state_t safe) noexcept
{
    return fpgaIf.write(rfManagerModName, rfManagerRegMap::safeState,
        helpers::types::safeStateToRegVal(safe));
}


/**
 * @brief      Gets the Current RF Band Index.
 *
 * @return     The phase target if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::rf_band_index_t> rfManagerController::getRfBandIndex() noexcept
{
    if(auto resp{fpgaIf.read(spiModName, spiRegMap::dsa_tgt)}; resp.has_value())
    {
        return resp.value();
    }

    return std::nullopt;
}

/**
 * @brief      Sets the selected Band Index and loads it's Parameters.
 *
 * @param[in]  bandIndex    The new value
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setRfBandIndex(const rf_band_index_t bandIndex) noexcept
{
    bool    result = false;
    logger::verbose(__FILE__, __FUNCTION__, "Setting Band Index to " + std::to_string(bandIndex) + "...");

    if(bandIndex > (_MAX_BAND_INDEX - 1))
    {
        logger::debug(__FILE__, __FUNCTION__, "Setting Band Index Failed!!!, Out of Bounds");
        return false;
    }

    // Code sequencing to set any required parameters


    return result;
}


/**
 * @brief      Gets the gain mode.
 *
 * @return     The gain mode if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::gain_mode_t> rfManagerController::getGainMode() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::mode_sel)}; resp.has_value())
    {
        return helpers::types::regValToGainMode(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the gain mode.
 *
 * @param[in]  mode  The mode
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setGainMode(const gain_mode_t mode) noexcept
{
    return fpgaIf.write(pidModName, pidRegMap::mode_sel, helpers::types::gainModeToRegVal(mode));
}

/**
 * @brief      Gets the mgc setpoint percentage.
 *
 * @return     The mgc setpoint percentage if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::percent_t> rfManagerController::getMgcSetpointPercentage() noexcept
{
    if(auto resp{getMgcSetpointVva()}; resp.has_value())
    {
        return static_cast<percent_t>(resp.value()) / mgcTop * 100.0;
    }

    return std::nullopt;
}

/**
 * @brief      Sets the mgc setpoint percentage.
 *
 * @param[in]  percentage  The percentage
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setMgcSetpointPercentage(const percent_t percentage) noexcept
{
    if((percentage < 0.0) || (percentage > 100.0))
    {
        return false;
    }

    return setMgcSetpointVva(static_cast<mmData_t>(percentage / 100.0 * mgcTop));
}

// TODO -- MB -- The MGC Gain Get Setpoint needs work....
/**
 * @brief      Gets the mgc setpoint gain.
 *
 * @return     The mgc setpoint gain if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getMgcSetpointGain() noexcept
{
    if(auto resp{getMgcSetpointVva()}; resp.has_value())
    {
        return (helpers::types::regValToDbm(resp.value()) + openLoopGain);
    }

    return std::nullopt;
}

// TODO -- MB -- The MGC Gain Set Setpoint needs work....
/**
 * @brief      Sets the mgc setpoint gain.
 *
 * @param[in]  gain  The gain
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setMgcSetpointGain(const dbm_t gain) noexcept
{
    if((gain > openLoopGain) || (gain < (openLoopGain - vvaRange)))
    {
        return false;
    }

    // return setMgcSetpointVva(gain - openLoopGain);
    return setMgcSetpointVva(0);
}

/**
 * @brief      Gets the mgc setpoint vva.
 *
 * @return     The mgc setpoint vva if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getMgcSetpointVva() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::mgc_sp)}; resp.has_value())
    {
        return resp.value();
    }

    return std::nullopt;
}

/**
 * @brief      Sets the mgc setpoint vva.
 *
 * @param[in]  val   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setMgcSetpointVva(const mmData_t val) noexcept
{
    if(val > mgc_top_max<mmData_t>)
    {
        return false;
    }

    return fpgaIf.write(pidModName, pidRegMap::mgc_sp, val);
}

/**
 * @brief      Gets the alc setpoint.
 *
 * @return     The alc setpoint if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getAlcSetpoint() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::alc_sp)}; resp.has_value())
    {
        return helpers::types::dbmRawToDbm(static_cast<dbm_raw_t>(resp.value()));
    }

    return std::nullopt;
}

/**
 * @brief      Sets the alc setpoint.
 *
 * @param[in]  power  The power
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setAlcSetpoint(const dbm_t power) noexcept
{
    if(auto pnom{getNominalPwr()}; pnom.has_value())
    {
        if(auto alcRange{getAlcDynamicRange()}; alcRange.has_value())
        {
            if((power <= pnom.value()) && (power >= (pnom.value() - alcRange.value())))
            {
                return fpgaIf.write(pidModName, pidRegMap::alc_sp, helpers::types::dbmToRegVal(power));
            }
        }
    }

    return false;
}

/**
 * @brief      Gets the agc setpoint.
 *
 * @return     The agc setpoint if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getAgcSetpoint() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::agc_sp)}; resp.has_value())
    {
        return helpers::types::dbmRawToDbm(static_cast<dbm_raw_t>(resp.value()));
    }

    return std::nullopt;
}

/**
 * @brief      Sets the agc setpoint.
 *
 * @param[in]  gain  The gain
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setAgcSetpoint(const dbm_t gain) noexcept
{
    if(auto pnom{getNominalPwr()}; pnom.has_value())
    {
        if(const auto dbmFull{pnom.value() - agcRef}; ((gain <= dbmFull) && (gain >= (dbmFull - agcRange))))
        {
            return fpgaIf.write(pidModName, pidRegMap::agc_sp, helpers::types::dbmToRegVal(gain));
        }
    }

    return false;
}

/**
 * @brief      Gets the blanking input.
 *
 * @return     The blanking input if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getBlankingInput() noexcept
{
    if(auto resp{fpgaIf.read(rfManagerModName, rfManagerRegMap::blankingInExtStat)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the blanking state.
 *
 * @return     The blanking state if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getBlankingState() noexcept
{
    if(auto resp{fpgaIf.read(rfManagerModName, rfManagerRegMap::blankStat)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the blanking pol.
 *
 * @return     The blanking pol if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::polarity_t> rfManagerController::getBlankingPol() noexcept
{
    if(auto resp{fpgaIf.read(rfManagerModName, rfManagerRegMap::blankPol)}; resp.has_value())
    {
        return helpers::types::regValToPol(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the blanking pol.
 *
 * @param[in]  pol   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setBlankingPol(const polarity_t pol) noexcept
{
    return fpgaIf.write(rfManagerModName, rfManagerRegMap::blankPol, helpers::types::polToRegVal(pol));
}

/**
 * @brief      Gets the shutdown input.
 *
 * @return     The shutdown input if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getShutdownInput() noexcept
{
    if(auto resp{fpgaIf.read(rfManagerModName, rfManagerRegMap::shutdownInExtStat)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the shutdown state.
 *
 * @return     The shutdown state if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getShutdownState() noexcept
{
    if(auto resp{fpgaIf.read(rfManagerModName, rfManagerRegMap::shutdownStat)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the shutdown pol.
 *
 * @return     The shutdown pol if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::polarity_t> rfManagerController::getShutdownPol() noexcept
{
    if(auto resp{fpgaIf.read(rfManagerModName, rfManagerRegMap::shutdownPol)}; resp.has_value())
    {
        return helpers::types::regValToPol(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the shutdown pol.
 *
 * @param[in]  pol   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setShutdownPol(const polarity_t pol) noexcept
{
    return fpgaIf.write(rfManagerModName, rfManagerRegMap::shutdownPol, helpers::types::polToRegVal(pol));
}

/**
 * @brief      Gets the blanking output pol.
 *
 * @return     The shutdown pol if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::polarity_t> rfManagerController::getBlankingOutputPol() noexcept
{
    if(auto resp{fpgaIf.read(rfManagerModName, rfManagerRegMap::blankOutputPol)}; resp.has_value())
    {
        return helpers::types::regValToPol(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the blanking output pol.
 *
 * @param[in]  pol   The new value
 *
 * @return     True if Sucessful, False otherwise.
 */
bool rfManagerController::setBlankingOutputPol(const polarity_t pol) noexcept
{
    return fpgaIf.write(rfManagerModName, rfManagerRegMap::blankOutputPol, helpers::types::polToRegVal(pol));
}

/**
 * @brief      Gets the phase current.
 *
 * @return     The phase current if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::phase_t> rfManagerController::getPhaseCur() noexcept
{
    if(auto resp{fpgaIf.read(spiModName, spiRegMap::dsa_cur)}; resp.has_value())
    {
        return resp.value();
    }

    return std::nullopt;
}

/**
 * @brief      Gets the phase target.
 *
 * @return     The phase target if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::phase_t> rfManagerController::getPhaseTgt() noexcept
{
    if(auto resp{fpgaIf.read(spiModName, spiRegMap::dsa_tgt)}; resp.has_value())
    {
        return resp.value();
    }

    return std::nullopt;
}

/**
 * @brief      Sets the phase target.
 *
 * @param[in]  ph    The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setPhaseTgt(const phase_t ph) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting Phase Target to " + std::to_string(ph) + "...");

    if(ph > (spiRegMap::dsa_lut_size - 1))
    {
        logger::debug(__FILE__, __FUNCTION__, "Setting Phase Target Failed!!!");
        return false;
    }

    return fpgaIf.write(spiModName, spiRegMap::dsa_tgt, ph);
}
/**
 * @brief      Gets the phase dwell.
 *
 * @return     The phase dwell if sucessful, std::nullopt otherwise.
 */
std::optional<std::chrono::nanoseconds> rfManagerController::getPhaseDwell() noexcept
{
    if(auto resp{fpgaIf.read(spiModName, spiRegMap::dsa_tgt)}; resp.has_value())
    {
        return std::chrono::nanoseconds{resp.value() * 10};
    }

    return std::nullopt;
}

/**
 * @brief      Sets the phase dwell.
 *
 * @param[in]  dwell  The dwell
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setPhaseDwell(const std::chrono::nanoseconds dwell) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting Phase Dwell time to " +
        std::to_string(dwell.count()) + " nanoSeconds...");

    return fpgaIf.write(spiModName, spiRegMap::dsa_dwell, (static_cast<mmData_t>(dwell.count()) / 10));
}

/**
 * @brief      Gets the cf forward.
 *
 * @return     The cf forward if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getCfFwd() noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConvRegMap::fwdOffset)}; resp.has_value())
    {
        return helpers::types::regValToDbm(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the cf forward.
 *
 * @param[in]  pwr   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setCfFwd(const dbm_t pwr) noexcept
{
    if(fpgaIf.write(pwrConvModName, powerConvRegMap::fwdOffset, helpers::types::dbmToRegVal(pwr)))
    {
        channelCf.at(port_t::FWD) = pwr;
        return true;
    }

    return false;
}

/**
 * @brief      Gets the cf reverse.
 *
 * @return     The cf reverse if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getCfRev() noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConvRegMap::revOffset)}; resp.has_value())
    {
        return helpers::types::regValToDbm(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the cf reverse.
 *
 * @param[in]  pwr   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setCfRev(const dbm_t pwr) noexcept
{
    if(fpgaIf.write(pwrConvModName, powerConvRegMap::revOffset, helpers::types::dbmToRegVal(pwr)))
    {
        channelCf.at(port_t::REF) = pwr;
        return true;
    }

    return false;
}

/**
 * @brief      Gets the cf inp.
 *
 * @return     The cf inp if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getCfInp() noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConvRegMap::inpOffset)}; resp.has_value())
    {
        return helpers::types::regValToDbm(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the cf inp.
 *
 * @param[in]  pwr   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setCfInp(const dbm_t pwr) noexcept
{
    if(fpgaIf.write(pwrConvModName, powerConvRegMap::inpOffset, helpers::types::dbmToRegVal(pwr)))
    {
        channelCf.at(port_t::IN) = pwr;
        return true;
    }

    return false;
}

/**
 * @brief      Gets the cf unb.
 *
 * @return     The cf unb if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getCfUnb() noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConvRegMap::unbOffset)}; resp.has_value())
    {
        return helpers::types::regValToDbm(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the cf unb.
 *
 * @param[in]  pwr   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setCfUnb(const dbm_t pwr) noexcept
{
    if(fpgaIf.write(pwrConvModName, powerConvRegMap::unbOffset, helpers::types::dbmToRegVal(pwr)))
    {
        channelCf.at(port_t::UNBAL) = pwr;
        return true;
    }

    return false;
}

/**
 * @brief      Gets the nominal power.
 *
 * @return     The nominal power if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getNominalPwr() noexcept
{
    if(auto resp{fpgaIf.read(rfManagerModName, rfManagerRegMap::pwrNominal)}; resp.has_value())
    {
        return helpers::types::regValToDbm(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the nominal power.
 *
 * @param[in]  pwr   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
//bool rfManagerController::setNominalPwr(const dbm_t pwr) noexcept
//{
//    return fpgaIf.write(rfManagerModName, rfManagerRegMap::pwrNominal, helpers::types::dbmToRegVal(pwr));
//}

/**
 * @brief      Gets the regulation det.
 *
 * @return     The regulation det if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::detector_t> rfManagerController::getRegulationDet() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::chan_sel)}; resp.has_value())
    {
        return helpers::types::regValToDetector(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the regulation det.
 *
 * @param[in]  det   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setRegulationDet(const detector_t det) noexcept
{
    bool result{true};

    if(detector_t::PEAK_FAST == det)
    {
        result = result && setPeakDecayRate(peakFastTimeConst);
    }
    else if(detector_t::PEAK_SLOW == det)
    {
        result = result && setPeakDecayRate(peakSlowTimeConst);
    }
    else
    {
        result = result && setPeakDecayRate(peakTauTimeConst);
    }

    return (result && fpgaIf.write(pidModName, pidRegMap::chan_sel, helpers::types::detectorToRegVal(det)));
}

/**
 * @brief      Gets the alc dynamic range.
 *
 * @return     The alc dynamic range.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getAlcDynamicRange() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::pid_alc_dyn_rng)}; resp.has_value())
    {
        return helpers::types::regValToDbm(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the alc dynamic range.
 *
 * @param[in]  range  The range
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setAlcDynamicRange(const dbm_t range) noexcept
{
    return fpgaIf.write(pidModName, pidRegMap::pid_alc_dyn_rng, helpers::types::dbmToRegVal(range));
}

/**
 * @brief      Gets the pid clock enable.
 *
 * @return     The pid clock enable.
 */
std::optional<rfManagerController::time_duration_t> rfManagerController::getPidClockEnable() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::intg_update_fact)}; resp.has_value())
    {
        return helpers::types::regMapToTime(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the pid clock enable.
 *
 * @param[in]  rate  The rate
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setPidClockEnable(const time_duration_t& rate) noexcept
{
    return fpgaIf.write(pidModName, pidRegMap::intg_update_fact, helpers::types::timeToFpgaRegMap(rate));
}

/**
 * @brief      Gets the alc overshoot drop.
 *
 * @return     The alc overshoot drop.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getAlcOvershootDrop() noexcept
{
    return fpgaIf.read(pidModName, pidRegMap::ovr_shoot_cooldwn_drop);
}

/**
 * @brief      Sets the alc overshoot drop.
 *
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setAlcOvershootDrop(const mmData_t val) noexcept
{
    return fpgaIf.write(pidModName, pidRegMap::ovr_shoot_cooldwn_drop, val);
}

/**
 * @brief      Gets the alc overshoot integrator drop.
 *
 * @return     The alc overshoot integrator drop.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getAlcOvershootIntegratorDrop() noexcept
{
    return fpgaIf.read(pidModName, pidRegMap::ovr_shoot_intg_drop);
}

/**
 * @brief      Sets the alc overshoot integrator drop.
 *
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setAlcOvershootIntegratorDrop(const mmData_t val) noexcept
{
    return fpgaIf.write(pidModName, pidRegMap::ovr_shoot_intg_drop, val);
}

/**
 * @brief      Gets the alc overshoot threshold.
 *
 * @return     The alc overshoot threshold.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getAlcOvershootThreshold() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::ovr_shoot_thold)}; resp.has_value())
    {
        return helpers::types::regValToDbm(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the alc overshoot threshoold.
 *
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setAlcOvershootThreshold(const dbm_t val) noexcept
{
    return fpgaIf.write(pidModName, pidRegMap::ovr_shoot_thold, helpers::types::dbmToRegVal(val));
}

/**
 * @brief      Gets the alc overshoot det.
 *
 * @return     The alc overshoot det.
 */
std::optional<rfManagerController::detector_t> rfManagerController::getAlcOvershootDet() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::ovr_shoot_src)}; resp.has_value())
    {
        return helpers::types::regValToDetector(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the alc overshoot det.
 *
 * @param[in]  det   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setAlcOvershootDet(const detector_t det) noexcept
{
    return fpgaIf.write(pidModName, pidRegMap::ovr_shoot_src, helpers::types::detectorToRegVal(det));
}

/**
 * @brief      Gets the pid fractional bits.
 *
 * @return     The pid fractional bits.
 */
std::optional<std::size_t> rfManagerController::getPidFractionalBits() noexcept
{
    if(auto resp{fpgaIf.read(pidModName, pidRegMap::pid_frac_bit_size_val)}; resp.has_value())
    {
        return std::size_t{resp.value()};
    }

    return std::nullopt;
}

/**
 * @brief      Gets the tau decay rate.
 *
 * @return     The tau decay rate.
 */
rfManagerController::decay_rate_t rfManagerController::getTauDecayRate() noexcept
{
    return peakTauTimeConst;
}

/**
 * @brief      Sets the tau decay rate.
 *
 * @param[in]  tau   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setTauDecayRate(const decay_rate_t tau) noexcept
{
    if(tau >= decay_rate_t{0})
    {
        peakTauTimeConst = tau;
        return true;
    }

    return false;
}

/**
 * @brief      Gets the inp pres.
 *
 * @return     The inp pres if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getInpPres() noexcept
{
    if(auto resp{fpgaIf.read(moveToPsModName, dmpRegMap::inp_pres)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the inp pres thold.
 *
 * @return     The inp pres thold if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getInpPresThold() noexcept
{
    if(auto resp{fpgaIf.read(moveToPsModName, dmpRegMap::inp_pres_thold)}; resp.has_value())
    {
        return helpers::types::regValToDbm(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the inp pres thold.
 *
 * @param[in]  thold  The thold
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setInpPresThold(const dbm_t thold) noexcept
{
    return fpgaIf.write(moveToPsModName, dmpRegMap::inp_pres_thold, helpers::types::dbmToRegVal(thold));
}

/**
 * @brief      Gets the inp pres hyst increment.
 *
 * @return     The inp pres hyst increment if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getInpPresHystInc() noexcept
{
    return fpgaIf.read(moveToPsModName, dmpRegMap::inp_pres_hyst_inc);
}

/**
 * @brief      Sets the inp pres hyst increment.
 *
 * @param[in]  val   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setInpPresHystInc(const mmData_t val) noexcept
{
    return fpgaIf.write(moveToPsModName, dmpRegMap::inp_pres_hyst_inc, val);
}

/**
 * @brief      Gets the inp pres hyst decrement.
 *
 * @return     The inp pres hyst decrement if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getInpPresHystDec() noexcept
{
    return fpgaIf.read(moveToPsModName, dmpRegMap::inp_pres_hyst_dec);
}

/**
 * @brief      Sets the inp pres hyst decrement.
 *
 * @param[in]  val   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setInpPresHystDec(const mmData_t val) noexcept
{
    return fpgaIf.write(moveToPsModName, dmpRegMap::inp_pres_hyst_dec, val);
}

/**
 * @brief      Gets the inp pres hyst thold.
 *
 * @return     The inp pres hyst thold if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getInpPresHystThold() noexcept
{
    return fpgaIf.read(moveToPsModName, dmpRegMap::inp_pres_hyst_thold);
}

/**
 * @brief      Sets the inp pres hyst thold.
 *
 * @param[in]  val   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setInpPresHystThold(const mmData_t val) noexcept
{
    return fpgaIf.write(moveToPsModName, dmpRegMap::inp_pres_hyst_thold, val);
}

/**
 * @brief      Gets the inp pres hyst accum.
 *
 * @return     The inp pres hyst accum if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getInpPresHystAccum() noexcept
{
    return fpgaIf.read(moveToPsModName, dmpRegMap::inp_pres_hyst_accum);
}

/**
 * @brief      Gets forward pres.
 *
 * @return     Forward pres if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getFwdPres() noexcept
{
    if(auto resp{fpgaIf.read(moveToPsModName, dmpRegMap::fwd_pres)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets forward pres thold.
 *
 * @return     Forward pres thold if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getFwdPresThold() noexcept
{
    if(auto resp{fpgaIf.read(moveToPsModName, dmpRegMap::fwd_pres_thold)}; resp.has_value())
    {
        return helpers::types::regValToDbm(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets forward pres thold.
 *
 * @param[in]  thold  The thold
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setFwdPresThold(const dbm_t thold) noexcept
{
    return fpgaIf.write(moveToPsModName, dmpRegMap::fwd_pres_thold, helpers::types::dbmToRegVal(thold));
}

/**
 * @brief      Gets forward pres hyst increment.
 *
 * @return     Forward pres hyst increment if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getFwdPresHystInc() noexcept
{
    return fpgaIf.read(moveToPsModName, dmpRegMap::fwd_pres_hyst_inc);
}

/**
 * @brief      Sets forward pres hyst increment.
 *
 * @param[in]  val   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setFwdPresHystInc(const mmData_t val) noexcept
{
    return fpgaIf.write(moveToPsModName, dmpRegMap::fwd_pres_hyst_inc, val);
}

/**
 * @brief      Gets forward pres hyst decrement.
 *
 * @return     Forward pres hyst decrement if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getFwdPresHystDec() noexcept
{
    return fpgaIf.read(moveToPsModName, dmpRegMap::fwd_pres_hyst_dec);
}

/**
 * @brief      Sets forward pres hyst decrement.
 *
 * @param[in]  val   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setFwdPresHystDec(const mmData_t val) noexcept
{
    return fpgaIf.write(moveToPsModName, dmpRegMap::fwd_pres_hyst_dec, val);
}

/**
 * @brief      Gets forward pres hyst thold.
 *
 * @return     Forward pres hyst thold if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getFwdPresHystThold() noexcept
{
    return fpgaIf.read(moveToPsModName, dmpRegMap::fwd_pres_hyst_thold);
}

/**
 * @brief      Sets forward pres hyst thold.
 *
 * @param[in]  val   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setFwdPresHystThold(const mmData_t val) noexcept
{
    return fpgaIf.write(moveToPsModName, dmpRegMap::fwd_pres_hyst_thold, val);
}

/**
 * @brief      Gets forward pres hyst accum.
 *
 * @return     Forward pres hyst accum if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getFwdPresHystAccum() noexcept
{
    return fpgaIf.read(moveToPsModName, dmpRegMap::fwd_pres_hyst_accum);
}

/**
 * @brief      Gets the peak decay rate.
 *
 * @return     The peak decay rate if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::decay_rate_t> rfManagerController::getPeakDecayRate() noexcept
{
    if(auto decayVal{fpgaIf.read(peakDetModName, peakDetRegMap::decay)}; decayVal.has_value())
    {
        return helpers::types::regMapToTime<decay_rate_t>(decayVal.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the peak decay rate.
 *
 * @param[in]  val   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setPeakDecayRate(const decay_rate_t& val) noexcept
{
    return fpgaIf.write(peakDetModName, peakDetRegMap::decay, helpers::types::decayTimeToFpgaRegMap(val));
}

/**
 * @brief      Gets the papr backoff.
 *
 * @return     The papr backoff if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getPaprBackoff() noexcept
{
    if(auto paprBackoff{fpgaIf.read(rfManagerModName, rfManagerRegMap::modProfBckoff)}; paprBackoff.has_value())
    {
        return helpers::types::regValToDbm(paprBackoff.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the papr backoff.
 *
 * @param[in]  val   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool rfManagerController::setPaprBackoff(const dbm_t val) noexcept
{
    return fpgaIf.write(rfManagerModName, rfManagerRegMap::modProfBckoff, helpers::types::dbmToRegVal(val));
}

/**
 * @brief      Gets the bit rf enable high time.
 *
 * @return     The bit rf enable high time.
 */
std::optional<rfManagerController::fpga_clock_rate_t> rfManagerController::getBitRfEnableHighTime() noexcept
{
    if(auto resp{fpgaIf.read(bitModName, bitRegMap::rf_enable_high_time)}; resp.has_value())
    {
        return helpers::types::regMapToTime(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the bit rf enable high time.
 *
 * @param[in]  time  The time
 *
 * @return     Whether the value was set
 */
bool rfManagerController::setBitRfEnableHighTime(const fpga_clock_rate_t& time) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting BIT RF Enable High Time to " +
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count()) + " nanoSeconds...");

    return fpgaIf.write(bitModName, bitRegMap::rf_enable_high_time, helpers::types::timeToFpgaRegMap(time));
}

/**
 * @brief      Gets the bit rf enable low time.
 *
 * @return     The bit rf enable low time.
 */
std::optional<rfManagerController::fpga_clock_rate_t> rfManagerController::getBitRfEnableLowTime() noexcept
{
    if(auto resp{fpgaIf.read(bitModName, bitRegMap::rf_enable_low_time)}; resp.has_value())
    {
        return helpers::types::regMapToTime(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the bit rf enable low time.
 *
 * @param[in]  time  The time
 *
 * @return     Whether the value was set
 */
bool rfManagerController::setBitRfEnableLowTime(const fpga_clock_rate_t& time) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting BIT RF Enable Low Time to " +
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count()) + " nanoSeconds...");

    return fpgaIf.write(bitModName, bitRegMap::rf_enable_low_time, helpers::types::timeToFpgaRegMap(time));
}

/**
 * @brief      Gets the bit rf guard time.
 *
 * @return     The bit rf guard time.
 */
std::optional<rfManagerController::fpga_clock_rate_t> rfManagerController::getBitRfGuardTime() noexcept
{
    if(auto resp{fpgaIf.read(bitModName, bitRegMap::rf_guard_time)}; resp.has_value())
    {
        return helpers::types::regMapToTime(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the bit rf guard time.
 *
 * @param[in]  time  The time
 *
 * @return     Whether the value was set
 */
bool rfManagerController::setBitRfGuardTime(const fpga_clock_rate_t& time) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting BIT RF Guard Time to " +
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count()) + " nanoSeconds...");

    return fpgaIf.write(bitModName, bitRegMap::rf_guard_time, helpers::types::timeToFpgaRegMap(time));
}

/**
 * @brief      Gets the bit rf modulation active time.
 *
 * @return     The bit rf modulation active time.
 */
std::optional<rfManagerController::fpga_clock_rate_t> rfManagerController::getBitRfModulationActiveTime() noexcept
{
    if(auto resp{fpgaIf.read(bitModName, bitRegMap::rf_modulation_active_time)}; resp.has_value())
    {
        return helpers::types::regMapToTime(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the bit rf modulation active time.
 *
 * @param[in]  time  The time
 *
 * @return     Whether the value was set
 */
bool rfManagerController::setBitRfModulationActiveTime(const fpga_clock_rate_t& time) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting BIT RF Modulation Active Time to " +
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count()) + " nanoSeconds...");

    return fpgaIf.write(bitModName, bitRegMap::rf_modulation_active_time, helpers::types::timeToFpgaRegMap(time));
}

/**
 * @brief      Gets the bit rf modulation inactive time.
 *
 * @return     The bit rf modulation inactive time.
 */
std::optional<rfManagerController::fpga_clock_rate_t> rfManagerController::getBitRfModulationInactiveTime() noexcept
{
    if(auto resp{fpgaIf.read(bitModName, bitRegMap::rf_modulation_inactive_time)}; resp.has_value())
    {
        return helpers::types::regMapToTime(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the bit rf modulation inactive time.
 *
 * @param[in]  time  The time
 *
 * @return     Whether the value was set
 */
bool rfManagerController::setBitRfModulationInactiveTime(const fpga_clock_rate_t& time) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting BIT RF Modulation Inactive Time to " +
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count()) + " nanoSeconds...");

    return fpgaIf.write(bitModName, bitRegMap::rf_modulation_inactive_time, helpers::types::timeToFpgaRegMap(time));
}

/**
 * @brief      Gets the bit function generate enable select.
 *
 * @return     The bit function generate enable select.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getBitFuncGenEnableSelect() noexcept
{
    if(auto resp{fpgaIf.read(bitModName, bitRegMap::func_gen_enable_select)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the bit function generate enable select.
 *
 * @param[in]  state  The state
 *
 * @return     Whether the value was set
 */
bool rfManagerController::setBitFuncGenEnableSelect(const signal_t state) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting BIT Function Generator to " +
        helpers::types::sigToStrActive(state) + "...");

    if(!bitEnableFunctionGeneratorConfig)
    {
        return false;
    }

    return fpgaIf.write(bitModName, bitRegMap::func_gen_enable_select, helpers::types::sigToRegVal(state));
}

/**
 * @brief      Gets the bit osc pwm high time.
 *
 * @return     The bit osc pwm high time.
 */
std::optional<rfManagerController::fpga_clock_rate_t> rfManagerController::getBitOscPwmHighTime() noexcept
{
    if(auto resp{fpgaIf.read(bitModName, bitRegMap::osc_pwm_high_time)}; resp.has_value())
    {
        return helpers::types::regMapToTime(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the bit osc pwm high time.
 *
 * @param[in]  time  The time
 *
 * @return     Whether the value was set
 */
bool rfManagerController::setBitOscPwmHighTime(const fpga_clock_rate_t& time) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting BIT Oscillator PWM High Time to " +
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count()) + " nanoSeconds...");

    return fpgaIf.write(bitModName, bitRegMap::osc_pwm_high_time, helpers::types::timeToFpgaRegMap(time));
}

/**
 * @brief      Gets the bit osc pwm low time.
 *
 * @return     The bit osc pwm low time.
 */
std::optional<rfManagerController::fpga_clock_rate_t> rfManagerController::getBitOscPwmLowTime() noexcept
{
    if(auto resp{fpgaIf.read(bitModName, bitRegMap::osc_pwm_low_time)}; resp.has_value())
    {
        return helpers::types::regMapToTime(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the bit osc pwm low time.
 *
 * @param[in]  time  The time
 *
 * @return     Whether the value was set
 */
bool rfManagerController::setBitOscPwmLowTime(const fpga_clock_rate_t& time) noexcept
{
    logger::verbose(__FILE__, __FUNCTION__, "Setting BIT Oscillator PWM Low Time to " +
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count()) + " nanoSeconds...");

    return fpgaIf.write(bitModName, bitRegMap::osc_pwm_low_time, helpers::types::timeToFpgaRegMap(time));
}

/**
 * @brief      Sets the BIT Pulse parameters.
 *
 * @param[in]  pulseWidth  The pulse width
 * @param[in]  dutyCycle   The duty cycle
 *
 * @return     If the parameters were set properly
 */
bool rfManagerController::setBitEnablePulseParams(const fpga_clock_rate_t& pulseWidth,
    const percent_t dutyCycle) noexcept
{
    if(!bitEnableFunctionGeneratorConfig || (pulseWidth > bitMaxPulseWidth) ||
        (dutyCycle < 0.0) || (dutyCycle > bitMaxDutyCycle))
    {
        return false;
    }

    // calculate high and low time values

    // high time = pulse width
    // low time = pw/dc * (100.0 - dc)
    //             = pw * (100.0 - dc) / dc

    bool result{false};
    const auto lowTimeNsCnt{
        (static_cast<percent_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(pulseWidth).count()) *
        ((100.0 - dutyCycle) / dutyCycle))
    };
    const auto lowTimeNs{
        std::chrono::duration<fpga_clock_rate_t::rep, std::nano>(static_cast<fpga_clock_rate_t::rep>(lowTimeNsCnt))
    };
    const auto lowTimeFpgaClk{std::chrono::duration_cast<fpga_clock_rate_t>(lowTimeNs)};

    // Get safe state to restore later
    if(auto curSafeState{getSafeState()}; curSafeState.has_value())
    {
        // Enter Safe State to change BIT parameters
        if(setSafeState(safe_state_t::SAFE))
        {
            // Set Values
            result = setBitRfEnableHighTime(pulseWidth) && setBitRfEnableLowTime(lowTimeFpgaClk);
        }

        // Reset Safe State
        setSafeState(curSafeState.value());
    }

    return result;
}

/**
 * @brief      Sets the BIT RF Modulation parameters.
 *
 * @param[in]  activeTime  The pulse width
 * @param[in]  dutyCycle   The duty cycle
 *
 * @return     If the parameters were set properly
 */
bool rfManagerController::setBitRfModulationParams(const fpga_clock_rate_t& activeTime,
    const percent_t dutyCycle) noexcept
{
    if(!bitEnableFunctionGeneratorConfig || (dutyCycle < 0.0) || (dutyCycle > 100.0))
    {
        return false;
    }

    // calculate high and low time values

    // high time = pulse width
    // low time = pw/dc * (100.0 - dc)
    //             = pw * (100.0 - dc) / dc

    bool result{false};
    const auto lowTimeNsCnt{
        (static_cast<percent_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(activeTime).count()) *
        ((100.0 - dutyCycle) / dutyCycle))
    };
    const auto lowTimeNs{
        std::chrono::duration<fpga_clock_rate_t::rep, std::nano>(static_cast<fpga_clock_rate_t::rep>(lowTimeNsCnt))
    };
    const auto lowTimeFpgaClk{std::chrono::duration_cast<fpga_clock_rate_t>(lowTimeNs)};

    // Get safe state to restore later
    if(auto curSafeState{getSafeState()}; curSafeState.has_value())
    {
        // Enter Safe State to change BIT parameters
        if(setSafeState(safe_state_t::SAFE))
        {
            // Set Values
            result = setBitRfModulationActiveTime(activeTime) && setBitRfModulationInactiveTime(lowTimeFpgaClk);
        }

        // Reset Safe State
        setSafeState(curSafeState.value());
    }

    return result;
}

/**
 * @brief      Sets the rf droop correction rise.
 *
 * @param[in]  line  The line
 * @param[in]  val   The new value
 *
 * @return     Whether the value was written correctly or not.
 */
bool rfManagerController::setRfDroopCorrectionRise(const droopLine_t line, const mmData_t val) noexcept
{
    using std::string_literals::operator""s;

    logger::verbose(__FILE__, __FUNCTION__, "Setting RF Droop Correction Rise for line "s +
        ((droopLine_t::LINE_1 == line)?("1"s):("2"s)) + " to "s + std::to_string(val));

    if(val > 0x3FFF)
    {
        logger::warn(__FILE__, __FUNCTION__, "Droop Correction for Rise is out of range!");
        return false;
    }

    reg_t offset{dacRegMap::droopRiseVertOffset};
    offset += ((droopLine_t::LINE_1 == line)?(dacRegMap::initDropCnt_1):(dacRegMap::initDropCnt_2));

    return fpgaIf.write(dacModName, offset, val);
}

/**
 * @brief      Sets the rf droop correction run.
 *
 * @param[in]  line  The line
 * @param[in]  val   The new value
 *
 * @return     Whether the value was written correctly or not.
 */
bool rfManagerController::setRfDroopCorrectionRun(const droopLine_t line, const mmData_t val) noexcept
{
    using std::string_literals::operator""s;

    logger::verbose(__FILE__, __FUNCTION__, "Setting RF Droop Correction Run for line "s +
        ((droopLine_t::LINE_1 == line)?("1"s):("2"s)) + " to "s + std::to_string(val));

    if(val > 1000000)
    {
        logger::warn(__FILE__, __FUNCTION__, "Droop Correction for Run is out of range!");
        return false;
    }

    reg_t offset{dacRegMap::droopRiseHorizOffset};
    offset += ((droopLine_t::LINE_1 == line)?(dacRegMap::droopBase_1):(dacRegMap::droopBase_2));

    return fpgaIf.write(dacModName, offset, val);
}

/**
 * @brief      Sets the rf droop correction drop.
 *
 * @param[in]  line  The line
 * @param[in]  val   The new value
 *
 * @return     Whether the value was written correctly or not.
 */
bool rfManagerController::setRfDroopCorrectionDrop(const droopLine_t line, const mmData_t val) noexcept
{
    using std::string_literals::operator""s;

    logger::verbose(__FILE__, __FUNCTION__, "Setting RF Droop Correction Drop Count for line "s +
        ((droopLine_t::LINE_1 == line)?("1"s):("2"s)) + " to "s + std::to_string(val));

    if(val > 0x3FFF)
    {
        logger::warn(__FILE__, __FUNCTION__, "Droop Correction for Drop is out of range!");
        return false;
    }

    reg_t offset{dacRegMap::droopDropCntOffset};
    offset += ((droopLine_t::LINE_1 == line)?(dacRegMap::droopBase_1):(dacRegMap::droopBase_2));

    return fpgaIf.write(dacModName, offset, val);
}

/**
 * @brief      Gets the power hardware fault backoff.
 *
 * @return     The power hardware fault backoff if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getPwrHwFaultBackoff() noexcept
{
    if(auto hwrFltBckoff{fpgaIf.read(rfManagerModName, rfManagerRegMap::pwrHwrFltBckoff)}; hwrFltBckoff.has_value())
    {
        return helpers::types::regValToDbm(hwrFltBckoff.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the power software fault backoff.
 *
 * @return     The power software fault backoff if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getPwrSwFaultBackoff() noexcept
{
    if(auto swrFltBckoff{fpgaIf.read(rfManagerModName, rfManagerRegMap::pwrSwrFltBckoff)}; swrFltBckoff.has_value())
    {
        return helpers::types::regValToDbm(swrFltBckoff.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the hardware available power.
 *
 * @return     The hardware available power if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getPwrHwAvail() noexcept
{
    if(auto pwrHwrAvail{fpgaIf.read(rfManagerModName, rfManagerRegMap::pwrHwrAvail)}; pwrHwrAvail.has_value())
    {
        return helpers::types::regValToDbm(pwrHwrAvail.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the power avail.
 *
 * @return     The power avail if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getPwrAvail() noexcept
{
    if(auto pwrAvail{fpgaIf.read(rfManagerModName, rfManagerRegMap::pwrAvail)}; pwrAvail.has_value())
    {
        return helpers::types::regValToDbm(pwrAvail.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the dac output.
 *
 * @return     The dac output if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getDacOutput() noexcept
{
    return fpgaIf.read(dacModName, dacRegMap::readData);
}

/**
 * @brief      Gets the rms timer status.
 *
 * @return     The rms timer status.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getRmsTimerStatus() noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConvRegMap::rmsTimerStatus)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the rms timer enable.
 *
 * @return     The rms timer enable.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getRmsTimerEnable() noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConvRegMap::rmsTimerEnable)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the rms timer enable.
 *
 * @param[in]  state  The state
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setRmsTimerEnable(const signal_t state) noexcept
{
    return fpgaIf.write(pwrConvModName, powerConvRegMap::rmsTimerEnable, helpers::types::sigToRegVal(state));
}

/**
 * @brief      Gets the rms timer power thold.
 *
 * @return     The rms timer power threshold if successful, std::nullopt otherwise.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getRmsTimerPowerThold() noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConvRegMap::rmsTimerPowerThreshold)}; resp.has_value())
    {
        return helpers::types::dbmRawToDbm(static_cast<dbm_raw_t>(resp.value()));
    }

    return std::nullopt;
}

/**
 * @brief      Sets the rms timer power thold.
 *
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise
 */
bool rfManagerController::setRmsTimerPowerThold(const dbm_t val) noexcept
{
    return fpgaIf.write(pwrConvModName, powerConvRegMap::rmsTimerPowerThreshold, helpers::types::dbmToRegVal(val));
}

/**
 * @brief      Gets the rms timer count top.
 *
 * @return     The rms timer count top if successful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getRmsTimerCountTop() noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConvRegMap::rmsTimerCountTop)}; resp.has_value())
    {
        return resp.value();
    }

    return std::nullopt;
}

/**
 * @brief      Sets the rms timer count top.
 *
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setRmsTimerCountTop(const mmData_t val) noexcept
{
    return fpgaIf.write(pwrConvModName, powerConvRegMap::rmsTimerCountTop, val);
}

/**
 * @brief      Gets the rms timer count thold.
 *
 * @return     The rms timer count threshold if successful, std::nullopt otherwise.
 */
std::optional<rfManagerController::mmData_t> rfManagerController::getRmsTimerCountThold() noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConvRegMap::rmsTimerCountThreshold)}; resp.has_value())
    {
        return resp.value();
    }

    return std::nullopt;
}

/**
 * @brief      Sets the rms timer count top.
 *
 * @param[in]  val   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setRmsTimerCountThold(const mmData_t val) noexcept
{
    return fpgaIf.write(pwrConvModName, powerConvRegMap::rmsTimerCountThreshold, val);
}

/**
 * @brief      Converts an RF Port to the Power Conversion Address Offset Base
 *
 * @param[in]  port  The port
 *
 * @return     The Offset Base of the given port in the Power Conversion Address Space.
 */
rfManagerController::reg_t rfManagerController::powerConversionPortOffset(const port_t port) const noexcept
{
    reg_t rtn{powerConvRegMap::fwdRmsFilterBase};

    switch(port)
    {
    case port_t::REF:
        rtn = powerConvRegMap::revRmsFilterBase;
        break;

    case port_t::IN:
        rtn = powerConvRegMap::inpRmsFilterBase;
        break;

    case port_t::UNBAL:
        rtn = powerConvRegMap::unbRmsFilterBase;
        break;

    case port_t::FWD:
    default:
        break;
    }

    return rtn;
}

/**
 * @brief      Gets the rms filter status.
 *
 * @param[in]  port  The port
 *
 * @return     The rms filter status if successful, otherwise std::nullopt.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getRmsFilterStatus(const port_t port) noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConversionPortOffset(port) + powerConvRegMap::detRmsFilterStatus)};
        resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Gets the rms filtered value.
 *
 * @param[in]  port  The port
 *
 * @return     The rms filtered value if successful, otherwise std::nullopt.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getRmsFilteredValue(const port_t port) noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConversionPortOffset(port) + powerConvRegMap::detRmsFilteredValue)};
        resp.has_value())
    {
        return resp.value();
    }

    return std::nullopt;
}

/**
 * @brief      Gets the rms filter enable.
 *
 * @param[in]  port  The port
 *
 * @return     The rms filter enable if successful, otherwise std::nullopt.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getRmsFilterEnable(const port_t port) noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConversionPortOffset(port) + powerConvRegMap::detRmsFilteredValue)};
        resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the rms filter enable.
 *
 * @param[in]  port   The port
 * @param[in]  state  The state
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setRmsFilterEnable(const port_t port, const signal_t state) noexcept
{
    return fpgaIf.write(pwrConvModName, powerConversionPortOffset(port) + powerConvRegMap::detRmsFilteredValue,
        helpers::types::sigToRegVal(state));
}

/**
 * @brief      Gets the rms filter force cw.
 *
 * @param[in]  port  The port
 *
 * @return     The rms filter force cw if successful, otherwise std::nullopt.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getRmsFilterForceCw(const port_t port) noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConversionPortOffset(port) + powerConvRegMap::detRmsFilteredValue)};
        resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}



/**
 * @brief      Sets the rms filter force cw.
 *
 * @param[in]  port   The port
 * @param[in]  state  The state
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setRmsFilterForceCw(const port_t port, const signal_t state) noexcept
{
    return fpgaIf.write(pwrConvModName, powerConversionPortOffset(port) + powerConvRegMap::detRmsFilterForcePulse,
        helpers::types::sigToRegVal(state));
}

/**
 * @brief      Gets the rms filter force pulse.
 *
 * @param[in]  port  The port
 *
 * @return     The rms filter force pulse if successful, otherwise std::nullopt.
 */
std::optional<rfManagerController::signal_t> rfManagerController::getRmsFilterForcePulse(const port_t port) noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName,
        powerConversionPortOffset(port) + powerConvRegMap::detRmsFilterForcePulse)}; resp.has_value())
    {
        return helpers::types::regValToSig(resp.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the rms filter force pulse.
 *
 * @param[in]  port   The port
 * @param[in]  state  The state
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setRmsFilterForcePulse(const port_t port, const signal_t state) noexcept
{
    return fpgaIf.write(pwrConvModName, powerConversionPortOffset(port) + powerConvRegMap::detRmsFilterForceCw,
        helpers::types::sigToRegVal(state));
}

/**
 * @brief      Gets the rms filter offset.
 *
 * @param[in]  port  The port
 *
 * @return     The rms filter offset if successful, otherwise std::nullopt.
 */
std::optional<rfManagerController::dbm_t> rfManagerController::getRmsFilterOffset(const port_t port) noexcept
{
    if(auto resp{fpgaIf.read(pwrConvModName, powerConversionPortOffset(port) + powerConvRegMap::detRmsFilterOffset)};
        resp.has_value())
    {
        return helpers::types::dbmRawToDbm(static_cast<dbm_raw_t>(resp.value()));
    }

    return std::nullopt;
}

/**
 * @brief      Sets the rms filter offset.
 *
 * @param[in]  port  The port
 * @param[in]  pwr   The new value
 *
 * @return     True if successful, False otherwise.
 */
bool rfManagerController::setRmsFilterOffset(const port_t port, const dbm_t pwr) noexcept
{
    return fpgaIf.write(pwrConvModName, powerConversionPortOffset(port) + powerConvRegMap::detRmsFilterOffset,
        helpers::types::dbmToRegVal(pwr));
}

/**
 * @brief      Reads a raw adc.
 *
 * @param[in]  chan  The channel
 *
 * @return     The raw adc value if sucessful, std::nullopt otherwise.
 */
std::optional<rfManagerController::adc_raw_t> rfManagerController::readRawAdc(const chan_t& chan) noexcept
{
    fpgaIf.write(peakDetModName, peakDetRegMap::snap_trig, 1);

    return fpgaIf.read(peakDetModName,
        peakDetRegMap::base_det_addr + (channelMap.at(chan) << 2));
}

/**
 * @brief      Reads all Power Levels
 * What are we going to report in here? All 12 Detector/Port streams, 3 Dectector/VSWR, 4 Port/PARR
 *
 * @return     A tuple with all of the power readings
 */
rfManagerController::powerTuple rfManagerController::pollPowerLevels() noexcept
{
    using namespace helpers::types;

    fpgaIf.write(moveToPsModName, dmpRegMap::snapreg_trig, 0);

    auto resp{fpgaIf.read(moveToPsModName, {
        dmpRegMap::snapreg_fwd_rms, dmpRegMap::snapreg_fwd_env, dmpRegMap::snapreg_fwd_peak, dmpRegMap::fwd_peak_hold,
        dmpRegMap::snapreg_rev_rms, dmpRegMap::snapreg_rev_env, dmpRegMap::snapreg_rev_peak, dmpRegMap::rev_peak_hold,
        dmpRegMap::snapreg_inp_rms, dmpRegMap::snapreg_inp_env, dmpRegMap::snapreg_inp_peak, dmpRegMap::inp_peak_hold,
        dmpRegMap::snapreg_unb_rms, dmpRegMap::snapreg_unb_env, dmpRegMap::snapreg_unb_peak, dmpRegMap::unb_peak_hold,
    })};

    auto readData = resp.value_or(std::vector<mmData_t>{
        dbmToRegVal(-70.0), dbmToRegVal(-70.0), dbmToRegVal(-70.0), dbmToRegVal(-70.0),
        dbmToRegVal(-70.0), dbmToRegVal(-70.0), dbmToRegVal(-70.0), dbmToRegVal(-70.0),
        dbmToRegVal(-70.0), dbmToRegVal(-70.0), dbmToRegVal(-70.0), dbmToRegVal(-70.0),
        dbmToRegVal(-70.0), dbmToRegVal(-70.0), dbmToRegVal(-70.0), dbmToRegVal(-70.0)
    });

    const auto floorData = [&](const port_t port, const detector_t det, const dbm_t pwr) -> dbm_t
    {
        return (((pwr - channelCf.at(port)) >= channelFloor.at({port, det}))?(pwr):(powerFloorValue));
    };

    const auto fwdRms{floorData(port_t::FWD, detector_t::RMS,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(0))))};
    const auto fwdEnv{floorData(port_t::FWD, detector_t::ENV,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(1))))};
    const auto fwdPeakSnap{floorData(port_t::FWD, detector_t::PEAK,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(2))))};
    const auto fwdPeak{floorData(port_t::FWD, detector_t::PEAK,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(3))))};
    const auto revRms{floorData(port_t::REF, detector_t::RMS,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(4))))};
    const auto revEnv{floorData(port_t::REF, detector_t::ENV,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(5))))};
    const auto revPeakSnap{floorData(port_t::REF, detector_t::PEAK,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(6))))};
    const auto revPeak{floorData(port_t::REF, detector_t::PEAK,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(7))))};
    const auto inpRms{floorData(port_t::IN, detector_t::RMS,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(8))))};
    const auto inpEnv{floorData(port_t::IN, detector_t::ENV,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(9))))};
    const auto inpPeakSnap{floorData(port_t::IN, detector_t::PEAK,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(10))))};
    const auto inpPeak{floorData(port_t::IN, detector_t::PEAK,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(11))))};
    const auto unbRms{floorData(port_t::UNBAL, detector_t::RMS,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(12))))};
    const auto unbEnv{floorData(port_t::UNBAL, detector_t::ENV,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(13))))};
    const auto unbPeakSnap{floorData(port_t::UNBAL, detector_t::PEAK,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(14))))};
    const auto unbPeak{floorData(port_t::UNBAL, detector_t::PEAK,
        dbmRawToDbm(static_cast<dbm_raw_t>(readData.at(15))))};


    const auto vswrRms{calcVswr(fwdRms, revRms)};
    const auto vswrEnv{calcVswr(fwdEnv, revEnv)};
    const auto vswrPeak{calcVswr(fwdPeakSnap, revPeakSnap)};
    const auto fwdPapr{calcPapr(fwdPeakSnap, fwdRms)};
    const auto revPapr{calcPapr(revPeakSnap, revRms)};
    const auto inpPapr{calcPapr(inpPeakSnap, inpRms)};
    const auto unbPapr{calcPapr(unbPeakSnap, unbRms)};
    const auto compr{calcCompr(inpPapr, fwdPapr)};

    // The use of std::make_tuple and std::tie here ensure at compile time that all of the data line up
    // and will give compile time errors if there are any count mis-matches
    powerTuple data{
        fwdRms, fwdEnv, fwdPeakSnap, fwdPeak,
        revRms, revEnv, revPeakSnap, revPeak,
        inpRms, inpEnv, inpPeakSnap, inpPeak,
        unbRms, unbEnv, unbPeakSnap, unbPeak,
        vswrRms, vswrEnv, vswrPeak,
        fwdPapr, revPapr, inpPapr, unbPapr,
        compr
    };

    std::tie(
        powerReportingDataSet.at(0).data,
        powerReportingDataSet.at(1).data,
        powerReportingDataSet.at(2).data,
        powerReportingDataSet.at(3).data,
        powerReportingDataSet.at(4).data,
        powerReportingDataSet.at(5).data,
        powerReportingDataSet.at(6).data,
        powerReportingDataSet.at(7).data,
        powerReportingDataSet.at(8).data,
        powerReportingDataSet.at(9).data,
        powerReportingDataSet.at(10).data,
        powerReportingDataSet.at(11).data,
        powerReportingDataSet.at(12).data,
        powerReportingDataSet.at(13).data,
        powerReportingDataSet.at(14).data,
        powerReportingDataSet.at(15).data,
        powerReportingDataSet.at(16).data,
        powerReportingDataSet.at(17).data,
        powerReportingDataSet.at(18).data,
        powerReportingDataSet.at(19).data,
        powerReportingDataSet.at(20).data,
        powerReportingDataSet.at(21).data,
        powerReportingDataSet.at(22).data,
        powerReportingDataSet.at(23).data
    ) = data;

    for(auto& dp: powerReportingDataSet)
    {
        dp.valid = resp.has_value();
    }

    updateIf.updateData(powerReportingDataSet);

    return data;
}

/**
 * @brief      Polls the FPGA for essential data that gets pushed to Status
 */
void rfManagerController::pollFpgaData() noexcept
{
    using namespace helpers::types;

    constexpr std::size_t inputSwitchIdx = 0;
    constexpr std::size_t inputSelectIdx = 1;
    constexpr std::size_t currPhaseIdx = 2;
    constexpr std::size_t alcSetIdx = 3;
    constexpr std::size_t agcSetIdx = 4;
    constexpr std::size_t mgcVvaSetIdx = 5;
    constexpr std::size_t mgcGainSetIdx = 6;
    constexpr std::size_t mgcPerSetIdx = 7;
    constexpr std::size_t onlineStateIdx = 8;
    constexpr std::size_t safeStateIdx = 9;
    constexpr std::size_t opModeIdx = 10;
    constexpr std::size_t regDetIdx = 11;
    constexpr std::size_t inputPresentIdx = 12;
    constexpr std::size_t forwardPresentIdx = 13;
    constexpr std::size_t dacOutputIdx = 14;
    constexpr std::size_t blankingInIdx = 15;
    constexpr std::size_t blankingPolIdx = 16;
    constexpr std::size_t blankingStateIdx = 17;
    constexpr std::size_t shutdownInIdx = 18;
    constexpr std::size_t shutdownPolIdx = 19;
    constexpr std::size_t shutdownStateIdx = 20;
    constexpr std::size_t getPwrHwFaultBackoffIdx = 21;
    constexpr std::size_t getPwrSwFaultBackoffIdx = 22;
    constexpr std::size_t getPwrHwAvailIdx = 23;
    constexpr std::size_t getNominalPwrIdx = 24;
    constexpr std::size_t getPwrAvailIdx = 25;
    constexpr std::size_t getPaprBackoffIdx = 26;
    constexpr std::size_t mgcTopIdx = 27;
    constexpr std::size_t openLoopGainIdx = 28;
    constexpr std::size_t vvaRangeIdx = 29;
    constexpr std::size_t alcRangeIdx = 30;
    constexpr std::size_t agcRangeIdx = 31;
    constexpr std::size_t agcRefIdx = 32;
    constexpr std::size_t bitFuncGenEnableSelectIdx = 33;
    constexpr std::size_t bitRfEnableHighTimeIdx = 34;
    constexpr std::size_t bitRfEnableLowTimeIdx = 35;
    constexpr std::size_t bitRfEnableDutyIdx = 36;
    constexpr std::size_t bitRfGuardTimeIdx = 37;
    constexpr std::size_t bitRfModulationActiveTimeIdx = 38;
    constexpr std::size_t bitRfModulationInactiveTimeIdx = 39;
    constexpr std::size_t bitRfModulationDutyIdx = 40;
    constexpr std::size_t bitRfOverallDutyIdx = 41;
    constexpr std::size_t bitOscPwmHighTimeIdx = 42;
    constexpr std::size_t bitOscPwmLowTimeIdx = 43;
    constexpr std::size_t manualInputSwitchIdx = 44;
    constexpr std::size_t rmsFltTmrStatusIdx = 45;
    constexpr std::size_t rmsFltInpStatusIdx = 46;
    constexpr std::size_t rmsFltFwdStatusIdx = 47;
    constexpr std::size_t rmsFltRevStatusIdx = 48;
    constexpr std::size_t rmsFltUnbStatusIdx = 49;
    constexpr std::size_t droopCorrectionLine1EnableIdx = 50;
    constexpr std::size_t droopCorrectionLine2EnableIdx = 51;

    const auto getInputSwitchData = [&](){
        const auto resp{getInputSwitch()};
        return std::make_pair(inpSwitchToStr(resp.value_or(input_switch_t::LOAD)), resp.has_value());
    };

    const auto getInputSelectData = [&](){
        const auto resp{getInputSelect()};
        return std::make_pair(inpSelToStr(resp.value_or(input_select_t::BIT)), resp.has_value());
    };

    const auto getCurrPhaseData = [&](){
        const auto resp{getPhaseCur()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getAlcSetData = [&](){
        const auto resp{getAlcSetpoint()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getAgcSetData = [&](){
        const auto resp{getAgcSetpoint()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getMgcSetVvaData = [&](){
        const auto resp{getMgcSetpointVva()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getMgcSetGainData = [&](){
        const auto resp{getMgcSetpointGain()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getMgcSetPerData = [&](){
        const auto resp{getMgcSetpointPercentage()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getRfStateData = [&](){
        const auto resp{getRfState()};
        return std::make_pair(onlineStateToStr(resp.value_or(online_state_t::COLD)), resp.has_value());
    };

    const auto getSafeStateData = [&](){
        const auto resp{getSafeState()};
        return std::make_pair(safeStateToStr(resp.value_or(safe_state_t::SAFE)), resp.has_value());
    };

    const auto getOpModeData = [&](){
        const auto resp{getGainMode()};
        return std::make_pair(gainModeToStr(resp.value_or(gain_mode_t::MGC)), resp.has_value());
    };

    const auto getRegulationDetData = [&](){
        const auto resp{getRegulationDet()};
        return std::make_pair(detectorToStr(resp.value_or(detector_t::ENV)), resp.has_value());
    };

    const auto getInputPresentData = [&](){
        const auto resp{getInpPres()};
        return std::make_pair(sigToBool(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getFwdPresData = [&](){
        const auto resp{getFwdPres()};
        return std::make_pair(sigToBool(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getDacOutputData = [&](){
        const auto resp{getDacOutput()};
        return std::make_pair(resp.value_or(0), resp.has_value());
    };

    const auto getBlankingInData = [&](){
        const auto resp{getBlankingInput()};
        return std::make_pair(sigToStrHigh(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getBlankingPolData = [&](){
        const auto resp{getBlankingPol()};
        return std::make_pair(polToStr(resp.value_or(polarity_t::ACTIVE_HIGH)), resp.has_value());
    };

    const auto getBlankingStateData = [&](){
        const auto resp{getBlankingState()};
        return std::make_pair(sigToStrActive(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getShutdownInData = [&](){
        const auto resp{getShutdownInput()};
        return std::make_pair(sigToStrHigh(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getShutdownPolData = [&](){
        const auto resp{getShutdownPol()};
        return std::make_pair(polToStr(resp.value_or(polarity_t::ACTIVE_HIGH)), resp.has_value());
    };

    const auto getShutdownStateData = [&](){
        const auto resp{getShutdownState()};
        return std::make_pair(sigToStrActive(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getPwrHwFaultBackoffData = [&](){
        const auto resp{getPwrHwFaultBackoff()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getPwrSwFaultBackoffData = [&](){
        const auto resp{getPwrSwFaultBackoff()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getPwrHwAvailData = [&](){
        const auto resp{getPwrHwAvail()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getNominalPwrData = [&](){
        const auto resp{getNominalPwr()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getPwrAvailData = [&](){
        const auto resp{getPwrAvail()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getPaprBackoffData = [&](){
        const auto resp{getPaprBackoff()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getBitFuncGenEnableSelectData = [&](){
        const auto resp{getBitFuncGenEnableSelect()};
        return std::make_pair(sigToStrBitIntExt(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getDroopCorrectionLine1EnableData = [&](){
    const auto resp{getDroopCorrectionLine1Enable()};
        return std::make_pair(resp.value_or(false) ? 1.0f : 0.0f, resp.has_value());
    };

    const auto getDroopCorrectionLine2EnableData = [&](){
        const auto resp{getDroopCorrectionLine2Enable()};
        return std::make_pair(resp.value_or(false) ? 1.0f : 0.0f, resp.has_value());
    };

    const auto getBitRfEnableTimeData = [&](){
        const auto fixFloatingPoint = [](const auto d){
            if constexpr(std::is_floating_point_v<decltype(d)>)
            {
                if(std::isnan(d))
                {
                    return 0.0;
                }
                else if(std::isinf(d))
                {
                    return 100.0;
                }
            }

            return d;
        };

        // Enable Values
        const auto enableHighTimeResp{getBitRfEnableHighTime()};
        const auto enableHighTimeValid{enableHighTimeResp.has_value()};
        const auto enableHighTimeVal{enableHighTimeResp.value_or(decltype(enableHighTimeResp)::value_type{0})};
        const auto enableHighResult{std::chrono::duration_cast<std::chrono::duration<double>>(enableHighTimeVal).count()};

        const auto enableLowTimeResp{getBitRfEnableLowTime()};
        const auto enableLowTimeValid{enableLowTimeResp.has_value()};
        const auto enableLowTimeVal{enableLowTimeResp.value_or(decltype(enableLowTimeResp)::value_type{0})};
        const auto enableLowResult{std::chrono::duration_cast<std::chrono::duration<double>>(enableLowTimeVal).count()};

        const auto enableDutyCycle{fixFloatingPoint(enableHighResult / (enableHighResult + enableLowResult))};
        const auto enableDutyValid{(enableHighTimeValid && enableLowTimeValid)};
        const auto enableDutyResult{helpers::types::floatToStr((enableDutyCycle * 100.0), 2)};

        // Modulation Values
        const auto modulationHighTimeResp{getBitRfModulationActiveTime()};
        const auto modulationHighTimeValid{modulationHighTimeResp.has_value()};
        const auto modulationHighTimeVal{
            modulationHighTimeResp.value_or(decltype(modulationHighTimeResp)::value_type{0})};
        const auto modulationHighResult{
            std::chrono::duration_cast<std::chrono::duration<double>>(modulationHighTimeVal).count()};

        const auto modulationLowTimeResp{getBitRfModulationInactiveTime()};
        const auto modulationLowTimeValid{modulationLowTimeResp.has_value()};
        const auto modulationLowTimeVal{
            modulationLowTimeResp.value_or(decltype(modulationLowTimeResp)::value_type{0})};
        const auto modulationLowResult{
            std::chrono::duration_cast<std::chrono::duration<double>>(modulationLowTimeVal).count()};

        const auto modulationDutyCycle{
            fixFloatingPoint(modulationHighResult / (modulationHighResult + modulationLowResult))};
        const auto modulationDutyValid{(modulationHighTimeValid && modulationLowTimeValid)};
        const auto modulationDutyResult{helpers::types::floatToStr((modulationDutyCycle * 100.0), 2)};

        // Overall Values
        const auto overallDuty{(enableDutyCycle * modulationDutyCycle * 100.0)};
        const auto overallDutyValid{(enableDutyValid && modulationDutyValid)};

        return std::make_tuple(
            enableHighResult, enableHighTimeValid,
            enableLowResult, enableLowTimeValid,
            enableDutyResult, enableDutyValid,
            modulationHighResult, modulationHighTimeValid,
            modulationLowResult, modulationLowTimeValid,
            modulationDutyResult, modulationDutyValid,
            overallDuty, overallDutyValid
        );
    };

    const auto getBitRfGuardTimeData = [&](){
        const auto resp{getBitRfGuardTime()};
        const auto fpgaClkTime{resp.value_or(fpga_clock_rate_t{0})};
        const auto nanoSecTime{std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(fpgaClkTime)};
        return std::make_pair(nanoSecTime.count(), resp.has_value());
    };

    const auto getBitOscPwmHighTimeData = [&](){
        const auto resp{getBitOscPwmHighTime()};
        const auto fpgaClkTime{resp.value_or(fpga_clock_rate_t{0})};
        const auto secTime{std::chrono::duration_cast<std::chrono::duration<double>>(fpgaClkTime)};
        return std::make_pair(secTime.count(), resp.has_value());
    };

    const auto getBitOscPwmLowTimeData = [&](){
        const auto resp{getBitOscPwmLowTime()};
        const auto fpgaClkTime{resp.value_or(fpga_clock_rate_t{0})};
        const auto secTime{std::chrono::duration_cast<std::chrono::duration<double>>(fpgaClkTime)};
        return std::make_pair(secTime.count(), resp.has_value());
    };

    const auto getManualInputSwitchData = [&](){
        const auto resp{getManualInputSwitch()};
        return std::make_pair(
            manualInpSwitchToStr(resp.value_or(manual_input_switch_t::INPUT)),
            resp.has_value()
        );
    };

    const auto getAlcDynamicRangeData = [&](){
        const auto resp{getAlcDynamicRange()};
        return std::make_pair(resp.value_or(0.0), resp.has_value());
    };

    const auto getFltTmrStatus = [&](){
        const auto resp{getRmsTimerStatus()};
        return std::make_pair(sigToStrActive(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getFltInpStatus = [&](){
        const auto resp{getRmsFilterStatus(port_t::IN)};
        return std::make_pair(sigToStrActive(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getFltFwdStatus = [&](){
        const auto resp{getRmsFilterStatus(port_t::FWD)};
        return std::make_pair(sigToStrActive(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getFltRevStatus = [&](){
        const auto resp{getRmsFilterStatus(port_t::REF)};
        return std::make_pair(sigToStrActive(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    const auto getFltUnbStatus = [&](){
        const auto resp{getRmsFilterStatus(port_t::UNBAL)};
        return std::make_pair(sigToStrActive(resp.value_or(signal_t::INACTIVE)), resp.has_value());
    };

    std::tie(slowDataSet.at(inputSwitchIdx).data, slowDataSet.at(inputSwitchIdx).valid) = getInputSwitchData();
    std::tie(slowDataSet.at(inputSelectIdx).data, slowDataSet.at(inputSelectIdx).valid) = getInputSelectData();
    std::tie(slowDataSet.at(currPhaseIdx).data, slowDataSet.at(currPhaseIdx).valid) = getCurrPhaseData();
    std::tie(slowDataSet.at(alcSetIdx).data, slowDataSet.at(alcSetIdx).valid) = getAlcSetData();
    std::tie(slowDataSet.at(agcSetIdx).data, slowDataSet.at(agcSetIdx).valid) = getAgcSetData();
    std::tie(slowDataSet.at(mgcVvaSetIdx).data, slowDataSet.at(mgcVvaSetIdx).valid) = getMgcSetVvaData();
    std::tie(slowDataSet.at(mgcGainSetIdx).data, slowDataSet.at(mgcGainSetIdx).valid) = getMgcSetGainData();
    std::tie(slowDataSet.at(mgcPerSetIdx).data, slowDataSet.at(mgcPerSetIdx).valid) = getMgcSetPerData();
    std::tie(slowDataSet.at(onlineStateIdx).data, slowDataSet.at(onlineStateIdx).valid) = getRfStateData();
    std::tie(slowDataSet.at(safeStateIdx).data, slowDataSet.at(safeStateIdx).valid) = getSafeStateData();
    std::tie(slowDataSet.at(opModeIdx).data, slowDataSet.at(opModeIdx).valid) = getOpModeData();
    std::tie(slowDataSet.at(regDetIdx).data, slowDataSet.at(regDetIdx).valid) = getRegulationDetData();
    std::tie(slowDataSet.at(inputPresentIdx).data, slowDataSet.at(inputPresentIdx).valid) = getInputPresentData();
    std::tie(slowDataSet.at(forwardPresentIdx).data, slowDataSet.at(forwardPresentIdx).valid) = getFwdPresData();
    std::tie(slowDataSet.at(dacOutputIdx).data, slowDataSet.at(dacOutputIdx).valid) = getDacOutputData();
    std::tie(slowDataSet.at(blankingInIdx).data, slowDataSet.at(blankingInIdx).valid) = getBlankingInData();
    std::tie(slowDataSet.at(blankingPolIdx).data, slowDataSet.at(blankingPolIdx).valid) = getBlankingPolData();
    std::tie(slowDataSet.at(blankingStateIdx).data, slowDataSet.at(blankingStateIdx).valid) = getBlankingStateData();
    std::tie(slowDataSet.at(shutdownInIdx).data, slowDataSet.at(shutdownInIdx).valid) = getShutdownInData();
    std::tie(slowDataSet.at(shutdownPolIdx).data, slowDataSet.at(shutdownPolIdx).valid) = getShutdownPolData();
    std::tie(slowDataSet.at(shutdownStateIdx).data, slowDataSet.at(shutdownStateIdx).valid) = getShutdownStateData();
    std::tie(slowDataSet.at(getPwrHwFaultBackoffIdx).data, slowDataSet.at(getPwrHwFaultBackoffIdx).valid) =
        getPwrHwFaultBackoffData();
    std::tie(slowDataSet.at(getPwrSwFaultBackoffIdx).data, slowDataSet.at(getPwrSwFaultBackoffIdx).valid) =
        getPwrSwFaultBackoffData();
    std::tie(slowDataSet.at(getPwrHwAvailIdx).data, slowDataSet.at(getPwrHwAvailIdx).valid) = getPwrHwAvailData();
    std::tie(slowDataSet.at(getNominalPwrIdx).data, slowDataSet.at(getNominalPwrIdx).valid) = getNominalPwrData();
    std::tie(slowDataSet.at(getPwrAvailIdx).data, slowDataSet.at(getPwrAvailIdx).valid) = getPwrAvailData();
    std::tie(slowDataSet.at(getPaprBackoffIdx).data, slowDataSet.at(getPaprBackoffIdx).valid) = getPaprBackoffData();
    std::tie(slowDataSet.at(bitFuncGenEnableSelectIdx).data, slowDataSet.at(bitFuncGenEnableSelectIdx).valid) =
        getBitFuncGenEnableSelectData();
    std::tie(
        slowDataSet.at(bitRfEnableHighTimeIdx).data, slowDataSet.at(bitRfEnableHighTimeIdx).valid,
        slowDataSet.at(bitRfEnableLowTimeIdx).data, slowDataSet.at(bitRfEnableLowTimeIdx).valid,
        slowDataSet.at(bitRfEnableDutyIdx).data, slowDataSet.at(bitRfEnableDutyIdx).valid,
        slowDataSet.at(bitRfModulationActiveTimeIdx).data, slowDataSet.at(bitRfModulationActiveTimeIdx).valid,
        slowDataSet.at(bitRfModulationInactiveTimeIdx).data, slowDataSet.at(bitRfModulationInactiveTimeIdx).valid,
        slowDataSet.at(bitRfModulationDutyIdx).data, slowDataSet.at(bitRfModulationDutyIdx).valid,
        slowDataSet.at(bitRfOverallDutyIdx).data, slowDataSet.at(bitRfOverallDutyIdx).valid
    ) = getBitRfEnableTimeData();
    std::tie(slowDataSet.at(bitRfGuardTimeIdx).data, slowDataSet.at(bitRfGuardTimeIdx).valid) =
        getBitRfGuardTimeData();
    std::tie(slowDataSet.at(bitOscPwmHighTimeIdx).data, slowDataSet.at(bitOscPwmHighTimeIdx).valid) =
        getBitOscPwmHighTimeData();
    std::tie(slowDataSet.at(bitOscPwmLowTimeIdx).data, slowDataSet.at(bitOscPwmLowTimeIdx).valid) =
        getBitOscPwmLowTimeData();
    std::tie(slowDataSet.at(manualInputSwitchIdx).data, slowDataSet.at(manualInputSwitchIdx).valid) =
        getManualInputSwitchData();
    std::tie(slowDataSet.at(droopCorrectionLine1EnableIdx).data, slowDataSet.at(droopCorrectionLine1EnableIdx).valid) = getDroopCorrectionLine1EnableData();
    std::tie(slowDataSet.at(droopCorrectionLine2EnableIdx).data, slowDataSet.at(droopCorrectionLine2EnableIdx).valid) = getDroopCorrectionLine2EnableData();

    slowDataSet.at(mgcTopIdx).data = mgcTop;
    slowDataSet.at(mgcTopIdx).valid = true;
    slowDataSet.at(openLoopGainIdx).data = openLoopGain;
    slowDataSet.at(openLoopGainIdx).valid = true;
    slowDataSet.at(vvaRangeIdx).data = vvaRange;
    slowDataSet.at(vvaRangeIdx).valid = true;
    std::tie(slowDataSet.at(alcRangeIdx).data, slowDataSet.at(alcRangeIdx).valid) = getAlcDynamicRangeData();
    slowDataSet.at(agcRangeIdx).data = agcRange;
    slowDataSet.at(agcRangeIdx).valid = true;
    slowDataSet.at(agcRefIdx).data = agcRef;
    slowDataSet.at(agcRefIdx).valid = true;

    std::tie(slowDataSet.at(rmsFltTmrStatusIdx).data, slowDataSet.at(rmsFltTmrStatusIdx).valid) = getFltTmrStatus();
    std::tie(slowDataSet.at(rmsFltInpStatusIdx).data, slowDataSet.at(rmsFltInpStatusIdx).valid) = getFltInpStatus();
    std::tie(slowDataSet.at(rmsFltFwdStatusIdx).data, slowDataSet.at(rmsFltFwdStatusIdx).valid) = getFltFwdStatus();
    std::tie(slowDataSet.at(rmsFltRevStatusIdx).data, slowDataSet.at(rmsFltRevStatusIdx).valid) = getFltRevStatus();
    std::tie(slowDataSet.at(rmsFltUnbStatusIdx).data, slowDataSet.at(rmsFltUnbStatusIdx).valid) = getFltUnbStatus();

    updateIf.updateData(slowDataSet);
}
