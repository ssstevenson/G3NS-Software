#include <controller/kissChunk.h>

#include <unordered_map>

using namespace empower::kiss;

/**
 * @brief      Constructs a new instance.
 */
chunk::chunk():
    protoVer{protocolVersion::default_ver},
    cmdId{cmd_id_t::dummy},
    data{std::nullopt}
{}

/**
 * @brief      Constructs a new instance.
 *
 * @param      it    Serial Message Iterator
 */
chunk::chunk(ser_msg_t::const_iterator& it):
    chunk{}
{
    consume(it);
}

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  id    The Command ID
 */
chunk::chunk(const cmd_id_t& id):
    protoVer{protocolVersion::default_ver},
    cmdId{id},
    data{std::nullopt}
{}

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  id    The Command ID
 * @param[in]  d     The Data Value
 */
chunk::chunk(const cmd_id_t& id, const data_t& d):
    protoVer{protocolVersion::default_ver},
    cmdId{id},
    data{d}
{}

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  id    The Command ID
 * @param[in]  d     The Data Value
 * @param[in]  ver   The Protocol Version
 */
chunk::chunk(const cmd_id_t& id, const data_t& d, const protocolVersion& ver):
    protoVer{ver},
    cmdId{id},
    data{d}
{}

/**
 * @brief      Converts a Chunk to the number of bytes it would hold in a serial message.
 *
 * @return     The number of Bytes
 */
chunk::operator ser_msg_t::size_type()
{
    return size();
}

/**
 * @brief      Pulls in a Chunk from a given point in a Serial Message
 *
 * @param      it    The Serial Message iterator
 */
void chunk::consume(ser_msg_t::const_iterator& it)
{
    using namespace helpers::serConn;

    // Fill in Status Flags
    auto status{getVal<std::uint8_t>(it)};
    auto idLen{statusToCmdIdLen(status)};
    protoVer = statusToProtoVer(status);

    // Read Command ID
    static const std::unordered_map<cmd_id_len_t, std::function<cmd_id_t(ser_msg_t::const_iterator&)>> cmdIdMapping{
        {cmd_id_len_t::bits_8, [&](auto& l_it){ return getVal<cmd_id_t, 1>(l_it); }},
        {cmd_id_len_t::bits_16, [&](auto& l_it){ return getVal<cmd_id_t, 2>(l_it); }},
        {cmd_id_len_t::bits_24, [&](auto& l_it){ return getVal<cmd_id_t, 3>(l_it); }},
        {cmd_id_len_t::bits_32, [&](auto& l_it){ return getVal<cmd_id_t, 4>(l_it); }}
    };

    if(auto mapIt{cmdIdMapping.find(idLen)}; mapIt != cmdIdMapping.end())
    {
        cmdId = mapIt->second(it);
    }

    // Read Data
    const auto dataType{getVal<std::uint8_t>(it)};
    constexpr std::uint8_t byteArrSize{static_cast<std::uint8_t>(data_type_t::t_byte_array)};
    if(dataType < byteArrSize)
    {
        static const std::unordered_map<data_type_t, std::function<void(data_t&, ser_msg_t::const_iterator&)>> mapping{
            {data_type_t::t_bit, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<bool>(msgIt); }},
            {data_type_t::t_int8, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<std::int8_t>(msgIt); }},
            {data_type_t::t_uint8, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<std::uint8_t>(msgIt); }},
            {data_type_t::t_int16, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<std::int16_t>(msgIt); }},
            {data_type_t::t_uint16, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<std::uint16_t>(msgIt); }},
            {data_type_t::t_int32, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<std::int32_t>(msgIt); }},
            {data_type_t::t_uint32, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<std::uint32_t>(msgIt); }},
            {data_type_t::t_int64, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<std::int64_t>(msgIt); }},
            {data_type_t::t_uint64, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<std::uint64_t>(msgIt); }},
            {data_type_t::t_float, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<float>(msgIt); }},
            {data_type_t::t_double, [](data_t& d, ser_msg_t::const_iterator& msgIt){
                d = getVal<double>(msgIt); }},
            {data_type_t::t_ldouble, []([[maybe_unused]] data_t& d, ser_msg_t::const_iterator& msgIt){
                std::advance(msgIt, 10); }}
        };

        if(auto mapIt{mapping.find(static_cast<data_type_t>(dataType))}; mapIt != mapping.end())
        {
            mapIt->second(data, it);
        }
    }
    else
    {
        const auto arrLen{dataType - 127};
        data = base_vec_t{};
        std::copy(it, std::next(it, arrLen), std::back_inserter(std::get<base_vec_t>(data.value())));
        std::advance(it, arrLen);
    }
}

