#ifndef TYPES_H_
#define TYPES_H_

#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <map>
#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <zmq.hpp>

namespace empower::helpers::types
{
    // Enumerated Types
    enum class detector_t: std::size_t { ENV, RMS, PEAK, PEAK_FAST, PEAK_SLOW };
    enum class port_t: std::size_t { IN, FWD, REF, UNBAL };
    enum class droopLine_t: std::size_t { LINE_1, LINE_2 };

    // Base Type Definitions
    using dbm_t = double;
    using dbm_raw_t = std::int16_t;
    using mmData_t = std::uint32_t;
    using pwm_t = std::uint32_t;
    using reg_t = std::uint64_t;
    using ser_data_t = std::uint8_t;
    using ser_ext_data_t = std::uint16_t;
    using temp_t = std::int32_t;
    using timeout_t = std::chrono::milliseconds;
    using percent_t = double;
    using freq_t = double;
    using phase_t = std::uint32_t;
    using rf_band_index_t = std::uint8_t;
    using decay_rate_t = std::chrono::duration<double, std::milli>;
    constexpr std::intmax_t fpga_clk_period_ns = 10;
    using fpga_period_t = std::ratio_multiply<std::ratio<fpga_clk_period_ns, 1>, std::nano>;
    using fpga_clock_rate_t = std::chrono::duration<mmData_t, fpga_period_t>;
    using time_duration_t = std::chrono::duration<double>;

    // Derived Type Definitions
    using adc_raw_t = mmData_t;
    using vswr_t = dbm_t;
    using papr_t = dbm_t;
    using compr_t = dbm_t;
    using watts_t = dbm_t;
    using chan_t = std::pair<port_t, detector_t>;
    typedef std::pair<dbm_t, adc_raw_t> calDataPoint_t;
    typedef std::vector<calDataPoint_t> calVec_t;
    typedef std::map<chan_t, calVec_t> portMap_t;

    enum class gain_mode_t: mmData_t { ALC, AGC, MGC };

    // Struct Types
    /**
     * @brief      This structure is used to push status data
     */
    struct updateIfData_t {
        std::string key;
        std::string label;
        std::variant<bool, std::string, double, float, int, unsigned, std::int64_t, std::uint64_t> data;
        std::string units;
        timeout_t timeout;
        bool valid;

        updateIfData_t():
            key{""},
            label{""},
            data{0},
            units{""},
            timeout{0},
            valid{false}
        {}

        updateIfData_t(const std::string& k, const std::string& l, const std::string& u, const timeout_t& t):
            key{k},
            label{l},
            data{0},
            units{u},
            timeout{t},
            valid{false}
        {}
    };

    using data_set_t = std::vector<updateIfData_t>;

    // Helper Methods
    /**
     * @brief      Helper to fill updateIf structure
     *
     * @param      data     The data
     * @param[in]  key      The key
     * @param[in]  label    The label
     * @param[in]  units    The units
     * @param[in]  timeout  The timeout
     */
    inline void updateIfDataFill(updateIfData_t& data, const std::string& key, \
        const std::string& label, const std::string& units, const timeout_t& timeout)
    {
        data.key = key;
        data.label = label;
        data.units = units;
        data.timeout = timeout;
        data.valid = false;
    }

    /**
     * @brief      Extracts a port_t from a chan_t
     *
     * @param[in]  c     Channel
     *
     * @return     The associated port_t
     */
    [[nodiscard]] constexpr auto chanToPort(const chan_t& c)
    {
        return c.first;
    }

    /**
     * @brief      Sets the Port of a given Channel
     *
     * @param      c     Channel to be set
     * @param[in]  p     Port to be used
     */
    constexpr void chanSetPort(chan_t& c, const port_t& p)
    {
        c.first = p;
    }

    /**
     * @brief      Extracts a detector_t from a chan_t
     *
     * @param[in]  c     Channel
     *
     * @return     The associated detector_t
     */
    [[nodiscard]] constexpr auto chanToDetector(const chan_t& c)
    {
        return c.second;
    }

    /**
     * @brief      Sets the Detector of a given Channel
     *
     * @param      c     Channel to be set
     * @param[in]  d     Detector to be used
     */
    constexpr void chanSetDetector(chan_t& c, const detector_t& d)
    {
        c.second = d;
    }

