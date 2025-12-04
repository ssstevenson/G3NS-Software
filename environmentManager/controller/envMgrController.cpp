/*!
* \file
* \brief Environmental Manager Controller CORE Class
*
-------------------------------------------------------------------------------

 Name        : envMgrController.cpp

 Author      : Steve Stevenson

 Version     : v 00.90.00 [ Initial Dev Build ]

 Description : Environmental Manager Controller CORE Class Definitions

-------------------------------------------------------------------------------

Copyright   : (C) Copyright 2018 - 2020 Empower RF Systems

-------------------------------------------------------------------------------
*
*
*
*
*/

#include "envMgrController.h"

#include <helpers/jsonUpdateHelpers.h>
#include <logger/logger.h>
#include <numeric>
#include <bitset>
#include <iostream>

using namespace empower;

envMgrController::envMgrController(zmq::context_t& zmqCtx, configManagerConnection& config):
/// ***************************************************************************
/// Summary:
///
///     Class constructor method
///
/// Inputs:
///
///     config - Configuration Interface reference
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///

    // INITIALIZERS CALLS
    //
    configIF{config},
    updateIF{zmqCtx},
    requestIF{zmqCtx},
    fpgaIF{zmqCtx},
    workingDoc{},
    cduClient{std::string("172.16.0.70"), uint16_t(9000)},
    SystemTypeInfo{},
    EnviroCfg{},
    PumpCfg{},
    ThermalBand{},
    CurrThermalBand{0},

    mapCfgParm{
        {"FHI_MOD_INVENTORY"                            , FHI_MOD_INVENTORY                            },
        {"FHI_MOD_FAN_CTRL"                             , FHI_MOD_FAN_CTRL                             },
        {"ENV_AIR_COOL_ENABLE"                          , ENV_AIR_COOL_ENABLE                          },
        {"ENV_AIR_COOL_MANUAL_ENABLE"                   , ENV_AIR_COOL_MANUAL_ENABLE                   },
        {"ENV_AIR_COOL_MANUAL_SETPOINT"                 , ENV_AIR_COOL_MANUAL_SETPOINT                 },
        {"ENV_AIR_COOL_MAXIMUM_THRESHOLD"               , ENV_AIR_COOL_MAXIMUM_THRESHOLD               },
        {"ENV_AIR_COOL_MINIMUM_THRESHOLD"               , ENV_AIR_COOL_MINIMUM_THRESHOLD               },
        {"ENV_AIR_COOL_OPERATIONAL_BANDS"               , ENV_AIR_COOL_OPERATIONAL_BANDS               },
        {"ENV_AIR_COOL_MINIMUM_OPERATIONAL_BAND"        , ENV_AIR_COOL_MINIMUM_OPERATIONAL_BAND        },
        {"ENV_AIR_COOL_FAN_PWM_FREQUENCY"               , ENV_AIR_COOL_FAN_PWM_FREQUENCY               },
        {"ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_UP"    , ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_UP    },
        {"ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_DOWN"  , ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_DOWN  },
        {"ENV_AIR_COOL_SAMPLE_PERIOD"                   , ENV_AIR_COOL_SAMPLE_PERIOD                   },
        {"ENV_AIR_COOL_TACHOMETER_SAMPLE_PERIOD"        , ENV_AIR_COOL_TACHOMETER_SAMPLE_PERIOD        },
        {"ENV_AIR_COOL_FAN_1_PRESENCE"                  , ENV_AIR_COOL_FAN_1_PRESENCE                  },
        {"ENV_AIR_COOL_FAN_1_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_1_PULSES_PER_REV            },
        {"ENV_AIR_COOL_FAN_2_PRESENCE"                  , ENV_AIR_COOL_FAN_2_PRESENCE                  },
        {"ENV_AIR_COOL_FAN_2_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_2_PULSES_PER_REV            },
        {"ENV_AIR_COOL_FAN_3_PRESENCE"                  , ENV_AIR_COOL_FAN_3_PRESENCE                  },
        {"ENV_AIR_COOL_FAN_3_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_3_PULSES_PER_REV            },
        {"ENV_AIR_COOL_FAN_4_PRESENCE"                  , ENV_AIR_COOL_FAN_4_PRESENCE                  },
        {"ENV_AIR_COOL_FAN_4_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_4_PULSES_PER_REV            },
        {"ENV_AIR_COOL_FAN_5_PRESENCE"                  , ENV_AIR_COOL_FAN_5_PRESENCE                  },
        {"ENV_AIR_COOL_FAN_5_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_5_PULSES_PER_REV            },
        {"ENV_AIR_COOL_FAN_6_PRESENCE"                  , ENV_AIR_COOL_FAN_6_PRESENCE                  },
        {"ENV_AIR_COOL_FAN_6_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_6_PULSES_PER_REV            },
        {"ENV_AIR_COOL_FAN_7_PRESENCE"                  , ENV_AIR_COOL_FAN_7_PRESENCE                  },
        {"ENV_AIR_COOL_FAN_7_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_7_PULSES_PER_REV            },
        {"ENV_AIR_COOL_FAN_8_PRESENCE"                  , ENV_AIR_COOL_FAN_8_PRESENCE                  },
        {"ENV_AIR_COOL_FAN_8_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_8_PULSES_PER_REV            },
        {"ENV_AIR_COOL_FAN_8_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_8_PULSES_PER_REV            },
        {"ENV_AIR_COOL_FAN_8_PULSES_PER_REV"            , ENV_AIR_COOL_FAN_8_PULSES_PER_REV            },

        {"ENV_LIQUID_COOL_ENABLE"                       , ENV_LIQUID_COOL_ENABLE                       },
        {"ENV_LIQUID_COOL_MAX_TEMPERATURE_THRESHOLD"    , ENV_LIQUID_COOL_MAX_TEMPERATURE_THRESHOLD    },
        {"ENV_LIQUID_COOL_MIN_TEMPERATURE_THRESHOLD"    , ENV_LIQUID_COOL_MIN_TEMPERATURE_THRESHOLD    },
        {"ENV_LIQUID_COOL_TEMPERATURE_SAMPLE_PERIOD"    , ENV_LIQUID_COOL_TEMPERATURE_SAMPLE_PERIOD    },
        {"ENV_LIQUID_COOL_MAX_PRESSURE_THRESHOLD"       , ENV_LIQUID_COOL_MAX_PRESSURE_THRESHOLD       },
        {"ENV_LIQUID_COOL_MIN_PRESSURE_THRESHOLD"       , ENV_LIQUID_COOL_MIN_PRESSURE_THRESHOLD       },
        {"ENV_LIQUID_COOL_PRESSURE_SAMPLE_PERIOD"       , ENV_LIQUID_COOL_PRESSURE_SAMPLE_PERIOD       },
        {"ENV_LIQUID_COOL_MAX_FLOW_RATE_THRESHOLD"      , ENV_LIQUID_COOL_MAX_FLOW_RATE_THRESHOLD      },
        {"ENV_LIQUID_COOL_MIN_FLOW_RATE_THRESHOLD"      , ENV_LIQUID_COOL_MIN_FLOW_RATE_THRESHOLD      },
        {"ENV_LIQUID_COOL_FLOW_RATE_SAMPLE_PERIOD"      , ENV_LIQUID_COOL_FLOW_RATE_SAMPLE_PERIOD      },

        {"ENV_CDU_1_PRESENCE"                           , ENV_CDU_1_PRESENCE                           },
        {"ENV_CDU_1_NETWORK_IP_ADDRESS"                 , ENV_CDU_1_NETWORK_IP_ADDRESS                 },
        {"ENV_CDU_1_NETWORK_IP_PORT"                    , ENV_CDU_1_NETWORK_IP_PORT                    },
        {"ENV_CDU_1_COLD_STANDBY_SHUTDOWN"              , ENV_CDU_1_COLD_STANDBY_SHUTDOWN              },
        {"ENV_CDU_2_PRESENCE"                           , ENV_CDU_2_PRESENCE                           },
        {"ENV_CDU_2_NETWORK_IP_ADDRESS"                 , ENV_CDU_2_NETWORK_IP_ADDRESS                 },
        {"ENV_CDU_2_NETWORK_PORT"                       , ENV_CDU_2_NETWORK_IP_PORT                    },
        {"ENV_CDU_2_COLD_STANDBY_SHUTDOWN"              , ENV_CDU_2_COLD_STANDBY_SHUTDOWN              },
        {"ENV_CDU_3_PRESENCE"                           , ENV_CDU_3_PRESENCE                           },
        {"ENV_CDU_3_NETWORK_IP_ADDRESS"                 , ENV_CDU_3_NETWORK_IP_ADDRESS                 },
        {"ENV_CDU_3_NETWORK_PORT"                       , ENV_CDU_3_NETWORK_IP_PORT                    },
        {"ENV_CDU_3_COLD_STANDBY_SHUTDOWN"              , ENV_CDU_3_COLD_STANDBY_SHUTDOWN              },
        {"ENV_CDU_4_PRESENCE"                           , ENV_CDU_4_PRESENCE                           },
        {"ENV_CDU_4_NETWORK_IP_ADDRESS"                 , ENV_CDU_4_NETWORK_IP_ADDRESS                 },
        {"ENV_CDU_4_NETWORK_PORT"                       , ENV_CDU_4_NETWORK_IP_PORT                    },
        {"ENV_CDU_4_COLD_STANDBY_SHUTDOWN"              , ENV_CDU_4_COLD_STANDBY_SHUTDOWN              },


        {"SENSOR_REMOTE_RACK1_ANALOG_01"                , SENSOR_REMOTE_RACK1_ANALOG_01                },
        {"SENSOR_REMOTE_RACK1_ANALOG_02"                , SENSOR_REMOTE_RACK1_ANALOG_02                },
        {"SENSOR_REMOTE_RACK1_ANALOG_03"                , SENSOR_REMOTE_RACK1_ANALOG_03                },
        {"SENSOR_REMOTE_RACK1_ANALOG_04"                , SENSOR_REMOTE_RACK1_ANALOG_04                },
        {"SENSOR_REMOTE_RACK1_ANALOG_05"                , SENSOR_REMOTE_RACK1_ANALOG_05                },
        {"SENSOR_REMOTE_RACK1_ANALOG_06"                , SENSOR_REMOTE_RACK1_ANALOG_06                },
        {"SENSOR_REMOTE_RACK1_ANALOG_07"                , SENSOR_REMOTE_RACK1_ANALOG_07                },
        {"SENSOR_REMOTE_RACK1_ANALOG_08"                , SENSOR_REMOTE_RACK1_ANALOG_08                },
        {"SENSOR_REMOTE_RACK2_ANALOG_01"                , SENSOR_REMOTE_RACK2_ANALOG_01                },
        {"SENSOR_REMOTE_RACK2_ANALOG_02"                , SENSOR_REMOTE_RACK2_ANALOG_02                },
        {"SENSOR_REMOTE_RACK2_ANALOG_03"                , SENSOR_REMOTE_RACK2_ANALOG_03                },
        {"SENSOR_REMOTE_RACK2_ANALOG_04"                , SENSOR_REMOTE_RACK2_ANALOG_04                },
        {"SENSOR_REMOTE_RACK2_ANALOG_05"                , SENSOR_REMOTE_RACK2_ANALOG_05                },
        {"SENSOR_REMOTE_RACK2_ANALOG_06"                , SENSOR_REMOTE_RACK2_ANALOG_06                },
        {"SENSOR_REMOTE_RACK2_ANALOG_07"                , SENSOR_REMOTE_RACK2_ANALOG_07                },
        {"SENSOR_REMOTE_RACK2_ANALOG_08"                , SENSOR_REMOTE_RACK2_ANALOG_08                }

    },

    defaultTimeout{5000},

    EnviroData{
        updateIfData_t{"ENV_COOLING_TYPE", "System Cooling Method (Air Cooled or Liquid Cooled)", "", defaultTimeout},
        updateIfData_t{"ENV_COOLING_MODE", "System Cooling Mode (Auto or Manual)", "", defaultTimeout},
//        updateIfData_t{"ENV_MAX_TEMP","Current Max Temp Read", "deg C", defaultTimeout},
        updateIfData_t{"ENV_MAX_REGULATION_TEMP","Current Max Temp Read", "degC", defaultTimeout},
        updateIfData_t{"ENV_THERMAL_BAND", "Current Operational Thermal Band", "", defaultTimeout},
//        updateIfData_t{"ENV_SETPOINT","Current PWM Setpoint","percent",defaultTimeout},
        updateIfData_t{"ENV_PWM_OUTPUT","Current PWM Setpoint","percent",defaultTimeout},
        updateIfData_t{"ENV_TACH_SAMPLE_PERIOD","Sample period of Fan tachs in msecs","percent", defaultTimeout},
        updateIfData_t{"ENV_FAILSAFE_ACTIVE", "Failsafe Active State (true is Active)", "", defaultTimeout},
        updateIfData_t{"ENV_FAN_1_RPM", "Fan 1 RPM", "RPM", defaultTimeout},
        updateIfData_t{"ENV_FAN_2_RPM", "Fan 2 RPM", "RPM", defaultTimeout},
        updateIfData_t{"ENV_FAN_3_RPM", "Fan 3 RPM", "RPM", defaultTimeout},
        updateIfData_t{"ENV_FAN_4_RPM", "Fan 4 RPM", "RPM", defaultTimeout},
        updateIfData_t{"ENV_FAN_5_RPM", "Fan 5 RPM", "RPM", defaultTimeout},
        updateIfData_t{"ENV_FAN_6_RPM", "Fan 6 RPM", "RPM", defaultTimeout},
        updateIfData_t{"ENV_FAN_7_RPM", "Fan 7 RPM", "RPM", defaultTimeout},
        updateIfData_t{"ENV_FAN_8_RPM", "Fan 8 RPM", "RPM", defaultTimeout}
    },

    defaultAuxTimeout{5000},

    AuxCdu1Data{
        // CDU DEVICE ERROR STATUS
        updateIfData_t{"ENV_CDU_1_COMMS_OK",                     "CDU 1 System Communication  Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_LEAK",                    "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_PHASE_ERROR",             "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_SYSTEM_EMERGENCY_STOP",        "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_SYSTEM_ACCESSORY_1_SHUTDOWN",  "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_SYSTEM_ACCESSORY_2_SHUTDOWN",  "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_SYSTEM_REMOTE_ON_OFF",         "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_SYSTEM_COLD_STANDBY_SHUTDOWN", "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},

        updateIfData_t{"ENV_CDU_1_LOOP_1_EXCHANGE_OVERPRESSURE", "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_LOOP_1_FILTER_OVERPRESSURE",   "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_LOOP_2_EXCHANGE_OVERPRESSURE", "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_LOOP_2_FILTER_OVERPRESSURE",   "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},

        updateIfData_t{"ENV_CDU_1_RESERVOIR_1_FULL",             "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_RESERVOIR_1_LOW",              "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_RESERVOIR_2_FULL",             "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_RESERVOIR_2_LOW",              "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},

        updateIfData_t{"ENV_CDU_1_PUMP_1_ALARM",                 "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_2_ALARM",                 "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_3_ALARM",                 "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_4_ALARM",                 "CDU 1 System Shutdown Event Active State (true is Active)", "", defaultAuxTimeout},

        // CDU DEVICE DATA
        updateIfData_t{"ENV_CDU_1_PUMP_1_FLOW",                  "CDU 1 Pump 1 Flow in GPM", "GPM",   defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_1_TEMP",                  "CDU 1 Pump 1 Temp",        "degC", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_2_FLOW",                  "CDU 1 Pump 2 Flow in GPM", "GPM",   defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_2_TEMP",                  "CDU 1 Pump 2 Temp",        "degC", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_3_FLOW",                  "CDU 1 Pump 3 Flow in GPM", "GPM",   defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_3_TEMP",                  "CDU 1 Pump 3 Temp",        "degC", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_4_FLOW",                  "CDU 1 Pump 4 Flow in GPM", "GPM",   defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_PUMP_4_TEMP",                  "CDU 1 Pump 4 Temp",        "degC", defaultAuxTimeout},

        updateIfData_t{"ENV_CDU_1_LOOP_1_PRESSURE",              "CDU 1 System Loop 1 Pressure",    "PSI",   defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_LOOP_1_TEMP",                  "CDU 1 System Loop 1 Temp",        "degC", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_LOOP_2_PRESSURE",              "CDU 1 System Loop 2 Pressure",    "PSI",   defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_LOOP_2_TEMP",                  "CDU 1 System Loop 2 Temp",        "degC", defaultAuxTimeout},

        updateIfData_t{"ENV_CDU_1_RESERVOIR_1_TEMP",             "CDU 1 System Reservoir 1 Temp",   "degC", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_RESERVOIR_2_TEMP",             "CDU 1 System Reservoir 2 Temp",   "degC", defaultAuxTimeout},

        updateIfData_t{"ENV_CDU_1_PRESENCE",                     "CDU 1 System Presence  Active State (true is Active)", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_NETWORK_IP_ADDRESS",           "CDU 1 Controller Network IP Address", "", defaultAuxTimeout},
        updateIfData_t{"ENV_CDU_1_NETWORK_IP_PORT",              "CDU 1 Controller Network IP Port", "", defaultAuxTimeout},
    },

    defaultAuxModBusTimeout{5000},

    AuxCdu1ModBusData{
        // CDU PUMP 1 MODBUS DATA
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE"                  ,   "Operating Mode"                   ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_CONTROL_MODE"                    ,   "Control Mode"                     ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_CONFIG_MODE"                     ,   "Config Mode"                      ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_TWIN_CONTROL_MODE"               ,   "Twin Control Mode"                ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_NIGHT_MODE_ACTIVATION"           ,   "Night Mode Activation"            ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_AIR_VENTING_PROCEDURE"           ,   "Air Venting Procedure"            ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_SETPOINT"         ,   "Constant Curve Setpoint"          ,   "RPM"  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_AIR_VENTING_AT_POWER_ON"         ,   "Air Venting at Power On"          ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_INPUT_POWER"                     ,   "Input Power"                      ,   " W "  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_HEAD"                            ,   "Head"                             ,   "PSI"  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_FLOW"                            ,   "Flow"                             ,   "GPM"  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_SPEED"                           ,   "Speed"                            ,   "RPM"  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_WATER_TEMPERATURE"               ,   "Water Temperature"                ,   " C "  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_WINDING_1_TEMPERATURE"           ,   "Winding 1 Temperature"            ,   " C "  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_WINDING_2_TEMPERATURE"           ,   "Winding 2 Temperature"            ,   " C "  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_WINDING_3_TEMPERATURE"           ,   "Winding 3 Temperature"            ,   " C "  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_POWER_MODULE_TEMPERATURE"        ,   "Power Module Temperature"         ,   " C "  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_QUADRANT_CURRENT"                ,   "Quadrant Current"                 ,   " A "  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_STATUS_IO"            ,   "Bit Fields Status I/O"            ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ALARM_1"              ,   "Bit Fields Alarm 1"               ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ALARM_2"              ,   "Bit Fields Alarm 2"               ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ERROR"                ,   "Bit Fields Error"                 ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_ACTIVE_ERROR_CODE"               ,   "Active Error Code"                ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_MIN_SETPOINT"     ,   "Constant Curve Min Setpoint"      ,   "RPM"  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_MAX_SETPOINT"     ,   "Constant Curve Max Setpoint"      ,   "RPM"  , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_COMMUNICATIONS_PROTOCOL"         ,   "Communications Protocol"          ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_COMMUNICATIONS_BAUD_RATE"        ,   "Communication Baud Rate"          ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_LIFETIME_TIMER_LSW"              ,   "Lifetime Timer (LSW)"             ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_LIFETIME_TIMER_MSW"              ,   "Lifetime Timer (MSW)"             ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_0_25_LSW"      ,   "Power Consumption 0-25% (LSW)"    ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_0_25_MSW"      ,   "Power Consumption 0-25% (MSW)"    ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_25_50_LSW"     ,   "Power Consumption 25%-50% (LSW)"  ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_25_50_MSW"     ,   "Power Consumption 25%-50% (MSW)"  ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_50_75_LSW"     ,   "Power Consumption 50-75% (LSW)"   ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_50_75_MSW"     ,   "Power Consumption 50-75% (MSW)"   ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_75_100_LSW"    ,   "Power Consumption 75-100% (LSW)"  ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_75_100_MSW"    ,   "Power Consumption 75-100% (MSW)"  ,   "  "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_1_MODBUS_CURRENT_INDEX_LOG"               ,   "Current Index Log"                ,   "  "   , defaultAuxModBusTimeout },

        // CDU PUMP 2 MODBUS DATA
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE"                  ,   "Operating Mode"                   ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_CONTROL_MODE"                    ,   "Control Mode"                     ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_CONFIG_MODE"                     ,   "Config Mode"                      ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_TWIN_CONTROL_MODE"               ,   "Twin Control Mode"                ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_NIGHT_MODE_ACTIVATION"           ,   "Night Mode Activation"            ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_AIR_VENTING_PROCEDURE"           ,   "Air Venting Procedure"            ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_SETPOINT"         ,   "Constant Curve Setpoint"          ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_AIR_VENTING_AT_POWER_ON"         ,   "Air Venting at Power On"          ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_INPUT_POWER"                     ,   "Input Power"                      ,   " W "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_HEAD"                            ,   "Head"                             ,   "PSI"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_FLOW"                            ,   "Flow"                             ,   "GPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_SPEED"                           ,   "Speed"                            ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_WATER_TEMPERATURE"               ,   "Water Temperature"                ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_WINDING_1_TEMPERATURE"           ,   "Winding 1 Temperature"            ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_WINDING_2_TEMPERATURE"           ,   "Winding 2 Temperature"            ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_WINDING_3_TEMPERATURE"           ,   "Winding 3 Temperature"            ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_POWER_MODULE_TEMPERATURE"        ,   "Power Module Temperature"         ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_QUADRANT_CURRENT"                ,   "Quadrant Current"                 ,   " A "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_STATUS_IO"            ,   "Bit Fields Status I/O"            ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_1"              ,   "Bit Fields Alarm 1"               ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_2"              ,   "Bit Fields Alarm 2"               ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ERROR"                ,   "Bit Fields Error"                 ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_ACTIVE_ERROR_CODE"               ,   "Active Error Code"                ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_MIN_SETPOINT"     ,   "Constant Curve Min Setpoint"      ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_MAX_SETPOINT"     ,   "Constant Curve Max Setpoint"      ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_COMMUNICATIONS_PROTOCOL"         ,   "Communications Protocol"          ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_COMMUNICATIONS_BAUD_RATE"        ,   "Communication Baud Rate"          ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_LIFETIME_TIMER_LSW"              ,   "Lifetime Timer (LSW)"             ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_LIFETIME_TIMER_MSW"              ,   "Lifetime Timer (MSW)"             ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_0_25_LSW"      ,   "Power Consumption 0-25% (LSW)"    ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_0_25_MSW"      ,   "Power Consumption 0-25% (MSW)"    ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_25_50_LSW"     ,   "Power Consumption 25%-50% (LSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_25_50_MSW"     ,   "Power Consumption 25%-50% (MSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_50_75_LSW"     ,   "Power Consumption 50-75% (LSW)"   ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_50_75_MSW"     ,   "Power Consumption 50-75% (MSW)"   ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_75_100_LSW"    ,   "Power Consumption 75-100% (LSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_75_100_MSW"    ,   "Power Consumption 75-100% (MSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_2_MODBUS_CURRENT_INDEX_LOG"               ,   "Current Index Log"                ,   "  "    , defaultAuxModBusTimeout },

        // CDU PUMP 3 MODBUS DATA
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_OPERATING_MODE"                  ,   "Operating Mode"                   ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_CONTROL_MODE"                    ,   "Control Mode"                     ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_CONFIG_MODE"                     ,   "Config Mode"                      ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_TWIN_CONTROL_MODE"               ,   "Twin Control Mode"                ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_NIGHT_MODE_ACTIVATION"           ,   "Night Mode Activation"            ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_AIR_VENTING_PROCEDURE"           ,   "Air Venting Procedure"            ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_SETPOINT"         ,   "Constant Curve Setpoint"          ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_AIR_VENTING_AT_POWER_ON"         ,   "Air Venting at Power On"          ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_INPUT_POWER"                     ,   "Input Power"                      ,   " W "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_HEAD"                            ,   "Head"                             ,   "PSI"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_FLOW"                            ,   "Flow"                             ,   "GPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_SPEED"                           ,   "Speed"                            ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_WATER_TEMPERATURE"               ,   "Water Temperature"                ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_WINDING_1_TEMPERATURE"           ,   "Winding 1 Temperature"            ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_WINDING_2_TEMPERATURE"           ,   "Winding 2 Temperature"            ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_WINDING_3_TEMPERATURE"           ,   "Winding 3 Temperature"            ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_POWER_MODULE_TEMPERATURE"        ,   "Power Module Temperature"         ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_QUADRANT_CURRENT"                ,   "Quadrant Current"                 ,   " A "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_STATUS_IO"            ,   "Bit Fields Status I/O"            ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ALARM_1"              ,   "Bit Fields Alarm 1"               ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ALARM_2"              ,   "Bit Fields Alarm 2"               ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ERROR"                ,   "Bit Fields Error"                 ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_ACTIVE_ERROR_CODE"               ,   "Active Error Code"                ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_MIN_SETPOINT"     ,   "Constant Curve Min Setpoint"      ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_MAX_SETPOINT"     ,   "Constant Curve Max Setpoint"      ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_COMMUNICATIONS_PROTOCOL"         ,   "Communications Protocol"          ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_COMMUNICATIONS_BAUD_RATE"        ,   "Communication Baud Rate"          ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_LIFETIME_TIMER_LSW"              ,   "Lifetime Timer (LSW)"             ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_LIFETIME_TIMER_MSW"              ,   "Lifetime Timer (MSW)"             ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_0_25_LSW"      ,   "Power Consumption 0-25% (LSW)"    ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_0_25_MSW"      ,   "Power Consumption 0-25% (MSW)"    ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_25_50_LSW"     ,   "Power Consumption 25%-50% (LSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_25_50_MSW"     ,   "Power Consumption 25%-50% (MSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_50_75_LSW"     ,   "Power Consumption 50-75% (LSW)"   ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_50_75_MSW"     ,   "Power Consumption 50-75% (MSW)"   ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_75_100_LSW"    ,   "Power Consumption 75-100% (LSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_75_100_MSW"    ,   "Power Consumption 75-100% (MSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_3_MODBUS_CURRENT_INDEX_LOG"               ,   "Current Index Log"                ,   "  "    , defaultAuxModBusTimeout },

        // CDU PUMP 4 MODBUS DATA
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE"                  ,   "Operating Mode"                   ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_CONTROL_MODE"                    ,   "Control Mode"                     ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_CONFIG_MODE"                     ,   "Config Mode"                      ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_TWIN_CONTROL_MODE"               ,   "Twin Control Mode"                ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_NIGHT_MODE_ACTIVATION"           ,   "Night Mode Activation"            ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_AIR_VENTING_PROCEDURE"           ,   "Air Venting Procedure"            ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_SETPOINT"         ,   "Constant Curve Setpoint"          ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_AIR_VENTING_AT_POWER_ON"         ,   "Air Venting at Power On"          ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_INPUT_POWER"                     ,   "Input Power"                      ,   " W "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_HEAD"                            ,   "Head"                             ,   "PSI"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_FLOW"                            ,   "Flow"                             ,   "GPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_SPEED"                           ,   "Speed"                            ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_WATER_TEMPERATURE"               ,   "Water Temperature"                ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_WINDING_1_TEMPERATURE"           ,   "Winding 1 Temperature"            ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_WINDING_2_TEMPERATURE"           ,   "Winding 2 Temperature"            ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_WINDING_3_TEMPERATURE"           ,   "Winding 3 Temperature"            ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_POWER_MODULE_TEMPERATURE"        ,   "Power Module Temperature"         ,   " C "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_QUADRANT_CURRENT"                ,   "Quadrant Current"                 ,   " A "   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_STATUS_IO"            ,   "Bit Fields Status I/O"            ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_1"              ,   "Bit Fields Alarm 1"               ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_2"              ,   "Bit Fields Alarm 2"               ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ERROR"                ,   "Bit Fields Error"                 ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_ACTIVE_ERROR_CODE"               ,   "Active Error Code"                ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_MIN_SETPOINT"     ,   "Constant Curve Min Setpoint"      ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_MAX_SETPOINT"     ,   "Constant Curve Max Setpoint"      ,   "RPM"   , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_COMMUNICATIONS_PROTOCOL"         ,   "Communications Protocol"          ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_COMMUNICATIONS_BAUD_RATE"        ,   "Communication Baud Rate"          ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_LIFETIME_TIMER_LSW"              ,   "Lifetime Timer (LSW)"             ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_LIFETIME_TIMER_MSW"              ,   "Lifetime Timer (MSW)"             ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_0_25_LSW"      ,   "Power Consumption 0-25% (LSW)"    ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_0_25_MSW"      ,   "Power Consumption 0-25% (MSW)"    ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_25_50_LSW"     ,   "Power Consumption 25%-50% (LSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_25_50_MSW"     ,   "Power Consumption 25%-50% (MSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_50_75_LSW"     ,   "Power Consumption 50-75% (LSW)"   ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_50_75_MSW"     ,   "Power Consumption 50-75% (MSW)"   ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_75_100_LSW"    ,   "Power Consumption 75-100% (LSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_75_100_MSW"    ,   "Power Consumption 75-100% (MSW)"  ,   "  "    , defaultAuxModBusTimeout },
        updateIfData_t{ "ENV_CDU_1_PUMP_4_MODBUS_CURRENT_INDEX_LOG"               ,   "Current Index Log"                ,   "  "    , defaultAuxModBusTimeout }

    },


    ExtDeviceData{},

    deviceKeysPMods{
//        {ExternalDeviceDataIndex::MBRD,     "MBRD"},
        {ExternalDeviceDataIndex::PA_1,     "PA_1"},
        {ExternalDeviceDataIndex::PA_2,     "PA_2"},
        {ExternalDeviceDataIndex::PA_3,     "PA_3"},
        {ExternalDeviceDataIndex::PA_4,     "PA_4"},
        {ExternalDeviceDataIndex::PA_5,     "PA_5"},
        {ExternalDeviceDataIndex::PA_6,     "PA_6"},
        {ExternalDeviceDataIndex::PA_7,     "PA_7"},
        {ExternalDeviceDataIndex::PA_8,     "PA_8"},
        {ExternalDeviceDataIndex::DRIVER_1, "DRIVER_1"},
        {ExternalDeviceDataIndex::DRIVER_2, "DRIVER_2"},
        {ExternalDeviceDataIndex::DRIVER_3, "DRIVER_3"}
    },

    bands{},
    allPmodAddrs{},
    curPmod{},
    msgHdr{},
    msgData{}, // TODO -- MB -- Remove after refactor???
    EnviroMgrInitComplete{false},
    CoolingFailsafeActive{false},
    tmrHysteresisStartPoint{std::chrono::steady_clock::now()},
    tmrSamplePeriodStartPoint{std::chrono::steady_clock::now()},
    CoolSetpointComplete{false},
    StatusNoDataAvailPollCnt{0},
    ModBusNoDataAvailPollCnt{0}
{
    logger::info(__FILE__, __FUNCTION__, "ENVIRONMENT_MANAGER_ONLINE - Loading Configuration...");

    // INTERNAL INITIALIZATION(s)
    //
    initEnviroConfig();
//    initEnviroStatusData();
    initModuleDevicesVect();
    loadModuleDevicesVect();
}



/**
 * Initialization Methods
 *
 *
 *
 */

void envMgrController::initEnviroConfig()
/// ***************************************************************************
/// Summary:
///
///     Initializes EnviroMgrController operating structures, storage and data
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    logger::verbose(__FILE__, __FUNCTION__, "Starting Initialization of Enviro Configuration ...");

    loadRackSlotModeInfo();
//    exit(0);

    loadEnviroConfig();
    initThermalBandData();
    initAirCoolAlgo();
    EnviroMgrInitComplete = true;

    logger::verbose(__FILE__, __FUNCTION__, "Initialization of Enviro Configuration - COMPLETE");
}


bool envMgrController::loadRackSlotModeInfo()
/// ***************************************************************************
/// Summary:
///
///     Method called by initEnviroConfig() to get and save internally, system
///     Rack, Slot and Mode info which resides within the empowerStatic directory
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     true - method succeeded, false otherwise
///     sets proper values/types in the SystemTypeInfo structure
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    bool        rtn = false;

    std::ifstream rackFile("/empowerStatic/currentRack.txt");
    std::stringstream rackBuffer;
    std::string rackStr;
    rackBuffer << rackFile.rdbuf();
    rackStr = rackBuffer.str();

    std::ifstream slotFile("/empowerStatic/currentSlot.txt");
    std::stringstream slotBuffer;
    std::string slotStr;
    slotBuffer << slotFile.rdbuf();
    slotStr = slotBuffer.str();

    std::ifstream modeFile("/empowerStatic/operationalMode.txt");
    std::stringstream modeBuffer;
    std::string modeStr;
    modeBuffer << modeFile.rdbuf();
    modeStr = modeBuffer.str();



    SystemTypeInfo.rack = std::stoi(rackStr);
    SystemTypeInfo.slot = std::stoi(slotStr);

    if( (0 == SystemTypeInfo.rack) && (0 == SystemTypeInfo.slot))
    {
        if(0 == modeStr.compare("AMPLIFIER"))
            SystemTypeInfo.mode = SystemModeTypes::STANDALONE;
        else
            SystemTypeInfo.mode = SystemModeTypes::SOS_CONTROLLER;
    }
    else
        SystemTypeInfo.mode = SystemModeTypes::SOS_BOOSTER;

    return rtn;
}


bool envMgrController::processConfigChangeUpdate(std::string parmName)
/// ***************************************************************************
/// Summary:
///
///     Method called by parser when a configChangeNotify event occurs
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     true - method succeeded, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
//    std::cout << "\n\n\n========>>> "<< __FUNCTION__ << "() --> " << "Received ConfigChangeNotify Message..\n\n" << std::endl;
    logger::verbose(__FILE__, __FUNCTION__, std::string("Rec'd configChangeNotify for ") + parmName +
        std::string(" parameter, Attempting Configuration Update ..."));

    /// TODO : ADD code to Validate Parameter Value sent
    ///
    // test for parm entry
    if( mapCfgParm.count(parmName) > 0 )
    {
        // access map to exec function
        auto index = mapCfgParm.at(parmName);
        switch(index)
        {
            case FHI_MOD_INVENTORY:
                EnviroCfg.InvntryModName = configIF.getParam<std::string>("FHI_MOD_INVENTORY").value_or("INVENTORY");
                break;

            case FHI_MOD_FAN_CTRL:
                EnviroCfg.FanCtrlModName = configIF.getParam<std::string>("FHI_MOD_FAN_CTRL").value_or("FAN_CTRL");
                break;

            case ENV_AIR_COOL_ENABLE:
                EnviroCfg.AirCoolEn = configIF.getParam<bool>(parmName).value_or(true);
                std::tie (EnviroData[CoolType].data, EnviroData[CoolType].valid) = getCoolingType();
                break;

            case ENV_AIR_COOL_MANUAL_ENABLE:
                EnviroCfg.AirCoolManModeEn = configIF.getParam<bool>(parmName).value_or(false);
                std::tie (EnviroData[CoolMode].data, EnviroData[CoolMode].valid) = getCoolingMode();
                break;

            case ENV_AIR_COOL_MANUAL_SETPOINT:
                EnviroCfg.AirCoolManSetpoint = configIF.getParam<mmData_t>(parmName).value_or(30);
                break;

            case ENV_AIR_COOL_MAXIMUM_THRESHOLD:
                EnviroCfg.AirCoolMaxThreshold = configIF.getParam<temp_t>(parmName).value_or(70);
                break;

            case ENV_AIR_COOL_MINIMUM_THRESHOLD:
                EnviroCfg.AirCoolMinThreshold = configIF.getParam<temp_t>(parmName).value_or(30);
                break;

            case ENV_AIR_COOL_OPERATIONAL_BANDS:
                EnviroCfg.AirCoolOperBands = configIF.getParam<mmData_t>(parmName).value_or(10);
                break;

            case ENV_AIR_COOL_MINIMUM_OPERATIONAL_BAND:
                EnviroCfg.AirCoolOperBands = configIF.getParam<mmData_t>(parmName).value_or(3);
                break;

            case ENV_AIR_COOL_FAN_PWM_FREQUENCY:
                EnviroCfg.AirCoolPwmFreq = configIF.getParam<mmData_t>(parmName).value_or(16000);
                break;

            case ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_UP:
                EnviroCfg.AirCoolHysteresisUp = static_cast<timeout_t>(configIF.getParam<mmData_t>(parmName).value_or(750));
                break;

            case ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_DOWN:
                EnviroCfg.AirCoolHysteresisDown = static_cast<timeout_t>(configIF.getParam<mmData_t>(parmName).value_or(15000));
                break;

            case ENV_AIR_COOL_SAMPLE_PERIOD:
                EnviroCfg.AirCoolTempSamplePeriod = static_cast<timeout_t>(configIF.getParam<mmData_t>(parmName).value_or(500));
                break;

            case ENV_AIR_COOL_TACHOMETER_SAMPLE_PERIOD:
                EnviroCfg.AirCoolTachSamplePeriod = static_cast<timeout_t>(configIF.getParam<mmData_t>(parmName).value_or(500));
                EnviroData[FanTachSamplePeriod].data = configIF.getParam<mmData_t>(parmName).value_or(500);
                EnviroData[FanTachSamplePeriod].valid = true;
                break;

            case ENV_AIR_COOL_FAN_1_PRESENCE:
                EnviroCfg.AirCoolFanPresent[0] = configIF.getParam<bool>(parmName).value_or(true);
                break;

            case ENV_AIR_COOL_FAN_1_PULSES_PER_REV:
                EnviroCfg.AirCoolPulsesPerRev[0] = configIF.getParam<mmData_t>(parmName).value_or(2);
                break;

            case ENV_AIR_COOL_FAN_2_PRESENCE:
                EnviroCfg.AirCoolFanPresent[1] = configIF.getParam<bool>(parmName).value_or(true);
                break;

            case ENV_AIR_COOL_FAN_2_PULSES_PER_REV:
                EnviroCfg.AirCoolPulsesPerRev[1] = configIF.getParam<mmData_t>(parmName).value_or(2);
                break;

            case ENV_AIR_COOL_FAN_3_PRESENCE:
                EnviroCfg.AirCoolFanPresent[2] = configIF.getParam<bool>(parmName).value_or(true);
                break;

            case ENV_AIR_COOL_FAN_3_PULSES_PER_REV:
                EnviroCfg.AirCoolPulsesPerRev[2] = configIF.getParam<mmData_t>(parmName).value_or(2);
                break;

            case ENV_AIR_COOL_FAN_4_PRESENCE:
                EnviroCfg.AirCoolFanPresent[3] = configIF.getParam<bool>(parmName).value_or(true);
                break;

            case ENV_AIR_COOL_FAN_4_PULSES_PER_REV:
                EnviroCfg.AirCoolPulsesPerRev[3] = configIF.getParam<mmData_t>(parmName).value_or(2);
                break;

            case ENV_AIR_COOL_FAN_5_PRESENCE:
                EnviroCfg.AirCoolFanPresent[4] = configIF.getParam<bool>(parmName).value_or(true);
                break;

            case ENV_AIR_COOL_FAN_5_PULSES_PER_REV:
                EnviroCfg.AirCoolPulsesPerRev[4] = configIF.getParam<mmData_t>(parmName).value_or(2);
                break;

            case ENV_AIR_COOL_FAN_6_PRESENCE:
                EnviroCfg.AirCoolFanPresent[5] = configIF.getParam<bool>(parmName).value_or(true);
                break;

            case ENV_AIR_COOL_FAN_6_PULSES_PER_REV:
                EnviroCfg.AirCoolPulsesPerRev[5] = configIF.getParam<mmData_t>(parmName).value_or(2);
                break;

            case ENV_AIR_COOL_FAN_7_PRESENCE:
                EnviroCfg.AirCoolFanPresent[6] = configIF.getParam<bool>(parmName).value_or(true);
                break;

            case ENV_AIR_COOL_FAN_7_PULSES_PER_REV:
                EnviroCfg.AirCoolPulsesPerRev[6] = configIF.getParam<mmData_t>(parmName).value_or(2);
                break;

            case ENV_AIR_COOL_FAN_8_PRESENCE:
                EnviroCfg.AirCoolFanPresent[7] = configIF.getParam<bool>(parmName).value_or(true);
                break;

            case ENV_AIR_COOL_FAN_8_PULSES_PER_REV:
                EnviroCfg.AirCoolPulsesPerRev[7] = configIF.getParam<mmData_t>(parmName).value_or(2);
                break;

/// TODO : Add rest of Liquid and CDU config updates
            case ENV_LIQUID_COOL_ENABLE:
                EnviroCfg.LiquidCoolEn = configIF.getParam<bool>(parmName).value_or(false);
                break;


            case ENV_CDU_1_PRESENCE:
                EnviroCfg.Cdu1Presence = configIF.getParam<bool>(parmName).value_or(false);
                AuxCdu1Data[ENV_CDU_PRESENCE].data = EnviroCfg.Cdu1Presence;
                AuxCdu1Data[ENV_CDU_PRESENCE].valid = true;
                break;

           case ENV_CDU_1_NETWORK_IP_ADDRESS:
                EnviroCfg.Cdu1NetworkIP = configIF.getParam<std::string>(parmName).value_or("172.16.0.70");
                AuxCdu1Data[ENV_CDU_NETWORK_IP_ADDRESS].data = EnviroCfg.Cdu1NetworkIP;
                AuxCdu1Data[ENV_CDU_NETWORK_IP_ADDRESS].valid = true;
                cduClient.setIpAddress(EnviroCfg.Cdu1NetworkIP);
                break;

           case ENV_CDU_1_NETWORK_IP_PORT:
               EnviroCfg.Cdu1NetworkPort = configIF.getParam<mmData_t>(parmName).value_or(9000);
               AuxCdu1Data[ENV_CDU_NETWORK_IP_PORT].data = EnviroCfg.Cdu1NetworkPort;
               AuxCdu1Data[ENV_CDU_NETWORK_IP_PORT].valid = true;
               cduClient.setIpPort( static_cast<uint16_t>(EnviroCfg.Cdu1NetworkPort));
               break;

           case ENV_CDU_1_COLD_STANDBY_SHUTDOWN:
               EnviroCfg.Cdu1ColdStandbyShutdown = configIF.getParam<bool>("ENV_CDU_1_COLD_STANDBY_SHUTDOWN").value_or(true);
               AuxCdu1Data[ENV_CDU_1_COLD_STANDBY_SHUTDOWN].data = EnviroCfg.Cdu1ColdStandbyShutdown;
               AuxCdu1Data[ENV_CDU_1_COLD_STANDBY_SHUTDOWN].valid = true;
               std::cout << __FUNCTION__ << "() --> "<< "ENV_CDU_1_COLD_STANDBY_SHUTDOWN Update COMPLETED: " << EnviroCfg.Cdu1ColdStandbyShutdown << "\n\n" << std::endl;
               break;


            default:
                logger::warn(__FILE__, __FUNCTION__, std::string("Configuration Update FAILED!!! Parameter: ") +
                    parmName + std::string(" NOT SUPPORTED"));
                return false;
//                break;
        }
    }
    else
    {
        // ERROR EXIT Log Msg
        logger::critical(__FILE__, __FUNCTION__, std::string("Configuration Update FAILED!!! Parameter: ") +
            parmName + std::string(" NOT FOUND"));
        return false;

    }

    // EXIT Log Msg
    logger::verbose(__FILE__, __FUNCTION__, "Configuration Update COMPLETED");

//    std::cout << __FUNCTION__ << "() --> "<< "Configuration Update COMPLETED..\n\n" << std::endl;

    return true;
}

bool envMgrController::loadEnviroConfig()
/// ***************************************************************************
/// Summary:
///
///     Loads configuration data from Configuration Manager into local data
///     structure for use during operations
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   true on success, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{

    /// FIXME : Test code for dry run on Controller
    ///

    // instance

    //vars
//    std::string     msg{"tester"};
    std::string     hostIP{"172.16.0.70"};

    uint16_t        hostPort{9000};




/// TODO -SSS : Add check of System INFO for MODE, to load PARMS
///
    switch(SystemTypeInfo.mode)
    {
        case STANDALONE:
        case SOS_BOOSTER:
            break;

        case SOS_CONTROLLER:
            break;

        default:
            break;

    }
    // Environmental Config
    //
    EnviroCfg.InvntryModName = configIF.getParam<std::string>("FHI_MOD_INVENTORY").value_or("INVENTORY");

    EnviroCfg.AirCoolEn = configIF.getParam<bool>("ENV_AIR_COOL_ENABLE").value_or(true);

    EnviroCfg.LiquidCoolEn  = configIF.getParam<bool>("ENV_LIQUID_COOL_ENABLE").value_or(true);

    if(EnviroCfg.AirCoolEn)
    {
        EnviroCfg.FanCtrlModName = configIF.getParam<std::string>("FHI_MOD_FAN_CTRL").value_or("FAN_CTRL");

        EnviroCfg.AirCoolManModeEn = configIF.getParam<bool>("ENV_AIR_COOL_MANUAL_ENABLE").value_or(false);
        EnviroCfg.AirCoolManSetpoint = configIF.getParam<mmData_t>("ENV_AIR_COOL_MANUAL_SETPOINT").value_or(30);
        EnviroCfg.AirCoolMaxThreshold = configIF.getParam<temp_t>("ENV_AIR_COOL_MAXIMUM_THRESHOLD").value_or(70);
        EnviroCfg.AirCoolMinThreshold = configIF.getParam<temp_t>("ENV_AIR_COOL_MINIMUM_THRESHOLD").value_or(30);
        EnviroCfg.AirCoolOperBands = configIF.getParam<mmData_t>("ENV_AIR_COOL_OPERATIONAL_BANDS").value_or(10);
        EnviroCfg.AirCoolMinOperBand = configIF.getParam<mmData_t>("ENV_AIR_COOL_MINIMUM_OPERATIONAL_BAND").value_or(3);
        EnviroCfg.AirCoolPwmFreq = configIF.getParam<mmData_t>("ENV_AIR_COOL_FAN_PWM_FREQUENCY").value_or(16000);

        EnviroCfg.AirCoolHysteresisUp = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_UP").value_or(750));
        EnviroCfg.AirCoolHysteresisDown = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_DOWN").value_or(15000));
        EnviroCfg.AirCoolTempSamplePeriod = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_AIR_COOL_SAMPLE_PERIOD").value_or(500));
        EnviroCfg.AirCoolTachSamplePeriod = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_AIR_COOL_TACHOMETER_SAMPLE_PERIOD").value_or(500));

        /// TODO :Add test of Max > Min Temp Thresh
        /// set to default parms if fails
        ///

        /// TODO : We add System Push data here for Items only accessed during config
        ///
        EnviroData[FanTachSamplePeriod].data = configIF.getParam<mmData_t>("ENV_AIR_COOL_SAMPLE_PERIOD").value_or(500);
        EnviroData[FanTachSamplePeriod].valid = true;


#if 0
        /// TODO -SSS: TEST OUTPUT - Config Output Dump on StartUp --- remove when done
        ///
        msgHdr = "\n\n[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
        msgData = "\n-> EnviroCfg.InvntryModName         : "   +               (EnviroCfg.InvntryModName                  ) + \
                  "\n-> EnviroCfg.FanCtrlModName         : "   +               (EnviroCfg.FanCtrlModName                  ) + \
                  "\n-> EnviroCfg.AirCoolEn              : "   + std::to_string(EnviroCfg.AirCoolEn                       ) + \
                  "\n-> EnviroCfg.AirCoolManModeEn       : "   + std::to_string(EnviroCfg.AirCoolManModeEn                ) + \
                  "\n-> EnviroCfg.AirCoolManSetpoint     : "   + std::to_string(EnviroCfg.AirCoolManSetpoint              ) + \
                  "\n-> EnviroCfg.AirCoolMaxThreshold    : "   + std::to_string(EnviroCfg.AirCoolMaxThreshold             ) + \
                  "\n-> EnviroCfg.AirCoolMinThreshold    : "   + std::to_string(EnviroCfg.AirCoolMinThreshold             ) + \
                  "\n-> EnviroCfg.AirCoolOperBands       : "   + std::to_string(EnviroCfg.AirCoolOperBands                ) + \
                  "\n-> EnviroCfg.AirCoolMinOperBand     : "   + std::to_string(EnviroCfg.AirCoolMinOperBand              ) + \
                  "\n-> EnviroCfg.AirCoolPwmFreq         : "   + std::to_string(EnviroCfg.AirCoolPwmFreq                  ) + \
                  "\n-> EnviroCfg.AirCoolHysteresisUp    : "   + std::to_string(EnviroCfg.AirCoolHysteresisUp.count()     ) + \
                  "\n-> EnviroCfg.AirCoolHysteresisDown  : "   + std::to_string(EnviroCfg.AirCoolHysteresisDown.count()   ) + \
                  "\n-> EnviroCfg.AirCoolTempSamplePeriod: "   + std::to_string(EnviroCfg.AirCoolTempSamplePeriod.count() ) + \
                  "\n-> EnviroCfg.AirCoolTachSamplePeriod: "   + std::to_string(EnviroCfg.AirCoolTachSamplePeriod.count() );
        std::cout  << msgHdr << msgData << std::endl;
#endif

        for(std::size_t i = 0; i < MAX_SYS_FAN_COUNT; i++)
        {
            std::string tmpStr = "ENV_AIR_COOL_FAN_" + std::to_string(i+1);
            EnviroCfg.AirCoolFanPresent[i] = configIF.getParam<bool>(tmpStr + "_PRESENCE").value_or(true);
            EnviroCfg.AirCoolPulsesPerRev[i] = configIF.getParam<mmData_t>(tmpStr + "_PULSES_PER_REV").value_or(2);
        }

        logger::verbose(__FILE__, __FUNCTION__, "AIR COOL Configuration load complete");

    }

    // check and do Liquid Config
    if(EnviroCfg.LiquidCoolEn)
    {
        EnviroCfg.LiquidCoolMaxTempThreshold = configIF.getParam<temp_t>("ENV_LIQUID_COOL_MAX_TEMPERATURE_THRESHOLD").value_or(120);
        EnviroCfg.LiquidCoolMinTempThreshold = configIF.getParam<temp_t>("ENV_LIQUID_COOL_MIN_TEMPERATURE_THRESHOLD").value_or(0);
        EnviroCfg.LiquidCoolTempSamplePeriod = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_LIQUID_COOL_TEMPERATURE_SAMPLE_PERIOD").value_or(500));

        EnviroCfg.LiquidCoolMaxPressureThreshold = configIF.getParam<mmData_t>("ENV_LIQUID_COOL_MAX_PRESSURE_THRESHOLD").value_or(60);
        EnviroCfg.LiquidCoolMinPressureThreshold = configIF.getParam<mmData_t>("ENV_LIQUID_COOL_MIN_PRESSURE_THRESHOLD").value_or(0);
        EnviroCfg.LiquidCoolPressureSamplePeriod = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_LIQUID_COOL_PRESSURE_SAMPLE_PERIOD").value_or(500));

        EnviroCfg.LiquidCoolMaxFlowRateThreshold = configIF.getParam<mmData_t>("ENV_LIQUID_COOL_MAX_FLOW_RATE_THRESHOLD").value_or(25.0);
        EnviroCfg.LiquidCoolMinFlowRateThreshold = configIF.getParam<mmData_t>("ENV_LIQUID_COOL_MIN_FLOW_RATE_THRESHOLD").value_or(1.0);
        EnviroCfg.LiquidCoolFlowRateSamplePeriod = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_LIQUID_COOL_FLOW_RATE_SAMPLE_PERIOD").value_or(500));

        EnviroCfg.Cdu1Presence  = configIF.getParam<bool>("ENV_CDU_1_PRESENCE").value_or(true);
        AuxCdu1Data[ENV_CDU_PRESENCE].data = EnviroCfg.Cdu1Presence;
        AuxCdu1Data[ENV_CDU_PRESENCE].valid = true;

        EnviroCfg.Cdu1NetworkIP = configIF.getParam<std::string>("ENV_CDU_1_NETWORK_IP_ADDRESS").value_or(hostIP);
        AuxCdu1Data[ENV_CDU_NETWORK_IP_ADDRESS].data = EnviroCfg.Cdu1NetworkIP;
        AuxCdu1Data[ENV_CDU_NETWORK_IP_ADDRESS].valid = true;

        EnviroCfg.Cdu1NetworkPort = configIF.getParam<mmData_t>("ENV_CDU_1_NETWORK_IP_PORT").value_or(hostPort);
        AuxCdu1Data[ENV_CDU_NETWORK_IP_PORT].data = EnviroCfg.Cdu1NetworkPort;
        AuxCdu1Data[ENV_CDU_NETWORK_IP_PORT].valid = true;

        EnviroCfg.Cdu1ColdStandbyShutdown  = configIF.getParam<bool>("ENV_CDU_1_COLD_STANDBY_SHUTDOWN").value_or(true);
        AuxCdu1Data[ENV_CDU_1_COLD_STANDBY_SHUTDOWN].data = EnviroCfg.Cdu1ColdStandbyShutdown;
        AuxCdu1Data[ENV_CDU_1_COLD_STANDBY_SHUTDOWN].valid = true;

        // initialize CDU IP connection after IP or Port changes
        cduClient.setIpAddress(EnviroCfg.Cdu1NetworkIP);
        cduClient.setIpPort(static_cast<uint16_t>(EnviroCfg.Cdu1NetworkPort));

        /// TODO : Test of Config get
        /// read in Pump.Config and parse specific parameters
        ///

//        cduClient.getCfgData("../config/Pump.Config");


        /// -- end

#if 0
        /// TODO -SSS: TEST OUTPUT - Config Output Dump on StartUp --- remove when done
        ///
        msgHdr = "\n\n[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
        msgData = "\n-> EnviroCfg.InvntryModName                 : "   +               (EnviroCfg.InvntryModName                        ) + \
                  "\n-> EnviroCfg.LiquidCoolEn                   : "   + std::to_string(EnviroCfg.LiquidCoolEn                          ) + \
                  "\n-> EnviroCfg.LiquidCoolManModeEn            : "   + std::to_string(EnviroCfg.LiquidCoolManModeEn                   ) + \
                  "\n-> EnviroCfg.LiquidCoolManSetpoint          : "   + std::to_string(EnviroCfg.LiquidCoolManSetpoint                 ) + \
                  "\n-> EnviroCfg.LiquidCoolMaxTempThreshold     : "   + std::to_string(EnviroCfg.LiquidCoolMaxTempThreshold            ) + \
                  "\n-> EnviroCfg.LiquidCoolMinTempThreshold     : "   + std::to_string(EnviroCfg.LiquidCoolMinTempThreshold            ) + \
                  "\n-> EnviroCfg.LiquidCoolTempSamplePeriod     : "   + std::to_string(EnviroCfg.LiquidCoolTempSamplePeriod.count()    ) + \
                  "\n-> EnviroCfg.LiquidCoolMaxPressureThreshold : "   + std::to_string(EnviroCfg.LiquidCoolMaxPressureThreshold        ) + \
                  "\n-> EnviroCfg.LiquidCoolMinPressureThreshold : "   + std::to_string(EnviroCfg.LiquidCoolMinPressureThreshold        ) + \
                  "\n-> EnviroCfg.LiquidCoolPressureSamplePeriod : "   + std::to_string(EnviroCfg.LiquidCoolPressureSamplePeriod.count()) + \
                  "\n-> EnviroCfg.LiquidCoolMaxFlowRateThreshold : "   + std::to_string(EnviroCfg.LiquidCoolMaxFlowRateThreshold        ) + \
                  "\n-> EnviroCfg.LiquidCoolMinFlowRateThreshold : "   + std::to_string(EnviroCfg.LiquidCoolMinFlowRateThreshold        ) + \
                  "\n-> EnviroCfg.LiquidCoolFlowRateSamplePeriod : "   + std::to_string(EnviroCfg.LiquidCoolFlowRateSamplePeriod.count()) + \

                  "\n-> EnviroCfg.Cdu1Presence                   : "   + std::to_string(EnviroCfg.Cdu1Presence     ) + \
                  "\n-> EnviroCfg.Cdu1NetworkIP                  : "   +               (EnviroCfg.Cdu1NetworkIP    ) + \
                  "\n-> EnviroCfg.Cdu1NetworkPort                : "   + std::to_string(EnviroCfg.Cdu1NetworkPort  ) + \
                  "\n-> EnviroCfg.Cdu1ColdStandbyShutdown        : "   + std::to_string(EnviroCfg.Cdu1ColdStandbyShutdown  );

        std::cout  << msgHdr << msgData << std::endl;
#endif

        logger::verbose(__FILE__, __FUNCTION__, "LIQUID COOL Configuration load complete");
    }

    logger::verbose(__FILE__, __FUNCTION__, "Configuration load complete");

    return true;
}


void envMgrController::dumpEnviroConfig()
/// ***************************************************************************
/// Summary: -- UNDER CONSTRUCTION
///
///     Method for 'dumping' the contents of one or all config parameters to
///     the console, logger or both
///
/// Inputs:
///
///   < TBD >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
/*
    EnviroCfg.InvntryModName            = configIF.getParam<std::string>("FHI_MOD_INVENTORY"             );
    EnviroCfg.FanCtrlModName            = configIF.getParam<std::string>("FHI_MOD_FAN_CTRL"              );
    EnviroCfg.AirCoolEn                 = configIF.getParam<bool>       ("ENV_AIR_COOL_ENABLE"           );
    EnviroCfg.AirCoolManModeEn              = configIF.getParam<bool>       ("ENV_AIR_COOL_MANUAL_ENABLE"    );
    EnviroCfg.AirCoolManSetpoint        = configIF.getParam<mmData_t>   ("ENV_AIR_COOL_MANUAL_SETPOINT"  );
    EnviroCfg.AirCoolMaxThreshold       = configIF.getParam<temp_t>     ("ENV_AIR_COOL_MAXIMUM_THRESHOLD");
    EnviroCfg.AirCoolMinThreshold       = configIF.getParam<temp_t>     ("ENV_AIR_COOL_MINIMUM_THRESHOLD");
    EnviroCfg.AirCoolOperBands          = configIF.getParam<mmData_t>   ("ENV_AIR_COOL_OPERATIONAL_BANDS");
    EnviroCfg.AirCoolPwmFreq            = configIF.getParam<mmData_t>   ("ENV_AIR_COOL_FAN_PWM_FREQUENCY");

    EnviroCfg.AirCoolHysteresisUp       = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_UP"  ));
    EnviroCfg.AirCoolHysteresisDown     = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_DOWN"));
    EnviroCfg.AirCoolTempSamplePeriod   = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_AIR_COOL_SAMPLE_PERIOD"                 ));
    EnviroCfg.AirCoolTachSamplePeriod   = static_cast<timeout_t>(configIF.getParam<mmData_t>("ENV_AIR_COOL_TACHOMETER_SAMPLE_PERIOD"      ));
*/

    // Full dump


}


bool envMgrController::initThermalBandData()
/// ***************************************************************************
/// Summary:
///
///     Initialize thermal data struct with settings from config load
///
///     BAND_THERMAL_SIZE = (MAX_THRESHOLD - MIN_THRESHOLD) / NUMBER_OF_BANDS
///
///     Band X Upper Bound = MAX_THRESHOLD - ((NUMBER_OF_BANDS - X) * (BAND_THERMAL_SIZE))
///
///     Band X Lower Bound = MAX_THRESHOLD - ((NUMBER_OF_BANDS - X + 1) * (BAND_THERMAL_SIZE))
///
///     PWM Duty Cycle % of band X = 100 - ((NUMBER_OF_BANDS - X) * (100 / NUMBER_OF_BANDS))
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     true - method succeeded, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // VARS
    temp_t  BandThermSize = 0;

    // Config data
    temp_t  MaxTempThresh = 0;
    temp_t  MinTempThresh = 0;
    mmData_t  NumBands = 0;

    // init vars
    //
    // clear previous vector contents
    ThermalBand.clear();
    // ThermalBand.assign(NumBands+1, {});

    logger::verbose(__FILE__, __FUNCTION__, "Starting Initialization of Thermal Band Data ...");


    // get cooling parms
//    if(EnviroCfg.AirCoolEn)
    if(1)
    {
        MaxTempThresh = EnviroCfg.AirCoolMaxThreshold;
        MinTempThresh = EnviroCfg.AirCoolMinThreshold;
        NumBands = EnviroCfg.AirCoolOperBands;
    }
    else
    {
        // Add Liquid Cool parms here later
        return false;
    }

    /// PARt of Debug
    // std::cout << "\n\n" << std::endl;

    // BAND_THERMAL_SIZE = (MAX_THRESHOLD - MIN_THRESHOLD) / NUMBER_OF_BANDS
    BandThermSize = (MaxTempThresh - MinTempThresh) / static_cast<temp_t>(NumBands);

    ThermalBand.resize(NumBands + 1);

#if 0
    /// TODO - SSS : TEST OUTPUT -- remove when done
    msgHdr = "\n[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
    msgData = "BandThermSize: " + std::to_string(BandThermSize) + \
            " ThermalBand[size]: " + std::to_string(ThermalBand.size()) + \
            " MaxTempThresh: " + std::to_string(MaxTempThresh) + \
            " MinTempThresh: " + std::to_string(MinTempThresh) + \
            " NumBands: " + std::to_string(NumBands);
    std::cout  << msgHdr << msgData << std::endl;
#endif

    for(std::vector<EnvThermalBandStruct>::iterator it = ThermalBand.begin(); it < ThermalBand.end(); it++)
    {
        auto idx = std::distance(ThermalBand.begin(), it);
        auto nBands = NumBands;

        // Band X Upper Bound = MAX_THRESHOLD - ((NUMBER_OF_BANDS - X) * (BAND_THERMAL_SIZE))
        it->MaxBandThresh = MaxTempThresh - (static_cast<temp_t>(nBands - idx) * (BandThermSize));

        /// SSS TEST OFFSET
        ///
        if(0 == idx)
        {
            it->MinBandthresh = -50;
        }
        else
        {
            // Band X Lower Bound = MAX_THRESHOLD - ((NUMBER_OF_BANDS - X + 1) * (BAND_THERMAL_SIZE))
            it->MinBandthresh = MaxTempThresh - (static_cast<temp_t>(nBands - idx+1) * (BandThermSize));
        }

        // PWM Duty Cycle % of band X = 100 - ((NUMBER_OF_BANDS - X) * (100 / NUMBER_OF_BANDS))
        it->BandSetpoint = static_cast<uint32_t>(100 - ((nBands - idx) * (100 / nBands)));

#if 0
        /// TODO -- SSS -- TEST OUTPUT - remove when done
        msgHdr = "\n[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
        msgData = "idx: " + std::to_string(idx) + " nBands: " + std::to_string(nBands) + \
                " BandSetpoint: " + std::to_string(it->BandSetpoint) + \
                " MinBandthresh: " + std::to_string(it->MinBandthresh) + \
                " MaxBandThresh: " + std::to_string(it->MaxBandThresh);
        std::cout  << msgHdr << msgData << std::endl;
#endif
    }

    logger::verbose(__FILE__, __FUNCTION__, "Thermal Band Initialize complete");

    return true;
}

bool envMgrController::initAirCoolAlgo()
/// ***************************************************************************
/// Summary:
///
///     Additional method to initialize Air Cool Algorithm
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     true - method succeeded, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    CurrThermalBand = EnviroCfg.AirCoolMinOperBand;
    return true;
}

bool envMgrController::initModuleDevicesVect()
/// ***************************************************************************
/// Summary:
///
///     Initializes vector data required to construct access keys for Module
///     device parameters
///
/// Inputs:
///
///     < None >
///
/// Returns:
///
///     true - method succeeded, false otherwise
///
/// Assumptions:
///
///     < None >
///
/// ***************************************************************************
///
{
    // Init Module Data
    //
 /*
    for(std::size_t deviceIter = firstModuleDevice; deviceIter < maxModuleDevices; ++deviceIter)
    {
        ModuleDevicesVect.push_back({ \
            "PMOD_" + deviceKeys.at(static_cast<moduleManagerController::devices>(deviceIter)) + \
            "_PRESENCE", false, \
            "PMOD_" + deviceKeys.at(static_cast<moduleManagerController::devices>(deviceIter)) + \
            "_TEMP", -100} \
        );
    }
*/
    logger::verbose(__FILE__, __FUNCTION__, "Constructed Module Access Keys and Initialized Data structure...");

    return true;
}

bool envMgrController::loadModuleDevicesVect()
/// ***************************************************************************
/// Summary:
///
///     Loads and stores configuration data for System 'Module' devices
///     Mainly presence and temperature
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     true - method succeeded, false otherwise
///
/// Assumptions:
///
///   < None >
///
/// ***************************************************************************
///
{
    // Init Module Data
    // Log start
    logger::verbose(__FILE__, __FUNCTION__, "Attempting Retrieval of System Power Module Configuration, ( PMods Present )...");

/*
    //
    for(std::size_t deviceIter = firstModuleDevice; deviceIter < maxModuleDevices; deviceIter++)
    {
//        const moduleManagerController::devices dev = static_cast<moduleManagerController::devices>(deviceIter);
//        const std::string keyHeader{deviceKeys.at(dev)};

        ModuleDevicesVect[deviceIter].DevicePresent = configIF.getParam<bool>(ModuleDevicesVect[deviceIter].DevicePresenceAccessKey);
        ModuleDevicesVect[deviceIter].DeviceTemp = configIF.getParam<temp_t>(ModuleDevicesVect[deviceIter].DeviceTemperatureAccessKey);
    }
*/
    logger::verbose(__FILE__, __FUNCTION__, "Retrieval of System Power Module Configuration - COMPLETE");

    return true;
}


std::tuple<std::string, bool> envMgrController::getCoolingMode()
/// ***************************************************************************
/// Summary:
///
///     Get system Cooling MODE which currently is either AUTO or MANUAL
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     std::tuple (pair) denoting Cooling MODE in text either AUTO or MANUAL
///
/// Assumptions:
///
///   < None >
///
/// ***************************************************************************
///
{
    std::string     coolmode;


    if(EnviroCfg.AirCoolManModeEn) coolmode = "MANUAL";
    else coolmode = "AUTO";

    return std::make_tuple(coolmode, true);
}


std::tuple<std::string, bool> envMgrController::getCoolingType()
/// ***************************************************************************
/// Summary:
///
///     Get system Cooling TYPE which currently is either AIR_COOL or
///     LIQUID_COOL
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     std::tuple (pair) denoting Cooling TYPE in text either AIR_COOL or
///     LIQUID_COOL and validity state of data
///
/// Assumptions:
///
///   < None >
///
/// ***************************************************************************
///
{
    std::string     cooltype;

    if(EnviroCfg.AirCoolEn && !EnviroCfg.LiquidCoolEn) cooltype = "AIR_COOL";
    else if(!EnviroCfg.AirCoolEn && EnviroCfg.LiquidCoolEn) cooltype = "LIQUID_COOL";
    else cooltype = "AIR_COOL, LIQUID_COOL";

    return std::make_tuple(cooltype, true);
}


std::tuple<std::string, bool> envMgrController::getCoolingState()
/// ***************************************************************************
/// Summary:
///
///     Get system Cooling STATE which currently is either COOLING ENABLED or
///     COOLING DISABLED
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     std::tuple (pair) denoting Cooling STATE in text either COOLING ENABLED
///     or COOLING DISABLED and validity state of data
///
/// Assumptions:
///
///   < None >
///
/// ***************************************************************************
///
{
    std::string     coolstate;

    if(EnviroCfg.AirCoolEn) coolstate = "COOLING ENABLED";
    else coolstate = "COOLING DISABLED";

    return std::make_tuple(coolstate, true);
}


bool envMgrController::getMaxSystemTemp(temp_t& maxTemp)
/// ***************************************************************************
/// Summary:
///
///     Get Max system temperature from their reported Status via Status Processor
///
/// Inputs:
///
///     maxTemp -  referenced variable used to return temperature data
///
/// Returns:
///
///   true - method succeeded, false otherwise
///
/// Assumptions:
///
///   < None >
///
/// ***************************************************************************
///
{
    // VARS
    static temp_t   LastMaxTemp = _DEFAULT_CURR_MAX_TEMP;
    temp_t          CurrMaxTemp = _DEFAULT_CURR_MAX_TEMP;

//    std::size_t     iterValue = 0;

    std::bitset<32> PModTempError = 0x00000000;

    std::string     key{"PA_1_TEMP"};
    std::string     newPresentKey{"PA_1_PRESENT"};
    std::string     newValueKey{"PA_1_TEMP"};
    std::string     maxValueKey{"PA_1_TEMP"};
    std::string     sDataState{"NA"};

    const std::string parm1{"resultPayload"};
    const std::string parm2{"value"};
    const std::string valueExt{"_TEMP"};
    const std::string presentExt{"_PRESENT"};


/// TODO SSS : REWRITE TO USE PMOD TEMP from STATUS PROC MSG -WIP

    for( const auto device : deviceKeysPMods)
    {

        const std::string keyHeader = device.second;
        newPresentKey = keyHeader + presentExt;
        newValueKey = keyHeader + valueExt;

        ExtDeviceData[device.first].DevicePresenceAccessKey = newPresentKey;
        ExtDeviceData[device.first].DeviceTemperatureAccessKey = newValueKey;

        if( auto currDevTemp{requestIF.getStatusParam<temp_t>(newValueKey, parm1, parm2)}; currDevTemp.has_value() )
        {
            // We have current device with value
            ExtDeviceData[device.first].DeviceTemp = currDevTemp.value();

            // Do boundary check for valid value
            if((150 < ExtDeviceData[device.first].DeviceTemp)
                || (-50 > ExtDeviceData[device.first].DeviceTemp)
              )
            {
                // We have an INVALID Reading -- IGNORE for now
                PModTempError [device.first] = 1;
                CurrMaxTemp = _INVALID_CURR_MAX_TEMP;
                continue;
            }
            else
            {
                // test and set of Max Temp
//                std::cout << "CurrMaxTemp " << CurrMaxTemp << std::endl;
//                std::cout << "PMOD Device " << keyHeader << " Temp :  " << ExtDeviceData[device.first].DeviceTemp << std::endl;
                if( CurrMaxTemp < ExtDeviceData[device.first].DeviceTemp)
                {
//                    std::cout << "PMOD Device " << keyHeader << " New High Temp, " << std::endl;
                    CurrMaxTemp = ExtDeviceData[device.first].DeviceTemp;
                    maxValueKey = newValueKey;
                }

                // we got data clear error bit
                PModTempError [device.first] = 0;

                /// TODO -- SSS -- TEST OUTPUT - remove when done
                msgHdr = "\n[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
                msgData = "keyHeader: " + keyHeader + \
                        ", deviceIter: " + std::to_string(device.first) + ", newValueKey: " + newValueKey + \
                        ", DeviceTemp: " + std::to_string(ExtDeviceData[device.first].DeviceTemp) + \
                        ", maxValue: " + std::to_string(CurrMaxTemp);
//                std::cout  << msgHdr << msgData << std::endl;
            }
        }
        else
        {
//            std::cout << "PMOD Device " << keyHeader << " Temp NOT Present, " << std::endl;
            PModTempError [device.first] = 1;

        }

    }   // end of for loop


#if 0  /// --- TEST CODE ---
///
///
    temp_t         maxValue = 0;
    static int     testTemp;
    static int     dirDn;

    if(dirDn)
    {
        if(--testTemp < 0)
        {
            dirDn = false;
        }
    }
    else
    {
        if(++testTemp > 80)
        {
            dirDn = true;
        }
    }

    CurrMaxTemp = maxValue;

    if( testTemp > CurrMaxTemp)
    {
        // Set Result
        maxTemp = testTemp;
        setSamplePeriodStartPoint();

        std::string sMaxTemp = std::to_string(maxTemp);

        logger::verbose(__FILE__, __FUNCTION__, std::string("Getting System Device Temperatures...") +
            keyHeader + std::string(" is MAX TEMP @ ") + sMaxTemp);

        return true;
    }

///
///
/// END of TEST Code
///
#endif


    if(LastMaxTemp != CurrMaxTemp)
    {
        LastMaxTemp = CurrMaxTemp;
        std::string sMaxTemp = std::to_string(CurrMaxTemp);

        logger::verbose(__FILE__, __FUNCTION__, std::string("Getting System Device Temperatures...") +
                        maxValueKey + std::string(" is MAX TEMP @ ") + sMaxTemp);
    }

    // Set Result
    maxTemp = CurrMaxTemp;

    // reset timer for next poll
    setSamplePeriodStartPoint();

    switch(CurrMaxTemp)
    {
        case _DEFAULT_CURR_MAX_TEMP:
            sDataState = "N/A";
            EnviroData[CurrentMaxTemp].data = sDataState;
            EnviroData[CurrentMaxTemp].valid = true;
            coolingFailsafe();
            break;

        case _INVALID_CURR_MAX_TEMP:
            sDataState = "INVALID";
            EnviroData[CurrentMaxTemp].data = sDataState;
            EnviroData[CurrentMaxTemp].valid = true;
            coolingFailsafe();
            break;

        default:
            // update Reported data
            EnviroData[CurrentMaxTemp].data = maxTemp;
            EnviroData[CurrentMaxTemp].valid = true;

            // set Failsafe status to OFF and update Reported data
            setFailsafeActive(false);
            EnviroData[FailsafeActive].data = false;
            EnviroData[FailsafeActive].valid = true;
            break;
    }

    return true;
}

bool envMgrController::computeCoolPoints(mmData_t setpoint, pwm_t& hival, pwm_t& loval)
/// ***************************************************************************
/// Summary:
///
///     Calculate  FPGA values for cooling level high and low times used for
///     PWM duty cycle control (i.e. setpoint )
///
/// Inputs:
///
///     setpoint -  For Air Cool systems: value ranging from 0-100, representing
///                 the range of fan speed (0 = off, 100 = full speed)
///
///     hival -     For Air Cool systems: reference to var used to return the
///                 clock count for PWM 'ON' (high) period
///
///     loval -     For Air Cool systems: reference to var used to return the
///                 clock count for PWM 'OFF' (low) period
///
/// Returns:
///
///   bool = true if successful, false otherwise
///
/// Assumptions:
///
///   < None >
///
/// ***************************************************************************
///
{
    // vars
    pwm_t           Fan_Clock_Period{0x189E}; // Period for 16 KHz, should be pulled from configuration
    double          DutyCycle{setpoint / 100.0};
    static double   Computed_Fan_Period{0};

    /// Added calls and code to obtain system clock period and Fan KHz
    ///
    if(std::abs(Computed_Fan_Period) < 0.1)
    {
        if(auto resp{fpgaIF.read(EnviroCfg.InvntryModName, sysInvntryRegisterMap::sysfreq)}; resp.has_value())
        {
            pwm_t System_Clock_Freq_Hz{resp.value_or(0)};

            if((System_Clock_Freq_Hz > 300000000) || (System_Clock_Freq_Hz < 1000000))
            {
                System_Clock_Freq_Hz = 100000000;
            }

            Computed_Fan_Period = (static_cast<double>(System_Clock_Freq_Hz) / static_cast<double>(EnviroCfg.AirCoolPwmFreq));
        }
        else
        {
            // update refs with computed values
            hival = static_cast<pwm_t>((DutyCycle * static_cast<double>(Fan_Clock_Period)));
            loval = Fan_Clock_Period - hival;
            Computed_Fan_Period = 0;
            return true;
        }
    }

    // update refs with computed values
    hival = static_cast<pwm_t>(DutyCycle * Computed_Fan_Period);
    loval = static_cast<pwm_t>(Computed_Fan_Period) - hival;

#if 0
    /// TODO -- SSS -- TEST OUTPUT - remove when done
    msgHdr = "\n\n[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
    msgData = "DutyCycle: " + std::to_string(DutyCycle) + \
            " Computed_Fan_Period: " + std::to_string(Computed_Fan_Period) + \
            " Fan_Clock_Period: " + std::to_string(Fan_Clock_Period) + \
            " hival: " + std::to_string(hival) + " loval: " + std::to_string(loval) + \
            " System_Clock_Freq_Hz: " + std::to_string(System_Clock_Freq_Hz);
    std::cout  << msgHdr << msgData << std::endl;
#endif

    return true;
}

bool envMgrController::setFanPWM(mmData_t setpoint)
/// ***************************************************************************
/// Summary:
///
///     Writes PWM Hi and Lo Registers with proper values computed from setpoint
///
/// Inputs:
///
///     setpoint - duty cycle setpoint for current Thermal Band
///
/// Returns:
///
///     boolean output of function true is success, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // vars
    bool        result{true};
    pwm_t       PwmOnTime{0};
    pwm_t       PwmOffTime{0};


    computeCoolPoints(setpoint, PwmOnTime, PwmOffTime);
    EnviroData[CurrentSetpoint].data = setpoint;
    EnviroData[CurrentSetpoint].valid = true;

    /// TODO -- SSS -- TEST OUTPUT - remove when done
    logger::verbose(__FILE__, __FUNCTION__, std::string("Fan Setting Update : ") +
        std::to_string(setpoint) + std::string("%, ") + std::to_string(PwmOnTime) + std::string(" ON Time, ") +
        std::to_string(PwmOffTime) + std::string(" OFF Time"));

    /// TODO -- SSS Add code to take some action on FPGA write errors, currently only detects
    ///
    // Write PWM regs
    // 'ON' time
    if(!fpgaIF.write(EnviroCfg.FanCtrlModName, fanRegisterMapOffset::pwmhigh, PwmOnTime))
    {
        logger::warn(__FILE__, __FUNCTION__, "***ERROR - FAILED TO WRITE PWM ON TIME TO FPGA");
        result = false;
    }

    // 'OFF' time
    if(!fpgaIF.write(EnviroCfg.FanCtrlModName, fanRegisterMapOffset::pwmlow, PwmOffTime))
    {
        logger::warn(__FILE__, __FUNCTION__, "***ERROR - FAILED TO WRITE PWM OFF TIME TO FPGA");
        result = false;
    }

    logger::severity_t logSeverity{logger::severity_t::verbose};
    bool fanEnable{setpoint > 0};
    std::string logMsg;

    if(setFanEnableState(fanEnable))
    {
        logMsg = ((fanEnable)?("Fan Enable Engaged..."):("Fan Enable Disengaged..."));
    }
    else
    {
        logMsg = "***ERROR - COMMAND FAILED!!!";
        logSeverity = logger::severity_t::warn;
        result = false;
    }
    logger::genLog(__FILE__, __FUNCTION__, logMsg, logSeverity);

    setHysteresisStartPoint();

    return result;
}


void envMgrController::setCoolSetpointComplete(bool complete)
{
    CoolSetpointComplete = complete;
}


bool envMgrController::getCoolSetpointComplete()
{
    return CoolSetpointComplete;
}

bool envMgrController::regulateCoolLevel()
/// ***************************************************************************
/// Summary:
///
///     Based on current Max System Temp, compute following:
/// 	- whether max temp has changed
/// 	- whether setpoint for cooling needs modification
/// 	- if so modify setpoint and update Status data
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     boolean output of function true is success, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // vars
    temp_t          maxtemp;
    std::size_t     bandCnt = EnviroCfg.AirCoolOperBands + 1;



    getMaxSystemTemp(maxtemp);

    // do FailSafe test
    if(( _DEFAULT_CURR_MAX_TEMP == maxtemp) || ( _INVALID_CURR_MAX_TEMP == maxtemp))
    {
//        std::cout << "No Valid Temperatures Found!!!, Engaging FailSafe Mode ..." << std::endl;
        logger::warn(__FILE__, __FUNCTION__, "No Valid Temperatures Found!!!, Engaging FailSafe Mode ...");
        maxtemp = _FAILSAFE_ERROR_MAX_TEMP;
    }




/*
    /// TODO -- SSS -- TEST OUTPUT - remove when done
    std::cout  << "\n\n[ " << __FUNCTION__ << "(): line "<< __LINE__ <<" ]: "<< std::endl;

    /// TODO -- SSS -- TEST OUTPUT - remove when done
    msgHdr = "\n\n[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
    msgData = "maxtemp: " + std::to_string(maxtemp) + \
            ", CurrThermalBand: " + std::to_string(CurrThermalBand) + \
            ", MinBandthresh: " + std::to_string(ThermalBand[CurrThermalBand].MinBandthresh) + \
            ", MaxBandThresh: " + std::to_string(ThermalBand[CurrThermalBand].MaxBandThresh);
    std::cout  << msgHdr << msgData << std::endl;
*/


    /**
     * Check to see of we are in correct Thermal Band AND
     * That we have transitions Fan setpoint to this band
     *
     * We check both because fan can lag behind due to hysteresis
     *
     */
    if(((maxtemp >= ThermalBand[CurrThermalBand].MinBandthresh) && \
        (maxtemp <= ThermalBand[CurrThermalBand].MaxBandThresh) && \
        getCoolSetpointComplete()) \
    )
    {
//        return true;
    }
    else if(((maxtemp >= ThermalBand[CurrThermalBand].MinBandthresh) && \
        (maxtemp <= ThermalBand[CurrThermalBand].MaxBandThresh) && \
        !getCoolSetpointComplete()) \
    )
    {
        setFanPWM( ThermalBand[CurrThermalBand].BandSetpoint );
        setCoolSetpointComplete(true);
    }
    else if((maxtemp > ThermalBand[CurrThermalBand].MaxBandThresh) && ((CurrThermalBand + 1) <= (bandCnt - 1)))
    {
        if(isTransitionUpTimeoutExpired())
        {
            // we have OK to transition up to next band
            setCoolSetpointComplete(false);

            // get setpoint for current next maxtemp
            ++CurrThermalBand;
            if(CurrThermalBand >= bandCnt)
            {
                CurrThermalBand = bandCnt - 1;
                setCoolSetpointComplete(true);
            }
            else if(maxtemp <= ThermalBand[CurrThermalBand].MaxBandThresh)
            {
                setCoolSetpointComplete(true);
            }
            else
            {
                setCoolSetpointComplete(false);
            }

            setFanPWM(ThermalBand[CurrThermalBand].BandSetpoint);
        }
    }
    else if((maxtemp < ThermalBand[CurrThermalBand].MinBandthresh) && (static_cast<int>(CurrThermalBand - 1) >= 0))
    {
        // We have a new temp different than previous
        if( isTransitionDownTimeoutExpired())
        {
            // we have OK to transition down to next band
            setCoolSetpointComplete(false);
            if(CurrThermalBand > EnviroCfg.AirCoolMinOperBand)
            {
                --CurrThermalBand;
            }
            else
            {
                CurrThermalBand = EnviroCfg.AirCoolMinOperBand;
            }

            setFanPWM(ThermalBand[CurrThermalBand].BandSetpoint);

            setCoolSetpointComplete( \
                (maxtemp <= ThermalBand[CurrThermalBand].MaxBandThresh) && \
                (maxtemp >= ThermalBand[CurrThermalBand].MinBandthresh) \
            );
        }
    }

    // Update EnviroData
    EnviroData[CurrentThermalBand].data = CurrThermalBand;
    EnviroData[CurrentThermalBand].valid = true;
    EnviroData[CurrentSetpoint].data = ThermalBand[CurrThermalBand].BandSetpoint;
    EnviroData[CurrentSetpoint].valid = true;


    /// TODO -- SSS -- TEST OUTPUT - remove when done
    msgHdr = "\n\n[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
    msgData = "maxtemp: " + std::to_string(maxtemp) + \
            ", CurrThermalBand: " + std::to_string(CurrThermalBand) + \
            ", CurrSetPoint: " + std::to_string(ThermalBand[CurrThermalBand].BandSetpoint) + \
            ", MinBandthresh: " + std::to_string(ThermalBand[CurrThermalBand].MinBandthresh) + \
            ", MaxBandThresh: " + std::to_string(ThermalBand[CurrThermalBand].MaxBandThresh);
//    std::cout  << msgHdr << msgData << std::endl;

    return true;
}


void envMgrController::coolingFailsafe(void)
/// ***************************************************************************
/// Summary:
///
///     Perform FailSafe check of cooling state of system
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    /// TODO : ADD FailSafe check for Engagement
    ///

    // We are called because we have no valid temperature reported
    // Engage Failsafe mode
    setFailsafeActive(true);
    // update Reported data
    EnviroData[FailsafeActive].data = true;
    EnviroData[FailsafeActive].valid = true;

    std::string sDataState = "N/A";
    EnviroData[CurrentMaxTemp].data = sDataState;
    EnviroData[CurrentMaxTemp].valid = true;


}


bool envMgrController::getFanTachs()
/// ***************************************************************************
/// Summary:
///
///     Gathers Fan tach info from FPGA, converts to RPM and stores in
///     EnviroData struct
///
///     FPGA Req'd calculation for RPM:
///     RPM = 60 x (1/register value * number of pulses per clock * clock period)
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     boolean output of function true is success, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // VARS
    bool result = false;

    std::chrono::milliseconds zeroTime(0);
    std::chrono::milliseconds defaultTime(500);

    // First get tachwin size
    // Use tachwin for conversion
    //
    if(auto tachWin{fpgaIF.read(EnviroCfg.FanCtrlModName, fanRegisterMapOffset::tachwin)}; 0 == tachWin.value_or(0))
    {
//        auto writeVal{static_cast<mmData_t>(EnviroCfg.AirCoolTachSamplePeriod.count())};
        unsigned int writeVal{0};

        if(0 == EnviroCfg.AirCoolTachSamplePeriod.count())
        {
            writeVal = _DEFAULT_TACH_WIN_REG_VALUE;
            EnviroCfg.AirCoolTachSamplePeriod = defaultTime;
        }
        else
        {
            writeVal = static_cast<mmData_t>(EnviroCfg.AirCoolTachSamplePeriod.count() * 100000);
        }

        fpgaIF.write(EnviroCfg.FanCtrlModName, fanRegisterMapOffset::tachwin, writeVal);
    }

    // Compute RPM conversion factor
    mmData_t convertToRPM = static_cast<mmData_t>(((_MSEC_PER_MINUTE / EnviroCfg.AirCoolTachSamplePeriod.count()) / _DEFAULT_PULSES_PER_REV));

    /// PART of Debug
    // std::cout << "\n\n" << std::endl;

    // Get Fan tach info
    for(std::size_t i = 0; i < MAX_SYS_FAN_COUNT; ++i)
    {
        if(EnviroCfg.AirCoolFanPresent.at(i))
        {
            auto rawTachData{fpgaIF.read(EnviroCfg.FanCtrlModName, (fanRegisterMapOffset::tach1reg + (i << 2)))};

            // compute conversion factor for RPM here
            convertToRPM = static_cast<mmData_t>((_MSEC_PER_MINUTE / EnviroCfg.AirCoolTachSamplePeriod.count()) / \
                                                 EnviroCfg.AirCoolPulsesPerRev.at(i));

            // add store EnviroData struct
            EnviroData[ i + EnvDataIndex::Fan1RPM ].data = (rawTachData.value_or(0) * convertToRPM);
            EnviroData[ i + EnvDataIndex::Fan1RPM].valid = true;

            /// TODO -- SSS -- TEST OUTPUT - remove when done
#if 0
            uint64_t rpm_val = (rawTachData.value_or(0) * convertToRPM);
            msgHdr = "[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
            msgData = "Fan " + std::to_string(i + 1) + \
                    " RPMs: " + std::to_string( rpm_val ) +\
                    " rawTachData: " + std::to_string( rawTachData.value_or(1000) ) + \
                    " convertToRPM: " + std::to_string(convertToRPM);
            std::cout  << msgHdr << msgData << std::endl;
#endif

        }
    }
//    std::cout  << "\n\n" << std::endl;

    return result;
}

bool envMgrController::pushEnviroStatus( )
/// ***************************************************************************
/// Summary:
///
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     boolean output of function true is success, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    return updateIF.updateData(EnviroData);


}

bool envMgrController::pushAuxCduStatus( )
/// ***************************************************************************
/// Summary:
///
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///     boolean output of function true is success, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{

    return updateIF.updateData(AuxCdu1Data);

}

void envMgrController::setHysteresisStartPoint(void)
/// ***************************************************************************
/// Summary:
///
///     Sets var tmrHysteresisStartPoint to steady_clock::now()
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    tmrHysteresisStartPoint = std::chrono::steady_clock::now();
}

void envMgrController::getHysteresisStartPoint(std::chrono::steady_clock::time_point& timePoint)
/// ***************************************************************************
/// Summary:
///
///     Gets value of var tmrHysteresisStartPoint
///
/// Inputs:
///
///     Reference to a timepoint variable to return timepoint value
///
/// Returns:
///
///   Returns timepoint value equal to current tmrHysteresisStartPoint
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    timePoint = tmrHysteresisStartPoint;
}

void envMgrController::setSamplePeriodStartPoint()
/// ***************************************************************************
/// Summary:
///
///     Sets var tmrSamplePeriodStartPoint to steady_clock::now()
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    tmrSamplePeriodStartPoint = std::chrono::steady_clock::now();
}

void envMgrController::getSamplePeriodStartPoint(std::chrono::steady_clock::time_point& timePoint)
/// ***************************************************************************
/// Summary:
///
///     Gets value var tmrSamplePeriodStartPoint
///
/// Inputs:
///
///     Reference to a timepoint variable to return timepoint value
///
/// Returns:
///
///   Returns timepoint value equal to current tmrSamplePeriodStartPoint
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    timePoint = tmrSamplePeriodStartPoint;
}

bool envMgrController::isSamplePeriodTimeOutExpired()
/// ***************************************************************************
/// Summary:
///
///     Computes elapsed time (duration) since last poll of system devices
///     for temperature readings and compares duration with configured delay
///     for sample polling period [ ENV_AIR_COOL_SAMPLE_PERIOD ]
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   true - if period has expired, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    std::chrono::milliseconds durationAirCoolTempSamplePeriod(EnviroCfg.AirCoolTempSamplePeriod);
    std::chrono::steady_clock::time_point SamplePeriodCheckPoint = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point SamplePeriodStartPoint;
    getSamplePeriodStartPoint(SamplePeriodStartPoint);

    std::chrono::milliseconds elapsed(std::chrono::duration_cast<std::chrono::milliseconds>(SamplePeriodCheckPoint - SamplePeriodStartPoint).count());

    return (elapsed >= durationAirCoolTempSamplePeriod);
}

bool envMgrController::isTransitionUpTimeoutExpired()
/// ***************************************************************************
/// Summary:
///
///     Computes elapsed time (duration) since last PWM write and compares
///     duration with configured delay for temperature transition UP period
///     [ ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_UP ]
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   true - if period has expired, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // VARS
    std::chrono::steady_clock::time_point   hysterCheckPoint = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point   hysterStartPoint;
    std::chrono::milliseconds               durationAirCoolHysteresisUp(EnviroCfg.AirCoolHysteresisUp);

    // init vars
    getHysteresisStartPoint(hysterStartPoint);

    std::chrono::milliseconds elapsed(std::chrono::duration_cast<std::chrono::milliseconds>(hysterCheckPoint - hysterStartPoint).count());

    return (elapsed >= durationAirCoolHysteresisUp);
}

bool envMgrController::isTransitionDownTimeoutExpired()
/// ***************************************************************************
/// Summary:
///
///     Computes elapsed time (duration) since last PWM write and compares
///     duration with configured delay for temperature transition DOWN period
///     [ ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_DOWN ]
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   true - if period has expired, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    std::chrono::steady_clock::time_point   hysterCheckPoint = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point   hysterStartPoint;
    std::chrono::milliseconds               durationAirCoolHysteresisDown(EnviroCfg.AirCoolHysteresisDown);

    // init vars
    getHysteresisStartPoint(hysterStartPoint);

    std::chrono::milliseconds elapsed(std::chrono::duration_cast<std::chrono::milliseconds>(hysterCheckPoint - hysterStartPoint).count());

    return (elapsed >= durationAirCoolHysteresisDown);
}


void envMgrController::coolingAirCoolTickerTasks()
/// ***************************************************************************
/// Summary:
///
///     Tasks related to Air Cool functionality triggered by a 'ticker' method
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    //vars
    temp_t      maxTemp = -99;

    std::string sDataState = "NA";


    // Exit if we have not completed initialization
    //
    if(!EnviroMgrInitComplete)
    {
//        std::cout << "\n\n[" << __FUNCTION__ << "] EnviroMgrInitComplete False, Exiting......" << std::endl;
        logger::verbose(__FILE__, __FUNCTION__, "EnviroMgrInitComplete False, Exiting ...");
        return;
    }

    std::tie (EnviroData[CoolType].data, EnviroData[CoolType].valid) = getCoolingType();

    if(EnviroCfg.AirCoolEn)
    {
        if(EnviroCfg.AirCoolManModeEn)
        {
            // we need to force Duty Cycle setpoint
            //Validate Manual Setpoint
            if(EnviroCfg.AirCoolManSetpoint > 100)
            {
                EnviroCfg.AirCoolManSetpoint = 100;
            }
            setFanPWM(EnviroCfg.AirCoolManSetpoint);

            // update actual system max temp
            getMaxSystemTemp(maxTemp);

            // update Reported data -
            // We're Manual, Not using Temp or Thermal Bands
//            EnviroData[CurrentMaxTemp].data = sDataState;
//            EnviroData[CurrentMaxTemp].valid = true;

            EnviroData[CurrentThermalBand].data = sDataState;
            EnviroData[CurrentThermalBand].valid = true;
            getFanTachs();
            pushEnviroStatus();

        }
        else
        {
            if( isSamplePeriodTimeOutExpired() )
            {
                regulateCoolLevel();
                getFanTachs();
                pushEnviroStatus();
            }
        }
        std::tie (EnviroData[CoolMode].data, EnviroData[CoolMode].valid) = getCoolingMode();

    }
    else
    {
//        std::cout << "\n\n[" << __FUNCTION__ << "] Air Cool NOT ENABLED......" << std::endl;

    }

}


void envMgrController::getAuxCdu1Data()
/// ***************************************************************************
/// Summary:
///
///     Tasks related to Air Cool functionality triggered by a 'ticker' method
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // vars
    SYSTEM_REPORT_DATA_S    CduStatusData;
//    GENERAL_COMMANDS        CduCommandData;


    bool                systemON{false};

    std::string         keySysState{"RF_ONLINE_STATE"};

    const std::string   parm1{"resultPayload"};
    const std::string   parm2{"value"};
    const std::string   staleData{"--"};

    using namespace helpers::types;

    // Exit if we have not completed initialization
    //
    if(!EnviroMgrInitComplete)
    {
//        std::cout << "\n\n[" << __FUNCTION__ << "] EnviroMgrInitComplete False, Exiting......" << std::endl;
        logger::warn(__FILE__, __FUNCTION__, "EnviroMgrInitComplete False, Exiting......");
        return;
    }

    std::tie (EnviroData[CoolType].data, EnviroData[CoolType].valid) = getCoolingType();
    auto currSysState = strToOnlineState(requestIF.getStatusParam<std::string>(keySysState, parm1, parm2).value_or("COLD"));
    systemON = currSysState != online_state_t::COLD;


    if(EnviroCfg.LiquidCoolEn)
    {
        switch(SystemTypeInfo.mode)
        {
            case STANDALONE:
            case SOS_BOOSTER:
                break;

            case SOS_CONTROLLER:
#if 0
                /// TODO : REFACTOR here
                ///
                ///
#else

                if(EnviroCfg.Cdu1Presence && systemON)
                {
                    // Let's check state of our system
//                    CduCommandData.commandID = htonl(TLV_SET_PUMPS_ON);
//                    cduClient.send_data(&CduCommandData, sizeof(CduCommandData));

                    // We are either ONLINE or in HOT STANDBY -
                    // Connect to CDU and get data
                    if(cduClient.clientConnected())
                    {
//                        std::cout << "\n ===>>> *** Still connected, Checking CDU Data ***" << std::endl;
                        logger::verbose(__FILE__, __FUNCTION__, "cduClient Connected, Checking for CDU Data  ...");
                        cduClient.receive();

                        // Let's check state of our system
//                        CduCommandData.commandID = htonl(TLV_SET_PUMPS_ON);
//                        if(cduClient.send_data(&CduCommandData, sizeof(CduCommandData)) )
//                        {
//                            cduClient.receive();
//                        }
//                        else
//                        {
//                            cduClient.setConnected(false);
//                        }
                    }
                    else
                    {
//                        std::cout << "\n ===>>> *** Starting cduClient ***" << std::endl;
                        logger::verbose(__FILE__, __FUNCTION__, "cduClient Disconnected, (re)starting cduClient ...");
                        cduClient.conn();
                        logger::verbose(__FILE__, __FUNCTION__, "Checking for CDU Data after (re)connect ...");
                        if(cduClient.clientConnected())
                        {
//                            std::cout << "\n ===>>> *** Checking CDU Data after reconnect ***" << std::endl;
//                            CduCommandData.commandID = htonl(TLV_SET_PUMPS_ON);
                            logger::verbose(__FILE__, __FUNCTION__, "cduClient Connected, Checking for CDU Data  ...");
                            cduClient.receive();
                        }
                    }

                    if( cduClient.getStatusAvailable())
                    {
                        logger::verbose(__FILE__, __FUNCTION__, "CDU Data available, retrieving ...");
                        // We need to collect data
                        CduStatusData = cduClient.getStatusData();

                        // COMMS State
                        AuxCdu1Data[ENV_CDU_COMMS_OK].data = cduClient.clientConnected();
                        AuxCdu1Data[ENV_CDU_COMMS_OK].valid = true;


                        // ERROR Status
                        AuxCdu1Data[ENV_CDU_PUMP_LEAK].data = CduStatusData.ALARM_STATE.PUMP_LEAK;
                        AuxCdu1Data[ENV_CDU_PUMP_LEAK].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_PHASE_ERROR].data = CduStatusData.ALARM_STATE.PHASE_ERROR;
                        AuxCdu1Data[ENV_CDU_PUMP_PHASE_ERROR].valid = true;

                        AuxCdu1Data[ENV_CDU_SYSTEM_EMERGENCY_STOP].data = CduStatusData.ALARM_STATE.EMERGENCY_STOP;
                        AuxCdu1Data[ENV_CDU_SYSTEM_EMERGENCY_STOP].valid = true;

                        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_SHUTDOWN].data = CduStatusData.ALARM_STATE.ACC1_SHUTDOWN;
                        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_SHUTDOWN].valid = true;

                        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_2_SHUTDOWN].data = CduStatusData.ALARM_STATE.ACC2_SHUTDOWN;
                        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_2_SHUTDOWN].valid = true;

                        AuxCdu1Data[ENV_CDU_SYSTEM_REMOTE_ON_OFF].data = CduStatusData.ALARM_STATE.REMOTE_ON_OFF;
                        AuxCdu1Data[ENV_CDU_SYSTEM_REMOTE_ON_OFF].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_1_EXCHANGE_OVERPRESSURE].data = CduStatusData.ALARM_STATE.LOOP1_EXCHNG_PRES;
                        AuxCdu1Data[ENV_CDU_LOOP_1_EXCHANGE_OVERPRESSURE].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_1_FILTER_OVERPRESSURE].data = CduStatusData.ALARM_STATE.LOOP1_FILTER_PRES;
                        AuxCdu1Data[ENV_CDU_LOOP_1_FILTER_OVERPRESSURE].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_2_EXCHANGE_OVERPRESSURE].data = CduStatusData.ALARM_STATE.LOOP2_EXCHNG_PRES;
                        AuxCdu1Data[ENV_CDU_LOOP_2_EXCHANGE_OVERPRESSURE].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_2_FILTER_OVERPRESSURE].data = CduStatusData.ALARM_STATE.LOOP2_FILTER_PRES;
                        AuxCdu1Data[ENV_CDU_LOOP_2_FILTER_OVERPRESSURE].valid = true;

                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_FULL].data = CduStatusData.ALARM_STATE.RESERVOIR1_FULL;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_FULL].valid = true;

                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_LOW].data = CduStatusData.ALARM_STATE.RESERVOIR1_LOW;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_LOW].valid = true;

                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_FULL].data = CduStatusData.ALARM_STATE.RESERVOIR2_FULL;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_FULL].valid = true;

                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_LOW].data = CduStatusData.ALARM_STATE.RESERVOIR2_LOW;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_LOW].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_1_ALARM].data = CduStatusData.ALARM_STATE.PUMP1_ALARM;
                        AuxCdu1Data[ENV_CDU_PUMP_1_ALARM].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_2_ALARM].data = CduStatusData.ALARM_STATE.PUMP2_ALARM;
                        AuxCdu1Data[ENV_CDU_PUMP_2_ALARM].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_3_ALARM].data = CduStatusData.ALARM_STATE.PUMP3_ALARM;
                        AuxCdu1Data[ENV_CDU_PUMP_3_ALARM].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_4_ALARM].data = CduStatusData.ALARM_STATE.PUMP4_ALARM;
                        AuxCdu1Data[ENV_CDU_PUMP_4_ALARM].valid = true;


                        // We move his later to another method
                        // Parse data into report
                        AuxCdu1Data[ENV_CDU_PUMP_1_FLOW].data = CduStatusData.PUMP_INFO[0].pumpFlow;
                        AuxCdu1Data[ENV_CDU_PUMP_1_FLOW].valid = true;
                        AuxCdu1Data[ENV_CDU_PUMP_1_TEMP].data = CduStatusData.PUMP_INFO[0].pumpTemp;
                        AuxCdu1Data[ENV_CDU_PUMP_1_TEMP].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_2_FLOW].data = CduStatusData.PUMP_INFO[1].pumpFlow;
                        AuxCdu1Data[ENV_CDU_PUMP_2_FLOW].valid = true;
                        AuxCdu1Data[ENV_CDU_PUMP_2_TEMP].data = CduStatusData.PUMP_INFO[1].pumpTemp;
                        AuxCdu1Data[ENV_CDU_PUMP_2_TEMP].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_3_FLOW].data = CduStatusData.PUMP_INFO[2].pumpFlow;
                        AuxCdu1Data[ENV_CDU_PUMP_3_FLOW].valid = true;
                        AuxCdu1Data[ENV_CDU_PUMP_3_TEMP].data = CduStatusData.PUMP_INFO[2].pumpTemp;
                        AuxCdu1Data[ENV_CDU_PUMP_3_TEMP].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_4_FLOW].data = CduStatusData.PUMP_INFO[3].pumpFlow;
                        AuxCdu1Data[ENV_CDU_PUMP_4_FLOW].valid = true;
                        AuxCdu1Data[ENV_CDU_PUMP_4_TEMP].data = CduStatusData.PUMP_INFO[3].pumpTemp;
                        AuxCdu1Data[ENV_CDU_PUMP_4_TEMP].valid = true;

                        if( CduStatusData.LOOP_INFO_AVAIL[0])
                        {
                            AuxCdu1Data[ENV_CDU_LOOP_1_PRESSURE].data = CduStatusData.LOOP_INFO[0].loopPressure;
                            AuxCdu1Data[ENV_CDU_LOOP_1_PRESSURE].valid = true;
                            AuxCdu1Data[ENV_CDU_LOOP_1_TEMP].data = CduStatusData.LOOP_INFO[0].loopTemp;
                            AuxCdu1Data[ENV_CDU_LOOP_1_TEMP].valid = true;
                        }
                        else
                        {
                            AuxCdu1Data[ENV_CDU_LOOP_1_PRESSURE].data = std::string("--");
                            AuxCdu1Data[ENV_CDU_LOOP_1_PRESSURE].valid = true;
                            AuxCdu1Data[ENV_CDU_LOOP_1_TEMP].data = std::string("--");
                            AuxCdu1Data[ENV_CDU_LOOP_1_TEMP].valid = true;
                        }

                        if( CduStatusData.LOOP_INFO_AVAIL[1])
                        {
                            AuxCdu1Data[ENV_CDU_LOOP_2_PRESSURE].data = CduStatusData.LOOP_INFO[1].loopPressure;
                            AuxCdu1Data[ENV_CDU_LOOP_2_PRESSURE].valid = true;
                            AuxCdu1Data[ENV_CDU_LOOP_2_TEMP].data = CduStatusData.LOOP_INFO[1].loopTemp;
                            AuxCdu1Data[ENV_CDU_LOOP_2_TEMP].valid = true;
                        }
                        else
                        {
                            AuxCdu1Data[ENV_CDU_LOOP_2_PRESSURE].data = std::string("--");
                            AuxCdu1Data[ENV_CDU_LOOP_2_PRESSURE].valid = true;
                            AuxCdu1Data[ENV_CDU_LOOP_2_TEMP].data = std::string("--");
                            AuxCdu1Data[ENV_CDU_LOOP_2_TEMP].valid = true;
                        }


                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_TEMP].data = CduStatusData.RESV_TEMP[0].reservoirTemp;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_TEMP].valid = true;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_TEMP].data = CduStatusData.RESV_TEMP[1].reservoirTemp;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_TEMP].valid = true;

                    }
                }
