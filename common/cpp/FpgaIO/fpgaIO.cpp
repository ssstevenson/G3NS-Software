#include <iostream>
#include <unistd.h>
#include <time.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored  "-Wreorder"

#include "fpgaIO.h"


FpgaIO::FpgaIO(unsigned int  baseAddr, size_t size)
    : fd{-1},
      shmFd{-1},
      fpgaAddr{baseAddr},
      fpgaMemSize{size},
      fpgaMappedMemByte{nullptr},
      fpgaMappedMax{0},
      sharedMutex{}
{
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open(/dev/mem) failed");
        return;
    }

    unsigned int  pageSize = static_cast<unsigned int > (sysconf(_SC_PAGESIZE));
    if (pageSize <= 0) {
        perror("Invalid page size");
        close(fd);
        return;
    }

    unsigned int  pageMask = pageSize - 1;
    unsigned int  pageAddr = baseAddr & ~pageMask;
    unsigned int  nextPageAddr = baseAddr + size;
    if (nextPageAddr % pageSize != 0)
        nextPageAddr += pageSize - (nextPageAddr % pageSize);

    fpgaMemSize = nextPageAddr - pageAddr;

    void *mappedBaseAddr = mmap(nullptr, fpgaMemSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,  (fpgaAddr) );
    if (mappedBaseAddr == MAP_FAILED) {
        perror("FPGA mmap failed");
            close(fd);
        mappedBaseAddr = nullptr;
        return;
    }
    fpgaMappedMax = fpgaMemSize-4;
    fpgaMappedMemByte  = reinterpret_cast < unsigned char *  > (mappedBaseAddr);
    fpgaInitSharedMutex();
    dbgprintf("[%s]---> Physical address =0x%llx, size =0x%lx \n", __FUNCTION__,baseAddr, size );
    dbgprintf("[%s]---> mapped to  address =0x%llx, size = 0x%lx \n", __FUNCTION__,fpgaMappedMemByte, fpgaMemSize );

}

FpgaIO::~FpgaIO()
{
    if (fpgaMappedMemByte)
        fpgaUnmapMemory();
    if (fd >= 0)
        close(fd);
    if (shmFd >= 0)
    shm_unlink(SHM_NAME);
}

// Unmap memory
void FpgaIO::fpgaUnmapMemory()
{
    if (fpgaMappedMemByte && munmap(reinterpret_cast < void * > (fpgaMappedMemByte), fpgaMemSize) == -1)
        perror("munmap");
}

void FpgaIO::fpgaInitSharedMutex()
{
    bool created = false;
    shmFd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR , 0666);
    if (shmFd >= 0)
    {
        created = true;
    }
    else if (errno == EEXIST)
    {
        shmFd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (shmFd < 0)
        {
            perror("shm_open error opening existing FPGA Mutex");
            syslog(LOG_ERR, "[%s]---> Error opening existing FPGA Mutex \n", __FUNCTION__ );
            return;
        }
    }
    else
    {
        perror("shm_open... Unable to Create FPGA MUTEX ");
        syslog(LOG_ERR, "[%s]---> Unable to Create FPGA MUTEX \n", __FUNCTION__ );
        return;
    }

    if (ftruncate(shmFd, sizeof(SharedMutex)) != -1)
    {
        void* baseAddr = mmap(nullptr, sizeof(SharedMutex),
                              PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
        if (baseAddr != MAP_FAILED)
        {
            sharedMutex = static_cast<SharedMutex*>(baseAddr);
            close(shmFd);

            if (created) {
                memset(sharedMutex, 0, sizeof(SharedMutex));
                __sync_synchronize();
                pthread_mutexattr_t attr;
                pthread_mutexattr_init(&attr);
                pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
                pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
                pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
                if (pthread_mutex_init(&sharedMutex->mutex, &attr) != 0 )
                {
                    syslog(LOG_ERR, " [%s] pthread_mutex_init failed!", __FUNCTION__);
                }
                pthread_mutexattr_destroy(&attr);
                 __sync_synchronize();
                sharedMutex->initialized = 1;
                __sync_synchronize();
            }
            else
            {
                int retries= 20;
                while (sharedMutex->initialized != 1 )
                {
                    usleep(500);
                    if ( retries -- <= 0 )
                    {
                        syslog(LOG_ERR, "[%s]---> Unable to Create FPGA MUTEX \n", __FUNCTION__ );
                        break;  /* Should never happen unless process  creator crashes */
                    }
                }
            }

        }
        else
        {
            perror("mmap failed for FPGA Mutex ");
            syslog(LOG_ERR, "[%s]---> mmap failed for FPGA Mutex \n", __FUNCTION__ );
            close(shmFd);
        }
    }
    else
    {
        perror("ftruncate failed for FPGA Mutex ");
        syslog(LOG_ERR, "[%s]---> ftruncate failed for FPGA Mutex \n", __FUNCTION__ );
        close(shmFd);
    }

}