    /**
     * @brief      Takes a Clibration Data point, and extracts the dBm Floating Point value
     *
     * @param[in]  dp    Calibration Data point to be used
     *
     * @return     dBm Floating Point value for this point.
     */
    [[nodiscard]] constexpr auto calDataPointToDbmF(const calDataPoint_t& dp)
    {
        return dp.first;
    }

    /**
     * @brief      Convert a dBm_t value to a dBm Raw (centi-dBm) value.
     *
     * @param[in]  val   The value
     *
     * @return     centi-dBm value
     */
    [[nodiscard]] constexpr auto dbmToDbmRaw(const dbm_t val)
    {
        return static_cast<dbm_raw_t>(val * 100.0);
    }

    /**
     * @brief      Converts a dBm value to a Memory Mapped Register value
     *
     * @param[in]  val   The value
     *
     * @return     Memory Mapped version of a dBm value
     */
    [[nodiscard]] constexpr auto dbmToRegVal(const dbm_t val)
    {
        return static_cast<mmData_t>(dbmToDbmRaw(val));
    }

    /**
     * @brief      Converts a Raw dBm (centi-dBm) value to a normal dBm Floating Point value
     *
     * @param[in]  val   The value
     *
     * @return     dBm_t Floating Point value
     */
    [[nodiscard]] constexpr auto dbmRawToDbm(const dbm_raw_t val)
    {
        return static_cast<dbm_t>(val) / 100.0;
    }

    /**
     * @brief      Memory Mapped Register Value to dBm Float Point value
     *
     * @param[in]  val   The value
     *
     * @return     dBm Floating Point value
     */
    [[nodiscard]] constexpr auto regValToDbm(const mmData_t val)
    {
        return dbmRawToDbm(static_cast<dbm_raw_t>(val));
    }

    /**
     * @brief      Extract the Raw dBm (centi-dBm) value from a Calibration Data Point
     *
     * @param[in]  dp    The Calibration Data Point
     *
     * @return     Raw dBm (centi-dBm) value.
     */
    [[nodiscard]] constexpr auto calDataPointToDbm(const calDataPoint_t& dp)
    {
        return dbmToDbmRaw(dp.first);
    }

    /**
     * @brief      Extract the Raw ADC value from a Calibration Data Point
     *
     * @param[in]  dp    The Calibration Data Point
     *
     * @return     The RAW ADC Count
     */
    [[nodiscard]] constexpr auto calDataPointToRawAdc(const calDataPoint_t& dp)
    {
        return dp.second;
    }

    /**
     * @brief      Calculates the vswr.
     *
     * @param[in]  fwd   Forward
     * @param[in]  rev   The reverse
     *
     * @return     The vswr.
     */
    [[nodiscard]] inline auto calcVswr(const dbm_t fwd, const dbm_t rev)
    {
        vswr_t res{0.0};

        if(fwd > rev)
        {
            res = std::pow(10.0, ((rev - fwd) / 20.0));
            res = (1.0 + res) / (1.0 - res);

            if(!std::isfinite(res))
            {
                res = 0.0;
            }
        }

        return res;
    }

    /**
     * @brief      Calculates the Peak to Average Power Ratio.
     *
     * @param[in]  peak  The peak
     * @param[in]  avg   The average
     *
     * @return     The papr.
     */
    [[nodiscard]] constexpr auto calcPapr(const dbm_t peak, const dbm_t avg) -> papr_t
    {
        return peak - avg;
    }

    /**
     * @brief      Calculates the compression.
     *
     * @param[in]  inPapr   In papr
     * @param[in]  outPapr  The out papr
     *
     * @return     The compression.
     */
    [[nodiscard]] constexpr auto calcCompr(const papr_t inPapr, const papr_t outPapr) ->compr_t
    {
        return (inPapr - outPapr);
    }

    /**
     * @brief      Converts Watts to dBm
     *
     * @param[in]  val   Input value in Watts
     *
     * @return     The converted dBm value
     */
    [[nodiscard]] inline auto wattsTodBm(const watts_t val)
    {
        // dBm = 10*log10(1000*watts)
        dbm_t res{};
        res = 10.0 * std::log10(1000.0 * static_cast<dbm_t>(val));
        return res;
    }

    /**
     * @brief      Converts dBm to Watts
     *
     * @param[in]  val   Input value in dBm
     *
     * @return     The converted Watts value.
     */
    [[nodiscard]] inline auto dBmToWatts(const dbm_t val)
    {
        // watts = (10 ^ (dBm / 10)) / 1000
        watts_t res;
        res = std::pow(10.0, (val / 10.0)) / 1000.0;
        return res;
    }

