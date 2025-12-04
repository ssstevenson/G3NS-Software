/*!
 * \file
 * \brief M2M Messaging Control
 *
 ===============================================================================

 Name        : M2M System Messaging Controller ( SysMsgCtrl.cpp )
 Author      : Marc Obbad
 Version     :

 Description :  This module is responsible for converting M2M Commanads to JSON command
                and send them to target processor over ZeroMQ


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
* 1.0      06/1/2025   Marc Obbad       Initial Release.

*
*****************************************************************************/


#include <iostream>
#include <sstream>
#include <zmq.hpp>
#include <JsonAPI/jsonFormatter.h>
#include "SysMsgCtrl.h"

SysMsgCtrl::SysMsgCtrl():m2mToMessage{},m2mCmdResp{},statusIf{}
{
    initM2mToCmd();
    initCmdResponse();
}

//
//
//

string SysMsgCtrl::getCmdMessage( string m2m)
{
    string msg;

    if( m2mToMessage.count(m2m) > 0 )
    {
        msg = m2mToMessage[m2m];
    }
    return msg;
}

void SysMsgCtrl::genericRfManagerCommand(string msg, string &cmd, float value, string& reply)
{

    JsonFormater jsonString;
    dbgprintf ("[%s]- Processing command : %s \n", __FUNCTION__, msg.c_str() );
    cmd = getCmdMessage(msg);
    if (cmd.empty())
    {
        syslog(LOG_ERR,"[%s]--> M2M getCmdMessage(): M2M Command to JSON mapping  failed: %s", __FUNCTION__, msg.c_str());
        printf ("[%s]- !!!!M2M Command [%s] to JSON mapping failed!!!\n", __FUNCTION__, msg.c_str());
    }
    else
    {
        string jmsg = jsonString.createSimpleMessage(cmd, 0,value);
        dbgprintf ("[%s]- M2M cmd to RF manager IPC : %s \n", __FUNCTION__, jmsg.c_str());
        syslog(LOG_DEBUG,"[%s]- M2M cmd to RF manager IPC : %s \n", __FUNCTION__, jmsg.c_str());
        sendReqToProcess(Module::RF_MANAGER, jmsg, reply);
        syslog(LOG_DEBUG,"[%s] M2M Response From RF: %s: ", __FUNCTION__, reply.c_str());
        dbgprintf ("[%s]- M2M response from RF manager: %s \n", __FUNCTION__, reply.c_str());
    }

}

void SysMsgCtrl::genericRfManagerCommand(string msg, string &cmd, vector<string>  &params, string& reply)
{

    JsonFormater jsonString;
    dbgprintf ("[%s]- Processing command : %s \n", __FUNCTION__, msg.c_str() );
    cmd = getCmdMessage(msg);
    if (cmd.empty())
    {
        syslog(LOG_ERR,"[%s]--> M2M getCmdMessage(): M2M Command to JSON mapping  failed: %s", __FUNCTION__, msg.c_str());
        printf ("[%s]- !!!!M2M Command [%s] to JSON mapping failed!!!\n", __FUNCTION__, msg.c_str());
    }
    else
    {
        string jmsg =  jsonString.createJsonWithParams(cmd, 0 , params);
        dbgprintf ("[%s]- M2M cmd to RF manager IPC : %s \n", __FUNCTION__, jmsg.c_str());
        syslog(LOG_DEBUG,"[%s]- M2M cmd to RF manager IPC : %s \n", __FUNCTION__, jmsg.c_str());
        sendReqToProcess(Module::RF_MANAGER, jmsg, reply);
        syslog(LOG_DEBUG,"[%s] M2M Response From RF: %s: ", __FUNCTION__, reply.c_str());
        dbgprintf ("[%s]- M2M response from RF manager: %s \n", __FUNCTION__, reply.c_str());
    }

}


void SysMsgCtrl::SetGenericValue(string m2mMsg, float power)
{
    // this method is for command such as  LD<xx>, LM<xxx>..MO..MS.....
    string cmd,reply ;
    genericRfManagerCommand(m2mMsg,cmd, power,reply);
}


