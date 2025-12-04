/** @file */
/* =========================================================================*
* Copyright (C)Empower RF, 2021.  All rights reserved.                      *
*                                                                           *
*                                                                           *
* ==========================================================================*
*
*****************************************************************************
* Module Name: envMgrLCDUcmds.h
*
* Functional Description:
*
* This file defines Simpe  Messages Between LCC and Gen3 SOM
*
* Version History:
*
* Version   Date        Author          Description
* -------   ----------  ----------      --------------------------------
* 0         04/08/2021  Marc Obbad      Initial Release.
*
* 0.1       06/24/2021  Steve S         Ported into Env Mgr w/Renames and Cleanups
*
* 0.2       05/27/2022  Steve S         Updated for new CDU MODBUS Support
*
* 0.3       06/06/2022  Steve S         Updated for new CDU MODBUS Support
*
*****************************************************************************/
#ifndef __GEN_LCC_COMMANDS_H__
#define __GEN_LCC_COMMANDS_H__



#define  MAX_PUMP           4
#define  MAX_LOOP           2
#define  MAX_RESERVOIR      2
#define  MAX_MSG_SIZE       64   //64 * 4 (UNSIGNED INT )


// ////////////////////////////////////////////////
//
// This Enum define Shutodwn Reason oter than the obvious
// pnes such as LEAK
////////////////////////////////////////////////////
typedef enum {
    PUMP_ALARM_ERROR = 0x100,
    LOW_FLOW_ERROR,
    HI_TEMP_ERROR,
    PUMP_1_ERROR,
    PUMP_2_ERROR,
    PUMP_3_ERROR,
    PUMP_4_ERROR,
    RESERV_1_LEVEL_LOW_ERROR,
    RESERV_2_LEVEL_LOW_ERROR,
    RESERV_1_LEVEL_HIGH_ERROR,
    RESERV_2_LEVEL_HIGH_ERROR,
} SHUTDOWN_REASON;

//
typedef struct __attribute__ ((__packed__))
{
   unsigned int    EventDetected;
   unsigned int    loopID ;  // This is pump group [1]
   unsigned int    data[2];

} PUMP_EVENT_ERROR_S;
//
//
typedef struct __attribute__ ((__packed__))
{
    unsigned int    pumpID ;
    unsigned int    pumpFlowRaw;
    float           pumpFlow;
    unsigned int    pumpTempRaw;
    float           pumpTemp;

} PUMP_INFO_S;

typedef struct __attribute__ ((__packed__))
{
    unsigned int        loopID ;
    unsigned int        loopPressureRaw;
    float               loopPressure;
    unsigned int        loopTempRaw;
    float               loopTemp;

} PUMP_LOOP_S;

typedef struct __attribute__ ((__packed__))
{
    unsigned int    reservoirId;
    unsigned int    reservoirTempRaw;
    float           reservoirTemp;

} RESERVOIR_TEMP_S;

typedef struct __attribute__ ((__packed__))
{
    PUMP_INFO_S          pump[MAX_PUMP];
    PUMP_LOOP_S          loop[MAX_LOOP];
    RESERVOIR_TEMP_S     reservoir[MAX_RESERVOIR];

} SYSTEM_INFO_S;

typedef struct __attribute__ ((__packed__))
{
    unsigned int    Loop1Id ;
    unsigned int    Loop1IdState ; 
    unsigned int    Loop2Id ;
    unsigned int    Loop2IdState;

} PUMP_SYSTEM_STATUS_S;

typedef struct __attribute__ ((__packed__))
{
    unsigned int      commandID;  // Command send or received
    PUMP_SYSTEM_STATUS_S  status;

} SYSTEM_STATUS_MSG_S;

////////////////////////////////////////////////////
//   GENERIC MESSAGE FORMAT SENT OVE
////////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__))
{
    unsigned int      commandID;    // Command send or received
    unsigned int      commandSize;  // Not needed since we have few commands
    unsigned int      data[MAX_MSG_SIZE] ;

} GENERAL_COMMANDS;