    /**
     * @brief      Returns a string representation of a port.
     *
     * @param[in]  p     The port
     *
     * @return     String representation of the port.
     */
    [[nodiscard]] inline auto portToStr(const port_t p) -> std::string
    {
        switch(p)
        {
        case port_t::IN:
            return "INPUT";
        case port_t::FWD:
            return "FORWARD";
        case port_t::REF:
            return "REVERSE";
        case port_t::UNBAL:
            return "UNBALANCED";
        default:
            break;
        }
        return "";
    }

    /**
     * @brief      Returns a port enumeration of a string representation
     *
     * @param[in]  p     The port string
     *
     * @return     The port that the string represents
     */
    [[nodiscard]] inline auto strToPort(const std::string& p)
    {
        std::string upStr;
        std::transform(std::cbegin(p), std::cend(p), std::back_inserter(upStr),
            [](const auto& ch){ return std::toupper(ch); });
        if(upStr.compare("UNBALANCED") == 0)
        {
            return port_t::UNBAL;
        }
        else if(upStr.compare("FORWARD") == 0)
        {
            return port_t::FWD;
        }
        else if(upStr.compare("REVERSE") == 0)
        {
            return port_t::REF;
        }
        return port_t::IN;
    }

    /**
     * @brief      Returns a string representation of a detector.
     *
     * @param[in]  d     The detector.
     *
     * @return     String representation of the detector.
     */
    [[nodiscard]] inline auto detectorToStr(const detector_t d) -> std::string
    {
        switch(d)
        {
        case detector_t::ENV:
            return "ENVELOPE";
        case detector_t::PEAK:
            return "PEAK";
        case detector_t::RMS:
            return "RMS";
        case detector_t::PEAK_FAST:
            return "PEAKFAST";
        case detector_t::PEAK_SLOW:
            return "PEAKSLOW";
        default:
            break;
        }
        return "";
    }

    /**
     * @brief      Returns a detector enumeration of a string representation
     *
     * @param[in]  p     The port string.
     *
     * @return     The port.
     */
    [[nodiscard]] inline auto strToDetector(const std::string_view p)
    {
        std::string upStr;
        std::transform(std::cbegin(p), std::cend(p), std::back_inserter(upStr),
            [](const auto& ch){ return std::toupper(ch); });
        if(upStr.compare("RMS") == 0)
        {
            return detector_t::RMS;
        }
        else if(upStr.compare("ENVELOPE") == 0)
        {
            return detector_t::ENV;
        }
        else if(upStr.compare("PEAKFAST") == 0)
        {
            return detector_t::PEAK_FAST;
        }
        else if(upStr.compare("PEAKSLOW") == 0)
        {
            return detector_t::PEAK_SLOW;
        }
        return detector_t::PEAK;
    }

    /**
     * @brief      Returns the Memory Mapped value for a given detector
     *
     * @param[in]  d     The detector.
     *
     * @return     Memory Mapped value of the detector.
     */
    [[nodiscard]] inline auto detectorToRegVal(const detector_t d)
    {
        static const std::unordered_map<detector_t, mmData_t> conversion = {
            {detector_t::RMS, 0}, {detector_t::ENV, 1}, {detector_t::PEAK, 2},
            {detector_t::PEAK_FAST, 2}, {detector_t::PEAK_SLOW, 2}
        };

        std::unordered_map<detector_t, mmData_t>::const_iterator it = conversion.find(d);

        if(it == conversion.end())
        {
            return mmData_t{1};
        }

        return mmData_t{it->second};
    }

    /**
     * @brief      Converts a Memory Mapped Register value to a Detector enumeration
     *
     * @param[in]  d     The Register value.
     *
     * @return     Detector enumeration
     */
    [[nodiscard]] inline auto regValToDetector(const mmData_t d)
    {
        static const std::unordered_map<mmData_t, detector_t> conversion = {
            {0, detector_t::RMS}, {1, detector_t::ENV}, {2, detector_t::PEAK}
        };

        std::unordered_map<mmData_t, detector_t>::const_iterator it = conversion.find(d);

        if(it == conversion.end())
        {
            return detector_t::ENV;
        }

        return it->second;
    }

