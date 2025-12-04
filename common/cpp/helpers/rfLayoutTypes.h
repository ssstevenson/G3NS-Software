#ifndef RF_LAYOUT_TYPES_H_
#define RF_LAYOUT_TYPES_H_

#include <array>
#include <configManagerConnection/configManagerConnection.h>
#include <helpers/types.h>
#include <limits>
#include <logger/logger.h>
#include <numeric>
#include <set>
#include <tuple>
#include <unordered_map>

namespace empower::helpers::types
{
    enum class bands_t: std::size_t {
        A = 0, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T,
        U, V, W, X, Y, Z, AA, BB, CC, DD, EE, FF, END
    };

    enum class pa_enable_t: mmData_t {
        NONE = 0, A, B, C, D, E, F, G, END
    };

    enum class online_state_t: mmData_t { ON, HOT, COLD };
    enum class safe_state_t: bool { SAFE, RUNNING };
    enum class source_select_t: bool { SW, HW };
    enum class polarity_t: bool { ACTIVE_HIGH, ACTIVE_LOW };
    enum class input_select_t: bool { INPUT, BIT };
    enum class input_switch_t: bool { INPUT, LOAD };
    enum class manual_input_switch_t: mmData_t { INPUT, LOAD, NONE };
    enum class signal_t: bool { ACTIVE, INACTIVE };
    enum class pmod_protocol_t: std::uint32_t { XMEGA_128_1, XMEGA_256_1, KISS_1 };
    enum class switch_function_t: std::size_t { TR, DUMMY };

    typedef std::tuple<dbm_t, dbm_t, dbm_t, dbm_t, freq_t> offset_point_t;
    // ^^^            <Fwd,   Rev,   Inp,   Unb,   Cutoff> ^^^
    constexpr std::size_t OFFSET_POINTS_PER_PATH = 8;   // This coresponds with 3 bits
    typedef std::array<offset_point_t, OFFSET_POINTS_PER_PATH> band_offsets_t;

    using pmod_addr_t = ser_data_t;
    typedef std::tuple<mmData_t, reg_t, bool> hwr_config_t;
    // ^^^ <Hardware Fault Line, Error LUT Index, Inverted> ^^^
    typedef std::tuple<pmod_addr_t, hwr_config_t, pmod_protocol_t> pmod_dev_t;
    typedef std::set<pmod_dev_t> pallet_addrs_t;
    typedef std::pair<pmod_dev_t, pallet_addrs_t> dvr_grp_t;
    typedef std::set<dvr_grp_t> driver_groups_t;
    typedef std::tuple<pa_enable_t, driver_groups_t, band_offsets_t> rf_path_t;
    typedef std::unordered_map<bands_t, rf_path_t> rf_bands_t;

    using pmod_bits_t = std::bitset<10>;

    typedef std::unordered_map<pmod_addr_t, std::string> pmod_device_keys_t;
    typedef std::unordered_map<pa_enable_t, pmod_bits_t> pa_en_pmod_bits_t;

    /**
     * @brief      Converts a Band Type to a numeric index
     *
     * @param[in]  b     The Band
     *
     * @return     The numeric index for the given Band.
     */
    [[nodiscard]] constexpr auto bandTypeIndex(const bands_t b)
    {
        return std::clamp<const std::underlying_type<bands_t>::type>(
            static_cast<std::underlying_type<bands_t>::type>(b) -
            static_cast<std::underlying_type<bands_t>::type>(bands_t::A), 0, 31);
    }

    /**
     * @brief      Extract the Bus Address from a PMOD Device.
     *
     * @param[in]  dev   The device.
     *
     * @return     The RS-485 Bus Address
     */
    [[nodiscard]] constexpr auto pmodDevToBusAddr(const pmod_dev_t& dev)
    {
        return std::get<0>(dev);
    }

    /**
     * @brief      Extract the Device Configuration for a given PMOD Device.
     *
     * @param[in]  dev   The device.
     *
     * @return     The Device Configuration for the given PMOD.
     */
    [[nodiscard]] constexpr auto pmodDevToHwrConfig(const pmod_dev_t& dev)
    {
        return std::get<1>(dev);
    }

    /**
     * @brief      Extracts the Protocol for a given PMOD Device.
     *
     * @param[in]  dev   The device.
     *
     * @return     The Protocol for the given PMOD.
     */
    [[nodiscard]] constexpr auto pmodDevToProtocol(const pmod_dev_t& dev)
    {
        return std::get<2>(dev);
    }

    /**
     * @brief      Extract the PMOD Fault Line from a given PMOD Hardware Configuration
     *
     * @param[in]  cfg   The configuration
     *
     * @return     The PMOD Fault Line
     */
    [[nodiscard]] constexpr auto hwrConfigToFaultLine(const hwr_config_t& cfg)
    {
        return std::get<0>(cfg);
    }

    /**
     * @brief      Extract the Error LUT Index from a given PMOD Hardware Configuration
     *
     * @param[in]  cfg   The configuration
     *
     * @return     The Error LUT Index
     */
    [[nodiscard]] constexpr auto hwrConfigToErrLutIdx(const hwr_config_t& cfg)
    {
        return std::get<1>(cfg);
    }

    /**
     * @brief      Extract the Inverted signal state from a given PMOD Hardware Configuration
     *
     * @param[in]  cfg   The configuration
     *
     * @return     The Inverted State
     */
    [[nodiscard]] constexpr auto hwrConfigToInverted(const hwr_config_t& cfg)
    {
        return std::get<2>(cfg);
    }

    /**
     * @brief      Extract the PMOD Driver from a given Driver Group
     *
     * @param[in]  grp   The group
     *
     * @return     The PMOD Driver
     */
    [[nodiscard]] constexpr auto dvrGrpToDvrDev(const dvr_grp_t& grp)
    {
        return std::get<0>(grp);
    }

    /**
     * @brief      Extract the PMOD Pallets from a given Driver Group
     *
     * @param[in]  grp   The group
     *
     * @return     The PMOD Pallets
     */
    [[nodiscard]] inline auto dvrGrpToPalletDevs(const dvr_grp_t& grp)
    {
        return std::get<1>(grp);
    }