////////////////////////////////////////////////////
//  MPDBUS related INFO
////////////////////////////////////////////////////
#define HEAD_TO_PSI                 (static_cast<float>(1.42197))
#define LITREPERSEC_TO_GALPERMIN    (static_cast<float>(15.8503))
#define CUBICMETER_TO_GALPERMIN     (static_cast<float>(4.403))

typedef enum
{
    E_PUMP_OPER_MODE_OFF,
    E_PUMP_OPER_MODE_ONE_PUMP_TWIN_MASTER
} MODBUS_OPER_MODE_REG_E;
//

typedef enum
{
    E_PUMP_CTRL_MODE_CONSTANT_PRESSURE   = 0x0001,
    E_PUMP_CTRL_MODE_PROPORTIIONAL_PRESSURE,
    E_PUMP_CTRL_MODE_CONSTANT_CURVE,
    E_PUMP_MAX_CTRL_MODE
} PUMP_CTRL_MODE_REG_E;

static std::string PUMP_CTRL_MODE_S[4] = { "UNDEFINED", "CONSTANT PRESSURE", "PROPORTIONAL PRESSURE", "CONSTANT CURVE" };
//const char *PUMP_CTRL_MODE_S[4] = { "UNDEFINED", "CONSTANT PRESSSURE", "PROPORTIONAL PRESSURE", "CONSTANT CURVE" };

//

typedef enum
{
    E_PUMP_NIGHT_MODE_NON_ACTIVE,
    E_PUMP_NIGHT_MODE_ACTIVE
} PUMP_NIGHT_MODE_REG_E;

typedef enum
{
    E_PUMP_AV_PROCEDURE_NON_ACTIVE,
    E_PUMP_AV_PROCEDURE_ACTIVE
} PUMP_AIR_VENTING_PROCEDURE_REG_E;

typedef enum
{
    E_PUMP_AV_POWER_ON_INACTIVE,
    E_PUMP_AV_POWER_ON_ACTIVE
} PUMP_AV_POWER_ON_REG_E;

/*
DUAL PUMP SETUP
*/
typedef enum
{
    E_PUMP_TWIN_MASTER,
    E_PUMP_TWIN_SLAVE,
    E_PUMP_SINGLE
} PUMP_CIRCULAR_CONFIG_REG_E;

static std::string CIRCULAR_CONFIG_S[]={ "TWIN MASTER", "TWIN SLAVE", "SINGLE" };
//const char *CIRCULAR_CONFIG_S[]={ "TWIN MASTER", "TWIN SLAVE", "SINGLE" };

typedef enum
{
    E_PUMP_TWIN_BACKUP,
    E_PUMP_TWIN_ALTERNALE,
    E_PUMP_TWIN_PARALLEL,
    E_PUMP_TWIN_FORCED_PARALLEL,
} PUMP_TWIN_CTRL_MODE_REG_E;

static std::string TWIN_CRTL_CONFIG_S[]={ "TWIN BACKUP ", "TWIN ALTERNATE", "TWIN PARALLEL", "TWIN FORCED PARALLEL" };
//const char *TWIN_CRTL_CONFIG_S[]={ "TWIN BACKUP ", "TWIN ALTERNATE", "TWIN PARALLEL", "TWIN FORCED PARALLEL" };

typedef struct __attribute__ ((__packed__))
{
    unsigned short  OperMode;
    unsigned short  ctrlMode;
    unsigned short  nightMode;
    unsigned short  airVentingProc;
    unsigned short  propPressureSetpoint;
    unsigned short  constPressureSetpoint;
    unsigned short  curvePressureSetpoint;
    unsigned short  airVentPowerOn;
} ModBusConfig1S;


typedef struct __attribute__ ((__packed__))
{
    unsigned short  circularConfigMode;
    unsigned short  twinCtrlMode;

} ModBusTwinMode_S;

