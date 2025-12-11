/*!
 * \file
 * \brief M2M Command Processor
 *
 ===============================================================================

 Name        : M2M Command Processor ( m2mProcessor.cpp )
 Author      : Marc Obbad
 Version     :

 Description :  This module is responsible for Processing M2M JSON Commands.
                Commands are received over ZeroMQ. They are processed and a reply is sent back.

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
#include <JsonAPI/jsonFormatter.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored  "-Wreorder"
#pragma GCC diagnostic ignored  "-Wdouble-promotion"

#include "m2mProcessor.h"


void M2MProcessor::m2mRun()
{
    string jsonCmd = "UNKOWN";
    int sequenceNumber = 0;
    float val = 0.0f;
    string result;
    string reply;
    JsonFormater jsonString;
    while (true /* !stop_m2mthread*/)
    {
        syslog(LOG_DEBUG,"[%s]-Waiting for IPC message from M2M", __FUNCTION__);
        dbgprintf("[%s]-Waiting for IPC message from M2M\n", __FUNCTION__);
        string  m2mMsg = getM2MCommand();
        jsonCmd = "UNKOWN";
        result="complete";
        val = 0.0f;
        if (!m2mMsg.empty() )
        {
            //
            string vals;
            if (jsonString.parseMessage(m2mMsg, jsonCmd, sequenceNumber, val, vals) )
            {
                   syslog(LOG_DEBUG,"[%s]- IPC message from M2M : %s [size=%lu]", __FUNCTION__, jsonCmd.c_str(),jsonCmd.size());
                   // call API bases on jsonCmd
                   if( cmdAPI.count(jsonCmd) > 0 )
                   {
                       // access map API to process command
                       cmdAPI.at(jsonCmd)(val, vals);
                       if ( !vals.empty() ) {
                           result = vals;
                       }
                       // Prepare a reply and send it
                   }
                   else
                   {
                       result="?";
                   }
            }
        }

      reply = jsonString.createSimpleMessage(jsonCmd,sequenceNumber,val,result);
      syslog(LOG_DEBUG,"[%s]-sending reply:: %s", __FUNCTION__,  reply.c_str());
      dbgprintf("[%s]-sending reply : %s\n", __FUNCTION__,reply.c_str());
      sendReply(reply);
    }
}


