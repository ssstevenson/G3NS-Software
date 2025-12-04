#include <heartbeatResponse/heartbeatResponse.h>
#include <helpers/mainHelper.h>
#include <logger/logger.h>
#include <parser/mqttInterfaceParser.h>

/**
 * @brief      The RF Manager Main function
 *
 * @return     Should not return, if it does it is an Error.
 */
int main()
{
    zmq::context_t zmqCtx{1};
    empower::configManagerConnection configIf{zmqCtx};
    empower::logger::getInst().setup(zmqCtx, PROCESS_NAME);
    empower::helpers::poller poller;
    empower::heartbeatResponse hb{zmqCtx, poller, configIf, PROCESS_NAME};
    empower::mqttInterfaceParser parser{zmqCtx, poller, configIf};
    empower::helpers::run(poller);

    return EXIT_FAILURE;
}
