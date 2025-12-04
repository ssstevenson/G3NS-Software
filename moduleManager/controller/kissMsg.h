#ifndef KISS_MSG_H_
#define KISS_MSG_H_

#include <bitset>
#include <controller/kissChunk.h>
#include <optional>
#include <serialInterfaceConnection/serialInterfaceConnection.h>
#include <tuple>

namespace empower::kiss
{
    /**
     * @brief      This class describes a KISS Message.
     */
    class message
    {
    public:
        using ser_msg_t = serialInterfaceConnection::ser_msg_t;
        using ser_data_t = helpers::types::ser_data_t;
        using chunk_vec_t = std::vector<chunk>;
        using chunk_iterator = chunk_vec_t::iterator;
        using const_chunk_iterator = chunk_vec_t::const_iterator;
        using chunk_back_insert_iterator = std::back_insert_iterator<chunk_vec_t>;
        using dev_addr_t = helpers::types::ser_data_t;
        using flags_type = std::uint16_t;
        using opt_uint32_t = std::optional<std::uint32_t>;
        using ser_msg_back_in_it = std::back_insert_iterator<ser_msg_t>;

        enum class protocolVersion
        {
            default_ver
        };

        enum class bitFieldSizes
        {
            ommitted, bits_8, bits_16, bits_24
        };

        enum class checksumTypes
        {
            ommitted, bits_8_xor, bits_8_xor_final_bitwise_negation, bit_16_crc
        };

        enum class msg_type_t: ser_data_t
        {
            command = 0xFF, query = 0xFE, setterAck = 0xFD, queryResp = 0xFC,
            g_00 = 0x00, g_01 = 0x01, g_02 = 0x02, g_03 = 0x03, g_04 = 0x04, g_05 = 0x05, g_06 = 0x06, g_07 = 0x07,
            g_08 = 0x08, g_09 = 0x09, g_0A = 0x0A, g_0B = 0x0B, g_0C = 0x0C, g_0D = 0x0D, g_0E = 0x0E, g_0F = 0x0F,
            g_10 = 0x10, g_11 = 0x11, g_12 = 0x12, g_13 = 0x13, g_14 = 0x14, g_15 = 0x15, g_16 = 0x16, g_17 = 0x17,
            g_18 = 0x18, g_19 = 0x19, g_1A = 0x1A, g_1B = 0x1B, g_1C = 0x1C, g_1D = 0x1D, g_1E = 0x1E, g_1F = 0x1F,
            g_20 = 0x20, g_21 = 0x21, g_22 = 0x22, g_23 = 0x23, g_24 = 0x24, g_25 = 0x25, g_26 = 0x26, g_27 = 0x27,
            g_28 = 0x28, g_29 = 0x29, g_2A = 0x2A, g_2B = 0x2B, g_2C = 0x2C, g_2D = 0x2D, g_2E = 0x2E, g_2F = 0x2F,
            g_30 = 0x30, g_31 = 0x31, g_32 = 0x32, g_33 = 0x33, g_34 = 0x34, g_35 = 0x35, g_36 = 0x36, g_37 = 0x37,
            g_38 = 0x38, g_39 = 0x39, g_3A = 0x3A, g_3B = 0x3B, g_3C = 0x3C, g_3D = 0x3D, g_3E = 0x3E, g_3F = 0x3F,
            g_40 = 0x40, g_41 = 0x41, g_42 = 0x42, g_43 = 0x43, g_44 = 0x44, g_45 = 0x45, g_46 = 0x46, g_47 = 0x47,
            g_48 = 0x48, g_49 = 0x49, g_4A = 0x4A, g_4B = 0x4B, g_4C = 0x4C, g_4D = 0x4D, g_4E = 0x4E, g_4F = 0x4F,
            g_50 = 0x50, g_51 = 0x51, g_52 = 0x52, g_53 = 0x53, g_54 = 0x54, g_55 = 0x55, g_56 = 0x56, g_57 = 0x57,
            g_58 = 0x58, g_59 = 0x59, g_5A = 0x5A, g_5B = 0x5B, g_5C = 0x5C, g_5D = 0x5D, g_5E = 0x5E, g_5F = 0x5F,
            g_60 = 0x60, g_61 = 0x61, g_62 = 0x62, g_63 = 0x63, g_64 = 0x64, g_65 = 0x65, g_66 = 0x66, g_67 = 0x67,
            g_68 = 0x68, g_69 = 0x69, g_6A = 0x6A, g_6B = 0x6B, g_6C = 0x6C, g_6D = 0x6D, g_6E = 0x6E, g_6F = 0x6F,
            g_70 = 0x70, g_71 = 0x71, g_72 = 0x72, g_73 = 0x73, g_74 = 0x74, g_75 = 0x75, g_76 = 0x76, g_77 = 0x77,
            g_78 = 0x78, g_79 = 0x79, g_7A = 0x7A, g_7B = 0x7B, g_7C = 0x7C, g_7D = 0x7D, g_7E = 0x7E, g_7F = 0x7F,

