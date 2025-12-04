#pragma once
#include <cstdint>

static constexpr std::uint32_t  FpgaBase = 0x80000000;
static constexpr size_t  FpgaMemSize = 0x00200000;  // 512*4096

// Inventory  Manager

class FpgaInventory {
public:
    static constexpr std::uint32_t  offset    =   0x000000000;
    // registers defs
    static constexpr std::uint32_t    softwareVers = 0x0000;
    static constexpr std::uint32_t    firmwareVers = 0x0004;
};


// Fault Manager
class FpgaFaultManager
{
    public:
        static constexpr std::uint32_t  offset   =   0x00090000;
        // registers defs
        static constexpr std::uint32_t    battleShort   = 0x0000;
            enum class BattleShort { NORMAL, MASKALL};
        static constexpr std::uint32_t   faultClear     = 0x0004;
            enum class FaultClear { NORMAL, CLEARALL};
        static constexpr std::uint32_t   softwareMute   = 0x0008;
            enum class  SoftwareMure { NORMAL, MUTE};
        static constexpr std::uint32_t   summaryFaultInputCtrl   = 0x000C;
            enum class  SummaryFaultACTION { NORMAL, MUTE};
        static constexpr std::uint32_t   summaryFaultInputStatus   = 0x0010;
            enum class SummaryFaultInputStatus { INACTIVE, ACTIVE};
        static constexpr std::uint32_t   summaryFaultOutputCtrl    = 0x0014;
        static constexpr std::uint32_t   summaryFaultOutputStatus  = 0x0018;
        static constexpr std::uint32_t  dutyCycleBackOffCurrent   = 0x001C;
};


// PSU Manager CTRL reg
class FpgaPSU {
    public:
        static constexpr std::uint32_t  offset   =   0x00160000;
        // registers defs
        static constexpr std::uint32_t    ctrl = 0x0000;
};


// RF Manager Module  offset and regs keeping same names as in RF manager
class FpgaRfManager {
     public:
        static constexpr std::uint32_t   offset   =   0x00170000;
         // registers defs
        static constexpr std::uint32_t  inpSel = 0x0000;
        static constexpr std::uint32_t  rfEnable = 0x0004;
        static constexpr std::uint32_t  phyReset = 0x0008;
        static constexpr std::uint32_t  safeState = 0x000C;
        static constexpr std::uint32_t  blankPol = 0x0010;
        static constexpr std::uint32_t  blankTst = 0x0014;
        static constexpr std::uint32_t  blankStat = 0x0018;
        static constexpr std::uint32_t  shutdownPol = 0x001C;
        static constexpr std::uint32_t  shutdownTst = 0x0020;
        static constexpr std::uint32_t  shutdownStat = 0x0024;
        static constexpr std::uint32_t  pwrHwrFltBckoff = 0x0028; // Fault Manager
        static constexpr std::uint32_t  pwrSwrFltBckoff = 0x002C; // Fault Manager
        static constexpr std::uint32_t  pwrHwrAvail = 0x0030;
        static constexpr std::uint32_t  pwrNominal = 0x0034;
        static constexpr std::uint32_t  pwrAvail = 0x0040;
        static constexpr std::uint32_t  modProfBckoff = 0x0044; // Fault Manager
        static constexpr std::uint32_t  dutyCycleBckoff = 0x0048; // Fault Manager
        static constexpr std::uint32_t  softwareMute = 0x004C;
        static constexpr std::uint32_t  inputSwitchControl = 0x0050;
        static constexpr std::uint32_t  blankingInExtStat = 0x0054;
        static constexpr std::uint32_t  shutdownInExtStat = 0x0058;
        static constexpr std::uint32_t  blankOutputPol = 0x005C;
        static constexpr std::uint32_t  manualInputSwitchCtrl = 0x0064;
        static constexpr std::uint32_t  rfInpSwitchSeqMuteTime = 0x006C;
};