typedef struct __attribute__ ((__packed__))
{
    unsigned short  InputPower;
    unsigned short  Head;
    unsigned short  Flow;
    unsigned short  Speed;
    unsigned short  WaterTemp;
    unsigned short  ExternalWaterTemp;
    unsigned short  Winding1Temp;
    unsigned short  Winding2Temp;
    unsigned short  Winding3Temp;
    unsigned short  PowerModuleTemp;
    unsigned short  QuadrantCurrent;
    unsigned short  BitFieldStatusIO;
    unsigned short  BitFieldAlarm1;
    unsigned short  BitFieldAlarm2;
    unsigned short  BitFieldErrors;
    unsigned short  ActiveErrorCode;
} ModBusPumpModBusStatus_S;

typedef struct  __attribute__ ((__packed__))
{
    unsigned short  lifeTimerLSW;
    unsigned short  lifeTimerMSW;
    unsigned short  PowerConsumptionLSW_25;
    unsigned short  PowerConsumptionMSW_25;
    unsigned short  PowerConsumptionLSW_50;
    unsigned short  PowerConsumptionMSW_50;
    unsigned short  PowerConsumptionLSW_75;
    unsigned short  PowerConsumptionMSW_75;
    unsigned short  PowerConsumptionLSW_100;
    unsigned short  PowerConsumptionMSW_100;
    unsigned short  currentIDxLog;
} PumpModBusLogCntTbl_S;

// Twin Slave Status
typedef struct  __attribute__ ((__packed__))
{
	unsigned short TwinSlaveRPM;
	unsigned short TwinSlaveStartStop;
	unsigned short TwinSlaveInputpower;
	unsigned short TwinSlaveHead;
	unsigned short TwinSlaveFlow;
	unsigned short TwinSlaveSpeed;
	unsigned short TwinSlaveWindingTemp1;
	unsigned short TwinSlaveWindingTemp2;
	unsigned short TwinSlaveWindingTemp3;
	unsigned short TwinSlavePowerModuleTemp;
	unsigned short TwinSlaveQuadratureCurrent;
	unsigned short TwinSlaveBitFieldAlarm1;
	unsigned short TwinSlaveBitFieldAlarm2;
	unsigned short TwinSlaveBitFieldErrors;
} TwinPumpModBusStatus_S;  


//const char *PumpBitErrorrs[15]=
static std::string PumpBitErrorrs[]=
{
    "INTERNAL COMM. LOST (E1)",
    "MOTOR OVERLOAD (E2)",
    "DC-BUS OVERVOLTAGE (E3)",
    "TRIP CONTROL ERROR (E4)",

    " EPROM MEMORY CORRUPTED ERROR (E5)",
    "GRID VOLTAGE ERROR (E6)",
    "MOTOR WINDING TEMPERATURE ERROR (E7)",
    "POWER MODULE TEMPERATURE ERROR (E8)",

    "NTC HW ERROR (E9)",
    "FACTORY DATA MEMORY CORRUPTED ERROR (E5)",
    "HYDRAULIC MAPS DATA MEMORY CORRUPTED ERROR (E5)",
    "DRY-RUN DETECT (E10)",

    "NTC POWER MODULE FAIL (E9)",
    "ROTOR BLOCKED (E4)",
    "MOTOR UNCONNECTED (E9)",
    " N.U"
};

#define MAX_ACTIVE_ERROR_CODE  11

//const char *PumpActiveErrors[]=
static std::string PumpActiveErrors[]=
{
    "NO ERROR",
    "INTERNAL COMM. LOST",
    "MOTOR OVERLOAD",
    "DC-BUS OVERVOLTAGE",
    "TRIP CONTROL ERROR",
    "DATA MEMORY CORRUPTED ERROR",
    "GRID VOLTAGE ERROR",
    "MOTOR_WINDING_TEMPERATURE_ERROR",
    "POWER MODULE TEMPERATURE ERROR",
    "GENERIC HW ERROR",
    "DRY-RUN DETECT"
};

