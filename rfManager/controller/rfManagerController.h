#ifndef RF_NAMAGER_CONTROLLER_H_
#define RF_NAMAGER_CONTROLLER_H_

#include <configManagerConnection/configManagerConnection.h>
#include <fpgaInterfaceConnection/fpgaInterfaceConnection.h>
#include <helpers/jsonUpdateHelpers.h>
#include <helpers/rfLayoutTypes.h>
#include <map>
#include <statusUpdateInterface/statusUpdateInterface.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <zmq.hpp>
#include <helpers/debug.h>

namespace empower
{
    /**
     * @brief      Controls the data flow into a rf manager object and updates the view whenever data changes.
     */
    class rfManagerController
    {
    public:
        using adc_raw_t = helpers::types::adc_raw_t;
        using chan_t = helpers::types::chan_t;
        using dbm_t = helpers::types::dbm_t;
        using dbm_raw_t = helpers::types::dbm_raw_t;
        using vswr_t = helpers::types::vswr_t;
        using papr_t = helpers::types::papr_t;
        using compr_t = helpers::types::compr_t;
        using detector_t = helpers::types::detector_t;
        using mmData_t = helpers::types::mmData_t;
        using port_t = helpers::types::port_t;
        using portMap_t = helpers::types::portMap_t;
        using reg_t = helpers::types::reg_t;
        using updateIfData_t = helpers::types::updateIfData_t;
        using percent_t = helpers::types::percent_t;
        using timeout_t = helpers::types::timeout_t;
        using gain_mode_t = helpers::types::gain_mode_t;
        using online_state_t = helpers::types::online_state_t;
        using safe_state_t = helpers::types::safe_state_t;
        using polarity_t = helpers::types::polarity_t;
        using input_select_t = helpers::types::input_select_t;
        using input_switch_t = helpers::types::input_switch_t;
        using phase_t = helpers::types::phase_t;
        using signal_t = helpers::types::signal_t;
        using decay_rate_t = helpers::types::decay_rate_t;
        using fpga_clock_rate_t = helpers::types::fpga_clock_rate_t;
        using time_duration_t = helpers::types::time_duration_t;
        using droopLine_t = helpers::types::droopLine_t;
        using manual_input_switch_t = helpers::types::manual_input_switch_t;
        using rf_band_index_t = helpers::types::rf_band_index_t;


        using powerTuple = std::tuple<
            dbm_t, dbm_t, dbm_t, dbm_t,      // FWD RMS, ENV, PEAK_SNAP, & PEAK
            dbm_t, dbm_t, dbm_t, dbm_t,      // REV RMS, ENV, PEAK_SNAP, & PEAK
            dbm_t, dbm_t, dbm_t, dbm_t,      // INP RMS, ENV, PEAK_SNAP, & PEAK
            dbm_t, dbm_t, dbm_t, dbm_t,      // UNB RMS, ENV, PEAK_SNAP, & PEAK
            vswr_t, vswr_t, vswr_t,   // VSWR RMS, ENV, & PEAK
            papr_t, papr_t, papr_t, papr_t, // PAPR FWD, REV, INP, & UNB
            compr_t                   // Compression
        >;

        using powerDataSetArray = std::array<updateIfData_t, std::tuple_size_v<powerTuple>>;
        using slowDataUpdateArray = std::array<updateIfData_t, 52>;

        using gpioKeyPair_t = std::pair<std::string, reg_t>;
        using gpioKeyArray_t = std::array<gpioKeyPair_t, 25>;


        /**
         * @brief       RF Band Data Parameters Struct
         */
        typedef struct rfBandParameters
        {
            std::string     label;
            std::string     band_cal_tbl;
            std::string     band_dpd_config;
            dbm_t           cpl_factor;
        } rfBandParam_t;

        static constexpr std::size_t _MAX_BAND_INDEX = 256;
        using rfBandSetting_t = std::array<rfBandParam_t, _MAX_BAND_INDEX>;
        //

        ~rfManagerController() {}

        ///////////////////////////////////////////////////////////////////////////////
        /// Added getters to reduce time to get data again from configuration manager
        // Also access data from M2M
        //////////////////////////////////////////////////////////////////////////////

        mmData_t getMaxVva() { return mgcTop; };
        //
        const std::map<port_t, dbm_t>& getChannelCFMap() const {
            return channelCf;
        }
        //
        const std::map<chan_t, dbm_t>& getChannelFloorMap() const {
            return channelFloor;
        }

    private:
        /**
         * @brief      RF Manager Register Map
         */
        struct rfManagerRegMap
        {
            static constexpr reg_t inpSel = 0x0000;
            static constexpr reg_t rfEnable = 0x0004;
            static constexpr reg_t phyReset = 0x0008;
            static constexpr reg_t safeState = 0x000C;
            static constexpr reg_t blankPol = 0x0010;
            static constexpr reg_t blankTst = 0x0014;
            static constexpr reg_t blankStat = 0x0018;
            static constexpr reg_t shutdownPol = 0x001C;
            static constexpr reg_t shutdownTst = 0x0020;
            static constexpr reg_t shutdownStat = 0x0024;
            static constexpr reg_t pwrHwrFltBckoff = 0x0028; // Fault Manager
            static constexpr reg_t pwrSwrFltBckoff = 0x002C; // Fault Manager
            static constexpr reg_t pwrHwrAvail = 0x0030;
            static constexpr reg_t pwrNominal = 0x0034;
            static constexpr reg_t pwrAvail = 0x0040;
            static constexpr reg_t modProfBckoff = 0x0044; // Fault Manager
            static constexpr reg_t dutyCycleBckoff = 0x0048; // Fault Manager
            static constexpr reg_t softwareMute = 0x004C;
            static constexpr reg_t inputSwitchControl = 0x0050;
            static constexpr reg_t blankingInExtStat = 0x0054;
            static constexpr reg_t shutdownInExtStat = 0x0058;
            static constexpr reg_t blankOutputPol = 0x005C;
            static constexpr reg_t manualInputSwitchCtrl = 0x0064;
            static constexpr reg_t rfInpSwitchSeqMuteTime = 0x006C;
        };

