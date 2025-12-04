#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <cstring>
#include "fpgaDefs.h"
#include <helpers/debug.h>
#include <helpers/types.h>
#include <helpers/rfLayoutTypes.h>
#include <syslog.h>
#include "fpgaIO.h"

using namespace std;
using  namespace empower;
using namespace empower::helpers::types;

class FpgaHal: public FpgaIO {

    public:

    FpgaHal(unsigned int phys_addr , size_t Memsize):FpgaIO(phys_addr, Memsize) {};
    FpgaHal():FpgaIO(FpgaBase, FpgaMemSize) {};
     ~FpgaHal() = default;
     //
     using dbm_t = double;
     // No default, copy or move constructors

    void faultClear();
    void setRfOnOff( bool on );
    void setPsuOnOff( bool on );
    bool getRfState();
    bool getPsuState();
    // MGC
    void setMgcLevel(float level);
    unsigned int getMgcLevel ();
    //AGC
    void setAgcLevel(float level);
    unsigned int getAgcLevel ();
    //ALC
    void setAlcSetpoint(double level);
    unsigned int getAlcSetpoint ();
    // Power Conversion
    float getCfFwd() ;
    void setCfFwd(float pwr);
    //
    float getCfInput();
    //
    float getCfRev();
    //
    detector_t  getDetectorMode();
    unsigned int getFirmwareVers ();


    private:
    dbm_t  getAlcDynamicRange() noexcept
    {
       unsigned int val = readReg (FpgaPID::offset + FpgaPID::pid_alc_dyn_rng) ;
       return helpers::types::regValToDbm(val);

    }
    dbm_t  getNominalPower () noexcept
    {
       unsigned int val = readReg (FpgaRfManager::offset + FpgaRfManager::pwrNominal) ;
       return helpers::types::regValToDbm(val);
    }
    //



};

