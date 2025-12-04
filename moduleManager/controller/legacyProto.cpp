#include <controller/legacyProto.h>

#include <logger/logger.h>
#include <optional>
#include <string>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      upIf  The Status Update Interface
 * @param      ser   The Serial Interface
 * @param[in]  key   The Module Key Name
 * @param[in]  addr  The RS485 Bus Address
 * @param[in]  time  The Status Key Timeout
 */
legacyProto::legacyProto(statusUpdateInterface& upIf, serialInterfaceConnection& ser,
        const std::string& key, const pmod_addr_t addr, const timeout_t& time):
    protocolInterface{upIf, ser, key, addr, time}
{
    buildDataset();
}

/**
 * @brief      Reads data from the PMOD device, and reports the status
 */
void legacyProto::poll()
{
    using std::string_literals::operator""s;

    std::optional<ser_msg_t> data;
    bool hasMessage{true};

    std::for_each(dataset.begin(), dataset.end(), [](auto& d){ d.valid = false; });
    dataset.at(updateIfPresentIdx).valid = true;

    // Request Status Data
    try
    {
        if(data = serIf.read(buildPmodMsg(cmds::status), serMsgStartTimeout, serMsgCharTimeout, false);
            !data.has_value())
        {
            logger::warn(__FILE__, __FUNCTION__, "PMOD Polling Failed");
            hasMessage = false;
        }
    }
    catch(const std::exception& e)
    {
        logger::critical(__FILE__, __FUNCTION__, "Unhandled SHI Exception - "s + std::string(e.what()));
        hasMessage = false;
    }

    // Decode Status Data Response
    if(hasMessage)
    {
        ser_msg_t& serData{data.value()};
        std::string rawDataStr{helpers::types::vecToDataStr(serData)};

        logger::verbose(__FILE__, __FUNCTION__, "Raw PMOD Message: "s + rawDataStr);

        dataset.at(updateIfRawIdx).data = std::move(rawDataStr);
        dataset.at(updateIfRawIdx).valid = true;

        if(validatePacket(serData))
        {
            logger::verbose(__FILE__, __FUNCTION__, "Decoding PMOD Message for "s + deviceKey);
            decodeStatus(serData);

            pollErrCnt = 0;
            dataset.at(updateIfPresentIdx).data = true;
        }
    }

    if(++pollErrCnt > pollingRetryCnt)
    {
        pollErrCnt = pollingRetryCnt;
        dataset.at(updateIfPresentIdx).data = false;
    }

    updateIf.updateData(dataset);
}

/**
 * @brief      Resets the PMOD
 */
void legacyProto::reset()
{
    serIf.write(buildPmodMsg(cmds::reset), false);
}

/**
 * @brief      Sends Fault Clear to the PMOD
 */
void legacyProto::faultClear()
{
    serIf.write(buildPmodMsg(cmds::clearAlarms), false);
}

/**
 * @brief      Builds a dataset of Status Keys
 */