//                else if(EnviroCfg.Cdu1Presence && !systemON)
                else
                {
                    cduClient.receive();

//                    if(cduClient.clientConnected())
//                    {
////                        std::cout << "\n\n[" << __FUNCTION__ << "] Current System State is OFF so we turn CDU 1 OFF" << std::endl;
//                        // we are not on - shut off pump
//                        CduCommandData.commandID = htonl(TLV_SET_PUMPS_OFF);
//                        if(!cduClient.send_data(&CduCommandData, sizeof(CduCommandData)) )
//                        {
//                            cduClient.setConnected(false);
//                        }
//
//                    }
//                    else
//                    {
//                        cduClient.conn();
//                        if(cduClient.clientConnected())
//                        {
//                            //   std::cout << "\n\n[" << __FUNCTION__ << "] Current System State is OFF so we turn CDU 1 OFF" << std::endl;
//                            // we are not on - shut off pump
//                            CduCommandData.commandID = htonl(TLV_SET_PUMPS_OFF);
//                            cduClient.send_data(&CduCommandData, sizeof(CduCommandData));
//                        }
//                    }

                    if( !systemON )
                    {
                        logger::verbose(__FILE__, __FUNCTION__, "System is not ON, setting CDU Data as STALE, updating ...");
                        // We need to set 'stale' data indicators '--'

                        // COMMS State
                        AuxCdu1Data[ENV_CDU_COMMS_OK].data = cduClient.clientConnected();
                        AuxCdu1Data[ENV_CDU_COMMS_OK].valid = true;


                        // ERROR Status
                        AuxCdu1Data[ENV_CDU_PUMP_LEAK].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_LEAK].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_PHASE_ERROR].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_PHASE_ERROR].valid = true;

                        AuxCdu1Data[ENV_CDU_SYSTEM_EMERGENCY_STOP].data = staleData;
                        AuxCdu1Data[ENV_CDU_SYSTEM_EMERGENCY_STOP].valid = true;

                        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_SHUTDOWN].data = staleData;
                        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_SHUTDOWN].valid = true;

                        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_2_SHUTDOWN].data = staleData;
                        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_2_SHUTDOWN].valid = true;

                        AuxCdu1Data[ENV_CDU_SYSTEM_REMOTE_ON_OFF].data = staleData;
                        AuxCdu1Data[ENV_CDU_SYSTEM_REMOTE_ON_OFF].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_1_EXCHANGE_OVERPRESSURE].data = staleData;
                        AuxCdu1Data[ENV_CDU_LOOP_1_EXCHANGE_OVERPRESSURE].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_1_FILTER_OVERPRESSURE].data = staleData;
                        AuxCdu1Data[ENV_CDU_LOOP_1_FILTER_OVERPRESSURE].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_2_EXCHANGE_OVERPRESSURE].data = staleData;
                        AuxCdu1Data[ENV_CDU_LOOP_2_EXCHANGE_OVERPRESSURE].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_2_FILTER_OVERPRESSURE].data = staleData;
                        AuxCdu1Data[ENV_CDU_LOOP_2_FILTER_OVERPRESSURE].valid = true;

                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_FULL].data = staleData;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_FULL].valid = true;

                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_LOW].data = staleData;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_LOW].valid = true;

                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_FULL].data = staleData;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_FULL].valid = true;

                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_LOW].data = staleData;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_LOW].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_1_ALARM].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_1_ALARM].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_2_ALARM].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_2_ALARM].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_3_ALARM].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_3_ALARM].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_4_ALARM].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_4_ALARM].valid = true;


                        // We move his later to another method
                        // Parse data into report
                        AuxCdu1Data[ENV_CDU_PUMP_1_FLOW].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_1_FLOW].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_1_TEMP].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_1_TEMP].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_2_FLOW].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_2_FLOW].valid = true;
                        AuxCdu1Data[ENV_CDU_PUMP_2_TEMP].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_2_TEMP].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_3_FLOW].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_3_FLOW].valid = true;
                        AuxCdu1Data[ENV_CDU_PUMP_3_TEMP].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_3_TEMP].valid = true;

                        AuxCdu1Data[ENV_CDU_PUMP_4_FLOW].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_4_FLOW].valid = true;
                        AuxCdu1Data[ENV_CDU_PUMP_4_TEMP].data = staleData;
                        AuxCdu1Data[ENV_CDU_PUMP_4_TEMP].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_1_PRESSURE].data = staleData;
                        AuxCdu1Data[ENV_CDU_LOOP_1_PRESSURE].valid = true;
            //            AuxCdu1Data[ENV_CDU_LOOP_1_TEMP].data = CduStatusData.LOOP_INFO[0].loopTemp;
                        AuxCdu1Data[ENV_CDU_LOOP_1_TEMP].data = std::string("N/A");
                        AuxCdu1Data[ENV_CDU_LOOP_1_TEMP].valid = true;

                        AuxCdu1Data[ENV_CDU_LOOP_2_PRESSURE].data = staleData;
                        AuxCdu1Data[ENV_CDU_LOOP_2_PRESSURE].valid = true;
            //            AuxCdu1Data[ENV_CDU_LOOP_2_TEMP].data = CduStatusData.LOOP_INFO[0].loopTemp;
                        AuxCdu1Data[ENV_CDU_LOOP_2_TEMP].data = std::string("N/A");
                        AuxCdu1Data[ENV_CDU_LOOP_2_TEMP].valid = true;

                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_TEMP].data = staleData;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_1_TEMP].valid = true;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_TEMP].data = staleData;
                        AuxCdu1Data[ENV_CDU_RESERVOIR_2_TEMP].valid = true;

                    }


                }