void SysMsgCtrl::getGenericCmdReply(string m2mMsg, float &power, string &rep)
{

    string cmdreply,cmd;
    genericRfManagerCommand(m2mMsg,cmd,0,cmdreply);
    if (!cmdreply.empty())
    {
        string messageType;
        int sequenceNumber;
        string result;
        float val{0.0};
        JsonFormater jsonString;
        //
        jsonString.parseMessage(cmdreply, messageType, sequenceNumber, val, result);
        if (cmd == messageType)
        {
           string reply =  processReplyMsg(messageType, val, result);
           rep = reply;
           power = val;
        }
    }
}

string  SysMsgCtrl::processReplyMsg(const string messageType, float val, string &result)
{
   string reply;

    if( m2mCmdResp.count(messageType) > 0 )
    {
        reply = m2mCmdResp.at(messageType)(result, val);
    }

    return reply;

}
void SysMsgCtrl::clearUiM2mControl()
{
}
void SysMsgCtrl::setUiM2mControl()
{
}

void SysMsgCtrl::initM2mToCmd(void)
{
    // init map to exec function -- uppercase only

    m2mToMessage["CF"]  = std::string("setCentralFrequency");
    m2mToMessage["BW"]  = std::string("setBandwidth");

    m2mToMessage["CF?"]  = std::string("getCentralFrequency");
    m2mToMessage["BW?"]  = std::string("getBandwidth");
    m2mToMessage["AOD"]  = std::string("disableAutoRF");
    m2mToMessage["AOE"]  = std::string("enableAutoRF");
    m2mToMessage["AO?"]  =  std::string("getAutoRFState");
   // m2mToMessage["B<"] =
   // m2mToMessage["B?"] =
   // m2mToMessage["BIT"] =
    m2mToMessage["DF"]   =  std::string("setFastDetectionMode");
    m2mToMessage["DS"]   =  std::string("setSlowDetectionMode");
    m2mToMessage["D?"]   =  std::string("getDetectionModeState");
    m2mToMessage["FC"]   =  std::string("gefaultClear");
    //m2mToMessage["FO<"] =

    m2mToMessage["LD<"] = std::string("setAlcLevel");
    m2mToMessage["L?"]  =  std::string("getAlcLevel");
    m2mToMessage["LG<"] =  std::string("setAgcLevel");
    m2mToMessage["LG?"] = std::string("getAgcLevel");
    m2mToMessage["LM<"] =  std::string("setMgcLevel");
    m2mToMessage["LM?"] = std::string("getMgcLevel");
    m2mToMessage["MC"] = std::string("coldStandBy");

    // new multi-modes
    //m2mToMessage["MLC"] =
    m2mToMessage["MSC<"] = std::string("saveConfig");
    m2mToMessage["MSC?"] = std::string("getConfig");
    //m2mToMessage["MSD"] =
    m2mToMessage["MO"] =  std::string("online");

    m2mToMessage["MS"] =  std::string("hotStandBy");
    m2mToMessage["M?"] =  std::string("getModeState");
    m2mToMessage["PF"] =  std::string("getForwardPower");
    m2mToMessage["PI"] =  std::string("getInputPower");
    m2mToMessage["PR"] =  std::string("getReversePower");
    m2mToMessage["PV"] =   std::string("getVswrState");
    m2mToMessage["SA<"] =  std::string("switchAntenna");
    m2mToMessage["SA?"] =  std::string("getCurrentAntenna");
    m2mToMessage["SS"] =  std::string("systemStatus");
    //m2mToMessage["ST"] =  std::string("SystemStatus");
    m2mToMessage["SUD"] = std::string("setUnitsToDBm");
    m2mToMessage["SUW"] = std::string("setUnitsToWatts");
    m2mToMessage["SU?"] = std::string("getUnits");
    m2mToMessage["T?"] =  std::string("getSystemTemp");
    m2mToMessage["TR<"] = std::string("setTRSwitch");
    m2mToMessage["TR?"] = std::string("getTRSwitch");
}


