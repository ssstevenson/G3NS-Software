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

#include <FpgaIO/fpgaHal.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wreorder"

#include "M2MCP.h"


M2MC_CP::M2MC_CP():userThread{},notifThread{}, notifThreadRunning(false), m2mCmd{},act(),Delimiter{"\n"},sysMsgCtrl{},
                    SoftwareBundle{},RfsID{},SysModel{},SysSN{},SysFW{},
                    useTimeout{false},M2M_TIMEOUT{0}
{
    initCommand();
    getSoftwareAndFirmware();
}

M2MC_CP::~M2MC_CP()
{
    pthread_join(notifThread, NULL);
}

void M2MC_CP::processM2MMessage( string &message,  string &replies)
{
   // make a copy..
    string msgS {message};

    vector<string> vect = split_commands(msgS);

    for(unsigned int i = 0; i < vect.size(); ++i)
    {
        string reply;
        string msg = vect[i];

        if (msg.at(0) != '!' )
        {
            string cmdrply;
            processCommand(msg,cmdrply);
            if (cmdrply!= "NA")
            {
                reply = cmdrply;
            }
        }
        else
        {
            reply.clear();
            vector <string >  cmdVect;
            vector <string >  respVect;
            string cmd;
            if ( processCommandBlock (msg, cmdVect ) )
            {
                for(vector<string>::const_iterator pcmd = cmdVect.begin(); pcmd != cmdVect.end(); ++pcmd)
                {
                    cmd = *pcmd;
                    if (cmd.empty() )
                    {
                        break;
                    }
                    else
                    {
                        string cmdRely;
                        processCommand(cmd,cmdRely);
                        if ( ( !cmdRely.empty() ) && (cmdRely != "NA"))
                        {
                            respVect.push_back(cmdRely);
                        }
                    }
                }  // end of iteration

                if ( respVect.size() > 1)
                {
                    reply += "!";

                    for (unsigned int j= 0; j< respVect.size() -1; j++)
                    {
                        reply += respVect.at(j);
                        if ( j != respVect.size() -1 )
                        {
                            reply += ";";
                        }
                    }
                    reply += "!";
                }
                else if (!respVect.empty() )
                {
                    reply += respVect.at(0);
                }
            }
            else
            {
                printf("!!!! ERROR %s()====> Aggregated command error !!!!! \n", __FUNCTION__);
            }

        }
        replies += reply;
        // sendUiUpdate();
    }
}

bool M2MC_CP::processCommandBlock(string msg,  vector <string> &cmd)
{
    return ( m2mBundle(msg, cmd) );
}

bool M2MC_CP::processCommand(string msg, string &reply)
{

    string              key;
    locale              loc;
    bool                cmdOk = false;
    // init reply
    reply.clear();

    // we need to do a little massage of data
    //
   dbgprintf("[%s]---> Message(len=%lu):%s\n", __FUNCTION__, msg.size(), msg.c_str() );
    if(!msg.empty())
    {
        std::transform(msg.begin(), msg.end(), msg.begin(), std::ptr_fun<int, int>(std::toupper));

        auto extractCommandPrefix = [](const std::string& input) -> std::string {
            std::smatch match;
            std::regex pattern(R"(^\s*([A-Z]+(?:<|\?)?))");  // matches e.g., MSC< or LDIO?

            if (std::regex_search(input, match, pattern)) {
                return match[1];
            }
            return "";
        };
        // clean string of  leading and trailing white spaces
        trim(msg);
        // Key can be two to three characters
        key = extractCommandPrefix(msg);
        if( m2mCmd.count(key) > 0 )
        {
            // access map to API function
            syslog(LOG_DEBUG, "[%s]---> Command found :%s len is %lu \n", __FUNCTION__, key.c_str() , key.size());
            reply = m2mCmd.at(key)(msg) + Delimiter ;
            cmdOk = true;
        }
        else if ( msg[0] != '\0')
        {
            syslog(LOG_INFO, "[%s]---> Command[%s] --- CMD Key[%s] NOT found  \n", __FUNCTION__, msg.c_str(), key.size() > 0 ? key.c_str(): "NotFound");
            reply.assign("?\n");
        }

    }
    else
    {
       syslog(LOG_INFO, "[%s]---> M2M  Message is empty \n", __FUNCTION__);
       printf ( "[%s]---> M2M Message is empty \n", __FUNCTION__);
    }

    return(cmdOk);

}