/**
 * @brief      Converts this Chunk to a Serial Message
 *
 * @return     The Serial Message
 */
[[nodiscard]]
auto chunk::serialize() -> ser_msg_t
{
    ser_msg_t msg;
    serialize(msg);
    return msg;
}

/**
 * @brief      Converts this chunk to a Serial Message at the tail of the given Iterator
 *
 * @param      msg   The message
 */
void chunk::serialize(ser_msg_t& msg)
{
    serialize(std::back_inserter(msg));
}

/**
 * @brief      Converts this chunk to a Serial Message using the given back inserter iterator
 *
 * @param      it    The back inserter iterator
 */
void chunk::serialize(ser_msg_back_in_it&& it)
{
    serialize(it);
}

/**
 * @brief      Converts this chunk to a Serial Message using the given back inserter iterator
 *
 * @param      it    The back inserter iterator
 */
void chunk::serialize(ser_msg_back_in_it& it)
{
    using namespace helpers::serConn;

    static const std::unordered_map<std::size_t, std::function<
        void(ser_msg_back_in_it&, const std::underlying_type_t<cmd_id_t>&)>> cmdIdMapping
    {
        {1, [](auto& lamdaIt, const auto& d){ putVal<std::underlying_type_t<cmd_id_t>, 1>(lamdaIt, d); }},
        {2, [](auto& lamdaIt, const auto& d){ putVal<std::underlying_type_t<cmd_id_t>, 2>(lamdaIt, d); }},
        {3, [](auto& lamdaIt, const auto& d){ putVal<std::underlying_type_t<cmd_id_t>, 3>(lamdaIt, d); }},
        {4, [](auto& lamdaIt, const auto& d){ putVal<std::underlying_type_t<cmd_id_t>, 4>(lamdaIt, d); }},
    };

    const std::size_t cmdIdBytesNeeded{helpers::types::bytesNeeded(static_cast<std::underlying_type_t<cmd_id_t>>(cmdId))};

    // Status Field
    putVal<ser_data_t, 1>(it,
        (protoVerToStatusBitField(protoVer) | cmdIdLenToStatusBitField(numBytesToCmdIdLen(cmdIdBytesNeeded))));

    // Command ID Field
    if(auto mapIt{cmdIdMapping.find(cmdIdBytesNeeded)}; mapIt != cmdIdMapping.end())
    {
        mapIt->second(it, static_cast<std::underlying_type_t<cmd_id_t>>(cmdId));
    }

    // Data Type Field
    const auto dataType{dataToDataType(data)};
    putVal(it, dataType);

    // Data
    if(data.has_value())
    {
        std::visit([&it](auto&& arg){
            using T = std::decay_t<decltype(arg)>;

            if constexpr(std::is_same_v<T, base_vec_t>)
            {
                std::copy(std::begin(arg), std::end(arg), it);
            }
            else if constexpr(std::is_same_v<T, float>)
            {
                putVal(it, static_cast<std::uint32_t>(arg));
            }
            else if constexpr(std::is_same_v<T, double>)
            {
                putVal(it, static_cast<std::uint64_t>(arg));
            }
            else if constexpr(!std::is_same_v<T, long double>)
            {
                putVal(it, arg);
            }
        }, data.value());
    }
}

/**
 * @brief      Calculates the number of Bytes needed in a Serial Message for the chunk
 *
 * @return     Byte Count
 */
[[nodiscard]]
auto chunk::size() const -> ser_msg_t::size_type
{
    static constexpr ser_msg_t::size_type baseSize{2}; // Command Len and Data Type fields
    static const std::unordered_map<data_type_t, ser_msg_t::size_type> typeMapping{
        {data_type_t::t_void, 0}, {data_type_t::t_bit, 1}, {data_type_t::t_int8, 1}, {data_type_t::t_uint8, 1},
        {data_type_t::t_int16, 2}, {data_type_t::t_uint16, 2}, {data_type_t::t_int32, 4}, {data_type_t::t_uint32, 4},
        {data_type_t::t_int64, 8}, {data_type_t::t_uint64, 8}, {data_type_t::t_float, 4}, {data_type_t::t_double, 8},
        {data_type_t::t_ldouble, 10}, {data_type_t::t_ack, 1}
    };

    data_type_t dataType{dataToDataType(data)};

    ser_msg_t::size_type result{baseSize};

    // Dynamic length of Command ID
    result += helpers::types::bytesNeeded(static_cast<std::underlying_type_t<cmd_id_t>>(cmdId));

    if(auto it{typeMapping.find(dataType)}; it != typeMapping.end())
    {
        result += it->second;
    }
    else if(dataType >= data_type_t::t_byte_array)
    {
        result += static_cast<ser_msg_t::size_type>(static_cast<std::underlying_type_t<data_type_t>>(dataType) -
            static_cast<std::underlying_type_t<data_type_t>>(data_type_t::t_byte_array));
    }

    return result;
}