#endif
                break;

            default:
                break;

        }
    }
    else
    {
//        std::cout << "\n\n[" << __FUNCTION__ << "] Liquid Cool NOT ENABLED......" << std::endl;

    }

}


void envMgrController::getAuxCdu1Data(SYSTEM_REPORT_DATA_S & statusdata, bool freshData)
/// ***************************************************************************
/// Summary:
///
///     Tasks related to Air Cool functionality triggered by a 'ticker' method
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // vars
    SYSTEM_REPORT_DATA_S    CduStatusData;

    const std::string   staleData{"--"};


    /// TODO SSS : Add methods to validate range of values and format floats to single decimal

    if( freshData )
    {
        logger::verbose(__FILE__, __FUNCTION__, "CDU Data available, retrieving ...");
        // We need to collect data
        CduStatusData = statusdata;

        // COMMS State
        AuxCdu1Data[ENV_CDU_COMMS_OK].data                      = cduClient.clientConnected();

        // ALARM Status
        AuxCdu1Data[ENV_CDU_PUMP_LEAK].data                     = CduStatusData.ALARM_STATE.PUMP_LEAK;
        AuxCdu1Data[ENV_CDU_PUMP_PHASE_ERROR].data              = CduStatusData.ALARM_STATE.PHASE_ERROR;
        AuxCdu1Data[ENV_CDU_SYSTEM_EMERGENCY_STOP].data         = CduStatusData.ALARM_STATE.EMERGENCY_STOP;
        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_SHUTDOWN].data     = CduStatusData.ALARM_STATE.ACC1_SHUTDOWN;
        AuxCdu1Data[ENV_CDU_SYSTEM_ACCESSORY_2_SHUTDOWN].data   = CduStatusData.ALARM_STATE.ACC2_SHUTDOWN;
        AuxCdu1Data[ENV_CDU_SYSTEM_REMOTE_ON_OFF].data          = CduStatusData.ALARM_STATE.REMOTE_ON_OFF;
        AuxCdu1Data[ENV_CDU_LOOP_1_EXCHANGE_OVERPRESSURE].data  = CduStatusData.ALARM_STATE.LOOP1_EXCHNG_PRES;
        AuxCdu1Data[ENV_CDU_LOOP_1_FILTER_OVERPRESSURE].data    = CduStatusData.ALARM_STATE.LOOP1_FILTER_PRES;
        AuxCdu1Data[ENV_CDU_LOOP_2_EXCHANGE_OVERPRESSURE].data  = CduStatusData.ALARM_STATE.LOOP2_EXCHNG_PRES;
        AuxCdu1Data[ENV_CDU_LOOP_2_FILTER_OVERPRESSURE].data    = CduStatusData.ALARM_STATE.LOOP2_FILTER_PRES;
        AuxCdu1Data[ENV_CDU_RESERVOIR_1_FULL].data              = CduStatusData.ALARM_STATE.RESERVOIR1_FULL;
        AuxCdu1Data[ENV_CDU_RESERVOIR_1_LOW].data               = CduStatusData.ALARM_STATE.RESERVOIR1_LOW;
        AuxCdu1Data[ENV_CDU_RESERVOIR_2_FULL].data              = CduStatusData.ALARM_STATE.RESERVOIR2_FULL;
        AuxCdu1Data[ENV_CDU_RESERVOIR_2_LOW].data               = CduStatusData.ALARM_STATE.RESERVOIR2_LOW;
        AuxCdu1Data[ENV_CDU_PUMP_1_ALARM].data                  = CduStatusData.ALARM_STATE.PUMP1_ALARM;
        AuxCdu1Data[ENV_CDU_PUMP_2_ALARM].data                  = CduStatusData.ALARM_STATE.PUMP2_ALARM;
        AuxCdu1Data[ENV_CDU_PUMP_3_ALARM].data                  = CduStatusData.ALARM_STATE.PUMP3_ALARM;
        AuxCdu1Data[ENV_CDU_PUMP_4_ALARM].data                  = CduStatusData.ALARM_STATE.PUMP4_ALARM;

        // We move his later to another method
        // Parse data into report
        AuxCdu1Data[ENV_CDU_PUMP_1_FLOW].data = dataValidateAndFormat(FlowVal, CduStatusData.PUMP_INFO[0].pumpFlow, 1);
        AuxCdu1Data[ENV_CDU_PUMP_1_TEMP].data = dataValidateAndFormat(TempVal, CduStatusData.PUMP_INFO[0].pumpTemp, 1);

        AuxCdu1Data[ENV_CDU_PUMP_2_FLOW].data = dataValidateAndFormat(FlowVal, CduStatusData.PUMP_INFO[1].pumpFlow, 1);
        AuxCdu1Data[ENV_CDU_PUMP_2_TEMP].data = dataValidateAndFormat(TempVal, CduStatusData.PUMP_INFO[1].pumpTemp, 1);

        AuxCdu1Data[ENV_CDU_PUMP_3_FLOW].data = dataValidateAndFormat(FlowVal, CduStatusData.PUMP_INFO[2].pumpFlow, 1);
        AuxCdu1Data[ENV_CDU_PUMP_3_TEMP].data = dataValidateAndFormat(TempVal, CduStatusData.PUMP_INFO[2].pumpTemp, 1);

        AuxCdu1Data[ENV_CDU_PUMP_4_FLOW].data = dataValidateAndFormat(FlowVal, CduStatusData.PUMP_INFO[3].pumpFlow, 1);
        AuxCdu1Data[ENV_CDU_PUMP_4_TEMP].data = dataValidateAndFormat(TempVal, CduStatusData.PUMP_INFO[3].pumpTemp, 1);

        if( CduStatusData.LOOP_INFO_AVAIL[0])
        {
            auto loop1Press = CduStatusData.LOOP_INFO[0].loopPressure;
            AuxCdu1Data[ENV_CDU_LOOP_1_PRESSURE].data = dataValidateAndFormat(PressureVal, loop1Press, 1);
            logger::warn(__FILE__, __FUNCTION__, "CDU LOOP 1 Pressure Data INVALID!!!, setting to stale -- ...");

            AuxCdu1Data[ENV_CDU_LOOP_1_TEMP].data = std::string("N/A");
        }
        else
        {
            AuxCdu1Data[ENV_CDU_LOOP_1_PRESSURE].data = staleData;
            AuxCdu1Data[ENV_CDU_LOOP_1_TEMP].data = std::string("N/A");
        }

        if( CduStatusData.LOOP_INFO_AVAIL[1])
        {
            auto loop2Press = CduStatusData.LOOP_INFO[1].loopPressure;
            AuxCdu1Data[ENV_CDU_LOOP_2_PRESSURE].data = dataValidateAndFormat(PressureVal, loop2Press, 1);
            AuxCdu1Data[ENV_CDU_LOOP_2_TEMP].data = std::string("N/A");
        }
        else
        {
            AuxCdu1Data[ENV_CDU_LOOP_2_PRESSURE].data = staleData;
            AuxCdu1Data[ENV_CDU_LOOP_2_TEMP].data = std::string("N/A");
        }

        // validate reservoir temps first
        auto resv1Temp = CduStatusData.RESV_TEMP[0].reservoirTemp;
        AuxCdu1Data[ENV_CDU_RESERVOIR_1_TEMP].data = dataValidateAndFormat(TempVal, resv1Temp, 1);

        auto resv2Temp = CduStatusData.RESV_TEMP[1].reservoirTemp;
        AuxCdu1Data[ENV_CDU_RESERVOIR_2_TEMP].data = dataValidateAndFormat(TempVal, resv2Temp, 1);

        // enable all
        for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_COMMS_OK);
                parm < static_cast<long  unsigned int>(ENV_CDU_PRESENCE); parm++)
        {
            AuxCdu1Data[parm].valid = true;
        }

    }
    else
    {
        logger::verbose(__FILE__, __FUNCTION__, "System is not ON, setting CDU Data as STALE, updating ...");
        // We need to set 'stale' data indicators '--'

        // COMMS State
        AuxCdu1Data[ENV_CDU_COMMS_OK].data = cduClient.clientConnected();
        AuxCdu1Data[ENV_CDU_COMMS_OK].valid = true;


        for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_PUMP_LEAK);
                parm < static_cast<long  unsigned int>(ENV_CDU_PRESENCE); parm++)
        {
            AuxCdu1Data[parm].data  = staleData;
            AuxCdu1Data[parm].valid = true;
        }
    }

    updateIF.updateData(AuxCdu1Data);
}