string M2MC_CP::m2mSetCentralFreq(string msg)
{
    string   reply; (void) msg;

    reply="NA\n";


    return(reply);
}

string M2MC_CP::m2mSetBandwidth(string msg)
{
    string   reply; (void) msg;

    reply="NA\n";


    return(reply);
}

string M2MC_CP::m2mGetCentralFreq(string msg)
{

   string   reply; (void) msg;

    reply="NA\n";

    return reply;

}
string M2MC_CP::m2mGetBandwidth(string msg)
{

    string   reply; (void) msg;

    reply="NA\n";

    return reply;

}

bool M2MC_CP::m2mBundle(string msgS,  vector <string>  &cmds)
{
    bool   reply = true;

    string msg = msgS;
    // We know the first character is !, check last one is ! "

    size_t pos1 = 0, pos2 = 0;
    size_t pos = 0;
    pos = msg.find("!") ;
    if ( pos != std::string::npos)
    {
        pos1 = pos;
        pos = msg.find("!",pos +1) ;
        if ( pos != std::string::npos)
        {
            pos2= pos;
        }

    }
    if ( pos2 == 0 )
    {
        printf("!!!! ERROR %s()====> Aggregated command missing last tag !!!! \n", __FUNCTION__);
        reply = false;
    }
    else
    {
        // Remove ! from command..
        msg = msg.substr(pos1+1, pos2 -1);
        // We do have a good bundle of command....
        // Parse commands

        cmds.clear();
        std::string delimiter = ";";
        std::string token;
        pos = 0;
        while ((pos = msg.find(delimiter)) != std::string::npos)
        {
            //cmds[i++] = msg.substr(0, pos);
            cmds.push_back(msg.substr(0, pos));
            std::cout << token << std::endl;
            msg.erase(0, pos + delimiter.length());
        }
        cmds.push_back(msg);

    }


    return(reply);
}


string M2MC_CP::m2mEnableAutoRF(string msg)
{

    string   reply; (void) msg;
    reply.clear();
    reply = "NA";
    return(reply);

}

string M2MC_CP::m2mDisableAutoRF(string msg)
{

    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);

}

string M2MC_CP::m2mAutoRFstate(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);

}

