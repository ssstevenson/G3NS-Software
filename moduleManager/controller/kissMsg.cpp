#include <controller/kissMsg.h>

#include <limits>
#include <numeric>
#include <unordered_map>

using namespace empower::kiss;

/**
 * @brief      Constructs a new instance.
 */
message::message():
    header{},
    chunks{},
    isValid{0},
    rawMessage{std::nullopt}
{}

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  m     Serial Message
 */
message::message(const ser_msg_t& m):
    message{}
{
    fromSerMsg(m);
}

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  fromAddr       The from address
 * @param[in]  toAddr         To address
 * @param[in]  seqNum         The sequence number
 * @param[in]  msgType        The message type
 * @param[in]  protocol       The protocol
 * @param[in]  broadcastResp  Forcebroadcast response
 * @param[in]  ack            Acknowledgement
 * @param[in]  csType         The checksum type
 */
message::message(const dev_addr_t fromAddr, const dev_addr_t toAddr, const std::optional<std::size_t>& seqNum,
        const msg_type_t msgType, const protocolVersion protocol, const bool broadcastResp, const bool ack,
        const checksumTypes csType):
    message{}
{
    setFromAddr(fromAddr);
    setToAddr(toAddr);
    setSeqNum(seqNum);
    setMsgType(msgType);
    setProtocolVersion(protocol);
    setBroadcastResp(broadcastResp);
    setAck(ack);
    setChecksumType(csType);
}

/**
 * @brief      Clears the object.
 */
void message::clear()
{
    header = {};
    chunks.clear();
    isValid.reset();
}

/**
 * @brief      Converts this message to a Boolean of being valid or not.
 *
 * @return     Boolean representing if this message is configured into a valid state.
 */
message::operator bool() const
{
    bool result{
        isValid.test(headerInfoTupTo) ||
        isValid.test(headerInfoTupFrom) ||
        isValid.test(headerInfoTupFlags) ||
        isValid.test(headerInfoTupForceBroadcast) ||
        isValid.test(headerInfoTupAckSetter) ||
        isValid.test(headerInfoTupProtocolVersion) ||
        isValid.test(headerInfoTupSeqNumFieldSize) ||
        isValid.test(headerInfoTupPayloadLenFieldSize) ||
        isValid.test(headerInfoTupChecksumType) ||
        isValid.test(headerInfoTupMessageType)
    };

    return result;
}

/**
 * @brief      Assignment operator.
 *
 * @param[in]  m     Serial Message
 *
 * @return     The result of the assignment
 */
message& message::operator=(const ser_msg_t& m)
{
    fromSerMsg(m);
    return *this;
}

/**
 * @brief      Gets the beginning of the Chunks vector
 *
 * @return     Input Iterator pointing to the beginning of the Chunks vector
 */
auto message::begin() -> decltype(chunks.begin())
{
    return chunks.begin();
}

/**
 * @brief      Gets the ending of the Chunks vector
 *
 * @return     Input Iterator pointing to the end of the Chunks vector
 */
auto message::end() -> decltype(chunks.end())
{
    return chunks.end();
}

/**
 * @brief      Gets the constant beginning of the Chunks Vector
 *
 * @return     Constant Input Iterator pointing to the beginning of the Chunks vector
 */
auto message::cbegin() -> decltype(chunks.cbegin())
{
    return chunks.cbegin();
}

/**
 * @brief      Gets the constant ending of the Chunks vector
 *
 * @return     Constant Input Iterator pointing to the end of the Chunks vector
 */
auto message::cend() -> decltype(chunks.cend())
{
    return chunks.cend();
}

/**
 * @brief      Pushes a new chunk onto the back of the Chunks vector
 *
 * @param[in]  ch    The chunk
 */
void message::push_back(const chunk& ch)
{
    chunks.push_back(ch);
}

/**
 * @brief      Fills in data from Serial Message
 *
 * @param[in]  m     The serial message
 */
