#include "siggen.h"

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  target  The target
 */
siggen::siggen(const std::string& target):
    scpiInstrument()
{
    open(target);
}

/**
 * @brief      Turns the RF On or Off
 *
 * @param[in]  on    True enables the RF Output, False disables it.
 *
 * @return     True if the command was successful, False otherwise.
 */
bool siggen::outputEnable(const bool on)
{
    if(!isConnOpen())
    {
        return false;
    }

    return sendOnOff("OUTP:STAT", on);
}

/**
 * @brief      Sets the frequency.
 *
 * @param[in]  freq   The frequency
 * @param[in]  units  The units
 *
 * @return     True if the command was successful, False otherwise.
 */
bool siggen::setFreq(const std::uint32_t freq, const std::string& units)
{
    if(!isConnOpen())
    {
        return false;
    }

    return send("FREQ " + std::to_string(freq) + " " + units);
}

/**
 * @brief      Sets the amp.
 *
 * @param[in]  ampl  The amplitude in dBm
 *
 * @return     True if the command was successful, False otherwise.
 */
bool siggen::setAmp(const double ampl)
{
    if(!isConnOpen())
    {
        return false;
    }

    return send("POW " + std::to_string(ampl));
}