unsigned int  FpgaIO::readReg ( unsigned int  regOffset )
{
    unsigned int   value = 0xFFFFFFFF;

   //
    if ( validateRegOffset(regOffset) )
    {
       //

        unsigned int *regAddress = reinterpret_cast < unsigned int *> ( (fpgaMappedMemByte + regOffset) );
        volatile unsigned  int   *reg = reinterpret_cast <  volatile unsigned int  *> (regAddress);

        struct timespec ts;
        // Set timeout to 500ms from now
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 500000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }

        int lock_status = pthread_mutex_timedlock(&sharedMutex->mutex, &ts);

        if (lock_status == ETIMEDOUT) {
            syslog(LOG_ERR, "[FPGAIO %s] Mutex Timeout! Hardware or Lock stuck.", __FUNCTION__);
            return 0xFFFFFFFF;
        }

        // int lock_status = pthread_mutex_lock(&sharedMutex->mutex);
        if (lock_status == 0 || lock_status == EOWNERDEAD)
        {
            if (lock_status == EOWNERDEAD)
            {
                syslog(LOG_WARNING, "[FPGAIO %s]---> Mutex owner issues--Making it consistent.", __FUNCTION__);
                pthread_mutex_consistent(&sharedMutex->mutex);
            }
            value =  *reg;
        }
        else
        {
            // Handle other lock issues
            syslog(LOG_ERR, "[FPGAIO %s]---> Mutex lock failed with status %d", __FUNCTION__, lock_status);
        }
        pthread_mutex_unlock(&sharedMutex->mutex);
    }
    else
    {
       syslog(LOG_ERR, "[%s]---> !!! Failed   reading FPGA reg =0x%x !!!!!", __FUNCTION__,regOffset );
    }
     syslog(LOG_DEBUG, "[ FPGAIO %s]---> Reading [FPGA base = 0x%08x ] register Offset 0x%08x  ---Value=0x%08x", __FUNCTION__, fpgaAddr, regOffset, value );

    return value;
}
//////
//////