void message::fromSerMsg(const ser_msg_t& m)
{
    using namespace helpers::serConn;

    rawMessage.emplace();
    std::copy(std::begin(m), std::end(m), std::back_inserter(rawMessage.value()));

    if(m.size() < 5)
    {
        return;
    }

    clear();

    header = getHeaderInfo(m);

    ser_msg_t::const_iterator msgIt{std::next(std::begin(m), std::get<headerInfoTupPayloadOffset>(header))};
    while(std::distance(msgIt, std::cend(m)) >= 3)
    {
        emplace_back(msgIt);
    }

    isValid.set();
}

/**
 * @brief      Converts the Message Data to a serial byte array
 *
 * @return     The serial byte array vector
 */
[[nodiscard]]
auto message::serialize() -> ser_msg_t
{
    using namespace helpers::serConn;

    ser_msg_t msg;
    auto it{std::back_inserter(msg)};

    const auto toAddr{getToAddr()};
    const auto fromAddr{getFromAddr()};
    const auto packedHeader{getPackedHeader()};
    const auto seqNum{getSeqNum()};
    const auto seqNumLen{getSeqNumLen()};
    const auto payloadLen{getPayloadLen()};
    const auto payloadLenSize{getPayloadLenSize()};
    const auto msgType{getMsgType()};

    putVal(it, toAddr);
    putVal(it, fromAddr);
    putVal(it, packedHeader);
    packOpt(it, seqNum, seqNumLen);
    packOpt(it, payloadLen, payloadLenSize);
    putVal(it, msgType);
    std::for_each(begin(), end(), [&it](auto& ch){ ch.serialize(it); });
    calculateChecksum(msg, getChecksumType());

    return msg;
}

/**
 * @brief      Packs an Optional parameter into the serial data
 *
 * @param      it    The iterator
 * @param[in]  val   The value
 * @param[in]  size  The size
 */
void message::packOpt(ser_msg_back_in_it& it, const opt_uint32_t& val, const bitFieldSizes& size)
{
    static const auto putVal_1 = [&it](const opt_uint32_t::value_type& optVal){
        helpers::serConn::putVal<opt_uint32_t::value_type, 1>(it, optVal); };
    static const auto putVal_2 = [&it](const opt_uint32_t::value_type& optVal){
        helpers::serConn::putVal<opt_uint32_t::value_type, 2>(it, optVal); };
    static const auto putVal_3 = [&it](const opt_uint32_t::value_type& optVal){
        helpers::serConn::putVal<opt_uint32_t::value_type, 3>(it, optVal); };

    static const std::unordered_map<bitFieldSizes, std::function<void(const opt_uint32_t::value_type&)>> mapping{
        {bitFieldSizes::bits_8, putVal_1}, {bitFieldSizes::bits_16, putVal_2}, {bitFieldSizes::bits_24, putVal_3}
    };

    if(auto mapIt{mapping.find(size)}; ((mapIt != mapping.end()) && val.has_value()))
    {
        mapIt->second(val.value());
    }
}

/**
 * @brief      Gets the raw message.
 *
 * @return     The raw message.
 */
[[nodiscard]]
auto message::getRawMessage() const -> std::optional<ser_msg_t>
{
    return rawMessage;
}

/**
 * @brief      Gets the from address.
 *
 * @return     The from address.
 */
auto message::getFromAddr() const -> std::tuple_element<headerInfoTupFrom, header_info_t>::type
{
    return std::get<headerInfoTupFrom>(header);
}

/**
 * @brief      Sets the from address.
 *
 * @param[in]  a     The new value
 */
void message::setFromAddr(const dev_addr_t a)
{
    if(isAddrValid(a))
    {
        std::get<headerInfoTupFrom>(header) = a;
        isValid.set(headerInfoTupFrom);
    }
}

/**
 * @brief      Gets to address.
 *
 * @return     To address.
 */
auto message::getToAddr() const -> std::tuple_element<headerInfoTupTo, header_info_t>::type
{
    return std::get<headerInfoTupTo>(header);
}

/**
 * @brief      Sets to address.
 *
 * @param[in]  a     The new value
 */
void message::setToAddr(const dev_addr_t a)
{
    if(isAddrValid(a))
    {
        std::get<headerInfoTupTo>(header) = a;
        isValid.set(headerInfoTupTo);
    }
}

