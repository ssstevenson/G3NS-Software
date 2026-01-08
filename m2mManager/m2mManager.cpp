#include <iostream>
#include <thread>
#include <chrono>
#include <zmq.hpp>
#include <configManagerConnection/configManagerConnection.h>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <statusRequestInterface/statusRequestInterface.h>
#include "m2mTCP.h"
#include "m2mUDP.h"
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
    openlog("m2mManager", LOG_PID | LOG_CONS, LOG_LOCAL1);
    std::shared_ptr<zmq::context_t> m2mctx = std::make_shared<zmq::context_t>(1);
    configManagerConnection  configIf{*m2mctx};
    std::shared_ptr<messageFactoryConnection > psmsgIf = std::make_shared<messageFactoryConnection >(*m2mctx);
    std::shared_ptr<statusRequestInterface > pStatusReqIface = std::make_shared<statusRequestInterface >(*m2mctx);


    syslog(LOG_INFO, "M2M  Manager Started  ");

    //
    // Lambda to handle the common logic for both TCP and UDP
    //
    auto handleNetworkConfig = [&](auto* m2mInstance, const std::string& connType, unsigned short port)  noexcept
    {

        if (!m2mInstance)
        {
            syslog(LOG_ERR, "---> Error Instance for  %s and port  %d \n", connType.c_str(), port);
            return;
        }

        // If the port is different, stop the session if running
        unsigned short currentPort = m2mInstance->getLocalPort();
        if (port != currentPort)
        {
            syslog(LOG_INFO, "---> Port for  %s  Changed [new port =%d<---old port= %d]", connType.c_str(), port, currentPort);
        //    if (m2mInstance->isRunning())
       //     {
                syslog(LOG_INFO, "---> Stopping %s  Session ", connType.c_str() );
                // Stop the running M2M session
                m2mInstance->setRunning(false);
                m2mInstance->stopNotificationThread();
        //    }

            if (currentPort != 0 )
            {
                // means port is closed
                m2mInstance->restartServer();
            }

        }

        // If the M2M instance is not running, start it
        if (!m2mInstance->isRunning())
        {
            // Setup the server socket with the new port
            syslog(LOG_DEBUG, "---> Setting up New session for  %s  ", connType.c_str());
            m2mInstance->SetupServerSocket();
            if  ( (currentPort == 0 ) || ( port != currentPort) )
                m2mInstance->setLocalPort(port);
            // Setup other configurations
            m2mInstance->setZmqCtx(m2mctx);
            m2mInstance->setMsgFactoryConnection(psmsgIf);
            m2mInstance->setStatusRequestIface(pStatusReqIface);

            // Start the threads for M2M server and notifications
            m2mInstance->startThread();
            m2mInstance->startNotificationThread();
        }

    };


    M2MTCP *m2tmcp;
    UDPServer *m2mudp;
    bool tcpRunning = false;
    bool udpRuning  = false;
    unsigned short currentPort = 0;
    //
    while (true )
    {

        // TCP Config
        bool tcpEnable = configIf.getParam<bool>("M2M_TCP_ENABLE").value_or(false);
        auto tcpPort{static_cast<unsigned short>(configIf.getParam<uint32_t>("M2M_TCP_PORT").value_or(3000))};
        //
        // TCP Config
        bool udpEnable = configIf.getParam<bool>("M2M_UDP_ENABLE").value_or(false);
        auto udpPort{static_cast<unsigned short>(configIf.getParam<uint32_t>("M2M_UDP_PORT").value_or(4000))};

        // Handle TCP connection
        if ( tcpEnable )
        {
            // Handle TCP connection based on the port
            if (currentPort == 0 ) {
                currentPort = tcpPort;
            }
            else if ( currentPort != tcpPort ) {
                syslog(LOG_DEBUG, "---> TCP Configuration new port =%d -- old port = %d  ", tcpPort, currentPort );
                currentPort = tcpPort;
            }
            m2tmcp = M2MTCP::getMyInstance(tcpPort);
            handleNetworkConfig(m2tmcp, "TCP", tcpPort );
            tcpRunning = true;
        }
        else if (tcpRunning)
        {
            m2tmcp = M2MTCP::getMyInstance(tcpPort);
            if (m2tmcp->isRunning())
            {
                m2tmcp->stopNotificationThread();
                m2tmcp->setRunning(false);
            }
            tcpRunning = false;
        }


        // Handle UDP connection
        if ( udpEnable )
        {
            // Handle UDP connection based on the port
            m2mudp = UDPServer::getMyInstance(udpPort);
            handleNetworkConfig(m2mudp, "UDP", udpPort );
            udpRuning = true;
        }
        else if (udpRuning)
        {
            m2mudp = UDPServer::getMyInstance(udpPort);
            if (m2mudp->isRunning())
            {
                m2mudp->stopNotificationThread();
                m2mudp->setRunning(false);
            }
            udpRuning = false;
        }

        // Handle Serial

        // sleep for 10 seconds and check of any configuration changes
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }  // while loop




    ///
    return 0;
}