        /**
         * @brief      ADC Register Map
         */
        struct adcRegMap
        {
            static constexpr reg_t ctrl = 0x0008;
            static constexpr reg_t data = 0x000C;
            static constexpr reg_t stat = 0x0010;
        };

        /**
         * @brief      DAC Register Map
         */
        struct dacRegMap
        {
            static constexpr reg_t writeData = 0x0000;
            static constexpr reg_t dds = 0x0004;
            static constexpr reg_t ceiling = 0x0008;
            static constexpr reg_t srcSel = 0x000C;
            static constexpr reg_t readData = 0x0010;
            static constexpr reg_t initDropCnt_1 = 0x0014;
            static constexpr reg_t riseHoriz_1 = 0x0018;
            static constexpr reg_t riseVert_1 = 0x001C;
            static constexpr reg_t initDropCnt_2 = 0x0020;
            static constexpr reg_t riseHoriz_2 = 0x0024;
            static constexpr reg_t riseVert_2 = 0x0028;

            static constexpr reg_t droopBase_1 = initDropCnt_1;
            static constexpr reg_t droopBase_2 = initDropCnt_2;
            static constexpr reg_t droopDropCntOffset = 0x0000;
            static constexpr reg_t droopRiseHorizOffset = 0x0004;
            static constexpr reg_t droopRiseVertOffset = 0x0008;
        };

        /**
         * @brief      Power Conversion Register Map
         */
        struct powerConvRegMap
        {
            static constexpr reg_t tableBase = 0x0000;
            static constexpr reg_t ctrl = 0x8000;
            static constexpr reg_t fwdOffset = 0x8004;
            static constexpr reg_t revOffset = 0x8008;
            static constexpr reg_t inpOffset = 0x800C;
            static constexpr reg_t unbOffset = 0x8010;

            static constexpr reg_t rmsTimerStatus = 0x8100;
            static constexpr reg_t rmsTimerEnable = 0x8104;
            static constexpr reg_t rmsTimerPowerThreshold = 0x8108;
            static constexpr reg_t rmsTimerCountTop = 0x810C;
            static constexpr reg_t rmsTimerCountThreshold = 0x8110;

            static constexpr reg_t fwdRmsFilterStatus = 0x8120;
            static constexpr reg_t fwdRmsFilteredValue = 0x8124;
            static constexpr reg_t fwdRmsFilterEnable = 0x8128;
            static constexpr reg_t fwdRmsFilterForcePulse = 0x812C;
            static constexpr reg_t fwdRmsFilterForceCw = 0x8130;
            static constexpr reg_t fwdRmsFilterOffset = 0x8134;

            static constexpr reg_t revRmsFilterStatus = 0x8140;
            static constexpr reg_t revRmsFilteredValue = 0x8144;
            static constexpr reg_t revRmsFilterEnable = 0x8148;
            static constexpr reg_t revRmsFilterForcePulse = 0x814C;
            static constexpr reg_t revRmsFilterForceCw = 0x8150;
            static constexpr reg_t revRmsFilterOffset = 0x8154;

            static constexpr reg_t inpRmsFilterStatus = 0x8160;
            static constexpr reg_t inpRmsFilteredValue = 0x8164;
            static constexpr reg_t inpRmsFilterEnable = 0x8168;
            static constexpr reg_t inpRmsFilterForcePulse = 0x816C;
            static constexpr reg_t inpRmsFilterForceCw = 0x8170;
            static constexpr reg_t inpRmsFilterOffset = 0x8174;

            static constexpr reg_t unbRmsFilterStatus = 0x8180;
            static constexpr reg_t unbRmsFilteredValue = 0x8184;
            static constexpr reg_t unbRmsFilterEnable = 0x8188;
            static constexpr reg_t unbRmsFilterForcePulse = 0x818C;
            static constexpr reg_t unbRmsFilterForceCw = 0x8190;
            static constexpr reg_t unbRmsFilterOffset = 0x8194;

            static constexpr reg_t fwdRmsFilterBase = fwdRmsFilterStatus;
            static constexpr reg_t revRmsFilterBase = revRmsFilterStatus;
            static constexpr reg_t inpRmsFilterBase = inpRmsFilterStatus;
            static constexpr reg_t unbRmsFilterBase = unbRmsFilterStatus;
            static constexpr reg_t detRmsFilterStatus = 0x0000;
            static constexpr reg_t detRmsFilteredValue = 0x0004;
            static constexpr reg_t detRmsFilterEnable = 0x0008;
            static constexpr reg_t detRmsFilterForcePulse = 0x000C;
            static constexpr reg_t detRmsFilterForceCw = 0x0010;
            static constexpr reg_t detRmsFilterOffset = 0x0014;
        };

