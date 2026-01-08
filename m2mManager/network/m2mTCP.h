/*!
 * \file
 * \brief M2M command Processor
 *
 ===============================================================================

 Name        : M2MCP.h
 Author      : Marc Obbad
 Version     :

 Description :  This header file has defines and declarations for all the
                M2M commands...


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
* 0      12/1/2025   Marc Obbad       Initial Release.

*
*****************************************************************************/

#pragma once

#include <signal.h>
#include <vector>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "M2MCP.h"
#include "socketBase.h"
#include "systemConfig.h"
#include <mutex>
#include <thread>
#include <atomic>

class M2MTCP: public TCPServerSocket, public M2MC_CP
{

    public:

        virtual ~M2MTCP();
        static M2MTCP *getMyInstance( unsigned short  port );
        unsigned short getServerPort() { return serverPort; };
        // Prevent copying (fix -Weffc++ warnings)
        M2MTCP(const M2MTCP&) = delete;
        M2MTCP& operator=(const M2MTCP&) = delete;

        void startThread() override ;
        void handleExecption() override;
         void  M2MTCPMain ( );
         void  newConnMain();
        static void sigHandler(int signal, siginfo_t *si, void *arg);
        void init_exception();

        // Recreate Socket Server
        void restartServer();
        //
        void setRemoteIP ( std::string remote) { remoteIP=remote; };
        void setRemotePort (int port ) { remotePort = port; };
        bool isConnected () { return connected; };
        void setConnected ( bool val) { connected = val; };
        //
        void m2Mrespond( string response) override;
        //
        void closeConnection();
        void m2mCloseServerConnection();
        //
        bool isRunning() { return isrunning;};
        void setRunning ( bool halt) { isrunning = halt;};

        TCPSocket * getTcpConn() { return newConn; };
        void setNewConnection (TCPSocket  *conn) { newConn=conn ;} ;

        void stopConnection() { setRunning(false);};
        void SetupServerSocket();

        static std::mutex m2m_mutex;
    private:
        static M2MTCP   *myInstance; // Single Instance of this object
        bool   isrunning;
        //
        std::string  remoteIP  ;
        int         remotePort;
        bool  connected;
        TCPSocket  *newConn;
        M2MTCP(unsigned short  srvrPort);
        int serverSocket;
        int clientScoket;
        unsigned short  serverPort;
        void m2mCloseConnection() override;

        std::thread clientThread ;



};

