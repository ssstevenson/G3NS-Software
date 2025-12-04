
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
#include <logger/logger.h>
#include "envMgrLCDUclient.h"
#include <sstream>
#include <string>



using namespace empower;

pthread_t LCC_PUMP_CLIENT::rxThread  = 0;
pthread_t LCC_PUMP_CLIENT::socketThread = 0;



CONFIGURATION_DATA_S  ConfigData;
SosConfSet_T  params[]=
{
    {"LOOP1_FLOW_RATE"                          , TYPE_NUMBER    ,      &ConfigData.Loop1Flow},
    {"LOOP2_FLOW_RATE"                          , TYPE_NUMBER    ,      &ConfigData.Loop2Flow},
    {"STOP_LOOP_ON_LOW_FLOW"                    , TYPE_BOOLEAN   ,      &ConfigData.stopLoopOnLowFlow},
    {"PUMP_PURGE_TIME_SECONDS"                  , TYPE_NUMBER    ,      &ConfigData.pumpPurgeTimeInSeconds},
    //
    {"SHUTDOWN_LINE_PER_RACK"                   , TYPE_BOOLEAN    ,     &ConfigData.shutdownLinePerRack},
    //
    {"NIGHT_MODE_ENABLED"                       , TYPE_BOOLEAN   ,      &ConfigData.nightModeOn},
    {"AIRVENTING_PROC_ON"                       , TYPE_BOOLEAN   ,      &ConfigData.AirVentingProcedureOn},
    {"AIRVENTING_ON_POWER"                      , TYPE_BOOLEAN   ,      &ConfigData.AirVentingPowerOn},
    //

    {"STOP_LOOP_ON_REMOTE_OFF_1"                 , TYPE_BOOLEAN   ,      &ConfigData.stopLoopOnRemoteOnOff_1},
    {"STOP_LOOP_ON_REMOTE_OFF_2"                 , TYPE_BOOLEAN   ,      &ConfigData.stopLoopOnRemoteOnOff_2},
    //
    {"STOP_ON_LEAK"                              , TYPE_BOOLEAN   ,     &ConfigData.stopOnLeak},
    //
    // LOOP 1
    //
    {"LOOP1_ACTIVE"                              , TYPE_BOOLEAN   ,      &ConfigData.loop1Active},
    {"STOP_LOOP_ON_ACCESSORY_SHUTDOWN_1"         , TYPE_BOOLEAN   ,      &ConfigData.stopLoopOnAccessoryShutDown_1},
    {"STOP_LOOP_ON_EXCHANGE_PRESSURE_1"          , TYPE_BOOLEAN   ,      &ConfigData.stopLoopOnExchagePressure_1},
    {"STOP_LOOP_ON_FILTER_PRESSURE_1"            , TYPE_BOOLEAN   ,      &ConfigData.stopLoopOnfilterPressureSnsr_1},
    //
    // LOOP 2
    //
    {"LOOP2_ACTIVE"                              , TYPE_BOOLEAN   ,      &ConfigData.loop2Active},
    {"STOP_LOOP_ON_ACCESSORY_SHUTDOWN_2"         , TYPE_BOOLEAN   ,      &ConfigData.stopLoopOnAccessoryShutDown_2},
    {"STOP_LOOP_ON_EXCHANGE_PRESSURE_2"          , TYPE_BOOLEAN   ,      &ConfigData.stopLoopOnExchagePressure_2},
    {"STOP_LOOP_ON_FILTER_PRESSURE_2"            , TYPE_BOOLEAN   ,      &ConfigData.stopLoopOnfilterPressureSnsr_2},
    //
    // HEAT EXCHANGER DATA
    //
    {"LIQUID_TO_AIR_EXCHANGE_PRESENT"            , TYPE_BOOLEAN   ,      &ConfigData.startLiquidToAirExchanger},
    {"SKIP_PUMP_REGULATION_AFTER_START"          , TYPE_BOOLEAN   ,      &ConfigData.skipPumpRegulation},
    {"COLD_START_ENABLED"                        , TYPE_BOOLEAN   ,      &ConfigData.coldStartEnabled},
    {"COLD_START_TRIGGER_TEMP"                   , TYPE_NUMBER    ,      &ConfigData.coldStartTriggerTemp},
    {"COLD_START_PREHEAT_TIME_SEC"               , TYPE_NUMBER    ,      &ConfigData.coldStartPreHeaTime},
    {"FLUSH_COOLANT_PREHEAT_TIME_SEC"            , TYPE_NUMBER    ,      &ConfigData.flushCoolantPreHeaTime},
    {"BOOSTER_PREHEAT_TIME_SEC"                  , TYPE_NUMBER    ,      &ConfigData.boosterPreHeaTime},
    {"PRE_HEAT_DELAY_SEC"                        , TYPE_NUMBER    ,      &ConfigData.PreHeatDelay},
    {"INITIAL_WARMUP_TEMP"                       , TYPE_NUMBER    ,      &ConfigData.intialWarmupTemp},
    {"PUMP_TEMP_OPER_LIMIT"                      , TYPE_NUMBER    ,      &ConfigData.pumpOperTempLimit},
	//
	//  WORK AROUND FLOW SENSORS
	//
	{"LOOP_VALID_MIN_FLOW"                      ,TYPE_NUMBER     ,      &ConfigData.validFlowMin},
	{"LOOP_VALID_MAX_FLOW"                      ,TYPE_NUMBER     ,      &ConfigData.validFlowMax},
	{"LOOP1_FORCE_VOLTAGE"                      ,TYPE_NUMBER     ,      &ConfigData.loop1ForceVoltage},
	{"LOOP2_FORCE_VOLTAGE"                      ,TYPE_NUMBER     ,      &ConfigData.loop2ForceVoltage},
    {""                                          ,      0            ,        0                       }
};


unsigned int validFlowMin;
unsigned int validFlowMax;
unsigned int loop1ForceVoltage;
unsigned int loop2ForceVoltage;

LCC_PUMP_CLIENT::LCC_PUMP_CLIENT(string srvaddress , uint16_t srvport):
    sock(-1),
    address(srvaddress),
    port(srvport),
    isconnected(false),
    server(),
    StatusAvailable(false),
    ModBusStatusAvailable(false),
    gSystemStatusData(),
    gModBusPumpData(),
    gSystemPumpON(false),
    Loop1PumpON(false),
    Loop2PumpON(false),
    connect_error_cnt(1),
    loop_cnt(0),
    info_cnt(0),
    alarm_cnt(0)
{
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        logger::info(__FILE__, __FUNCTION__, "cduClient: Failed to create socket, Exiting......");
        perror("Could not create socket");
    }
}


void LCC_PUMP_CLIENT::renewSock()
{
    ::close(sock);
    sock = -1;
    isconnected = false;
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (LCC_PUMP_CLIENT::sock == -1)
    {
        logger::info(__FILE__, __FUNCTION__, "cduClient: Failed to create NEW socket, Exiting......");
//        perror("Could not create socket");
        return;
    }

    connect_error_cnt = 1;
    logger::info(__FILE__, __FUNCTION__, " ===>>> cduClient: Success!! created NEW socket.");

}


