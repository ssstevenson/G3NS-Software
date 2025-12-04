/*!
 * \file
 * \brief SysMsgCtrl.h
 ===============================================================================
*/


#pragma once

#include <signal.h>
#include <vector>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "M2MZeroMQ.h"
#include <unordered_map>
#include <functional>
#include <helpers/debug.h>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <statusRequestInterface/statusRequestInterface.h>

using namespace std;
using namespace empower;

class  SysMsgCtrl : public M2MZeroMQ {
    public :
        SysMsgCtrl();
        ~SysMsgCtrl() {};
        void clearUiM2mControl();
        void setUiM2mControl();

        //
        void SetGenericValue(string msg, float power);
        void getGenericCmdReply(string msg, float &val, string &rep);
        // Shared Data Structure
        string processMSC(string msg);
        std::string getSystemDescription();

        void setStatusRequestIface(std::shared_ptr<statusRequestInterface > pStatusReqIface)
        {
            statusIf = pStatusReqIface;
        }
        std::string  getSysTemps();
        bool  getNotificationMsg(std::string &reply);
 private:
        std::unordered_map<std::string, std::string> m2mToMessage;
        string processReplyMsg(const string messageType, float val, string &result);
        void initM2mToCmd();
        // For commands with no paameters
        void genericRfManagerCommand(string msg, string &cmd, float value, string& reply);
        // For commands with  parameters
        void genericRfManagerCommand(string msg, string &cmd, vector<string>  &params, string& reply);
        string getCmdMessage( string m2m);


        //
        using resAPI = std::function<std::string(std::string &, float &)>;
        std::unordered_map<std::string, resAPI>  m2mCmdResp;
        void initCmdResponse(void);
        std::string  m2mSetCentralFreq(string & rep, float &val);
        std::string  m2mSetBandwidth(string & rep, float &val);

        std::string  m2mGetCentralFreq(string & rep, float &val);
        std::string  m2mGetBandwidth(string & rep, float &val);
        std::string  m2mDisableAutoRF(string & rep, float &val);
        std::string  m2mEnableAutoRF(string & rep, float &val);
        std::string  m2mAutoRFstate(string & rep, float &val);

        std::string  m2mFastDetectMode(string & rep, float &val);
        std::string  m2mSlowDetectMode(string & rep, float &val);
        std::string  m2mDetectModeState(string & rep, float &val);
        std::string  m2mFaultClear(string & rep, float &val);

        std::string  m2mSetALClevel(string & rep, float &val);
        std::string  m2mGetALClevel(string & rep, float &val);
        std::string  m2mSetAGClevel(string & rep, float &val);
        std::string  m2mGetAGClevel(string & rep, float &val);
        std::string  m2mSetMGClevel(string & rep, float &val);
        std::string  m2mGetMGClevel(string & rep, float &val);
        std::string  m2mSHUTDOWN(string & rep, float &val);

        std::string  m2mONLINE(string & rep, float &val);
        std::string  m2mSTANDBY(string & rep, float &val);
        std::string  m2mModeState(string & rep, float &val);

        std::string  m2mGetFwdPwr(string & rep, float &val);
        std::string  m2mGetInpPwr(string & rep, float &val);
        std::string  m2mGetRevPwr(string & rep, float &val);
        std::string  m2mGetVSWR(string & rep, float &val);
        std::string  m2mSetAnt(string & rep, float &val);
        std::string  m2mGetAnt(string & rep, float &val);
        std::string  m2mGetSysStatus(string & rep, float &val);
        std::string  m2mSetUnitsToDBM(string & rep, float &val);
        std::string  m2mSetUnitsToWatts(string & rep, float &val);
        std::string  m2mGetUnits(string & rep, float &val);
        std::string  m2mGetSysTemps(string & rep, float &val);
        std::string  m2mSetTRswitch(string & rep, float &val);
        std::string  m2mGetTRswitch(string & rep, float &val);
        std::string  m2mSaveCfg(string & rep, float &val);
        std::string  m2mGetCfg(string & rep, float &val);

        std::string strTolower(std::string_view sv)
        {
            std::string out;
            out.reserve(sv.size());
            std::transform(
                sv.begin(), sv.end(), std::back_inserter(out),
                [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); }
            );
            return out;
        }
        template<typename Map>
        std::string codeFromNameStr(std::string_view name,
                             const Map& data,
                             std::string defaultValue = "NA")
        {
            auto it = data.find(name);
            if (it != data.end())
             return std::string{ it->second };
            return defaultValue;
        }

    std::shared_ptr<statusRequestInterface> statusIf;
};

