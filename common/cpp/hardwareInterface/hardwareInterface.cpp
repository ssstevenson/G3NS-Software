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
    FpgaIO( static_cast <unsigned int > (base),  length) ,
    addr{base},
    len{length},
    pageAddr{0},
    pageLen{0}
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


}

/**
 * @brief      Destroys the object.
 */
hardwareInterface::~hardwareInterface()
{
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
   unsigned val = readReg( static_cast < std::uint32_t > (offset) );
   if (val == std::numeric_limits<unsigned int>::max())
   {
       return std::nullopt;
   } else
   {
       return val & mask;
   }
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

    std::vector<unsigned int > result;
    result.reserve(stop - start + 1);

    for ( reg_t regOffset = start; regOffset <= stop; regOffset++)
    {
        result.push_back(readReg( static_cast <std::uint32_t> (regOffset)) );
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
std::optional<std::vector<std::pair<hardwareInterface::reg_t, hardwareInterface::mmData_t> >>
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
bool hardwareInterface::write(const reg_t offset, const mmData_t val, const mmData_t mask)  noexcept
{
    if(!validateAddress(offset))
    {
        return false;
    }

   if ( mask == 0xFFFFFFFF)
    {
        writeReg(static_cast<std::uint32_t>(offset), val);
    }
    else
    {
        writeReg(static_cast <std::uint32_t> (offset), val, mask);
    }

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
bool hardwareInterface::writeRange(const reg_t offset, const std::vector<mmData_t>& data, const mmData_t mask)  noexcept
{

    const reg_t stop{offset + data.size() -1 };

    if(!validateAddress(offset) || !validateAddress(stop) || data.empty())
    {
        return false;
    }
    std::uint32_t writeAddr =  static_cast <std::uint32_t> (offset);
    if (mask == fullMask)
    {
        for (auto datum : data)
            writeReg(writeAddr++, datum );
    }
    else
    {
        for (auto datum : data)
            writeReg(writeAddr++, datum , mask);
    }

    return true;
}



/**
 * @brief      Checks if a given offset is valid
 *
 * @param[in]  offset  The offset
 *
 * @return     True if the given offset is valid for the setup, False otherwise.
 */
bool hardwareInterface::validateAddress(reg_t offset)  noexcept
{
    return (validateRegOffset( static_cast <std::uint32_t> (offset) ) );
}