/**
 * @brief      Gets the sequence number.
 *
 * @return     The sequence number.
 */
auto message::getSeqNum() const -> std::tuple_element<headerInfoTupSeqNum, header_info_t>::type
{
    return std::get<headerInfoTupSeqNum>(header);
}

/**
 * @brief      Gets the sequence number length.
 *
 * @return     The sequence number length.
 */
auto message::getSeqNumLen() const -> std::tuple_element<headerInfoTupSeqNumFieldSize, header_info_t>::type
{
    return std::get<headerInfoTupSeqNumFieldSize>(header);
}

/**
 * @brief      Sets the sequence number.
 *
 * @param[in]  seq   The new value
 */
void message::setSeqNum(const opt_uint32_t& seq)
{
    auto& seqNumRef{std::get<headerInfoTupSeqNum>(header)};
    auto& seqFieldSizeRef{std::get<headerInfoTupSeqNumFieldSize>(header)};
    auto& flags{std::get<headerInfoTupFlags>(header)};

    flags &= 0xCFFF;

    if(!seq.has_value())
    {
        seqNumRef = 0;
        seqFieldSizeRef = bitFieldSizes::ommitted;
    }
    else
    {
        seqNumRef = seq.value();

        if(seqNumRef <= std::numeric_limits<std::uint8_t>::max())
        {
            seqFieldSizeRef = bitFieldSizes::bits_8;
            flags |= 0x1000;
        }
        else if(seqNumRef <= std::numeric_limits<std::uint16_t>::max())
        {
            seqFieldSizeRef = bitFieldSizes::bits_16;
            flags |= 0x2000;
        }
        else
        {
            seqFieldSizeRef = bitFieldSizes::bits_24;
            flags |= 0x3000;
        }
    }

    isValid.set(headerInfoTupSeqNum);
    isValid.set(headerInfoTupSeqNumFieldSize);
    isValid.set(headerInfoTupFlags);
}

/**
 * @brief      Gets the message type.
 *
 * @return     The message type.
 */
auto message::getMsgType() const -> std::tuple_element<headerInfoTupMessageType, header_info_t>::type
{
    return std::get<headerInfoTupMessageType>(header);
}

/**
 * @brief      Sets the message type.
 *
 * @param[in]  t     The new value
 */
void message::setMsgType(const msg_type_t t)
{
    std::get<headerInfoTupMessageType>(header) = t;
    isValid.set(headerInfoTupMessageType);
}

/**
 * @brief      Gets the protocol version.
 *
 * @return     The protocol version.
 */
auto message::getProtocolVersion() const -> std::tuple_element<headerInfoTupProtocolVersion, header_info_t>::type
{
    return std::get<headerInfoTupProtocolVersion>(header);
}

/**
 * @brief      Sets the protocol version.
 *
 * @param[in]  v     The new value
 */
void message::setProtocolVersion(const protocolVersion v)
{
    std::get<headerInfoTupProtocolVersion>(header) = v;
    std::get<headerInfoTupFlags>(header) &= 0x3FFF;
    isValid.set(headerInfoTupProtocolVersion);
    isValid.set(headerInfoTupFlags);
}

/**
 * @brief      Gets the broadcast response.
 *
 * @return     The broadcast response.
 */
auto message::getBroadcastResp() const -> std::tuple_element<headerInfoTupForceBroadcast, header_info_t>::type
{
    return std::get<headerInfoTupForceBroadcast>(header);
}

/**
 * @brief      Sets the broadcast response.
 *
 * @param[in]  resp  The response
 */
void message::setBroadcastResp(const bool resp)
{
    static constexpr flags_type broadcastRespMask{0x0200};

    auto& flagRef{std::get<headerInfoTupFlags>(header)};

    std::get<headerInfoTupForceBroadcast>(header) = resp;

    if(resp)
    {
        flagRef |= broadcastRespMask;
    }
    else
    {
        flagRef &= static_cast<flags_type>(~broadcastRespMask);
    }

    isValid.set(headerInfoTupForceBroadcast);
    isValid.set(headerInfoTupFlags);
}

