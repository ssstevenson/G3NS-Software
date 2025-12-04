#ifndef KISS_PROTOCOL_H_
#define KISS_PROTOCOL_H_

#include <controller/kissMsg.h>
#include <controller/kissUpdateData.h>
#include <controller/protocolInterface.h>
#include <optional>

namespace empower
{
    /**
     * @brief      This class describes a kiss protocol.
     */
    class kissProto:
        public protocolInterface
    {
    public:
        using pmod_addr_t = protocolInterface::pmod_addr_t;
        using data_set_t = protocolInterface::data_set_t;
        using ser_msg_t = protocolInterface::ser_msg_t;
        using pmod_data_t = protocolInterface::pmod_data_t;
        using timeout_t = protocolInterface::timeout_t;
        using dev_addr_t = kiss::message::dev_addr_t;
        using msg_type_t = kiss::message::msg_type_t;
        using protocolVersion = kiss::message::protocolVersion;
        using checksumTypes = kiss::message::checksumTypes;
        using chunk_id_t = kiss::chunk::cmd_id_t;

    private:
        kiss::updateData updateData;
        std::size_t seqNumCnt;

    public:
        kissProto(statusUpdateInterface& upIf, serialInterfaceConnection& ser,
            const std::string& key, const pmod_addr_t addr, const timeout_t& time);

        void poll();
        void reset();
        void faultClear();

    private:
        [[nodiscard]] auto getNextSeqNum() -> std::size_t;
        [[nodiscard]] auto buildMessage(const dev_addr_t& to, const msg_type_t& type) -> kiss::message;
    };
}

#endif
