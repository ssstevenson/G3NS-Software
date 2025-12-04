/** @file */
/* =========================================================================*
* Copyright (C)Empower RF, 2021.  All rights reserved.                      *
*                                                                           *
*                                                                           *
* ==========================================================================*
*
*****************************************************************************
* Module Name: Commands.h
*
* Functional Description:
*
* This file defines Simpe  Messages Between LCC and Gen3 SOM
*
* Version History:
*
* Version  Date        Author         Description
* -------  ----------  ----------     --------------------------------
* 0       04/08/2021 Marc Obbad       Initial Release.
*
*****************************************************************************/
#ifndef __GEN3_LCC_CLIENT_H__
#define __GEN3_LCC_CLIENT_H__

#include <arpa/inet.h>
#include <string>
#include <sys/socket.h>

#include "Commands.h"
#include "conf.h"

// using namespace std;

class LCC_PUMP_CLIENT
{
    public:
        using CONFIG_TYPE = LoadConfig::CONFIG_TYPE;
        using SosConfSet_T = LoadConfig::SosConfSet_T;

    private:
        int sock;
        std::string address;
        std::uint16_t port;
        struct sockaddr_in server;
        bool isconnected;

    public:
        LCC_PUMP_CLIENT(std::string, std::uint16_t);

        bool clientConnected() { return isconnected;};
        void setConnected(bool incon) { isconnected = incon; };

        bool conn();
        bool send_data(void *data, const std::size_t len);
        void sendPumpOn(const int);
        void sendPumpOff(const int);
        void sendCfgData(CONFIGURATION_DATA_S &cfgdata);
        int  receive();
        void processMessage(GENERAL_COMMANDS &msg);
        void close();
        void printMessage(const std::string &msg);
        static pthread_t rxThread;
        static pthread_t socketThread;
        //
        static void *rxMain(void *ptr);
        static void *clientManager(void *ptr);
        bool getCfgData(const char *filename);
        void printConfig(GENERAL_COMMANDS &msg);
        void sendGetConfig();
};

#endif