    /**
     * @brief      Determines if a structure has an rf band.
     *
     * @param[in]  band       The band
     * @param[in]  structure  The structure
     *
     * @return     True if band, False otherwise.
     */
    [[nodiscard]] inline auto hasBand(const bands_t& band, const rf_bands_t& structure)
    {
        return structure.count(band) > 0;
    }

    /**
     * @brief      Returns the Register Value for a given PA Enable
     *
     * @param[in]  en    The PA Enable
     *
     * @return     The Register Value
     */
    [[nodiscard]] inline auto paEnToRegVal(const pa_enable_t& en)
    {
        static const std::unordered_map<pa_enable_t, mmData_t> conversion = {
            {pa_enable_t::A, 1}, {pa_enable_t::B, 2}, {pa_enable_t::C, 3}, {pa_enable_t::D, 4},
            {pa_enable_t::E, 5}, {pa_enable_t::F, 6}, {pa_enable_t::G, 7},
            {pa_enable_t::NONE, 0}, {pa_enable_t::END, 8}
        };

        return conversion.at(en);
    }

    /**
     * @brief      Returns a PA Enable type given a Register Value
     *
     * @param[in]  en    The Register Value
     *
     * @return     The PA Enable type
     */
    [[nodiscard]] inline auto regValToPaEn(const mmData_t& en)
    {
        static const std::unordered_map<mmData_t, pa_enable_t> conversion = {
            {1, pa_enable_t::A}, {2, pa_enable_t::B}, {3, pa_enable_t::C}, {4, pa_enable_t::D},
            {5, pa_enable_t::E}, {6, pa_enable_t::F}, {7, pa_enable_t::G},
            {0, pa_enable_t::NONE}, {8, pa_enable_t::END}
        };

        std::unordered_map<mmData_t, pa_enable_t>::const_iterator it = conversion.find(en);

        if(it == std::end(conversion))
        {
            return pa_enable_t::END;
        }

        return it->second;
    }

    /**
     * @brief      Converts an Online State to the RF Register Value
     *
     * @param[in]  state  The state
     *
     * @return     RF Register value for the given Online State.
     */
    [[nodiscard]] constexpr auto onlineStateToRfRegVal(const online_state_t state)
    {
        return ((online_state_t::ON == state)?(mmData_t{1}):(mmData_t{0}));
    }

    /**
     * @brief      Converts an Online State to the PSU Register Value
     *
     * @param[in]  state  The state
     *
     * @return     PSU Register value for the given Online State
     */
    [[nodiscard]] constexpr auto onlineStateToPsuRegVal(const online_state_t state)
    {
        return ((online_state_t::COLD == state)?(mmData_t{0}):(mmData_t{1}));
    }

    /**
     * @brief      Converts an Online State to a String representing that value
     *
     * @param[in]  state  The state
     *
     * @return     String representation
     */
    [[nodiscard]] inline auto onlineStateToStr(const online_state_t state)
    {
        static const std::unordered_map<online_state_t, std::string> conversion = {
            {online_state_t::ON, "ON"}, {online_state_t::HOT, "HOT"}, {online_state_t::COLD, "COLD"}
        };

        return conversion.at(state);
    }

    /**
     * @brief      Converts an Online State to a Command String representing that value
     *
     * @param[in]  state  The state
     *
     * @return     Command String representation
     */
    [[nodiscard]] inline auto onlineStateToCmdStr(const online_state_t state)
    {
        static const std::unordered_map<online_state_t, std::string> conversion = {
            {online_state_t::ON, "O"}, {online_state_t::HOT, "S"}, {online_state_t::COLD, "C"}
        };

        return conversion.at(state);
    }

    /**
     * @brief      Converts a String representation to an Online State value
     *
     * @param[in]  str   The string
     *
     * @return     The Online State the string represents.
     */
    [[nodiscard]] inline auto strToOnlineState(const std::string& str)
    {
        auto result{online_state_t::COLD};
        switch(std::toupper(str.front()))
        {
        case 'O':
            result = online_state_t::ON;
            break;
        case 'H':
            result = online_state_t::HOT;
            break;
        case 'C':
        default:
            break;
        }

        return result;
    }

    /**
     * @brief      Converts RF and PSU control register values to an Online State
     *
     * @param[in]  rfCtrl   The rf control
     * @param[in]  psuCtrl  The psu control
     *
     * @return     The Online State
     */
    [[nodiscard]] constexpr auto regValToOnlineState(const mmData_t rfCtrl, const mmData_t psuCtrl)
    {
        return ((0 != rfCtrl)?(online_state_t::ON):(((0 != psuCtrl)?(online_state_t::HOT):(online_state_t::COLD))));
    }

    /**
     * @brief      Converts the Safe State to a Register value
     *
     * @param[in]  safe  The safe state
     *
     * @return     Register Value representing the Safe State
     */
    [[nodiscard]] constexpr auto safeStateToRegVal(const safe_state_t safe)
    {
        return ((safe_state_t::RUNNING == safe)?(mmData_t{0}):(mmData_t{1}));
    }

    /**
     * @brief      Converts the Safe State to a String representation
     *
     * @param[in]  safe  The safe state
     *
     * @return     The String representing the given Safe State
     */
    [[nodiscard]] inline auto safeStateToStr(const safe_state_t safe)
    {
        static const std::unordered_map<safe_state_t, std::string> conversion = {
            {safe_state_t::SAFE, "SAFE"}, {safe_state_t::RUNNING, "RUNNING"}
        };

        return conversion.at(safe);
    }

    /**
     * @brief      Converts a Register Value to the Safe State
     *
     * @param[in]  reg   The register value
     *
     * @return     Safe State represented by the given Register Value
     */
    [[nodiscard]] constexpr auto regValToSafeState(const mmData_t reg)
    {
        return ((0 != reg)?(safe_state_t::SAFE):(safe_state_t::RUNNING));
    }

