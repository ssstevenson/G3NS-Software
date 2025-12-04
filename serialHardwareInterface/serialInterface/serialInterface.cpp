#include "serialInterface.h"

#include <algorithm>
#include <chrono>
#include <logger/logger.h>
#include <thread>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  baseAddr  The base address
 */
serialInterface::serialInterface(const hw_reg_t& baseAddr):
    hwIf{baseAddr, 0x10000},
    readTimeout{},
    readTimeoutStart{},
    readTimeoutWorking{}
{}

/**
 * @brief      Read data from a UART
 *
 * @param[in]  startTimeout  The start timeout
 * @param[in]  charTimeout   The character timeout
 *
 * @return     Vector of the serial data if sucessful, std::nullopt otherwise.
 */
std::optional<std::vector<serialInterface::ser_data_t>> serialInterface::read(const timeout_t& startTimeout,
    const timeout_t& charTimeout)
{
    std::vector<ser_data_t> data;
    std::optional<ser_data_t> datum;

    // Wait until the trasmitter is done before moving into the RX wait block
    while(isTxRunning())
    {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    readTimeoutStart = startTimeout;
    readTimeout = charTimeout;
    updateReadTimeout();

    try
    {
        while(!checkReadStartTimeout())
        {
            if(datum = readByte(); datum.has_value())
            {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if(!datum.has_value())
        {
            logger::verbose(__FILE__, __FUNCTION__, "Message Rx Timeout!!!");
            return std::nullopt;
        }
    }
    catch(const std::exception& e)
    {
        logger::debug(__FILE__, __FUNCTION__, std::string("Read Error - ") + std::string(e.what()));
        return std::nullopt;
    }

    // This try catch block is here in case a bad allocation occurs in the emplace_back method
    try
    {
        data.emplace_back(datum.value());
        updateReadTimeout();

        while(!checkReadTimeout())
        {
            if(datum = readByte(); datum.has_value())
            {
                updateReadTimeout();
                data.emplace_back(datum.value());
            }

            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
    catch(const std::exception& e)
    {
        logger::debug(__FILE__, __FUNCTION__, std::string("Read Error - ") + std::string(e.what()));
        return std::nullopt;
    }

    return data;
}

/**
 * @brief      Read data from a UART
 *
 * @param[in]  data          The data to write before reading
 * @param[in]  startTimeout  The start timeout
 * @param[in]  charTimeout   The character timeout
 *
 * @return     Vector of the serial data if sucessful, std::nullopt otherwise.
 */
std::optional<std::vector<serialInterface::ser_data_t>> serialInterface::read(const std::vector<ser_data_t>& data,
    const timeout_t& startTimeout, const timeout_t& charTimeout)
{
    if(write(data))
    {
        return read(startTimeout, charTimeout);
    }

    return std::nullopt;
}

/**
 * @brief      Write to a UART
 *
 * @param[in]  data  The data
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::write(const std::vector<ser_data_t>& data)
{
    clearWriteBuffer();

    for(auto it: data)
    {
        if(!writeByte(it))
        {
            logger::debug(__FILE__, __FUNCTION__, "Write Failure!!!");
            return false;
        }
    }

    clearReadBuffer();
    startTransmission();

    return true;
}

/**
 * @brief      Reads a byte.
 *
 * @return     The byte read if sucessful, std::nullopt otherwise.
 */
std::optional<serialInterface::ser_data_t> serialInterface::readByte() const
{
    if(isRxBufferEmpty())
    {
        return std::nullopt;
    }

    if(auto curData{hwIf.read(static_cast<hw_reg_t>(registerMap::READ))}; curData.has_value())
    {
        auto& data{curData.value()};

        if((data & masks::readDataLiveParityError) != 0)
        {
            logger::verbose(__FILE__, __FUNCTION__, std::string("Raw Full Read (Parity Error) - ") +
                helpers::types::toHexString(data));
            throw parityError();
        }

        if((data & masks::readDataLiveFramingError) != 0)
        {
            logger::verbose(__FILE__, __FUNCTION__, std::string("Raw Full Read (Framing Error) - ") +
                helpers::types::toHexString(data));
            throw framingError();
        }

        return static_cast<ser_data_t>(data & masks::readData);
    }

    logger::debug(__FILE__, __FUNCTION__, "Hardware Interface Issue");

    return std::nullopt;
}

/**
 * @brief      Writes a byte.
 *
 * @param[in]  datum  The data
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::writeByte(const ser_data_t& datum) const noexcept
{
    if(isTxBufferFull())
    {
        logger::debug(__FILE__, __FUNCTION__, "TX Buffer Full!!!");
        return false;
    }

    return hwIf.write(static_cast<hw_reg_t>(registerMap::WRITE), static_cast<hw_reg_t>(datum));
}

/**
 * @brief      Sets the baud rate.
 *
 * @param[in]  baud  The baud
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::setBaud(const std::uint32_t baud) const noexcept
{
    if(auto baseClk{getClockRate()}; baseClk.has_value() && (baud > 0))
    {
        return hwIf.write(static_cast<hw_reg_t>(registerMap::BAUD_SET), ((baseClk.value() / baud) >> 4));
    }

    return false;
}

/**
 * @brief      Gets the baud.
 *
 * @return     The baud if sucessful, std::nullopt otherwise.
 */
std::optional<std::uint32_t> serialInterface::getBaud() const noexcept
{
    if(auto baudSet{hwIf.read(static_cast<hw_reg_t>(registerMap::BAUD_SET))}, baseClk{getClockRate()};
        baudSet.has_value() && baseClk.has_value())
    {
        std::uint32_t baud{(baseClk.value() / baudSet.value()) >> 4};

        if(0 >= baud)
        {
            return std::nullopt;
        }

        return baud;
    }

    return std::nullopt;
}

/**
 * @brief      Sets the bits.
 *
 * @param[in]  bits  The bits
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::setBits(const mmData_t bits) const noexcept
{
    return hwIf.write(static_cast<hw_reg_t>(registerMap::CTRL), bits, masks::setBits);
}

std::optional<serialInterface::mmData_t> serialInterface::getBits() const noexcept
{
    return hwIf.read(static_cast<hw_reg_t>(registerMap::CTRL), masks::setBits);
}

/**
 * @brief      Sets the parity.
 *
 * @param[in]  parity  The parity
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::setParity(const parityStates parity) const noexcept
{
    return hwIf.write(static_cast<hw_reg_t>(registerMap::CTRL), parityStateToReg.at(parity), masks::parity);
}

std::optional<serialInterface::parityStates> serialInterface::getParity() const noexcept
{
    if(auto reg{hwIf.read(static_cast<hw_reg_t>(registerMap::CTRL), masks::parity)}; reg.has_value())
    {
        return regToParityState.at(reg.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the stop.
 *
 * @param[in]  stop  The stop
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::setStop(const stopStates stop) const noexcept
{
    return hwIf.write(static_cast<hw_reg_t>(registerMap::CTRL), stopStatesToReg.at(stop), masks::stop);
}

/**
 * @brief      Gets the stop.
 *
 * @return     The stop if sucessful, std::nullopt otherwise.
 */
std::optional<serialInterface::stopStates> serialInterface::getStop() const noexcept
{
    if(auto reg{hwIf.read(static_cast<hw_reg_t>(registerMap::CTRL), masks::stop)}; reg.has_value())
    {
        return regToStopStates.at(reg.value());
    }

    return std::nullopt;
}

/**
 * @brief      Sets the duplex.
 *
 * @param[in]  dup   The new value
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::setDuplex(const duplex& dup) const noexcept
{
    return hwIf.write(static_cast<hw_reg_t>(registerMap::CTRL), ((duplex::FULL == dup)?(masks::duplex):(0)),
        masks::duplex);
}

/**
 * @brief      Gets the duplex.
 *
 * @return     The duplex if sucessful, std::nullopt otherwise.
 */
std::optional<serialInterface::duplex> serialInterface::getDuplex() const noexcept
{
    if(auto reg{hwIf.read(static_cast<hw_reg_t>(registerMap::CTRL), masks::duplex)}; reg.has_value())
    {
        return ((0 != reg.value())?(duplex::FULL):(duplex::HALF));
    }

    return std::nullopt;
}

/**
 * @brief      Clears the Read Buffer.
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::clearReadBuffer() const noexcept
{
    return hwIf.write(static_cast<hw_reg_t>(registerMap::CLEAR_RX), 0);
}

/**
 * @brief      Clears the Write Buffer.
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::clearWriteBuffer() const noexcept
{
    return hwIf.write(static_cast<hw_reg_t>(registerMap::CLEAR_TX), 0);
}

/**
 * @brief      Starts a transmission.
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::startTransmission() const noexcept
{
    return hwIf.write(static_cast<hw_reg_t>(registerMap::START), 0);
}

/**
 * @brief      Determines if receive buffer empty.
 *
 * @return     True if sucessful, False otherwise.
 */
bool serialInterface::isRxBufferEmpty() const noexcept
{
    if(auto data{hwIf.read(static_cast<hw_reg_t>(registerMap::STATUS), masks::rxEmpty)}; data.has_value())
    {
        return (0 != data.value());
    }

    // If we cannot determine the state of the RX Buffer, stating that it is not empty is the safe state
    return false;
}

/**
 * @brief      Determines if receive buffer full.
 *
 * @return     True if receive buffer full, False otherwise.
 */
bool serialInterface::isRxBufferFull() const noexcept
{
    if(auto data{hwIf.read(static_cast<hw_reg_t>(registerMap::STATUS), masks::rxFull)}; data.has_value())
    {
        return (0 != data.value());
    }

    // If we cannot determine the state of the RX Buffer, stating that it is full is the safe state
    return true;
}

/**
 * @brief      Determines if transmit buffer full.
 *
 * @return     True if transmit buffer full, False otherwise.
 */
bool serialInterface::isTxBufferFull() const noexcept
{
    if(auto data{hwIf.read(static_cast<hw_reg_t>(registerMap::STATUS), masks::txFull)}; data.has_value())
    {
        return (0 != data.value());
    }

    // If we cannot determine the state of the TX Buffer, stating that it is full is the safe state
    return true;
}

/**
 * @brief      Determines if transmit running.
 *
 * @return     True if transmit running, False otherwise.
 */
bool serialInterface::isTxRunning() const noexcept
{
    if(auto data{hwIf.read(static_cast<hw_reg_t>(registerMap::STATUS), masks::txRunning)}; data.has_value())
    {
        return (0 != data.value());
    }

    // If we cannot determine the state of the TX Running, stating that it is running is the safe state
    return true;
}

/**
 * @brief      Converts a string to Parity state
 *
 * @param[in]  str   The string
 *
 * @return     The Parity state
 */
serialInterface::parityStates serialInterface::strToParity(const std::string& str) const noexcept
{
    parityStates rtn(parityStates::NONE);
    std::string strUp;
    try
    {
        std::transform(str.begin(), str.end(), strUp.begin(), ::toupper);

        if(strUp == "EVEN")
        {
            rtn = parityStates::EVEN;
        }
        else if(strUp == "ODD")
        {
            rtn = parityStates::ODD;
        }
    }
    catch(...)
    {}

    return rtn;
}

/**
 * @brief      Converts a String to Stop Bits value
 *
 * @param[in]  str   The string
 *
 * @return     The stop bits value.
 */
serialInterface::stopStates serialInterface::strToStop(const std::string& str) const noexcept
{
    stopStates rtn(stopStates::ONE);

    if(str == "1.5")
    {
        rtn = stopStates::ONE_HALF;
    }
    else if(str == "2")
    {
        rtn = stopStates::TWO;
    }

    return rtn;
}

/**
 * @brief      Gets the clock rate.
 *
 * @return     The clock rate if sucessful, std::nullopt otherwise.
 */
std::optional<std::uint32_t> serialInterface::getClockRate() const noexcept
{
    return hwIf.read(static_cast<hw_reg_t>(registerMap::BASE_FREQ));
}

/**
 * @brief      Checks if there has been a Read Timeout
 *
 * @return     True if there has been a timeout, False otherwise.
 */
bool serialInterface::checkReadTimeout() const
{
    auto currentTime = std::chrono::duration_cast<timeout_t>(
        std::chrono::steady_clock::now().time_since_epoch());
    return ((currentTime.count() - readTimeoutWorking.count()) > readTimeout.count());
}

/**
 * @brief      Checks if there has been a Read Start Timeout
 *
 * @return     True if there has been a timeout, False otherwise.
 */
bool serialInterface::checkReadStartTimeout() const
{
    auto currentTime = std::chrono::duration_cast<timeout_t>(
        std::chrono::steady_clock::now().time_since_epoch());
    return ((currentTime.count() - readTimeoutWorking.count()) > readTimeoutStart.count());
}

/**
 * @brief      Periodically updates the Read timeout
 */
void serialInterface::updateReadTimeout() noexcept
{
    readTimeoutWorking = std::chrono::duration_cast<timeout_t>(
        std::chrono::steady_clock::now().time_since_epoch());
}
