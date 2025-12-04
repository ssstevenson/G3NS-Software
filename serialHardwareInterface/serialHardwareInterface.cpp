#include <algorithm>
#include <heartbeatResponse/heartbeatResponse.h>
#include <helpers/mainHelper.h>
#include <helpers/types.h>
#include <logger/logger.h>
#include <parser/serialHardwareInterfaceParser.h>
#include <string>

/**
 * @brief      Serial Hardware Interface Main Function
 *
 * @param[in]  argc  The count of arguments
 * @param      argv  The arguments array
 *
 * @return     Should not return, if it does then it is an Error
 */
int main(int argc, char* argv[])
{
    using std::string_literals::operator""s;

    zmq::context_t zmqCtx{1};
    empower::configManagerConnection configIf{zmqCtx};

    if(argc < 2)
    {
        return EXIT_FAILURE;
    }

    auto devStr = std::string{argv[1]};
    std::transform(devStr.begin(), devStr.end(), devStr.begin(), ::toupper);

    auto pName = std::string(PROCESS_NAME) + "-"s + devStr;

    devStr = "SHI_"s + devStr;
    auto sockName = configIf.getParam<std::string>(devStr + "_ZMQ_SOCKET_NAME"s).value_or("");
    auto baseAddrStr = configIf.getParam<std::string>(devStr + "_BASE_HEX_ADDRESS"s).value_or("");

    auto baseAddr{empower::helpers::types::strToNum<empower::helpers::types::reg_t>(baseAddrStr, 16).value_or(0)};

    empower::logger::getInst().setup(zmqCtx, pName);

    if((sockName.size() == 0) || (baseAddr < 0x80000000))
    {
        empower::logger::critical(__FILE__, __FUNCTION__, "Configuration Manager Values not Correct");
        return EXIT_FAILURE;
    }

    empower::helpers::poller poller;
    empower::serialHardwareInterfaceParser parser{zmqCtx, poller, configIf, sockName, baseAddr};
    empower::heartbeatResponse hb{zmqCtx, poller, configIf, pName};
    empower::helpers::run(poller);

    return EXIT_FAILURE;
}