    /**
     * @brief      Converts the Source Select to a Register Value
     *
     * @param[in]  src   The source select
     *
     * @return     Register Value representing the given Source Select
     */
    [[nodiscard]] constexpr auto srcSelToRegVal(const source_select_t src)
    {
        return ((source_select_t::SW == src)?(mmData_t{1}):(mmData_t{0}));
    }

    /**
     * @brief      Converts the Switch Source Select to a Register Value
     *
     * @param[in]  src   The source select
     *
     * @return     Register Value representing the given Switch Source Select
     */
    constexpr auto switchSrcSelToRegVal(const source_select_t src)
    {
        return ((source_select_t::SW == src)?(mmData_t{1}):(mmData_t{2}));
    }

    /**
     * @brief      Returns a String representation of the given Source Select
     *
     * @param[in]  src   The source select
     *
     * @return     String representation of the given Source Select
     */
    [[nodiscard]] inline auto srcSelToStr(const source_select_t src)
    {
        static const std::unordered_map<source_select_t, std::string> conversion = {
            {source_select_t::SW, "SW"}, {source_select_t::HW, "HW"}
        };

        return conversion.at(src);
    }

    /**
     * @brief      Returns the Source Select of the given String representation
     *
     * @param[in]  str   The string
     *
     * @return     The Source Select
     */
    [[nodiscard]] inline auto strToSrcSel(const std::string& str)
    {
        static const std::unordered_map<std::string, source_select_t> conversion = {
            {"SW", source_select_t::SW}, {"HW", source_select_t::HW}
        };

        std::unordered_map<std::string, source_select_t>::const_iterator it = conversion.find(str);

        if(it == std::end(conversion))
        {
            return source_select_t::SW;
        }

        return it->second;
    }

    /**
     * @brief      Converts the Register Value to a particular Source Select
     *
     * @param[in]  reg   The register
     *
     * @return     Source Selection based on the given Register Value
     */
    [[nodiscard]] constexpr auto regValToSrcSel(const mmData_t reg)
    {
        return ((0 != reg)?(source_select_t::SW):(source_select_t::HW));
    }

    /**
     * @brief      Converts the Register Value to a particular Switch Source Select
     *
     * @param[in]  reg   The register
     *
     * @return     Switch Source Selection based on the given Register Value
     */
    [[nodiscard]] constexpr auto regValToSwitchSrcSel(const mmData_t reg)
    {
        return ((0x00000002 == reg)?(source_select_t::HW):(source_select_t::SW));
    }

    /**
     * @brief      Converts a Polarity state to a Register Value
     *
     * @param[in]  pol   The polarity
     *
     * @return     Register Value representing the given Polarity
     */
    [[nodiscard]] constexpr auto polToRegVal(const polarity_t pol)
    {
        return ((polarity_t::ACTIVE_HIGH == pol)?(mmData_t{1}):(mmData_t{0}));
    }

    /**
     * @brief      Converts a Polarity to a String representation
     *
     * @param[in]  pol   The polarity
     *
     * @return     String representation of the polarity.
     */
    [[nodiscard]] inline auto polToStr(const polarity_t pol)
    {
        static const std::unordered_map<polarity_t, std::string> conversion = {
            {polarity_t::ACTIVE_HIGH, "HIGH"}, {polarity_t::ACTIVE_LOW, "LOW"}
        };

        return conversion.at(pol);
    }

    /**
     * @brief      Converts a String representation to a particular Polarity
     *
     * @param[in]  str   The string
     *
     * @return     The Polarity represented by the given String.
     */
    [[nodiscard]] inline auto strToPol(const std::string& str)
    {
        static const std::unordered_map<std::string, polarity_t> conversion = {
            {"HIGH", polarity_t::ACTIVE_HIGH}, {"LOW", polarity_t::ACTIVE_LOW}
        };

        std::unordered_map<std::string, polarity_t>::const_iterator it = conversion.find(str);

        if(it == std::end(conversion))
        {
            return polarity_t::ACTIVE_HIGH;
        }

        return it->second;
    }

    /**
     * @brief      Converts Register Value to a Polarity
     *
     * @param[in]  reg   The register value
     *
     * @return     The Polarity represented by the given Register Value
     */
    [[nodiscard]] constexpr auto regValToPol(const mmData_t reg)
    {
        return ((0 != reg)?(polarity_t::ACTIVE_HIGH):(polarity_t::ACTIVE_LOW));
    }

    /**
     * @brief      Converts an Input Select state to a Register Value
     *
     * @param[in]  sw    The Input Select
     *
     * @return     The Register Value represented by the given Input Select state
     */
    [[nodiscard]] constexpr auto inpSelToRegVal(const input_select_t sw)
    {
        return ((input_select_t::INPUT == sw)?(mmData_t{1}):(mmData_t{0}));
    }

    /**
     * @brief      Converts the Input Select to a String representation
     *
     * @param[in]  sw    The input select
     *
     * @return     String representation of the given Input Select
     */
    [[nodiscard]] inline auto inpSelToStr(const input_select_t sw)
    {
        static const std::unordered_map<input_select_t, std::string> conversion = {
            {input_select_t::INPUT, "INPUT"}, {input_select_t::BIT, "BIT"}
        };

        return conversion.at(sw);
    }

    /**
     * @brief      Converts a String representation to an Input Select state
     *
     * @param[in]  str   The string
     *
     * @return     Input Select represented by the given String
     */
    [[nodiscard]] inline auto strToInpSel(const std::string& str)
    {
        static const std::unordered_map<std::string, input_select_t> conversion = {
            {"INPUT", input_select_t::INPUT}, {"BIT", input_select_t::BIT}
        };

        std::unordered_map<std::string, input_select_t>::const_iterator it = conversion.find(str);

        if(it == std::end(conversion))
        {
            return input_select_t::BIT;
        }

        return it->second;
    }

    /**
     * @brief      Converts a Register Value to an Input Select
     *
     * @param[in]  reg   The register value
     *
     * @return     Input Select state represented by the given Register Value
     */
    [[nodiscard]] constexpr auto regValToInpSel(const mmData_t reg)
    {
        return ((0 != reg)?(input_select_t::INPUT):(input_select_t::BIT));
    }