void legacyProto::buildDataset()
{
    using std::string_literals::operator""s;

    dataset.clear();
    dataset.emplace_back(deviceKey + "_RAW"s,           deviceKey + " Raw Response"s,        "",  timeout);
    dataset.emplace_back(deviceKey + "_CURRENT_CNT"s,   deviceKey + " Number of Currents"s,  "",  timeout);
    dataset.emplace_back(deviceKey + "_CURRENT_1"s,     deviceKey + " Current 1"s,           "A", timeout);
    dataset.emplace_back(deviceKey + "_CURRENT_2"s,     deviceKey + " Current 2"s,           "A", timeout);
    dataset.emplace_back(deviceKey + "_CURRENT_3"s,     deviceKey + " Current 3"s,           "A", timeout);
    dataset.emplace_back(deviceKey + "_CURRENT_4"s,     deviceKey + " Current 4"s,           "A", timeout);
    dataset.emplace_back(deviceKey + "_CURRENT_5"s,     deviceKey + " Current 5"s,           "A", timeout);
    dataset.emplace_back(deviceKey + "_CURRENT_6"s,     deviceKey + " Current 6"s,           "A", timeout);
    dataset.emplace_back(deviceKey + "_CURRENT_7"s,     deviceKey + " Current 7"s,           "A", timeout);
    dataset.emplace_back(deviceKey + "_CURRENT_8"s,     deviceKey + " Current 8"s,           "A", timeout);
    dataset.emplace_back(deviceKey + "_TOTAL_CURRENT"s, deviceKey + " Total Current"s,       "A", timeout);
    dataset.emplace_back(deviceKey + "_INPUT_VOLTAGE"s, deviceKey + " Input Voltage"s,       "V", timeout);
    dataset.emplace_back(deviceKey + "_TEMP"s,          deviceKey + " Temp"s,                "C", timeout);
    dataset.emplace_back(deviceKey + "_ALARM_ILIM"s,    deviceKey + " Current Limit Alarm"s, "",  timeout);
    dataset.emplace_back(deviceKey + "_ALARM_SEQ"s,     deviceKey + " PS Sequencer Alarm"s,  "",  timeout);
    dataset.emplace_back(deviceKey + "_ALARM_GATE"s,    deviceKey + " Gate Alarm"s,          "",  timeout);
    dataset.emplace_back(deviceKey + "_ALARM_TEMP"s,    deviceKey + " Temperature Alarm"s,   "",  timeout);
    dataset.emplace_back(deviceKey + "_ALARM_ANALOG"s,  deviceKey + " Analog Alarm"s,        "",  timeout);
    dataset.emplace_back(deviceKey + "_ENABLED"s,       deviceKey + " Enabled"s,             "",  timeout);
    dataset.emplace_back(deviceKey + "_ALARM_HIGH"s,    deviceKey + " ADC High Alarms"s,     "",  timeout);
    dataset.emplace_back(deviceKey + "_WARN_HIGH"s,     deviceKey + " ADC High Warnings"s,   "",  timeout);
    dataset.emplace_back(deviceKey + "_ALARM_LOW"s,     deviceKey + " ADC Low Alarms"s,      "",  timeout);
    dataset.emplace_back(deviceKey + "_WARN_LOW"s,      deviceKey + " ADC Low Warnings"s,    "",  timeout);
    dataset.emplace_back(deviceKey + "_PRESENT"s,       deviceKey + " Is Present"s,          "",  timeout);
}

/**
 * @brief      Builds a pmod message.
 *
 * @param[in]  command     The command
 * @param[in]  insertData  Data Field information
 *
 * @return     The pmod message.
 */
[[nodiscard]]
legacyProto::ser_msg_t legacyProto::buildPmodMsg(const cmds command, const ser_msg_t& insertData)
{
    ser_msg_t data;

    // Master Address
    data.emplace_back(0);

    // Slave Address
    data.emplace_back(curAddr);

    // Dummy Length
    data.emplace_back(0);

    // Status
    data.emplace_back(static_cast<pmod_data_t>(status::completed_ok));

    // Command
    data.emplace_back(static_cast<pmod_data_t>(command));

    // Appending data to be sent
    std::copy(insertData.begin(), insertData.end(), std::back_inserter(data));

    // Inserting Real Size
    data.at(2) = static_cast<pmod_data_t>(data.size() - 2);

    // Checksum
    data.emplace_back(std::accumulate(data.begin(), data.end(), pmod_data_t{0}, std::bit_xor<pmod_data_t>()));

    return data;
}

/**
 * @brief      Validates a Data Packet
 *
 * @param      data  The packet
 *
 * @return     True if the packet is valid, False otherwise.
 */
