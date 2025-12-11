#ifndef HARDWARE_INTERFACE_H_
#define HARDWARE_INTERFACE_H_

#include <climits>
#include <helpers/types.h>
#include <optional>
#include <string>
#include <vector>
#include <FpgaIO/fpgaIO.h>

namespace empower
{
    /**
     * @brief      This class describes a hardware interface.
     */
    class hardwareInterface : public FpgaIO
    {
    public:
        using reg_t = helpers::types::reg_t;
        using mmData_t = helpers::types::mmData_t;

    private:

        /* Not needed but used in readRange */
        reg_t addr;
        reg_t len;
        reg_t pageAddr;
        reg_t pageLen;

        /**
         * @brief      Hardware Object Structure
         */
        struct hwObject
        {
            std::string name;
            reg_t addr;
            reg_t len;
        };

        static constexpr mmData_t fullMask = static_cast<mmData_t>(-1);

        hardwareInterface() = delete;
        hardwareInterface(hardwareInterface&) = delete;
        hardwareInterface(hardwareInterface&&) = delete;
        void operator=(hardwareInterface&) = delete;
        void operator=(hardwareInterface&&) = delete;

    public:
        explicit hardwareInterface(reg_t addr, reg_t len);
        ~hardwareInterface();

        [[nodiscard]] std::optional<mmData_t> read(const reg_t offset, const mmData_t mask = fullMask) noexcept;
        [[nodiscard]] std::optional<std::vector<mmData_t>> readRange(const reg_t start, const reg_t stop) noexcept;
        [[nodiscard]] std::optional<std::vector<std::pair<reg_t, mmData_t>>> readRangeAddr(const reg_t start, const reg_t stop) noexcept;
        bool write(const reg_t offset, const mmData_t val, const mmData_t mask = fullMask)  noexcept;
        bool writeRange(const reg_t offset, const std::vector<mmData_t>& data, const mmData_t mask = fullMask)  noexcept;
        [[nodiscard]] static reg_t getPageSize() noexcept {
            return static_cast<reg_t>(::sysconf(_SC_PAGESIZE));
        }


    private:
        [[nodiscard]] bool validateAddress(reg_t offset)  noexcept;

    };
}

#endif