    /**
     * @brief      Converts an Input Switch state to a Register Value
     *
     * @param[in]  sw    The Input Switch state
     *
     * @return     Register Value represnting the given Input Switch state
     */
    [[nodiscard]] constexpr auto inpSwitchToRegVal(const input_switch_t sw)
    {
        return ((input_switch_t::INPUT == sw)?(mmData_t{1}):(mmData_t{0}));
    }

    /**
     * @brief      Converts an Input Switch to a String
     *
     * @param[in]  sw    The Input Switch state
     *
     * @return     String representing the given Input Switch state
     */
    [[nodiscard]] inline auto inpSwitchToStr(const input_switch_t sw)
    {
        static const std::unordered_map<input_switch_t, std::string> conversion = {
            {input_switch_t::INPUT, "INPUT"}, {input_switch_t::LOAD, "LOAD"}
        };

        return conversion.at(sw);
    }

    /**
     * @brief      Converts a String representation to an input Switch
     *
     * @param[in]  str   The string
     *
     * @return     Input Switch represented by the given String
     */
    [[nodiscard]] inline auto strToInpSwitch(const std::string& str)
    {
        static const std::unordered_map<std::string, input_switch_t> conversion = {
            {"INPUT", input_switch_t::INPUT}, {"LOAD", input_switch_t::LOAD}
        };

        std::unordered_map<std::string, input_switch_t>::const_iterator it = conversion.find(str);

        if(it == std::end(conversion))
        {
            return input_switch_t::LOAD;
        }

        return it->second;
    }

    /**
     * @brief      Converts a Register Value to an Input Switch
     *
     * @param[in]  reg   The register
     *
     * @return     Input Switch represented by the Register Value
     */
    [[nodiscard]] constexpr auto regValToInpSwitch(const mmData_t reg)
    {
        return ((0 != reg)?(input_switch_t::INPUT):(input_switch_t::LOAD));
    }

    /**
     * @brief      Converts a Register Value to a Manual Input Switch
     *
     * @param[in]  reg   The register
     *
     * @return     Manual Input Switch represented by the Register Value
     */
    [[nodiscard]] constexpr auto regValToManualInpSwitch(const mmData_t reg)
    {
        auto resp{manual_input_switch_t::NONE};

        if(reg == 0b10)
        {
            resp = manual_input_switch_t::INPUT;
        }
        else if(reg == 0b01)
        {
            resp = manual_input_switch_t::LOAD;
        }

        return resp;
    }

    /**
     * @brief      Converts a Manual Input Switch to a Register Value
     *
     * @param[in]  sw    The Switch
     *
     * @return     Register Value represented by the Manual Input Switch
     */
    [[nodiscard]] constexpr auto manualInpSwitchToRegVal(const manual_input_switch_t sw)
    {
        mmData_t resp{0};

        if(sw == manual_input_switch_t::INPUT)
        {
            resp = 0b10;
        }
        else if(sw == manual_input_switch_t::LOAD)
        {
            resp = 0b01;
        }

        return resp;
    }

    /**
     * @brief      Converts a Manual Input Switch to readable String Value
     *
     * @param[in]  sw    The Switch
     *
     * @return     A String representing the Manual Input Switch Value
     */
    [[nodiscard]] inline auto manualInpSwitchToStr(const manual_input_switch_t sw)
    {
        static const std::unordered_map<manual_input_switch_t, std::string> conversion = {
            {manual_input_switch_t::INPUT, "MANUAL_INPUT"},
            {manual_input_switch_t::LOAD, "MANUAL_LOAD"},
            {manual_input_switch_t::NONE, "MANUAL_NONE"}
        };

        return conversion.at(sw);
    }

    /**
     * @brief      Converts a Signal value to a Register value
     *
     * @param[in]  sig   The signal
     *
     * @return     Register Value represented by the Signal
     */
    [[nodiscard]] constexpr auto sigToRegVal(const signal_t sig)
    {
        return ((signal_t::ACTIVE == sig)?(mmData_t{1}):(mmData_t{0}));
    }

    /**
     * @brief      Converts a Signal value to a Boolean
     *
     * @param[in]  sig   The signal
     *
     * @return     True if the signal is Active, False otherwise.
     */
    [[nodiscard]] constexpr auto sigToBool(const signal_t sig)
    {
        return (signal_t::ACTIVE == sig);
    }

    /**
     * @brief      Converts a Boolean value to a Signal
     *
     * @param[in]  b     The boolean value
     *
     * @return     ACTIVE if the boolean is True, INACTIVE otherwise.
     */
    [[nodiscard]] constexpr auto boolToSig(const bool b)
    {
        return ((true == b)?(signal_t::ACTIVE):(signal_t::INACTIVE));
    }

    /**
     * @brief      Returns a string representation of a signal as Active or Inactive.
     *
     * @param[in]  sig   The signal
     *
     * @return     String representation of the signal.
     */
    [[nodiscard]] inline auto sigToStrActive(const signal_t sig)
    {
        static const std::unordered_map<signal_t, std::string> conversion = {
            {signal_t::ACTIVE, "ACTIVE"}, {signal_t::INACTIVE, "INACTIVE"}
        };

        return conversion.at(sig);
    }

    /**
     * @brief      Returns a string representation of a signal as High or Low.
     *
     * @param[in]  sig   The signal
     *
     * @return     String representation of the signal.
     */
    [[nodiscard]] inline auto sigToStrHigh(const signal_t sig)
    {
        static const std::unordered_map<signal_t, std::string> conversion = {
            {signal_t::ACTIVE, "HIGH"}, {signal_t::INACTIVE, "LOW"}
        };

        return conversion.at(sig);
    }

    /**
     * @brief      Returns a string representation of a signal as Internal or External for BIT Input Selection Switch.
     *
     * @param[in]  sig   The signal
     *
     * @return     String representation of the signal.
     */
    [[nodiscard]] inline auto sigToStrBitIntExt(const signal_t sig)
    {
        static const std::unordered_map<signal_t, std::string> conversion = {
            {signal_t::ACTIVE, "1: Internal Generator"}, {signal_t::INACTIVE, "0: External/Customer Input"}
        };

        return conversion.at(sig);
    }