/**
 * @brief      Gets the acknowledge.
 *
 * @return     The acknowledge.
 */
auto message::getAck() const -> std::tuple_element<headerInfoTupAckSetter, header_info_t>::type
{
    return std::get<headerInfoTupAckSetter>(header);
}

/**
 * @brief      Sets the acknowledge.
 *
 * @param[in]  a     The new value
 */
void message::setAck(const bool a)
{
    constexpr flags_type ackSetterMask{0x0100};

    auto& flagRef{std::get<headerInfoTupFlags>(header)};

    std::get<headerInfoTupAckSetter>(header) = a;

    if(a)
    {
        flagRef |= ackSetterMask;
    }
    else
    {
        flagRef &= static_cast<flags_type>(~ackSetterMask);
    }

    isValid.set(headerInfoTupAckSetter);
    isValid.set(headerInfoTupFlags);
}

/**
 * @brief      Gets the checksum type.
 *
 * @return     The checksum type.
 */
auto message::getChecksumType() const -> std::tuple_element<headerInfoTupChecksumType, header_info_t>::type
{
    static const std::unordered_map<flags_type, checksumTypes> mapping{
        {0x0001, checksumTypes::bits_8_xor}, {0x0002, checksumTypes::bits_8_xor_final_bitwise_negation},
        {0x0003, checksumTypes::bit_16_crc}
    };

    const auto& flagRef{std::get<headerInfoTupFlags>(header)};

    if(auto mapIt{mapping.find(flagRef & 0x0007)}; mapIt != mapping.end())
    {
        return mapIt->second;
    }

    return checksumTypes::ommitted;
}

/**
 * @brief      Sets the checksum type.
 *
 * @param[in]  ct    The new value
 */
void message::setChecksumType(const checksumTypes ct)
{
    static const std::unordered_map<checksumTypes, flags_type> mapping{
        {checksumTypes::ommitted, 0x0000}, {checksumTypes::bits_8_xor, 0x0001},
        {checksumTypes::bits_8_xor_final_bitwise_negation, 0x0002}, {checksumTypes::bit_16_crc, 0x0003}
    };

    auto& flagRef{std::get<headerInfoTupFlags>(header)};

    if(auto mapIt{mapping.find(ct)}; mapIt != mapping.end())
    {
        flagRef = (flagRef & 0xFFF8) | mapIt->second;
        isValid.set(headerInfoTupFlags);
    }
}

/**
 * @brief      Calculates the payload length.
 *
 * @return     The payload length.
 */
auto message::calcPayloadLen() -> std::pair<ser_msg_t::size_type, bitFieldSizes>
{
    auto& payloadLen{std::get<headerInfoTupPayloadLen>(header)};
    auto& payloadLenFieldSize{std::get<headerInfoTupPayloadLenFieldSize>(header)};
    payloadLen = std::accumulate(begin(), end(), ser_msg_t::size_type{1}); // Initial value of 1 is for Message Type
    payloadLenFieldSize = byteCountToBitField(helpers::types::bytesNeeded(payloadLen.value_or(0)));
    return {payloadLen.value_or(0), payloadLenFieldSize};
}

/**
 * @brief      Gets the payload length.
 *
 * @return     The payload length.
 */
auto message::getPayloadLen() const -> std::tuple_element<headerInfoTupPayloadLen, header_info_t>::type
{
    return std::get<headerInfoTupPayloadLen>(header);
}

/**
 * @brief      Gets the payload length size.
 *
 * @return     The payload length size.
 */
auto message::getPayloadLenSize() const -> std::tuple_element<headerInfoTupPayloadLenFieldSize, header_info_t>::type
{
    return std::get<headerInfoTupPayloadLenFieldSize>(header);
}

/**
 * @brief      Determines whether the specified a is address valid.
 *
 * @param[in]  a     Address to check.
 *
 * @return     True if the specified a is address valid, False otherwise.
 */
[[nodiscard]]
auto message::isAddrValid(const dev_addr_t a) const -> bool
{
    return ((a < 0x20) || (a == 0xFF));
}

