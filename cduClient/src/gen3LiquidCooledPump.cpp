

/*
* Copyright (C) 2021  Empower RF, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
 *
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* EMPOWER RF  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/


/*
* Implementation for the custom Test Application to Send Comamnds over TCP/IP.
* This softwate emulate client side
*/

#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <netdb.h>

#include "gen3LiquidCooledPump.h"

//#define DEBUG_MODBUS_NET

pthread_t LCC_PUMP_CLIENT::rxThread  = 0;
pthread_t LCC_PUMP_CLIENT::socketThread = 0;

static  CONFIGURATION_DATA_S  ConfigData;
LCC_PUMP_CLIENT::SosConfSet_T params[]=
{
    {"LOOP1_FLOW_RATE",                     LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.Loop1Flow},
    {"LOOP2_FLOW_RATE",                     LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.Loop2Flow},
    {"STOP_LOOP_ON_LOW_FLOW",               LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopLoopOnLowFlow},
    {"PUMP_PURGE_TIME_SECONDS",             LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.pumpPurgeTimeInSeconds},
    {"SHUTDOWN_LINE_PER_RACK",              LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.shutdownLinePerRack},
    {"NIGHT_MODE_ENABLED",                  LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.nightModeOn},
    {"AIRVENTING_PROC_ON",                  LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.AirVentingProcedureOn},
    {"AIRVENTING_ON_POWER",                 LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.AirVentingPowerOn},
    {"STOP_LOOP_ON_REMOTE_OFF_1",           LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopLoopOnRemoteOnOff_1},
    {"STOP_LOOP_ON_REMOTE_OFF_2",           LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopLoopOnRemoteOnOff_2},
    {"STOP_ON_LEAK",                        LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopOnLeak},
    {"LOOP1_ACTIVE",                        LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.loop1Active},
    {"STOP_LOOP_ON_ACCESSORY_SHUTDOWN_1",   LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopLoopOnAccessoryShutDown_1},
    {"STOP_LOOP_ON_EXCHANGE_PRESSURE_1",    LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopLoopOnExchagePressure_1},
    {"STOP_LOOP_ON_FILTER_PRESSURE_1",      LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopLoopOnfilterPressureSnsr_1},
    {"LOOP2_ACTIVE",                        LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.loop2Active},
    {"STOP_LOOP_ON_ACCESSORY_SHUTDOWN_2",   LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopLoopOnAccessoryShutDown_2},
    {"STOP_LOOP_ON_EXCHANGE_PRESSURE_2",    LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopLoopOnExchagePressure_2},
    {"STOP_LOOP_ON_FILTER_PRESSURE_2",      LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.stopLoopOnfilterPressureSnsr_2},
    {"LIQUID_TO_AIR_EXCHANGE_PRESENT",      LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.startLiquidToAirExchanger},
    {"SKIP_PUMP_REGULATION_AFTER_START",    LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.skipPumpRegulation},
    {"COLD_START_ENABLED",                  LCC_PUMP_CLIENT::CONFIG_TYPE::BOOLEAN_T,    &ConfigData.coldStartEnabled},
    {"COLD_START_TRIGGER_TEMP",             LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.coldStartTriggerTemp},
    {"COLD_START_PREHEAT_TIME_SEC",         LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.coldStartPreHeaTime},
    {"FLUSH_COOLANT_PREHEAT_TIME_SEC",      LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.flushCoolantPreHeaTime},
    {"BOOSTER_PREHEAT_TIME_SEC",            LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.bosterPreHeaTime},
    {"PRE_HEAT_DELAY_SEC",                  LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.PreHeatDelay},
    {"INITIAL_WARMUP_TEMP",                 LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.intialWarmupTemp},
    {"PUMP_TEMP_OPER_LIMIT",                LCC_PUMP_CLIENT::CONFIG_TYPE::NUMBER_T,     &ConfigData.pumpOperTempLimit},
    {"",                                    LCC_PUMP_CLIENT::CONFIG_TYPE::NONE_T,       0}
};

