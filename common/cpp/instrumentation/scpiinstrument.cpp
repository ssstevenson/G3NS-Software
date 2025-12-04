#include "scpiinstrument.h"

#include <array>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 */
scpiInstrument::scpiInstrument():
    sock{0},
    server{},
    isOpen{false}
{
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
}

/**
 * @brief      Destroys the object.
 */
scpiInstrument::~scpiInstrument()
{
    if(sock > 0)
    {
        ::close(sock);
        sock = 0;
    }
}

/**
 * @brief      Resets the object.
 *
 * @return     True if the reset was successful, False otherwise.
 */
bool scpiInstrument::reset()
{
    return send("*RST");
}

/**
 * @brief      Gets the device ID string
 *
 * @return     The identifier string if sucessful, std::nullopt otherwise.
 */
std::optional<std::string> scpiInstrument::getId()
{
    return readVal("*IDN?");
}

/**
 * @brief      Opens a SCPI connection to a device
 *
 * @param[in]  target  Target IP Address
 * @param[in]  port    Target Port
 *
 * @return     True if the connection was successfully opened, False otherwise.
 */
bool scpiInstrument::open(const std::string& target, const std::uint16_t port)
{
    server.sin_addr.s_addr = inet_addr(target.c_str());
    server.sin_family = AF_INET;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
    server.sin_port = htons(port);
#pragma GCC diagnostic pop

    //TCP connect
    isOpen = (::connect(sock, reinterpret_cast<struct sockaddr *>(&server) , sizeof(server)) >= 0);

    return isOpen;
}

/**
 * @brief      Sends command to the device
 *
 * @param[in]  m     Command to be sent
 *
 * @return     True if the data was sent successfully, False otherwise
 */
bool scpiInstrument::send(const std::string& m)
{
    using std::string_literals::operator""s;
    bool res = false;

    const std::string msgOut{m + "\n"s};
    res = (::write(sock, msgOut.c_str(), msgOut.size()) >= 0);

    sched_yield();

    return res;
}

/**
 * @brief      Read data from a device
 *
 * @return     Data read from the device if sucessful, std::nullopt otherwise.
 */
std::optional<std::string> scpiInstrument::recv()
{
    std::array<char, rxBufSize> readBuf{0};

    if(!isOpen)
    {
        return std::nullopt;
    }

    if(auto readCnt{::read(sock, readBuf.data(), readBuf.size())}; readCnt > 0)
    {
        return std::string(readBuf.data(), static_cast<std::string::size_type>(readCnt));
    }

    return std::nullopt;
}

/**
 * @brief      Reads a value.
 *
 * @param[in]  msg   The Command Message
 *
 * @return     Received data from the command if sucessful, std::nullopt otherwise.
 */
std::optional<std::string> scpiInstrument::readVal(const std::string& msg)
{
    if(send(msg))
    {
        return recv();
    }

    return std::nullopt;
}

/**
 * @brief      Sends on off for a given parameter.
 *
 * @param[in]  msg   The Command
 * @param[in]  on    Whether this should be set to ON or OFF
 *
 * @return     True if this command write was successful, False otherwise
 */
bool scpiInstrument::sendOnOff(const std::string& msg, bool on)
{
    using std::string_literals::operator""s;
    return send(msg + (on?" ON"s:" OFF"s));
}

/**
 * @brief      Determines if connection open.
 *
 * @return     True if connection open, False otherwise.
 */
bool scpiInstrument::isConnOpen()
{
    return isOpen;
}

/**
 * @brief      Resets the given target.
 *
 * @param[in]  target  The target
 *
 * @return     True if it was reset successfully, False otherwise.
 */
bool scpiInstrument::reset(const std::string& target)
{
    if(scpiInstrument inst; inst.open(target))
    {
        return inst.reset();
    }

    return false;
}

/**
 * @brief      Gets the identifier.
 *
 * @param[in]  target  The target
 *
 * @return     The identifier string if sucessful, std::nullopt otherwise.
 */
std::optional<std::string> scpiInstrument::getId(const std::string& target)
{
    if(scpiInstrument inst; inst.open(target))
    {
        return inst.getId();
    }

    return std::nullopt;
}