    /**
     * @brief      Converts a String to a Signal
     *
     * @param[in]  str   The string
     *
     * @return     The Signal represented by the given String
     */
    [[nodiscard]] inline auto strToSig(const std::string& str)
    {
        static const std::unordered_map<std::string, signal_t> conversion = {
            {"ACTIVE", signal_t::ACTIVE}, {"INACTIVE", signal_t::INACTIVE}
        };

        std::unordered_map<std::string, signal_t>::const_iterator it = conversion.find(str);

        if(it == std::end(conversion))
        {
            return signal_t::INACTIVE;
        }

        return it->second;
    }

    /**
     * @brief      Converts a String to a Signal
     *
     * @param[in]  str   The string
     *
     * @return     The Signal represented by the given String
     */
    [[nodiscard]] inline auto strToSigBitIntExt(const std::string& str)
    {
        static const std::unordered_map<std::string, signal_t> conversion = {
            {"INTERNAL", signal_t::ACTIVE}, {"EXTERNAL", signal_t::INACTIVE}
        };

        std::unordered_map<std::string, signal_t>::const_iterator it = conversion.find(str);

        if(it == std::end(conversion))
        {
            return signal_t::INACTIVE;
        }

        return it->second;
    }

    /**
     * @brief      Converts a Register Value to a Signal
     *
     * @param[in]  reg   The register value
     *
     * @return     Signal representative of the given Regiser value.
     */
    [[nodiscard]] constexpr auto regValToSig(const mmData_t reg)
    {
        return ((0 != reg)?(signal_t::ACTIVE):(signal_t::INACTIVE));
    }

    /**
     * @brief      Extracts the PA Enable for a given RF Path
     *
     * @param[in]  path  The path
     *
     * @return     PA Enable
     */
    [[nodiscard]] constexpr auto rfPathToPaEn(const rf_path_t& path)
    {
        return std::get<0>(path);
    }

    /**
     * @brief      Extracts a Driver Group from an RF Path
     *
     * @param[in]  path  The path
     *
     * @return     Driver Group
     */
    [[nodiscard]] inline auto rfPathToDvrGrp(const rf_path_t& path)
    {
        return std::get<1>(path);
    }

    /**
     * @brief      Extracts the Power Conversion Offsets for a given RF Path
     *
     * @param[in]  path  The RF Path
     *
     * @return     Power Conversion Offsets for the given RF Path
     */
    [[nodiscard]] constexpr auto rfPathToOffsets(const rf_path_t& path)
    {
        return std::get<2>(path);
    }

    /**
     * @brief      Finds an offset.
     *
     * @param[in]  off   The offset
     * @param[in]  idx   The index
     *
     * @return     Offset value for a given Index
     */
    [[nodiscard]] constexpr auto& findOffset(const band_offsets_t& off, const std::size_t idx = 0)
    {
        if(idx > OFFSET_POINTS_PER_PATH)
        {
            return off.at(0);
        }
        return off.at(idx);
    }

    /**
     * @brief      Finds an offset given the Center Frequency and signal Bandwidth.
     *
     * @param[in]  off   The Offset
     * @param[in]  cf    Center Frequency
     * @param[in]  bw    Signal Bandwidth
     *
     * @return     The appropriate Offset value for the given Center Frequency and Bandwidth
     */
    [[nodiscard]] inline auto& findOffset(const band_offsets_t& off, const freq_t& cf, const freq_t& bw = 0.0)
    {
        return *std::lower_bound(std::begin(off), std::end(off), (cf + (bw / 2.0)),
            [](const auto& lhs, const auto& rhs){ return std::get<4>(lhs) < rhs; });
    }

    /**
     * @brief      Extract the Forward channel Offset
     *
     * @param[in]  off   The Offsets
     *
     * @return     Forward Channel Offset
     */
    [[nodiscard]] constexpr auto offsetToFwdOff(const offset_point_t& off)
    {
        return std::get<0>(off);
    }

    /**
     * @brief      Extract the Reverse channel Offset
     *
     * @param[in]  off   The Offsets
     *
     * @return     Reverse Channel Offset
     */
    [[nodiscard]] constexpr auto offsetToRevOff(const offset_point_t& off)
    {
        return std::get<1>(off);
    }

    /**
     * @brief      Extract the Input channel Offset
     *
     * @param[in]  off   The Offsets
     *
     * @return     Input Channel Offset
     */
    [[nodiscard]] constexpr auto offsetToInpOff(const offset_point_t& off)
    {
        return std::get<2>(off);
    }

    /**
     * @brief      Extract the Unbalanced channel Offset
     *
     * @param[in]  off   The Offsets
     *
     * @return     Unbalanced Channel Offset
     */
    [[nodiscard]] constexpr auto offsetToUnbOff(const offset_point_t& off)
    {
        return std::get<3>(off);
    }

    /**
     * @brief      Extract the Cutoff Frequency from the Offset Point
     *
     * @param[in]  off   The Offset point
     *
     * @return     Cutoff Frequency
     */
    [[nodiscard]] constexpr auto offsetToFreqOff(const offset_point_t& off)
    {
        return std::get<4>(off);
    }

    /**
     * @brief      Extracts all of the Drivers from a given RF Path
     *
     * @param[in]  path  The path
     *
     * @return     The Drivers in the given Path
     */
    [[nodiscard]] inline auto rfPathToDvrs(const rf_path_t& path)
    {
        pallet_addrs_t result;
        auto dvrGrp{rfPathToDvrGrp(path)};
        std::transform(std::begin(dvrGrp), std::end(dvrGrp), std::inserter(result, std::end(result)),
            [](const auto& val){ return val.first;});
        return result;
    }

    /**
     * @brief      Extracts all of the Pallets from a given RF Path
     *
     * @param[in]  path  The path
     *
     * @return     The Pallets in the given Path
     */
    [[nodiscard]] inline auto rfPathToPallets(const rf_path_t& path)
    {
        pallet_addrs_t result;
        auto dvrGrp{rfPathToDvrGrp(path)};
        std::for_each(std::begin(dvrGrp), std::end(dvrGrp),
            [&result](const auto& data){ result.insert(std::begin(std::get<1>(data)), std::end(std::get<1>(data))); });
        return result;
    }