        /**
         * @brief      Peak Detector Register Map
         */
        struct peakDetRegMap
        {
            static constexpr reg_t decay = 0x0000;
            static constexpr reg_t ave_win = 0x0004;
            static constexpr reg_t snap_trig = 0x0008;
            static constexpr reg_t fwd_rms = 0x000C;
            static constexpr reg_t fwd_env = 0x0010;
            static constexpr reg_t rev_rms = 0x0014;
            static constexpr reg_t rev_env = 0x0018;
            static constexpr reg_t inp_rms = 0x001C;
            static constexpr reg_t inp_env = 0x0020;
            static constexpr reg_t unb_rms = 0x0024;
            static constexpr reg_t unb_env = 0x0028;
            static constexpr reg_t fwd_peak = 0x002C;
            static constexpr reg_t rev_peak = 0x0030;
            static constexpr reg_t inp_peak = 0x0034;
            static constexpr reg_t unb_peak = 0x0038;
            static constexpr reg_t fwd_ave_peak = 0x003C;
            static constexpr reg_t rev_ave_peak = 0x0040;
            static constexpr reg_t inp_ave_peak = 0x0044;
            static constexpr reg_t unb_ave_peak = 0x0048;
            static constexpr reg_t fifo_read = 0x004C;
            static constexpr reg_t fifo_src_ctrl = 0x0050;
            static constexpr reg_t fifo_ch_ctrl = 0x0054;
            static constexpr reg_t fifo_dec_fact = 0x0058;
            static constexpr reg_t ave_en = 0x005C;

            static constexpr reg_t base_det_addr = fwd_rms;
        };

        /**
         * @brief      PID Register Map
         */
        struct pidRegMap
        {
            static constexpr reg_t alc_sp = 0x0000;
            static constexpr reg_t agc_sp = 0x0004;
            static constexpr reg_t mgc_sp = 0x0008;
            static constexpr reg_t pid_kp_lg_p = 0x000C;
            static constexpr reg_t pid_ki_lg_p = 0x0010;
            static constexpr reg_t pid_kd_lg_p = 0x0014;
            static constexpr reg_t accumulator = 0x0018;
            static constexpr reg_t map_offs = 0x001C;
            static constexpr reg_t mode_sel = 0x0020;
            // static constexpr reg_t threshold1 = 0x0024; // old names remove after test
            // static constexpr reg_t threshold2 = 0x0028;
            static constexpr reg_t pid_dac_low_limit = 0x0024;
            static constexpr reg_t pid_dac_high_limit = 0x0028;
            static constexpr reg_t pid_alc_dyn_rng = 0x002C;
            static constexpr reg_t chan_sel = 0x0034;
            static constexpr reg_t accum_clr = 0x0038;
            static constexpr reg_t accum_hold_clr = 0x0040;
            static constexpr reg_t accum_hold = 0x0044;
            static constexpr reg_t intg_update_fact = 0x0048;
            static constexpr reg_t diff_update_fact = 0x004C;
            static constexpr reg_t map_out_out = 0x0050;
            static constexpr reg_t pid_mgcsp_db = 0x0054;
            static constexpr reg_t adc_delay = 0x0084;
            static constexpr reg_t accum_start_delay = 0x0088;
            static constexpr reg_t pid_regulation_margin = 0x0090;
            static constexpr reg_t pid_reset_delay = 0x0094;
            static constexpr reg_t pid_inp_chan_margin_high = 0x0098;
            static constexpr reg_t pid_inp_chan_margin_low = 0x009C;
            static constexpr reg_t pid_inp_chan_sel_sushold = 0x00A0;
            static constexpr reg_t pid_lg_sm_thold = 0x00AC;
            static constexpr reg_t pid_kp_sm_p = 0x00B0;
            static constexpr reg_t pid_ki_sm_p = 0x00B4;
            static constexpr reg_t pid_kd_sm_p = 0x00B8;
            static constexpr reg_t pid_frac_bit_size_val = 0x00BC;
            static constexpr reg_t pid_kp_lg_n = 0x00C0;
            static constexpr reg_t pid_ki_lg_n = 0x00C4;
            static constexpr reg_t pid_kd_lg_n = 0x00C8;
            static constexpr reg_t pid_kp_sm_n = 0x00CC;
            static constexpr reg_t pid_ki_sm_n = 0x00D0;
            static constexpr reg_t pid_kd_sm_n = 0x00D4;
            static constexpr reg_t ovr_shoot_cooldwn_drop = 0x00D8;
            static constexpr reg_t ovr_shoot_thold = 0x00DC;
            static constexpr reg_t ovr_shoot_src = 0x00E0;
            static constexpr reg_t ovr_shoot_intg_drop = 0x00E4;
            static constexpr reg_t pid_sum_fault_delay = 0x00E8;
            static constexpr reg_t pid_rf_mode = 0x00F0;
            static constexpr reg_t pid_osck = 0x00F8;
            static constexpr reg_t alc_hw_mode = 0x00FC;
            static constexpr reg_t pid_suspend_level = 0x0114;
            static constexpr reg_t pid_blanking_start_dly = 0x011C;
            static constexpr reg_t pid_blanking_suspend_dly = 0x0120;
            static constexpr reg_t pid_blanking_timeout_dly = 0x0124;
            static constexpr reg_t pid_inp_suspend_dly = 0x0128;
            static constexpr reg_t pid_inp_timeout_dly = 0x012C;
        };