/**
    Connect to a host on a certain port number
*/
bool LCC_PUMP_CLIENT::conn()
{
    bool    ret = false;
    long    arg;


    if (sock > 0 )
    {
//        cout << "\n\n=== >> Attempting connection to IP " << address << endl;
        //setup address structure
        if(inet_addr(address.c_str()) !=  inet_addr("255.255.255.255") )
        {
            struct hostent *he;
            struct in_addr **addr_list;

            //resolve the hostname, its not an ip address
            if ( (he = gethostbyname( address.c_str() ) ) == NULL)
            {
                //gethostbyname failed
                herror("gethostbyname");
                logger::info(__FILE__, __FUNCTION__, "cduClient: Failed to resolve hostname, Exiting......");
//                cout << "Failed to resolve hostname\n";
                connect_error_cnt++;
                return false;
            }

            //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
//            addr_list = (struct in_addr **) he->h_addr_list;
            addr_list = reinterpret_cast<struct in_addr **>(he->h_addr_list);

            if( addr_list[0] != NULL )
            {
                //strcpy(ip , inet_ntoa(*addr_list[i]) );
                server.sin_addr = *addr_list[0];
//                cout<<address<<" resolved to "<<inet_ntoa(*addr_list[0])<<endl;
            }

//            for(int i = 0; addr_list[i] != NULL; i++)
//            {
//                //strcpy(ip , inet_ntoa(*addr_list[i]) );
//                server.sin_addr = *addr_list[i];
//                cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i])<<endl;
//
//                break;
//            }
        }
        else
        {
            server.sin_addr.s_addr = inet_addr( address.c_str() );
        }

        server.sin_family = AF_INET;
        server.sin_port = htons( port );

        // Set non-blocking
        if( (arg = fcntl(sock, F_GETFL, NULL)) < 0) {
           fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
           exit(0);
        }
        arg |= O_NONBLOCK;
        if( fcntl(sock, F_SETFL, arg) < 0) {
           fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
           exit(0);
        }

//        cout << "ATTEMPTING TO CONNECT (NON-BLOCKING MODE ENABLED...." << endl;
        //Connect to remote server
        if (connect(sock , reinterpret_cast<struct sockaddr *>(&server) , sizeof(server)) < 0)
        {
            connect_error_cnt++;
            std::string logmsg = "cduClient: connect failed (cnt: " + std::to_string(connect_error_cnt) + " ), exiting......";
            logger::warn(__FILE__, __FUNCTION__, logmsg);
//            perror("connect failed. Error");
        }
        else 
        {
            isconnected = true;
            ret = true;
            connect_error_cnt = 0;
            logger::info(__FILE__, __FUNCTION__, "cduClient connection established ...");

//            cout<<"Connected.\n";
        }
    }
 
    return ret;
}

unsigned int LCC_PUMP_CLIENT::getConnectErrorCnt()
{
    return connect_error_cnt;
}


/**
    Send data to the connected host
*/
void LCC_PUMP_CLIENT::sendPumpOn()
{
    GENERAL_COMMANDS msg;

    msg.commandID = htonl(TLV_SET_PUMPS_ON);
    if (isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
        gSystemPumpON = true;
    }
}

void LCC_PUMP_CLIENT::sendLoop1On()
{
    GENERAL_COMMANDS msg;

    msg.commandID = htonl(TLV_SET_LOOP1_ON);
    if (isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
        Loop1PumpON = true;
    }
}

void LCC_PUMP_CLIENT::sendLoop2On()
{
    GENERAL_COMMANDS msg;

    msg.commandID = htonl(TLV_SET_LOOP2_ON);
    if (isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
        Loop2PumpON = true;
    }
}

void LCC_PUMP_CLIENT::sendPumpOff()
{
    GENERAL_COMMANDS msg;

    msg.commandID = htonl(TLV_SET_PUMPS_OFF);
    if (isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
        gSystemPumpON = false;
    }
}


void LCC_PUMP_CLIENT::sendLoop1Off()
{
    GENERAL_COMMANDS msg;

    msg.commandID = htonl(TLV_SET_LOOP1_OFF);
    if (isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
        Loop1PumpON = false;
    }
}


void LCC_PUMP_CLIENT::sendLoop2Off()
{
    GENERAL_COMMANDS msg;

    msg.commandID = htonl(TLV_SET_LOOP2_OFF);
    if (isconnected)
    {
        send_data(&msg, sizeof(GENERAL_COMMANDS));
        Loop2PumpON = false;
    }
}


void LCC_PUMP_CLIENT::sendGetSystemInfo()
{
    GENERAL_COMMANDS msg;

    msg.commandID = htonl(TLV_GET_STATUS_INFORMATION);
    if (isconnected)
        send_data(&msg, sizeof(GENERAL_COMMANDS));
}

void LCC_PUMP_CLIENT::sendGetConfig()
{
    GENERAL_COMMANDS msg;

    msg.commandID = htonl(TLV_SYSTEM_CONGIURATION_READ);
    if (isconnected)
        send_data(&msg, sizeof(GENERAL_COMMANDS));
}