void M2MProcessor::setCentralFrequency(float &val, string &msg)
{
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::setBandwidth(float &val, string &msg)
{
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}

void M2MProcessor::getCentralFrequency(float &val, string &msg)
{
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::getBandwidth(float &val, string &msg)
{
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::disableAutoRF(float &val, string &msg)
{
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::enableAutoRF(float &val, string &msg)
{
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::getAutoRFState(float &val, string &msg)
{
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}

void M2MProcessor::setFastDetectionMode(float &val, string &msg)
{
  // Fixme Steve help
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::setSlowDetectionMode(float &val, string &msg)
{
     // Fixme  Steve help
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::getDetectionModeState(float &val, string &msg)
{
    msg = detectorToStr(fpga.getDetectorMode());
}
void M2MProcessor::faultClear(float &val, string &msg)
{
   // Fixme Marc Need to push FC to all booster
   fpga.faultClear();
}

void M2MProcessor::setAlcLevel(float &val, string &msg)
{
    fpga.setAlcSetpoint(static_cast < double> (val));
}
void M2MProcessor::getAlcLevel(float &val, string &msg)
{
  val = fpga.getAlcSetpoint();
   syslog(LOG_DEBUG,"\n [%s] ---AALC REG =[%f]:", __FUNCTION__,  val );
}
void M2MProcessor::setAgcLevel(float &val, string &msg)
{
   fpga.setAgcLevel (val);
}
void M2MProcessor::getAgcLevel(float &val, string &msg)
{
   val =  static_cast<float> (fpga.getAgcLevel ());
   syslog(LOG_DEBUG,"\n [%s] ---AGC REG =[%f]:", __FUNCTION__,  val );
}
void M2MProcessor::setMgcLevel(float &val, string &msg)
{
    fpga.setMgcLevel (rfCtrl.getMaxVva()*(val/100) );
}
void M2MProcessor::getMgcLevel(float &val, string &msg)
{
    float  power = static_cast<float> (fpga.getMgcLevel()) ;
    float  mgcMax =  static_cast<float> (rfCtrl.getMaxVva());
    if (mgcMax > 0.0 )
        val =  static_cast<float> ( (power / mgcMax) * 100.0);

    syslog(LOG_DEBUG,"\n [%s] ---MGC REG=[%f]: MGC MAX VVA=[%f], Percentage VAl=[%f]", __FUNCTION__,power ,mgcMax,  val );
}




void M2MProcessor::coldStandBy(float &val, string &msg)
{
    unsigned long regOffset;
    unsigned long value;

    // Set RF off
    setRfOnOff(false);
   // Set PSU off
   setPsuOnOff(false);

}
void M2MProcessor::online(float &val, string &msg)
{
   if (!getPsuState() )
   {
       setPsuOnOff(true);
   }
   setRfOnOff(true);
}
void M2MProcessor::hotStandBy(float &val, string &msg)
{
    if (!getPsuState() )
    {
        setPsuOnOff(true);
    }
    setRfOnOff(false);
}
void M2MProcessor::getModeState(float &val, string &msg)
{
    auto rfState = [] (const int rfEnable, const int psuCtrl)
    {
        return (rfEnable != 0) ? std::string("online") :
               (psuCtrl != 0) ? std::string("standby") :
               std::string("shutdown");
    };
    msg = rfState(getRfState(), getPsuState());
}

void M2MProcessor::getForwardPower(float &val, string &msg)
{
  val = fpga.getCfFwd();
}
void M2MProcessor::getInputPower(float &val, string &msg)
{
  val = fpga.getCfInput();
}
void M2MProcessor::getReversePower(float &val, string &msg)
{
  val = fpga.getCfRev();
}
//
void M2MProcessor::getVswrState(float &val, string &msg)
{
    // Fixme never implemented
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::switchAntenna(float &val, string &msg)
{
    // Fixme Steve
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::getCurrentAntenna(float &val, string &msg)
{
    // Fixme Steve
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::SystemStatus(float &val, string &msg)
{
  msg = formSystemStatus();
}

void M2MProcessor::setUnitsToDBm(float &val, string &msg)
{
    // Fixme never implemented
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::setUnitsToWatts(float &val, string &msg)
{
    // Fixme never implemented
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::getUnits(float &val, string &msg)
{
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::getSystemTemp(float &val, string &msg)
{
     // Fixme No need for ZeroMQ, this is a parameter
  syslog(LOG_DEBUG,"\n [%s] ---Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::setTRSwitch(float &val, string &msg)
{
    // Fixme Steve
  syslog(LOG_DEBUG,"\n [%s] -Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}
void M2MProcessor::getTRSwitch(float &val, string &msg)
{
    // Fixme Steve
  syslog(LOG_DEBUG,"\n [%s] -Received val =%f Message: %s", __FUNCTION__,val,  msg.c_str());
}



void M2MProcessor::saveConfig(float &val, string &msg)
{
     // Fixme Steve using processMSC(msc)
    string mscMessage{msg};
    // Create an MSC struct and use a lambda to parse and fill the struct
    MSC msc;

    auto paramsToMsc = [&msc, &val](const string& str) {
        stringstream ss(str);
        string item;
        vector<string> paramss;

        while (getline(ss, item, ',')) {
            paramss.push_back(item);
        }

        if (paramss.size() == static_cast<int>(val) ) {
            msc.index = stoi(paramss[0]);
            msc.ctrl = paramss[1];
            msc.startup = paramss[2];
            msc.modulation = paramss[3];
            msc.detectionMode = paramss[4];
            msc.power = paramss[5];
            msc.unit = paramss[6];
            msc.par = stoi(paramss[7]);
            msc.outputLevel = stof(paramss[8]);
        } else
        {
           syslog(LOG_ERR," [%s] -Number of Params  =%d,  Expecting : 9 ", __FUNCTION__,static_cast<int>(val));
        }
    };

    // Use the lambda to fill the MSC struct with values from the string
    paramsToMsc(mscMessage);
    processMSC(msc);
    msg = std::string("complete");
}



void M2MProcessor::getConfig(float &val, string &msg)
{
     // Fixme Steve
    msg = formMSC();
}

//##############################################################################################
//
//   HELPERS FUNCTION to Process complex commands
//
//###############################################################################################

void M2MProcessor::processMSC(MSC &msc)
{
    // Fixme Steve
    std::function<std::string(const std::string&)> getStartupMode = [](const std::string& mode) -> std::string {
        static const unordered_map<std::string, std::string> modeMap = {
            {"C", "Cold"},
            {"S", "StandBy"},
            {"O", "Online"}
        };
        auto it = modeMap.find(mode);
        return it != modeMap.end() ? it->second : "Unknown";
    };

    std::function<std::string(const std::string&)> getDetectionMode = [](const std::string& mode) -> std::string {
        static const unordered_map<std::string, std::string> modeMap = {
            {"F", "Fast peak"},
            {"S", "Slow peak"},
            {"R", "RMS"}
        };
        auto it = modeMap.find(mode);
        return it != modeMap.end() ? it->second : "Unknown";
    };

    std::function<std::string(const std::string&)> getPowerMode = [](const std::string& mode) -> std::string {
        static const unordered_map<std::string, std::string> modeMap = {
            {"A", "ALC"},
            {"G", "AGC"},
            {"M", "MGC"}
        };
        auto it = modeMap.find(mode);
        return it != modeMap.end() ? it->second : "Unknown";
    };

    cout << "Processing  of  MSC Parameters Not Implemented:" << endl;
    cout << "       index        : " << msc.index << endl;
    cout << "       ctrl mode    : " << (msc.ctrl == "R" ? "Remote" : "Local") << endl;
    cout << "       startup mode : " << getStartupMode(msc.startup) << endl;
    cout << "       modulation   : " << msc.modulation << endl;
    cout << "       detectionMode: " << getDetectionMode(msc.detectionMode) << endl;
    cout << "       power mgmt   : " << getPowerMode(msc.power) << endl;
    cout << "       unit         : " << (msc.unit == "D" ? "dBm" : "Watts") << endl;
    cout << "       PAR/Carriers : " << msc.par << endl;
    cout << "       output Level : " << msc.outputLevel << endl;
}
//
//
//
string M2MProcessor::formSystemStatus()
{
    // Fixme Steve -- return a reply as default one, dont add SS
    string reply{"SNL 00 0000"};  // default values

    auto rfState = [] (const int rfEnable, const int psuCtrl)
    {
        return (rfEnable != 0) ? 'O' :
               (psuCtrl != 0) ? 'S' : 'C';

    };
    reply.at(0) = rfState(getRfState(), getPsuState());

    return reply;
}
//
//
//

string M2MProcessor::formMSC()
{
     // Fixme Steve , return a string as the reply below
    string reply{"(1, R, S, CW, S, A, D, 255.0, 55.50)"};  // default values

    return reply;
}

/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
///   Bind Json commands received to associatd command API for  faster processing
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void  M2MProcessor::initCommand()
{


    cmdAPI["setCentralFrequency"] = std::bind(&M2MProcessor::setCentralFrequency, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["setBandwidth"] = std::bind(&M2MProcessor::setBandwidth, this, std::placeholders::_1,std::placeholders::_2);

    cmdAPI["getCentralFrequency"] = std::bind(&M2MProcessor::getCentralFrequency, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getBandwidth"] = std::bind(&M2MProcessor::getBandwidth, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["disableAutoRF"] = std::bind(&M2MProcessor::disableAutoRF, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["enableAutoRF"] = std::bind(&M2MProcessor::enableAutoRF, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getAutoRFState"] = std::bind(&M2MProcessor::getAutoRFState, this, std::placeholders::_1,std::placeholders::_2);
    // m2mToMessage["B<"] =  std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    // m2mToMessage["B?"] =  std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    // m2mToMessage["BIT"] =  std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["setFastDetectionMode"] = std::bind(&M2MProcessor::setFastDetectionMode, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["setSlowDetectionMode"] = std::bind(&M2MProcessor::setSlowDetectionMode, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getDetectionModeState"] = std::bind(&M2MProcessor::getDetectionModeState, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["faultClear"] = std::bind(&M2MProcessor::faultClear, this, std::placeholders::_1,std::placeholders::_2);
    //m2mToMessage["FO<"]  = std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    //m2mToMessage["ID?"]  =  std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    //m2mToMessage["IF?"]  =  std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    //m2mToMessage["IM?"]  =  std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    // m2mToMessage["IN?"] = std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    // m2mToMessage["IS?"] =  std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    // m2mToMessage["IV?"] =  std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["setAlcLevel"] = std::bind(&M2MProcessor::setAlcLevel, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getAlcLevel"] = std::bind(&M2MProcessor::getAlcLevel, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["setAgcLevel"] = std::bind(&M2MProcessor::setAgcLevel, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getAgcLevel"] = std::bind(&M2MProcessor::getAgcLevel, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["setMgcLevel"] = std::bind(&M2MProcessor::setMgcLevel, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getMgcLevel"] = std::bind(&M2MProcessor::getMgcLevel, this, std::placeholders::_1,std::placeholders::_2);


    // new multi-modes
    //m2mToMessage["MLC"] = std::bind(&M2MC_CP::m2mLoadCfg, this, std::placeholders::_1,std::placeholders::_2) = std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    //m2mToMessage["MSD"] = std::bind(&M2MC_CP::m2mStoreDefault, this, std::placeholders::_1,std::placeholders::_2) = std::bind(&M2MProcessor::setCentralFreq, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["saveConfig"] = std::bind(&M2MProcessor::saveConfig, this, std::placeholders::_1,std::placeholders::_2);  // MSC<......>
    cmdAPI["getConfig"] = std::bind(&M2MProcessor::getConfig, this, std::placeholders::_1,std::placeholders::_2);   // MSC?

    cmdAPI["coldStandBy"] = std::bind(&M2MProcessor::coldStandBy, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["online"] = std::bind(&M2MProcessor::online, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["hotStandBy"] = std::bind(&M2MProcessor::hotStandBy, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getModeState"] = std::bind(&M2MProcessor::getModeState, this, std::placeholders::_1,std::placeholders::_2);

    cmdAPI["getForwardPower"] = std::bind(&M2MProcessor::getForwardPower, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getInputPower"] = std::bind(&M2MProcessor::getInputPower, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getReversePower"] = std::bind(&M2MProcessor::getReversePower, this, std::placeholders::_1,std::placeholders::_2);
    //
    cmdAPI["getVswrState"] = std::bind(&M2MProcessor::getVswrState, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["switchAntenna"] = std::bind(&M2MProcessor::switchAntenna, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getCurrentAntenna"] = std::bind(&M2MProcessor::getCurrentAntenna, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["systemStatus"] = std::bind(&M2MProcessor::SystemStatus, this, std::placeholders::_1,std::placeholders::_2);

    cmdAPI["setUnitsToDBm"] = std::bind(&M2MProcessor::setUnitsToDBm, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["setUnitsToWatts"] = std::bind(&M2MProcessor::setUnitsToWatts, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getUnits"] = std::bind(&M2MProcessor::getUnits, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getSystemTemp"] = std::bind(&M2MProcessor::getSystemTemp, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["setTRSwitch"] = std::bind(&M2MProcessor::setTRSwitch, this, std::placeholders::_1,std::placeholders::_2);
    cmdAPI["getTRSwitch"] = std::bind(&M2MProcessor::getTRSwitch, this, std::placeholders::_1,std::placeholders::_2);
}