/**
 * @brief      Gets the protocol version.
 *
 * @return     The protocol version.
 */
[[nodiscard]]
auto chunk::getProtocolVersion() const -> protocolVersion
{
    return protoVer;
}

/**
 * @brief      Sets the protocol version.
 *
 * @param[in]  ver   The new value
 */
void chunk::setProtocolVersion(const protocolVersion& ver)
{
    protoVer = ver;
}

/**
 * @brief      Gets the command identifier.
 *
 * @return     The command identifier.
 */
[[nodiscard]]
auto chunk::getCommandId() const -> cmd_id_t
{
    return cmdId;
}

/**
 * @brief      Sets the command identifier.
 *
 * @param[in]  id    The new value
 */
void chunk::setCommandId(const cmd_id_t& id)
{
    cmdId = id;
}

/**
 * @brief      Gets the data.
 *
 * @return     The data.
 */
[[nodiscard]]
auto chunk::getData() const -> data_t
{
    return data;
}

/**
 * @brief      Sets the data.
 *
 * @param[in]  d     The new value
 */
void chunk::setData(const data_t& d)
{
    data = std::move(d);
}

/**
 * @brief      Calculates how many bytes are needed to store a given Data Type
 *
 * @param[in]  dt    The Data Type
 *
 * @return     Byte Count
 */
[[nodiscard]]
auto chunk::dataTypeToBytes(const data_type_t& dt) -> std::size_t
{
    static const std::unordered_map<data_type_t, std::size_t> mapping{
        {data_type_t::t_bit, 1}, {data_type_t::t_int8, 1}, {data_type_t::t_uint8, 1},
        {data_type_t::t_int16, 2}, {data_type_t::t_uint16, 2}, {data_type_t::t_int32, 4}, {data_type_t::t_uint32, 4},
        {data_type_t::t_int64, 8}, {data_type_t::t_uint64, 8}, {data_type_t::t_float, 4}, {data_type_t::t_double, 8},
        {data_type_t::t_ldouble, 10}
    };
    std::size_t result{0};

    if(static_cast<std::size_t>(dt) >= static_cast<std::size_t>(data_type_t::t_byte_array))
    {
        result = static_cast<std::size_t>(dt) - static_cast<std::size_t>(data_type_t::t_byte_array);
    }
    else
    {
        if(auto mapIt{mapping.find(dt)}; mapIt != mapping.end())
        {
            result = mapIt->second;
        }
    }

    return result;
}

/**
 * @brief      Calculates the number of Bytes needed for a given Command ID
 *
 * @param[in]  id    The Command ID
 *
 * @return     Byte Count
 */
[[nodiscard]]
auto chunk::cmdIdLenToBytes(const cmd_id_len_t& id) -> std::size_t
{
    static const std::unordered_map<cmd_id_len_t, std::size_t> mapping{
        {cmd_id_len_t::bits_8, 1}, {cmd_id_len_t::bits_16, 2}, {cmd_id_len_t::bits_24, 3}, {cmd_id_len_t::bits_32, 4}
    };

    if(auto mapIt{mapping.find(id)}; mapIt != mapping.end())
    {
        return mapIt->second;
    }

    return 0;
}

/**
 * @brief      Gets the chunk length.
 *
 * @param[in]  it    The iterator
 *
 * @return     The chunk length.
 */
[[nodiscard]]
auto chunk::getChunkLen(const ser_msg_t::const_iterator& it) -> ser_msg_t::size_type
{
    ser_data_t status{*it};
    auto cmdIdLen{static_cast<ser_msg_t::difference_type>(statusToCmdIdLen(status))};
    ser_msg_t::const_iterator cmdTypeItr{std::next(it, cmdIdLen + 1)};
    auto cmdDataType{helpers::serConn::getVal<data_type_t, 1>(cmdTypeItr)};
    // Status Byte + Command ID + Data Type + Data
    return (2 + static_cast<ser_msg_t::size_type>(cmdIdLen) + dataTypeToBytes(cmdDataType));
}

