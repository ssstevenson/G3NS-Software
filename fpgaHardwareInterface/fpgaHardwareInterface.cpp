#include <fpgaInterface/fpgaInterface.h>
#include <heartbeatResponse/heartbeatResponse.h>
#include <helpers/mainHelper.h>
#include <logger/logger.h>
#include <syslog.h>

/**
 * @brief      FPGA Hardware Interface Main Function
 *
 * @return     Only returns on Failure
 */
int main()
{
    openlog("FpgaHwInterface", LOG_PID | LOG_CONS, LOG_LOCAL0);
    zmq::context_t zmqCtx{1};
    empower::configManagerConnection configIf{zmqCtx};
    empower::logger::getInst().setup(zmqCtx, PROCESS_NAME);
    empower::helpers::poller poller;
    empower::fpgaInterface parser{zmqCtx, poller, configIf};
    empower::heartbeatResponse hb{zmqCtx, poller, configIf, PROCESS_NAME};
    empower::helpers::run(poller);

    return EXIT_FAILURE;
}
