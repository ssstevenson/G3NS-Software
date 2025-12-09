
#pragma once
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

#define SHM_NAME "/fpgaAccessMutex" // Shared memory for mutex

using namespace std;
using  namespace empower;
using namespace empower::helpers::types;

class FpgaIO {

    public:

        FpgaIO(unsigned int phys_addr, size_t msize );
        FpgaIO():FpgaIO(FpgaBase,FpgaMemSize) {};
        ~FpgaIO();
        //
        FpgaIO(const FpgaIO&) = delete;
        FpgaIO& operator=(const FpgaIO&) = delete;

        // Move operations also disabled (optional, but recommended)
        FpgaIO(FpgaIO&&) = delete;
        FpgaIO& operator=(FpgaIO&&) = delete;

    public:  // low level can move to  private
        unsigned int  readReg ( unsigned int  regOffset );
        void writeReg ( unsigned int  regOffset, unsigned int  value );
        void setRegBits ( unsigned int  regOffset, unsigned int  bitMask );
        void clearRegBits ( unsigned int  regOffset, unsigned int  bitMask );
        void writeReg ( unsigned int  regOffset, unsigned int  value, unsigned int  mask );


        bool validateRegOffset (unsigned int regOffset)
        {
            return ( (sharedMutex) && (fpgaMappedMemByte != 0 ) && (regOffset <=fpgaMappedMax) );
        }

      private:
        int fd;
        int shmFd;
        unsigned int  fpgaAddr;
        size_t fpgaMemSize;
        unsigned int  fpgaMappedMax;
        unsigned char   *fpgaMappedMemByte;
        void fpgaUnmapMemory();
        void fpgaInitSharedMutex();

       struct SharedMutex
        {
            pthread_mutex_t mutex;
            int initialized;
        };
       SharedMutex *sharedMutex;
};