    /**
     * @brief      Extracts all of the Pallets and Drivers from a given RF Path
     *
     * @param[in]  path  The path
     *
     * @return     The PMOD Devices associated with the given RF Path
     */
    [[nodiscard]] inline auto rfPathToActivePmod(const rf_path_t& path)
    {
        const pallet_addrs_t drivers{rfPathToDvrs(path)};
        const pallet_addrs_t pallets{rfPathToPallets(path)};
        pallet_addrs_t result;

        std::set_union(
            std::begin(drivers), std::end(drivers),
            std::begin(pallets), std::end(pallets),
            std::inserter(result, std::begin(result))
        );

        return result;
    }

    /**
     * @brief      Extracts bitmap of the PMOD Error bits for use in the FPGA from a given RF Path
     *
     * @param[in]  path  The path
     *
     * @return     The bitmap of the PMOD Error bits
     */
    [[nodiscard]] inline auto rfPathToPmodBits(const rf_path_t& path)
    {
        pmod_bits_t result(0x3FF);

        for(const auto& dev: rfPathToActivePmod(path))
        {
            result.reset(hwrConfigToErrLutIdx(pmodDevToHwrConfig(dev)));
        }

        return result;
    }

    /**
     * @brief      Extracts bitmap of the PMOD PA Enable bits for use in the FPGA from a given RF Path.
     *
     * @param[in]  structure  The structure
     *
     * @return     The bitmap of the PMOD PA Enable bits.
     */
    [[nodiscard]] inline auto getAllPaEnPmodBits(const rf_bands_t& structure)
    {
        pa_en_pmod_bits_t result{
            {pa_enable_t::A, 0x3FF}, {pa_enable_t::B, 0x3FF}, {pa_enable_t::C, 0x3FF}, {pa_enable_t::D, 0x3FF},
            {pa_enable_t::E, 0x3FF}, {pa_enable_t::F, 0x3FF}, {pa_enable_t::G, 0x3FF}, {pa_enable_t::NONE, 0x3FF}
        };

        for(const auto& kv: structure)
        {
            const auto path{std::get<1>(kv)};
            result.at(rfPathToPaEn(path)) = rfPathToPmodBits(path);
        }

        return result;
    }

    /**
     * @brief      Determines the RF Path for a given band in a given RF Band Structure
     *
     * @param[in]  band       The band
     * @param[in]  structure  The structure
     *
     * @return     The RF Path
     */
    [[nodiscard]] inline auto bandToRfPath(const bands_t& band, const rf_bands_t& structure)
    {
        auto res{std::begin(structure)->second};
        if(hasBand(band, structure))
        {
            res = structure.at(band);
        }
        return res;
    }

    /**
     * @brief      Gets all PMODs in the system.
     *
     * @param[in]  structure  The RF Bands structure
     *
     * @return     All PMODs.
     */
    [[nodiscard]] inline auto getAllPmods(const rf_bands_t& structure)
    {
        pallet_addrs_t result;

        for(const auto& kv: structure)
        {
            const auto path{std::get<1>(kv)};
            const pallet_addrs_t drivers{rfPathToDvrs(path)};
            const pallet_addrs_t pallets{rfPathToPallets(path)};

            std::set_union(
                std::begin(drivers), std::end(drivers),
                std::begin(pallets), std::end(pallets),
                std::inserter(result, std::end(result))
            );
        }

        return result;
    }

    /**
     * @brief      Returns a PMOD Device for a given Bus Address
     *
     * @param[in]  allPmods  All pmods
     * @param[in]  addr      The bus address
     *
     * @return     The PMOD Device for the given Bus Address
     */
    [[nodiscard]] inline auto pmodAddrToDevice(const pallet_addrs_t& allPmods, const pmod_addr_t& addr)
    {
        pmod_dev_t result;

        for(const auto& dev: allPmods)
        {
            if(pmodDevToBusAddr(dev) == addr)
            {
                result = dev;
            }
        }

        return result;
    }

    /**
     * @brief      Returns a string representation of a band.
     *
     * @param[in]  b     The band
     *
     * @return     String representation of the band.
     */
    [[nodiscard]] inline auto bandToStr(const bands_t& b)
    {
        static const std::unordered_map<bands_t, std::string> conversion = {
            {bands_t::A, "A"}, {bands_t::B, "B"}, {bands_t::C, "C"}, {bands_t::D, "D"},
            {bands_t::E, "E"}, {bands_t::F, "F"}, {bands_t::G, "G"}, {bands_t::H, "H"},
            {bands_t::I, "I"}, {bands_t::J, "J"}, {bands_t::K, "K"}, {bands_t::L, "L"},
            {bands_t::M, "M"}, {bands_t::N, "N"}, {bands_t::O, "O"}, {bands_t::P, "P"},
            {bands_t::Q, "Q"}, {bands_t::R, "R"}, {bands_t::S, "S"}, {bands_t::T, "T"},
            {bands_t::U, "U"}, {bands_t::V, "V"}, {bands_t::W, "W"}, {bands_t::X, "X"},
            {bands_t::Y, "Y"}, {bands_t::Z, "Z"}, {bands_t::AA, "AA"}, {bands_t::BB, "BB"},
            {bands_t::CC, "CC"}, {bands_t::DD, "DD"}, {bands_t::EE, "EE"}, {bands_t::FF, "FF"}
        };

        return conversion.at(b);
    }