[[nodiscard]]
bool legacyProto::validatePacket(ser_msg_t& data)
{
    using std::string_literals::operator""s;

    logger::verbose(__FILE__, __FUNCTION__, "Validating Packet...");

    // Minimum Packet Size
    if(data.size() < 6)
    {
        logger::verbose(__FILE__, __FUNCTION__, "Packet Data Length too small: "s + std::to_string(data.size()));
        return false;
    }

    if((data.size() - 3) > data.at(2))
    {
        data.resize(data.at(2) + 3u);
    }

    if(!(data.at(2) == (data.size() - 3)))
    {
        logger::verbose(__FILE__, __FUNCTION__, "Packet Data Length Doesn't Match Length Field: "s +
            std::to_string(data.at(2) + 3) + ":"s + std::to_string(data.size()));
        return false;
    }

    if(std::accumulate(data.begin(), data.end(), pmod_data_t{0}, std::bit_xor<pmod_data_t>()) != 0)
    {
        logger::verbose(__FILE__, __FUNCTION__, "Packet Checksum Failed");
        return false;
    }

    if(data.at(3) != static_cast<pmod_data_t>(status::completed_ok))
    {
        logger::verbose(__FILE__, __FUNCTION__, "Packet Status Bad: "s + std::to_string(data.at(3)));
        return false;
    }

    if(((data.size() % 2) == 1) && ((data.size() < sizeOfStatusMsgZero) || (data.size() > sizeOfStatusMsgEight)))
    {
        logger::warn(__FILE__, __FUNCTION__, "Packet Status Wrong Length");
        return false;
    }

    return true;
}

/**
 * @brief      Decodes a status message to be sent to the Status Update Processor
 *
 * @param[in]  data  The data packet
 */
void legacyProto::decodeStatus(const ser_msg_t& data)
{
    using namespace helpers::serConn;

    auto setData = [&](const data_set_t::difference_type idx, const auto& d){
        data_set_t::iterator dataIt = std::next(dataset.begin(), idx);
        dataIt->data = d;
        dataIt->valid = true;
    };
    ser_msg_t::const_iterator it{std::next(data.cbegin(), 5)};
    const std::size_t currChanRadingsCnt{((data.size() - sizeOfStatusMsgZero) / 2)};

    setData(updateIfCurrCntIdx, static_cast<unsigned int>(currChanRadingsCnt));
    for(std::size_t idx{0}; idx < currChanRadingsCnt; ++idx)
    {
        setData(static_cast<data_set_t::difference_type>(idx + updateIfCurr1Idx),
            (static_cast<double>(getVal<std::uint16_t>(it)) / 100.0));
    }
    setData(updateIfTotalCurrIdx, (static_cast<double>(getVal<std::uint16_t>(it)) / 100.0));
    setData(updateIfInputVoltIdx, (static_cast<double>(getVal<std::uint16_t>(it)) / 100.0));
    setData(updateIfTempIdx, getVal<std::int16_t>(it));
    setData(updateIfAlarmILimIdx, ((*it & 0x01) == 0x01));
    setData(updateIfAlarmSeqIdx, ((*it & 0x20) == 0x20));
    setData(updateIfAlarmGateIdx, ((*it & 0x04) == 0x04));
    setData(updateIfAlarmTempIdx, ((*it & 0x08) == 0x08));
    setData(updateIfAlarmAnalogIdx, ((*it & 0x10) == 0x10));
    setData(updateIfEnabledIdx, ((*it & 0x02) == 0x02));
    it++;
    setData(updateIfAlarmHighIdx, helpers::types::numToBitSetCsv(getVal<std::uint16_t>(it)));
    setData(updateIfWarnHighIdx, helpers::types::numToBitSetCsv(getVal<std::uint16_t>(it)));
    setData(updateIfAlarmLowIdx, helpers::types::numToBitSetCsv(getVal<std::uint16_t>(it)));
    setData(updateIfWarnLowIdx, helpers::types::numToBitSetCsv(getVal<std::uint16_t>(it)));
    setData(updateIfPresentIdx, true);
}
