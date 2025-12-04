#include <controller/kissProto.h>

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
kissProto::kissProto(statusUpdateInterface& upIf, serialInterfaceConnection& ser,
        const std::string& key, const pmod_addr_t addr, const timeout_t& time):
    protocolInterface{upIf, ser, key, addr, time},
    updateData{upIf, key, time},
    seqNumCnt{0}
{}

/**
 * @brief      Reads data from the PMOD device, and reports the status
 */
void kissProto::poll()
{
    using std::string_literals::operator""s;

    auto msg{buildMessage(curAddr, msg_type_t::g_01)};

    // Request Status Data
    try
    {
        if(auto data{serIf.read(msg.serialize(), serMsgStartTimeout, serMsgCharTimeout, true)}; data.has_value())
        {
            msg = data.value();
            pollErrCnt = 0;
        }
        else
        {
            msg.clear();
            logger::warn(__FILE__, __FUNCTION__, "PMOD Polling Failed");
        }
    }
    catch(const std::exception& e)
    {
        msg.clear();
        logger::critical(__FILE__, __FUNCTION__, "Unhandled SHI Exception - "s + std::string(e.what()));
    }

    if(++pollErrCnt > pollingRetryCnt)
    {
        pollErrCnt = pollingRetryCnt;
    }

    updateData.update(msg, (pollErrCnt < pollingRetryCnt));
}

/**
 * @brief      Resets the PMOD
 */
void kissProto::reset()
{
    auto msg{buildMessage(curAddr, msg_type_t::command)};
    msg.emplace_back(chunk_id_t::reset);
    serIf.write(msg.serialize(), true);
}

/**
 * @brief      Sends Fault Clear to the PMOD
 */
void kissProto::faultClear()
{
    auto msg{buildMessage(curAddr, msg_type_t::command)};
    msg.emplace_back(chunk_id_t::dummy);
    serIf.write(msg.serialize(), true);
}

/**
 * @brief      Gets the next sequence number.
 *
 * @return     The next sequence number.
 */
[[nodiscard]]
auto kissProto::getNextSeqNum() -> std::size_t
{
    return seqNumCnt++;
}

/**
 * @brief      Builds a message.
 *
 * @param[in]  to    Device Address the message is being sent to.
 * @param[in]  type  The type of message
 *
 * @return     The message.
 */
[[nodiscard]]
auto kissProto::buildMessage(const dev_addr_t& to, const msg_type_t& type) -> kiss::message
{
    const auto fromAddr{dev_addr_t{0}};
    const auto toAddr{to};
    const auto seqNum{getNextSeqNum()};
    const auto msgType{type};
    const auto protocol{protocolVersion::default_ver};
    const auto broadcastResp{bool{false}};
    const auto ackSetter{bool{false}};
    const auto checksumType{checksumTypes::bits_8_xor};

    return kiss::message{fromAddr, toAddr, seqNum, msgType, protocol, broadcastResp, ackSetter, checksumType};
}
