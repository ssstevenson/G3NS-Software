#include <iostream>
#include <unistd.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wfloat-conversion"

#include "fpgaHal.h"

void  FpgaHal::faultClear()
{
    // FIXME  Is SOS System, need to send FC command to all boosters
    writeReg(FpgaFaultManager::offset + FpgaFaultManager::faultClear, static_cast< uint32_t> ( FpgaFaultManager::FaultClear::CLEARALL));
}
// MGC Get and Set
void  FpgaHal::setMgcLevel(float level)
{
   writeReg (FpgaPID::offset + FpgaPID::mgc_sp,static_cast< unsigned int > (level) );
}
//
unsigned int FpgaHal::getMgcLevel()
{
    return (readReg (FpgaPID::offset + FpgaPID::mgc_sp ));
}
 // AGC Get and Ste
void FpgaHal::setAgcLevel(float level)
{
   writeReg (FpgaPID::offset + FpgaPID::agc_sp,static_cast< unsigned int > (level) );
}
unsigned int FpgaHal::getAgcLevel ()
{
    return (readReg (FpgaPID::offset + FpgaPID::agc_sp) );
}

//ALC Set and get
void FpgaHal::setAlcSetpoint(double power)
{
    auto pnom  = getNominalPower();
    auto alcRange = getAlcDynamicRange();

    if((power <= pnom) && (power >= (pnom - alcRange) ) )
    {
       writeReg(FpgaPID::offset + FpgaPID::alc_sp, helpers::types::dbmToRegVal(power));
       syslog(LOG_DEBUG, "[%s]--->Writing value[0x%x] to register 0x%x \n",
              __FUNCTION__, helpers::types::dbmToRegVal(power) ,FpgaPID::offset + FpgaPID::alc_sp );
    }
    else
    {
       syslog(LOG_ERR, "[%s]--->ALC ERROR: target power[%f],pnom[%f],alcRange[%f] \n",
              __FUNCTION__, power,pnom,alcRange );
    }

}
unsigned int FpgaHal::getAlcSetpoint ()
{
    float power = readReg(FpgaPID::offset + FpgaPID::alc_sp);

    return helpers::types::dbmRawToDbm(static_cast<dbm_raw_t>(power));
}

//  RF Set and get
void  FpgaHal::setRfOnOff( bool on )
{
    writeReg (FpgaRfManager::offset + FpgaRfManager::rfEnable, static_cast<std::uint32_t> ( (on ? 1:0) ));
}
//
bool  FpgaHal::getRfState()
{
    return (static_cast < bool> (readReg (FpgaRfManager::offset + FpgaRfManager::rfEnable)) );
}
// PSU Set and Get
void  FpgaHal::setPsuOnOff( bool on )
{
   writeReg (FpgaPSU::offset + FpgaPSU::ctrl,  static_cast<std::uint32_t> ((on ? 1:0) ));
}
//
bool  FpgaHal::getPsuState()
{
    return ( static_cast < bool> (readReg (FpgaPSU::offset + FpgaPSU::ctrl) ) );
}

