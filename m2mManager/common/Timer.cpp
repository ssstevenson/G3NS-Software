
#include <string.h>
#include <time.h>
#include <signal.h>
#include "Timer.h"


using namespace std;

#define MSEC_PER_SEC 1000
#define NSEC_PER_MSEC (1000 * 1000)


Timer  *Timer::myInstance = NULL;


Timer  *Timer::getMyInstance()
{
    if (myInstance == NULL)
    {
        TIMER_HANLER handler = Timer::retransmit;
        myInstance = new Timer(TIMER_INTERVAL,  handler );
    }

    return myInstance;
}

Timer::Timer( int time_ms,   TIMER_HANLER  timeHandler):BaseTimer(timeHandler,0,time_ms)
    ,timer_time(time_ms),timerAccessLock(),rtxCount(0),timerThread(),mutex()
{
    rtxCount = 0;
    if (pthread_mutex_init(&timerAccessLock, NULL))
    {
        printf("%s():Failed to initialize the Access mutex.\n", __FUNCTION__);
    }
//    timer_time = time_ms;
    sem_init(&mutex, 0, 0);

    startThread();


}

Timer::~Timer()
{
}


void Timer::startThread()
{
    printf(".......starting Timer Thread for MO(time..)  .......\n");
    pthread_create (&timerThread, NULL,  timerMain, static_cast <void *> (this) );
}

void  *Timer::timerMain ( void *ptr)
{
   Timer *timer =  reinterpret_cast <Timer *> (ptr);


    while (1)
    {
        timer->notReady();

        if (timer->isTimerOn() )
        {
            printf("\n This is disabled as it is used for packet retransmission \n");
            break;
            // send MO command to MPA
            //string data="MO\n";
            // M2MControl::m2mONLINE(data);
            //  SosControl *sosCtrl = :SosControl::getMyInstance();
            // sosCtrl->sendSrvrMessageToCl( data.c_str(), data.size() );
            //  sosCtrl->sendUiUpdate();
        }
    }

    return NULL;
}

void Timer::retransmit( int  sig)
{
    sig=sig;
   Timer::getMyInstance()->ready();
    return ;
}

void BaseTimer::setTimerHandler( TIMER_HANLER  timeHandler)
{
    struct sigaction act;
    sigset_t set;

    sigemptyset( &set );
    sigaddset( &set, SIGALRM );

    act.sa_flags = 0;
    act.sa_mask = set;
    act.sa_handler = timeHandler;

    sigaction( SIGALRM, &act, NULL );
}

void BaseTimer::timerDelete()
{
    struct itimerspec tim_spec;

    tim_spec.it_interval.tv_sec = 0;
    tim_spec.it_interval.tv_nsec = 0;

    tim_spec.it_value.tv_sec = 0;
    tim_spec.it_value.tv_nsec = 0;

    if (::timer_settime(t_id, 0, &tim_spec, NULL))
        perror("timer_settime");

      timerOn = false;
      dbgprintf("[%s]--->Timer is deactivated \n", __FUNCTION__ );
}

void BaseTimer::setTimer(  int secp,  int msec)
{
    unsigned int  nsec = (static_cast <unsigned int> (msec) )*NSEC_PER_MSEC;
    unsigned int sec =  static_cast <unsigned int>  (secp);
      struct itimerspec tim_spec;

    if ( msec >= 1000 )
    {

        unsigned int rsec = static_cast <unsigned int> (msec) /1000;
        unsigned int rmsec = static_cast <unsigned int> (msec) - rsec*1000;
        nsec  = rmsec*NSEC_PER_MSEC;
        sec += rsec;
    }

    tim_spec.it_interval.tv_sec = sec;
    tim_spec.it_interval.tv_nsec = nsec;

    tim_spec.it_value.tv_sec = sec;
    tim_spec.it_value.tv_nsec = nsec;



    if (::timer_settime(t_id, 0, &tim_spec, NULL)) {
        perror("timer_settime");
    }
    else {
        timerOn = true;
    }

     dbgprintf("\n[%s] -Setting up Timer (Sec=%d, Msec=%d ) ...timer iis  %s \n", __FUNCTION__, sec,msec,  (timerOn==true?"ON":"FALSE") );
}


