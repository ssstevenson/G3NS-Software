#include "fpgaHardwareManager.h"
#include <logger/logger.h>
#include <helpers/types.h>
#include <configManagerConnection/configManagerConnection.h>

using namespace empower;

fpgaHardwareManager& fpgaHardwareManager::getInstance()
{
    static fpgaHardwareManager instance;
    return instance;
}

void fpgaHardwareManager::registerModule(const std::string& moduleName, reg_t baseAddress)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    moduleMap[moduleName] = baseAddress;
    logger::info(__FILE__, __FUNCTION__, "Registered FPGA module: " + moduleName + 
                 " at base address: 0x" + helpers::types::toHexString(baseAddress));
}

void fpgaHardwareManager::registerModulesFromConfig(configManagerConnection& configIf, 
                                                     const std::string& moduleListParam)
{
    using std::string_literals::operator""s;
    try
    {
        auto moduleList = helpers::types::dataTokenizer<std::string>(
            configIf.getParam<std::string>(moduleListParam).value_or(""));

        for(auto& moduleCfgKey : moduleList)
        {
            auto moduleName = configIf.getParam<std::string>(moduleCfgKey).value_or("");
            if(auto baseAddr = helpers::types::strToNum<reg_t>(
                configIf.getParam<std::string>(moduleCfgKey + "_BASE_HEX_ADDRESS"s).value_or(""), 16);
                baseAddr.has_value() && !moduleName.empty())
            {
                registerModule(moduleName, baseAddr.value());
            }
        }
        logger::info(__FILE__, __FUNCTION__, "Module registration from config complete");
    }
    catch(const std::exception& e)
    {
        logger::critical(__FILE__, __FUNCTION__, "Failed to register modules from config: "s + e.what());
        throw;
    }
}

std::optional<fpgaHardwareManager::reg_t> fpgaHardwareManager::getModuleBaseAddress(
    const std::string& moduleName)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    auto it = moduleMap.find(moduleName);
    if (it != moduleMap.end())
    {
        return it->second;
    }
    logger::warn(__FILE__, __FUNCTION__, "Module not found: " + moduleName);
    return std::nullopt;
}

hardwareInterface* fpgaHardwareManager::getHwIf(reg_t addr)
{
    const auto baseAddr = addr & pageMask;
    
    std::lock_guard<std::mutex> lock(mapMutex);
    
    // C++17 performant method - fixes the bug in original code
    auto it = memoryMap.lower_bound(baseAddr);
    if (it == memoryMap.end() || it->first != baseAddr)
    {
        // Friend access allows us to create hardwareInterface
        it = memoryMap.emplace_hint(it, baseAddr,
            std::make_unique<hardwareInterface>(baseAddr, defaultPageSize));
        logger::debug(__FILE__, __FUNCTION__, "Created hardware interface for page at: 0x" + 
                      helpers::types::toHexString(baseAddr));
    }
    return it->second.get();
}

std::unique_lock<std::shared_timed_mutex> fpgaHardwareManager::acquireExclusiveLock(
    std::chrono::microseconds timeout)
{
    std::unique_lock<std::shared_timed_mutex> lock(accessMutex, std::defer_lock);
    if (lock.try_lock_for(timeout))
    {
        return lock;
    }
    logger::warn(__FILE__, __FUNCTION__, "Failed to acquire exclusive FPGA lock within timeout");
    return std::unique_lock<std::shared_timed_mutex>();
}

std::shared_lock<std::shared_timed_mutex> fpgaHardwareManager::acquireSharedLock(
    std::chrono::microseconds timeout)
{
    std::shared_lock<std::shared_timed_mutex> lock(accessMutex, std::defer_lock);
    if (lock.try_lock_for(timeout))
    {
        return lock;
    }
    logger::warn(__FILE__, __FUNCTION__, "Failed to acquire shared FPGA lock within timeout");
    return std::shared_lock<std::shared_timed_mutex>();
}

size_t fpgaHardwareManager::getActiveMappingCount() const
{
    std::lock_guard<std::mutex> lock(mapMutex);
    return memoryMap.size();
}

std::vector<fpgaHardwareManager::reg_t> fpgaHardwareManager::getActiveMappingAddresses() const
{
    std::lock_guard<std::mutex> lock(mapMutex);
    std::vector<reg_t> addresses;
    addresses.reserve(memoryMap.size());
    for (const auto& [addr, _] : memoryMap)
    {
        addresses.push_back(addr);
    }
    return addresses;
}