/**
 * @brief      Converts a given Command ID Length to Status Flags Bit Field
 *
 * @param[in]  cmdIdLen  The command identifier length
 *
 * @return     Flgas
 */
[[nodiscard]]
auto chunk::cmdIdLenToStatusBitField(const cmd_id_len_t& cmdIdLen) -> ser_data_t
{
    static const std::unordered_map<cmd_id_len_t, ser_data_t> mapping{
        {cmd_id_len_t::bits_8, 0x00}, {cmd_id_len_t::bits_16, 0x01}, {cmd_id_len_t::bits_24, 0x02},
        {cmd_id_len_t::bits_32, 0x03}
    };

    ser_data_t result{0};

    if(auto mapIt{mapping.find(cmdIdLen)}; mapIt != mapping.end())
    {
        result = mapIt->second;
    }

    return result;
}

/**
 * @brief      Gets the Command ID Length based on Status Flags
 *
 * @param[in]  ch    Serial Character
 *
 * @return     The Command ID Length
 */
[[nodiscard]]
auto chunk::statusToCmdIdLen(const ser_data_t& ch) -> cmd_id_len_t
{
    static const std::unordered_map<std::size_t, cmd_id_len_t> mapping{
        {0, cmd_id_len_t::bits_8}, {1, cmd_id_len_t::bits_16}, {2, cmd_id_len_t::bits_24}, {3, cmd_id_len_t::bits_32}
    };

    if(auto mapIt{mapping.find(ch & 0x03)}; mapIt != mapping.end())
    {
        return mapIt->second;
    }

    return cmd_id_len_t::bits_8;
}

/**
 * @brief      Calculates the number of bytes needed for a given Command ID
 *
 * @param[in]  bytes  The bytes
 *
 * @return     Byte Count
 */
[[nodiscard]]
auto chunk::numBytesToCmdIdLen(const std::size_t& bytes) -> cmd_id_len_t
{
    static const std::unordered_map<std::size_t, cmd_id_len_t> mapping{
        {1, cmd_id_len_t::bits_8}, {2, cmd_id_len_t::bits_16}, {3, cmd_id_len_t::bits_24}, {4, cmd_id_len_t::bits_32}
    };

    cmd_id_len_t result{cmd_id_len_t::bits_32};

    if(auto mapIt{mapping.find(bytes)}; mapIt != mapping.end())
    {
        result = mapIt->second;
    }

    return result;
}

/**
 * @brief      Calculates a Protocol Version based on a Status Serial Message Character.
 *
 * @param[in]  ch    The Serial Message Character.
 *
 * @return     Protocol Version
 */
[[nodiscard]]
auto chunk::statusToProtoVer(const ser_data_t& ch) -> protocolVersion
{
    static const std::unordered_map<ser_data_t, protocolVersion> mapping{
        {0, protocolVersion::default_ver}
    };

    if(auto mapIt{mapping.find(ch & 0xC0)}; mapIt != mapping.end())
    {
        return mapIt->second;
    }

    return protocolVersion::default_ver;
}

/**
 * @brief      Calculates the Status Bit Field for a given Protocol Version
 *
 * @param[in]  ver   The Protocol Version
 *
 * @return     Status Bit Field
 */
[[nodiscard]]
auto chunk::protoVerToStatusBitField(const protocolVersion& ver) -> ser_data_t
{
    static const std::unordered_map<protocolVersion, ser_data_t> mapping{
        {protocolVersion::default_ver, 0}
    };

    ser_data_t result{0};

    if(auto mapIt{mapping.find(ver)}; mapIt != mapping.end())
    {
        result = mapIt->second;
    }

    return result;
}

/**
 * @brief      Gives the Data Type of a given stored Data field
 *
 * @param[in]  d     The Data Field
 *
 * @return     The Data Type
 */
[[nodiscard]]
auto chunk::dataToDataType(const data_t& d) -> data_type_t
{
    data_type_t result{data_type_t::t_void};

    if(d.has_value())
    {
        auto idx{static_cast<std::underlying_type_t<data_type_t>>(d.value().index() + 1)};

        if(idx >= 12)
        {
            result = data_type_t::t_byte_array;
        }
        else
        {
            result = static_cast<data_type_t>(idx);
        }
    }

    return result;
}