    /**
     * @brief      Converts a string to a Channel
     *
     * @param[in]  chan  The channel
     *
     * @return     The Channel the string represented
     */
    [[nodiscard]] inline auto strToChan(const std::string& chan) -> chan_t
    {
        std::size_t splitPos = chan.find_first_of('_');
        std::string port = chan.substr(0, splitPos);
        std::string det = chan.substr(splitPos);

        return {strToPort(port), strToDetector(det)};
    }

    /**
     * @brief      Converts a Channel to a string representation
     *
     * @param[in]  ch    The channel.
     *
     * @return     The string representation of the given Channel.
     */
    [[nodiscard]] inline auto chanToChanStr(const chan_t& ch) -> std::string
    {
        return portToStr(chanToPort(ch)) + std::string("_") + detectorToStr(chanToDetector(ch));
    }

    /**
     * @brief      Returns the Register Value of a given Gain Mode
     *
     * @param[in]  gm    The Gain Mode.
     *
     * @return     The Register Value that represents the Gain Mode.
     */
    [[nodiscard]] inline auto gainModeToRegVal(const gain_mode_t& gm)
    {
        static const std::unordered_map<gain_mode_t, mmData_t> conversion = {
            {gain_mode_t::ALC, 0}, {gain_mode_t::AGC, 1}, {gain_mode_t::MGC, 2}
        };

        return conversion.at(gm);
    }

    /**
     * @brief      Returns the Gain Mode a given Register Value represents.
     *
     * @param[in]  data  The data
     *
     * @return     Gain Mode.
     */
    [[nodiscard]] inline auto regValToGainMode(const mmData_t& data)
    {
        static const std::unordered_map<mmData_t, gain_mode_t> conversion = {
            {0, gain_mode_t::ALC}, {1, gain_mode_t::AGC}, {2, gain_mode_t::MGC}
        };

        std::unordered_map<mmData_t, gain_mode_t>::const_iterator it = conversion.find(data);

        if(it == conversion.end())
        {
            return gain_mode_t::MGC;
        }

        return it->second;
    }

    /**
     * @brief      Returns a String representation for a given Gain Mode.
     *
     * @param[in]  gm    The Gain Mode.
     *
     * @return     String representation of the Gain Mode.
     */
    [[nodiscard]] inline auto gainModeToStr(const gain_mode_t& gm)
    {
        static const std::unordered_map<gain_mode_t, std::string> conversion = {
            {gain_mode_t::ALC, "ALC"}, {gain_mode_t::AGC, "AGC"}, {gain_mode_t::MGC, "MGC"}
        };

        return conversion.at(gm);
    }

    /**
     * @brief      Returns the Command String value of a given Gain Mode.
     *
     * @param[in]  gm    The Gain Mode.
     *
     * @return     The Command String for the given Gain Mode.
     */
    [[nodiscard]] inline auto gainModeToCmdStr(const gain_mode_t& gm)
    {
        static const std::unordered_map<gain_mode_t, std::string> conversion = {
            {gain_mode_t::ALC, "A"}, {gain_mode_t::AGC, "G"}, {gain_mode_t::MGC, "M"}
        };

        return conversion.at(gm);
    }

    /**
     * @brief      Returns the Gain Mode for a given String representation.
     *
     * @param[in]  str   The string
     *
     * @return     Gain Mode.
     */
    [[nodiscard]] inline auto strToGainMode(const std::string& str)
    {
        static const std::unordered_map<std::string, gain_mode_t> conversion = {
            {"ALC", gain_mode_t::ALC}, {"AGC", gain_mode_t::AGC}, {"MGC", gain_mode_t::MGC}
        };

        std::unordered_map<std::string, gain_mode_t>::const_iterator it = conversion.find(str);

        if(it == conversion.end())
        {
            return gain_mode_t::MGC;
        }

        return it->second;
    }

    /**
     * @brief      Converts a String type to a Numeric type
     *
     * @param[in]  str    The string
     * @param[in]  radix  The radix
     *
     * @tparam     T      The type of the converted number.
     *
     * @return     The number the string represents.
     */
    template<typename T>
    [[nodiscard]] inline std::optional<T> strToNum(const std::string& str, [[maybe_unused]] const int radix = 10)
    {
        std::optional<T> result{};

        try
        {
            if constexpr(std::numeric_limits<T>::is_integer || std::is_same_v<T, bool>)
            {
                if constexpr(std::numeric_limits<T>::is_signed)
                {
                    result = static_cast<T>(std::stoll(str, 0, radix));
                }
                else
                {
                    result = static_cast<T>(std::stoull(str, 0, radix));
                }
            }
            else if constexpr(std::is_floating_point_v<T>)
            {
                result = static_cast<T>(std::stold(str, 0));
            }
        }
        catch(...) {}

        return result;
    }