// PID Controller Module  offset and regs keeping same names
class FpgaPID {
     public:
        static constexpr std::uint32_t  offset   =   0x00110000;
        // registers defs
        static constexpr std::uint32_t  alc_sp = 0x0000;
        static constexpr std::uint32_t  agc_sp = 0x0004;
        static constexpr std::uint32_t  mgc_sp = 0x0008;
        static constexpr std::uint32_t  pid_kp_lg_p = 0x000C;
        static constexpr std::uint32_t  pid_ki_lg_p = 0x0010;
        static constexpr std::uint32_t  pid_kd_lg_p = 0x0014;
        static constexpr std::uint32_t  accumulator = 0x0018;
        static constexpr std::uint32_t  map_offs = 0x001C;
        static constexpr std::uint32_t  mode_sel = 0x0020;
        static constexpr std::uint32_t  threshold_1 = 0x0024;
        static constexpr std::uint32_t  threshold_2 = 0x0028;
        static constexpr std::uint32_t  pid_alc_dyn_rng = 0x002C;
        static constexpr std::uint32_t  chan_sel = 0x0034;
            enum class DetectorType { ENV, RMS, PEAK, PEAK_FAST, PEAK_SLOW };
        static constexpr std::uint32_t  accum_clr = 0x0038;
        static constexpr std::uint32_t  accum_hold_clr = 0x0040;
        static constexpr std::uint32_t  accum_hold = 0x0044;
        static constexpr std::uint32_t  intg_update_fact = 0x0048;
        static constexpr std::uint32_t  diff_update_fact = 0x004C;
        static constexpr std::uint32_t  map_out_out = 0x0050;
        static constexpr std::uint32_t  pid_mgcsp_db = 0x0054;
        static constexpr std::uint32_t  adc_delay = 0x0084;
        static constexpr std::uint32_t  accum_start_delay = 0x0088;
        static constexpr std::uint32_t  pid_regulation_margin = 0x0090;
        static constexpr std::uint32_t  pid_reset_delay = 0x0094;
        static constexpr std::uint32_t  pid_inp_chan_margin_high = 0x0098;
        static constexpr std::uint32_t  pid_inp_chan_margin_low = 0x009C;
        static constexpr std::uint32_t  pid_inp_chan_sel_sushold = 0x00A0;
        static constexpr std::uint32_t  pid_lg_sm_thold = 0x00AC;
        static constexpr std::uint32_t  pid_kp_sm_p = 0x00B0;
        static constexpr std::uint32_t  pid_ki_sm_p = 0x00B4;
        static constexpr std::uint32_t  pid_kd_sm_p = 0x00B8;
        static constexpr std::uint32_t  pid_frac_bit_size_val = 0x00BC;
        static constexpr std::uint32_t  pid_kp_lg_n = 0x00C0;
        static constexpr std::uint32_t  pid_ki_lg_n = 0x00C4;
        static constexpr std::uint32_t  pid_kd_lg_n = 0x00C8;
        static constexpr std::uint32_t  pid_kp_sm_n = 0x00CC;
        static constexpr std::uint32_t  pid_ki_sm_n = 0x00D0;
        static constexpr std::uint32_t  pid_kd_sm_n = 0x00D4;
        static constexpr std::uint32_t  ovr_shoot_cooldwn_drop = 0x00D8;
        static constexpr std::uint32_t  ovr_shoot_thold = 0x00DC;
        static constexpr std::uint32_t  ovr_shoot_src = 0x00E0;
        static constexpr std::uint32_t  ovr_shoot_intg_drop = 0x00E4;
};

// Power Conversion Module   offset and regs keeping same names
class FpgaPower {
    public:
    static constexpr std::uint32_t offset = 0x00120000;
    static constexpr std::uint32_t tableBase = 0x0000;
    static constexpr std::uint32_t ctrl = 0x8000;
    static constexpr std::uint32_t fwdOffset = 0x8004;
    static constexpr std::uint32_t revOffset = 0x8008;
    static constexpr std::uint32_t inpOffset = 0x800C;
    static constexpr std::uint32_t unbOffset = 0x8010;

    static constexpr std::uint32_t rmsTimerStatus = 0x8100;
    static constexpr std::uint32_t rmsTimerEnable = 0x8104;
    static constexpr std::uint32_t rmsTimerPowerThreshold = 0x8108;
    static constexpr std::uint32_t rmsTimerCountTop = 0x810C;
    static constexpr std::uint32_t rmsTimerCountThreshold = 0x8110;

    static constexpr std::uint32_t fwdRmsFilterStatus = 0x8120;
    static constexpr std::uint32_t fwdRmsFilteredValue = 0x8124;
    static constexpr std::uint32_t fwdRmsFilterEnable = 0x8128;
    static constexpr std::uint32_t fwdRmsFilterForcePulse = 0x812C;
    static constexpr std::uint32_t fwdRmsFilterForceCw = 0x8130;
    static constexpr std::uint32_t fwdRmsFilterOffset = 0x8134;

    static constexpr std::uint32_t revRmsFilterStatus = 0x8140;
    static constexpr std::uint32_t revRmsFilteredValue = 0x8144;
    static constexpr std::uint32_t revRmsFilterEnable = 0x8148;
    static constexpr std::uint32_t revRmsFilterForcePulse = 0x814C;
    static constexpr std::uint32_t revRmsFilterForceCw = 0x8150;
    static constexpr std::uint32_t revRmsFilterOffset = 0x8154;

    static constexpr std::uint32_t inpRmsFilterStatus = 0x8160;
    static constexpr std::uint32_t inpRmsFilteredValue = 0x8164;
    static constexpr std::uint32_t inpRmsFilterEnable = 0x8168;
    static constexpr std::uint32_t inpRmsFilterForcePulse = 0x816C;
    static constexpr std::uint32_t inpRmsFilterForceCw = 0x8170;
    static constexpr std::uint32_t inpRmsFilterOffset = 0x8174;

    static constexpr std::uint32_t unbRmsFilterStatus = 0x8180;
    static constexpr std::uint32_t unbRmsFilteredValue = 0x8184;
    static constexpr std::uint32_t unbRmsFilterEnable = 0x8188;
    static constexpr std::uint32_t unbRmsFilterForcePulse = 0x818C;
    static constexpr std::uint32_t unbRmsFilterForceCw = 0x8190;
    static constexpr std::uint32_t unbRmsFilterOffset = 0x8194;