    /**
     * @brief      Returns a band from a given string representation
     *
     * @param[in]  str   The string
     *
     * @return     The band
     */
    [[nodiscard]] inline auto strToBand(const std::string& str)
    {
        static const std::unordered_map<std::string, bands_t> conversion = {
            {"A", bands_t::A}, {"B", bands_t::B}, {"C", bands_t::C}, {"D", bands_t::D},
            {"E", bands_t::E}, {"F", bands_t::F}, {"G", bands_t::G}, {"H", bands_t::H},
            {"I", bands_t::I}, {"J", bands_t::J}, {"K", bands_t::K}, {"L", bands_t::L},
            {"M", bands_t::M}, {"N", bands_t::N}, {"O", bands_t::O}, {"P", bands_t::P},
            {"Q", bands_t::Q}, {"R", bands_t::R}, {"S", bands_t::S}, {"T", bands_t::T},
            {"U", bands_t::U}, {"V", bands_t::V}, {"W", bands_t::W}, {"X", bands_t::X},
            {"Y", bands_t::Y}, {"Z", bands_t::Z}, {"AA", bands_t::AA}, {"BB", bands_t::BB},
            {"CC", bands_t::CC}, {"DD", bands_t::DD}, {"EE", bands_t::EE}, {"FF", bands_t::FF},
        };

        std::string testStr;
        bands_t result{bands_t::FF};
        std::transform(std::begin(str), std::end(str), std::back_inserter(testStr),
            [](const auto& ch){ return std::toupper(ch); });

        if(std::unordered_map<std::string, bands_t>::const_iterator it = conversion.find(testStr);
            it != std::end(conversion))
        {
            result = it->second;
        }

        return result;
    }

    /**
     * @brief      Converts a Register Value to a Band Offset
     *
     * @param[in]  val   The register value
     *
     * @return     Band Offset pair for the given Register Value
     */
    [[nodiscard]] constexpr auto regValToBandOffset(const mmData_t val)
    {
        bands_t band = static_cast<bands_t>(val / OFFSET_POINTS_PER_PATH);
        std::size_t offset = (val % OFFSET_POINTS_PER_PATH);

        return std::make_pair(band, offset);
    }

    /**
     * @brief      Returns a String representation of a given Band Offset
     *
     * @param[in]  val   The register value
     *
     * @return     String Representation of Band/Offset pair
     */
    [[nodiscard]] inline auto bandOffsetToStr(const mmData_t val) -> std::string
    {
        const auto bandOff{regValToBandOffset(val)};
        return bandToStr(std::get<0>(bandOff)) + std::string("/") + std::to_string(std::get<1>(bandOff));
    }

    /**
     * @brief      Converts a Configuration String to a PMOD Protocol Enumeration
     *
     * @param[in]  val   The Configuration String
     *
     * @return     The PMOD Protocol Enumeration
     */
    [[nodiscard]] inline auto strToPmodProtocol(const std::string& str)
    {
        static const std::unordered_map<std::string, pmod_protocol_t> conversion = {
            {"XMEGA_128_1", pmod_protocol_t::XMEGA_128_1}, {"XMEGA_256_1", pmod_protocol_t::XMEGA_256_1},
            {"KISS_1", pmod_protocol_t::KISS_1},
        };

        std::string testStr;
        pmod_protocol_t result{pmod_protocol_t::XMEGA_256_1};
        std::transform(std::begin(str), std::end(str), std::back_inserter(testStr),
            [](const auto& ch){ return std::toupper(ch); });

        if(std::unordered_map<std::string, pmod_protocol_t>::const_iterator it = conversion.find(testStr);
            it != std::end(conversion))
        {
            result = it->second;
        }

        return result;
    }

    /**
     * @brief      Convert a Switch Function Enum type to a String
     *
     * @param[in]  swFunc  The Switch Function
     *
     * @return     String representation of the given Switch Function
     */
    inline auto switchFunctionToStr(const switch_function_t& swFunc)
    {
        static const std::unordered_map<switch_function_t, std::string> conversion = {
            {switch_function_t::TR, "TR"}, {switch_function_t::DUMMY, "DUMMY"}
        };

        return conversion.at(swFunc);
    }

    /**
     * @brief      Convert a String to a Switch Function
     *
     * @param[in]  str   The string
     *
     * @return     The Switch Function represented by the String
     */
    inline auto strToSwitchFunction(const std::string& str)
    {
        static const std::unordered_map<std::string, switch_function_t> conversion = {
            {"TR", switch_function_t::TR}, {"DUMMY", switch_function_t::DUMMY}
        };

        std::string testStr;
        auto result{switch_function_t::TR};
        std::transform(std::begin(str), std::end(str), std::back_inserter(testStr),
            [](const auto& ch){ return std::toupper(ch); });

        if(std::unordered_map<std::string, switch_function_t>::const_iterator it = conversion.find(testStr);
            it != std::end(conversion))
        {
            result = it->second;
        }

        return result;
    }