        /**
         * @brief      DMP Register Map
         */
        struct dmpRegMap
        {
            static constexpr reg_t fwd_rms = 0x0000;
            static constexpr reg_t fwd_env = 0x0004;
            static constexpr reg_t rev_rms = 0x0008;
            static constexpr reg_t rev_env = 0x000C;
            static constexpr reg_t inp_rms = 0x0010;
            static constexpr reg_t inp_env = 0x0014;
            static constexpr reg_t unb_rms = 0x0018;
            static constexpr reg_t unb_env = 0x001C;
            static constexpr reg_t fwd_peak = 0x0020;
            static constexpr reg_t rev_peak = 0x0024;
            static constexpr reg_t inp_peak = 0x0028;
            static constexpr reg_t unb_peak = 0x002C;
            static constexpr reg_t snap_trig = 0x0030;
            static constexpr reg_t decimate = 0x0034;
            static constexpr reg_t mux_sel = 0x0038;
            static constexpr reg_t dma_ctrl = 0x003C;
            static constexpr reg_t dma_stat1 = 0x0040;
            static constexpr reg_t dma_stat2 = 0x0044;
            static constexpr reg_t chan12 = 0x0048;
            static constexpr reg_t chan13 = 0x004C;
            static constexpr reg_t chan14 = 0x0050;
            static constexpr reg_t chan15 = 0x0054;
            static constexpr reg_t snapreg_fwd_rms = 0x0058;
            static constexpr reg_t snapreg_fwd_env = 0x005C;
            static constexpr reg_t snapreg_rev_rms = 0x0060;
            static constexpr reg_t snapreg_rev_env = 0x0064;
            static constexpr reg_t snapreg_inp_rms = 0x0068;
            static constexpr reg_t snapreg_inp_env = 0x006C;
            static constexpr reg_t snapreg_unb_rms = 0x0070;
            static constexpr reg_t snapreg_unb_env = 0x0074;
            static constexpr reg_t snapreg_fwd_peak = 0x0078;
            static constexpr reg_t snapreg_rev_peak = 0x007C;
            static constexpr reg_t snapreg_inp_peak = 0x0080;
            static constexpr reg_t snapreg_unb_peak = 0x0084;
            static constexpr reg_t snapreg_trig = 0x0088;
            static constexpr reg_t inp_pres = 0x0100;
            static constexpr reg_t inp_pres_thold = 0x0104;
            static constexpr reg_t inp_pres_hyst_inc = 0x0108;
            static constexpr reg_t inp_pres_hyst_dec = 0x010C;
            static constexpr reg_t inp_pres_hyst_thold = 0x0110;
            static constexpr reg_t inp_pres_hyst_accum = 0x0114;
            static constexpr reg_t inp_pres_stat = 0x0118;
            static constexpr reg_t fwd_pres = 0x0120;
            static constexpr reg_t fwd_pres_thold = 0x0124;
            static constexpr reg_t fwd_pres_hyst_inc = 0x0128;
            static constexpr reg_t fwd_pres_hyst_dec = 0x012C;
            static constexpr reg_t fwd_pres_hyst_thold = 0x0130;
            static constexpr reg_t fwd_pres_hyst_accum = 0x0134;
            static constexpr reg_t fwd_pres_stat = 0x0138;
            static constexpr reg_t fwd_peak_hold = 0x0180;
            static constexpr reg_t rev_peak_hold = 0x0184;
            static constexpr reg_t inp_peak_hold = 0x0188;
            static constexpr reg_t unb_peak_hold = 0x018C;
        };

        /**
         * @brief      SPI Register Map
         */
        struct spiRegMap
        {
            static constexpr reg_t dsa_tgt = 0x0000;
            static constexpr reg_t dsa_cur = 0x0004;
            static constexpr reg_t dsa_dwell = 0x0008;
            static constexpr reg_t dsa_lut_0 = 0x0200;
            static constexpr reg_t dsa_lut_90 = 0x0600;

            static constexpr reg_t dsa_lut_size = 128;
        };

        /**
         * @brief      PSU Register Map
         */
        struct psuRegMap
        {
            static constexpr reg_t ctrl = 0x0000; // PSU Manager
        };

        /**
         * @brief      Built In Test Register Map
         */
        struct bitRegMap
        {
            static constexpr reg_t rf_enable_high_time = 0x0020;
            static constexpr reg_t rf_enable_low_time = 0x0024;
            static constexpr reg_t rf_guard_time = 0x0028;
            static constexpr reg_t rf_modulation_active_time = 0x002C;
            static constexpr reg_t rf_modulation_inactive_time = 0x0030;
            static constexpr reg_t func_gen_enable_select = 0x0034;
            static constexpr reg_t osc_pwm_high_time = 0x0038;
            static constexpr reg_t osc_pwm_low_time = 0x003C;
        };

        /**
         * @brief      System Control Register Map
         */
        struct sysCtrlRegMap
        {
            static constexpr reg_t GPO1_CTRL = 0x0000;
            static constexpr reg_t GPO2_CTRL = 0x0004;
            static constexpr reg_t GPO3_CTRL = 0x0008;
            static constexpr reg_t GPO4_CTRL = 0x000C;
            static constexpr reg_t GPO5_CTRL = 0x0010;
            static constexpr reg_t GPO6_CTRL = 0x0014;
            static constexpr reg_t GPO7_CTRL = 0x0018;
            static constexpr reg_t GPO8_CTRL = 0x001C;
            static constexpr reg_t GPO9_CTRL = 0x0020;
            static constexpr reg_t GPO10_CTRL = 0x0024;
            static constexpr reg_t GPO11_CTRL = 0x0028;
            static constexpr reg_t GPI1_CTRL = 0x0080;
            static constexpr reg_t GPI2_CTRL = 0x0084;
            static constexpr reg_t GPI3_CTRL = 0x0088;
            static constexpr reg_t GPI4_CTRL = 0x008C;
            static constexpr reg_t GPI5_CTRL = 0x0090;
            static constexpr reg_t GPI6_CTRL = 0x0094;
            static constexpr reg_t GPI7_CTRL = 0x0098;
            static constexpr reg_t GPI8_CTRL = 0x009C;
            static constexpr reg_t GPI9_CTRL = 0x00A0;
            static constexpr reg_t GPI10_CTRL = 0x00A4;
            static constexpr reg_t GPI11_CTRL = 0x00A8;
            static constexpr reg_t GPI12_CTRL = 0x00AC;
            static constexpr reg_t GPI13_CTRL = 0x00B0;
            static constexpr reg_t GPI14_CTRL = 0x00B4;
        };

