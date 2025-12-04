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

#ifndef __TCP_M2M_SERVER_SOCKET__
#define __TCP_M2M_SERVER_SOCKET__

#include <signal.h>
#include <vector>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "M2MCP.h"
#include "socketBase.h"
#include "systemConfig.h"

class M2MTCP: public TCPServerSocket, public M2MC_CP
{

    public:

        virtual ~M2MTCP();
        static M2MTCP *getMyInstance();
        // Prevent copying (fix -Weffc++ warnings)
        M2MTCP(const M2MTCP&) = delete;
        M2MTCP& operator=(const M2MTCP&) = delete;

        void startThread() override ;
        void handleExecption() override;
        static void  *M2MTCPMain ( void *ptr);
        static void  *newConnMain( void *ptr);

        static void sigHandler(int signal, siginfo_t *si, void *arg);
        void init_exception();

        // Recreate Socket Server
        void restartM2MTCP();
        //
        void setRemoteIP ( std::string remote) { remoteIP=remote; };
        void setRemotePort (int port ) { remotePort = port; };
        bool isConnected () { return connected; };
        void setConnected ( bool val) { connected = val; };
        //
        void m2Mrespond( string response) override;
        //
        void closeConnection();

        //
        bool isRunning() { return isrunning;};

        TCPSocket * getTcpConn() { return newConn; };
        void setNewConnection (TCPSocket  *conn) { newConn=conn ;} ;

        void stopConnection() { setRunning(false);};

    private:
        static M2MTCP   *myInstance; // Single Instance of this object
        bool   isrunning;
        void setRunning ( bool halt) { isrunning = halt;};
        unsigned short m2m_tcp_server_port;
        //
        void SetupM2MTCPServer();
        std::string  remoteIP  ;
        int         remotePort;
        bool  connected;
        TCPSocket  *newConn;
        M2MTCP();


        void m2mCloseConnection() override;


};
#endif