    static constexpr std::uint32_t fwdRmsFilterBase = fwdRmsFilterStatus;
    static constexpr std::uint32_t revRmsFilterBase = revRmsFilterStatus;
    static constexpr std::uint32_t inpRmsFilterBase = inpRmsFilterStatus;
    static constexpr std::uint32_t unbRmsFilterBase = unbRmsFilterStatus;
    static constexpr std::uint32_t detRmsFilterStatus = 0x0000;
    static constexpr std::uint32_t detRmsFilteredValue = 0x0004;
    static constexpr std::uint32_t detRmsFilterEnable = 0x0008;
    static constexpr std::uint32_t detRmsFilterForcePulse = 0x000C;
    static constexpr std::uint32_t detRmsFilterForceCw = 0x0010;
    static constexpr std::uint32_t detRmsFilterOffset = 0x0014;
};

class FpgaDecMoveToPS  {
public:
    static constexpr std::uint32_t offset = 0x00140000;
    static constexpr std::uint32_t  fwd_rms = 0x0000;
    static constexpr std::uint32_t  fwd_env = 0x0004;
    static constexpr std::uint32_t  rev_rms = 0x0008;
    static constexpr std::uint32_t  rev_env = 0x000C;
    static constexpr std::uint32_t  inp_rms = 0x0010;
    static constexpr std::uint32_t  inp_env = 0x0014;
    static constexpr std::uint32_t  unb_rms = 0x0018;
    static constexpr std::uint32_t  unb_env = 0x001C;
    static constexpr std::uint32_t  fwd_peak = 0x0020;
    static constexpr std::uint32_t  rev_peak = 0x0024;
    static constexpr std::uint32_t  inp_peak = 0x0028;
    static constexpr std::uint32_t  unb_peak = 0x002C;
    static constexpr std::uint32_t  snap_trig = 0x0030;
    static constexpr std::uint32_t  decimate = 0x0034;
    static constexpr std::uint32_t  mux_sel = 0x0038;
    static constexpr std::uint32_t  dma_ctrl = 0x003C;
    static constexpr std::uint32_t  dma_stat1 = 0x0040;
    static constexpr std::uint32_t  dma_stat2 = 0x0044;
    static constexpr std::uint32_t  chan12 = 0x0048;
    static constexpr std::uint32_t  chan13 = 0x004C;
    static constexpr std::uint32_t  chan14 = 0x0050;
    static constexpr std::uint32_t  chan15 = 0x0054;
    static constexpr std::uint32_t  snapreg_fwd_rms = 0x0058;
    static constexpr std::uint32_t  snapreg_fwd_env = 0x005C;
    static constexpr std::uint32_t  snapreg_rev_rms = 0x0060;
    static constexpr std::uint32_t  snapreg_rev_env = 0x0064;
    static constexpr std::uint32_t  snapreg_inp_rms = 0x0068;
    static constexpr std::uint32_t  snapreg_inp_env = 0x006C;
    static constexpr std::uint32_t  snapreg_unb_rms = 0x0070;
    static constexpr std::uint32_t  snapreg_unb_env = 0x0074;
    static constexpr std::uint32_t  snapreg_fwd_peak = 0x0078;
    static constexpr std::uint32_t  snapreg_rev_peak = 0x007C;
    static constexpr std::uint32_t  snapreg_inp_peak = 0x0080;
    static constexpr std::uint32_t  snapreg_unb_peak = 0x0084;
    static constexpr std::uint32_t  snapreg_trig = 0x0088;
    static constexpr std::uint32_t  inp_pres = 0x0100;
    static constexpr std::uint32_t  inp_pres_thold = 0x0104;
    static constexpr std::uint32_t  inp_pres_hyst_inc = 0x0108;
    static constexpr std::uint32_t  inp_pres_hyst_dec = 0x010C;
    static constexpr std::uint32_t  inp_pres_hyst_thold = 0x0110;
    static constexpr std::uint32_t  inp_pres_hyst_accum = 0x0114;
    static constexpr std::uint32_t  inp_pres_stat = 0x0118;
    static constexpr std::uint32_t  fwd_pres = 0x0120;
    static constexpr std::uint32_t  fwd_pres_thold = 0x0124;
    static constexpr std::uint32_t  fwd_pres_hyst_inc = 0x0128;
    static constexpr std::uint32_t  fwd_pres_hyst_dec = 0x012C;
    static constexpr std::uint32_t  fwd_pres_hyst_thold = 0x0130;
    static constexpr std::uint32_t  fwd_pres_hyst_accum = 0x0134;
    static constexpr std::uint32_t  fwd_pres_stat = 0x0138;
    static constexpr std::uint32_t  fwd_peak_hold = 0x0180;
    static constexpr std::uint32_t  rev_peak_hold = 0x0184;
    static constexpr std::uint32_t  inp_peak_hold = 0x0188;
    static constexpr std::uint32_t  unb_peak_hold = 0x018C;
};