#if 0 // envMgrController::getAuxCdu1ModBusData() (old ver)
void envMgrController::getAuxCdu1ModBusData()
/// ***************************************************************************
/// Summary:
///
///     Tasks related to Air Cool functionality triggered by a 'ticker' method
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // vars
    MODBUS_REPORT_DATA_S    CduModBusPumpData;

//    SYSTEM_REPORT_DATA_S    CduStatusData;


    bool                systemON{false};

    std::string         keySysState{"RF_ONLINE_STATE"};

    const std::string   parm1{"resultPayload"};
    const std::string   parm2{"value"};
    const std::string   staleData{"--"};


    using namespace helpers::types;


    // Exit if we have not completed initialization
    //
    if(!EnviroMgrInitComplete)
    {
//        std::cout << "\n\n[" << __FUNCTION__ << "] EnviroMgrInitComplete False, Exiting......" << std::endl;
        logger::warn(__FILE__, __FUNCTION__, "EnviroMgrInitComplete False, Exiting......");
        return;
    }

    std::tie (EnviroData[CoolType].data, EnviroData[CoolType].valid) = getCoolingType();
    auto currSysState = strToOnlineState(requestIF.getStatusParam<std::string>(keySysState, parm1, parm2).value_or("COLD"));
    systemON = currSysState != online_state_t::COLD;


    if(EnviroCfg.LiquidCoolEn && EnviroCfg.Cdu1Presence)
    {

        if(systemON)
        {
            // Let's check state of our system
            // We are either ONLINE or in HOT STANDBY -
            // Connect to CDU and get data
            if(cduClient.clientConnected())
            {
//                std::cout << "\n ===>>> *** Still connected, Checking CDU MODBUS Data ***" << std::endl;
                logger::verbose(__FILE__, __FUNCTION__, "cduClient Connected, Checking for CDU MODBUS Data ...");
                cduClient.receive();

            }
            else
            {
//                std::cout << "\n ===>>> *** Starting cduClient ***" << std::endl;
                logger::verbose(__FILE__, __FUNCTION__, "cduClient Disconnected, (re)starting cduClient ...");
                cduClient.conn();
                logger::verbose(__FILE__, __FUNCTION__, "Checking for CDU Data after (re)connect ...");
                if(cduClient.clientConnected())
                {
//                    std::cout << "\n ===>>> *** Checking CDU Data after reconnect ***" << std::endl;
                    logger::verbose(__FILE__, __FUNCTION__, "cduClient Connected, Checking for CDU Data  ...");
                    cduClient.receive();
                }
                else
                {
                    // we have no connected send stale data
                    for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE); parm < static_cast<long  unsigned int>(MaxAuxCduModBusDataIndex); parm++)
                    {
                        AuxCdu1ModBusData[parm].data = staleData;
                        AuxCdu1ModBusData[parm].valid = true;

                    }
                }
            }

            if( cduClient.getModBusStatusAvailable())
            {
                logger::verbose(__FILE__, __FUNCTION__, "CDU MODBUS Data available, retrieving ...");

                // clear No Data count
                ModBusNoDataAvailPollCnt = 0;

                // We need to collect data
                CduModBusPumpData = cduClient.getModBusPumpData();

//                if(0 == CduModBusPumpData.pumpID)

                switch(CduModBusPumpData.pumpID)
                {
                    case 0:
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE               ].data = CduModBusPumpData.OperMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CONTROL_MODE                 ].data = CduModBusPumpData.ctrlMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_NIGHT_MODE_ACTIVATION        ].data = CduModBusPumpData.nightMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_AIR_VENTING_PROCEDURE        ].data = CduModBusPumpData.airVentProcedure;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_SETPOINT      ].data = CduModBusPumpData.constPressureSetpoint;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_AIR_VENTING_AT_POWER_ON      ].data = CduModBusPumpData.airVentPowerOn;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.InputPower;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_HEAD                         ].data = CduModBusPumpData.Head;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_FLOW                         ].data = CduModBusPumpData.Flow;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_SPEED                        ].data = CduModBusPumpData.Speed;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.WaterTemp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.Winding1Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.Winding2Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.Winding3Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.PowerModuleTemp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.QuadrantCurrent;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.BitFieldAlarm1;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.BitFieldAlarm2;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.BitFieldErrors;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_MIN_SETPOINT  ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_MAX_SETPOINT  ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_COMMUNICATIONS_PROTOCOL      ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_COMMUNICATIONS_BAUD_RATE     ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_LIFETIME_TIMER_LSW           ].data = CduModBusPumpData.LifeTimerLSW;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_LIFETIME_TIMER_MSW           ].data = CduModBusPumpData.LifeTimerMSW;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_0_25_LSW   ].data = CduModBusPumpData.PowerConsumptionLSW_25;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_0_25_MSW   ].data = CduModBusPumpData.PowerConsumptionMSW_25;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_25_50_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_50;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_25_50_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_50;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_50_75_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_75;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_50_75_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_75;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_75_100_LSW ].data = CduModBusPumpData.PowerConsumptionLSW_100;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_75_100_MSW ].data = CduModBusPumpData.PowerConsumptionMSW_100;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CURRENT_INDEX_LOG            ].data = CduModBusPumpData.CurrentIDxLog;


                        for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE);
                                parm < static_cast<long  unsigned int>(ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE); parm++)
                        {
                            AuxCdu1ModBusData[parm].valid = true;
                        }

                    break;

                    case 1:
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE               ].data = CduModBusPumpData.OperMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONTROL_MODE                 ].data = CduModBusPumpData.ctrlMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_NIGHT_MODE_ACTIVATION        ].data = CduModBusPumpData.nightMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_AIR_VENTING_PROCEDURE        ].data = CduModBusPumpData.airVentProcedure;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_SETPOINT      ].data = CduModBusPumpData.constPressureSetpoint;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_AIR_VENTING_AT_POWER_ON      ].data = CduModBusPumpData.airVentPowerOn;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.InputPower;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_HEAD                         ].data = CduModBusPumpData.Head;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_FLOW                         ].data = CduModBusPumpData.Flow;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_SPEED                        ].data = CduModBusPumpData.Speed;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.WaterTemp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.Winding1Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.Winding2Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.Winding3Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.PowerModuleTemp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.QuadrantCurrent;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.BitFieldAlarm1;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.BitFieldAlarm2;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.BitFieldErrors;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_MIN_SETPOINT  ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_MAX_SETPOINT  ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_COMMUNICATIONS_PROTOCOL      ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_COMMUNICATIONS_BAUD_RATE     ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_LIFETIME_TIMER_LSW           ].data = CduModBusPumpData.LifeTimerLSW;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_LIFETIME_TIMER_MSW           ].data = CduModBusPumpData.LifeTimerMSW;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_0_25_LSW   ].data = CduModBusPumpData.PowerConsumptionLSW_25;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_0_25_MSW   ].data = CduModBusPumpData.PowerConsumptionMSW_25;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_25_50_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_50;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_25_50_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_50;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_50_75_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_75;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_50_75_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_75;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_75_100_LSW ].data = CduModBusPumpData.PowerConsumptionLSW_100;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_75_100_MSW ].data = CduModBusPumpData.PowerConsumptionMSW_100;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CURRENT_INDEX_LOG            ].data = CduModBusPumpData.CurrentIDxLog;


                        for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE);
                                parm < static_cast<long  unsigned int>(ENV_CDU_1_PUMP_3_MODBUS_OPERATING_MODE); parm++)
                        {
                            AuxCdu1ModBusData[parm].valid = true;
                        }


                    break;

                    case 2:
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_OPERATING_MODE               ].data = CduModBusPumpData.OperMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CONTROL_MODE                 ].data = CduModBusPumpData.ctrlMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_NIGHT_MODE_ACTIVATION        ].data = CduModBusPumpData.nightMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_AIR_VENTING_PROCEDURE        ].data = CduModBusPumpData.airVentProcedure;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_SETPOINT      ].data = CduModBusPumpData.constPressureSetpoint;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_AIR_VENTING_AT_POWER_ON      ].data = CduModBusPumpData.airVentPowerOn;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.InputPower;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_HEAD                         ].data = CduModBusPumpData.Head;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_FLOW                         ].data = CduModBusPumpData.Flow;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_SPEED                        ].data = CduModBusPumpData.Speed;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.WaterTemp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.Winding1Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.Winding2Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.Winding3Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.PowerModuleTemp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.QuadrantCurrent;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.BitFieldAlarm1;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.BitFieldAlarm2;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.BitFieldErrors;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_MIN_SETPOINT  ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_MAX_SETPOINT  ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_COMMUNICATIONS_PROTOCOL      ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_COMMUNICATIONS_BAUD_RATE     ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_LIFETIME_TIMER_LSW           ].data = CduModBusPumpData.LifeTimerLSW;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_LIFETIME_TIMER_MSW           ].data = CduModBusPumpData.LifeTimerMSW;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_0_25_LSW   ].data = CduModBusPumpData.PowerConsumptionLSW_25;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_0_25_MSW   ].data = CduModBusPumpData.PowerConsumptionMSW_25;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_25_50_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_50;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_25_50_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_50;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_50_75_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_75;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_50_75_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_75;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_75_100_LSW ].data = CduModBusPumpData.PowerConsumptionLSW_100;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_75_100_MSW ].data = CduModBusPumpData.PowerConsumptionMSW_100;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CURRENT_INDEX_LOG            ].data = CduModBusPumpData.CurrentIDxLog;


                        for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_3_MODBUS_OPERATING_MODE);
                                parm < static_cast<long  unsigned int>(ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE); parm++)
                        {
                            AuxCdu1ModBusData[parm].valid = true;
                        }

                    break;

                    case 3:
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE               ].data = CduModBusPumpData.OperMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONTROL_MODE                 ].data = CduModBusPumpData.ctrlMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_NIGHT_MODE_ACTIVATION        ].data = CduModBusPumpData.nightMode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_AIR_VENTING_PROCEDURE        ].data = CduModBusPumpData.airVentProcedure;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_SETPOINT      ].data = CduModBusPumpData.constPressureSetpoint;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_AIR_VENTING_AT_POWER_ON      ].data = CduModBusPumpData.airVentPowerOn;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.InputPower;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_HEAD                         ].data = CduModBusPumpData.Head;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_FLOW                         ].data = CduModBusPumpData.Flow;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_SPEED                        ].data = CduModBusPumpData.Speed;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.WaterTemp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.Winding1Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.Winding2Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.Winding3Temp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.PowerModuleTemp;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.QuadrantCurrent;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.BitFieldAlarm1;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.BitFieldAlarm2;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.BitFieldErrors;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_MIN_SETPOINT  ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_MAX_SETPOINT  ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_COMMUNICATIONS_PROTOCOL      ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_COMMUNICATIONS_BAUD_RATE     ].data = staleData;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_LIFETIME_TIMER_LSW           ].data = CduModBusPumpData.LifeTimerLSW;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_LIFETIME_TIMER_MSW           ].data = CduModBusPumpData.LifeTimerMSW;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_0_25_LSW   ].data = CduModBusPumpData.PowerConsumptionLSW_25;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_0_25_MSW   ].data = CduModBusPumpData.PowerConsumptionMSW_25;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_25_50_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_50;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_25_50_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_50;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_50_75_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_75;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_50_75_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_75;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_75_100_LSW ].data = CduModBusPumpData.PowerConsumptionLSW_100;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_75_100_MSW ].data = CduModBusPumpData.PowerConsumptionMSW_100;
                        AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CURRENT_INDEX_LOG            ].data = CduModBusPumpData.CurrentIDxLog;


                        for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE);
                                parm < static_cast<long  unsigned int>(MaxAuxCduModBusDataIndex); parm++)
                        {
                            AuxCdu1ModBusData[parm].valid = true;
                        }

                    break;

                    default:
                        // we have are connected, but no data bad index -- send stale data
                        for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE);
                                parm < static_cast<long  unsigned int>(MaxAuxCduModBusDataIndex); parm++)
                        {
                            AuxCdu1ModBusData[parm].data = staleData;
                            AuxCdu1ModBusData[parm].valid = true;

                        }
                        break;

                }   // end switch
            }   // end if
            else
            {
                ModBusNoDataAvailPollCnt++;

                if( ModBusNoDataAvailPollCnt > 5)
                {
                    // set data as stale
                    // we have are connected, but no data -- send stale data
                    for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE);
                            parm < static_cast<long  unsigned int>(MaxAuxCduModBusDataIndex); parm++)
                    {
                        AuxCdu1ModBusData[parm].data = staleData;
                        AuxCdu1ModBusData[parm].valid = true;

                    }

                }
            }
        }

        else // system is off
        {
            for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE);
                    parm < static_cast<long  unsigned int>(MaxAuxCduModBusDataIndex); parm++)
            {
                AuxCdu1ModBusData[parm].data = staleData;
                AuxCdu1ModBusData[parm].valid = true;

            }
        }

        // update after
        updateIF.updateData(AuxCdu1ModBusData);

    }   // end if
}
#endif

