#ifndef FPGA_DIRECT_CONNECTION_H_
#define FPGA_DIRECT_CONNECTION_H_

//.. #include <hardwareInterface/hardwareInterface.h>
#include <hardwareInterface/fpgaHardwareManager.h>
#include <configManagerConnection/configManagerConnection.h>
#include <optional>
#include <chrono>
#include <vector>

namespace empower
{
    /**
     * @brief Provides direct, lightweight access to FPGA hardware without ZMQ/JSON overhead.
     *        Uses shared fpgaHardwareManager to prevent duplicate memory mappings.
     *        
     *        This is the recommended replacement for direct hardwareInterface usage.
     */
    class fpgaDirectConnection
    {
    public:
        using reg_t = fpgaHardwareManager::reg_t;
        using mmData_t = fpgaHardwareManager::mmData_t;


        enum fpgaStatus {
            FPGA_STATUS_OK = 0,
            FPGA_STATUS_BUSY,
            FPGA_STATUS_ERROR
        };

        fpgaDirectConnection() = default;
        ~fpgaDirectConnection() = default;

        // Initialize from configuration
        void initializeFromConfig(configManagerConnection& configIf, 
                                   const std::string& moduleListParam = "FHI_MODULE_LIST");

        // Direct address-based access
        std::optional<mmData_t> hwFpgaRead(reg_t address, fpgaStatus& status);
        fpgaStatus hwFpgaWrite(reg_t address, mmData_t writeVal) noexcept;
        
        // Batch operations
        std::vector<std::optional<mmData_t>> hwFpgaReadBatch(const std::vector<reg_t>& addresses, 
                                                              fpgaStatus& status);
        fpgaStatus hwFpgaWriteBatch(const std::vector<std::pair<reg_t, mmData_t>>& writeOps) noexcept;
        
        // Module-based access
        std::optional<mmData_t> hwFpgaReadModule(const std::string& moduleName, 
                                                  reg_t offset, fpgaStatus& status);
        fpgaStatus hwFpgaWriteModule(const std::string& moduleName, 
                                     reg_t offset, mmData_t writeVal) noexcept;

        // Range operations
        std::optional<std::vector<mmData_t>> hwFpgaReadRange(reg_t startAddr, reg_t endAddr, 
                                                              fpgaStatus& status);
        fpgaStatus hwFpgaWriteRange(reg_t startAddr, const std::vector<mmData_t>& data) noexcept;

    private:
        fpgaHardwareManager& hwMgr = fpgaHardwareManager::getInstance();
        static constexpr reg_t pageMask = fpgaHardwareManager::pageMask;
    };
}

#endif