typedef struct  __attribute__ ((__packed__))
{
    unsigned short              pumpID;
    ModBusConfig1S              PumpCFG;
    ModBusTwinMode_S            PumpTwinCFG;
    ModBusPumpModBusStatus_S    PumpStatus;
    PumpModBusLogCntTbl_S       PumpLogCntrTable;
	TwinPumpModBusStatus_S      SlavePumpStatus;
} PumpModBusInfo_S;

#define  PMOD_INFO_MSG_SIZE   (sizeof(PumpModBusInfo_S)/sizeof (unsigned short))

typedef union
{
    unsigned short data[PMOD_INFO_MSG_SIZE];
    PumpModBusInfo_S  PumpModBusInfoS;
} PumpModBusInfo_S_U;

typedef struct __attribute__ ((__packed__))
{
    unsigned int      commandID;
    PumpModBusInfo_S  modBusInfo;
} GENERAL_MODBUS_MSG_S;


// 
//  Cold Start 
//
enum COLD_START_STATUS 
{
    COLD_START_BYPASSED,  // Cold Start Bypassed -- Initial reservoir is good
    COLD_START_PASS,      // Cold Start succeeded
    COLD_START_FAILED     // Cold Start failed, see Failure Reason
};

enum COLD_START_FAILURE_REASON
{
    COLD_START_INITIAL_WARMUP_FAIL = 1,
    COLD_START_BOOSTER_WARMUP_FAIL,
};
typedef struct __attribute__((__packed__))
{
    unsigned int  status;               // casted as COLD_START_STATUS
    unsigned int  failReason;           // casted as COLD_START_FAILURE_REASON
    unsigned int  reservoirTemp;        // (last known reservoir temp)
    unsigned int  coldStartTemp;        // From config-- temperature to trigger cold start
    unsigned int  reservoirPreheatTime; // from configuration --Time in seconds to preheat reservoir
    unsigned int  InitialPreheatTemp;   // from configuration -- Reservoir good initial temp
    unsigned int  boosterPreheatTime;   // from configuration -- time in seconds to heat the transmitter
    unsigned int  pumpTempOperLimit;    // from configuration  -- Pumps lower temp limit to be turned on

} PUMP_COLD_START_STATUS_S;

typedef struct __attribute__ ((__packed__))
{
    unsigned int                   commandID;  // Command send or received
    PUMP_COLD_START_STATUS_S       coldStart;

} GENERAL_COLD_START_STATUS_MSG_S;



// Configuration Data
typedef struct __attribute__ ((__packed__))
{ 
    //
    //  Flow config and LowFlow Action
    //
    unsigned int            Loop1Flow;
    unsigned int            Loop2Flow;
    unsigned int            stopLoopOnLowFlow;
    unsigned int            pumpPurgeTimeInSeconds; 
    //
    // Events Actions:When a loop has an error, do we stop the other loop or not
    // 
    //
    unsigned int            shutdownLinePerRack;
    //
    //   Pump  Air Venting
    //
    unsigned int             nightModeOn;
    unsigned int             AirVentingProcedureOn;
    unsigned int             AirVentingPowerOn;
    //
    unsigned int             stopLoopOnRemoteOnOff_1;
    unsigned int             stopLoopOnRemoteOnOff_2; // NO HW yet for this
    //
    unsigned int             stopOnLeak;
    //
    unsigned int             loop1Active;
    unsigned int             stopLoopOnAccessoryShutDown_1;
    unsigned int             stopLoopOnExchagePressure_1;
    unsigned int             stopLoopOnfilterPressureSnsr_1; 
    //
    unsigned int             loop2Active;
    unsigned int             stopLoopOnAccessoryShutDown_2;
    unsigned int             stopLoopOnExchagePressure_2;
    unsigned int             stopLoopOnfilterPressureSnsr_2;
    //
    // Liquid To Air Fan
    unsigned int             startLiquidToAirExchanger;
    unsigned int             skipPumpRegulation;
    //
    
    //
    // ColdStart
    //
    unsigned int             coldStartEnabled;
    unsigned int             coldStartTriggerTemp;
    unsigned int             coldStartPreHeaTime;
    unsigned int             flushCoolantPreHeaTime;
    unsigned int             boosterPreHeaTime;
    unsigned int             PreHeatDelay;
    unsigned int             intialWarmupTemp;
    unsigned int             pumpOperTempLimit;
    //
	// Added Parameters to support BAD sensors reading
	unsigned int validFlowMin;
	unsigned int validFlowMax;
	unsigned int loop1ForceVoltage;
	unsigned int loop2ForceVoltage;
} CONFIGURATION_DATA_S;