void envMgrController::getAuxCdu1ModBusData(MODBUS_REPORT_DATA_S & modbuspumpdata, bool freshData)
/// ***************************************************************************
/// Summary:
///
///     Tasks related to Air Cool functionality triggered by a 'ticker' method
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // vars
    MODBUS_REPORT_DATA_S    CduModBusPumpData;

    const std::string   staleData{"--"};



    if(freshData)
    {
        CduModBusPumpData = modbuspumpdata;

//        cout << "\n> MODBUS DATA - PUMP: " << CduModBusPumpData.pumpID << endl;
//        cout << "  OPERATING MODE: " << CIRCULAR_CONFIG_S[CduModBusPumpData.circularConfigMode] << endl;

        switch(CduModBusPumpData.pumpID)
        {
            case 1:
//                cout << "\n> MODBUS DATA - PUMP 1 " << endl;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE               ].data = static_cast<std::string>(((CduModBusPumpData.OperMode )? "ON": "OFF"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CONTROL_MODE                 ].data = PUMP_CTRL_MODE_S[CduModBusPumpData.ctrlMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CONFIG_MODE                  ].data = CIRCULAR_CONFIG_S[CduModBusPumpData.circularConfigMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_TWIN_CONTROL_MODE            ].data = TWIN_CRTL_CONFIG_S[CduModBusPumpData.twinCtrlMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_NIGHT_MODE_ACTIVATION        ].data = static_cast<std::string>(((CduModBusPumpData.nightMode )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_AIR_VENTING_PROCEDURE        ].data = static_cast<std::string>(((CduModBusPumpData.airVentProcedure )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_SETPOINT      ].data = static_cast<unsigned int>(CduModBusPumpData.constPressureSetpoint);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_AIR_VENTING_AT_POWER_ON      ].data = static_cast<std::string>(((CduModBusPumpData.airVentPowerOn )? "ACTIVE": "INACTIVE"));

                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.InputPower;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_HEAD                         ].data = static_cast<unsigned int>(CduModBusPumpData.Head);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_FLOW                         ].data = static_cast<unsigned int>(CduModBusPumpData.Flow);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_SPEED                        ].data = CduModBusPumpData.Speed;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.WaterTemp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.Winding1Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.Winding2Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.Winding3Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.PowerModuleTemp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.QuadrantCurrent;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.BitFieldAlarm1;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.BitFieldAlarm2;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.BitFieldErrors;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_MIN_SETPOINT  ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_MAX_SETPOINT  ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_COMMUNICATIONS_PROTOCOL      ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_COMMUNICATIONS_BAUD_RATE     ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_LIFETIME_TIMER_LSW           ].data = CduModBusPumpData.LifeTimerLSW;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_LIFETIME_TIMER_MSW           ].data = CduModBusPumpData.LifeTimerMSW;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_0_25_LSW   ].data = CduModBusPumpData.PowerConsumptionLSW_25;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_0_25_MSW   ].data = CduModBusPumpData.PowerConsumptionMSW_25;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_25_50_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_50;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_25_50_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_50;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_50_75_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_75;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_50_75_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_75;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_75_100_LSW ].data = CduModBusPumpData.PowerConsumptionLSW_100;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_75_100_MSW ].data = CduModBusPumpData.PowerConsumptionMSW_100;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_1_MODBUS_CURRENT_INDEX_LOG            ].data = CduModBusPumpData.CurrentIDxLog;

                for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE);
                        parm < static_cast<long  unsigned int>(ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE); parm++)
                {
                    AuxCdu1ModBusData[parm].valid = true;
                }
                if(CduModBusPumpData.circularConfigMode == E_PUMP_TWIN_MASTER)
                {
                    // We are in Master/Slave mode
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE               ].data = static_cast<std::string>(((CduModBusPumpData.ctrlMode )? "ON": "OFF"));
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONTROL_MODE                 ].data = PUMP_CTRL_MODE_S[CduModBusPumpData.ctrlMode];
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONFIG_MODE                  ].data = CIRCULAR_CONFIG_S[E_PUMP_TWIN_SLAVE];
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_TWIN_CONTROL_MODE            ].data = TWIN_CRTL_CONFIG_S[CduModBusPumpData.twinCtrlMode];

                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.TwinPumpData.TwinSlaveInputpower;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_HEAD                         ].data = static_cast<unsigned int>(CduModBusPumpData.Head);
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_FLOW                         ].data = static_cast<unsigned int>(CduModBusPumpData.Flow);
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_SPEED                        ].data = CduModBusPumpData.TwinPumpData.TwinSlaveSpeed;
//                  AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.TwinPumpData.WaterTemp;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.TwinPumpData.TwinSlaveWindingTemp1;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.TwinPumpData.TwinSlaveWindingTemp2;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.TwinPumpData.TwinSlaveWindingTemp3;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.TwinPumpData.TwinSlavePowerModuleTemp;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.TwinPumpData.TwinSlaveQuadratureCurrent;
//                  AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.TwinPumpData.TwinSlaveBitFieldAlarm1;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.TwinPumpData.TwinSlaveBitFieldAlarm2;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.TwinPumpData.TwinSlaveBitFieldErrors;
//                  AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;

                    for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE);
                        parm < static_cast<long  unsigned int>(ENV_CDU_1_PUMP_2_MODBUS_ACTIVE_ERROR_CODE); parm++)
                    {
                        AuxCdu1ModBusData[parm].valid = true;
                    }
                }

            break;

            case 2:
//                cout << "\n> MODBUS DATA - PUMP 2 " << endl;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE               ].data = static_cast<std::string>(((CduModBusPumpData.OperMode )? "ON": "OFF"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONTROL_MODE                 ].data = PUMP_CTRL_MODE_S[CduModBusPumpData.ctrlMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONFIG_MODE                  ].data = CIRCULAR_CONFIG_S[CduModBusPumpData.circularConfigMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_TWIN_CONTROL_MODE            ].data = TWIN_CRTL_CONFIG_S[CduModBusPumpData.twinCtrlMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_NIGHT_MODE_ACTIVATION        ].data = static_cast<std::string>(((CduModBusPumpData.nightMode )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_AIR_VENTING_PROCEDURE        ].data = static_cast<std::string>(((CduModBusPumpData.airVentProcedure )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_SETPOINT      ].data = static_cast<unsigned int>(CduModBusPumpData.constPressureSetpoint);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_AIR_VENTING_AT_POWER_ON      ].data = static_cast<std::string>(((CduModBusPumpData.airVentPowerOn )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.InputPower;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_HEAD                         ].data = static_cast<unsigned int>(CduModBusPumpData.Head);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_FLOW                         ].data = static_cast<unsigned int>(CduModBusPumpData.Flow);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_SPEED                        ].data = CduModBusPumpData.Speed;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.WaterTemp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.Winding1Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.Winding2Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.Winding3Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.PowerModuleTemp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.QuadrantCurrent;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.BitFieldAlarm1;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.BitFieldAlarm2;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.BitFieldErrors;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_MIN_SETPOINT  ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_MAX_SETPOINT  ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_COMMUNICATIONS_PROTOCOL      ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_COMMUNICATIONS_BAUD_RATE     ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_LIFETIME_TIMER_LSW           ].data = CduModBusPumpData.LifeTimerLSW;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_LIFETIME_TIMER_MSW           ].data = CduModBusPumpData.LifeTimerMSW;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_0_25_LSW   ].data = CduModBusPumpData.PowerConsumptionLSW_25;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_0_25_MSW   ].data = CduModBusPumpData.PowerConsumptionMSW_25;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_25_50_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_50;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_25_50_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_50;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_50_75_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_75;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_50_75_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_75;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_75_100_LSW ].data = CduModBusPumpData.PowerConsumptionLSW_100;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_75_100_MSW ].data = CduModBusPumpData.PowerConsumptionMSW_100;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_2_MODBUS_CURRENT_INDEX_LOG            ].data = CduModBusPumpData.CurrentIDxLog;


                for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE);
                        parm < static_cast<long  unsigned int>(ENV_CDU_1_PUMP_3_MODBUS_OPERATING_MODE); parm++)
                {
                    AuxCdu1ModBusData[parm].valid = true;
                }


            break;

            case 3:
