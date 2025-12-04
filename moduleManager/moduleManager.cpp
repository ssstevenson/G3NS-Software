#include <heartbeatResponse/heartbeatResponse.h>
#include <helpers/mainHelper.h>
#include <logger/logger.h>
#include <parser/moduleManagerParser.h>

/**
 * @brief      The Module Manager Main function
 *
 * @return     Should not exit, it is a Failure if it does.
 */
int main()
{
    zmq::context_t zmqCtx{1};
    empower::configManagerConnection configIf{zmqCtx};
    empower::logger::getInst().setup(zmqCtx, PROCESS_NAME);
    empower::helpers::poller poller;
    empower::moduleManagerParser parser{zmqCtx, poller, configIf};
    empower::heartbeatResponse hb{zmqCtx, poller, configIf, PROCESS_NAME};
    empower::helpers::run(poller);

    return EXIT_FAILURE;
}