//
//    APis to get Forward, reverse and Input power
//
float  FpgaHal::getCfFwd()
{

    writeReg(FpgaDecMoveToPS::offset +  FpgaDecMoveToPS::snapreg_trig, static_cast<std::uint32_t> ( 1));
    short val, valr, valp, vale;


    valr =  static_cast < short> (readReg(FpgaDecMoveToPS::offset + FpgaDecMoveToPS::fwd_rms)) ;
    valp =  static_cast < short> (readReg(FpgaDecMoveToPS::offset + FpgaDecMoveToPS::fwd_peak)) ;
    vale =  static_cast < short> (readReg(FpgaDecMoveToPS::offset + FpgaDecMoveToPS::fwd_env)) ;

    switch (getDetectorMode())
    {
        case detector_t::RMS :
            val =  valr;
            break;
        case  detector_t::PEAK:
            val =  valp;
            break;
        default:
            val =  vale;
            break;

    }
    float powerf = regValToDbm(val);
    dbgprintf("[ FPGA %s]---> Converted Power [%d] --> [ %f]\n", __FUNCTION__,val, powerf );
    dbgprintf("[ FPGA %s]---> Converted Power: RMS [%f]   Peak [%f] ENV [%f] \n", __FUNCTION__, regValToDbm(valr) + regValToDbm(valp), regValToDbm(vale) );
    return powerf;


}
//
void FpgaHal::setCfFwd(float power)
{
    writeReg(FpgaPower::offset + FpgaPower::fwdOffset, dbmToRegVal(power));
}
//
float FpgaHal::getCfInput()
{

    // short  powerOffset =  static_cast < short> (readReg (FpgaPower::offset + FpgaPower::revOffset)) ;
    writeReg(FpgaDecMoveToPS::offset +  FpgaDecMoveToPS::snapreg_trig, 1);
    short val, valr, valp, vale;
    valr =  static_cast < short> (readReg(FpgaDecMoveToPS::offset + FpgaDecMoveToPS::inp_rms)) ;
    valp =  static_cast < short> (readReg(FpgaDecMoveToPS::offset + FpgaDecMoveToPS::inp_peak)) ;
    vale =  static_cast < short> (readReg(FpgaDecMoveToPS::offset + FpgaDecMoveToPS::inp_env)) ;

    switch (getDetectorMode())
    {
        case detector_t::RMS :
            val =  valr;
            break;
        case  detector_t::PEAK:
            val =  valp;
            break;
        default:
            val =  vale;
            break;

    }
    float powerf = regValToDbm(val);
    dbgprintf("[ FPGA %s]---> Converted Power [%d] --> [ %f]\n", __FUNCTION__,val, powerf );
    dbgprintf("[ FPGA %s]---> Converted Power: RMS [%f]   Peak [%f] ENV [%f] \n", __FUNCTION__, regValToDbm(valr) + regValToDbm(valp), regValToDbm(vale) );
    return powerf;
}
//
float FpgaHal::getCfRev()
{

    // short  powerOffset =  static_cast < short> (readReg (FpgaPower::offset + FpgaPower::revOffset)) ;
    writeReg(FpgaDecMoveToPS::offset +  FpgaDecMoveToPS::snapreg_trig, 1);

    short val, valr, valp, vale;
    valr =  static_cast < short> (readReg(FpgaDecMoveToPS::offset + FpgaDecMoveToPS::rev_rms)) ;
    valp =  static_cast < short> (readReg(FpgaDecMoveToPS::offset + FpgaDecMoveToPS::rev_peak)) ;
    vale =  static_cast < short> (readReg(FpgaDecMoveToPS::offset + FpgaDecMoveToPS::rev_env)) ;

    switch (getDetectorMode())
    {
        case detector_t::RMS :
            val =  valr;
            break;
        case  detector_t::PEAK:
            val =  valp;
            break;
        default:
            val =  vale;
            break;

    }
    float powerf = regValToDbm(val);
     dbgprintf("[ FPGA %s]---> Converted Power [%d] --> [ %f]\n", __FUNCTION__,val, powerf );
     dbgprintf("[ FPGA %s]---> Converted Power: RMS [%f]   Peak [%f] ENV [%f] \n", __FUNCTION__, regValToDbm(valr) + regValToDbm(valp), regValToDbm(vale) );
    return powerf;
}


detector_t  FpgaHal::getDetectorMode()
{
    unsigned int det =  readReg (FpgaPID::offset + FpgaPID::chan_sel);
    return regValToDetector(det);

}
 unsigned int FpgaHal::getFirmwareVers ()
 {
    return (readReg (FpgaInventory::offset + FpgaInventory::firmwareVers) );
 }

#pragma GCC diagnostic pop