            queryRespGrpStart = g_00, queryRespGrpEnd = g_7F,

            grp_bootloader = g_00, grp_general = g_01, grp_man_info = g_02, grp_analogs = g_03,
            grp_get_num_bytes = g_04, grp_get_flash_crc = g_05
        };

        using header_info_t = std::tuple<ser_data_t, ser_data_t, flags_type, bool, bool, protocolVersion, bitFieldSizes,
            opt_uint32_t, bitFieldSizes, std::optional<ser_msg_t::size_type>, checksumTypes,
            opt_uint32_t, ser_msg_t::difference_type, msg_type_t>;
        static constexpr std::size_t headerInfoTupTo = 0;
        static constexpr std::size_t headerInfoTupFrom = 1;
        static constexpr std::size_t headerInfoTupFlags = 2;
        static constexpr std::size_t headerInfoTupForceBroadcast = 3;
        static constexpr std::size_t headerInfoTupAckSetter = 4;
        static constexpr std::size_t headerInfoTupProtocolVersion = 5;
        static constexpr std::size_t headerInfoTupSeqNumFieldSize = 6;
        static constexpr std::size_t headerInfoTupSeqNum = 7;
        static constexpr std::size_t headerInfoTupPayloadLenFieldSize = 8;
        static constexpr std::size_t headerInfoTupPayloadLen = 9;
        static constexpr std::size_t headerInfoTupChecksumType = 10;
        static constexpr std::size_t headerInfoTupChecksum = 11;
        static constexpr std::size_t headerInfoTupPayloadOffset = 12;
        static constexpr std::size_t headerInfoTupMessageType = 13;

    private:
        static constexpr ser_msg_t::size_type seqNumOffBase = 4;

        header_info_t header;
        chunk_vec_t chunks;
        std::bitset<std::tuple_size_v<header_info_t>> isValid;
        std::optional<ser_msg_t> rawMessage;

    public:
        message();
        message(const ser_msg_t& m);
        message(const dev_addr_t fromAddr, const dev_addr_t toAddr, const std::optional<std::size_t>& seqNum,
            const msg_type_t msgType, const protocolVersion protocol, const bool broadcastResp, const bool ack,
            const checksumTypes csType);

        void clear();

        operator bool() const;
        message& operator=(const ser_msg_t& m);
        auto begin() -> decltype(chunks.begin());
        auto end() -> decltype(chunks.end());
        auto cbegin() -> decltype(chunks.cbegin());
        auto cend() -> decltype(chunks.cend());
        void push_back(const chunk& ch);
        template<class... Args> void emplace_back(Args&&... args);

        void fromSerMsg(const ser_msg_t& m);
        [[nodiscard]] auto serialize() -> ser_msg_t;
        void packOpt(ser_msg_back_in_it& it, const opt_uint32_t& val, const bitFieldSizes& size);
        [[nodiscard]] auto getRawMessage() const -> std::optional<ser_msg_t>;