    /**
     * @brief      Gets the rf layout structure from configuration.
     *
     * @param      configIf  The configuration if
     *
     * @return     The rf layout structure.
     */
    [[nodiscard]] inline auto getRfLayoutStructure(configManagerConnection& configIf)
    {
        using std::string_literals::operator""s;

        rf_bands_t result;

        try
        {
            std::size_t idx{0};

            auto bandCnt = strToNum<std::size_t>(
                configIf.getParam<std::string>("INV_RF_BAND_COUNT").value_or(""), 10).value_or(0);

            std::generate_n(std::inserter(result, std::begin(result)), bandCnt,
                [&idx](){ return std::make_pair(static_cast<bands_t>(idx++), rf_path_t()); });

            for(auto& [key, val]: result)
            {

                auto rfPathVal = configIf.getParam<std::string>("INV_RF_BAND_"s +
                    std::to_string(static_cast<std::size_t>(key) + 1) + "_RF_PATH"s).value_or("");
                auto rfPath = "INV_RF_PATH_"s + rfPathVal + "_"s;

                std::get<0>(val) = regValToPaEn(strToNum<mmData_t>(
                    configIf.getParam<std::string>(rfPath + "PA_EN"s).value_or(""), 10).value_or(0));

                auto dvrGrpsVec = dataTokenizer<pmod_addr_t>(
                    configIf.getParam<std::string>(rfPath + "DVR_GRP"s).value_or(""), ",", 10);

                for(auto& dvrGrp: dvrGrpsVec)
                {
                    auto dvrGrpStr{"INV_RF_DVR_GRP_" + std::to_string(dvrGrp)};

                    auto dvrAddr = strToNum<pmod_addr_t>(
                        configIf.getParam<std::string>(dvrGrpStr + "_ADDR"s).value_or(""), 10).value_or(0);
                    auto dvrFault = strToNum<mmData_t>(
                        configIf.getParam<std::string>(dvrGrpStr + "_FAULT"s).value_or(""), 10).value_or(0);
                    auto dvrIndex = strToNum<reg_t>(
                        configIf.getParam<std::string>(dvrGrpStr + "_INDEX"s).value_or(""), 10).value_or(0);
                    auto dvrInvert = strToNum<bool>(
                        configIf.getParam<std::string>(dvrGrpStr + "_INVERT"s).value_or(""), 10).value_or(0);
                    auto palletAddrVec = dataTokenizer<pmod_addr_t>(configIf.getParam<std::string>(dvrGrpStr +
                        "_PALLET_ADDR"s).value_or(""), ",", 10);
                    auto palletFaultVec = dataTokenizer<mmData_t>(configIf.getParam<std::string>(dvrGrpStr +
                        "_PALLET_FAULT"s).value_or(""), ",", 10);
                    auto palletIndexVec = dataTokenizer<reg_t>(configIf.getParam<std::string>(dvrGrpStr +
                        "_PALLET_INDEX"s).value_or(""), ",", 10);
                    auto palletInvertVec = dataTokenizer<bool>(configIf.getParam<std::string>(dvrGrpStr +
                        "_PALLET_INVERT"s).value_or(""), ",", 10);
                    auto driverProtocol = strToPmodProtocol(
                        configIf.getParam<std::string>(dvrGrpStr + "_DVR_PROTOCOL"s).value_or("XMEGA_256_1"));
                    auto palletProtocol = strToPmodProtocol(
                        configIf.getParam<std::string>(dvrGrpStr + "_PALLET_PROTOCOL"s).value_or("XMEGA_256_1"));

                    pallet_addrs_t palletDataVec;
                    std::vector<hwr_config_t> hwrConfigVec;
                    std::vector<std::size_t> palletConfigs(palletFaultVec.size());
                    std::iota(std::begin(palletConfigs), std::end(palletConfigs), 0);

                    std::transform(std::begin(palletConfigs), std::end(palletConfigs), std::back_inserter(hwrConfigVec),
                        [&palletFaultVec, &palletIndexVec, &palletInvertVec](const auto& i){
                            return std::make_tuple(palletFaultVec.at(i), palletIndexVec.at(i), palletInvertVec.at(i));
                        });

                    std::transform(std::begin(palletAddrVec), std::end(palletAddrVec), std::begin(hwrConfigVec),
                        std::inserter(palletDataVec, std::end(palletDataVec)),
                        [palletProtocol](auto& addr, auto& hwr){ return pmod_dev_t{addr, hwr, palletProtocol}; }
                    );

                    std::get<1>(val).emplace(std::make_pair(
                        pmod_dev_t{dvrAddr, std::make_tuple(dvrFault, dvrIndex, dvrInvert), driverProtocol},
                        palletDataVec)
                    );
                }

                // This is for the offsets...
                auto& offsetsArray{std::get<2>(val)};
                const auto offsetStr{rfPath + "OFF_"};

                auto fwd = dataTokenizer<dbm_t>(configIf.getParam<std::string>(offsetStr + "FWD"s).value_or(""));
                auto rev = dataTokenizer<dbm_t>(configIf.getParam<std::string>(offsetStr + "REV"s).value_or(""));
                auto inp = dataTokenizer<dbm_t>(configIf.getParam<std::string>(offsetStr + "INP"s).value_or(""));
                auto unb = dataTokenizer<dbm_t>(configIf.getParam<std::string>(offsetStr + "UNB"s).value_or(""));
                auto freq = dataTokenizer<freq_t>(configIf.getParam<std::string>(offsetStr + "FREQ"s).value_or(""));

                auto dataSizes = {fwd.size(), rev.size(), inp.size(), unb.size(), freq.size()};
                const auto isCorrectSize = [tmp = OFFSET_POINTS_PER_PATH](const auto s){
                    return s == tmp; };

                if(!std::all_of(std::begin(dataSizes), std::end(dataSizes), isCorrectSize))
                {
                    const auto resetToSize = [tmp = OFFSET_POINTS_PER_PATH](auto& vec){
                        vec.resize(tmp);
                        std::fill_n(std::begin(vec), tmp, vec.at(0));
                    };

                    resetToSize(fwd);
                    resetToSize(rev);
                    resetToSize(inp);
                    resetToSize(unb);
                    resetToSize(freq);
                }

                for(std::size_t pIdx = 0; pIdx < OFFSET_POINTS_PER_PATH; ++pIdx)
                {
                    offsetsArray.at(pIdx) = {fwd.at(pIdx), rev.at(pIdx), inp.at(pIdx), unb.at(pIdx), freq.at(pIdx)};
                }
            }
        }
        catch(const std::exception& e)
        {
            logger::critical(__FILE__, __FUNCTION__,
                std::string("Failed to get parameter from Configuration Manager - ") + std::string(e.what()));
            std::terminate();
        }

        return result;
    }

    /**
     * @brief      Gets the device keys for the given PMOD Addresses given from the Configuration
     *
     * @param      configIf  The configuration interface
     * @param[in]  addrs     The addrs
     *
     * @return     The device keys.
     */
    [[nodiscard]] inline auto getDeviceKeys(configManagerConnection& configIf, const pallet_addrs_t& addrs)
    {
        pmod_device_keys_t result;

        try
        {
            std::transform(std::begin(addrs), std::end(addrs), std::inserter(result, std::end(result)),
                [&configIf](auto& a){
                    return std::make_pair(pmodDevToBusAddr(a), configIf.getParam<std::string>(
                        std::string("INV_RF_PMOD_NAME_") + std::to_string(pmodDevToBusAddr(a))).value_or(""));
                });
        }
        catch(const std::exception& e)
        {
            logger::critical(__FILE__, __FUNCTION__,
                std::string("Failed to get parameter from Configuration Manager - ") + std::string(e.what()));
            std::terminate();
        }

        return result;
    }
}

#endif