#define  CONFIGURATION_DATA_S_LEN  (sizeof(CONFIGURATION_DATA_S)/4)
typedef union 
{
    CONFIGURATION_DATA_S config_data_u;
    unsigned int         data[CONFIGURATION_DATA_S_LEN];
} CONFIGURATION_DATA_U;


////////////////////////////////////////////////////
/// 202200606 - FROM v0.2 - MODBUS SUPPORT
/// TODO: SSS ADDIN

typedef enum {
    MODBUS_OPERATING_MODE = 0,
    MODBUS_CONTROL_MODE,
    MODBUS_NIGHT_MODE_ACTIVATION,
    MODBUS_AIR_VENTING_PROCEDURE,
    MODBUS_CONSTANT_CURVE_SETPOINT,
    MODBUS_AIR_VENTING_AT_POWER_ON,
    MODBUS_INPUT_POWER,
    MODBUS_HEAD,
    MODBUS_FLOW,
    MODBUS_SPEED,
    MODBUS_WATER_TEMPERATURE,
    MODBUS_WINDING_1_TEMPERATURE,
    MODBUS_WINDING_2_TEMPERATURE,
    MODBUS_WINDING_3_TEMPERATURE,
    MODBUS_POWER_MODULE_TEMPERATURE,
    MODBUS_QUADRANT_CURRENT,
    MODBUS_BIT_FIELDS_STATUS_IO,
    MODBUS_BIT_FIELDS_ALARM_1,
    MODBUS_BIT_FIELDS_ALARM_2,
    MODBUS_BIT_FIELDS_ERROR,
    MODBUS_ACTIVE_ERROR_CODE,
    MODBUS_CONSTANT_CURVE_MIN_SETPOINT,
    MODBUS_CONSTANT_CURVE_MAX_SETPOINT,
    MODBUS_COMMUNICATIONS_PROTOCOL,
    MODBUS_COMMUNICATIONS_BAUD_RATE,
    MODBUS_LIFETIME_TIMER_LSW,
    MODBUS_LIFETIME_TIMER_MSW,
    MODBUS_POWER_CONSUMPTION_0_25_LSW,
    MODBUS_POWER_CONSUMPTION_0_25_MSW,
    MODBUS_POWER_CONSUMPTION_25_50_LSW,
    MODBUS_POWER_CONSUMPTION_25_50_MSW,
    MODBUS_POWER_CONSUMPTION_50_75_LSW,
    MODBUS_POWER_CONSUMPTION_50_75_MSW,
    MODBUS_POWER_CONSUMPTION_75_100_LSW,
    MODBUS_POWER_CONSUMPTION_75_100_MSW,
    MODBUS_CURRENT_INDEX_LOG,

    MODBUS_MAX_PARM_IDX

} MODBUS_DATA_PARM_IDX_E;

typedef struct
{
    unsigned int TwinSlaveRPM;
    unsigned int TwinSlaveStartStop;
    unsigned int TwinSlaveInputpower;
    unsigned int TwinSlaveHead;
    unsigned int TwinSlaveFlow;
    unsigned int TwinSlaveSpeed;
    int          TwinSlaveWindingTemp1;
    int          TwinSlaveWindingTemp2;
    int          TwinSlaveWindingTemp3;
    int          TwinSlavePowerModuleTemp;
    unsigned int TwinSlaveQuadratureCurrent;
    unsigned int TwinSlaveBitFieldAlarm1;
    unsigned int TwinSlaveBitFieldAlarm2;
    unsigned int TwinSlaveBitFieldErrors;
} MB_TWIN_DATA_S;



