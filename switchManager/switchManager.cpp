#include <heartbeatResponse/heartbeatResponse.h>
#include <helpers/mainHelper.h>
#include <logger/logger.h>
#include <parser/switchManagerParser.h>

/**
 * @brief      The Switch Manager Main Function
 *
 * @return     This should not exit, if it does it is an Error
 */
int main()
{
    zmq::context_t zmqCtx{1};
    empower::configManagerConnection configIf{zmqCtx};
    empower::logger::getInst().setup(zmqCtx, PROCESS_NAME);
    empower::helpers::poller poller;
    empower::switchManagerParser parser{zmqCtx, poller, configIf};
    empower::heartbeatResponse hb{zmqCtx, poller, configIf, PROCESS_NAME};
    empower::helpers::run(poller);

    return EXIT_FAILURE;
}
