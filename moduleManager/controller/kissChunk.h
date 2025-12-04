#ifndef KISS_CHUNK_H_
#define KISS_CHUNK_H_

#include <helpers/types.h>
#include <optional>
#include <serialInterfaceConnection/serialInterfaceConnection.h>

namespace empower::kiss
{
    /**
     * @brief      This class describes a KISS Protocol Chunk.
     */
    class chunk
    {
    public:
        using ser_msg_t = serialInterfaceConnection::ser_msg_t;
        using ser_data_t = helpers::types::ser_data_t;
        using base_vec_t = std::vector<ser_data_t>;
        using ser_msg_back_in_it = std::back_insert_iterator<ser_msg_t>;
        using data_t = std::optional<std::variant<bool, std::int8_t, std::uint8_t, std::int16_t, std::uint16_t,
            std::int32_t, std::uint32_t, std::int64_t, std::uint64_t, float, double, long double, base_vec_t>>;

        enum class data_type_t: std::uint8_t
        {
            t_void = 0, t_bit = 1, t_int8 = 2, t_uint8 = 3, t_int16 = 4, t_uint16 = 5, t_int32 = 6, t_uint32 = 7,
            t_int64 = 8, t_uint64 = 9, t_float = 10, t_double = 11, t_ldouble = 12, t_ack = 127, t_byte_array = 128
        };

        enum class protocolVersion: std::uint8_t
        {
            default_ver = 0
        };

        enum class cmd_id_len_t: ser_msg_t::size_type
        {
            bits_8 = 1, bits_16 = 2, bits_24 = 3, bits_32 = 4
        };

        enum class cmd_id_t: std::uint32_t
        {
            dummy = 0, dsa_rf_a = 1, dsa_rf_b = 2, current_rf_a = 3, current_rf_b = 4, factory_lock = 5,
            factory_unlock = 6, reset = 7, get_checksum = 8, enter_bootloader = 9, erase_prog_flash = 10,
            intel_hex_record = 11, get_num_bytes = 12, store_app_checksum = 13, store_sys_config = 14,
            fault_clear = 15, power_on_enable = 16, power_ready_enable = 17, power_ready_threshold = 18,
            amp_device_type = 19,

            ack = 127
        };

    private:
        protocolVersion protoVer;
        cmd_id_t cmdId;
        data_t data;

    public:
        chunk();
        chunk(ser_msg_t::const_iterator& it);
        chunk(const cmd_id_t& id);
        chunk(const cmd_id_t& id, const data_t& d);
        chunk(const cmd_id_t& id, const data_t& d, const protocolVersion& ver);

        operator ser_msg_t::size_type();

        void consume(ser_msg_t::const_iterator& it);
        [[nodiscard]] auto serialize() -> ser_msg_t;
        void serialize(ser_msg_t& msg);
        void serialize(ser_msg_back_in_it&& it);
        void serialize(ser_msg_back_in_it& it);

        [[nodiscard]] auto size() const -> ser_msg_t::size_type;

        [[nodiscard]] auto getProtocolVersion() const -> protocolVersion;
        void setProtocolVersion(const protocolVersion& ver);
        [[nodiscard]] auto getCommandId() const -> cmd_id_t;
        void setCommandId(const cmd_id_t& id);
        [[nodiscard]] auto getData() const -> data_t;
        void setData(const data_t& d = std::nullopt);

    private:
        [[nodiscard]] static auto dataTypeToBytes(const data_type_t& dt) -> std::size_t;
        [[nodiscard]] static auto cmdIdLenToBytes(const cmd_id_len_t& id) -> std::size_t;
        [[nodiscard]] static auto getChunkLen(const ser_msg_t::const_iterator& it) -> ser_msg_t::size_type;
        [[nodiscard]] static auto cmdIdLenToStatusBitField(const cmd_id_len_t& cmdIdLen) -> ser_data_t;
        [[nodiscard]] static auto statusToCmdIdLen(const ser_data_t& ch) -> cmd_id_len_t;
        [[nodiscard]] static auto numBytesToCmdIdLen(const std::size_t& bytes) -> cmd_id_len_t;
        [[nodiscard]] static auto statusToProtoVer(const ser_data_t& ch) -> protocolVersion;
        [[nodiscard]] static auto protoVerToStatusBitField(const protocolVersion& ver) -> ser_data_t;
        [[nodiscard]] static auto dataToDataType(const data_t& d) -> data_type_t;
    };
}

#endif