        auto getFromAddr() const -> std::tuple_element<headerInfoTupFrom, header_info_t>::type;
        void setFromAddr(const dev_addr_t a);
        auto getToAddr() const -> std::tuple_element<headerInfoTupTo, header_info_t>::type;
        void setToAddr(const dev_addr_t a);
        auto getSeqNum() const -> std::tuple_element<headerInfoTupSeqNum, header_info_t>::type;
        auto getSeqNumLen() const -> std::tuple_element<headerInfoTupSeqNumFieldSize, header_info_t>::type;
        void setSeqNum(const opt_uint32_t& seq = std::nullopt);
        auto getMsgType() const -> std::tuple_element<headerInfoTupMessageType, header_info_t>::type;
        void setMsgType(const msg_type_t t);
        auto getProtocolVersion() const -> std::tuple_element<headerInfoTupProtocolVersion, header_info_t>::type;
        void setProtocolVersion(const protocolVersion v);
        auto getBroadcastResp() const -> std::tuple_element<headerInfoTupForceBroadcast, header_info_t>::type;
        void setBroadcastResp(const bool resp);
        auto getAck() const -> std::tuple_element<headerInfoTupAckSetter, header_info_t>::type;
        void setAck(const bool a);
        auto getChecksumType() const -> std::tuple_element<headerInfoTupChecksumType, header_info_t>::type;
        void setChecksumType(const checksumTypes ct);
        auto calcPayloadLen() -> std::pair<ser_msg_t::size_type, bitFieldSizes>;
        auto getPayloadLen() const -> std::tuple_element<headerInfoTupPayloadLen, header_info_t>::type;
        auto getPayloadLenSize() const -> std::tuple_element<headerInfoTupPayloadLenFieldSize, header_info_t>::type;

    private:
        [[nodiscard]] auto isAddrValid(const dev_addr_t a) const -> bool;
        [[nodiscard]] auto getPackedHeader() -> flags_type;

        [[nodiscard]] static auto bitFieldToByteCount(const bitFieldSizes bf) -> std::size_t;
        [[nodiscard]] static auto byteCountToBitField(const std::size_t bytes) -> bitFieldSizes;
        [[nodiscard]] static auto checksumToByteCount(const checksumTypes cs) -> std::size_t;
        [[nodiscard]] static auto flagsToProtocolVersion(const flags_type f) -> protocolVersion;
        [[nodiscard]] static auto flagsToSeqNumFieldSize(const flags_type f) -> bitFieldSizes;
        [[nodiscard]] static auto flagsToLenFieldSize(const flags_type f) -> bitFieldSizes;
        [[nodiscard]] static auto flagsToBroadcastResponse(const flags_type f) -> bool;
        [[nodiscard]] static auto flagsToAck(const flags_type f) -> bool;
        [[nodiscard]] static auto flagsToChecksumType(const flags_type f) -> checksumTypes;
        [[nodiscard]] static auto flagsToHeaderSize(const flags_type f) -> std::size_t;
        [[nodiscard]] static auto getTo(const ser_msg_t& m) -> ser_data_t;
        [[nodiscard]] static auto getFrom(const ser_msg_t& m) -> ser_data_t;
        [[nodiscard]] static auto getFlags(const ser_msg_t& m) -> flags_type;
        [[nodiscard]] static auto getOptParam(const ser_msg_t& m, const ser_msg_t::size_type& off,
            const bitFieldSizes& s) -> opt_uint32_t;
        [[nodiscard]] static auto getChecksum(const ser_msg_t& m, const checksumTypes& t) -> opt_uint32_t;
        [[nodiscard]] static auto getHeaderInfo(const ser_msg_t& m) -> header_info_t;

        [[nodiscard]] static auto protoVersionToProtocolBitmask(const protocolVersion& proto) -> flags_type;
        [[nodiscard]] static auto seqNumSizeToProtocolBitmask(const bitFieldSizes& size) -> flags_type;
        [[nodiscard]] static auto lengthSizeToProtocolBitmask(const bitFieldSizes& len) -> flags_type;
        [[nodiscard]] static constexpr auto broadcastToProtocolBitmask(const bool& en) -> flags_type;
        [[nodiscard]] static constexpr auto ackToProtocolBitmask(const bool& en) -> flags_type;
        [[nodiscard]] static auto checksumToProtocolBitmask(const checksumTypes& en) -> flags_type;

        static void calculateChecksum(ser_msg_t& msg, const checksumTypes& csType);
    };

    template<class... Args>
    void message::emplace_back(Args&&... args)
    {
        chunks.emplace_back(std::forward<Args...>(args)...);
    }
}

#endif