        static constexpr mmData_t mgc_bits = 14;
        static constexpr mmData_t mgc_size = (1 << mgc_bits);
        template<typename T>
        static constexpr T mgc_top_max = mgc_size - 1;

        static constexpr auto defaultPowerTimeout = timeout_t{5000};
        static constexpr auto defaultFpgaTimeout = timeout_t{10000};

        configManagerConnection& configIf;
        statusUpdateInterface updateIf;
        fpgaInterfaceConnection fpgaIf;
        mmData_t mgcTop;
        dbm_t openLoopGain;
        dbm_t vvaRange;
        dbm_t agcRange;
        dbm_t agcRef;
        dbm_t powerFloorValue;
        decay_rate_t peakFastTimeConst;
        decay_rate_t peakSlowTimeConst;
        decay_rate_t peakTauTimeConst;
        bool bitEnableFunctionGeneratorConfig;
        time_duration_t bitMaxPulseWidth;
        percent_t bitMaxDutyCycle;
        bool bitOscPresentConfig;

        std::map<chan_t, dbm_t> channelFloor;
        std::map<port_t, dbm_t> channelCf;

        powerDataSetArray powerReportingDataSet;
        slowDataUpdateArray slowDataSet;

        portMap_t calibrationPortMapping;
        const std::map<chan_t, mmData_t> powerConvToggleMap;

        const std::string rfManagerModName;
        const std::string adcModName;
        const std::string dacModName;
        const std::string peakDetModName;
        const std::string moveToPsModName;
        const std::string pidModName;
        const std::string pwrConvModName;
        const std::string spiModName;
        const std::string psuModName;
        const std::string bitModName;
        const std::string sysCtrlModName;

        const std::map<chan_t, mmData_t> channelMap;
        const gpioKeyArray_t gpioKeyArray;
        rfBandSetting_t rfBandSettings;

    public:
        rfManagerController(zmq::context_t& zmqCtx, configManagerConnection& conf);

        void initializeData();
        void initializeFPGA();
        void initializeDroopCorrection();
        void initializePowerRegulation();
        void initializePowerConversionFiltering();
        void initializePowerConversionTables();
        void initializeDsaStepTables();
        void initializeGpioCrossbar();

