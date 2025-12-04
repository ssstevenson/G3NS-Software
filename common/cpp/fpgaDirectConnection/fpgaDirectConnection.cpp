#include "fpgaDirectConnection.h"
#include <logger/logger.h>
#include <syslog.h>
#include <chrono>
#include <ctime>

using namespace empower;

void fpgaDirectConnection::initializeFromConfig(configManagerConnection& configIf, 
                                                 const std::string& moduleListParam)
{
    hwMgr.registerModulesFromConfig(configIf, moduleListParam);
}

std::optional<fpgaDirectConnection::mmData_t> fpgaDirectConnection::hwFpgaRead(
    reg_t address, fpgaStatus& status)
{
    auto lock = hwMgr.acquireSharedLock(std::chrono::microseconds(3000));
    if (!lock.owns_lock())
    {
        logger::warn(__FILE__, __FUNCTION__, "FPGA busy (read timeout)");
        status = FPGA_STATUS_BUSY;
        return std::nullopt;
    }

    reg_t pageAddr = address & pageMask;
    reg_t offset = address & ~pageMask;
    
    auto data = hwMgr.getHwIf(pageAddr)->read(offset);
    status = data.has_value() ? FPGA_STATUS_OK : FPGA_STATUS_ERROR;
    return data;
}

fpgaDirectConnection::fpgaStatus fpgaDirectConnection::hwFpgaWrite(
    const reg_t address, const mmData_t writeVal) noexcept
{
    auto lock = hwMgr.acquireExclusiveLock(std::chrono::microseconds(3000));
    if (!lock.owns_lock())
    {
        logger::warn(__FILE__, __FUNCTION__, "FPGA busy (write timeout)");
        return FPGA_STATUS_BUSY;
    }

    const reg_t pageAddr = address & pageMask;
    const reg_t offset = address & ~pageMask;
    auto hwIfPtr = hwMgr.getHwIf(pageAddr);

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    syslog(LOG_INFO, "FPGA Direct Write - Addr: 0x%lx, Time: %s, Val: 0x%x", 
           address, std::ctime(&time_t_now), writeVal);

    if (hwIfPtr->write(offset, writeVal))
    {
        if (auto readData = hwIfPtr->read(offset); readData == writeVal)
        {
            return FPGA_STATUS_OK;
        }
    }

    return FPGA_STATUS_ERROR;
}

std::vector<std::optional<fpgaDirectConnection::mmData_t>> fpgaDirectConnection::hwFpgaReadBatch(
    const std::vector<reg_t>& addresses, fpgaStatus& status)
{
    auto lock = hwMgr.acquireSharedLock(std::chrono::microseconds(5000));
    std::vector<std::optional<mmData_t>> results;
    results.reserve(addresses.size());
    
    if (!lock.owns_lock())
    {
        logger::warn(__FILE__, __FUNCTION__, "FPGA busy (batch read timeout)");
        status = FPGA_STATUS_BUSY;
        return results;
    }

    bool allSuccess = true;
    for (const auto& address : addresses)
    {
        reg_t pageAddr = address & pageMask;
        reg_t offset = address & ~pageMask;
        auto data = hwMgr.getHwIf(pageAddr)->read(offset);
        allSuccess &= data.has_value();
        results.push_back(data);
    }

    status = allSuccess ? FPGA_STATUS_OK : FPGA_STATUS_ERROR;
    return results;
}

fpgaDirectConnection::fpgaStatus fpgaDirectConnection::hwFpgaWriteBatch(
    const std::vector<std::pair<reg_t, mmData_t>>& writeOps) noexcept
{
    auto lock = hwMgr.acquireExclusiveLock(std::chrono::microseconds(10000));
    if (!lock.owns_lock())
    {
        logger::warn(__FILE__, __FUNCTION__, "FPGA busy (batch write timeout)");
        return FPGA_STATUS_BUSY;
    }

    bool allSuccess = true;
    for (const auto& [address, value] : writeOps)
    {
        reg_t pageAddr = address & pageMask;
        reg_t offset = address & ~pageMask;
        allSuccess &= hwMgr.getHwIf(pageAddr)->write(offset, value);
    }

    return allSuccess ? FPGA_STATUS_OK : FPGA_STATUS_ERROR;
}

std::optional<fpgaDirectConnection::mmData_t> fpgaDirectConnection::hwFpgaReadModule(
    const std::string& moduleName, reg_t offset, fpgaStatus& status)
{
    auto baseAddr = hwMgr.getModuleBaseAddress(moduleName);
    if (!baseAddr.has_value())
    {
        logger::warn(__FILE__, __FUNCTION__, "Module not found: " + moduleName);
        status = FPGA_STATUS_ERROR;
        return std::nullopt;
    }
    
    return hwFpgaRead(baseAddr.value() + offset, status);
}

fpgaDirectConnection::fpgaStatus fpgaDirectConnection::hwFpgaWriteModule(
    const std::string& moduleName, reg_t offset, mmData_t writeVal) noexcept
{
    auto baseAddr = hwMgr.getModuleBaseAddress(moduleName);
    if (!baseAddr.has_value())
    {
        logger::warn(__FILE__, __FUNCTION__, "Module not found: " + moduleName);
        return FPGA_STATUS_ERROR;
    }
    
    return hwFpgaWrite(baseAddr.value() + offset, writeVal);
}

std::optional<std::vector<fpgaDirectConnection::mmData_t>> fpgaDirectConnection::hwFpgaReadRange(
    reg_t startAddr, reg_t endAddr, fpgaStatus& status)
{
    auto lock = hwMgr.acquireSharedLock(std::chrono::microseconds(5000));
    if (!lock.owns_lock())
    {
        logger::warn(__FILE__, __FUNCTION__, "FPGA busy (range read timeout)");
        status = FPGA_STATUS_BUSY;
        return std::nullopt;
    }

    reg_t pageAddr = startAddr & pageMask;
    reg_t startOffset = startAddr & ~pageMask;
    reg_t endOffset = endAddr & ~pageMask;
    
    auto data = hwMgr.getHwIf(pageAddr)->readRange(startOffset, endOffset);
    status = data.has_value() ? FPGA_STATUS_OK : FPGA_STATUS_ERROR;
    return data;
}

fpgaDirectConnection::fpgaStatus fpgaDirectConnection::hwFpgaWriteRange(
    reg_t startAddr, const std::vector<mmData_t>& data) noexcept
{
    auto lock = hwMgr.acquireExclusiveLock(std::chrono::microseconds(10000));
    if (!lock.owns_lock())
    {
        logger::warn(__FILE__, __FUNCTION__, "FPGA busy (range write timeout)");
        return FPGA_STATUS_BUSY;
    }

    reg_t pageAddr = startAddr & pageMask;
    reg_t offset = startAddr & ~pageMask;
    
    bool success = hwMgr.getHwIf(pageAddr)->writeRange(offset, data);
    return success ? FPGA_STATUS_OK : FPGA_STATUS_ERROR;
}