bool LCC_PUMP_CLIENT::send_data(void *data, unsigned int len)
{
    //Send some data
    if( send(sock , data , len , 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
//    cout<<"Data sent!\n";

    return true;
}


 void  LCC_PUMP_CLIENT::processMessage(GENERAL_COMMANDS &msg)
{
    unsigned int            commandId = ntohl(msg.commandID);
//    unsigned int            msgSize   = ntohl(msg.commandSize);
    unsigned int            id = ntohl(msg.data[0]);

    // SSS Vars
    SYSTEM_REPORT_DATA_S    systemStatusData = {};
    MODBUS_REPORT_DATA_S    modbusPumpData = {};

    stringstream            ss;

    std::string             alarm_msg;



//    printf("\n\n %s() TCP  Message received--[ msg size=%d -- cmd=0x%0x (0x%0x)] ---\n",__FUNCTION__, msgSize, commandId, msg.commandID );
//    printf(" %s() TCP  Message received--[ raw msg size=%d -- raw cmd= 0x%0x] ---\n\n",__FUNCTION__, msg.commandSize, msg.commandID);

    systemStatusData = gSystemStatusData;
    setStatusAvailable(false);
    ModBusStatusAvailable = false;
    systemStatusData.ALARM_STATE.ALARM_UPDATE = false;


#define LOOP_CNT_MAX        10

    if( loop_cnt >= (LOOP_CNT_MAX-1) )
    {
        // clear alarm state
        systemStatusData.ALARM_STATE = {};
        if(0 == alarm_cnt)
            gSystemStatusData.ALARM_STATE = {};
    }

    systemStatusData.ALARM_STATE.ALARM_UPDATE = false;


    switch(commandId)
    {
        case TLV_PUMP_LEAK:
            /// ENV_CDU_STATUS_PUMP_LEAK
            systemStatusData.ALARM_STATE.PUMP_LEAK = true;
            alarm_cnt++;
            setStatusAvailable(true);
            logger::critical(__FILE__, __FUNCTION__, "*** SYSTEM CDU ALARM : PUMP_LEAK  Event detected!!! ");
//            cout << " PUMP_LEAK  Event detected...System shutting down " << endl;
            break;

        case TLV_PUMP_PHASE_ERROR:
            /// ENV_CDU_STATUS_PUMP_PHASE_ERROR
            systemStatusData.ALARM_STATE.PHASE_ERROR = true;
            alarm_cnt++;
            setStatusAvailable(true);
            logger::critical(__FILE__, __FUNCTION__, "*** SYSTEM CDU ALARM : PUMP_PHASE_ERROR  Event detected!!! ");
//            cout << " PUMP_PHASE_ERROR Event detected...System shutting down " << endl;
            break;
        case TLV_SYSTEM_EMERGENCY_STOP:
            /// ENV_CDU_STATUS_SYSTEM_EMERGENCY_STOP
            systemStatusData.ALARM_STATE.EMERGENCY_STOP = true;
            alarm_cnt++;
            setStatusAvailable(true);
            logger::critical(__FILE__, __FUNCTION__, "*** SYSTEM CDU ALARM : SYSTEM_EMERGENCY_STOP  Event detected!!! >>> System shutting down... ");
//            cout << " SYSTEM_EMERGENCY_STOP Event detected...System shutting down " << endl;
            break;
        case TLV_SYSTEM_ACCESSORY_SHUTDOWN:
            /// ENV_CDU_STATUS_SYSTEM_ACCESSORY_SHUTDOWN
            systemStatusData.ALARM_STATE.ACC1_SHUTDOWN = true;
            alarm_cnt++;
            setStatusAvailable(true);
            logger::critical(__FILE__, __FUNCTION__, "*** SYSTEM CDU ALARM : SYSTEM_ACCESSORY_SHUTDOWN 1  Event detected!!! >>> System shutting down... ");
//            cout << " SYSTEM_ACCESSORY_SHUTDOWN 1  Event detected...System shutting down " << endl;
            break;
        case TLV_SYSTEM_ACCESSORY_2_SHUTDOWN:
            /// ENV_CDU_STATUS_SYSTEM_ACCESSORY_2_SHUTDOWN
            systemStatusData.ALARM_STATE.ACC2_SHUTDOWN = true;
            alarm_cnt++;
            setStatusAvailable(true);
            logger::critical(__FILE__, __FUNCTION__, "*** SYSTEM CDU ALARM : SYSTEM_ACCESSORY_SHUTDOWN 2  Event detected!!! >>> System shutting down... ");
//            cout << " SYSTEM_ACCESSORY_SHUTDOWN 2  Event detected...System shutting down " << endl;
            break;
        case TLV_SYSTEM_REMOTE_ON_OFF:
            /// ENV_CDU_STATUS_SYSTEM_REMOTE_ON_OFF
            systemStatusData.ALARM_STATE.REMOTE_ON_OFF = true;
            alarm_cnt++;
            setStatusAvailable(true);
            logger::critical(__FILE__, __FUNCTION__, "*** SYSTEM CDU ALARM : SYSTEM_ACCESSORY_SHUTDOWN 2  Event detected!!! >>> System shutting down... ");
//            cout << " SYSTEM_REMOTE_ON_OFF  Event detected...System shutting down " << endl;
            break;
        case TLV_SYSTEM_EXCHANGE_PRESSURE:
            /// ENV_CDU_STATUS_SYSTEM_EXCHANGE_OVERPRESSURE
            switch(id)
            {
                case 1:
                    systemStatusData.ALARM_STATE.LOOP1_EXCHNG_PRES = true;
                    break;
                case 2:
                    systemStatusData.ALARM_STATE.LOOP2_EXCHNG_PRES = true;
                    break;
            }
            alarm_cnt++;
            setStatusAvailable(true);
            alarm_msg = "*** SYSTEM CDU ALARM : SYSTEM_EXCHANGE_PRESSURE LOOP " + std::to_string(id) + " Event detected!!!";
            logger::critical(__FILE__, __FUNCTION__, alarm_msg);
//            cout << " SYSTEM_EXCHANGE_PRESSURE Event detected for loop" << id<< "...System shutting down " << endl;
            break;
        case TLV_SYSTEM_FILTER_PRESSURE:
            /// ENV_CDU_STATUS_SYSTEM_FILTER_OVERPRESSURE
            switch(id)
            {
                case 1:
                    systemStatusData.ALARM_STATE.LOOP1_FILTER_PRES = true;
                    break;
                case 2:
                    systemStatusData.ALARM_STATE.LOOP2_FILTER_PRES = true;
                    break;
            }
            alarm_cnt++;
            setStatusAvailable(true);
            alarm_msg = "*** SYSTEM CDU ALARM : SYSTEM_FILTER_PRESSURE LOOP " + std::to_string(id) + " Event detected!!!";
            logger::critical(__FILE__, __FUNCTION__, alarm_msg);
//            cout << " SYSTEM_FILTER_PRESSURE Event detected for loop " << id<< "...System shutting down " << endl;
            break;
        case TLV_SYSTEM_RESERVOIR_FULL:
            /// ENV_CDU_STATUS_SYSTEM_RESERVOIR_FULL
            switch(id)
            {
                case 1:
                    systemStatusData.ALARM_STATE.RESERVOIR1_FULL = true;
                    break;
                case 2:
                    systemStatusData.ALARM_STATE.RESERVOIR2_FULL = true;
                    break;
            }
            alarm_cnt++;
            setStatusAvailable(true);
            alarm_msg = "*** SYSTEM CDU ALARM : SYSTEM_RESERVOIR_FULL Reservoir " + std::to_string(id) + " Event detected!!!";
            logger::critical(__FILE__, __FUNCTION__, alarm_msg);
//            cout << " SYSTEM_RESERVOIR_FULL Event detected for Reservoir " << id<< "...System shutting down " << endl;
            break;
        case TLV_SYSTEM_RESERVOIR_LOW:
            /// ENV_CDU_STATUS_SYSTEM_RESERVOIR_LOW
            switch(id)
            {
                case 1:
                    systemStatusData.ALARM_STATE.RESERVOIR1_LOW = true;
                    break;
                case 2:
                    systemStatusData.ALARM_STATE.RESERVOIR2_LOW = true;
                    break;
            }
            alarm_cnt++;
            setStatusAvailable(true);
            alarm_msg = "*** SYSTEM CDU ALARM : SYSTEM_RESERVOIR_LOW Reservoir " + std::to_string(id) + " Event detected!!!";
            logger::critical(__FILE__, __FUNCTION__, alarm_msg);
//            cout << " SYSTEM_RESERVOIR_LOW Event detected for reservoir " << id<< "...System shutting down " << endl;
            break;
        case TLV_PUMP_ALARM:
            /// ENV_CDU_STATUS_PUMP_ALARM
            switch(id)
            {
                case 1:
                    systemStatusData.ALARM_STATE.PUMP1_ALARM = true;
                    break;
                case 2:
                    systemStatusData.ALARM_STATE.PUMP2_ALARM = true;
                    break;
                case 3:
                    systemStatusData.ALARM_STATE.PUMP3_ALARM = true;
                    break;
                case 4:
                    systemStatusData.ALARM_STATE.PUMP4_ALARM = true;
                    break;
            }
            alarm_cnt++;
            setStatusAvailable(true);
            alarm_msg = "*** SYSTEM CDU ALARM : PUMP_ALARM - Pump " + std::to_string(id) + " Event detected!!!";
            logger::critical(__FILE__, __FUNCTION__, alarm_msg);
//            cout << " PUMP_ALARM Event detected for PUMP =" << id<< "...System shutting down " << endl;
            break;

        case TLV_PUMP_STATUS:
        {
            /// ENV_CDU_PUMP_1_FLOW
            /// ENV_CDU_PUMP_1_TEMP
            /// ENV_CDU_PUMP_2_FLOW
            /// ENV_CDU_PUMP_2_TEMP
            /// ENV_CDU_PUMP_3_FLOW
            /// ENV_CDU_PUMP_3_TEMP
            /// ENV_CDU_PUMP_4_FLOW
            /// ENV_CDU_PUMP_4_TEMP
            PUMP_INFO_S  pumpInfo;
            pumpInfo.pumpID      = ntohl(msg.data[0]);
            pumpInfo.pumpFlowRaw = ntohl(msg.data[1]);
            pumpInfo.pumpFlow    = static_cast<float>( (ntohl(msg.data[2])) );
            pumpInfo.pumpTempRaw = ntohl(msg.data[3]);
            pumpInfo.pumpTemp    = static_cast<float>( (ntohl(msg.data[4])) );

            systemStatusData.PUMP_INFO[pumpInfo.pumpID-1].pumpID      = pumpInfo.pumpID     ;
            systemStatusData.PUMP_INFO[pumpInfo.pumpID-1].pumpFlowRaw = pumpInfo.pumpFlowRaw;
            systemStatusData.PUMP_INFO[pumpInfo.pumpID-1].pumpFlow    = pumpInfo.pumpFlow   ;
            systemStatusData.PUMP_INFO[pumpInfo.pumpID-1].pumpTempRaw = pumpInfo.pumpTempRaw;
            systemStatusData.PUMP_INFO[pumpInfo.pumpID-1].pumpTemp    = pumpInfo.pumpTemp   ;
            systemStatusData.PUMP_INFO_AVAIL[pumpInfo.pumpID-1]       = true                ;
            systemStatusData.ALARM_STATE.ALARM_UPDATE = false;
            setStatusAvailable(true);
            info_cnt++;


//            cout << " PUMP " <<  pumpInfo.pumpID << " FLOW [raw:"<< pumpInfo.pumpFlowRaw << ",val:"<<  pumpInfo.pumpFlow << "]"<< endl;
//            cout << " PUMP " <<  pumpInfo.pumpID << " TEMP [raw:"<< pumpInfo.pumpTempRaw << ",val:"<<  pumpInfo.pumpTemp << "]"<< endl;
            break;
        }
        case TLV_LOOP_STATUS:
        {
            /// ENV_CDU_LOOP_1_PRESSURE
            /// ENV_CDU_LOOP_1_TEMP
            /// ENV_CDU_LOOP_2_PRESSURE
            /// ENV_CDU_LOOP_2_TEMP
            PUMP_LOOP_S  loopInfo;
            loopInfo.loopID          = ntohl(msg.data[0]);
            loopInfo.loopPressureRaw = ntohl(msg.data[1]);

            double tmp = static_cast<double>(loopInfo.loopPressureRaw);
            tmp = ( tmp * 0.04);
            loopInfo.loopPressure    = static_cast<float>( tmp );

//            loopInfo.loopTempRaw     = ntohl(msg.data[3]);
//            loopInfo.loopTemp        = static_cast<float>( (ntohl(msg.data[4])) );

            systemStatusData.LOOP_INFO[loopInfo.loopID-1].loopID          = loopInfo.loopID         ;
            systemStatusData.LOOP_INFO[loopInfo.loopID-1].loopPressureRaw = loopInfo.loopPressureRaw;
            systemStatusData.LOOP_INFO[loopInfo.loopID-1].loopPressure    = loopInfo.loopPressure   ;
//            systemStatusData.LOOP_INFO[loopInfo.loopID-1].loopTempRaw     = loopInfo.loopTempRaw    ;
//            systemStatusData.LOOP_INFO[loopInfo.loopID-1].loopTemp        = loopInfo.loopTemp       ;
            systemStatusData.LOOP_INFO_AVAIL[loopInfo.loopID-1]           = true                    ;
            systemStatusData.ALARM_STATE.ALARM_UPDATE = false;
            setStatusAvailable(true);
            info_cnt++;


//            cout << "\n\n===>>> Loop " <<  loopInfo.loopID   << " Pressure [raw:"<< loopInfo.loopPressureRaw << ",val:"<<  loopInfo.loopPressure << "]\n\n"<< endl;
//            cout << "\n\n===>>> Loop " <<  loopInfo.loopID   << " TEMP [raw:"<< loopInfo.loopTempRaw << ",val:"<<  loopInfo.loopTemp  << "]"<< endl;
            break;
        }

        case TLV_RESERVOIR_STATUS:
        {
            /// ENV_CDU_RESERVOIR_1_TEMP
            /// ENV_CDU_RESERVOIR_2_TEMP
            RESERVOIR_TEMP_S  reservoir1, reservoir2;
            reservoir1.reservoirId      = ntohl(msg.data[0]);
            reservoir1.reservoirTempRaw = ntohl(msg.data[1]);
            reservoir1.reservoirTemp    = static_cast<float>( (ntohl(msg.data[2])) );
            reservoir2.reservoirId      = ntohl(msg.data[3]);
            reservoir2.reservoirTempRaw = ntohl(msg.data[4]);
            reservoir2.reservoirTemp    = static_cast<float>( ntohl(msg.data[5]) );

            systemStatusData.RESV_TEMP[0].reservoirId      = reservoir1.reservoirId     ;
            systemStatusData.RESV_TEMP[0].reservoirTempRaw = reservoir1.reservoirTempRaw;
            systemStatusData.RESV_TEMP[0].reservoirTemp    = reservoir1.reservoirTemp   ;
            systemStatusData.RESV_TEMP[1].reservoirId      = reservoir2.reservoirId     ;
            systemStatusData.RESV_TEMP[1].reservoirTempRaw = reservoir2.reservoirTempRaw;
            systemStatusData.RESV_TEMP[1].reservoirTemp    = reservoir2.reservoirTemp   ;
            systemStatusData.ALARM_STATE.ALARM_UPDATE = false;
            setStatusAvailable(true);
            info_cnt++;


//            cout << " RESERVOIR 1 ID: " <<  reservoir1.reservoirId   << " TEMP [raw: "<< reservoir1.reservoirTempRaw << ",val: "<<  reservoir1.reservoirTemp  << "]"<< endl;
//            cout << " RESERVOIR 2 ID: " <<  reservoir2.reservoirId   << " TEMP [raw: "<< reservoir2.reservoirTempRaw << ",val: "<<  reservoir2.reservoirTemp  << "]"<< endl;
            break;
        }

        ///
        case TLV_SYSTEM_MODBUS_INFO:
        {
            int i;
            GENERAL_MODBUS_MSG_S  *modBusMsg = reinterpret_cast<GENERAL_MODBUS_MSG_S*>(&msg);
            PumpModBusInfo_S_U *mobBusInfo_u = reinterpret_cast<PumpModBusInfo_S_U*>(&modBusMsg->modBusInfo);
            PumpModBusInfo_S *mobBusInfo    = &(mobBusInfo_u->PumpModBusInfoS);

            /// SSS ADDIN : data struct to pass pump info as string
            info_cnt++;
            setModBusStatusAvailable(true);
            systemStatusData.ALARM_STATE.ALARM_UPDATE = false;

            //
            for (i= 0; i < static_cast<int>( PMOD_INFO_MSG_SIZE ); i++)
            {
                mobBusInfo_u->data[i] = static_cast<unsigned short>( ntohl(msg.data[i]));

#ifdef DEBUG_MODBUS_NET
                printf("[%s] --- Modbus Data Index %d [ net val =%x ]\n",__FUNCTION__, i, msg.data[i]  );
#endif
            }


//            cout << "\n\n\n MODBUS INFO for Pump: " << mobBusInfo->pumpID << endl;
            // Configuration
//            cout << "   Configuration: \n" << endl;

            // Operation Mode:
            modbusPumpData.OperMode = ( mobBusInfo->PumpCFG.OperMode != E_PUMP_OPER_MODE_OFF );
//            cout << "-------Operation Mode: " << ((  modbusPumpData.OperMode )? "ON": "OFF" ) << endl;
//            cout << "-------Operation Mode Value: " << mobBusInfo->PumpCFG.OperMode << endl;


            // Control Mode:
            std::string ctrlMode;
            if (mobBusInfo->PumpCFG.ctrlMode < E_PUMP_MAX_CTRL_MODE )
            {
                modbusPumpData.ctrlMode = mobBusInfo->PumpCFG.ctrlMode;

//                cout << "-------Control Mode: " << PUMP_CTRL_MODE_S[mobBusInfo->PumpCFG.ctrlMode]  << endl;
            }

            // Night Mode:
            modbusPumpData.nightMode = ( mobBusInfo->PumpCFG.nightMode != E_PUMP_NIGHT_MODE_NON_ACTIVE) ;
//            cout << "-------Night Mode: " << (( modbusPumpData.nightMode) ? "ACTIVE": "INACTIVE") << endl;

            // Air Venting Procedure:
            modbusPumpData.airVentProcedure = mobBusInfo->PumpCFG.airVentingProc;
//            cout << "-------Air Venting Procedure: " << (( modbusPumpData.airVentProcedure == E_PUMP_AV_PROCEDURE_NON_ACTIVE) ? "INACTIVE": "ACTIVE") << endl;


            // Proportional Pressure Setpoint
            float p = (static_cast<float>( mobBusInfo->PumpCFG.propPressureSetpoint))/100;
            modbusPumpData.propPressureSetpoint = p*HEAD_TO_PSI;
//            cout << "-------Proportional Pressure Setpoint : " << p << " [m] ---->"  << modbusPumpData.propPressureSetpoint << " [PSI]" <<endl;


            // Constant Pressure Setpoint
            p = (static_cast<float>(mobBusInfo->PumpCFG.constPressureSetpoint))/100;
            modbusPumpData.constPressureSetpoint = p*HEAD_TO_PSI;
//            cout << "-------Constant Pressure Setpoint : " << p << " [m] ---->"  << modbusPumpData.constPressureSetpoint << " [PSI]" <<endl;


            // Curve Pressure Setpoint
            modbusPumpData.curvePressureSetpoint = (static_cast<float>(mobBusInfo->PumpCFG.curvePressureSetpoint));
//            cout << "-------Curve Pressure Setpoint : " << modbusPumpData.curvePressureSetpoint<< " [ RPM]" << endl;

            // Air Vent Power On:
            modbusPumpData.airVentPowerOn =  mobBusInfo->PumpCFG.airVentPowerOn;
//            cout << "-------Air Vent Power On: " << ( (modbusPumpData.airVentPowerOn == E_PUMP_AV_POWER_ON_INACTIVE) ? "INACTIVE": "ACTIVE" ) << endl;

            //
//            cout << "   TWIN mode for Pump : " << mobBusInfo->pumpID +1 << endl;
            modbusPumpData.circularConfigMode = mobBusInfo->PumpTwinCFG.circularConfigMode;
            if (modbusPumpData.circularConfigMode <= E_PUMP_SINGLE)
            {
//                cout << "-------Twin Mode Circular : " << CIRCULAR_CONFIG_S[modbusPumpData.circularConfigMode] << endl;
            }

            modbusPumpData.twinCtrlMode = mobBusInfo->PumpTwinCFG.twinCtrlMode;
            if (mobBusInfo->PumpTwinCFG.twinCtrlMode <= E_PUMP_TWIN_FORCED_PARALLEL)
            {
//                cout << "-------Twin Control Mode  : " << TWIN_CRTL_CONFIG_S[mobBusInfo->PumpTwinCFG.twinCtrlMode] << endl;
            }

            //
//            cout << "   Status PumpId : " << mobBusInfo->pumpID + 1 << endl;
            modbusPumpData.InputPower =  mobBusInfo->PumpStatus.InputPower;
//            cout << "-------Input Power : " <<  dec << mobBusInfo->PumpStatus.InputPower << "[W]" << endl;


            float tmp = static_cast<float>(mobBusInfo->PumpStatus.Head); tmp /= 100;
            modbusPumpData.Head = tmp*HEAD_TO_PSI;
//            cout << "-------HEAD : " << tmp << " [m] ---->" << modbusPumpData.Head << " [PSI]" << endl;

            // Modbus Pump Flow GPM and Speed RPM
            tmp = static_cast<float>(mobBusInfo->PumpStatus.Flow); tmp /= 100;
//            modbusPumpData.Flow = tmp*LITREPERSEC_TO_GALPERMIN;
            modbusPumpData.Flow = tmp*CUBICMETER_TO_GALPERMIN;

//            cout << "-------FLOW : " << tmp << " [L/S] ---->" << tmp*CUBICMETER_TO_GALPERMIN << " [GPM]" << endl;
//            cout << "-------FLOW : " << tmp << " [L/S] ---->" << modbusPumpData.Flow << " [GPM]" << endl;

            modbusPumpData.Speed = mobBusInfo->PumpStatus.Speed;
 //           cout << "-------Speed : " << dec << mobBusInfo->PumpStatus.Speed << " [RPM]" << endl;

            // Modbus Water Temp deg C
            int itmp = static_cast<int>( (static_cast<float>(mobBusInfo->PumpStatus.WaterTemp))/10);
            modbusPumpData.WaterTemp = static_cast<float>(itmp);
//            cout << "-------Water temp : " << modbusPumpData.WaterTemp << " [Degree Celsius]" << endl;

            int exttmp = static_cast<int>( (static_cast<float>(mobBusInfo->PumpStatus.ExternalWaterTemp))/10);
            modbusPumpData.ExtwaterTemp = static_cast<float>(exttmp);
//            cout << "-------Water temp : " << modbusPumpData.WaterTemp << " [Degree Celsius]" << endl;


//             mobBusInfo->PumpStatus.WaterTemp;

               modbusPumpData.ExtwaterTemp     =  static_cast<float>(mobBusInfo->PumpStatus.ExternalWaterTemp/10);
               modbusPumpData.Winding1Temp     =  static_cast<int>(mobBusInfo->PumpStatus.Winding1Temp);
               modbusPumpData.Winding2Temp     =  static_cast<int>(mobBusInfo->PumpStatus.Winding2Temp);
               modbusPumpData.Winding3Temp     =  static_cast<int>(mobBusInfo->PumpStatus.Winding3Temp);
               modbusPumpData.PowerModuleTemp  =  static_cast<int>(mobBusInfo->PumpStatus.PowerModuleTemp);

               modbusPumpData.QuadrantCurrent  =  static_cast<unsigned int>(mobBusInfo->PumpStatus.QuadrantCurrent);
               modbusPumpData.BitFieldStatusIO =  static_cast<unsigned int>(mobBusInfo->PumpStatus.BitFieldStatusIO);
               modbusPumpData.BitFieldAlarm1   =  static_cast<unsigned int>(mobBusInfo->PumpStatus.BitFieldAlarm1);
               modbusPumpData.BitFieldAlarm2   =  static_cast<unsigned int>(mobBusInfo->PumpStatus.BitFieldAlarm2);

//            cout << "-------BIT FIELD ERRORS : " << hex << mobBusInfo->PumpStatus.BitFieldErrors << endl;
            modbusPumpData.BitFieldErrors = mobBusInfo->PumpStatus.BitFieldErrors;
            if (modbusPumpData.BitFieldErrors > 0 )
            {
//                cout << endl;
                unsigned short tmps = (modbusPumpData.BitFieldErrors & 0xFFFF);
                for (i=0; i< 15; i++ )
                {
                    if ( (tmps >> i ) & 0x01 )
                    {
//                       cout << " BIT "<< i << "------>" <<  PumpBitErrorrs[i] << endl;
                    }
                }
            }
            else
//                 cout << " NONE." << endl;
            modbusPumpData.ActiveErrorCode = mobBusInfo->PumpStatus.ActiveErrorCode;
            if (modbusPumpData.ActiveErrorCode < MAX_ACTIVE_ERROR_CODE)
            {
//                cout << "-------ACTIVE ERROR : " << PumpActiveErrors[modbusPumpData.ActiveErrorCode] << endl;
            }

            modbusPumpData.LifeTimerLSW = mobBusInfo->PumpLogCntrTable.lifeTimerLSW;
            modbusPumpData.LifeTimerMSW = mobBusInfo->PumpLogCntrTable.lifeTimerMSW;

            modbusPumpData.PowerConsumptionLSW_25  = mobBusInfo->PumpLogCntrTable.PowerConsumptionLSW_25;
            modbusPumpData.PowerConsumptionMSW_25  = mobBusInfo->PumpLogCntrTable.PowerConsumptionMSW_25;

            modbusPumpData.PowerConsumptionLSW_50  = mobBusInfo->PumpLogCntrTable.PowerConsumptionLSW_50;
            modbusPumpData.PowerConsumptionMSW_50  = mobBusInfo->PumpLogCntrTable.PowerConsumptionMSW_50;

            modbusPumpData.PowerConsumptionLSW_75  = mobBusInfo->PumpLogCntrTable.PowerConsumptionLSW_75;
            modbusPumpData.PowerConsumptionMSW_75  = mobBusInfo->PumpLogCntrTable.PowerConsumptionMSW_75;

            modbusPumpData.PowerConsumptionLSW_100 = mobBusInfo->PumpLogCntrTable.PowerConsumptionLSW_100;
            modbusPumpData.PowerConsumptionMSW_100 = mobBusInfo->PumpLogCntrTable.PowerConsumptionMSW_100;

            modbusPumpData.CurrentIDxLog = mobBusInfo->PumpLogCntrTable.currentIDxLog;


//            cout << "===========================LOG COUNTER =====================" << endl;
//            cout << "-------Life Timer : "              <<  mobBusInfo->PumpLogCntrTable.lifeTimerLSW           + (mobBusInfo->PumpLogCntrTable.lifeTimerMSW << 16) << endl;
//            cout << "-------Power Consumption 0-25  : " <<  mobBusInfo->PumpLogCntrTable.PowerConsumptionLSW_25  + (mobBusInfo->PumpLogCntrTable.PowerConsumptionMSW_25 << 16) << endl;
//            cout << "-------Power Consumption 25-50 : " <<  mobBusInfo->PumpLogCntrTable.PowerConsumptionLSW_50  + (mobBusInfo->PumpLogCntrTable.PowerConsumptionMSW_50 << 16) << endl;
//            cout << "-------Power Consumption 50-75 : " <<  mobBusInfo->PumpLogCntrTable.PowerConsumptionLSW_75  + (mobBusInfo->PumpLogCntrTable.PowerConsumptionLSW_75 << 16) << endl;
//            cout << "-------Power Consumption 50-75 : " <<  mobBusInfo->PumpLogCntrTable.PowerConsumptionLSW_100 + (mobBusInfo->PumpLogCntrTable.PowerConsumptionMSW_100 << 16) << endl;
//            cout << "-------Current Index Log       : " <<  mobBusInfo->PumpLogCntrTable.currentIDxLog << endl;
           
            

            ////////////////////////
//            cout << endl;
//            cout << endl;
//            cout << " ###########  MODBUS INFO for Twin Slave Pump " << mobBusInfo->pumpID  << "  ############"  << endl;
//            cout << "----- TWIN SLAVE DRIVEN CURVE : " <<  dec << mobBusInfo->SlavePumpStatus.TwinSlaveRPM << "[RPM]" << endl;
//            const char  *tmps = mobBusInfo->SlavePumpStatus.TwinSlaveStartStop ? "START" : " STOP";
//            cout << "----- TWIN SLAVE State : " << tmps <<  endl;
//            cout << "----- TWIN SLAVE Input Power : " << dec << mobBusInfo->SlavePumpStatus.TwinSlaveInputpower << "Watts"  <<  endl;
//            tmp = static_cast<float>(mobBusInfo->SlavePumpStatus.TwinSlaveHead); tmp /= 100;
//            cout << "----- TWIN SLAVE HEAD : " << tmp << " [m] ---->" << tmp*HEAD_TO_PSI << " [PSI]" << endl;
//            tmp = static_cast<float>(mobBusInfo->SlavePumpStatus.TwinSlaveFlow); tmp /= 10;
//            cout << "----- TWIN SLAVE Flow : " << tmp << " [L/S] ---->" << tmp*CUBICMETER_TO_GALPERMIN << " [G/M]" << endl;
//            cout << "----- TWIN SLAVE Speed : " <<  mobBusInfo->SlavePumpStatus.TwinSlaveSpeed << " [RPM]" << endl;

            if (modbusPumpData.circularConfigMode == E_PUMP_TWIN_MASTER)
            {
//                cout << "-------Twin Mode Circular : " << CIRCULAR_CONFIG_S[modbusPumpData.circularConfigMode] << endl;
                modbusPumpData.TwinPumpData.TwinSlaveRPM               =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveRPM);
                modbusPumpData.TwinPumpData.TwinSlaveStartStop         =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveStartStop);
                modbusPumpData.TwinPumpData.TwinSlaveInputpower        =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveInputpower);
                modbusPumpData.TwinPumpData.TwinSlaveHead              =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveHead);
                modbusPumpData.TwinPumpData.TwinSlaveFlow              =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveFlow);
                modbusPumpData.TwinPumpData.TwinSlaveSpeed             =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveSpeed);

                modbusPumpData.TwinPumpData.TwinSlaveWindingTemp1      =  static_cast<int>(mobBusInfo->SlavePumpStatus.TwinSlaveWindingTemp1);
                modbusPumpData.TwinPumpData.TwinSlaveWindingTemp2      =  static_cast<int>(mobBusInfo->SlavePumpStatus.TwinSlaveWindingTemp2);
                modbusPumpData.TwinPumpData.TwinSlaveWindingTemp3      =  static_cast<int>(mobBusInfo->SlavePumpStatus.TwinSlaveWindingTemp3);
                modbusPumpData.TwinPumpData.TwinSlavePowerModuleTemp   =  static_cast<int>(mobBusInfo->SlavePumpStatus.TwinSlavePowerModuleTemp);

                modbusPumpData.TwinPumpData.TwinSlaveQuadratureCurrent =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveQuadratureCurrent);
                modbusPumpData.TwinPumpData.TwinSlaveBitFieldAlarm1    =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveBitFieldAlarm1);
                modbusPumpData.TwinPumpData.TwinSlaveBitFieldAlarm2    =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveBitFieldAlarm2);
                modbusPumpData.TwinPumpData.TwinSlaveBitFieldErrors    =  static_cast<unsigned int>(mobBusInfo->SlavePumpStatus.TwinSlaveBitFieldErrors);
            }


             /// SSS REFACTOR
             ///
             modbusPumpData.newData = true;
             modbusPumpData.pumpID = static_cast<unsigned int>(mobBusInfo->pumpID + 1);
             gModBusPumpData = modbusPumpData;

             /// REFACTOR BLOCK
             ///

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

        case TLV_GET_STATUS_INFORMATION:
            // new command
            PUMP_SYSTEM_STATUS_S  sysStatus;
            sysStatus.Loop1Id      = ntohl(msg.data[0]);
            sysStatus.Loop1IdState = ntohl(msg.data[1]);
            sysStatus.Loop2Id      = ntohl(msg.data[2]);
            sysStatus.Loop2IdState = ntohl(msg.data[3]);

            systemStatusData.LOOP_1_ON     = static_cast<bool>(sysStatus.Loop1IdState);
            systemStatusData.LOOP_2_ON     = static_cast<bool>(sysStatus.Loop2IdState);
            setStatusAvailable(true);
            info_cnt++;

            cout << "\n\n=> Loop 1 State =" << ( (sysStatus.Loop1IdState == 0) ? "STOPPED": "RUNNING") << endl;
            cout << "\n=> Loop 1 State ="   << ( (sysStatus.Loop2IdState == 0) ? "STOPPED": "RUNNING") << endl;


            break;
        
        case TLV_LOOP_COLD_START_STATUS:
        {
            const char *status_str[] = { "Bypassed", "Pass", "Fail" };
            const char *fail_reason_str[] = { "No failure", "Reservoir Warmup Fail", "Booster Warmup Failure" };
            PUMP_COLD_START_STATUS_S  coldStart;
            coldStart.status =  ntohl(msg.data[0]);
            coldStart.failReason = ntohl(msg.data[1]);
            coldStart.reservoirTemp = ntohl(msg.data[2]) ;
            coldStart.coldStartTemp = ntohl(msg.data[3]) ;
            coldStart.reservoirPreheatTime =  ntohl(msg.data[4]);
            coldStart.InitialPreheatTemp = ntohl(msg.data[5]);
            coldStart.boosterPreheatTime = ntohl(msg.data[6]);
            coldStart.pumpTempOperLimit = ntohl(msg.data[7]) ;

            cout << endl <<  " ===============Cold Start Events ============" <<  endl;
            if (coldStart.status <3)
                cout << " Cold Start Status :  " <<  status_str[coldStart.status]   << endl;
            if (coldStart.failReason <3)
                cout << " Cold Start Failure:  " <<  fail_reason_str[coldStart.failReason]   << endl;

//            cout << " Last Reservoir Temperature:" << dec << (int) (coldStart.reservoirTemp) <<  "  °C" << endl;
//            cout << " Cold Start Trigger Temperature:" << dec  << (int) (coldStart.coldStartTemp) << "  °C" << endl;
//            cout << " Reservoir PreHeat Time :"  << dec  <<  coldStart.reservoirPreheatTime  << "  Seconds" << endl;
//            cout << " Initial Pre-heat temperature :"  << dec  << (int) (coldStart.InitialPreheatTemp) << " °C" << endl;
//            cout << " Booster PreHeat Time :"  << dec <<  coldStart.boosterPreheatTime  << "  Seconds" << endl;
//            cout << " Pump Operational temperature ( lower limit) :" << dec  << (int) (coldStart.pumpTempOperLimit) << " °C" << endl;
//            cout <<   " =============================================" <<  endl;
            break;
        }
        default:
//            printf(" %s()!!! UNKOWN TCP  Message received--[ msg size=%d -- cmd=0x%0x (0x%0x)] ---\n",__FUNCTION__,msgSize, commandId, msg.commandID );
//            printf(" %s()!!! UNKOWN TCP  Message received--[ raw msg size=%d -- raw cmd= 0x%0x] ---\n",__FUNCTION__, msg.commandSize, msg.commandID);
            logger::warn(__FILE__, __FUNCTION__, "!!! UNKOWN TCP  Message received from CDU !!!......");

            setStatusAvailable(false);

           break;
    }


    // update global var
    if(StatusAvailable)
    {
        gSystemStatusData = systemStatusData;
        loop_cnt++;
    }


    /// FIXME : REMOVE WHEN DONE
    ///

//    printf(" %s() : loop_cnt: %d , info_cnt: %d , alarm_cnt: %d  ---\n",__FUNCTION__, loop_cnt, info_cnt, alarm_cnt);

    if(loop_cnt >= LOOP_CNT_MAX)
    {
//        printf("\n\n\n===>>> %s() : For loop_cnt: %d (~%d), We have this many other counts info_cnt: %d , alarm_cnt: %d  ---\n",__FUNCTION__, loop_cnt, LOOP_CNT_MAX,  info_cnt, alarm_cnt);

        std::string info_msg = "For loop_cnt: " + std::to_string(loop_cnt) + " We have these counts: "\
                " info_cnt: " + std::to_string(info_cnt) + \
                " alarm_cnt: " + std::to_string(alarm_cnt);

        logger::info(__FILE__, __FUNCTION__, info_msg);
        loop_cnt  = 0;
        info_cnt  = 0;
        alarm_cnt = 0;

    }
    /// end of fixme

}