string M2MC_CP::m2mFltrBandSel(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mFltrBandState(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mFastDetectMode(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mSlowDetectMode(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mDetectModeState(string msg)
{
   return getCmdReply("D? ",msg,2) ;
}

string M2MC_CP::m2mFaultClear(string msg)
{
   return sendGenericCmd(msg,2);
}

string M2MC_CP::m2mFactoryCmdEn(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mQryID(string msg)
{
    string   reply; (void) msg;
    reply = "ID? " + sysMsgCtrl.getSystemID();


    return(reply);
}

string M2MC_CP::m2mQryFW(string msg)
{
    string   reply; (void) msg;
    reply = "IF? " + SysFW;
     return(reply);
}

string M2MC_CP::m2mQryMfg(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "IM? Empower RF Systems Inc.";

    return(reply);
}
string M2MC_CP::m2mQryModel(string msg)
{
    string   reply;
    (void) msg;

    if (SysModel.empty() )
        SysModel = sysMsgCtrl.getSystemModel();
    reply = "IN? " + SysModel;

    return(reply);
}

string M2MC_CP::m2mQrySN(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    //reply = "IS? 1001";
    if (SysSN.empty() )
        SysSN = sysMsgCtrl.getSystemSN();
    reply = "IS? " + SysSN;
    return(reply);
}

string M2MC_CP::m2mQryVer(string msg)
{
    string   reply; (void) msg;
    reply.clear();
   // reply = "IV? V 03.00.00";
    reply = "IV? " + RfsID + "/" + SoftwareBundle;

    return(reply);
}


string M2MC_CP::m2mSetALClevel(string msg)
{
   return sendGenericCmd(msg, 3,true);
}



string M2MC_CP::m2mGetALClevel(string msg)
{
    return getCmdReply("L? ",msg,2) ;
}

string M2MC_CP::m2mSetAGClevel(string msg)
{
   return (sendGenericCmd(msg, 3,true));
}

string M2MC_CP::m2mGetAGClevel(string msg)
{

    return getCmdReply("LG? ",msg,3) ;

}

string M2MC_CP::m2mSetMGClevel(string msg)
{
    float pwr = 0.0;
    string reply{"?"};
    if ( getValueFromM2MCmd(msg, pwr) )
    {
        if ( (pwr >= 0.0f) &&  ( pwr <= 100.0f))
        {
            return (sendGenericCmd(msg, 3,true));
        }
    }

    return reply;

}

string M2MC_CP::m2mGetMGClevel(string msg)
{

   return getCmdReply("LM? ",msg,3) ;

}


string M2MC_CP::m2mSHUTDOWN(string msg)
{
   return sendGenericCmd(msg,2);
}


string M2MC_CP::m2mONLINE(string msg)
{

    return sendGenericCmd(msg,2);
}

string M2MC_CP::m2mTimedONLINE(string msg)
{
    string   reply;
    reply = "NA";
    float timeout;
    if ( getValueFromM2MCmd( msg, timeout ) )
    {
      // send MO, then MS after timeout
      m2mONLINE("MO");
      int sec = static_cast < int > (timeout);
      time_start(sec);

    }

    return(reply);
}

string M2MC_CP::m2mSTANDBY(string msg)
{
   return sendGenericCmd(msg,2);
}

string M2MC_CP::m2mModeState(string msg)
{
    return getCmdReply("M? ",msg,2) ;
}

// Network Functions
string M2MC_CP::m2mCloseNet(string msg)
{
    string   reply;
   reply.clear();
   m2mCloseConnection();

    return(reply);
}

void M2MC_CP::m2mSetTimeOut(int sec)
{
    setM2mTimeout(sec);
    setUseTimeout(true);
}
string M2MC_CP::m2mNetTimeout(string msg)
{
    string   reply;
    reply.clear();
    reply = "NA";

    float timeout;

    if ( getValueFromM2MCmd( msg, timeout ) )
    {
       int sec = static_cast < int > (timeout);
       m2mSetTimeOut(sec);
       syslog(LOG_DEBUG,"\n %s(): New M2M Timeout msg =%s  --> %d seconds\n ", __FUNCTION__, msg.c_str(), sec) ;
   }
    return(reply);
}

// RF Power Report Functions
string M2MC_CP::m2mGetFwdPwr(string msg)
{
   return getCmdReply("PF ",msg,2) ;
}

string M2MC_CP::m2mGetInpPwr(string msg)
{
    return getCmdReply("PI ",msg,2) ;
}




string M2MC_CP::m2mGetRevPwr(string msg)
{
     return getCmdReply("PR ",msg,2) ;
}



string M2MC_CP::m2mGetVSWR(string msg)
{
    string   reply = "?";
    return(reply);
}




// Antenna Functions
string M2MC_CP::m2mSetAnt(string msg)
{
    string   reply = "?";
    return(reply);
}


string M2MC_CP::m2mGetAnt(string msg)
{
    string   reply = "?";
    return(reply);
}

// Status  Functions
string M2MC_CP::m2mGetSysStatus(string msg)
{
   return getCmdReply("SS ",msg,2) ;
}


string M2MC_CP::m2mGetSysTimes(string msg)
{
    string   reply;
    reply.clear();
    reply = "NA";

    return(reply);
}

// Units Display/Report Functions
string M2MC_CP::m2mSetUnitsToDBM(string msg)
{
    string   reply;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mSetUnitsToWatts(string msg)
{
    string   reply;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mGetUnits(string msg)
{
   return getCmdReply("SU? ",msg,3) ;
}
// Temperature Report Functions
string M2MC_CP::m2mGetSysTemps(string msg)
{
    string   reply;

    // We are getting Temp from Status request Interface for Now
    string temp  = sysMsgCtrl.getSysTemps();
    if ( temp != "NA")
    {
        reply = "T? " + temp;
    }

    return(reply);
}

// T/R Switch Functions
string M2MC_CP::m2mSetTRswitch(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mGetTRswitch(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mLoadCfgIndex(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}
// Config page settings
//
string M2MC_CP::m2mSaveCfg(string msg)
{

    return (sysMsgCtrl.processMSC(msg)) ;
}
string M2MC_CP::m2mGetCfg(string msg)
{
    string   reply = getCmdReply("MSC?",msg,4) ;
    reply.replace(0, 4, "MSC");
    return reply;
}

string M2MC_CP::m2mStoreDefault(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}

string M2MC_CP::m2mIBIT(string msg)
{
    string   reply; (void) msg;
    reply.clear();
    reply = "NA";

    return(reply);
}

//
//
//

//

void M2MC_CP::sendMS()
{
    syslog(LOG_DEBUG, "[%s]--->  MO timer expired -- sending MS command \n", __FUNCTION__ );

    string msg="MS\n";
    m2mSTANDBY(msg);

}
void M2MC_CP::timerExpired()
{
   sendMS();
}
//
//
void M2MC_CP::time_start ( int sec)
{
    startTimer(sec);
    syslog(LOG_DEBUG, "[%s]---> Starting MO timer(%d seconds )\n", __FUNCTION__, sec );
}

void M2MC_CP::timer_delete()
{
   stopTimer();
     syslog(LOG_DEBUG, "[%s]--->  Deleting MO timer .. \n", __FUNCTION__ );
}


 [[nodiscard]] bool M2MC_CP::getValueFromM2MCmd( string str, float &pwr )
{
    size_t start = str.find('<');
    size_t end = str.find('>');
    bool ret = false;

    if (start != std::string::npos && end != std::string::npos && end > start)
    {
        // Extract the substring between '<' and '>'
        std::string num_str = str.substr(start + 1, end - start - 1);

        // Convert the substring to an integer
        pwr = std::stof(num_str);
        ret = true;
    }

   return ret;
}

void M2MC_CP::initCommand(void)
{
    // init map to exec function -- uppercase only

    m2mCmd["CF"]  = std::bind(&M2MC_CP::m2mSetCentralFreq, this, std::placeholders::_1);
    m2mCmd["BW"]  = std::bind(&M2MC_CP::m2mSetBandwidth, this, std::placeholders::_1);

    m2mCmd["CF?"]  = std::bind(&M2MC_CP::m2mGetCentralFreq, this, std::placeholders::_1);
    m2mCmd["BW?"]  = std::bind(&M2MC_CP::m2mGetBandwidth, this, std::placeholders::_1);
    m2mCmd["AOD"] = std::bind(&M2MC_CP::m2mDisableAutoRF, this, std::placeholders::_1);
    m2mCmd["AOE"] = std::bind(&M2MC_CP::m2mEnableAutoRF, this, std::placeholders::_1);
    m2mCmd["AO?"] = std::bind(&M2MC_CP::m2mAutoRFstate, this, std::placeholders::_1);
    m2mCmd["B<"] =  std::bind(&M2MC_CP::m2mFltrBandSel, this, std::placeholders::_1);
    m2mCmd["B?"] =  std::bind(&M2MC_CP::m2mFltrBandState, this, std::placeholders::_1);
    m2mCmd["BIT"] = std::bind(&M2MC_CP::m2mIBIT, this, std::placeholders::_1);
    m2mCmd["DF"] =  std::bind(&M2MC_CP::m2mFastDetectMode, this, std::placeholders::_1);
    m2mCmd["DS"] =  std::bind(&M2MC_CP::m2mSlowDetectMode, this, std::placeholders::_1);
    m2mCmd["D?"] =  std::bind(&M2MC_CP::m2mDetectModeState, this, std::placeholders::_1);
    m2mCmd["FC"] =  std::bind(&M2MC_CP::m2mFaultClear, this, std::placeholders::_1);
    m2mCmd["FO<"] = std::bind(&M2MC_CP::m2mFactoryCmdEn, this, std::placeholders::_1);
    m2mCmd["ID?"] = std::bind(&M2MC_CP::m2mQryID, this, std::placeholders::_1);
    m2mCmd["IF?"] = std::bind(&M2MC_CP::m2mQryFW, this, std::placeholders::_1);
    m2mCmd["IM?"] = std::bind(&M2MC_CP::m2mQryMfg, this, std::placeholders::_1);
    m2mCmd["IN?"] = std::bind(&M2MC_CP::m2mQryModel, this, std::placeholders::_1);
    m2mCmd["IS?"] = std::bind(&M2MC_CP::m2mQrySN, this, std::placeholders::_1);
    m2mCmd["IV?"] = std::bind(&M2MC_CP::m2mQryVer, this, std::placeholders::_1);
    m2mCmd["LD<"] = std::bind(&M2MC_CP::m2mSetALClevel, this, std::placeholders::_1);
    m2mCmd["L?"] =  std::bind(&M2MC_CP::m2mGetALClevel, this, std::placeholders::_1);
    m2mCmd["LG<"] = std::bind(&M2MC_CP::m2mSetAGClevel, this, std::placeholders::_1);
    m2mCmd["LG?"] = std::bind(&M2MC_CP::m2mGetAGClevel, this, std::placeholders::_1);
    m2mCmd["LM<"] = std::bind(&M2MC_CP::m2mSetMGClevel, this, std::placeholders::_1);
    m2mCmd["LM?"] = std::bind(&M2MC_CP::m2mGetMGClevel, this, std::placeholders::_1);
    m2mCmd["MC"] = std::bind(&M2MC_CP::m2mSHUTDOWN, this, std::placeholders::_1);

    // new multi-modes
    m2mCmd["MLC"] = std::bind(&M2MC_CP::m2mLoadCfgIndex, this, std::placeholders::_1);
    m2mCmd["MSC<"] = std::bind(&M2MC_CP::m2mSaveCfg, this, std::placeholders::_1);
    m2mCmd["MSC?"] = std::bind(&M2MC_CP::m2mGetCfg, this, std::placeholders::_1);
    m2mCmd["MSD"] = std::bind(&M2MC_CP::m2mStoreDefault, this, std::placeholders::_1);
    m2mCmd["MO"] =  std::bind(&M2MC_CP::m2mONLINE, this, std::placeholders::_1);
    m2mCmd["MO<"] = std::bind(&M2MC_CP::m2mTimedONLINE, this, std::placeholders::_1);
    m2mCmd["MS"] =  std::bind(&M2MC_CP::m2mSTANDBY, this, std::placeholders::_1);
    m2mCmd["M?"] =  std::bind(&M2MC_CP::m2mModeState, this, std::placeholders::_1);
    m2mCmd["NC"] =  std::bind(&M2MC_CP::m2mCloseNet, this, std::placeholders::_1);
    m2mCmd["NT<"] = std::bind(&M2MC_CP::m2mNetTimeout, this, std::placeholders::_1);
    m2mCmd["PF"] =  std::bind(&M2MC_CP::m2mGetFwdPwr, this, std::placeholders::_1);
    m2mCmd["PI"] =  std::bind(&M2MC_CP::m2mGetInpPwr, this, std::placeholders::_1);
    m2mCmd["PR"] =  std::bind(&M2MC_CP::m2mGetRevPwr, this, std::placeholders::_1);
    m2mCmd["PV"] =  std::bind(&M2MC_CP::m2mGetVSWR, this, std::placeholders::_1);
    m2mCmd["SA<"] = std::bind(&M2MC_CP::m2mSetAnt, this, std::placeholders::_1);
    m2mCmd["SA?"] = std::bind(&M2MC_CP::m2mGetAnt, this, std::placeholders::_1);
    m2mCmd["SS"] =  std::bind(&M2MC_CP::m2mGetSysStatus, this, std::placeholders::_1);
    m2mCmd["ST"] =  std::bind(&M2MC_CP::m2mGetSysTimes, this, std::placeholders::_1);
    m2mCmd["SUD"] = std::bind(&M2MC_CP::m2mSetUnitsToDBM, this, std::placeholders::_1);
    m2mCmd["SUW"] = std::bind(&M2MC_CP::m2mSetUnitsToWatts, this, std::placeholders::_1);
    m2mCmd["SU?"] = std::bind(&M2MC_CP::m2mGetUnits, this, std::placeholders::_1);
    m2mCmd["T?"] =  std::bind(&M2MC_CP::m2mGetSysTemps, this, std::placeholders::_1);
    m2mCmd["TR<"] = std::bind(&M2MC_CP::m2mSetTRswitch, this, std::placeholders::_1);
    m2mCmd["TR?"] = std::bind(&M2MC_CP::m2mGetTRswitch, this, std::placeholders::_1);
}

vector<string>  M2MC_CP::split(string str, char delimiter)
{
    vector<string> internal;
    stringstream ss(str); // Turn the string into a stream.
    string tok;

    while(getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }

    return internal;
}

void M2MC_CP::trim(string &cmd)
{
    string::size_type first = cmd.find_first_not_of(' ');
    string::size_type last = cmd.find_last_not_of(' ');
    if ((first==string::npos) && (last== string::npos)) //string contains only whitespaces
    {
        cmd="";
        return;
    }
    if (first==string::npos) //there are no leading whitespaces
        first=0;
    if (last==string::npos) //there are no trailing whitespaces
        last=cmd.length()-1;
    cmd = cmd.substr(first, last-first+1);
}


void M2MC_CP::sigHandler(int sig, siginfo_t *siginfo, void *context)
{

    printf("\n M2M  Caught Exception\n");

    // Print signal-specific info
    if ( (sig == SIGSEGV) || (sig == SIGINT)) {
        // Access the signal info
        std::cout << "Segmentation fault at address: " << siginfo->si_addr << std::endl;

        // Access the context info (optional, ARM-specific info can be retrieved here)
       // ucontext_t *uc = (ucontext_t*)context;

        // ARM specific information, you may inspect the program counter (pc), etc.
        // For ARM, the Program Counter is in uc_mcontext.arm_pc
       // std::cerr << "Program Counter (pc): " << (void*)uc->uc_mcontext.arm_pc << std::endl;

        // Print the backtrace (call stack)
        void *array[10];
        size_t size = backtrace(array, 10);
        char **symbols = backtrace_symbols(array, size);

        std::cout << "Backtrace:" << std::endl;
        for (size_t i = 0; i < size; ++i)
        {
            // Use abi demangling to get readable function names if possible
            int status;
            char *demangled = abi::__cxa_demangle(symbols[i], 0, 0, &status);
            if (status == 0)
            {
                std::cout << "  " << demangled << std::endl;
            } else
            {
                std::cout << "  " << symbols[i] << std::endl;
            }
            free(demangled);
        }
        free(symbols);
    }

    exit(1);
}

void M2MC_CP::init_exception()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof( struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction =  sigHandler;
    sa.sa_flags   = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGKILL, &sa, NULL);

}

void M2MC_CP::setZmqCtx( std::shared_ptr<zmq::context_t> ctx)
{
    sysMsgCtrl.setZmqSockets(ctx);
}

void M2MC_CP::setMsgFactoryConnection(std::shared_ptr<messageFactoryConnection > psmsgIf)
{
   sysMsgCtrl.setMsgFactoryConnection(psmsgIf);
}

void M2MC_CP::setStatusRequestIface(std::shared_ptr<statusRequestInterface > pStatusReqIface)
{
    sysMsgCtrl.setStatusRequestIface(pStatusReqIface);
}

void M2MC_CP::getSoftwareAndFirmware()
{

    FILE* pipe = popen("uname -a | grep RFSID", "r");
    std::ifstream file;
    file.open ("/empowerStatic/currentBundle.txt");
    if (pipe)
    {

        char buffer[128];
        std::string output = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            output += buffer;
        }
        fclose(pipe);

        std::regex rfsid_regex("RFSID=([0-9]+)");
        std::smatch match;

        if (std::regex_search(output, match, rfsid_regex))
        {
            RfsID = match[1];
        }

    }


    if (file.is_open())
    {
        std::string bundle;
        if (std::getline(file, bundle))
        {
            SoftwareBundle = bundle;
        }

    file.close();
    }

    FpgaHal  fpga(FpgaBase, FpgaMemSize);
    SysFW = std::to_string(fpga.getFirmwareVers());

}
//
//
//
void M2MC_CP::startNotificationThread()
{
    // Start Notification Thread
    if ( pthread_create (&notifThread, NULL,  notification_thread, (void *) this) == 0)
    {
        notifThreadRunning = true;
    }
    //
}
void  *M2MC_CP::notification_thread(void * ptr)
{
    M2MC_CP *m2mCp = reinterpret_cast < M2MC_CP * >  (ptr);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (m2mCp)
    {
        while (m2mCp->isnotifThreadRunning() )
        {
            string reply;
            // This is blocking call till a message is received
            if (m2mCp->sysMsgCtrl.getNotificationMsg(reply) )
            {
                if (!reply.empty())
                {
                   m2mCp->m2Mrespond( reply);
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
        }
    }

    return NULL;
}

void M2MC_CP::stopNotificationThread()
{

    if (notifThreadRunning) {
        notifThreadRunning = false;
        // Wait for the thread to actually finish its current 500ms poll
        pthread_join(notifThread, NULL);

    }

}
#pragma GCC diagnostic pop