typedef struct
{
    bool            newData;
    bool            OperMode;
    bool            nightMode;
    bool            airVentPowerOn;

    unsigned int    pumpID;
    unsigned int    ctrlMode;
    unsigned int    airVentProcedure;
    unsigned int    circularConfigMode;
    unsigned int    twinCtrlMode;
    unsigned int    InputPower;
    unsigned int    Speed;


    unsigned int    QuadrantCurrent;
    unsigned int    BitFieldStatusIO;
    unsigned int    BitFieldAlarm1;
    unsigned int    BitFieldAlarm2;
    unsigned int    BitFieldErrors;
    unsigned int    ActiveErrorCode;

    // LOG messages
    unsigned int    LifeTimerLSW;
    unsigned int    LifeTimerMSW;

    unsigned int    PowerConsumptionLSW_25;
    unsigned int    PowerConsumptionMSW_25;
    unsigned int    PowerConsumptionLSW_50;
    unsigned int    PowerConsumptionMSW_50;
    unsigned int    PowerConsumptionLSW_75;
    unsigned int    PowerConsumptionMSW_75;
    unsigned int    PowerConsumptionLSW_100;
    unsigned int    PowerConsumptionMSW_100;
    unsigned int    CurrentIDxLog;


    int             Winding1Temp;
    int             Winding2Temp;
    int             Winding3Temp;
    int             PowerModuleTemp;


    float           propPressureSetpoint;
    float           constPressureSetpoint;
    float           curvePressureSetpoint;
    float           Head;
    float           Flow;
    float           WaterTemp;
    float           ExtwaterTemp;

    MB_TWIN_DATA_S  TwinPumpData;

} MODBUS_REPORT_DATA_S;






////////////////////////////////////////////////////
/// 20220527 - FROM v0.1
/// TODO: SSS ADDIN
typedef struct
{
    unsigned int        pumpID ;
    unsigned int        pumpFlowRaw;
    float               pumpFlow;
    unsigned int        pumpTempRaw;
    float               pumpTemp;

} PUMP_INFO_SU;

typedef struct
{
    unsigned int        loopID ;
    unsigned int        loopPressureRaw;
    float               loopPressure;
    unsigned int        loopTempRaw;
    float               loopTemp;

} PUMP_LOOP_SU;

typedef struct
{
    unsigned int        reservoirId;
    unsigned int        reservoirTempRaw;
    float               reservoirTemp;

} RESERVOIR_TEMP_SU;

typedef struct
{
    bool                PUMP_LEAK;
    bool                PHASE_ERROR;
    bool                EMERGENCY_STOP;
    bool                ACC1_SHUTDOWN;
    bool                ACC2_SHUTDOWN;
    bool                REMOTE_ON_OFF;
    bool                LOOP1_EXCHNG_PRES;
    bool                LOOP1_FILTER_PRES;
    bool                LOOP2_EXCHNG_PRES;
    bool                LOOP2_FILTER_PRES;
    bool                RESERVOIR1_FULL;
    bool                RESERVOIR1_LOW;
    bool                RESERVOIR2_FULL;
    bool                RESERVOIR2_LOW;
    bool                PUMP1_ALARM;
    bool                PUMP2_ALARM;
    bool                PUMP3_ALARM;
    bool                PUMP4_ALARM;

    bool                ALARM_UPDATE;

} SYSTEM_ALARM_DATA_S;



