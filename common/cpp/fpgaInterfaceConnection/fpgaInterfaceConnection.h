#ifndef FPGA_INTERFACE_CONNECTION_H_
#define FPGA_INTERFACE_CONNECTION_H_

#include <helpers/types.h>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <optional>
#include <string>
#include <vector>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      This class describes a fpga interface connection.
     */
    class fpgaInterfaceConnection
    {
    public:
        using reg_t = helpers::types::reg_t;
        using mmData_t = helpers::types::mmData_t;

    private:
        zmq::context_t& ctx;
        std::unique_ptr<zmq::socket_t> reqRepSock;
        messageFactoryConnection msgIf;

        std::string fpgaRequestModuleWriteJsonStr;
        std::string fpgaRequestModuleReadJsonStr;
        std::string fpgaRequestModuleReadBatchJsonStr;
        std::string fpgaRequestModuleReadRangeJsonStr;

    public:
        fpgaInterfaceConnection(zmq::context_t& zmqCtx);

        bool write(const std::string& module_name, reg_t reg_offset, const mmData_t& reg_data) noexcept;
        [[nodiscard]] std::optional<mmData_t> read(const std::string& module_name, reg_t reg_offset) noexcept;
        [[nodiscard]] std::optional<std::vector<mmData_t>> read(const std::string& module,
            const std::vector<reg_t>& addr) noexcept;
        [[nodiscard]] std::optional<std::vector<mmData_t>> read(const std::string& module, const reg_t& start,
            const reg_t& stop) noexcept;

    private:
        void setupSocket();
    };
}

#endif
