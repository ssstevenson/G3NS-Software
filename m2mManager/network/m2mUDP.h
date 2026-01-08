/*!
 * \file
 * \brief M2M UDP Server
 *
 ===============================================================================

 Name        : m2mUDP.h
 Author      : Marc Obbad
 Version     :

 Description :  This header file has defines and declarations of UDP Server


===============================================================================

 Copyright   : (C) Copyright 2025 Empower RF Systems

===============================================================================
*
*
*  Author: Marc Obbad
*/

/*Version History:
*
* Version  Date        Author         Description
* -------  ----------  ----------     --------------------------------
* 0      12/15/2025   Marc Obbad       Initial Release.

*
*****************************************************************************/

#pragma once

#include <signal.h>
#include <vector>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include "M2MCP.h"
#include "socketBase.h"
#include "systemConfig.h"

class UDPServer: public UDPSocket, public M2MC_CP
{

    public:

        virtual ~UDPServer();
        static UDPServer *getMyInstance(unsigned short port );
        unsigned short getServerPort() { return serverPort; };
        void startThread();
        void handleExecption() override;
        void  udpServertMain ();

        // Recreate Socket Server
        void SetupServerSocket();
        void restartServer();
        void m2Mrespond( string response) override;
        bool isRunning() { return isrunning;};
        void setRunning ( bool stop) { isrunning = stop;};

        // Abstradt source addr and port
        int read ( char *buff, int maxLem ) {
            int len =  recvFrom(buff, maxLem, sourceAddress, sourcePort);
            return len;
        }

        int write( string &reply ) {
             int len = sendTo( reply.c_str(), static_cast <int> (reply.size()) , sourceAddress,sourcePort);
             return len;
        }
        void  closeConnection();
        static std::mutex m2m_mutex;

private:
        //void processData( UI_DATA &uiData );
        static UDPServer   *myInstance; // Single Instance of this object
        bool   isrunning;

        //

        UDPServer(unsigned short port);

        void m2mCloseConnection() override;
        string sourceAddress;             // Address of datagram source
        unsigned short sourcePort;        // Port of datagram source
        unsigned short  serverPort;


};