typedef struct
{
    SYSTEM_ALARM_DATA_S     ALARM_STATE;
    bool                    PUMP_INFO_AVAIL[MAX_PUMP];
    bool                    LOOP_INFO_AVAIL[MAX_LOOP];
    bool                    LOOP_1_ON;
    bool                    LOOP_2_ON;
    PUMP_INFO_SU            PUMP_INFO[MAX_PUMP];
    PUMP_LOOP_SU            LOOP_INFO[MAX_LOOP];
    RESERVOIR_TEMP_SU       RESV_TEMP[MAX_RESERVOIR];

} SYSTEM_REPORT_DATA_S;


/// TODO : SSS Saved for reference - REMOVE AFTER TEST
typedef struct
{
    bool                PUMP_LEAK;
    bool                PHASE_ERROR;
    bool                EMERGENCY_STOP;
    bool                ACC1_SHUTDOWN;
    bool                ACC2_SHUTDOWN;
    bool                REMOTE_ON_OFF;
    bool                LOOP1_EXCHNG_PRES;
    bool                LOOP1_FILTER_PRES;
    bool                LOOP2_EXCHNG_PRES;
    bool                LOOP2_FILTER_PRES;
    bool                RESERVOIR1_FULL;
    bool                RESERVOIR1_LOW;
    bool                RESERVOIR2_FULL;
    bool                RESERVOIR2_LOW;
    bool                PUMP1_ALARM;
    bool                PUMP2_ALARM;
    bool                PUMP3_ALARM;
    bool                PUMP4_ALARM;
    bool                PUMP_INFO_AVAIL[MAX_PUMP];
    bool                LOOP_INFO_AVAIL[MAX_LOOP];
    PUMP_INFO_SU        PUMP_INFO[MAX_PUMP];
    PUMP_LOOP_SU        LOOP_INFO[MAX_LOOP];
    RESERVOIR_TEMP_SU   RESV_TEMP[MAX_RESERVOIR];

} SYSTEM_REPORT_DATA_S_PREV;



///
////////////////////////////////////////////////////


////////////////////////////////////////////////////
///
//
////////////////////////////////////////////////////
//typedef struct __attribute__ ((__packed__))
//{
//    unsigned int      commandID;    // Command send or received
//    unsigned int      commandSize;  // Not needed since we have few commands
//    unsigned int      data[MAX_MSG_SIZE] ;
//
//} GENERAL_COMMANDS;
//


#define TLV_SYSTEM_CONFIGURATION     0x0200
#define TLV_SYSTEM_NOTIFICATION      0x0300
#define TLV_SYSTEM_REQUEST           0x0400

#define TLV_SYSTEM_ERROR_REPORT      0x0900


//
// Configuration Messages
//
#define TLV_SYSTEM_LOOP_NUMBER                  TLV_SYSTEM_CONFIGURATION  + 0   // Number of LOOPS--Harcoded to two
#define TLV_SYSTEM_PUMP_PER_LOOP                TLV_SYSTEM_CONFIGURATION  + 1   // Number of pumps per loop -- Hardcoded to two

#define TLV_SYSTEM_CONGIURATION_SETUP           TLV_SYSTEM_CONFIGURATION  + 100 
#define TLV_SYSTEM_CONGIURATION_READ            TLV_SYSTEM_CONFIGURATION  + 102   // Request Configuration Data or response
#define TLV_SYSTEM_CONGIURATION_ERROR           TLV_SYSTEM_CONFIGURATION  + 103   // REquest Configuration Data
////////////////////////////////////////////////////////////
//
// SYSTEM EVENTS, ALARMS AND NOTIFICATIONS
// SENT TO GEN3 CONTROLLER EVERY x seconds
//  These are commands set in commandID field of the message.
//  Every messaye type is associated with a data structure
/////////////////////////////////////////////////////////////