/**
 * @brief      Gets the packed header.
 *
 * @return     The packed header.
 */
[[nodiscard]]
auto message::getPackedHeader() -> flags_type
{
    flags_type result = protoVersionToProtocolBitmask(getProtocolVersion())
        | seqNumSizeToProtocolBitmask(getSeqNumLen())
        | lengthSizeToProtocolBitmask(calcPayloadLen().second)
        | broadcastToProtocolBitmask(getBroadcastResp())
        | ackToProtocolBitmask(getAck())
        | checksumToProtocolBitmask(getChecksumType());

    return result;
}

/**
 * @brief      Gives the number of bytes that are needed to store a given Bit Field Size.
 *
 * @param[in]  bf    The Bit Field.
 *
 * @return     Number of Bytes.
 */
[[nodiscard]]
auto message::bitFieldToByteCount(const bitFieldSizes bf) -> std::size_t
{
    static const std::unordered_map<bitFieldSizes, std::size_t> mapping{
        {bitFieldSizes::bits_8, 1}, {bitFieldSizes::bits_16, 2}, {bitFieldSizes::bits_24, 3}
    };

    std::size_t result {0};

    if(auto mapIt{mapping.find(bf)}; mapIt != mapping.end())
    {
        result = mapIt->second;
    }

    return result;
}

/**
 * @brief      Converts a given Byte Cound to Bit Field Size
 *
 * @param[in]  bytes  The bytes
 *
 * @return     Bit Field size.
 */
[[nodiscard]]
auto message::byteCountToBitField(const std::size_t bytes) -> bitFieldSizes
{
    static const std::unordered_map<std::size_t, bitFieldSizes> mapping{
        {1, bitFieldSizes::bits_8}, {2, bitFieldSizes::bits_16}, {3, bitFieldSizes::bits_24}
    };

    bitFieldSizes result{bitFieldSizes::ommitted};

    if(auto mapIt{mapping.find(bytes)}; mapIt != mapping.end())
    {
        result = mapIt->second;
    }

    return result;
}

/**
 * @brief      Bytes needed to store a given Checksum
 *
 * @param[in]  cs    The checksum
 *
 * @return     The Byte count
 */
[[nodiscard]]
auto message::checksumToByteCount(const checksumTypes cs) -> std::size_t
{
    static const std::unordered_map<checksumTypes, std::size_t> mapping{
        {checksumTypes::ommitted, 0}, {checksumTypes::bits_8_xor, 1},
        {checksumTypes::bits_8_xor_final_bitwise_negation, 1}, {checksumTypes::bit_16_crc, 2}
    };

    if(auto mapIt{mapping.find(cs)}; mapIt != mapping.end())
    {
        return mapIt->second;
    }

    return 0;
}

/**
 * @brief      Determines the Protocol Version from the KISS Flags
 *
 * @param[in]  f     Flags
 *
 * @return     Protocol Version
 */
[[nodiscard]]
auto message::flagsToProtocolVersion(const flags_type f) -> protocolVersion
{
    static const std::unordered_map<flags_type, protocolVersion> mapping{
        {0x0000, protocolVersion::default_ver}, {0x4000, protocolVersion::default_ver},
        {0x8000, protocolVersion::default_ver}, {0xC000, protocolVersion::default_ver}
    };

    if(auto mapIt{mapping.find(f)}; mapIt != mapping.end())
    {
        return mapIt->second;
    }

    return protocolVersion::default_ver;
}

/**
 * @brief      Determines the Sequence Number Field Size from the KISS Flags
 *
 * @param[in]  f     Flags
 *
 * @return     Sequence Number Field Size
 */
[[nodiscard]]
auto message::flagsToSeqNumFieldSize(const flags_type f) -> bitFieldSizes
{
    static const std::unordered_map<flags_type, bitFieldSizes> mapping{
        {0x0000, bitFieldSizes::ommitted}, {0x1000, bitFieldSizes::bits_8}, {0x2000, bitFieldSizes::bits_16},
        {0x3000, bitFieldSizes::bits_24}
    };

    if(auto mapIt{mapping.find(f & 0x0300)}; mapIt != mapping.end())
    {
        return mapIt->second;
    }

    return bitFieldSizes::ommitted;
}