        // Normal Getters / Setters
        [[nodiscard]] std::optional<online_state_t> getRfState() noexcept;
        bool setRfState(const online_state_t online) noexcept;
        [[nodiscard]] std::optional<input_select_t> getInputSelect() noexcept;
        bool setInputSelect(const input_select_t state) noexcept;
        [[nodiscard]] std::optional<bool> getDroopCorrectionLine1Enable() noexcept;
        [[nodiscard]] std::optional<bool> getDroopCorrectionLine2Enable() noexcept;
        [[nodiscard]] std::optional<input_switch_t> getInputSwitch() noexcept;
        [[nodiscard]] std::optional<manual_input_switch_t> getManualInputSwitch() noexcept;
        bool setManualInputSwitch(const manual_input_switch_t state) noexcept;
        [[nodiscard]] std::optional<fpga_clock_rate_t> getRfInputSwitchSeqMuteTime() noexcept;
        bool setRfInputSwitchSeqMuteTime(const fpga_clock_rate_t& time) noexcept;
        [[nodiscard]] std::optional<safe_state_t> getSafeState() noexcept;
        bool setSafeState(const safe_state_t safe) noexcept;
        [[nodiscard]] std::optional<rf_band_index_t> getRfBandIndex() noexcept;
        bool setRfBandIndex(const rf_band_index_t bandIndex) noexcept;
        [[nodiscard]] std::optional<gain_mode_t> getGainMode() noexcept;
        bool setGainMode(const gain_mode_t mode) noexcept;
        [[nodiscard]] std::optional<percent_t> getMgcSetpointPercentage() noexcept;
        bool setMgcSetpointPercentage(const percent_t percentage) noexcept;
        [[nodiscard]] std::optional<dbm_t> getMgcSetpointGain() noexcept;
        bool setMgcSetpointGain(const dbm_t gain) noexcept;
        [[nodiscard]] std::optional<mmData_t> getMgcSetpointVva() noexcept;
        bool setMgcSetpointVva(const mmData_t count) noexcept;
        [[nodiscard]] std::optional<dbm_t> getAlcSetpoint() noexcept;
        bool setAlcSetpoint(const dbm_t power) noexcept;
        [[nodiscard]] std::optional<dbm_t> getAgcSetpoint() noexcept;
        bool setAgcSetpoint(const dbm_t gain) noexcept;
        [[nodiscard]] std::optional<signal_t> getBlankingInput() noexcept;
        [[nodiscard]] std::optional<signal_t> getBlankingState() noexcept;
        [[nodiscard]] std::optional<polarity_t> getBlankingPol() noexcept;
        bool setBlankingPol(const polarity_t pol) noexcept;
        [[nodiscard]] std::optional<signal_t> getShutdownInput() noexcept;
        [[nodiscard]] std::optional<signal_t> getShutdownState() noexcept;
        [[nodiscard]] std::optional<polarity_t> getShutdownPol() noexcept;
        bool setShutdownPol(const polarity_t pol) noexcept;
        [[nodiscard]] std::optional<polarity_t> getBlankingOutputPol() noexcept;
        bool setBlankingOutputPol(const polarity_t pol) noexcept;
        [[nodiscard]] std::optional<phase_t> getPhaseCur() noexcept;
        [[nodiscard]] std::optional<phase_t> getPhaseTgt() noexcept;
        bool setPhaseTgt(const phase_t ph) noexcept;
        [[nodiscard]] std::optional<std::chrono::nanoseconds> getPhaseDwell() noexcept;
        bool setPhaseDwell(const std::chrono::nanoseconds dwell) noexcept;
        [[nodiscard]] std::optional<dbm_t> getCfFwd() noexcept;
        bool setCfFwd(const dbm_t pwr) noexcept;
        [[nodiscard]] std::optional<dbm_t> getCfRev() noexcept;
        bool setCfRev(const dbm_t pwr) noexcept;
        [[nodiscard]] std::optional<dbm_t> getCfInp() noexcept;
        bool setCfInp(const dbm_t pwr) noexcept;
        [[nodiscard]] std::optional<dbm_t> getCfUnb() noexcept;
        bool setCfUnb(const dbm_t pwr) noexcept;
        [[nodiscard]] std::optional<dbm_t> getNominalPwr() noexcept;
//        bool setNominalPwr(const dbm_t pwr) noexcept;
        [[nodiscard]] std::optional<detector_t> getRegulationDet() noexcept;
        bool setRegulationDet(const detector_t det) noexcept;
        [[nodiscard]] std::optional<dbm_t> getAlcDynamicRange() noexcept;
        bool setAlcDynamicRange(const dbm_t range) noexcept;
        [[nodiscard]] std::optional<time_duration_t> getPidClockEnable() noexcept;
        bool setPidClockEnable(const time_duration_t& rate) noexcept;
        [[nodiscard]] std::optional<mmData_t> getAlcOvershootDrop() noexcept;
        bool setAlcOvershootDrop(const mmData_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getAlcOvershootIntegratorDrop() noexcept;
        bool setAlcOvershootIntegratorDrop(const mmData_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getAlcOvershootThreshold() noexcept;
        bool setAlcOvershootThreshold(const dbm_t val) noexcept;
        [[nodiscard]] std::optional<detector_t> getAlcOvershootDet() noexcept;
        bool setAlcOvershootDet(const detector_t det) noexcept;
        [[nodiscard]] std::optional<std::size_t> getPidFractionalBits() noexcept;
        [[nodiscard]] decay_rate_t getTauDecayRate() noexcept;
        bool setTauDecayRate(const decay_rate_t tau) noexcept;
        [[nodiscard]] std::optional<signal_t> getInpPres() noexcept;
        [[nodiscard]] std::optional<dbm_t> getInpPresThold() noexcept;
        bool setInpPresThold(const dbm_t thold) noexcept;
        [[nodiscard]] std::optional<mmData_t> getInpPresHystInc() noexcept;
        bool setInpPresHystInc(const mmData_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getInpPresHystDec() noexcept;
        bool setInpPresHystDec(const mmData_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getInpPresHystThold() noexcept;
        bool setInpPresHystThold(const mmData_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getInpPresHystAccum() noexcept;
        [[nodiscard]] std::optional<signal_t> getFwdPres() noexcept;
        [[nodiscard]] std::optional<dbm_t> getFwdPresThold() noexcept;
        bool setFwdPresThold(const dbm_t thold) noexcept;
        [[nodiscard]] std::optional<mmData_t> getFwdPresHystInc() noexcept;
        bool setFwdPresHystInc(const mmData_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getFwdPresHystDec() noexcept;
        bool setFwdPresHystDec(const mmData_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getFwdPresHystThold() noexcept;
        bool setFwdPresHystThold(const mmData_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getFwdPresHystAccum() noexcept;
        [[nodiscard]] std::optional<decay_rate_t> getPeakDecayRate() noexcept;
        bool setPeakDecayRate(const decay_rate_t& val) noexcept;
        [[nodiscard]] std::optional<dbm_t> getPaprBackoff() noexcept;
        bool setPaprBackoff(const dbm_t val) noexcept;
        [[nodiscard]] std::optional<fpga_clock_rate_t> getBitRfEnableHighTime() noexcept;
        bool setBitRfEnableHighTime(const fpga_clock_rate_t& time) noexcept;
        [[nodiscard]] std::optional<fpga_clock_rate_t> getBitRfEnableLowTime() noexcept;
        bool setBitRfEnableLowTime(const fpga_clock_rate_t& time) noexcept;
        [[nodiscard]] std::optional<fpga_clock_rate_t> getBitRfGuardTime() noexcept;
        bool setBitRfGuardTime(const fpga_clock_rate_t& time) noexcept;
        [[nodiscard]] std::optional<fpga_clock_rate_t> getBitRfModulationActiveTime() noexcept;
        bool setBitRfModulationActiveTime(const fpga_clock_rate_t& time) noexcept;
        [[nodiscard]] std::optional<fpga_clock_rate_t> getBitRfModulationInactiveTime() noexcept;
        bool setBitRfModulationInactiveTime(const fpga_clock_rate_t& time) noexcept;
        [[nodiscard]] std::optional<signal_t> getBitFuncGenEnableSelect() noexcept;
        bool setBitFuncGenEnableSelect(const signal_t state) noexcept;
        [[nodiscard]] std::optional<fpga_clock_rate_t> getBitOscPwmHighTime() noexcept;
        bool setBitOscPwmHighTime(const fpga_clock_rate_t& time) noexcept;
        [[nodiscard]] std::optional<fpga_clock_rate_t> getBitOscPwmLowTime() noexcept;
        bool setBitOscPwmLowTime(const fpga_clock_rate_t& time) noexcept;
        bool setBitEnablePulseParams(const fpga_clock_rate_t& pulseWidth, const percent_t dutyCycle) noexcept;
        bool setBitRfModulationParams(const fpga_clock_rate_t& pulseWidth, const percent_t dutyCycle) noexcept;
        bool setDroopCorrectionLine1Enable(bool enable) noexcept;
        bool setDroopCorrectionLine2Enable(bool enable) noexcept;
        bool setRfDroopCorrectionRise(const droopLine_t line, const mmData_t val) noexcept;
        bool setRfDroopCorrectionRun(const droopLine_t line, const mmData_t val) noexcept;
        bool setRfDroopCorrectionDrop(const droopLine_t line, const mmData_t val) noexcept;
        [[nodiscard]] std::optional<dbm_t> getPwrHwFaultBackoff() noexcept;
        [[nodiscard]] std::optional<dbm_t> getPwrSwFaultBackoff() noexcept;
        [[nodiscard]] std::optional<dbm_t> getPwrHwAvail() noexcept;
        [[nodiscard]] std::optional<dbm_t> getPwrAvail() noexcept;
        [[nodiscard]] std::optional<mmData_t> getDacOutput() noexcept;
        [[nodiscard]] std::optional<signal_t> getRmsTimerStatus() noexcept;
        [[nodiscard]] std::optional<signal_t> getRmsTimerEnable() noexcept;
        bool setRmsTimerEnable(const signal_t state) noexcept;
        [[nodiscard]] std::optional<dbm_t> getRmsTimerPowerThold() noexcept;
        bool setRmsTimerPowerThold(const dbm_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getRmsTimerCountTop() noexcept;
        bool setRmsTimerCountTop(const mmData_t val) noexcept;
        [[nodiscard]] std::optional<mmData_t> getRmsTimerCountThold() noexcept;
        bool setRmsTimerCountThold(const mmData_t val) noexcept;
        [[nodiscard]] reg_t powerConversionPortOffset(const port_t port) const noexcept;
        [[nodiscard]] std::optional<signal_t> getRmsFilterStatus(const port_t port) noexcept;
        [[nodiscard]] std::optional<dbm_t> getRmsFilteredValue(const port_t port) noexcept;
        [[nodiscard]] std::optional<signal_t> getRmsFilterEnable(const port_t port) noexcept;
        bool setRmsFilterEnable(const port_t port, const signal_t state) noexcept;
        [[nodiscard]] std::optional<signal_t> getRmsFilterForceCw(const port_t port) noexcept;
        bool setRmsFilterForceCw(const port_t port, const signal_t state) noexcept;
        [[nodiscard]] std::optional<signal_t> getRmsFilterForcePulse(const port_t port) noexcept;
        bool setRmsFilterForcePulse(const port_t port, const signal_t state) noexcept;
        [[nodiscard]] std::optional<dbm_t> getRmsFilterOffset(const port_t port) noexcept;
        bool setRmsFilterOffset(const port_t port, const dbm_t pwr) noexcept;

        // Live Data Getters
        /**
         * @brief      Gets forward rms.
         *
         * @return     Forward rms.
         */
        [[nodiscard]] auto getFwdRms() noexcept { return getTupFwdRms(pollPowerLevels()); }

        /**
         * @brief      Gets forward environment.
         *
         * @return     Forward environment.
         */
        [[nodiscard]] auto getFwdEnv() noexcept { return getTupFwdEnv(pollPowerLevels()); }

        /**
         * @brief      Gets forward peak.
         *
         * @return     Forward peak.
         */
        [[nodiscard]] auto getFwdPeak() noexcept { return getTupFwdPeak(pollPowerLevels()); }

        /**
         * @brief      Gets the reverse rms.
         *
         * @return     The reverse rms.
         */
        [[nodiscard]] auto getRevRms() noexcept { return getTupRevRms(pollPowerLevels()); }

        /**
         * @brief      Gets the reverse environment.
         *
         * @return     The reverse environment.
         */
        [[nodiscard]] auto getRevEnv() noexcept { return getTupRevEnv(pollPowerLevels()); }

        /**
         * @brief      Gets the reverse peak.
         *
         * @return     The reverse peak.
         */
        [[nodiscard]] auto getRevPeak() noexcept { return getTupRevPeak(pollPowerLevels()); }

        /**
         * @brief      Gets the inp rms.
         *
         * @return     The inp rms.
         */
        [[nodiscard]] auto getInpRms() noexcept { return getTupInpRms(pollPowerLevels()); }

        /**
         * @brief      Gets the inp environment.
         *
         * @return     The inp environment.
         */
        [[nodiscard]] auto getInpEnv() noexcept { return getTupInpEnv(pollPowerLevels()); }

        /**
         * @brief      Gets the inp peak.
         *
         * @return     The inp peak.
         */
        [[nodiscard]] auto getInpPeak() noexcept { return getTupInpPeak(pollPowerLevels()); }

        /**
         * @brief      Gets the unb rms.
         *
         * @return     The unb rms.
         */
        [[nodiscard]] auto getUnbRms() noexcept { return getTupUnbRms(pollPowerLevels()); }

        /**
         * @brief      Gets the unb environment.
         *
         * @return     The unb environment.
         */
        [[nodiscard]] auto getUnbEnv() noexcept { return getTupUnbEnv(pollPowerLevels()); }

        /**
         * @brief      Gets the unb peak.
         *
         * @return     The unb peak.
         */
        [[nodiscard]] auto getUnbPeak() noexcept { return getTupUnbPeak(pollPowerLevels()); }

        /**
         * @brief      Gets the vswr rms.
         *
         * @return     The vswr rms.
         */
        [[nodiscard]] auto getVswrRms() noexcept { return getTupVswrRms(pollPowerLevels()); }

        /**
         * @brief      Gets the vswr environment.
         *
         * @return     The vswr environment.
         */
        [[nodiscard]] auto getVswrEnv() noexcept { return getTupVswrEnv(pollPowerLevels()); }

        /**
         * @brief      Gets the vswr peak.
         *
         * @return     The vswr peak.
         */
        [[nodiscard]] auto getVswrPeak() noexcept { return getTupVswrPeak(pollPowerLevels()); }

        /**
         * @brief      Gets forward papr.
         *
         * @return     Forward papr.
         */
        [[nodiscard]] auto getFwdPapr() noexcept { return getTupFwdPapr(pollPowerLevels()); }

        /**
         * @brief      Gets the reverse papr.
         *
         * @return     The reverse papr.
         */
        [[nodiscard]] auto getRevPapr() noexcept { return getTupRevPapr(pollPowerLevels()); }

        /**
         * @brief      Gets the inp papr.
         *
         * @return     The inp papr.
         */
        [[nodiscard]] auto getInpPapr() noexcept { return getTupInpPapr(pollPowerLevels()); }

        /**
         * @brief      Gets the unb papr.
         *
         * @return     The unb papr.
         */
        [[nodiscard]] auto getUnbPapr() noexcept { return getTupUnbPapr(pollPowerLevels()); }

        /**
         * @brief      Gets the compr.
         *
         * @return     The compr.
         */
        [[nodiscard]] auto getCompr() noexcept { return getTupCompr(pollPowerLevels()); }

        /**
         * @brief      Gets the tup forward rms.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup forward rms.
         */
        [[nodiscard]] static constexpr dbm_t getTupFwdRms(const powerTuple& tup) noexcept { return std::get<0>(tup); }

        /**
         * @brief      Gets the tup forward environment.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup forward environment.
         */
        [[nodiscard]] static constexpr dbm_t getTupFwdEnv(const powerTuple& tup) noexcept { return std::get<1>(tup); }

        /**
         * @brief      Gets the tup forward peak.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup forward peak.
         */
        [[nodiscard]] static constexpr dbm_t getTupFwdPeak(const powerTuple& tup) noexcept { return std::get<2>(tup); }

        /**
         * @brief      Gets the tup reverse rms.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup reverse rms.
         */
        [[nodiscard]] static constexpr dbm_t getTupRevRms(const powerTuple& tup) noexcept { return std::get<3>(tup); }

        /**
         * @brief      Gets the tup reverse environment.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup reverse environment.
         */
        [[nodiscard]] static constexpr dbm_t getTupRevEnv(const powerTuple& tup) noexcept { return std::get<4>(tup); }

        /**
         * @brief      Gets the tup reverse peak.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup reverse peak.
         */
        [[nodiscard]] static constexpr dbm_t getTupRevPeak(const powerTuple& tup) noexcept { return std::get<5>(tup); }

        /**
         * @brief      Gets the tup inp rms.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup inp rms.
         */
        [[nodiscard]] static constexpr dbm_t getTupInpRms(const powerTuple& tup) noexcept { return std::get<6>(tup); }

        /**
         * @brief      Gets the tup inp environment.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup inp environment.
         */
        [[nodiscard]] static constexpr dbm_t getTupInpEnv(const powerTuple& tup) noexcept { return std::get<7>(tup); }

        /**
         * @brief      Gets the tup inp peak.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup inp peak.
         */
        [[nodiscard]] static constexpr dbm_t getTupInpPeak(const powerTuple& tup) noexcept { return std::get<8>(tup); }

        /**
         * @brief      Gets the tup unb rms.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup unb rms.
         */
        [[nodiscard]] static constexpr dbm_t getTupUnbRms(const powerTuple& tup) noexcept { return std::get<9>(tup); }

        /**
         * @brief      Gets the tup unb environment.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup unb environment.
         */
        [[nodiscard]] static constexpr dbm_t getTupUnbEnv(const powerTuple& tup) noexcept { return std::get<10>(tup); }

        /**
         * @brief      Gets the tup unb peak.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup unb peak.
         */
        [[nodiscard]] static constexpr dbm_t getTupUnbPeak(const powerTuple& tup) noexcept { return std::get<11>(tup); }

        /**
         * @brief      Gets the tup vswr rms.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup vswr rms.
         */
        [[nodiscard]] static constexpr vswr_t getTupVswrRms(const powerTuple& tup) noexcept { return std::get<12>(tup); }

        /**
         * @brief      Gets the tup vswr environment.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup vswr environment.
         */
        [[nodiscard]] static constexpr vswr_t getTupVswrEnv(const powerTuple& tup) noexcept { return std::get<13>(tup); }

        /**
         * @brief      Gets the tup vswr peak.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup vswr peak.
         */
        [[nodiscard]] static constexpr vswr_t getTupVswrPeak(const powerTuple& tup) noexcept { return std::get<14>(tup); }

        /**
         * @brief      Gets the tup forward papr.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup forward papr.
         */
        [[nodiscard]] static constexpr papr_t getTupFwdPapr(const powerTuple& tup) noexcept { return std::get<15>(tup); }

        /**
         * @brief      Gets the tup reverse papr.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup reverse papr.
         */
        [[nodiscard]] static constexpr papr_t getTupRevPapr(const powerTuple& tup) noexcept { return std::get<16>(tup); }

        /**
         * @brief      Gets the tup inp papr.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup inp papr.
         */
        [[nodiscard]] static constexpr papr_t getTupInpPapr(const powerTuple& tup) noexcept { return std::get<17>(tup); }

        /**
         * @brief      Gets the tup unb papr.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup unb papr.
         */
        [[nodiscard]] static constexpr papr_t getTupUnbPapr(const powerTuple& tup) noexcept { return std::get<18>(tup); }

        /**
         * @brief      Gets the tup compr.
         *
         * @param[in]  tup   The tup
         *
         * @return     The tup compr.
         */
        [[nodiscard]] static constexpr compr_t getTupCompr(const powerTuple& tup) noexcept { return std::get<19>(tup); }

        [[nodiscard]] std::optional<adc_raw_t> readRawAdc(const chan_t& chan) noexcept;

        // Polling Routines
        powerTuple pollPowerLevels() noexcept;
        void pollFpgaData() noexcept;
    };
}

#endif