    /**
     * @brief      Tokenizes a string based on the given Token into a vector of either strings or numeric types
     *
     * @param[in]  str    The string
     * @param[in]  token  The token
     * @param[in]  radix  The radix
     *
     * @tparam     T      The type the values will be converted to.
     *
     * @return     The vector of the T type values
     */
    template<typename T>
    [[nodiscard]] inline auto dataTokenizer(const std::string& str, const std::string& token = ",",
        [[maybe_unused]] const int radix = 16)
    {
        std::string tokenStr;
        const std::string specialChars{R"([]()\.*+^?|{}$)"};

        std::for_each(std::begin(token), std::end(token), [&](const auto ch){
            if(specialChars.find(ch) != std::string::npos)
            {
                tokenStr += R"(\)";
            }

            tokenStr += ch;
        });

        const std::regex token_re{tokenStr};
        std::vector<T> data;

        try
        {
            if constexpr(std::is_same_v<std::string, T>)
            {
                std::copy(std::regex_token_iterator(std::begin(str), std::end(str), token_re, -1),
                    std::sregex_token_iterator(), std::back_inserter(data));
            }
            else
            {
                std::transform(std::sregex_token_iterator(std::begin(str), std::end(str), token_re, -1),
                    std::sregex_token_iterator(), std::back_inserter(data),
                    [radix](auto& in){ return strToNum<T>(in, radix).value_or(T{}); });
            }
        }
        catch (...)
        {
            data.clear();
        }
        return data;
    }

    /**
     * @brief      Converts a number to a Hex String
     *
     * @param[in]  i     The integral number.
     *
     * @tparam     T     The type of the integral number.
     *
     * @return     The Hex String that represents the integral number.
     */
    template<typename T>
    [[nodiscard]] inline auto toHexString(T i)
    {
        static_assert(std::is_integral_v<T>,
            "Template argument 'T' must be a fundamental integer type (e.g. int, short, etc..).");

        std::stringstream stream;
        stream << std::setfill ('0') << std::setw(sizeof(T)*2) << std::hex;

        if constexpr(std::is_same_v<std::uint8_t, T>)
        {
            stream << static_cast<std::int32_t>(i);
        }
        else if constexpr(std::is_same_v<std::int8_t, T>)
        {
            stream << static_cast<std::int32_t>(static_cast<std::uint8_t>(i));
        }
        else
        {
            stream << i;
        }

        return stream.str();
    }

    template<typename T>
    [[nodiscard]] inline auto floatToStr(const T val, const int precision = 0)
    {
        std::string result{};

        if constexpr(std::is_floating_point_v<T>)
        {
            auto intPrec{(std::numeric_limits<T>::digits10 + 1)};
            if(precision != 0)
            {
                intPrec = precision;
            }

            std::stringstream stream;
            stream << std::setprecision(intPrec) << val;
            result = stream.str();
        }
        else
        {
            static_assert("Function must be called with a floating point type!!!");
        }

        return result;
    }

    /**
     * @brief      Converts a vector or Serial Data values to a comma tokenized string of Hex Strings
     *
     * @param[in]  vec   The vector of Serial Data
     *
     * @return     The string of comma tokenized Hex strings.
     */
    template<typename T>
    [[nodiscard]] inline auto vecToDataStr(const std::vector<T>& vec)
    {
        std::string resp{""};

        if(0 == vec.size())
        {
            return resp;
        }

        for(auto& d: vec)
        {
            resp += toHexString(d);
            resp += std::string(",");
        }
        resp.erase(std::prev(resp.end()));
        return resp;
    }