/**
 * @brief      Determines the Data Length Field Size from the KISS Flags
 *
 * @param[in]  f     Flags
 *
 * @return     Data Length Field Length
 */
[[nodiscard]]
auto message::flagsToLenFieldSize(const flags_type f) -> bitFieldSizes
{
    static const std::unordered_map<flags_type, bitFieldSizes> mapping{
        {0x0000, bitFieldSizes::ommitted}, {0x0400, bitFieldSizes::bits_8}, {0x0800, bitFieldSizes::bits_16},
        {0x0C00, bitFieldSizes::bits_24}
    };

    if(auto mapIt{mapping.find(f & 0x0C00)}; mapIt != mapping.end())
    {
        return mapIt->second;
    }

    return bitFieldSizes::ommitted;
}

/**
 * @brief      Determines the Broadcast Response from the KISS Flags
 *
 * @param[in]  f     Flags
 *
 * @return     Broadcast Response
 */
[[nodiscard]]
auto message::flagsToBroadcastResponse(const flags_type f) -> bool
{
    return ((f & 0x0200) != 0);
}

/**
 * @brief      Determines the Acknowledgement from the KISS Flags
 *
 * @param[in]  f     Flags
 *
 * @return     Acknowledgement
 */
[[nodiscard]]
auto message::flagsToAck(const flags_type f) -> bool
{
    return ((f & 0x0200) != 0);
}

/**
 * @brief      Determines the Checksum Type from the KISS Flags
 *
 * @param[in]  f     Flags
 *
 * @return     Checksum Type
 */
[[nodiscard]]
auto message::flagsToChecksumType(const flags_type f) -> checksumTypes
{
    static const std::unordered_map<flags_type, checksumTypes> mapping{
        {0x0000, checksumTypes::ommitted}, {0x0001, checksumTypes::bits_8_xor},
        {0x0002, checksumTypes::bits_8_xor_final_bitwise_negation}, {0x0003, checksumTypes::bit_16_crc}
    };

    if(auto mapIt{mapping.find(f & 0x0007)}; mapIt != mapping.end())
    {
        return mapIt->second;
    }

    return checksumTypes::ommitted;
}

/**
 * @brief      Determines the Header Size based on the KISS Flags
 *
 * @param[in]  f     Flags
 *
 * @return     Header Size
 */
[[nodiscard]]
auto message::flagsToHeaderSize(const flags_type f) -> std::size_t
{
    std::size_t result{4};

    result += bitFieldToByteCount(flagsToSeqNumFieldSize(f));
    result += bitFieldToByteCount(flagsToLenFieldSize(f));
    result += checksumToByteCount(flagsToChecksumType(f));

    return result;
}

/**
 * @brief      Gets to.
 *
 * @param[in]  m     Serial Message
 *
 * @return     To.
 */
[[nodiscard]]
auto message::getTo(const ser_msg_t& m) -> ser_data_t
{
    return helpers::serConn::getValIndex<ser_data_t, 1>(m, 0);
}

/**
 * @brief      Gets the from.
 *
 * @param[in]  m     Serial Message
 *
 * @return     The from.
 */
[[nodiscard]]
auto message::getFrom(const ser_msg_t& m) -> ser_data_t
{
    return helpers::serConn::getValIndex<ser_data_t, 1>(m, 1);
}

/**
 * @brief      Gets the flags.
 *
 * @param[in]  m     Serial Message
 *
 * @return     The flags.
 */
[[nodiscard]]
auto message::getFlags(const ser_msg_t& m) -> flags_type
{
    return helpers::serConn::getValIndex<flags_type>(m, 2);
}

/**
 * @brief      Gets an optional parameter.
 *
 * @param[in]  m     Serial Message
 * @param[in]  off   Offset
 * @param[in]  s     Field Size
 *
 * @return     The optional parameter.
 */
