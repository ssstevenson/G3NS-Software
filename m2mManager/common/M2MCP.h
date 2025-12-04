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
* 0      06/1/2025   Marc Obbad       Initial Release.

*
*****************************************************************************/

//#ifndef __GEN3_M2MCP__
//#define __GEN3_M2MCP__
#pragma once
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <stdint.h>
#include <queue>
#include <map>
#include <poll.h>

#include <cerrno>    // for errno

#include <cstring>
#include <iostream>
#include <ctype.h>
#include <algorithm>
#include <functional>
#include <cctype>
#include <iomanip>
#include <pthread.h>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <execinfo.h>
#include <cxxabi.h>
#include <zmq.hpp>
#include <cstdio>
#include <regex>
#include <fstream>   // For file handling
#include "M2MTimer.h"
#include <helpers/debug.h>
#include "SysMsgCtrl.h"   // reflect  APIs to external world

using namespace std;
class M2MC_CP : public M2MTimer
{

    public:

        virtual ~M2MC_CP();
        M2MC_CP();
        // Define MAX BUNDLE COMMANDS
        static const int MAX_BUNDLE = 32;
        [[nodiscard]] bool getValueFromM2MCmd( string str, float &pwr );

        virtual void handleExecption() = 0;
        //
        virtual void startThread() = 0;
        pthread_t getMyThread() { return myThread; };
        ///
        void startNotificationThread();
        static void  *notification_thread(void *ptr);
         pthread_t getNotifThread() { return notifThread; };
        //
        void setZmqCtx( std::shared_ptr<zmq::context_t> ctx);
        void setMsgFactoryConnection(std::shared_ptr<messageFactoryConnection > msgIfConn);
        void setStatusRequestIface(std::shared_ptr<statusRequestInterface > pStatusReqIface);
        bool  isTimeOutUsed() { return useTimeout; };
        int getTimeout () { return M2M_TIMEOUT;} ;

        void print_hex(const  char *msg, int len )
        {
            for (int i=0; i< len; i++) {
                printf("%02X ", msg[i]);  // Uppercase hex, use %02x for lowercase
            }
            printf("\n");
        }
        bool is_delimiter(char c) {
            return c == '\r' || c == '\n';
        }

        void setDelimiter(const std::string& newDelimiter)
        {
            Delimiter = newDelimiter;  // Direct assignment, no need to call clear
        }
    protected:
       pthread_t myThread;
        pthread_t notifThread;
        // Process Message  received from from UI
        void processM2MMessage( std::string &message, std::string &reply );
        vector<string>  split(string str, char delimiter);
        bool processCommand(std::string msg, std::string &reply);
        bool processCommandBlock(std::string msg, vector <string> &cmd );

        std::chrono::high_resolution_clock::time_point getCurrentTime()
        {
            return std::chrono::high_resolution_clock::now();
        }
        // Catch Exceptions
        static void sigHandler(int sig, siginfo_t *siginfo, void *context) ;
        void init_exception();
        SysMsgCtrl sysMsgCtrl;
    private:
        using memApi = std::function<std::string(std::string)>;
        std::map<std::string, memApi>  m2mCmd;
        // std::map<std::string, std::function<std::string(std::string)> >  m2mCmd;
        void initCommand();

        string  getCmdReply(std::string_view prefix, const std::string msg, int cmdLen )
        {
            float power = 0.0f;
            string rep;
            sysMsgCtrl.getGenericCmdReply(msg.substr(0, static_cast<long unsigned int> (cmdLen)) , power,rep);
            std::string valreply {prefix};
            valreply += rep;
            dbgprintf("[%s]---> Response to cmd = %s ===> %s\n", __FUNCTION__, std::string(prefix).c_str(), valreply.c_str() );
            return valreply;
        }
        string sendGenericCmd( const string msg, int cmdLen, bool hasValue=false )
        {
            string   reply = "NA";
            float power = 0.0;
            bool valid = true;
            if (hasValue) {
                valid = getValueFromM2MCmd( msg, power );
            }

            if (valid) {
                sysMsgCtrl.SetGenericValue(msg.substr(0,static_cast<long unsigned int>(cmdLen)),  power);
            }
            return(reply);
        }
        std::vector<std::string> split_commands(const std::string& input)
        {
            std::string normalized = input;

            // Normalize \r\n to \n
            size_t pos = 0;
            while ((pos = normalized.find("\r\n", pos)) != std::string::npos) {
                normalized.replace(pos, 2, "\n");
            }
            while ((pos = normalized.find("\n\r", pos)) != std::string::npos) {
                normalized.replace(pos, 2, "\n");
            }
            // Replace remaining \r with \n
            std::replace(normalized.begin(), normalized.end(), '\r', '\n');

            std::vector<std::string> result;
            std::stringstream ss(normalized);
            std::string line;

            while (std::getline(ss, line, '\n')) {
                if (!line.empty()) {
                    result.push_back(line);
                }
            }

            return result;
        }