/**
    Receive data from the connected host
*/
int  LCC_PUMP_CLIENT::receive()
{
   GENERAL_COMMANDS msg = {};

    struct pollfd   fds[1];

    nfds_t          nfds;

//    int             timeout = 2000;     // orig
    int             timeout = 1000;


    fds[0].fd = sock;
    nfds = 1;
    fds[0].events = POLLIN;
    // Wait for Data to be present in Sockets
    int ret = poll(fds, nfds, timeout);


    if (ret > 0 )
    {
        if ( (fds[0].revents &  POLLIN ) && (fds[0].fd ==  sock) )
        {
            auto rxlen = ::recv(sock , &msg , sizeof(msg) , 0);
            if( rxlen > 0 )
            {
                processMessage(msg);
            }
        }
    }
    else
    {
      if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL) )
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
//    isconnected = false;

}

void LCC_PUMP_CLIENT::printMessage(string&  reply)
{
    printf("\nRX Data  In HEX: ");
    for ( unsigned int i= 0; i< reply.size(); i++ )
    {
        printf("0x%02X " , static_cast<unsigned char>(reply.at(i)) );
    }
    printf("\nRX Data  In Text: ");
    for ( unsigned int i= 0; i< reply.size(); i++ )
    {
        if (isprint(reply.at(i) ))
        cout << reply[i];
        else
            cout << ".";
    }
    printf("\n");

}

