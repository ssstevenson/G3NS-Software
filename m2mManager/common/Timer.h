/** @file */
/* =========================================================================*
* Copyright (C)Empower RF, 2025.  All rights reserved.                      *
*                                                                           *
*                                                                           *
* ==========================================================================*
*
*****************************************************************************
* Module Name: Timer Signal
*
* Functional Description:
*
* This file defines the retransmission timer ...
*
* Version History:
*
* Version  Date        Author         Description
* -------  ----------  ----------     --------------------------------
* 0       1/10/2025   Marc Obbad       Initial Release.
*
*****************************************************************************/
#ifndef __SOS_TIMER_H__
#define __SOS_TIMER_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>  /* Semaphore */

#include <helpers/debug.h>
using namespace std;

#define RETX_TIMER 500
#define  TIMER_INTERVAL    5*1000

typedef   void (*TIMER_HANLER)(int);
typedef   void (*TIMER_CALL_BACK_)(union sigval  arg);

class BaseTimer {

public:

        BaseTimer(TIMER_CALL_BACK_ hanle , int timeS, int timeM):t_id(),timeInSec(timeS),timeInMsec(timeM),timerCreated(false),timerOn(false),oneTime(true)
        {
            struct sigevent se;
            se.sigev_notify = SIGEV_THREAD;
            se.sigev_value.sival_ptr =this;
            se.sigev_notify_function = hanle;
            se.sigev_notify_attributes = NULL;

            if  (( timer_create(CLOCK_MONOTONIC, &se, &t_id)) == 0 )
                timerCreated = true;
        }

        BaseTimer(TIMER_HANLER hanle , int timeS, int timeM):t_id(),timeInSec(timeS),timeInMsec(timeM),timerCreated(false),timerOn(false),oneTime(true)
        {
            setTimerHandler (hanle) ;
            if  (( timer_create(CLOCK_MONOTONIC, NULL, &t_id)) == 0 )
                timerCreated = true;
        }
        
        BaseTimer(const BaseTimer &) = delete;
        BaseTimer& operator=(const BaseTimer&)= delete;

        virtual ~BaseTimer(){  timerDelete(); timer_delete(t_id) ;};
        void timerDelete();

    void startTimer() { setTimer(timeInSec,timeInMsec);};
    void stopTimer() { timerDelete(); };
    bool isTimerOn() { return timerOn; };
    void setTimeOn( bool on) { timerOn = on; };
    bool isOneTime() { return oneTime;};
    void SetOneTime( bool onOff) { oneTime = onOff;};
    timer_t  getTimerId () { return t_id; };

private:
    timer_t t_id;
    void setTimerHandler( TIMER_HANLER  timeHandler);
    void setTimer(  int sec,  int msec);
    int timeInSec, timeInMsec;
    bool timerCreated;
    bool  timerOn;
    bool  oneTime;

};


class Timer : public BaseTimer
{
    public:

        static Timer  *getMyInstance();
        ~Timer();
        
        Timer( const Timer& ) = delete;
        Timer &operator= (const Timer&)= delete;
        //
        void startThread();
        static void  *timerMain ( void *ptr);

        void notReady() { sem_wait(&mutex); };
        void ready() { sem_post(&mutex); };
        //
        //
        int getRtxCount() {
            int count;
            lock();
            count = rtxCount;
            unlock();
            return count;
        }
        void setRtxCount(int count) {

            lock();
            rtxCount = count;
            unlock();
        }

    private:
        int timer_time;
        static void  retransmit( int  sig);
        pthread_mutex_t    timerAccessLock;
        void lock() {pthread_mutex_lock(&timerAccessLock); };
        void unlock() {  pthread_mutex_unlock(&timerAccessLock); };
        int rtxCount;
        pthread_t           myThread;
        sem_t   mutex;
        static Timer  *myInstance;
        Timer( int time_us,   TIMER_HANLER  timeHandler);   // As singeleton for now

};


#endif /* __COMMANDS_H__ */