        void trim(std::string &cmd);
         std::string m2mDisableAutoRF(std::string msg) ;
         std::string m2mEnableAutoRF(std::string msg);
         std::string m2mAutoRFstate(std::string msg);
         std::string m2mFltrBandSel(std::string msg);
         std::string m2mFltrBandState(std::string msg);
         std::string m2mIBIT(std::string msg);
         std::string m2mFastDetectMode(std::string msg);
         std::string m2mSlowDetectMode(std::string msg);
         std::string m2mDetectModeState(std::string msg);
         std::string m2mFaultClear(std::string msg);
         std::string m2mFactoryCmdEn(std::string msg);
         std::string m2mQryID(std::string msg);
         std::string m2mQryFW(std::string msg);
         std::string m2mQryMfg(std::string msg);
         std::string m2mQryModel(std::string msg);
         std::string m2mQrySN(std::string msg);
         std::string m2mQryVer(std::string msg);
         std::string m2mSetALClevel(std::string msg);
         std::string m2mGetALClevel(std::string msg);
         std::string m2mSetAGClevel(std::string msg) ;
         std::string m2mGetAGClevel(std::string msg) ;
         std::string m2mSetMGClevel(std::string msg) ;
         std::string m2mGetMGClevel(std::string msg ) ;
         std::string m2mSHUTDOWN(std::string msg) ;

        // new multi-modes
         std::string m2mLoadCfgIndex(std::string msg) ;
         std::string m2mSaveCfg(std::string msg) ;
          std::string m2mGetCfg(std::string msg) ;
         std::string m2mStoreDefault(std::string msg) ;
         std::string m2mONLINE(std::string msg) ;
         std::string m2mTimedONLINE(std::string msg) ;
         std::string m2mSTANDBY(std::string msg) ;
         std::string m2mModeState(std::string msg) ;
         std::string m2mCloseNet(std::string msg) ;
         std::string m2mNetTimeout(std::string msg) ;
         std::string m2mGetFwdPwr(std::string msg) ;
         std::string m2mGetInpPwr(std::string msg) ;
         std::string m2mGetRevPwr(std::string msg) ;
         std::string m2mGetVSWR(std::string msg) ;
         std::string m2mSetAnt(std::string msg) ;
         std::string m2mGetAnt(std::string msg) ;
         std::string m2mGetSysStatus(std::string msg) ;
         std::string m2mGetSysTimes(std::string msg) ;
         std::string m2mSetUnitsToDBM(std::string msg) ;
         std::string m2mSetUnitsToWatts(std::string msg) ;
         std::string m2mGetUnits(std::string msg) ;
         std::string m2mGetSysTemps(std::string msg) ;
         std::string m2mSetTRswitch(std::string msg) ;
         std::string m2mGetTRswitch(std::string msg) ;
        //
         std::string m2mGetCentralFreq(std::string msg) ;
         std::string m2mGetBandwidth(std::string msg) ;
         std::string m2mSetCentralFreq(std::string msg) ;
         std::string m2mSetBandwidth(std::string msg) ;
         bool m2mBundle(std::string msg, vector <string>  &cmds);

        //
        void timer_delete();
        void time_start ( int sec);
        void timerExpired() override;
        void sendMS();
        struct sigaction act;
        virtual void m2mSetTimeOut (int sec);
        virtual void m2mCloseConnection() = 0;
        virtual void m2Mrespond( string reply) = 0;

        void getSoftwareAndFirmware();
        string Delimiter;
        string SoftwareBundle;
        string RfsID;
        string SysModel;
        string SysSN;
        string SysFW;
        // manager NT<xxx>
        bool     useTimeout;
        int    M2M_TIMEOUT;
        void setM2mTimeout ( int timeout ) {M2M_TIMEOUT = timeout;};
        void setUseTimeout( bool use ) { useTimeout = use ; };


};

//#endif