[[nodiscard]]
auto message::getOptParam(const ser_msg_t& m, const ser_msg_t::size_type& off, const bitFieldSizes& s) -> opt_uint32_t
{
    using namespace helpers::serConn;

    using funcDecl = std::function<opt_uint32_t(const ser_msg_t&, const ser_msg_t::size_type&)>;
    static constexpr auto funcOmmit = [&]([[maybe_unused]] const auto& msg, [[maybe_unused]] const auto& offset){
        return std::nullopt; };
    static constexpr auto funcBits8 = [&](const auto& msg, const auto& offset){
        return getValIndex<std::uint32_t, 1>(msg, offset); };
    static constexpr auto funcBits16 = [&](const auto& msg, const auto& offset){
        return getValIndex<std::uint32_t, 2>(msg, offset); };
    static constexpr auto funcBits24 = [&](const auto& msg, const auto& offset){
        return getValIndex<std::uint32_t, 3>(msg, offset); };

    static const std::unordered_map<bitFieldSizes, funcDecl> mapping{
        {bitFieldSizes::ommitted, funcOmmit}, {bitFieldSizes::bits_8, funcBits8}, {bitFieldSizes::bits_16, funcBits16},
        {bitFieldSizes::bits_24, funcBits24}
    };

    if(auto mapIt{mapping.find(s)}; mapIt != mapping.end())
    {
        return mapIt->second(m, off);
    }

    return std::nullopt;
}

/**
 * @brief      Gets the checksum.
 *
 * @param[in]  m     Serial Message
 * @param[in]  t     Checksum Type
 *
 * @return     The checksum.
 */
[[nodiscard]]
auto message::getChecksum(const ser_msg_t& m, const checksumTypes& t) -> opt_uint32_t
{
    using namespace helpers::serConn;

    using funcDecl = std::function<opt_uint32_t(const ser_msg_t&)>;
    static constexpr auto funcOmmit = [&]([[maybe_unused]] const ser_msg_t& msg){ return std::nullopt; };
    static constexpr auto funcBits8 = [&](const ser_msg_t& msg){
        return getValIndex<std::uint32_t, 1>(msg, (msg.size() - 1)); };
    static constexpr auto funcBits16 = [&](const ser_msg_t& msg){
        return getValIndex<std::uint32_t, 2>(msg, (msg.size() - 2)); };

    static const std::unordered_map<checksumTypes, funcDecl> mapping{
        {checksumTypes::ommitted, funcOmmit}, {checksumTypes::bits_8_xor, funcBits8},
        {checksumTypes::bits_8_xor_final_bitwise_negation, funcBits8}, {checksumTypes::bit_16_crc, funcBits16}
    };

    if(auto mapIt{mapping.find(t)}; mapIt != mapping.end())
    {
        return mapIt->second(m);
    }

    return std::nullopt;
}

/**
 * @brief      Gets the header information.
 *
 * @param[in]  m     Serial Message
 *
 * @return     The header information.
 */
[[nodiscard]]
auto message::getHeaderInfo(const ser_msg_t& m) -> header_info_t
{
    using namespace helpers::serConn;

    const auto to{getTo(m)};
    const auto from{getFrom(m)};
    const auto flags{getFlags(m)};
    const auto forceBroadcast{flagsToBroadcastResponse(flags)};
    const auto ackSetter{flagsToAck(flags)};
    const auto protocolVersion{flagsToProtocolVersion(flags)};
    const auto seqNumFieldSize{flagsToSeqNumFieldSize(flags)};
    const auto payloadLenFieldSize{flagsToLenFieldSize(flags)};
    const auto checksumType{flagsToChecksumType(flags)};
    const auto seqNum{getOptParam(m, seqNumOffBase, seqNumFieldSize)};
    const auto payloadLen{getOptParam(m, (seqNumOffBase + bitFieldToByteCount(seqNumFieldSize)), payloadLenFieldSize)};
    const auto checksum{getChecksum(m, checksumType)};
    const auto payloadOffset{seqNumOffBase + bitFieldToByteCount(seqNumFieldSize) +
        bitFieldToByteCount(payloadLenFieldSize) + 1};
    const auto messageType{getValIndex<msg_type_t, 1>(m, (payloadOffset - 1))};

    return std::make_tuple(to, from, flags, forceBroadcast, ackSetter, protocolVersion, seqNumFieldSize, seqNum,
        payloadLenFieldSize, payloadLen, checksumType, checksum, payloadOffset, messageType);
}