//                cout << "\n> MODBUS DATA - PUMP 3 " << endl;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_OPERATING_MODE               ].data = static_cast<std::string>(((CduModBusPumpData.OperMode )? "ON": "OFF"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_OPERATING_MODE               ].valid = true;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CONTROL_MODE                 ].data = PUMP_CTRL_MODE_S[CduModBusPumpData.ctrlMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CONFIG_MODE                  ].data = CIRCULAR_CONFIG_S[CduModBusPumpData.circularConfigMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_TWIN_CONTROL_MODE            ].data = TWIN_CRTL_CONFIG_S[CduModBusPumpData.twinCtrlMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_NIGHT_MODE_ACTIVATION        ].data = static_cast<std::string>(((CduModBusPumpData.nightMode )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_AIR_VENTING_PROCEDURE        ].data = static_cast<std::string>(((CduModBusPumpData.airVentProcedure )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_SETPOINT      ].data = static_cast<unsigned int>(CduModBusPumpData.constPressureSetpoint);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_AIR_VENTING_AT_POWER_ON      ].data = static_cast<std::string>(((CduModBusPumpData.airVentPowerOn )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.InputPower;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_HEAD                         ].data = static_cast<unsigned int>(CduModBusPumpData.Head);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_FLOW                         ].data = static_cast<unsigned int>(CduModBusPumpData.Flow);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_SPEED                        ].data = CduModBusPumpData.Speed;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.WaterTemp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.Winding1Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.Winding2Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.Winding3Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.PowerModuleTemp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.QuadrantCurrent;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.BitFieldAlarm1;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.BitFieldAlarm2;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.BitFieldErrors;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_MIN_SETPOINT  ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_MAX_SETPOINT  ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_COMMUNICATIONS_PROTOCOL      ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_COMMUNICATIONS_BAUD_RATE     ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_LIFETIME_TIMER_LSW           ].data = CduModBusPumpData.LifeTimerLSW;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_LIFETIME_TIMER_MSW           ].data = CduModBusPumpData.LifeTimerMSW;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_0_25_LSW   ].data = CduModBusPumpData.PowerConsumptionLSW_25;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_0_25_MSW   ].data = CduModBusPumpData.PowerConsumptionMSW_25;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_25_50_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_50;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_25_50_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_50;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_50_75_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_75;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_50_75_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_75;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_75_100_LSW ].data = CduModBusPumpData.PowerConsumptionLSW_100;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_75_100_MSW ].data = CduModBusPumpData.PowerConsumptionMSW_100;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_3_MODBUS_CURRENT_INDEX_LOG            ].data = CduModBusPumpData.CurrentIDxLog;


                for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_3_MODBUS_OPERATING_MODE);
                        parm < static_cast<long  unsigned int>(ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE); parm++)
                {
                    AuxCdu1ModBusData[parm].valid = true;
                }
                if(CduModBusPumpData.circularConfigMode == E_PUMP_TWIN_MASTER)
                {
                    // We are in Master/Slave mode
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE               ].data = static_cast<std::string>(((CduModBusPumpData.ctrlMode )? "ON": "OFF"));
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONTROL_MODE                 ].data = PUMP_CTRL_MODE_S[CduModBusPumpData.ctrlMode];
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONFIG_MODE                  ].data = CIRCULAR_CONFIG_S[E_PUMP_TWIN_SLAVE];
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_TWIN_CONTROL_MODE            ].data = TWIN_CRTL_CONFIG_S[CduModBusPumpData.twinCtrlMode];

                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.TwinPumpData.TwinSlaveInputpower;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_HEAD                         ].data = static_cast<unsigned int>(CduModBusPumpData.Head);
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_FLOW                         ].data = static_cast<unsigned int>(CduModBusPumpData.Flow);
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_SPEED                        ].data = CduModBusPumpData.TwinPumpData.TwinSlaveSpeed;
//                  AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.TwinPumpData.WaterTemp;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.TwinPumpData.TwinSlaveWindingTemp1;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.TwinPumpData.TwinSlaveWindingTemp2;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.TwinPumpData.TwinSlaveWindingTemp3;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.TwinPumpData.TwinSlavePowerModuleTemp;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.TwinPumpData.TwinSlaveQuadratureCurrent;
//                  AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.TwinPumpData.TwinSlaveBitFieldAlarm1;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.TwinPumpData.TwinSlaveBitFieldAlarm2;
                    AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.TwinPumpData.TwinSlaveBitFieldErrors;
