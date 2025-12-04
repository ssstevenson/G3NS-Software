#ifndef __SYS_CONFIG_H__
#define __SYS_CONFIG_H__


#define M2M_UDP_PORT         4000
#define M2M_TCP_PORT         3000
#define M2M_POLL_TIMEOUT     50



#define  RETRY_TIME_MS     1*1000   // Time between retransmitted messages
#define  UDP_POLL_TIMEOUT  3*1000
#define  TIMER_INTERVAL    5*1000    // Timer wake up every 1 seconds

#define  MAX_RTX_RETRY     10      // Maximum number to rtransmit message before daclaring it failing

#define  RETRAMISSION_ERROR   0x80000000

#define RS485_RESONSE_TIME  100*1000  // Wait for 100 ms
#define PSU_POLL_TIMEOUT    5       // Polling PSU every n seconds

#define MAX_M2M_MSG_LEN      (1024 - 32)
#define MAX_MQ_MSG           256



#define ETH_EXTERNAL        "eth0"
#define ETH_INTERNAL        "eth1"    //FIXME MARC!



#endif // END OF FILE