/**
 * @brief      Gives a Flags Bitmap with the given Protocol Version bits set
 *
 * @param[in]  proto  The protocol
 *
 * @return     Flags
 */
[[nodiscard]]
auto message::protoVersionToProtocolBitmask(const protocolVersion& proto) -> flags_type
{
    static const std::unordered_map<protocolVersion, flags_type> mapping{
        {protocolVersion::default_ver, 0}
    };

    flags_type result{0};

    if(auto it{mapping.find(proto)}; it != mapping.end())
    {
        result = it->second;
    }

    return result;
}

/**
 * @brief      Gives a Flags Bitmap with the given Sequence Number Size bits set
 *
 * @param[in]  size  The Sequence Number Bytes
 *
 * @return     Flags
 */
[[nodiscard]]
auto message::seqNumSizeToProtocolBitmask(const bitFieldSizes& size) -> flags_type
{
    static constexpr flags_type seqBitFildOffset{12};
    flags_type result{static_cast<flags_type>(((bitFieldToByteCount(size) & 0x0003) << seqBitFildOffset))};
    return result;
}

/**
 * @brief      Gives a Flags Bitmap with the given Payload Length Size bits set
 *
 * @param[in]  size  The Payload Length Field Size
 *
 * @return     Flags
 */
[[nodiscard]]
auto message::lengthSizeToProtocolBitmask(const bitFieldSizes& size) -> flags_type
{
    static constexpr flags_type lenBitFildOffset{10};
    flags_type result{static_cast<flags_type>(((bitFieldToByteCount(size) & 0x0003) << lenBitFildOffset))};
    return result;
}

/**
 * @brief      Gives a Flags Bitmap with the given Broadcast bit set
 *
 * @param[in]  en    The Broadcast Enable
 *
 * @return     Flags
 */
[[nodiscard]]
constexpr auto message::broadcastToProtocolBitmask(const bool& en) -> flags_type
{
    constexpr flags_type broadcastRespBitmask{0x0200};
    flags_type result{0};

    if(en)
    {
        result = broadcastRespBitmask;
    }

    return result;
}

/**
 * @brief      Gives a Flags Bitmap with the given Acknowledgement bit set
 *
 * @param[in]  en    The Acknowledgement Enable
 *
 * @return     Flags
 */
[[nodiscard]]
constexpr auto message::ackToProtocolBitmask(const bool& en) -> flags_type
{
    constexpr flags_type ackReceiptBitmask{0x0100};
    flags_type result{0};

    if(en)
    {
        result = ackReceiptBitmask;
    }

    return result;
}

/**
 * @brief      Gives a Flags Bitmap with the given Checksum Type bits set
 *
 * @param[in]  csType  The Checksum Type
 *
 * @return     Flags
 */
[[nodiscard]]
auto message::checksumToProtocolBitmask(const checksumTypes& csType) -> flags_type
{
    return static_cast<flags_type>(checksumToByteCount(csType) & 0x0007);
}

/**
 * @brief      Calculates the checksum.
 *
 * @param      msg     The Message
 * @param[in]  csType  The Checksum Type
 */
void message::calculateChecksum(ser_msg_t& msg, const checksumTypes& csType)
{
    auto backIt{std::back_inserter(msg)};

    if((checksumTypes::bits_8_xor == csType) || (checksumTypes::bits_8_xor_final_bitwise_negation == csType))
    {
        std::uint8_t checksum{0};

        checksum = std::accumulate(std::begin(msg), std::end(msg), std::uint8_t{0}, std::bit_xor<std::uint8_t>());

        if(checksumTypes::bits_8_xor_final_bitwise_negation == csType)
        {
            checksum ^= 0xFF;
        }

        helpers::serConn::putVal(backIt, checksum);
    }
}