//                  AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;

                    for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE);
                        parm < static_cast<long  unsigned int>(ENV_CDU_1_PUMP_4_MODBUS_ACTIVE_ERROR_CODE); parm++)
                    {
                        AuxCdu1ModBusData[parm].valid = true;
                    }
                }

            break;

            case 4:
//                cout << "\n> MODBUS DATA - PUMP 4 " << endl;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE               ].data = static_cast<std::string>(((CduModBusPumpData.OperMode )? "ON": "OFF"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONTROL_MODE                 ].data = PUMP_CTRL_MODE_S[CduModBusPumpData.ctrlMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONFIG_MODE                  ].data = CIRCULAR_CONFIG_S[CduModBusPumpData.circularConfigMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_TWIN_CONTROL_MODE            ].data = TWIN_CRTL_CONFIG_S[CduModBusPumpData.twinCtrlMode];
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_NIGHT_MODE_ACTIVATION        ].data = static_cast<std::string>(((CduModBusPumpData.nightMode )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_AIR_VENTING_PROCEDURE        ].data = static_cast<std::string>(((CduModBusPumpData.airVentProcedure )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_SETPOINT      ].data = static_cast<unsigned int>(CduModBusPumpData.constPressureSetpoint);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_AIR_VENTING_AT_POWER_ON      ].data = static_cast<std::string>(((CduModBusPumpData.airVentPowerOn )? "ACTIVE": "INACTIVE"));
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_INPUT_POWER                  ].data = CduModBusPumpData.InputPower;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_HEAD                         ].data = static_cast<unsigned int>(CduModBusPumpData.Head);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_FLOW                         ].data = static_cast<unsigned int>(CduModBusPumpData.Flow);
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_SPEED                        ].data = CduModBusPumpData.Speed;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WATER_TEMPERATURE            ].data = CduModBusPumpData.WaterTemp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WINDING_1_TEMPERATURE        ].data = CduModBusPumpData.Winding1Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WINDING_2_TEMPERATURE        ].data = CduModBusPumpData.Winding2Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_WINDING_3_TEMPERATURE        ].data = CduModBusPumpData.Winding3Temp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_MODULE_TEMPERATURE     ].data = CduModBusPumpData.PowerModuleTemp;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_QUADRANT_CURRENT             ].data = CduModBusPumpData.QuadrantCurrent;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_STATUS_IO         ].data = CduModBusPumpData.BitFieldStatusIO;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_1           ].data = CduModBusPumpData.BitFieldAlarm1;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_2           ].data = CduModBusPumpData.BitFieldAlarm2;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ERROR             ].data = CduModBusPumpData.BitFieldErrors;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_ACTIVE_ERROR_CODE            ].data = CduModBusPumpData.ActiveErrorCode;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_MIN_SETPOINT  ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_MAX_SETPOINT  ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_COMMUNICATIONS_PROTOCOL      ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_COMMUNICATIONS_BAUD_RATE     ].data = staleData;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_LIFETIME_TIMER_LSW           ].data = CduModBusPumpData.LifeTimerLSW;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_LIFETIME_TIMER_MSW           ].data = CduModBusPumpData.LifeTimerMSW;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_0_25_LSW   ].data = CduModBusPumpData.PowerConsumptionLSW_25;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_0_25_MSW   ].data = CduModBusPumpData.PowerConsumptionMSW_25;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_25_50_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_50;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_25_50_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_50;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_50_75_LSW  ].data = CduModBusPumpData.PowerConsumptionLSW_75;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_50_75_MSW  ].data = CduModBusPumpData.PowerConsumptionMSW_75;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_75_100_LSW ].data = CduModBusPumpData.PowerConsumptionLSW_100;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_75_100_MSW ].data = CduModBusPumpData.PowerConsumptionMSW_100;
                AuxCdu1ModBusData[ ENV_CDU_1_PUMP_4_MODBUS_CURRENT_INDEX_LOG            ].data = CduModBusPumpData.CurrentIDxLog;


                for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE);
                        parm < static_cast<long  unsigned int>(MaxAuxCduModBusDataIndex); parm++)
                {
                    AuxCdu1ModBusData[parm].valid = true;
                }

            break;

            default:
                // we have are connected, but no data bad index -- send stale data
                for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE);
                        parm < static_cast<long  unsigned int>(MaxAuxCduModBusDataIndex); parm++)
                {
                    AuxCdu1ModBusData[parm].data = staleData;
                    AuxCdu1ModBusData[parm].valid = true;

                }
                break;

        }   // end switch

        //        }   // end if
    }

    else // system is off
    {
        for(auto parm =  static_cast<long  unsigned int>(ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE);
                parm < static_cast<long  unsigned int>(MaxAuxCduModBusDataIndex); parm++)
        {
            AuxCdu1ModBusData[parm].data = staleData;
            AuxCdu1ModBusData[parm].valid = true;

        }
    }

    // update after
    updateIF.updateData(AuxCdu1ModBusData);
}



void envMgrController::coolingLiquidCoolTickerTasks()
/// ***************************************************************************
/// Summary:
///
///     Tasks related to Liquid Cooling functionality triggered by a 'ticker' method
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    using namespace helpers::types;

    // vars
    SYSTEM_REPORT_DATA_S    CduStatusData;
    MODBUS_REPORT_DATA_S    CduModBusPumpData;

    bool                    systemON{false};
    bool                    cduColdStandByShutdown{false};

    std::string             keySysState{"RF_ONLINE_STATE"};

    const std::string       parm1{"resultPayload"};
    const std::string       parm2{"value"};
    const std::string       staleData{"--"};


    // Exit if we have not completed initialization
    //
    if(!EnviroMgrInitComplete)
    {
//        std::cout << "\n\n[" << __FUNCTION__ << "] EnviroMgrInitComplete False, Exiting......" << std::endl;
        logger::warn(__FILE__, __FUNCTION__, "EnviroMgrInitComplete False, Exiting......");
        return;
    }

    std::tie (EnviroData[CoolType].data, EnviroData[CoolType].valid) = getCoolingType();
    auto currSysState = strToOnlineState(requestIF.getStatusParam<std::string>(keySysState, parm1, parm2).value_or("COLD"));
    systemON = currSysState != online_state_t::COLD;

    cduColdStandByShutdown = EnviroCfg.Cdu1ColdStandbyShutdown;


    if( EnviroCfg.LiquidCoolEn && (SOS_CONTROLLER == SystemTypeInfo.mode) )
    {
        if(EnviroCfg.Cdu1Presence)
        {
            // We are either ONLINE or in HOT STANDBY -
            // Connect to CDU and get data
            if(cduClient.clientConnected())
            {
                logger::verbose(__FILE__, __FUNCTION__, "cduClient Connected, Checking for CDU Data  ...");
                cduClient.receive();

            }
            else
            {
                logger::verbose(__FILE__, __FUNCTION__, "cduClient Disconnected, attempting (RE)connect to CDU ...");
                cduClient.conn();
                logger::verbose(__FILE__, __FUNCTION__, "Checking for CDU connected after (RE)connect attempt ...");
                if(cduClient.clientConnected())
                {
                    logger::verbose(__FILE__, __FUNCTION__, "cduClient Re-Connected, Checking for CDU Data  ...");
                    cduClient.receive();
                }
                else
                {
                    if(cduClient.getConnectErrorCnt() >= 10)
                    {
                        cduClient.renewSock();
                    }
                    return;
                }
            }



            /// TODO : Enable Pump ON/OFF command but validate with flag to prevent resends
            /// TODO : Add System Config for ENV_MGR_COLD_STANDBY_CDU_OFF
            ///
#if 0
            /// TODO -SSS: TEST OUTPUT - Config Output Dump on StartUp --- remove when done
            ///
            msgHdr = "\n\n[ " + std::string(__FUNCTION__) + "(): line " + std::to_string(__LINE__) + " ]: ";
            msgData = "\n-> cduClient.clientConnected()      : "   + std::to_string(cduClient.clientConnected()               ) + \
                      "\n-> cduClient.getSystemPumpONstate() : "   + std::to_string(cduClient.getSystemPumpONstate()          ) + \
                      "\n-> systemON                         : "   + std::to_string(systemON                                  ) + \
                      "\n-> cduColdStandByShutdown           : "   + std::to_string(cduColdStandByShutdown                    );
            std::cout  << msgHdr << msgData << std::endl;

            logger::verbose(__FILE__, __FUNCTION__, msgHdr);
            logger::verbose(__FILE__, __FUNCTION__, msgData);
#endif
            /// FIX : Force value for testing
            PumpCfg.SplitShutdownCntl = true;

            if(cduClient.clientConnected() && systemON)
            {
                if (PumpCfg.SplitShutdownCntl) {
                    if ((9000 == EnviroCfg.Cdu1NetworkPort) && !cduClient.getLoop1PumpONstate()) {
                        // Loop 1 Control in split config
                        logger::verbose(__FILE__, __FUNCTION__, "CDU LOOP1 PUMP ON sent... -- ...");
                        cout << __FUNCTION__ << "(): Sending Loop1 ON command to CDU" << endl;
                        cduClient.sendLoop1On();
                    }
                    if ((9001 == EnviroCfg.Cdu1NetworkPort) && !cduClient.getLoop2PumpONstate()) {
                        // Loop 2 Control in split config
                        logger::verbose(__FILE__, __FUNCTION__, "CDU LOOP2 PUMP ON sent... -- ...");
                        cout << __FUNCTION__ << "(): Sending Loop2 ON command to CDU" << endl;
                        cduClient.sendLoop2On();
                    }
                }
                if (!cduClient.getSystemPumpONstate() && !PumpCfg.SplitShutdownCntl)
                {
                    // We to set pump on (this is global version)
                    logger::verbose(__FILE__, __FUNCTION__, "CDU PUMP ON sent... -- ...");
                    cout << __FUNCTION__ << "(): Sending Pump ON command to CDU" << endl;
                    cduClient.sendPumpOn();
                }

                AuxCdu1Data[ENV_CDU_1_SYSTEM_COLD_STANDBY_SHUTDOWN].data = false;
                AuxCdu1Data[ENV_CDU_1_SYSTEM_COLD_STANDBY_SHUTDOWN].valid = true;
            }

            if(cduClient.clientConnected() && !systemON && cduColdStandByShutdown )
            {
                if (PumpCfg.SplitShutdownCntl) {
                    if ((9000 == EnviroCfg.Cdu1NetworkPort) && cduClient.getLoop1PumpONstate()) {
                        // Loop 1 Control in split config
                        logger::verbose(__FILE__, __FUNCTION__, "CDU LOOP1 PUMP OFF sent... -- ...");
                        cout << __FUNCTION__ << "(): Sending Loop1 OFF command to CDU" << endl;
                        cduClient.sendLoop1Off();
                    }
                    if ((9001 == EnviroCfg.Cdu1NetworkPort) && cduClient.getLoop2PumpONstate()) {
                        // Loop 2 Control in split config
                        logger::verbose(__FILE__, __FUNCTION__, "CDU LOOP2 PUMP OFF sent... -- ...");
                        cout << __FUNCTION__ << "(): Sending Loop2 OFF command to CDU" << endl;
                        cduClient.sendLoop2Off();
                    }
                }
                if (!cduClient.getSystemPumpONstate() && !PumpCfg.SplitShutdownCntl)
                {
                    // We to set pump on (this is global version)
                    logger::verbose(__FILE__, __FUNCTION__, "CDU PUMP ON sent... -- ...");
                    cout << __FUNCTION__ << "(): Sending Pump ON command to CDU" << endl;
                    cduClient.sendPumpOff();
                }

                AuxCdu1Data[ENV_CDU_1_SYSTEM_COLD_STANDBY_SHUTDOWN].data = true;
                AuxCdu1Data[ENV_CDU_1_SYSTEM_COLD_STANDBY_SHUTDOWN].valid = true;
            }

            // Check for What data is Available
            if( cduClient.getStatusAvailable())
            {
                StatusNoDataAvailPollCnt = 0;
                CduStatusData = cduClient.getStatusData();
                getAuxCdu1Data(CduStatusData, true);
            }
            else
                StatusNoDataAvailPollCnt++;

            if( cduClient.getModBusStatusAvailable())
            {
                ModBusNoDataAvailPollCnt = 0;
                CduModBusPumpData = cduClient.getModBusPumpData();
                getAuxCdu1ModBusData(CduModBusPumpData, true);
            }
            else
                ModBusNoDataAvailPollCnt++;




            if( (StatusNoDataAvailPollCnt > 50) || (ModBusNoDataAvailPollCnt > 50) )
            {
                // set data as stale
                if(StatusNoDataAvailPollCnt > 50)
                {
                    getAuxCdu1Data(CduStatusData, false);
                    logger::verbose(__FILE__, __FUNCTION__, "No fresh CDU Status Data, setting data to STALE -- ...");
                }

                if(ModBusNoDataAvailPollCnt > 50)
                 {
                    getAuxCdu1ModBusData(CduModBusPumpData, false);
                    logger::verbose(__FILE__, __FUNCTION__, "No fresh CDU MODBUS Data, setting data to STALE -- ...");
                 }

                if( (StatusNoDataAvailPollCnt > 100) || (ModBusNoDataAvailPollCnt > 100) )
                {
                    // we're connected but no data for over n loops
                    cduClient.renewSock();
                    StatusNoDataAvailPollCnt = 0;
                    ModBusNoDataAvailPollCnt = 0;
                }

            }
        }
    }
    else
    {
        logger::info(__FILE__, __FUNCTION__, "Liquid Cool NOT ENABLED ...");
    }
}


void envMgrController::Ticker100ms_Tasks()
/// ***************************************************************************
/// Summary:
///
///     Time triggered method binded to the systems 100 msec ticker
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
//    std::cout << "\n\n[" << __FUNCTION__ << "] Ticker 100 ms Function Calls......" << std::endl;

    if(0 == cduClient.getConnectErrorCnt() )
    {
        logger::verbose(__FILE__, __FUNCTION__, "cduClient connected, using 100 msec Ticker  ...");
        coolingLiquidCoolTickerTasks();
        pushAuxCduStatus();
    }

}

void envMgrController::Ticker500ms_Tasks()
/// ***************************************************************************
/// Summary:
///
///     Time triggered method binded to the systems 500 msec ticker
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
//    std::cout << "\n\n[" << __FUNCTION__ << "] Ticker 500 ms Function Calls......" << std::endl;
//    coolingAirCoolTickerTasks();

}

void envMgrController::Ticker1000ms_Tasks()
/// ***************************************************************************
/// Summary:
///
///     Time triggered method binded to the systems 1000 msec ticker
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
//    std::cout << "\n\n[" << __FUNCTION__ << "] Ticker 1000 ms Function Calls......" << std::endl;

    coolingAirCoolTickerTasks();

    if(cduClient.getConnectErrorCnt() >= 1)
    {
        logger::verbose(__FILE__, __FUNCTION__, "cduClient connect errors, using 1000 msec Ticker  ...");
        coolingLiquidCoolTickerTasks();
        pushAuxCduStatus();

    }


}

void envMgrController::Ticker5000ms_Tasks()
/// ***************************************************************************
/// Summary:
///
///     Time triggered method binded to the systems 5000 msec ticker
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
//    std::cout << "\n\n[" << __FUNCTION__ << "] Ticker 5000 ms Function Calls......" << std::endl;


}

void envMgrController::Ticker30000ms_Tasks()
/// ***************************************************************************
/// Summary:
///
///     Time triggered method binded to the systems 30000 msec ticker
///
/// Inputs:
///
///   < None >
///
/// Returns:
///
///   < None >
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
//    std::cout << "\n\n[" << __FUNCTION__ << "] Ticker 30000 ms Function Calls......" << std::endl;

}

/**
 * HWR (FPGA) ACCESS ROUTINES
 *
 *
 */

bool envMgrController::getFanEnableState(bool& enable)
/// ***************************************************************************
/// Summary:
///
///     Returns state of the Fan Enable Register
///
/// Inputs:
///
///     enable = (ref) to hold data to be returned
///
/// Returns:
///
///     boolean output of function true is success, false otherwise
///     enable = modified to state of Fan Enable, true = ON, false = OFF
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    if(auto resp{fpgaIF.read(EnviroCfg.FanCtrlModName, fanRegisterMapOffset::enable)}; resp.has_value())
    {
        enable = (0 != resp.value());
        return true;
    }

    logger::warn(__FILE__, __FUNCTION__, std::string("Failed to get Fan Enable State to ") +
        std::string(enable ? "Fan ON" : "Fan OFF"));

    return false;
}

bool envMgrController::setFanEnableState(const bool enable)
/// ***************************************************************************
/// Summary:
///
///     Sets FPGA Fan Enable register bit to enable Fan Power (Fan Vdd)
///
/// Inputs:
///
///     enable - boolean input of Fan enable , true = ON, false = OFF
///
/// Returns:
///
///     boolean output of function true is success, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    // The mask value here should be set as a static constexpr in the header,
    // and used here instead of the raw value
    // Using FPGA Reg Write method
    return fpgaIF.write(EnviroCfg.FanCtrlModName, fanRegisterMapOffset::enable, ((enable)?(1):(0)));
}


bool envMgrController::loadPumpConfig()
/// ***************************************************************************
/// Summary:
///     Takes parameters from message or file
///
/// Inputs:
///     None
///
/// Returns:
///     boolean output of function true is success, false otherwise
///
/// Assumptions:
///
///   < None >
/// ***************************************************************************
///
{
    return cduClient.loadPumpConfigFile();
}

#if 0
bool envMgrController::loadPumpConfigFile()
{
    static const char* const PUMP_CONFIG_PATH = "/empower/config/Pump.Config";

    printf("Loading Configuration from File %s\n", PUMP_CONFIG_PATH);

    if (!cduClient.clientConnected()) {
        printf("Cannot load configuration: client not connected\n");
        return false;
    }

    if (!cduClient.getCfgData(PUMP_CONFIG_PATH)) {
        printf("!!!!!!!!!!!!!!!!! Loading configuration failed !!!!\n");
        return false;
    }

    cduClient.sendCfgData(empower::LCC_PUMP_CLIENT::ConfigData);
    return true;
}

bool envMgrController::loadPumpConfigFile()
{
    bool success{false};
    auto fileName = "/empower/config/Pump.Config";

//    CONFIGURATION_DATA_S cfgData;
    printf("Loading Configuration from File %s  \n", fileName);
    if(cduClient.clientConnected())
    {
        if ( cduClient.getCfgData(fileName)  )
        {
            cduClient.sendCfgData(empower::LCC_PUMP_CLIENT::ConfigData);
            success = true;
        }
        else
        {
            printf("!!!!!!!!!!!!!!!!! Loading configuration failed !!!!\n");
            success = false;
        }
    }
    return success;
}
#endif

///==========================================================================
///
/// ------------------- UTILITY METHODS - (May move later)
///
///==========================================================================


bool envMgrController::dataRangeCheck(SysParamVal_t type, float & value)
{
    bool        result = false;

    switch(type)
    {
        case TempVal:
            if( (value >= _CDU_TEMP_MIN_DEG_C) && (value <= _CDU_TEMP_MAX_DEG_C))
                result = true;
            break;

        case PressureVal:
            if( (value >= _CDU_PRESSURE_MIN_PSI) && (value <= _CDU_PRESSURE_MAX_PSI))
                result = true;
            break;

        case FlowVal:
            if( (value >= _CDU_FLOW_MIN_GPM) && (value <= _CDU_FLOW_MAX_GPM))
                result = true;
            break;

        default:
            result = false;
            break;

    }

    return result;
}


std::string envMgrController::formatFloatToString(float & value, int precision)
{
    std::stringstream        ss;

    ss << std::fixed << std::setprecision(precision) << value;

    return ss.str();
}

std::string envMgrController::dataValidateAndFormat(SysParamVal_t type, float & value, int precision)
{
    std::string      result;

    if(dataRangeCheck(type, value))
        result = formatFloatToString(value, precision);
    else
        result.assign("--");

    return result;
}