void SysMsgCtrl::initCmdResponse(void)
{
    // init map to exec function -- uppercase only

    m2mCmdResp["setCentralFrequency"]  = std::bind(&SysMsgCtrl::m2mSetCentralFreq, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["setBandwidth"]  = std::bind(&SysMsgCtrl::m2mSetBandwidth, this, std::placeholders::_1, std::placeholders::_2);

    m2mCmdResp["getCentralFrequency"]  = std::bind(&SysMsgCtrl::m2mGetCentralFreq, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getBandwidth"]  = std::bind(&SysMsgCtrl::m2mGetBandwidth, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["disableAutoRF"] = std::bind(&SysMsgCtrl::m2mDisableAutoRF, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["enableAutoRF"] = std::bind(&SysMsgCtrl::m2mEnableAutoRF, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getAutoRFState"] = std::bind(&SysMsgCtrl::m2mAutoRFstate, this, std::placeholders::_1, std::placeholders::_2);
   // m2mCmdResp["B<"] =  std::bind(&SysMsgCtrl::m2mFltrBandSel, this, std::placeholders::_1, std::placeholders::_2);
    //m2mCmdResp["B?"] =  std::bind(&SysMsgCtrl::m2mFltrBandState, this, std::placeholders::_1, std::placeholders::_2);
   // m2mCmdResp["BIT"] = std::bind(&SysMsgCtrl::m2mIBIT, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["setFastDetectionMode"] =  std::bind(&SysMsgCtrl::m2mFastDetectMode, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["setSlowDetectionMode"] =  std::bind(&SysMsgCtrl::m2mSlowDetectMode, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getDetectionModeState"] =  std::bind(&SysMsgCtrl::m2mDetectModeState, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["gefaultClear"] =  std::bind(&SysMsgCtrl::m2mFaultClear, this, std::placeholders::_1, std::placeholders::_2);
   // m2mCmdResp["FO<"] = std::bind(&SysMsgCtrl::m2mFactoryCmdEn, this, std::placeholders::_1, std::placeholders::_2);

    m2mCmdResp["setAlcLevel"] = std::bind(&SysMsgCtrl::m2mSetALClevel, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getAlcLevel"] =  std::bind(&SysMsgCtrl::m2mGetALClevel, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["setAgcLevel"] = std::bind(&SysMsgCtrl::m2mSetAGClevel, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getAgcLevel"] = std::bind(&SysMsgCtrl::m2mGetAGClevel, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["setMgcLevel"] = std::bind(&SysMsgCtrl::m2mSetMGClevel, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getMgcLevel"] = std::bind(&SysMsgCtrl::m2mGetMGClevel, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["coldStandBy"] = std::bind(&SysMsgCtrl::m2mSHUTDOWN, this, std::placeholders::_1, std::placeholders::_2);

    // new multi-modes
   // m2mCmdResp["MLC"] = std::bind(&SysMsgCtrl::m2mLoadCfg, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["saveConfig"] = std::bind(&SysMsgCtrl::m2mSaveCfg, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getConfig"] = std::bind(&SysMsgCtrl::m2mGetCfg, this, std::placeholders::_1, std::placeholders::_2);
   // m2mCmdResp["MSD"] = std::bind(&SysMsgCtrl::m2mStoreDefault, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["online"] =  std::bind(&SysMsgCtrl::m2mONLINE, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["hotStandBy"] =  std::bind(&SysMsgCtrl::m2mSTANDBY, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getModeState"] =  std::bind(&SysMsgCtrl::m2mModeState, this, std::placeholders::_1, std::placeholders::_2);

    m2mCmdResp["getForwardPower"] =  std::bind(&SysMsgCtrl::m2mGetFwdPwr, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getInputPower"] =  std::bind(&SysMsgCtrl::m2mGetInpPwr, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getReversePower"] =  std::bind(&SysMsgCtrl::m2mGetRevPwr, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getVswrState"] =  std::bind(&SysMsgCtrl::m2mGetVSWR, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["switchAntenna"] = std::bind(&SysMsgCtrl::m2mSetAnt, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getCurrentAntenna"] = std::bind(&SysMsgCtrl::m2mGetAnt, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["systemStatus"] =  std::bind(&SysMsgCtrl::m2mGetSysStatus, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["setUnitsToDBm"] = std::bind(&SysMsgCtrl::m2mSetUnitsToDBM, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["setUnitsToWatts"] = std::bind(&SysMsgCtrl::m2mSetUnitsToWatts, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getUnits"] = std::bind(&SysMsgCtrl::m2mGetUnits, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getSystemTemp"] =  std::bind(&SysMsgCtrl::m2mGetSysTemps, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["setTRSwitch"] = std::bind(&SysMsgCtrl::m2mSetTRswitch, this, std::placeholders::_1, std::placeholders::_2);
    m2mCmdResp["getTRSwitch"] = std::bind(&SysMsgCtrl::m2mGetTRswitch, this, std::placeholders::_1, std::placeholders::_2);
}

std::string  SysMsgCtrl::m2mSetCentralFreq( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = resp;
return resp;
}
std::string  SysMsgCtrl::m2mSetBandwidth( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = resp;
return resp;
}

std::string  SysMsgCtrl::m2mGetCentralFreq( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = resp;
return resp;
}
std::string  SysMsgCtrl::m2mGetBandwidth( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = resp;
return resp;
}
std::string  SysMsgCtrl::m2mDisableAutoRF( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = resp;
return resp;
}
std::string  SysMsgCtrl::m2mEnableAutoRF( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = resp;
return resp;
}
std::string  SysMsgCtrl::m2mAutoRFstate( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = resp;
return resp;
}

std::string  SysMsgCtrl::m2mFastDetectMode( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = resp;
return resp;
}
std::string  SysMsgCtrl::m2mSlowDetectMode( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = resp;
    return resp;
}
std::string  SysMsgCtrl::m2mDetectModeState( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    static const std::unordered_map<std::string_view, std::string> data = {
        { "peekfast", "F" },
        { "peekslow", "S" },
        { "rms",      "R" }
    };
    return codeFromNameStr(strTolower(resp),data);

}
std::string  SysMsgCtrl::m2mFaultClear( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (std::string("NA"));
}

std::string  SysMsgCtrl::m2mSetALClevel( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
   return (std::string("NA"));
}
std::string  SysMsgCtrl::m2mGetALClevel( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return ( std::to_string(static_cast <int> (val*100) ) );
}
std::string  SysMsgCtrl::m2mSetAGClevel( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (std::string("NA"));
}
std::string  SysMsgCtrl::m2mGetAGClevel( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return ( std::to_string(static_cast <int> (val*100) ) );
}
std::string  SysMsgCtrl::m2mSetMGClevel( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (std::string("NA"));
}
std::string  SysMsgCtrl::m2mGetMGClevel( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
     return ( std::to_string(static_cast <int> (val*100) ) );
}
std::string  SysMsgCtrl::m2mSHUTDOWN( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (std::string("NA"));
}

std::string  SysMsgCtrl::m2mONLINE( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (std::string("NA"));
}
std::string  SysMsgCtrl::m2mSTANDBY( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
   return (std::string("NA"));
}
std::string  SysMsgCtrl::m2mModeState( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    static const std::unordered_map<std::string_view, std::string> data = {
        { "online", "O" },
        { "standby", "S" },
        { "shutdown", "C" }
    };
    return codeFromNameStr(strTolower(resp),data);
}

std::string  SysMsgCtrl::m2mGetFwdPwr( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return ( std::to_string(static_cast <int> (val*100) ) );
}
std::string  SysMsgCtrl::m2mGetInpPwr( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return ( std::to_string(static_cast <int> (val*100) ) );
}
std::string  SysMsgCtrl::m2mGetRevPwr( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
     return ( std::to_string(static_cast <int> (val*100) ) );
}
std::string  SysMsgCtrl::m2mGetVSWR( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{

    return resp;
}
std::string  SysMsgCtrl::m2mSetAnt( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (std::string("NA"));
}
std::string  SysMsgCtrl::m2mGetAnt( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return ( std::to_string(val) );
}
std::string  SysMsgCtrl::m2mGetSysStatus( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return resp;
}
std::string  SysMsgCtrl::m2mSetUnitsToDBM( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (std::string("NA"));
}
std::string  SysMsgCtrl::m2mSetUnitsToWatts( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (std::string("NA"));
}
std::string  SysMsgCtrl::m2mGetUnits( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{

    static const std::unordered_map<std::string_view, std::string> data = {
        { "dbm", "D" },
        { "watts", "W" }
    };
    return codeFromNameStr(strTolower(resp),data);

}
std::string  SysMsgCtrl::m2mGetSysTemps( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    resp = std::string("NA");
    if(auto curTemp{statusIf->getStatusParam<std::int32_t>("ENV_MAX_REGULATION_TEMP")}; curTemp.has_value())
    {
        resp =  std::string("T? ") + std::string("MT ") +  std::to_string(curTemp.value());
    }
    return resp;

}
std::string  SysMsgCtrl::m2mSetTRswitch( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
   return resp;
}
std::string  SysMsgCtrl::m2mGetTRswitch( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{

   return resp;
}


std::string  SysMsgCtrl::m2mSaveCfg( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (std::string("NA"));
}

std::string  SysMsgCtrl::m2mGetCfg( [[maybe_unused]] string &resp, [[maybe_unused]] float &val )
{
    return (resp);
}
//
//

string  SysMsgCtrl::processMSC(string msg)
{
    // Parse  and slpit msc<......> message
    auto parseParameters = [](const std::string& input) -> std::vector<std::string>
    {
        std::vector<std::string> params;
        size_t start = input.find('<');
        size_t end = input.find('>');

        if (start == std::string::npos || end == std::string::npos || end <= start + 1) {
            return params; // Return empty vector if brackets are not found or malformed
        }

        std::string paramStr = input.substr(start + 1, end - start - 1);
        std::stringstream ss(paramStr);
        std::string token;
        while (std::getline(ss, token, ',')) {
            params.push_back(token);
        }

        return params;
    };


    dbgprintf ("[%s]---msg : %s\n", __FUNCTION__, msg.c_str());
    // Step 1: Parse the parameters
    std::vector<std::string> params = parseParameters(msg);
    // We know the message MSC<
    string msgCmd{"MSC<"};
    string cmd, reply;

    genericRfManagerCommand(msgCmd, cmd, params,reply);

    return std::string("NA");
}

std::string  SysMsgCtrl::getSysTemps()
{
    string resp = std::string("NA");
    if(auto curTemp{statusIf->getStatusParam<std::int32_t>("ENV_MAX_REGULATION_TEMP")}; curTemp.has_value())
    {
        resp =  std::to_string(curTemp.value());
    }
    return resp;

}


bool  SysMsgCtrl::getNotificationMsg(std::string &reply)
{
    //syslog(LOG_INFO, "[%s]---> SysMsgCtrl::getNotificationMsg \n", __FUNCTION__ );
    string status;
    bool repOk = receiveNotification(status);

    //syslog(LOG_INFO, "[%s]---> receiveNotificationK [Got message:%s] \n", __FUNCTION__ , repOk?"Yes":"NO");
    if (repOk)
    {
       // syslog(LOG_INFO, "[%s]---> Got a notification Message \n", __FUNCTION__ );
        // Status is JSON String
        JsonFormater jsonString;
        string messageType;
        int sequenceNumber;
        string result;
        float val{0.0};

        // For now we onlt SS ot T? are pushed to M2M
        static const std::unordered_map<std::string_view, std::string> cmd = {
            { "systemStatus", "SS " },
            { "getSystemTemp", "T? " }
        };

        //
        jsonString.parseMessage(status, messageType, sequenceNumber, val, result);
        auto it = cmd.find(messageType);
        if (it != cmd.end())
            reply =  it->second + result;
    }

    return repOk;

}



