#ifndef SERIAL_INTERFACE_H_
#define SERIAL_INTERFACE_H_

#include <chrono>
#include <hardwareInterface/hardwareInterface.h>
#include <helpers/types.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace empower
{
    /**
     * @brief      This class describes a serial interface.
     */
    class serialInterface final
    {
    public:
        using hw_reg_t = hardwareInterface::reg_t;
        using mmData_t = hardwareInterface::mmData_t;
        using ser_data_t = helpers::types::ser_ext_data_t;
        using timeout_t = helpers::types::timeout_t;

        enum class parityStates { NONE, EVEN, ODD };
        enum class stopStates { ONE, ONE_HALF, TWO };
        enum class duplex { FULL, HALF };

        /**
         * @brief      Framing Error Exception
         */
        struct framingError: public std::exception
        {
            const char* what() const throw()
            {
                return "Framing Error";
            }
        };

        /**
         * @brief      Parity Error Exception
         */
        struct parityError: public std::exception
        {
            const char* what() const throw()
            {
                return "Parity Error";
            }
        };

    private:
        enum class registerMap: hw_reg_t
        {
            CTRL = 0x00,
            STATUS = 0x04,
            BASE_FREQ = 0x08,
            BAUD_SET = 0x0C,
            READ = 0x10,
            WRITE = 0x14,
            START = 0x18,
            CLEAR_TX = 0x1C,
            CLEAR_RX = 0x20
        };

        const std::unordered_map<parityStates, mmData_t> parityStateToReg =
        {
            {parityStates::NONE, 0x00000000},
            {parityStates::EVEN, 0x00000C00},
            {parityStates::ODD,  0x00000800}
        };

        const std::unordered_map<mmData_t, parityStates> regToParityState =
        {
            {0x00000000, parityStates::NONE},
            {0x00000C00, parityStates::EVEN},
            {0x00000800, parityStates::ODD}
        };

        const std::unordered_map<stopStates, mmData_t> stopStatesToReg =
        {
            {stopStates::ONE, 0x00000000},
            {stopStates::ONE_HALF, 0x00000100},
            {stopStates::TWO,  0x00000200}
        };

        const std::unordered_map<mmData_t, stopStates> regToStopStates =
        {
            {0x00000000, stopStates::ONE},
            {0x00000100, stopStates::ONE_HALF},
            {0x00000200, stopStates::TWO}
        };

        struct masks
        {
            static constexpr mmData_t parity = 0x00000C00;
            static constexpr mmData_t stop = 0x00000300;
            static constexpr mmData_t duplex = 0x00001000;
            static constexpr mmData_t setBits = 0xFF;
            static constexpr mmData_t readData = 0xFFFF;
            static constexpr mmData_t readDataDone = 0x80000000;
            static constexpr mmData_t readDataLiveFramingError = 0x2000;
            static constexpr mmData_t readDataLiveParityError = 0x1000;
            static constexpr mmData_t readDataLiveError = readDataLiveFramingError | readDataLiveParityError;
            static constexpr mmData_t txRunning = 0x08;
            static constexpr mmData_t txFull = 0x04;
            static constexpr mmData_t rxEmpty = 0x02;
            static constexpr mmData_t rxFull = 0x01;
        };

        mutable hardwareInterface hwIf;

        static constexpr auto readTimeoutDefault = timeout_t{1};
        static constexpr auto readTimeoutStartDefault = timeout_t{3000};
        timeout_t readTimeout;
        timeout_t readTimeoutStart;
        timeout_t readTimeoutWorking;

    public:
        serialInterface(const hw_reg_t& baseAddr);

        [[nodiscard]] std::optional<std::vector<ser_data_t>> read(const timeout_t& startTimeout,
            const timeout_t& charTimeout);
        [[nodiscard]] std::optional<std::vector<ser_data_t>> read(const std::vector<ser_data_t>& data,
            const timeout_t& startTimeout, const timeout_t& charTimeout);
        bool write(const std::vector<ser_data_t>& data);

        [[nodiscard]] std::optional<ser_data_t> readByte() const noexcept(false);
        bool writeByte(const ser_data_t& datum) const noexcept;

        bool setBaud(const std::uint32_t baud) const noexcept;
        [[nodiscard]] std::optional<std::uint32_t> getBaud() const noexcept;
        bool setBits(const mmData_t bits) const noexcept;
        [[nodiscard]] std::optional<mmData_t> getBits() const noexcept;
        bool setParity(const parityStates parity) const noexcept;
        [[nodiscard]] std::optional<parityStates> getParity() const noexcept;
        bool setStop(const stopStates stop) const noexcept;
        [[nodiscard]] std::optional<stopStates> getStop() const noexcept;
        bool setDuplex(const duplex& dup) const noexcept;
        [[nodiscard]] std::optional<duplex> getDuplex() const noexcept;

        bool clearReadBuffer() const noexcept;
        bool clearWriteBuffer() const noexcept;
        bool startTransmission() const noexcept;
        [[nodiscard]] bool isRxBufferEmpty() const noexcept;
        [[nodiscard]] bool isRxBufferFull() const noexcept;
        [[nodiscard]] bool isTxBufferFull() const noexcept;
        [[nodiscard]] bool isTxRunning() const noexcept;

        [[nodiscard]] parityStates strToParity(const std::string& str) const noexcept;
        [[nodiscard]] stopStates strToStop(const std::string& str) const noexcept;

    private:
        [[nodiscard]] std::optional<std::uint32_t> getClockRate() const noexcept;
        [[nodiscard]] bool checkReadTimeout() const;
        [[nodiscard]] bool checkReadStartTimeout() const;
        void updateReadTimeout() noexcept;
    };
}

#endif