LCC_PUMP_CLIENT::LCC_PUMP_CLIENT(std::string srvaddress, std::uint16_t srvport):
    sock{-1},
    address{srvaddress},
    port{srvport},
    server{},
    isconnected{false}
{
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if(sock == -1)
    {
        perror("Could not create socket");
    }
}

/**
    Connect to a host on a certain port number
*/
bool LCC_PUMP_CLIENT::conn()
{
    bool ret = false;
    if(sock > 0 )
    {
        //setup address structure
        if(inet_addr(address.c_str()) == static_cast<in_addr_t>(-1))
        {
            struct hostent *he;
            struct in_addr **addr_list;

            //resolve the hostname, its not an ip address
            if( (he = gethostbyname( address.c_str() ) ) == NULL)
            {
                //gethostbyname failed
                herror("gethostbyname");
                std::cout << "Failed to resolve hostname" << std::endl;

                return false;
            }

            //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
            // addr_list = (struct in_addr **) he->h_addr_list;
            addr_list = reinterpret_cast<struct in_addr **>(he->h_addr_list);

            for(int i = 0; addr_list[i] != NULL; i++)
            {
                //strcpy(ip , inet_ntoa(*addr_list[i]) );
                server.sin_addr = *addr_list[i];

                std::cout << address<<" resolved to "<<inet_ntoa(*addr_list[i])<< std::endl;

                break;
            }
        }
        else
        {
            server.sin_addr.s_addr = inet_addr( address.c_str() );
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        //Connect to remote server
        // if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        if(connect(sock, reinterpret_cast<struct sockaddr *>(&server), sizeof(server)) < 0)
        {
            perror("connect failed. Error");
        }
        else
        {
            isconnected = true;
            ret = true;
            std::cout << "Connected." << std::endl;
        }
    }

    return ret;
}

/**
    Send data to the connected host
*/
void LCC_PUMP_CLIENT::sendPumpOn(const int loop)
{
    GENERAL_COMMANDS msg;

    switch (loop)
    {
    case  1:
        msg.commandID = htonl(TLV_SET_LOOP1_ON);
        break;
    case 2:
        msg.commandID = htonl(TLV_SET_LOOP2_ON);
        break;
    case 3:
    default:
        msg.commandID = htonl(TLV_SET_PUMPS_ON);
        break;
    }

    if(isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
    }
}

void LCC_PUMP_CLIENT::sendGetConfig()
{
    GENERAL_COMMANDS msg;

    msg.commandID = htonl(TLV_SYSTEM_CONGIURATION_READ);

    if(isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
    }
}

void LCC_PUMP_CLIENT::sendPumpOff(const int loop)
{
    GENERAL_COMMANDS msg;

    switch (loop)
    {
    case  1:
        msg.commandID = htonl(TLV_SET_LOOP1_OFF);
        break;
    case 2:
        msg.commandID = htonl(TLV_SET_LOOP2_OFF);
        break;
    case 3:
    default:
        msg.commandID = htonl(TLV_SET_PUMPS_OFF);
        break;
    }

    if(isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
    }
}


bool LCC_PUMP_CLIENT::send_data(void *data, const std::size_t len)
{
    //Send some data
    if(send(sock, data, len, 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }

    std::cout << "Data sent!" << std::endl;

    return true;
}

void  LCC_PUMP_CLIENT::processMessage(GENERAL_COMMANDS &msg)
{
    unsigned int commandId = ntohl(msg.commandID);
    unsigned int msgSize   = ntohl(msg.commandSize);
    unsigned int id        = ntohl(msg.data[0]);

    switch(commandId)
    {
        case TLV_PUMP_LEAK:
            std::cout << " PUMP_LEAK  Event detected...System shutting down " << std::endl;
            break;

        case TLV_PUMP_PHASE_ERROR:
            std::cout << " PUMP_PHASE_ERROR Event detected...System shutting down " << std::endl;
            break;

        case TLV_SYSTEM_EMERGENCY_STOP:
            std::cout << " SYSTEM_EMERGENCY_STOP Event detected...System shutting down " << std::endl;
            break;

        case TLV_SYSTEM_ACCESSORY_SHUTDOWN:
            std::cout << " SYSTEM_ACCESSORY_SHUTDOWN 1  Event detected...System shutting down " << std::endl;
            break;

        case TLV_SYSTEM_ACCESSORY_2_SHUTDOWN:
            std::cout << " SYSTEM_ACCESSORY_SHUTDOWN 2  Event detected...System shutting down " << std::endl;
            break;

        case TLV_SYSTEM_REMOTE_ON_OFF:
            std::cout << " SYSTEM_REMOTE_ON_OFF  Event detected...System shutting down " << std::endl;
            break;

        case TLV_SYSTEM_EXCHANGE_PRESSURE:
            std::cout << " SYSTEM_EXCHANGE_PRESSURE Event detected for loop" << id<< "...System shutting down " << std::endl;
            break;

        case TLV_SYSTEM_FILTER_PRESSURE:
            std::cout << " SYSTEM_FILTER_PRESSURE Event detected for loop " << id<< "...System shutting down " << std::endl;
            break;

        case TLV_SYSTEM_RESERVOIR_FULL:
            std::cout << " SYSTEM_RESERVOIR_FULL Event detected for Reservoir " << id<< "...System shutting down " << std::endl;
            break;

        case TLV_SYSTEM_RESERVOIR_LOW:
            std::cout << " SYSTEM_RESERVOIR_LOW Event detected for reservoir " << id<< "...System shutting down " << std::endl;
            break;

        case TLV_PUMP_ALARM:
            std::cout << " PUMP_ALARM Event detected for PUMP =" << id<< "...System shutting down " << std::endl;
            break;

        case TLV_PUMP_STATUS:
        {
            PUMP_INFO_S  pumpInfo;
            pumpInfo.pumpID = ntohl(msg.data[0]);
            pumpInfo.pumpFlowRaw = ntohl(msg.data[1]);
            pumpInfo.pumpFlow = static_cast<float>(ntohl(msg.data[2]));
            pumpInfo.pumpTempRaw = ntohl(msg.data[3]);
            pumpInfo.pumpTemp = static_cast<float>(ntohl(msg.data[4]));
            std::cout << " PUMP " << pumpInfo.pumpID << "FLOW [raw:"<< pumpInfo.pumpFlowRaw << ",val:"<<  pumpInfo.pumpFlow << "]"<< std::endl;
            std::cout << " PUMP " << pumpInfo.pumpID << "TEMP [raw:"<< pumpInfo.pumpTempRaw << ",val:"<<  pumpInfo.pumpTemp << "]"<< std::endl;
            break;
        }
        case TLV_LOOP_STATUS:
        {
            PUMP_LOOP_S  loopInfo;
            loopInfo.loopID =  ntohl(msg.data[0]);
            loopInfo.loopPressureRaw = ntohl(msg.data[1]);
            loopInfo.loopPressure = static_cast<float>(ntohl(msg.data[2]));
            loopInfo.loopTempRaw = ntohl(msg.data[3]);
            loopInfo.loopTemp = static_cast<float>(ntohl(msg.data[4]));
            std::cout << " Loop " << loopInfo.loopID << "Pressure [raw:"<< loopInfo.loopPressureRaw << ",val:"<<  loopInfo.loopPressure << "]"<< std::endl;
            std::cout << " Loop " << loopInfo.loopID << "TEMP [raw:"<< loopInfo.loopTempRaw << ",val:"<<  loopInfo.loopTemp  << "]"<< std::endl;
            break;
        }
        case TLV_RESERVOIR_STATUS:
        {
            RESERVOIR_TEMP_S  reservoir1, reservoir2;
            reservoir1.reservoirId = static_cast<std::int32_t>(ntohl(msg.data[0]));
            reservoir1.reservoirTempRaw = ntohl(msg.data[1]);
            reservoir1.reservoirTemp = static_cast<float>(ntohl(msg.data[2]));
            reservoir2.reservoirId = static_cast<std::int32_t>(ntohl(msg.data[3]));
            reservoir2.reservoirTempRaw = ntohl(msg.data[4]);
            reservoir2.reservoirTemp = static_cast<float>(ntohl(msg.data[5]));
            std::cout << " Reservoir " << reservoir1.reservoirId << "Temperature [raw:" << reservoir1.reservoirTempRaw << ",val:" << reservoir1.reservoirTemp << "]" << std::endl;
            std::cout << " Reservoir " << reservoir2.reservoirId << "Temperature [raw:" << reservoir2.reservoirTempRaw << ",val:" << reservoir2.reservoirTemp << "]" << std::endl;
            break;
        }

        case TLV_SYSTEM_MODBUS_INFO:
        {
            float tmp;
            GENERAL_MODBUS_MSG_S *modBusMsg = reinterpret_cast<GENERAL_MODBUS_MSG_S*>(&msg);
            PumpModBusInfo_S_U *mobBusInfo_u = reinterpret_cast<PumpModBusInfo_S_U*>(&modBusMsg->modBusInfo);
            PumpModBusInfo_S *mobBusInfo = &(mobBusInfo_u->PumpModBusInfoS);

            for(int i= 0; i < static_cast<int>(PMOD_INFO_MSG_SIZE); i++)
            {
                mobBusInfo_u->data[i] = static_cast<unsigned short>(ntohl(msg.data[i]));

#ifdef DEBUG_MODBUS_NET
                printf("\n\n[%s] --- Modbus Data Index %d [ net val =%x ]\n",__FUNCTION__, i, msg.data[i]  );
#endif
            }

            std::cout << std::endl << " MODBUS INFO for Pump: " << mobBusInfo->pumpID + 1 << std::endl;
            // Configuration
            std::cout << "   Configuration: " << std::endl;
            std::cout << "-------Operation Mode: " << ( mobBusInfo->PumpCFG.OperMode == E_PUMP_OPER_MODE_OFF? "OFF": "ON" ) << std::endl;
            if(mobBusInfo->PumpCFG.ctrlMode < E_PUMP_MAX_CTRL_MODE )
            {
              std::cout << "-------Control Mode: " << PUMP_CTRL_MODE_S[mobBusInfo->PumpCFG.ctrlMode] << std::endl;
            }
            std::cout << "-------Night Mode: " << ( mobBusInfo->PumpCFG.nightMode == E_PUMP_NIGHT_MODE_NON_ACTIVE? "INACTIVE": "ACTIVE") << std::endl;
            std::cout << "-------Air Venting Procedure: " << (mobBusInfo->PumpCFG.airVentingProc == E_PUMP_AV_PROCEDURE_NON_ACTIVE? "INACTIVE": "ACTIVE") << std::endl;

            tmp = static_cast<float>(mobBusInfo->PumpCFG.propPressureSetpoint) / 100.0f;
            std::cout << "-------Proportional Pressure Setpoint : " << tmp << " [m] ---->"  << tmp*HEAD_TO_PSI << " [PSI]" << std::endl;
            tmp = static_cast<float>(mobBusInfo->PumpCFG.constPressureSetpoint) / 100.0f;
            std::cout << "-------Constant Pressure Setpoint : " << tmp << " [m] ---->" << tmp*HEAD_TO_PSI << " [PSI]" << std::endl;
            tmp = static_cast<float>(mobBusInfo->PumpCFG.curvePressureSetpoint);
            std::cout << "-------Curve Pressure Setpoint : " << tmp << " [RPM]" << std::endl;

            std::cout << "-------Air Vent Power On: " << (mobBusInfo->PumpCFG.airVentPowerOn == E_PUMP_AV_POWER_ON_INACTIVE? "INACTIVE": "ACTIVE") << std::endl;

            //
            std::cout << "   TWIN mode fpr Pump : " << (mobBusInfo->pumpID + 1) << std::endl;
            if(mobBusInfo->PumpTwinCFG.circularConfigMode <= E_PUMP_SINGLE)
            {
                std::cout << "-------Twin Mode Circular : " << CIRCULAR_CONFIG_S[mobBusInfo->PumpTwinCFG.circularConfigMode] << std::endl;
            }
            if(mobBusInfo->PumpTwinCFG.twinCtrlMode <= E_PUMP_TWIN_FORCED_PARALLEL)
            {
                std::cout << "-------Twin Control Mode  : " << TWIN_CRTL__CONFIG_S[mobBusInfo->PumpTwinCFG.twinCtrlMode] << std::endl;
            }

            //
            std::cout << "   Status PumpId : " << (mobBusInfo->pumpID + 1) << std::endl;
            std::cout << "-------Input Power : " << std::dec << mobBusInfo->PumpStatus.InputPower << "[W]" << std::endl;
            tmp = static_cast<float>(mobBusInfo->PumpStatus.Head) / 100.0f;
            std::cout << "-------HEAD : " << tmp << " [m] ---->" << (tmp * HEAD_TO_PSI) << " [PSI]" << std::endl;
            tmp = static_cast<float>(mobBusInfo->PumpStatus.Flow) / 100.0f;
            std::cout << "-------FLOW : " << tmp << " [L/S] ---->" << (tmp * LITREPERSEC_TO_GALPERMIN) << " [GPM]" << std::endl;
            std::cout << "-------Speed : " << std::dec << mobBusInfo->PumpStatus.Speed << " [RPM]" << std::endl;
            tmp = static_cast<float>(mobBusInfo->PumpStatus.WaterTemp) / 10.0f;
            std::cout << "-------Water temp : " << tmp << " [Degree Celcius]" << std::endl;
            //mobBusInfo->PumpStatus.WaterTemp
            // PumpStatus.ExternalWaterTemp
            // mobBusInfo->PumpStatus.Winding1Temp);
            //mobBusInfo->PumpStatus.Winding2Temp);
            //mobBusInfo->PumpStatus.Winding3Temp);
            // mobBusInfo->PumpStatus.PowerModuleTemp);
            //mobBusInfo->PumpStatus.QuadrantCurrent);
            // mobBusInfo->PumpStatus.BitFieldStatusIO);
            // mobBusInfo->PumpStatus.BitFieldAlarm1);
            // mobBusInfo->PumpStatus.BitFieldAlarm2);
            std::cout << "-------BIT FIELD ERRORS : " << std::hex << mobBusInfo->PumpStatus.BitFieldErrors << std::endl;
            if(mobBusInfo->PumpStatus.BitFieldErrors > 0)
            {
                std::cout << std::endl;
                unsigned tmps = mobBusInfo->PumpStatus.BitFieldErrors;
                for(int i = 0; i < 15; ++i)
                {
                    if((tmps >> i) & 0x01)
                    {
                       std::cout << " BIT " << i << "------>" <<  PumpBitErrorrs[i] << std::endl;
                    }
                }
            }
            else
            {
                std::cout << " NONE." << std::endl;
            }

            if(mobBusInfo->PumpStatus.ActiveErrorCode < MAX_ACTIVE_ERROR_CODE)
            {
                std::cout << "-------ACTIVE ERROR : " << PumpActiveErrors[mobBusInfo->PumpStatus.ActiveErrorCode] << std::endl;
            }
            std::cout << "===========================LOG COUNTER =====================" << std::endl;
            std::cout << "-------Life Timer : " <<  mobBusInfo->PumpLogCntrTable.lifeTimerLSW + (mobBusInfo->PumpLogCntrTable.lifeTimerMSW << 16) << std::endl;
            std::cout << "-------Power Consupmtion 0-25 : " <<  mobBusInfo->PumpLogCntrTable.PowerConsuptionLSW_25 + (mobBusInfo->PumpLogCntrTable.PowerConsuptionMSW_25 << 16) << std::endl;
            std::cout << "-------Power Consupmtion 25-50 : " <<  mobBusInfo->PumpLogCntrTable.PowerConsuptionLSW_50 + (mobBusInfo->PumpLogCntrTable.PowerConsuptionMSW_50 << 16) << std::endl;
            std::cout << "-------Power Consupmtion 50-75 : " <<  mobBusInfo->PumpLogCntrTable.PowerConsuptionLSW_75 + (mobBusInfo->PumpLogCntrTable.PowerConsuptionLSW_75 << 16) << std::endl;
            std::cout << "-------Power Consupmtion 50-75 : " <<  mobBusInfo->PumpLogCntrTable.PowerConsuptionLSW_100 + (mobBusInfo->PumpLogCntrTable.PowerConsuptionMSW_100 << 16) << std::endl;

            ////////////////////////
            std::cout << std::endl << std::endl;
            break;
        }

        case TLV_SYSTEM_CONGIURATION_READ:
            printConfig(msg);
            break;

        case TLV_SYSTEM_RESTART:
            {
              auto loopID = ntohl(msg.data[0]);
              std::cout << " Loop ID : " << loopID << " Started/Restarted " << std::endl;
            }
            break;

        case TLV_SYSTEM_INFORMATION:
        {
            PUMP_SYSTEM_STATUS_S sysStat;
            sysStat.Loop1Id = ntohl(msg.data[0]);
            sysStat.Loop1IdState = ntohl(msg.data[1]);
            sysStat.Loop2Id = ntohl(msg.data[2]);
            sysStat.Loop2IdState = ntohl(msg.data[3]);
            std::cout << "Loop 1 State = " << ((sysStat.Loop1IdState == 0)?"STOPPED":"RUNNING") << std::endl;
        }
            break;

        default:
            printf(" %s()!!! UNKOWN TCP  Message received--[ msg size=%d--cmd=0x%0x] ---\n", __FUNCTION__, msgSize,msg.commandID);
            break;
    }
}
/**
    Receive data from the connected host
*/
int LCC_PUMP_CLIENT::receive()
{
   GENERAL_COMMANDS msg;

    struct pollfd fds[1];
    std::size_t nfds ;
    int timeout = 2000;

    fds[0].fd = sock;
    nfds = 1;
    fds[0].events = POLLIN;
    // Wait for Data to be present in Sockets
    auto ret = poll(fds, nfds, timeout);

    if(ret > 0 )
    {
        if((fds[0].revents & POLLIN ) && (fds[0].fd == sock) )
        {
            unsigned int rxlen = 0;
            unsigned int toRead = sizeof(GENERAL_COMMANDS);
            unsigned int nbRead = 0;

            while (rxlen < sizeof (GENERAL_COMMANDS))
            {
                rxlen = static_cast<unsigned int>(::recv(sock, &msg, (toRead - rxlen), 0));
                nbRead += rxlen;
                if( nbRead == sizeof (GENERAL_COMMANDS) )
                {
                    break;
                }
                rxlen = toRead - nbRead;
            }

            if(nbRead > 0)
            {
                processMessage(msg);
                if(nbRead != toRead)
                {
                    std::cout << " !!!!!!!!!!TCP received Fragmented Message! !!!!!!!!! " << std::endl;
                }
            }
        }
    }
    else
    {
        if(fds[0].revents & (POLLERR | POLLHUP | POLLNVAL) )
        {
            ret = -1;
            close();
            isconnected = false;
        }
    }

    return ret;
}

void LCC_PUMP_CLIENT::close()
{
    ::close(sock);
}

void LCC_PUMP_CLIENT::printMessage(const std::string& reply)
{
    printf("\nRX Data  In HEX: ");
    for(unsigned int i = 0; i < reply.size(); ++i)
    {
        printf("0x%02X ", static_cast<unsigned char>(reply.at(i)));
    }
    printf("\nRX Data  In Text: ");
    for ( unsigned int i= 0; i< reply.size(); i++ )
    {
        if(isprint(reply.at(i)))
        {
            std::cout << reply[i];
        }
        else
        {
            std::cout << ".";
        }
    }
    printf("\n");
}

void *LCC_PUMP_CLIENT::clientManager(void *ptr)
{
    LCC_PUMP_CLIENT &client = *reinterpret_cast <LCC_PUMP_CLIENT *> (ptr);

    //receive and echo reply
    while (true)
    {
        if(!client.clientConnected())
        {
            client.conn();
        }
        sleep(10);
    }
}

void *LCC_PUMP_CLIENT::rxMain(void *ptr)
{
    LCC_PUMP_CLIENT &client = *reinterpret_cast<LCC_PUMP_CLIENT *>(ptr);

    //receive and echo reply
    while(true)
    {
        if(client.clientConnected())
        {
            client.receive();
        }
    }
}

bool LCC_PUMP_CLIENT::getCfgData(const char *filename)
{
    bool configLoaded = false;

    if(filename != NULL)
    {
        LoadConfig loadCfg(filename);

        int paramLen = sizeof(params) / sizeof(SosConfSet_T);

        if(loadCfg.load(params, paramLen))
        {
            configLoaded = true;
        }
    }

    return  configLoaded;
}

void LCC_PUMP_CLIENT::printConfig(GENERAL_COMMANDS &msg)
{
    CONFIGURATION_DATA_U  configData;

    for(size_t i=0; i < CONFIGURATION_DATA_S_LEN; i++)
    {
        configData.data[i] = ntohl(msg.data[i]);
    }

    printf("\nLoop1Flow=%d\n",configData.config_data_u.Loop1Flow);
    printf("Loop2Flow=%d\n",configData.config_data_u.Loop2Flow);
    printf("stopLoopOnLowFlow=%d\n",configData.config_data_u.stopLoopOnLowFlow);
    printf("pumpPurgeTimeInSeconds=%d\n",configData.config_data_u.pumpPurgeTimeInSeconds);
    printf("shutdownLinePerRack=%d\n",configData.config_data_u.shutdownLinePerRack);
    printf(" nightModeOn=%d\n",configData.config_data_u.nightModeOn);
    printf(" AirVentingProcedureOn=%d\n",configData.config_data_u.AirVentingProcedureOn);
    printf(" AirVentingPowerOn=%d\n",configData.config_data_u.AirVentingPowerOn);
    printf(" stopLoopOnRemoteOnOff_1=%d\n",configData.config_data_u.stopLoopOnRemoteOnOff_1);
    printf(" stopLoopOnRemoteOnOff_2=%d\n",configData.config_data_u.stopLoopOnRemoteOnOff_2);
    printf(" stopOnLeak=%d\n",configData.config_data_u.stopOnLeak);
    printf(" loop1Active=%d\n",configData.config_data_u.loop1Active);
    printf(" stopLoopOnAccessoryShutDown_1=%d\n",configData.config_data_u.stopLoopOnAccessoryShutDown_1);
    printf(" stopLoopOnExchagePressure_1=%d\n",configData.config_data_u.stopLoopOnExchagePressure_1);
    printf(" stopLoopOnfilterPressureSnsr_1=%d\n",configData.config_data_u.stopLoopOnfilterPressureSnsr_1);
    printf(" loop2Active=%d\n",configData.config_data_u.loop2Active);
    printf(" stopLoopOnAccessoryShutDown_2=%d\n",configData.config_data_u.stopLoopOnAccessoryShutDown_2);
    printf(" stopLoopOnExchagePressure_2=%d\n",configData.config_data_u.stopLoopOnExchagePressure_2);
    printf(" stopLoopOnfilterPressureSnsr_2=%d\n",configData.config_data_u.stopLoopOnfilterPressureSnsr_2);
    printf(" startLiquidToAirExchanger=%d\n\n",configData.config_data_u.startLiquidToAirExchanger);
}

void LCC_PUMP_CLIENT::sendCfgData(CONFIGURATION_DATA_S &cfgdata)
{
    GENERAL_COMMANDS msg;
    CONFIGURATION_DATA_U  configData;
    configData.config_data_u = cfgdata;
    msg.commandID = htonl(TLV_SYSTEM_CONGIURATION_SETUP);
    std::uint32_t dataLen = sizeof(CONFIGURATION_DATA_S);
    msg.commandSize = htonl(dataLen);
    // unsigned int *p = reinterpret_cast  <unsigned int *> (& (cfgdata.Loop1Flow));
    for (std::size_t i=0; i < CONFIGURATION_DATA_S_LEN; i++)
    {
        msg.data[i] = htonl( configData.data[i]);
        std::cout << " Data at " << i << " ---->" << configData.data[i] << std::endl;
    }
    if(isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
    }
}

//
//

int main(int argc , char *argv[])
{
    char  *host = NULL;
    std::uint16_t port = 0;
    // char *data = NULL;
    bool pumpCmd = false;
    bool pumpConfig = false;
    bool pumpConfigRead = false;
    char *cmd = NULL;
    char *filename = NULL;
    int  opt;
    int  loop =3; // all CDU

    while ((opt = getopt(argc, argv, "Gh:p:d:O:C:S:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            host = optarg;
            break;

        case 'p':
            port = static_cast<std::uint16_t>(atoi(optarg));
            break;

        // case 'd':
        //     data = optarg;
        //     break;

        case 'O':
           pumpCmd = true;
           cmd = optarg;
            break;

        case 'S':
            loop = atoi(optarg);
            break;

        case 'C':
            pumpConfig = true;
            filename = optarg;
            break;

        case 'G':
            pumpConfigRead = true;
            filename = optarg;
            break;
        default: /* '?' */
            printf( " %s  Usage: -h hostIP  -p port -O [""ON"", ""OFF""] -S [1=""Loop1"",2""Loop2"" or 3=""CDU""] on or OFF \n",  argv[0]);
            printf( " %s  Usage: -h hostIP  -p port -C configfile\n",  argv[0]);
            exit(1);
        }
    }

    if((host == NULL ) || (port == 0))
    {
        printf( " %s  Usage: -h hostIP  -p port -O [""ON"" ,""OFF""] --turn Pump on or OFF \n",  argv[0]);
        printf( " %s  Usage: -h hostIP  -p port -C configfile -- Configure the pump\n",  argv[0]);
        exit(EXIT_FAILURE);
    }

    std::string hostIP{host};
    //connect to host
    LCC_PUMP_CLIENT client(hostIP, port);
    client.conn();

    if(!client.clientConnected())
    {
        client.conn();
    }

    pthread_create (&LCC_PUMP_CLIENT::rxThread, NULL,  LCC_PUMP_CLIENT::rxMain, reinterpret_cast<void *>(&client));
    pthread_create (&LCC_PUMP_CLIENT::socketThread, NULL,  LCC_PUMP_CLIENT::clientManager, reinterpret_cast<void *>(&client));

    while(!client.clientConnected())
    {
        sleep(1);
        client.conn();
    }

    if(pumpCmd)
    {
        std::string command{cmd};
        if(command == "ON")
        {
            printf("Sending Pump On CMD \n");
            client.sendPumpOn(loop);
        }
        else if(command == "OFF")
        {
            printf("Sending Pump OFF CMD \n");
            client.sendPumpOff(loop);
        }
    }

    if(pumpConfig)
    {
        printf("Loading Configuration from File %s  \n", filename);
        if(client.getCfgData(filename) == 0)
        {
            client.sendCfgData(ConfigData);
        }
        else
        {
            printf("!!!!!!!!!!!!!!!!! Loading configuration failed !!!!\n");
        }
    }

    if(pumpConfigRead)
    {
        client.sendGetConfig();
    }
    pthread_join( LCC_PUMP_CLIENT::rxThread, NULL);
    pthread_join(LCC_PUMP_CLIENT::socketThread, NULL);

    client.close();
    //done
    return 0;
}
