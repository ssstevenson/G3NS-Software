#include "hardwareInterface.h"

#include <algorithm>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <syslog.h>
#include <chrono>
#include <ctime>



using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param[in]  base    The base address
 * @param[in]  length  The length in memory
 */
hardwareInterface::hardwareInterface(reg_t base, reg_t length):
    fd{-1},
    addr{base},
    len{length},
    pageAddr{0},
    pageLen{0},
    mapping{nullptr}
{
#ifdef SIMULATE_HARDWARE
    mapping = reinterpret_cast<volatile mmData_t*>(::malloc(len * sizeof(mmData_t)));
    fd = 1;
#else
    fd = ::open("/dev/mem", O_RDWR);
    if(fd > 0)
    {
        const reg_t pageSize{getPageSize()};

        pageAddr = (addr & ~(pageSize - 1));

        reg_t nextPageAddr = addr + len;
        const reg_t remainderInPage = nextPageAddr % pageSize;
        if(0 != remainderInPage)
        {
            nextPageAddr += pageSize - remainderInPage;
        }
        pageLen = nextPageAddr - pageAddr;

        mapping = static_cast<volatile mmData_t*>(::mmap(nullptr, pageLen, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
            static_cast<__off_t>(pageAddr)));
        if(MAP_FAILED == mapping)
        {
            ::close(fd);
            mapping = nullptr;
        }
    }
#endif
}

/**
 * @brief      Destroys the object.
 */
hardwareInterface::~hardwareInterface()
{
    if(nullptr != mapping)
    {
#ifdef SIMULATE_HARDWARE
        ::free(const_cast<mmData_t*>(mapping));
#else
        munmap(reinterpret_cast<void*>(pageAddr), pageLen);
    }
    if(fd > 0)
    {
        ::close(fd);
#endif
    }
}

/**
 * @brief      Read a given offset into the FPGA Memory Map
 *
 * @param[in]  offset  The offset
 * @param[in]  mask    The data mask
 *
 * @return     The read data if sucessful, std::nullopt otherwise.
 */
std::optional<hardwareInterface::mmData_t> hardwareInterface::read(const reg_t offset,
    const mmData_t mask) noexcept
{
    if(!validateAddress(offset))
    {
        return std::nullopt;
    }

    const reg_t readAddr{((offset + (addr - pageAddr)) % pageLen) >> 2};

    return mapping[readAddr] & mask;
}

/**
 * @brief      Reads a range.
 *
 * @param[in]  start  The start address
 * @param[in]  stop   The stop address
 *
 * @return     The read data if sucessful, std::nullopt otherwise.
 */
std::optional<std::vector<hardwareInterface::mmData_t>> hardwareInterface::readRange(const reg_t start,
    const reg_t stop) noexcept
{
    if(!validateAddress(start) || !validateAddress(stop) || (start > stop))
    {
        return std::nullopt;
    }

    const reg_t startIdx = (((start + (addr - pageAddr)) % pageLen) >> 2);
    const reg_t stopIdx = (((stop + (addr - pageAddr)) % pageLen) >> 2);

    std::vector<mmData_t> result;
    try
    {
        std::copy(const_cast<mmData_t*>(&mapping[startIdx]), const_cast<mmData_t*>(&mapping[stopIdx]),
            std::back_inserter(result));
    }
    catch(...)
    {
        return std::nullopt;
    }
    return result;
}

/**
 * @brief      Reads a range.
 *
 * @param[in]  start  The start address
 * @param[in]  stop   The stop address
 *
 * @return     The read data if sucessful, std::nullopt otherwise.
 */
std::optional<std::vector<std::pair<hardwareInterface::reg_t, hardwareInterface::mmData_t>>>
    hardwareInterface::readRangeAddr(const reg_t start, const reg_t stop) noexcept
{
    if(const std::optional<std::vector<mmData_t>> rangeData{readRange(start, stop)}; rangeData.has_value())
    {
        std::vector<std::pair<reg_t, mmData_t>> result;
        constexpr reg_t wordStepSize = 4;

        reg_t wordOffset{start & ~(wordStepSize - 1)};

        std::transform(std::begin(rangeData.value()), std::end(rangeData.value()), std::back_inserter(result),
            [&](const auto& val){
                const reg_t absAddr{wordOffset + pageAddr};
                auto zipOut{std::make_pair(absAddr, val)};
                wordOffset += wordStepSize;
                return zipOut;
            }
        );

        return result;
    }

    return std::nullopt;
}

/**
 * @brief      Write data to a given offset in the FPGA Memory Map
 *
 * @param[in]  offset  The offset address
 * @param[in]  val     The value to be written
 * @param[in]  mask    The mask for the data
 *
 * @return     True if the write was sucessful, False otherwise.
 */
bool hardwareInterface::write(const reg_t offset, const mmData_t val, const mmData_t mask) const noexcept
{
    if(!validateAddress(offset))
    {
        return false;
    }

    const reg_t writeAddr = (((offset + (addr - pageAddr)) % pageLen) >> 2);

    try
    {
        if(mask == fullMask)
        {
            mapping[writeAddr] = val;
        }
        else
        {
            mapping[writeAddr] = ((mapping[writeAddr] & ~mask) | (val & mask));
        }
    }
    catch(...)
    {
        return false;
    }
    //auto now = std::chrono::system_clock::now();
   // auto time_t_now = std::chrono::system_clock::to_time_t(now);
    //syslog(LOG_INFO, "FPGA Write - Address: 0x%lx, Time: %s, Value: %u, Mask: %u", offset, std::ctime(&time_t_now), val, mask);
    return true;
}

/**
 * @brief      Writes a range.
 *
 * @param[in]  offset  The offset address
 * @param[in]  data    The data
 * @param[in]  mask    The data mask
 *
 * @return     True if the write was sucessful, False otherwise.
 */
bool hardwareInterface::writeRange(const reg_t offset, const std::vector<mmData_t>& data, const mmData_t mask) const noexcept
{
    reg_t writeAddr{((offset + (addr - pageAddr)) % pageLen) >> 2};
    const reg_t stop{writeAddr + data.size()};

    if(!validateAddress(offset) || !validateAddress(stop) || data.empty())
    {
        return false;
    }

    try
    {
        if(mask == fullMask)
        {
            std::for_each(data.begin(), data.end(), [&](const auto& datum){
                mapping[writeAddr++] = datum;
            });
        }
        else
        {
            std::for_each(data.begin(), data.end(), [&](const auto& datum){
                mapping[writeAddr] = ((mapping[writeAddr] & ~mask) | (datum & mask));
                ++writeAddr;
            });
        }
    }
    catch(...)
    {
        return false;
    }

    return true;
}

/**
 * @brief      Checks if the Setup is valid
 *
 * @return     True if there is a valid File Descriptor, False otherwise.
 */
bool hardwareInterface::validSetup() const noexcept
{
    return (fd > 0);
}

/**
 * @brief      Gets the page size.
 *
 * @return     The page size.
 */
[[nodiscard]]
hardwareInterface::reg_t hardwareInterface::getPageSize() noexcept
{
    return static_cast<reg_t>(::sysconf(_SC_PAGESIZE));
}

/**
 * @brief      Checks if a given offset is valid
 *
 * @param[in]  offset  The offset
 *
 * @return     True if the given offset is valid for the setup, False otherwise.
 */
bool hardwareInterface::validateAddress(reg_t offset) const noexcept
{
    return ((offset <= len) && (nullptr != mapping) && validSetup());
}
