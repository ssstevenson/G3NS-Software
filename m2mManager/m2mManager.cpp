#include <zmq.hpp>
#include <configManagerConnection/configManagerConnection.h>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <statusRequestInterface/statusRequestInterface.h>
#include "m2mTCP.h"
#include <syslog.h>


/**
 * @brief      M2M Manager Main Function
 *
 * @return     Only exits in Failure
 */
using namespace std;
using namespace empower;

int main()
{
    openlog("m2mManager", LOG_PID | LOG_CONS, LOG_USER);
    std::shared_ptr<zmq::context_t> m2mctx = std::make_shared<zmq::context_t>(1);
    configManagerConnection  configIf{*m2mctx};
    std::shared_ptr<messageFactoryConnection > psmsgIf = std::make_shared<messageFactoryConnection >(*m2mctx);
    std::shared_ptr<statusRequestInterface > pStatusReqIface = std::make_shared<statusRequestInterface >(*m2mctx);
    //
    //  Check for TCP first
    //
    if(configIf.getParam<bool>("M2M_TCP_ENABLE").value_or(false))
    {
        auto port{static_cast<unsigned short>(configIf.getParam<uint32_t>("M2M_TCP_PORT").value_or(3000))};
        M2MTCP *m2tmcp = M2MTCP::getMyInstance();
        if ( port != m2tmcp->getLocalPort() )
        {
            m2tmcp->setLocalPort(port);
        }
        m2tmcp->setZmqCtx(m2mctx);
        m2tmcp->setMsgFactoryConnection(psmsgIf);
        m2tmcp->setStatusRequestIface(pStatusReqIface);
        m2tmcp->startThread();
        m2tmcp->startNotificationThread();
        pthread_join(m2tmcp->getMyThread(), NULL);
        pthread_join(m2tmcp->getNotifThread(), NULL);

    }
    // Get all M2M Configuration Data such as serial, UDP...etc



    ///
    return 0;
}
