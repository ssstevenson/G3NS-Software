#ifndef SCPI_INSTRUMENT_H_
#define SCPI_INSTRUMENT_H_

#include <arpa/inet.h>
#include <optional>
#include <string>
#include <sys/socket.h>

namespace empower
{
    /**
     * @brief      This class describes a scpi instrument.
     */
    class scpiInstrument
    {
    private:
        std::int32_t sock;
        struct sockaddr_in server;
        bool isOpen;
        static constexpr std::size_t rxBufSize = 256;

    public:
        scpiInstrument();
        virtual ~scpiInstrument();
        bool reset();
        [[nodiscard]] std::optional<std::string> getId();
        bool open(const std::string& target, const std::uint16_t port = 5025);
        bool send(const std::string& m);
        [[nodiscard]] std::optional<std::string> recv();
        [[nodiscard]] std::optional<std::string> readVal(const std::string& msg);
        bool sendOnOff(const std::string& msg, bool on = true);
        [[nodiscard]] bool isConnOpen();

        static bool reset(const std::string& target);
        [[nodiscard]] static std::optional<std::string> getId(const std::string& target);
    };
}

#endif
