/** @file */
/* =========================================================================*
* Copyright (C)Empower RF, 2021.  All rights reserved.                      *
*                                                                           *
*                                                                           *
* ==========================================================================*
*
*****************************************************************************
* Module Name: Commands.h
*
* Functional Description:
*
* This file defines Simpe  Messages Between LCC and Gen3 SOM
*
* Version History:
*
* Version  Date        Author         Description
* -------  ----------  ----------     --------------------------------
* 0       04/08/2021 Marc Obbad       Initial Release.
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
    int             reservoirId;
    unsigned int    reservoirTempRaw;
    float           reservoirTemp;

} RESERVOIR_TEMP_S;

typedef struct __attribute__ ((__packed__))
{
    PUMP_INFO_S          pump[MAX_PUMP];
    PUMP_LOOP_S          loop[MAX_LOOP];
    RESERVOIR_TEMP_S     reservoir[MAX_RESERVOIR];

} SYSTEM_INFO_S;

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
#define HEAD_TO_PSI (1.42197f)
#define LITREPERSEC_TO_GALPERMIN (15.8503f)
typedef enum
{
    E_PUMP_OPER_MODE_OFF,
    E_PUMP_OPER_MODE_ONE_PUMP_TWIN_MASTER
} MODBUS_OPER_MODE_REG_E;
//

typedef enum
{
    E_PUMP_CTRL_MODE_CONSTANT_PRESSURE = 0x0001,
    E_PUMP_CTRL_MODE_PROPORTIIONAL_PRESSURE,
    E_PUMP_CTRL_MODE_CONSTANT_CURVE,
    E_PUMP_MAX_CTRL_MODE
} PUMP_CTRL_MODE_REG_E;

const char *PUMP_CTRL_MODE_S[4] = { "UNDEFINED", "CONSTANT PRESSSURE", "PROPORTIONAL PRESSURE", "CONSTANT CURVE" };

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

const char *CIRCULAR_CONFIG_S[]={ "TWIN MASTER", "TWIN SLAVE", "SINGLE" };

typedef enum
{
    E_PUMP_TWIN_BACKUP,
    E_PUMP_TWIN_ALTERNALE,
    E_PUMP_TWIN_PARALLEL,
    E_PUMP_TWIN_FORCED_PARALLEL,
} PUMP_TWIN_CTRL_MODE_REG_E;

const char *TWIN_CRTL__CONFIG_S[]={ "TWIN BACKUP ", "TWIN ALTERNATE", "TWIN PARALLEL", "TWIN FORCED PARALLEL" };

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
    //
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
    unsigned short  PowerConsuptionLSW_25;
    unsigned short  PowerConsuptionMSW_25;
    unsigned short  PowerConsuptionLSW_50;
    unsigned short  PowerConsuptionMSW_50;
    unsigned short  PowerConsuptionLSW_75;
    unsigned short  PowerConsuptionMSW_75;
    unsigned short  PowerConsuptionLSW_100;
    unsigned short  PowerConsuptionMSW_100;
    unsigned short  currentIDxLog;
} PumpModBusLogCntTbl_S;

const char *PumpBitErrorrs[15]=
{
    "INTERNAL COMM. LOST (E1)",
    "MOTOR OVERLOAD (E2)",
    "DC-BUS OVERVOLTAGE (E3)",
    "TRIP CONTROL ERROR (E4)",

    " EPROM MEMORY CORRUPTED ERROR (E5)"
    "GRID VOLTAGE ERROR (E6)",
    "MOTOR WINDING TEMPERATURE ERROR (E7)",
    "POWER MODULE TEMPERATURE ERROR (E8)"

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
const char *PumpActiveErrors[MAX_ACTIVE_ERROR_CODE]=
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
} PumpModBusInfo_S;

#define PMOD_INFO_MSG_SIZE (sizeof(PumpModBusInfo_S)/sizeof (unsigned short))

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

// Configuration Data

typedef struct __attribute__ ((__packed__))
{
    //
    //  Flow config and LowFlow Action
    //
    unsigned int    Loop1Flow;
    unsigned int    Loop2Flow;
    unsigned int    stopLoopOnLowFlow;
    unsigned int    pumpPurgeTimeInSeconds;
    //
    // Events Actions:When a loop has an error, do we stop the other loop or not
    //
    //
    unsigned int    shutdownLinePerRack;
    //
    //   Pump  Air Venting
    //
    unsigned int    nightModeOn;
    unsigned int    AirVentingProcedureOn;
    unsigned int    AirVentingPowerOn;
    //
    unsigned int    stopLoopOnRemoteOnOff_1;
    unsigned int    stopLoopOnRemoteOnOff_2; // NO HW yet for this
    //
    unsigned int    stopOnLeak;
    //
    unsigned int    loop1Active;
    unsigned int    stopLoopOnAccessoryShutDown_1;
    unsigned int    stopLoopOnExchagePressure_1;
    unsigned int    stopLoopOnfilterPressureSnsr_1;
    //
    unsigned int    loop2Active;
    unsigned int    stopLoopOnAccessoryShutDown_2;
    unsigned int    stopLoopOnExchagePressure_2;
    unsigned int    stopLoopOnfilterPressureSnsr_2;
    //
    // Liquid To Air Fan
    unsigned int    startLiquidToAirExchanger;
    unsigned int    skipPumpRegulation;
    //

    //
    // ColdStart
    //
    unsigned int    coldStartEnabled;
    unsigned int    coldStartTriggerTemp;
    unsigned int    coldStartPreHeaTime;
    unsigned int    flushCoolantPreHeaTime;
    unsigned int    bosterPreHeaTime;
    unsigned int    PreHeatDelay;
    unsigned int    intialWarmupTemp;
    unsigned int    pumpOperTempLimit;
    //
} CONFIGURATION_DATA_S;

typedef struct __attribute__ ((__packed__))
{
     unsigned int    Loop1Id ;
     unsigned int    Loop1IdState ;
     unsigned int    Loop2Id ;
     unsigned int    Loop2IdState;

} PUMP_SYSTEM_STATUS_S;

#define  CONFIGURATION_DATA_S_LEN  (sizeof(CONFIGURATION_DATA_S)/4)
typedef union
{
    CONFIGURATION_DATA_S config_data_u;
    unsigned int         data[CONFIGURATION_DATA_S_LEN];
} CONFIGURATION_DATA_U;


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
#define TLV_SYSTEM_CONGIURATION_READ            TLV_SYSTEM_CONFIGURATION  + 102   // REquest Configuration Data

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


#define   TLV_GET_STATUS_INFORMATION       TLV_SYSTEM_REQUEST  + 1      // Sent by Gen3 Controller to Pump Controller
#define   TLV_SET_STATUS_RATE              TLV_SYSTEM_REQUEST  + 2      // data shoud have rate in seconds


#define   TLV_SET_PUMPS_ON                 TLV_SYSTEM_REQUEST  + 3      // Turn CDU  On
#define   TLV_SET_PUMPS_OFF                TLV_SYSTEM_REQUEST  + 4      // Turn CDU  Off

#define   TLV_SET_LOOP1_ON                 TLV_SYSTEM_REQUEST  + 100      // Turn Loop 1  ON
#define   TLV_SET_LOOP2_ON                 TLV_SYSTEM_REQUEST  + 101      // Turn Loop 2  ON
#define   TLV_SET_LOOP1_OFF                TLV_SYSTEM_REQUEST  + 102      // Turn Loop 1  OFF
#define   TLV_SET_LOOP2_OFF                TLV_SYSTEM_REQUEST  + 103      // Turn Loop 2  OFF

#endif /* __COMMANDS_H__ */
