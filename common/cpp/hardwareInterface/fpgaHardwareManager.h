#ifndef FPGA_HARDWARE_MANAGER_H_
#define FPGA_HARDWARE_MANAGER_H_

#include <hardwareInterface/hardwareInterface.h>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <string>
#include <optional>

namespace empower
{
    // Forward declaration
    class configManagerConnection;

    /**
     * @brief Singleton manager for shared FPGA hardware interface instances.
     *        Ensures only one hardwareInterface exists per memory page across all processes.
     */
    class fpgaHardwareManager
    {
    public:
        using reg_t = hardwareInterface::reg_t;
        using mmData_t = hardwareInterface::mmData_t;
        using modAddrLut_t = std::unordered_map<std::string, reg_t>;
        using hardwareInterfaceMap_t = std::map<reg_t, std::unique_ptr<hardwareInterface>>;

        static constexpr reg_t defaultPageSize = 0x10000;
        static constexpr reg_t pageMask = ~(defaultPageSize - 1);

        // Singleton access
        static fpgaHardwareManager& getInstance();

        // Module management
        void registerModule(const std::string& moduleName, reg_t baseAddress);
        void registerModulesFromConfig(configManagerConnection& configIf, 
                                       const std::string& moduleListParam = "FHI_MODULE_LIST");
        std::optional<reg_t> getModuleBaseAddress(const std::string& moduleName);
        
        // Hardware interface access
        hardwareInterface* getHwIf(reg_t addr);
        
        // Coordinated locking across all access methods
        std::unique_lock<std::shared_timed_mutex> acquireExclusiveLock(std::chrono::microseconds timeout);
        std::shared_lock<std::shared_timed_mutex> acquireSharedLock(std::chrono::microseconds timeout);

        // Diagnostic methods
        size_t getActiveMappingCount() const;
        std::vector<reg_t> getActiveMappingAddresses() const;

        // Delete copy and move
        fpgaHardwareManager(const fpgaHardwareManager&) = delete;
        fpgaHardwareManager& operator=(const fpgaHardwareManager&) = delete;
        fpgaHardwareManager(fpgaHardwareManager&&) = delete;
        fpgaHardwareManager& operator=(fpgaHardwareManager&&) = delete;

    private:
        fpgaHardwareManager() : accessMutex{}, mapMutex{}, moduleMap{}, memoryMap{} {}
        ~fpgaHardwareManager() = default;

        mutable std::shared_timed_mutex accessMutex;  // Coordinates all FPGA access
        mutable std::mutex mapMutex;                   // Protects map modifications
        modAddrLut_t moduleMap;
        hardwareInterfaceMap_t memoryMap;
    };
}

#endif
