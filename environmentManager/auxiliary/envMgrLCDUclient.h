/** @file */
/* =========================================================================*
* Copyright (C)Empower RF, 2021.  All rights reserved.                      *
*                                                                           *
*                                                                           *
* ==========================================================================*
*
*****************************************************************************
* Module Name: envMgrLCDUclient.h
*
* Functional Description:
*
* This file defines Simple  Messages Between LCC and Gen3 SOM
*
* Version History:
*
* Version  Date         Author          Description
* -------  ----------   ----------      --------------------------------
* 0       04/08/2021    Marc Obbad      Initial Release.
*
* 0.1       06/24/2021  Steve S         Ported into Env Mgr w/Renames and Cleanups
*
* 0.2       05/27/2022  Steve S         Updated for new CDU MODBUS Support
*

*****************************************************************************/
#ifndef __GEN3_LCC_CLIENT_H__
#define __GEN3_LCC_CLIENT_H__
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include<iostream>
#include<stdio.h>
#include<string.h>
#include <ctype.h>
#include<string>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>

#include <pthread.h>
#include <iostream>
#include "envMgrLCDUcmds.h"
#include "conf.h"
#include <poll.h>

using namespace std;

/////
//    Configuration related stuff
///

namespace empower
{
    class LCC_PUMP_CLIENT
    {
        private:
            int sock;
            string address;
            uint16_t port;
            bool isconnected;
            struct sockaddr_in server;
            bool StatusAvailable;
            bool ModBusStatusAvailable;

            void setStatusAvailable(bool dataready);
            SYSTEM_REPORT_DATA_S gSystemStatusData;

            void setModBusStatusAvailable(bool modBusDataReady);
            MODBUS_REPORT_DATA_S gModBusPumpData;

            bool gSystemPumpON;
            bool Loop1PumpON;
            bool Loop2PumpON;
            unsigned int connect_error_cnt;

            /// FIXME : SSS TEST VARS, REMOVE WHEN DONE
            unsigned int loop_cnt;
            unsigned int info_cnt;
            unsigned int alarm_cnt;



        public:
            bool clientConnected() { return isconnected;};
            void setConnected(bool incon) { isconnected= incon; };
            LCC_PUMP_CLIENT(string, uint16_t);
            bool conn();
            bool send_data(void *data, unsigned int len);
            void sendPumpOn();
            void sendLoop1On();
            void sendLoop2On();
            void sendPumpOff();
            void sendLoop1Off();
            void sendLoop2Off();
            void sendGetSystemInfo();
            void sendCfgData(CONFIGURATION_DATA_S &cfgdata);
            int  receive();
            void processMessage(GENERAL_COMMANDS &msg);
            void close();
            void printMessage(string &msg);

            static pthread_t rxThread;
            static pthread_t socketThread;
            //
            static void *rxMain(void *ptr);
            static void *clientManager(void *ptr);

            // newly added from Marc's update 2022-05-31
            bool getCfgData(const char *filename);
            void printConfig(GENERAL_COMMANDS &msg);
            void sendGetConfig();

            /// SSS ADDIN FOR ENV MGR I/F
            int  checkCDUdata(std::string hostIP, uint16_t hostPort, std::string msg );
            int  lccMain(std::string hostIP, uint16_t hostPort, std::string msg);
            bool loadPumpConfigFile();
            bool getStatusAvailable();
            bool getModBusStatusAvailable();
            void setIpAddress(std::string ipaddr);
            void setIpPort(uint16_t port);
            SYSTEM_REPORT_DATA_S getStatusData();
            MODBUS_REPORT_DATA_S getModBusPumpData();

            bool getSystemPumpONstate();
            bool getLoop1PumpONstate();
            bool getLoop2PumpONstate();

            void renewSock();
            unsigned int getConnectErrorCnt();

    };
}
#endif