#define TLV_PUMP_STATUS                     TLV_SYSTEM_NOTIFICATION + 0    // using PUMP_INFO_S as message body
#define TLV_LOOP_STATUS                     TLV_SYSTEM_NOTIFICATION + 1    // using PUMP_LOOP_S .......
#define TLV_RESERVOIR_STATUS                TLV_SYSTEM_NOTIFICATION + 2    // using RESERVOIR_TEMP_S  .....
#define TLV_PUMP_LEAK                       TLV_SYSTEM_NOTIFICATION + 3    // no mesage body, system shutdown
#define TLV_PUMP_ALARM                      TLV_SYSTEM_NOTIFICATION + 4    // no mesage body, does not cause shutdown unles flow is low
#define TLV_PUMP_PHASE_ERROR                TLV_SYSTEM_NOTIFICATION + 5    // no mesage body, system shutdown
#define TLV_SYSTEM_EMERGENCY_STOP           TLV_SYSTEM_NOTIFICATION + 6    // no mesage body, system shutdown
#define TLV_SYSTEM_ACCESSORY_SHUTDOWN       TLV_SYSTEM_NOTIFICATION + 7    // no mesage body, system shutdown
#define TLV_SYSTEM_ACCESSORY_2_SHUTDOWN     TLV_SYSTEM_NOTIFICATION + 8    // no mesage body, system shutdown
#define TLV_SYSTEM_REMOTE_ON_OFF            TLV_SYSTEM_NOTIFICATION + 9    // no mesage body, system shutdown
#define TLV_SYSTEM_EXCHANGE_PRESSURE        TLV_SYSTEM_NOTIFICATION + 10   // data[0]  has which EXCHANGER 1 or 2   system shutdown
#define TLV_SYSTEM_FILTER_PRESSURE          TLV_SYSTEM_NOTIFICATION + 12   // data[0]  has which FILTER    1 or 2   system shutdown
#define TLV_SYSTEM_RESERVOIR_FULL           TLV_SYSTEM_NOTIFICATION + 14   // data[0]   has  which RESERVOIR 1 or 2 does not cause shutdown
#define TLV_SYSTEM_RESERVOIR_LOW            TLV_SYSTEM_NOTIFICATION + 16   // data[0]   has which RESERVOIR 1 or 2 does not cause shutdown unles flow is low

#define TLV_SYSTEM_SHUTDOWN                 TLV_SYSTEM_NOTIFICATION + 100   // data[0] contains reason for shutwon as defined in SHUTDOWN_REASON

#define TLV_SYSTEM_INFORMATION              TLV_SYSTEM_NOTIFICATION + 200   // using  SYSTEM_INFO_S as body

#define TLV_SYSTEM_RESTART                  TLV_SYSTEM_NOTIFICATION + 300    // This indicate Pump has restarted and all events should be set to default
//
//  ModBus messages
#define TLV_SYSTEM_MODBUS_INFO              TLV_SYSTEM_NOTIFICATION + 400    // This indicate Pump has restarted and all events should be set to default

//
#define TLV_LOOP_COLD_START_STATUS           TLV_SYSTEM_NOTIFICATION + 500    // cold Start Status evenst
//

#define   TLV_GET_STATUS_INFORMATION       TLV_SYSTEM_REQUEST  + 1      // Sent by Gen3 Controller to Pump Controller
#define   TLV_SET_STATUS_RATE              TLV_SYSTEM_REQUEST  + 2      // data shoud have rate in seconds
#define   TLV_SET_PUMPS_ON                 TLV_SYSTEM_REQUEST  + 3      // Turn Pumps On
#define   TLV_SET_PUMPS_OFF                TLV_SYSTEM_REQUEST  + 4      // Turn Pumps Off

#define   TLV_SET_LOOP1_ON                 TLV_SYSTEM_REQUEST  + 100    // Turn Loop 1  ON
#define   TLV_SET_LOOP2_ON                 TLV_SYSTEM_REQUEST  + 101    // Turn Loop 2  ON
#define   TLV_SET_LOOP1_OFF                TLV_SYSTEM_REQUEST  + 102    // Turn Loop 1  OFF
#define   TLV_SET_LOOP2_OFF                TLV_SYSTEM_REQUEST  + 103    // Turn Loop 2  OFF



#endif /* __COMMANDS_H__ */