    /**
     * @brief      Appends the contents of a string to a given Calibration Data Vector.
     *
     * @param[out] result  The result
     * @param[in]  str     The string
     */
    inline void strToCalVec(calVec_t& result, const std::string& str)
    {
        std::regex re{","};
        result.clear();

        if(str.empty())
        {
            return;
        }

        std::transform(
            std::sregex_token_iterator(std::begin(str), std::end(str), re, -1),
            std::sregex_token_iterator(),
            std::back_inserter(result),
            [](const std::string& in){
                std::size_t slashPos = in.find_first_of("/");
                return std::make_pair(strToNum<float>(in.substr(0, slashPos), 0).value_or(0.0),
                    strToNum<adc_raw_t>(in.substr(slashPos+1), 16).value_or(0));
            }
        );

        std::sort(std::begin(result), std::end(result),
            [](const calDataPoint_t& lhs, const calDataPoint_t& rhs){
                return lhs.first < rhs.first;
            }
        );
    }

    /**
     * @brief      Convert a given Chrono Duration to Register Map value for Exponential Decay time constant
     *
     * @param[in]  dur     The duration
     *
     * @tparam     Rep     Chrono Data Representation
     * @tparam     Period  Chrono Time Period
     *
     * @return     Register Value representation of the Exponential Decay time constant
     */
    template<typename Rep, typename Period>
    [[nodiscard]] inline auto decayTimeToFpgaRegMap(const std::chrono::duration<Rep, Period>& dur)
    {
        auto recipExp{static_cast<double>(std::chrono::duration_cast<fpga_clock_rate_t>(dur).count())};
        return (static_cast<mmData_t>(std::exp(-1.0 / recipExp) * std::numeric_limits<mmData_t>::max()) + 1);
    }

    /**
     * @brief      Convert a given Chrono Duration to Register Map value
     *
     * @param[in]  dur     The duration
     *
     * @tparam     Rep     Chrono Data Representation
     * @tparam     Period  Chrono Time Period
     *
     * @return     Register Value representation
     */
    template<typename Rep, typename Period>
    [[nodiscard]] inline auto timeToFpgaRegMap(const std::chrono::duration<Rep, Period>& dur)
    {
        auto fpgaClk{std::chrono::duration_cast<fpga_clock_rate_t>(dur)};
        return static_cast<mmData_t>(fpgaClk.count());
    }

    /**
     * @brief      Converts a Regiseter Value to a given Chrono Time Duratation
     *
     * @param[in]  data       The Register Value
     *
     * @tparam     TIME_TYPE  The Chrono Time Duration (Defaulted to the internal FPGA Clock Rate)
     *
     * @return     The Chrono Time Duration
     */
    template <typename TIME_TYPE = fpga_clock_rate_t>
    [[nodiscard]] inline auto regMapToTime(const mmData_t data)
    {
        fpga_clock_rate_t tmpTime{data};

        if constexpr (std::is_same_v<TIME_TYPE, fpga_clock_rate_t>)
        {
            return tmpTime;
        }

        return std::chrono::duration_cast<TIME_TYPE>(tmpTime);
    }

    /**
     * @brief      Converts a number to a CSV string of which bits are set
     *
     * @param[in]  val   The value
     *
     * @tparam     T     The type of the Value
     *
     * @return     CSV String
     */
    template<typename T>
    [[nodiscard]] inline std::string numToBitSetCsv(const T& val)
    {
        static_assert(std::is_integral_v<T>,
            "Template argument 'T' must be a fundamental integer type (e.g. int, short, etc..).");

        std::string result{""};
        T running{val};

        for(std::size_t idx{1}; running != 0; ++idx, running /= 2)
        {
            if((running & 1) == 1)
            {
                result += std::to_string(idx);

                if((running & ~static_cast<T>(1)) != 0)
                {
                    result += ",";
                }
            }
        }

        return result;
    }

    /**
     * @brief      Calculates the number of bytes needed for a given integral value
     *
     * @param[in]  data  The data
     *
     * @tparam     T     The type of the Data
     *
     * @return     Number of Bytes needed to represent the given integral value.
     */
    template<typename T>
    static std::size_t bytesNeeded(const T& data)
    {
        using working_t = typename std::make_unsigned<T>::type;
        static_assert(std::is_integral_v<T>);

        std::size_t byteCnt{0};
        working_t workingData{static_cast<working_t>(data)};

        if constexpr(std::numeric_limits<T>::is_signed)
        {
            if(data < 0)
            {
                workingData = ~data;

                if((workingData & (T{1} << std::numeric_limits<T>::digits)) == 0)
                {
                    workingData <<= 1;
                }
            }
        }

        do
        {
            ++byteCnt;
        } while(workingData >>= 8);

        return byteCnt;
    }
}

#endif