void  FpgaIO::writeReg ( unsigned int  regOffset, unsigned int   value )
{

    if ( validateRegOffset(regOffset) )
    {
        //
        unsigned int *regAddress = reinterpret_cast < unsigned int *> (  (fpgaMappedMemByte + regOffset) );
        volatile unsigned  int   *reg = reinterpret_cast <  volatile unsigned int  *> (regAddress);
        int lock_status = pthread_mutex_lock(&sharedMutex->mutex);
        if (lock_status == 0 || lock_status == EOWNERDEAD)
        {
            if (lock_status == EOWNERDEAD)
            {
                syslog(LOG_WARNING, "[FPGAIO %s]---> Mutex owner issues--Making it consistent.", __FUNCTION__);
                pthread_mutex_consistent(&sharedMutex->mutex);
            }
            *reg = value;
        }
        else
        {
            // Handle other lock issues
            syslog(LOG_ERR, "[FPGAIO %s]---> Mutex lock failed with status %d", __FUNCTION__, lock_status);
        }

        pthread_mutex_unlock(&sharedMutex->mutex);

    }
    else
    {
        syslog(LOG_ERR, "[FPGAIO %s]---> !!! Failed   reading FPGA reg =0x%x !!!!!", __FUNCTION__,regOffset );
    }

    syslog(LOG_DEBUG, "[ FPGAIO %s]---> [FPGA base = 0x%08x ]  writing 0x%08x  to register Offset 0x%08x ", __FUNCTION__, fpgaAddr, value, regOffset );


}
//
//
//
void FpgaIO::writeReg ( unsigned int  regOffset, unsigned int  Newvalue, unsigned int  mask )
{
     unsigned int *regAddress = reinterpret_cast < unsigned int *> (  (fpgaMappedMemByte + regOffset) );
    unsigned int  value = 0;
    if ( validateRegOffset(regOffset) )
    {
        volatile unsigned  int   *reg = reinterpret_cast < volatile unsigned int  *> (regAddress);
        int lock_status = pthread_mutex_lock(&sharedMutex->mutex);
        if (lock_status == 0 || lock_status == EOWNERDEAD)
        {
            if (lock_status == EOWNERDEAD)
            {
                syslog(LOG_WARNING, "[FPGAIO %s]---> Mutex owner issues--Making it consistent.", __FUNCTION__);
                pthread_mutex_consistent(&sharedMutex->mutex);
            }
            value = *reg;
            value &=  ~mask;   // clear all bit fields
            value |=  (Newvalue & mask) ;   // Add new value and apply mask
            *reg = value;

        }
        else
        {
            // Handle other lock issues
            syslog(LOG_ERR, "[FPGAIO %s]---> Mutex lock failed with status %d", __FUNCTION__, lock_status);
        }

        pthread_mutex_unlock(&sharedMutex->mutex);
    }
    else
    {
        perror(" FpgaIO::writeReg(...,Mask)-->sharedMutex: ");
    }
    syslog(LOG_DEBUG, "[ FPGAIO %s]---> [FPGA base = 0x%08x ]  writing 0x%08x  to register 0x%08x [ Mask=0x%08x ]  ", __FUNCTION__, fpgaAddr, value, regOffset, mask  );
}
//
//
//
void FpgaIO::setRegBits ( unsigned int  regOffset, unsigned int  bitMask )
{
    unsigned int *regAddress = reinterpret_cast < unsigned int *> (  (fpgaMappedMemByte + regOffset) );
    //
    if ( validateRegOffset(regOffset) )
    {
        volatile unsigned  int   *reg = reinterpret_cast < volatile unsigned int  *> (regAddress);
        int lock_status = pthread_mutex_lock(&sharedMutex->mutex);
        if (lock_status == 0 || lock_status == EOWNERDEAD)
        {
            if (lock_status == EOWNERDEAD)
        {
            syslog(LOG_WARNING, "[%s]---> Mutex owner issues--Making it consistent.", __FUNCTION__);
            pthread_mutex_consistent(&sharedMutex->mutex);
        }
        unsigned int  value = *reg;
        value &=  ~bitMask; // clear all bit fields
        value |=  bitMask;  // set  all bit fields
        *reg = value;

        }
        else
        {
            // Handle other lock issues
            syslog(LOG_ERR, "[%s]---> Mutex lock failed with status %d", __FUNCTION__, lock_status);
        }
        pthread_mutex_unlock(&sharedMutex->mutex);
    }
    else
    {
    perror(" FpgaIO::writeReg()-->sharedMutex: ");
    }

}
void FpgaIO::clearRegBits ( unsigned int  regOffset, unsigned int  bitMask )
{
    unsigned int regAddress = static_cast < unsigned int > ( reinterpret_cast<std::uintptr_t> (fpgaMappedMemByte + regOffset) );
    //
    if ( validateRegOffset(regOffset) )
    {
        volatile unsigned  int   *reg = reinterpret_cast < volatile unsigned int  *> (regAddress);
        int lock_status = pthread_mutex_lock(&sharedMutex->mutex);
        if (lock_status == 0 || lock_status == EOWNERDEAD)
        {
            if (lock_status == EOWNERDEAD)
            {
                syslog(LOG_WARNING, "[%s]---> Mutex owner issues--Making it consistent.", __FUNCTION__);
                pthread_mutex_consistent(&sharedMutex->mutex);
            }

            unsigned int  value =  ( *reg);
            value &=  ~bitMask; // clear all bit fields
            *reg = value;


        }
        else
        {
            // Handle other lock issues
            syslog(LOG_ERR, "[%s]---> Mutex lock failed with status %d", __FUNCTION__, lock_status);
        }

        pthread_mutex_unlock(&sharedMutex->mutex);
    }
    else
    {
        perror(" FpgaIO::writeReg()-->sharedMutex: ");
    }
}

#pragma GCC diagnostic pop