void *LCC_PUMP_CLIENT::clientManager(void *ptr)
{
      LCC_PUMP_CLIENT &client = * ( reinterpret_cast <LCC_PUMP_CLIENT *> (ptr));

      //receive and echo reply
      while (true)
      {
          if (!client.clientConnected() )
          {
             client.conn();
          }
          sleep(10);
      }
}


void *LCC_PUMP_CLIENT::rxMain(void *ptr)
{
     LCC_PUMP_CLIENT &client = * ( reinterpret_cast <LCC_PUMP_CLIENT *> (ptr));
     //receive and echo reply
     while (true)
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
      
        int paramLen = sizeof(params)/sizeof(SosConfSet_T);

        if(loadCfg.loadConfig(params, paramLen) == 0)
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
    //
    printf(" skip Pump Regulation  = %d ",configData.config_data_u.skipPumpRegulation);
    //
    printf("COLD_START_ENABLED  = %d \n",configData.config_data_u.coldStartEnabled);
    printf("COLD_START_TRIGGER_TEMP = %d  Celc\n",configData.config_data_u.coldStartTriggerTemp);
    printf("COLD_START_PREHEAT_TIME_SEC  = %d sec\n",configData.config_data_u.coldStartPreHeaTime);
    printf("FLUSH_COOLANT_PREHEAT_TIME_SEC = %dsec \n",configData.config_data_u.flushCoolantPreHeaTime);
    printf("BOOSTER_PREHEAT_TIME_SEC = %d  sec\n",configData.config_data_u.boosterPreHeaTime);
    printf("PRE_HEAT_DELAY_SEC  = %d  sec\n",configData.config_data_u.PreHeatDelay);
    printf("INITIAL_WARMUP_TEMP  = %d  Celc\n",configData.config_data_u.intialWarmupTemp);
    printf("PUMP_TEMP_OPER_LIMIT  = %d  Celc\n",configData.config_data_u.pumpOperTempLimit);
    printf("\n\n");    
    
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
/// TODO : Add additional method called from ENV MGR
///     : should NOT stay stuck in thread but check for data, receive, parse and return
///

bool LCC_PUMP_CLIENT::loadPumpConfigFile()
{
    auto fileName = "/empower/config/Pump.Config";
    ;

//    CONFIGURATION_DATA_S cfgData;
    printf("Loading Configuration from File %s  \n", fileName);
    if ( getCfgData(fileName)  )
    {
        sendCfgData(ConfigData);
        return true;
    }
    else
    {
        printf("!!!!!!!!!!!!!!!!! Loading configuration failed !!!!\n");
        return false;
    }
}


int LCC_PUMP_CLIENT::checkCDUdata(std::string hostIP, uint16_t hostPort, std::string msg )
{

    if ( ( hostIP.empty() ) || (hostPort == 0 )  )
    {
        printf("\n\n[%s() @ line: %d]***ERROR: host=%s, port=%d, data=%s \n", __FUNCTION__, __LINE__, hostIP.c_str(), hostPort, msg.c_str() );
        exit(EXIT_FAILURE);
    }

    //connect to host
    LCC_PUMP_CLIENT client(hostIP, hostPort);

    client.conn();

    if(client.clientConnected())
    {
        client.receive();
    }

    client.close();

    //done
    return 0;
}




///
///
//int LCC_PUMP_CLIENT::lccMain(std::string hostIP, uint16_t hostPort, std::string msg )
//{
////    char    *host = NULL;
////    int     port = 0;
////    char    *data = NULL;
////    int     opt;
//
//
//    if ( ( hostIP.empty() ) || (hostPort == 0 )  )
//    {
//        printf("\n\n[%s() @ line: %d]***ERROR: host=%s, port=%d, data=%s \n", __FUNCTION__, __LINE__, hostIP.c_str(), hostPort, msg.c_str() );
//        exit(EXIT_FAILURE);
//    }
//
//
//    //connect to host
//    LCC_PUMP_CLIENT client(hostIP, hostPort);
//    client.conn();
//
//    pthread_create (&LCC_PUMP_CLIENT::rxThread, NULL,  LCC_PUMP_CLIENT::rxMain, static_cast<void *> ( &client));
//    pthread_create (&LCC_PUMP_CLIENT::socketThread, NULL,  LCC_PUMP_CLIENT::clientManager, static_cast<void *> ( &client));
//
//    pthread_join( LCC_PUMP_CLIENT::rxThread, NULL);
//    pthread_join(LCC_PUMP_CLIENT::socketThread, NULL);
//
//    client.close();
//    //done
//    return 0;
//}


/// TODO : add methods for Env Mgr to Interface to data and commands
///
///


bool LCC_PUMP_CLIENT::getStatusAvailable() { return StatusAvailable; }

void LCC_PUMP_CLIENT::setStatusAvailable(bool dataready) { StatusAvailable = dataready; }

// get data packets
SYSTEM_REPORT_DATA_S LCC_PUMP_CLIENT::getStatusData()
{
    return gSystemStatusData;

}



// MODBUS Additions
bool LCC_PUMP_CLIENT::getModBusStatusAvailable() { return ModBusStatusAvailable; }

void LCC_PUMP_CLIENT::setModBusStatusAvailable(bool modBusDataReady) { ModBusStatusAvailable = modBusDataReady; }

// get data packets
MODBUS_REPORT_DATA_S LCC_PUMP_CLIENT::getModBusPumpData()
{
    return gModBusPumpData;

}

void LCC_PUMP_CLIENT::setIpAddress(std::string ipaddr)  { address = ipaddr; }

void LCC_PUMP_CLIENT::setIpPort(uint16_t ipport)  { port = ipport; }

bool LCC_PUMP_CLIENT::getSystemPumpONstate() { return gSystemPumpON; }
bool LCC_PUMP_CLIENT::getLoop1PumpONstate()  { return Loop1PumpON; }
bool LCC_PUMP_CLIENT::getLoop2PumpONstate()  { return Loop2PumpON; }




