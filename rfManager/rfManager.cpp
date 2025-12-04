#include <heartbeatResponse/heartbeatResponse.h>
#include <helpers/mainHelper.h>
#include <logger/logger.h>
#include <parser/rfManagerParser.h>
#include <m2m/m2mProcessor.h>
#include <syslog.h>

/**
 * @brief      The RF Manager Main function
 *
 * @return     Should not return, if it does it is an Error.
 */
int main()
{
    openlog("rfManager", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO,"!!! RF MANAGER Process started!!!!");
    zmq::context_t zmqCtx{1};
    // create a shared pointer to RF manager ZMQ context
    std::shared_ptr<zmq::context_t> m2mctx(&zmqCtx, [](zmq::context_t*) {});
    empower::configManagerConnection configIf{zmqCtx};
    empower::logger::getInst().setup(zmqCtx, PROCESS_NAME);
    empower::helpers::poller poller;
    empower::rfManagerParser parser{zmqCtx, poller, configIf};
    empower::heartbeatResponse hb{zmqCtx, poller, configIf, PROCESS_NAME};
    // Start M2M Thread and pass FPGA
    FpgaHal fpgaHal(FpgaBase, FpgaMemSize);
    M2MProcessor m2mProcessor(parser.ctrl, m2mctx,fpgaHal);
    m2mProcessor.startM2MThread();
    empower::helpers::run(poller);

    // Wait for M2M thread
    m2mProcessor.getM2MThread().join();
    std::cout << "!!! RF MANAGER Process Stopped!!!!" << endl;
    syslog(LOG_INFO,"!!! RF MANAGER Process Stopped!!!!");
    closelog();
    return EXIT_FAILURE;
}
