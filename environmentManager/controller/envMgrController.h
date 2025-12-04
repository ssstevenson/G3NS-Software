/*!
*
*
*
-------------------------------------------------------------------------------

 Name        : envMgrController.h

 Author      : Steve Stevenson

 Version     : v 00.90.00 [ Initial Dev Build ]

 Description : Environmental Manager Controller CORE Class Declarations (header)

-------------------------------------------------------------------------------

Copyright   : (C) Copyright 2018 - 2020 Empower RF Systems

-------------------------------------------------------------------------------
*
*
*/

#ifndef ENVIRONMENT_MANAGER_CONTROLLER_H_
#define ENVIRONMENT_MANAGER_CONTROLLER_H_

#include <array>
#include <configManagerConnection/configManagerConnection.h>
#include <fpgaInterfaceConnection/fpgaInterfaceConnection.h>
#include <functional>
#include <helpers/types.h>

#include <helpers/rfLayoutTypes.h>
// #include <map>

#include <statusRequestInterface/statusRequestInterface.h>
#include <statusUpdateInterface/statusUpdateInterface.h>
#include <string>
#include <string_view>
#include <fstream>
#include <streambuf>
#include <variant>
#include <zmq.hpp>

#include "../auxiliary/envMgrLCDUclient.h"

/**
 * HARDCODE LIMIT PARMS
 *
 * We define these here temporarily till
 * we add parms to config table..
 *
 * BUT we will still have a defined default
 * if config value doesn't exist
 *
 **/
#define MAX_SYS_FAN_COUNT                   8
#define _DEFAULT_TACH_WIN_REG_VALUE         50000000
#define _MSEC_PER_MINUTE                    60000
#define _DEFAULT_PULSES_PER_REV             2

#define _DEFAULT_CTRLR_RACK                 0
#define _DEFAULT_CTRLR_SLOT                 0

#define _DEFAULT_CURR_MAX_TEMP              -99
#define _INVALID_CURR_MAX_TEMP              -66
#define _FAILSAFE_ERROR_MAX_TEMP            333

#define _CDU_TEMP_MIN_DEG_C                 -50
#define _CDU_TEMP_MAX_DEG_C                 200
#define _CDU_PRESSURE_MIN_PSI               0
#define _CDU_PRESSURE_MAX_PSI               250
#define _CDU_FLOW_MIN_GPM                   0
#define _CDU_FLOW_MAX_GPM                   200




namespace empower
{
    class envMgrController
    {
    public:
        using mmData_t = helpers::types::mmData_t;
        using reg_t = helpers::types::reg_t;
        using temp_t = helpers::types::temp_t;
        using timeout_t = helpers::types::timeout_t;
        using pwm_t = helpers::types::pwm_t;
        using updateIfData_t = helpers::types::updateIfData_t;

        // Module Related types
        using rf_bands_t = helpers::types::rf_bands_t;
        using pallet_addrs_t = helpers::types::pallet_addrs_t;
        using pmod_device_keys_t = helpers::types::pmod_device_keys_t;
        using pmod_addr_t = helpers::types::pmod_addr_t;

        // system related types
        using online_state_t = helpers::types::online_state_t;


        struct sysInvntryRegisterMap
        {
            // READ-ONLY Registers
            //
            static constexpr reg_t sysfreq = 0x0000000C;
        };

        struct registerMap
        {
            static constexpr reg_t module_base  = 0x80080000;
            static constexpr reg_t enable  = 0x80080000;
            static constexpr reg_t tachwin = 0x80080004;
            static constexpr reg_t pwmhigh = 0x80080008;
            static constexpr reg_t pwmlow  = 0x8008000C;

            // READ-ONLY Registers
            //
            static constexpr reg_t tach1reg = 0x80080010;
            static constexpr reg_t tach2reg = 0x80080014;
            static constexpr reg_t tach3reg = 0x80080018;
            static constexpr reg_t tach4reg = 0x8008001C;
            static constexpr reg_t tach5reg = 0x80080020;
            static constexpr reg_t tach6reg = 0x80080024;
            static constexpr reg_t tach7reg = 0x80080028;
            static constexpr reg_t tach8reg = 0x8008002C;
            static constexpr reg_t fanfreq = 0x80080030;
        };

        struct fanRegisterMapOffset
        {
            static constexpr reg_t module_base  = 0x80080000;
            static constexpr reg_t enable  = 0x00000000;
            static constexpr reg_t tachwin = 0x00000004;
            static constexpr reg_t pwmhigh = 0x00000008;
            static constexpr reg_t pwmlow  = 0x0000000C;

            // READ-ONLY Registers
            //
            static constexpr reg_t tach1reg = 0x00000010;
            static constexpr reg_t tach2reg = 0x00000014;
            static constexpr reg_t tach3reg = 0x00000018;
            static constexpr reg_t tach4reg = 0x0000001C;
            static constexpr reg_t tach5reg = 0x00000020;
            static constexpr reg_t tach6reg = 0x00000024;
            static constexpr reg_t tach7reg = 0x00000028;
            static constexpr reg_t tach8reg = 0x0000002C;
            static constexpr reg_t fanfreq = 0x00000030;
        };


        //----------------------------------------------------------------------
        // System Data Parametr Type Conversion
        //
        typedef enum SystemParamTypes
        {
            TempVal = 0,
            PressureVal,
            FlowVal
        } SysParamVal_t;



    private:
        //----------------------------------------------------------------------
        // INSTANCES OF SUPPORT CLASSES AND VARS
        //
        configManagerConnection&    configIF;
        statusUpdateInterface       updateIF;
        statusRequestInterface      requestIF;
        fpgaInterfaceConnection     fpgaIF;
        rapidjson::Document         workingDoc;

        LCC_PUMP_CLIENT             cduClient;



        //----------------------------------------------------------------------
        // System Mode and Type Storage
        //
        enum SystemModeTypes
        {
            STANDALONE = 0,
            SOS_BOOSTER,
            SOS_CONTROLLER
        };


        struct SystemTypeInfoStruct
        {
            int             rack;
            int             slot;
            SystemModeTypes mode;

        } SystemTypeInfo;


        //----------------------------------------------------------------------
        // Env Configuration Support Elements
        //
        enum EnvCfgParmIndex
        {
            FHI_MOD_INVENTORY = 0,
            FHI_MOD_FAN_CTRL,
            ENV_AIR_COOL_ENABLE,
            ENV_AIR_COOL_MANUAL_ENABLE,
            ENV_AIR_COOL_MANUAL_SETPOINT,
            ENV_AIR_COOL_MAXIMUM_THRESHOLD,
            ENV_AIR_COOL_MINIMUM_THRESHOLD,
            ENV_AIR_COOL_OPERATIONAL_BANDS,
            ENV_AIR_COOL_MINIMUM_OPERATIONAL_BAND,
            ENV_AIR_COOL_FAN_PWM_FREQUENCY,
            ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_UP,
            ENV_AIR_COOL_BAND_TRAVERSAL_HYSTERESIS_DOWN,
            ENV_AIR_COOL_SAMPLE_PERIOD,
            ENV_AIR_COOL_TACHOMETER_SAMPLE_PERIOD,
            ENV_AIR_COOL_FAN_1_PRESENCE,
            ENV_AIR_COOL_FAN_1_PULSES_PER_REV,
            ENV_AIR_COOL_FAN_2_PRESENCE,
            ENV_AIR_COOL_FAN_2_PULSES_PER_REV,
            ENV_AIR_COOL_FAN_3_PRESENCE,
            ENV_AIR_COOL_FAN_3_PULSES_PER_REV,
            ENV_AIR_COOL_FAN_4_PRESENCE,
            ENV_AIR_COOL_FAN_4_PULSES_PER_REV,
            ENV_AIR_COOL_FAN_5_PRESENCE,
            ENV_AIR_COOL_FAN_5_PULSES_PER_REV,
            ENV_AIR_COOL_FAN_6_PRESENCE,
            ENV_AIR_COOL_FAN_6_PULSES_PER_REV,
            ENV_AIR_COOL_FAN_7_PRESENCE,
            ENV_AIR_COOL_FAN_7_PULSES_PER_REV,
            ENV_AIR_COOL_FAN_8_PRESENCE,
            ENV_AIR_COOL_FAN_8_PULSES_PER_REV,

            // LIQUID COOL PARMS
            ENV_LIQUID_COOL_ENABLE,
//            ENV_LIQUID_COOL_MANUAL_ENABLE,
//            ENV_LIQUID_COOL_MANUAL_SETPOINT,
            ENV_LIQUID_COOL_MAX_TEMPERATURE_THRESHOLD,
            ENV_LIQUID_COOL_MIN_TEMPERATURE_THRESHOLD,
            ENV_LIQUID_COOL_TEMPERATURE_SAMPLE_PERIOD,
            ENV_LIQUID_COOL_MAX_PRESSURE_THRESHOLD,
            ENV_LIQUID_COOL_MIN_PRESSURE_THRESHOLD,
            ENV_LIQUID_COOL_PRESSURE_SAMPLE_PERIOD,
            ENV_LIQUID_COOL_MAX_FLOW_RATE_THRESHOLD,
            ENV_LIQUID_COOL_MIN_FLOW_RATE_THRESHOLD,
            ENV_LIQUID_COOL_FLOW_RATE_SAMPLE_PERIOD,

            ENV_CDU_1_PRESENCE,
            ENV_CDU_1_NETWORK_IP_ADDRESS,
            ENV_CDU_1_NETWORK_IP_PORT,
            ENV_CDU_1_COLD_STANDBY_SHUTDOWN,

            ENV_CDU_2_PRESENCE,
            ENV_CDU_2_NETWORK_IP_ADDRESS,
            ENV_CDU_2_NETWORK_IP_PORT,
            ENV_CDU_2_COLD_STANDBY_SHUTDOWN,

            ENV_CDU_3_PRESENCE,
            ENV_CDU_3_NETWORK_IP_ADDRESS,
            ENV_CDU_3_NETWORK_IP_PORT,
            ENV_CDU_3_COLD_STANDBY_SHUTDOWN,

            ENV_CDU_4_PRESENCE,
            ENV_CDU_4_NETWORK_IP_ADDRESS,
            ENV_CDU_4_NETWORK_IP_PORT,
            ENV_CDU_4_COLD_STANDBY_SHUTDOWN,

            // LIQUID COOL SENSORS
            SENSOR_REMOTE_RACK1_ANALOG_01,
            SENSOR_REMOTE_RACK1_ANALOG_02,
            SENSOR_REMOTE_RACK1_ANALOG_03,
            SENSOR_REMOTE_RACK1_ANALOG_04,
            SENSOR_REMOTE_RACK1_ANALOG_05,
            SENSOR_REMOTE_RACK1_ANALOG_06,
            SENSOR_REMOTE_RACK1_ANALOG_07,
            SENSOR_REMOTE_RACK1_ANALOG_08,

            SENSOR_REMOTE_RACK2_ANALOG_01,
            SENSOR_REMOTE_RACK2_ANALOG_02,
            SENSOR_REMOTE_RACK2_ANALOG_03,
            SENSOR_REMOTE_RACK2_ANALOG_04,
            SENSOR_REMOTE_RACK2_ANALOG_05,
            SENSOR_REMOTE_RACK2_ANALOG_06,
            SENSOR_REMOTE_RACK2_ANALOG_07,
            SENSOR_REMOTE_RACK2_ANALOG_08

        };

        struct EnvConfigStruct {
            // Air Cooled Parms
            bool        AirCoolEn;
            bool        AirCoolManModeEn;
            std::array<bool, MAX_SYS_FAN_COUNT> AirCoolFanPresent;

            mmData_t    AirCoolManSetpoint;
            temp_t      AirCoolMaxThreshold;
            temp_t      AirCoolMinThreshold;
            mmData_t    AirCoolOperBands;
            mmData_t    AirCoolMinOperBand;

            timeout_t   AirCoolHysteresisUp;
            timeout_t   AirCoolHysteresisDown;
            timeout_t   AirCoolTempSamplePeriod;
            mmData_t    AirCoolPwmFreq;
            timeout_t   AirCoolTachSamplePeriod;
            std::array<mmData_t, MAX_SYS_FAN_COUNT> AirCoolPulsesPerRev;

            std::string FanCtrlModName;
            std::string InvntryModName;

            // Liquid Cooled Parms
            bool        LiquidCoolEn;
            bool        LiquidCoolManModeEn;

            mmData_t    LiquidCoolManSetpoint;
            mmData_t    LiquidCoolMaxPressureThreshold;
            mmData_t    LiquidCoolMinPressureThreshold;
            mmData_t    LiquidCoolMaxFlowRateThreshold;
            mmData_t    LiquidCoolMinFlowRateThreshold;

            temp_t      LiquidCoolMaxTempThreshold;
            temp_t      LiquidCoolMinTempThreshold;

            timeout_t   LiquidCoolTempSamplePeriod;
            timeout_t   LiquidCoolPressureSamplePeriod;
            timeout_t   LiquidCoolFlowRateSamplePeriod;

            bool        Cdu1Presence;
            std::string Cdu1NetworkIP;
            mmData_t    Cdu1NetworkPort;
            bool        Cdu1ColdStandbyShutdown;

            bool        Cdu2Presence;
            std::string Cdu2NetworkIP;
            mmData_t    Cdu2NetworkPort;
            bool        Cdu2ColdStandbyShutdown;

            bool        Cdu3Presence;
            std::string Cdu3NetworkIP;
            mmData_t    Cdu3NetworkPort;
            bool        Cdu3ColdStandbyShutdown;

            bool        Cdu4Presence;
            std::string Cdu4NetworkIP;
            mmData_t    Cdu4NetworkPort;
            bool        Cdu4ColdStandbyShutdown;

        } EnviroCfg;


        struct EnvThermalBandStruct {
            temp_t      MaxBandThresh;
            temp_t      MinBandthresh;
            mmData_t    BandSetpoint;
        };

        struct CduPumpConfigStruct {
            bool    SplitShutdownCntl;
        } PumpCfg;

        std::vector<EnvThermalBandStruct> ThermalBand;

        std::size_t  CurrThermalBand;

        std::unordered_map<std::string, EnvCfgParmIndex> mapCfgParm;
//        std::map<std::string, std::string(*)(std::string)>::iterator itr;

        //----------------------------------------------------------------------
        // ENV STATUS AND DATA SUPPORT ELEMENTS
        //
        enum EnvDataIndex
        {
            CoolType = 0,
            CoolMode,
            CurrentMaxTemp,
            CurrentThermalBand,
            CurrentSetpoint,
            FanTachSamplePeriod,
            FailsafeActive,

            // Fan data
            Fan1RPM,
            Fan2RPM,
            Fan3RPM,
            Fan4RPM,
            Fan5RPM,
            Fan6RPM,
            Fan7RPM,
            Fan8RPM,
            MaxEnvDataIndex
        };

        const timeout_t defaultTimeout;
        std::array<updateIfData_t, MaxEnvDataIndex> EnviroData;



        //----------------------------------------------------------------------
        // AUXILIARY CDU STATUS AND DATA SUPPORT ELEMENTS
        //
        enum AuxCduDataIndex
        {
            // CDU Error status
            ENV_CDU_COMMS_OK = 0,
            ENV_CDU_PUMP_LEAK,
            ENV_CDU_PUMP_PHASE_ERROR,
            ENV_CDU_SYSTEM_EMERGENCY_STOP,
            ENV_CDU_SYSTEM_ACCESSORY_SHUTDOWN,
            ENV_CDU_SYSTEM_ACCESSORY_2_SHUTDOWN,
            ENV_CDU_SYSTEM_REMOTE_ON_OFF,
            ENV_CDU_1_SYSTEM_COLD_STANDBY_SHUTDOWN,

            ENV_CDU_LOOP_1_EXCHANGE_OVERPRESSURE,
            ENV_CDU_LOOP_1_FILTER_OVERPRESSURE,
            ENV_CDU_LOOP_2_EXCHANGE_OVERPRESSURE,
            ENV_CDU_LOOP_2_FILTER_OVERPRESSURE,
            ENV_CDU_RESERVOIR_1_FULL,
            ENV_CDU_RESERVOIR_1_LOW,
            ENV_CDU_RESERVOIR_2_FULL,
            ENV_CDU_RESERVOIR_2_LOW,
            ENV_CDU_PUMP_1_ALARM,
            ENV_CDU_PUMP_2_ALARM,
            ENV_CDU_PUMP_3_ALARM,
            ENV_CDU_PUMP_4_ALARM,


            // CDU System Device Data
            ENV_CDU_PUMP_1_FLOW,
            ENV_CDU_PUMP_1_TEMP,
            ENV_CDU_PUMP_2_FLOW,
            ENV_CDU_PUMP_2_TEMP,
            ENV_CDU_PUMP_3_FLOW,
            ENV_CDU_PUMP_3_TEMP,
            ENV_CDU_PUMP_4_FLOW,
            ENV_CDU_PUMP_4_TEMP,
            ENV_CDU_LOOP_1_PRESSURE,
            ENV_CDU_LOOP_1_TEMP,
            ENV_CDU_LOOP_2_PRESSURE,
            ENV_CDU_LOOP_2_TEMP,
            ENV_CDU_RESERVOIR_1_TEMP,
            ENV_CDU_RESERVOIR_2_TEMP,
            ENV_CDU_PRESENCE,
            ENV_CDU_NETWORK_IP_ADDRESS,
            ENV_CDU_NETWORK_IP_PORT,
            MaxAuxCduDataIndex
        };

        const timeout_t defaultAuxTimeout;
        std::array<updateIfData_t, MaxAuxCduDataIndex> AuxCdu1Data;


        //----------------------------------------------------------------------
        // AUXILIARY CDU MODBUS STATUS AND DATA SUPPORT ELEMENTS
        //
        enum AuxCduModBusDataIndex
        {
            // CDU Error status
            ENV_CDU_1_PUMP_1_MODBUS_OPERATING_MODE,
            ENV_CDU_1_PUMP_1_MODBUS_CONTROL_MODE,
            ENV_CDU_1_PUMP_1_MODBUS_CONFIG_MODE,
            ENV_CDU_1_PUMP_1_MODBUS_TWIN_CONTROL_MODE,
            ENV_CDU_1_PUMP_1_MODBUS_NIGHT_MODE_ACTIVATION,
            ENV_CDU_1_PUMP_1_MODBUS_AIR_VENTING_PROCEDURE,
            ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_SETPOINT,
            ENV_CDU_1_PUMP_1_MODBUS_AIR_VENTING_AT_POWER_ON,

            ENV_CDU_1_PUMP_1_MODBUS_INPUT_POWER,
            ENV_CDU_1_PUMP_1_MODBUS_HEAD,
            ENV_CDU_1_PUMP_1_MODBUS_FLOW,
            ENV_CDU_1_PUMP_1_MODBUS_SPEED,
            ENV_CDU_1_PUMP_1_MODBUS_WATER_TEMPERATURE,
            ENV_CDU_1_PUMP_1_MODBUS_WINDING_1_TEMPERATURE,
            ENV_CDU_1_PUMP_1_MODBUS_WINDING_2_TEMPERATURE,
            ENV_CDU_1_PUMP_1_MODBUS_WINDING_3_TEMPERATURE,
            ENV_CDU_1_PUMP_1_MODBUS_POWER_MODULE_TEMPERATURE,
            ENV_CDU_1_PUMP_1_MODBUS_QUADRANT_CURRENT,

            ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_STATUS_IO,
            ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ALARM_1,
            ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ALARM_2,
            ENV_CDU_1_PUMP_1_MODBUS_BIT_FIELDS_ERROR,
            ENV_CDU_1_PUMP_1_MODBUS_ACTIVE_ERROR_CODE,

            ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_MIN_SETPOINT,
            ENV_CDU_1_PUMP_1_MODBUS_CONSTANT_CURVE_MAX_SETPOINT,
            ENV_CDU_1_PUMP_1_MODBUS_COMMUNICATIONS_PROTOCOL,
            ENV_CDU_1_PUMP_1_MODBUS_COMMUNICATIONS_BAUD_RATE,

            // 32 bit values in 16 bit chucks split into 16 bit words
            // LSW - Least Sig Word,  MSW - Most Sig Word
            //
//            ENV_CDU_1_PUMP_1_MODBUS_LIFETIME_TIMER_MSW_AND_LSW,
//            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_0_25_MSW_AND_LSW,
//            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_25_50_MSW_AND_LSW,
//            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_50_75_MSW_AND_LSW,
//            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_75_100_MSW_AND_LSW,
//            ENV_CDU_1_PUMP_1_MODBUS_CURRENT_INDEX_LOG,

            ENV_CDU_1_PUMP_1_MODBUS_LIFETIME_TIMER_LSW,
            ENV_CDU_1_PUMP_1_MODBUS_LIFETIME_TIMER_MSW,

            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_0_25_LSW,
            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_0_25_MSW,

            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_25_50_LSW,
            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_25_50_MSW,

            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_50_75_LSW,
            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_50_75_MSW,

            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_75_100_LSW,
            ENV_CDU_1_PUMP_1_MODBUS_POWER_CONSUMPTION_75_100_MSW,

            ENV_CDU_1_PUMP_1_MODBUS_CURRENT_INDEX_LOG,

            // PUMP 2
            ENV_CDU_1_PUMP_2_MODBUS_OPERATING_MODE,
            ENV_CDU_1_PUMP_2_MODBUS_CONTROL_MODE,
            ENV_CDU_1_PUMP_2_MODBUS_CONFIG_MODE,
            ENV_CDU_1_PUMP_2_MODBUS_TWIN_CONTROL_MODE,
            ENV_CDU_1_PUMP_2_MODBUS_NIGHT_MODE_ACTIVATION,
            ENV_CDU_1_PUMP_2_MODBUS_AIR_VENTING_PROCEDURE,
            ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_SETPOINT,
            ENV_CDU_1_PUMP_2_MODBUS_AIR_VENTING_AT_POWER_ON,
            ENV_CDU_1_PUMP_2_MODBUS_INPUT_POWER,
            ENV_CDU_1_PUMP_2_MODBUS_HEAD,
            ENV_CDU_1_PUMP_2_MODBUS_FLOW,
            ENV_CDU_1_PUMP_2_MODBUS_SPEED,
            ENV_CDU_1_PUMP_2_MODBUS_WATER_TEMPERATURE,
            ENV_CDU_1_PUMP_2_MODBUS_WINDING_1_TEMPERATURE,
            ENV_CDU_1_PUMP_2_MODBUS_WINDING_2_TEMPERATURE,
            ENV_CDU_1_PUMP_2_MODBUS_WINDING_3_TEMPERATURE,
            ENV_CDU_1_PUMP_2_MODBUS_POWER_MODULE_TEMPERATURE,
            ENV_CDU_1_PUMP_2_MODBUS_QUADRANT_CURRENT,
            ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_STATUS_IO,
            ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_1,
            ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ALARM_2,
            ENV_CDU_1_PUMP_2_MODBUS_BIT_FIELDS_ERROR,
            ENV_CDU_1_PUMP_2_MODBUS_ACTIVE_ERROR_CODE,
            ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_MIN_SETPOINT,
            ENV_CDU_1_PUMP_2_MODBUS_CONSTANT_CURVE_MAX_SETPOINT,
            ENV_CDU_1_PUMP_2_MODBUS_COMMUNICATIONS_PROTOCOL,
            ENV_CDU_1_PUMP_2_MODBUS_COMMUNICATIONS_BAUD_RATE,
            ENV_CDU_1_PUMP_2_MODBUS_LIFETIME_TIMER_LSW,
            ENV_CDU_1_PUMP_2_MODBUS_LIFETIME_TIMER_MSW,
            ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_0_25_LSW,
            ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_0_25_MSW,
            ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_25_50_LSW,
            ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_25_50_MSW,
            ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_50_75_LSW,
            ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_50_75_MSW,
            ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_75_100_LSW,
            ENV_CDU_1_PUMP_2_MODBUS_POWER_CONSUMPTION_75_100_MSW,
            ENV_CDU_1_PUMP_2_MODBUS_CURRENT_INDEX_LOG,


            // PUMP 3
            ENV_CDU_1_PUMP_3_MODBUS_OPERATING_MODE,
            ENV_CDU_1_PUMP_3_MODBUS_CONTROL_MODE,
            ENV_CDU_1_PUMP_3_MODBUS_CONFIG_MODE,
            ENV_CDU_1_PUMP_3_MODBUS_TWIN_CONTROL_MODE,
            ENV_CDU_1_PUMP_3_MODBUS_NIGHT_MODE_ACTIVATION,
            ENV_CDU_1_PUMP_3_MODBUS_AIR_VENTING_PROCEDURE,
            ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_SETPOINT,
            ENV_CDU_1_PUMP_3_MODBUS_AIR_VENTING_AT_POWER_ON,
            ENV_CDU_1_PUMP_3_MODBUS_INPUT_POWER,
            ENV_CDU_1_PUMP_3_MODBUS_HEAD,
            ENV_CDU_1_PUMP_3_MODBUS_FLOW,
            ENV_CDU_1_PUMP_3_MODBUS_SPEED,
            ENV_CDU_1_PUMP_3_MODBUS_WATER_TEMPERATURE,
            ENV_CDU_1_PUMP_3_MODBUS_WINDING_1_TEMPERATURE,
            ENV_CDU_1_PUMP_3_MODBUS_WINDING_2_TEMPERATURE,
            ENV_CDU_1_PUMP_3_MODBUS_WINDING_3_TEMPERATURE,
            ENV_CDU_1_PUMP_3_MODBUS_POWER_MODULE_TEMPERATURE,
            ENV_CDU_1_PUMP_3_MODBUS_QUADRANT_CURRENT,
            ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_STATUS_IO,
            ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ALARM_1,
            ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ALARM_2,
            ENV_CDU_1_PUMP_3_MODBUS_BIT_FIELDS_ERROR,
            ENV_CDU_1_PUMP_3_MODBUS_ACTIVE_ERROR_CODE,
            ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_MIN_SETPOINT,
            ENV_CDU_1_PUMP_3_MODBUS_CONSTANT_CURVE_MAX_SETPOINT,
            ENV_CDU_1_PUMP_3_MODBUS_COMMUNICATIONS_PROTOCOL,
            ENV_CDU_1_PUMP_3_MODBUS_COMMUNICATIONS_BAUD_RATE,
            ENV_CDU_1_PUMP_3_MODBUS_LIFETIME_TIMER_LSW,
            ENV_CDU_1_PUMP_3_MODBUS_LIFETIME_TIMER_MSW,
            ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_0_25_LSW,
            ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_0_25_MSW,
            ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_25_50_LSW,
            ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_25_50_MSW,
            ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_50_75_LSW,
            ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_50_75_MSW,
            ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_75_100_LSW,
            ENV_CDU_1_PUMP_3_MODBUS_POWER_CONSUMPTION_75_100_MSW,
            ENV_CDU_1_PUMP_3_MODBUS_CURRENT_INDEX_LOG,


            // PUMP 4
            ENV_CDU_1_PUMP_4_MODBUS_OPERATING_MODE,
            ENV_CDU_1_PUMP_4_MODBUS_CONTROL_MODE,
            ENV_CDU_1_PUMP_4_MODBUS_CONFIG_MODE,
            ENV_CDU_1_PUMP_4_MODBUS_TWIN_CONTROL_MODE,
            ENV_CDU_1_PUMP_4_MODBUS_NIGHT_MODE_ACTIVATION,
            ENV_CDU_1_PUMP_4_MODBUS_AIR_VENTING_PROCEDURE,
            ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_SETPOINT,
            ENV_CDU_1_PUMP_4_MODBUS_AIR_VENTING_AT_POWER_ON,
            ENV_CDU_1_PUMP_4_MODBUS_INPUT_POWER,
            ENV_CDU_1_PUMP_4_MODBUS_HEAD,
            ENV_CDU_1_PUMP_4_MODBUS_FLOW,
            ENV_CDU_1_PUMP_4_MODBUS_SPEED,
            ENV_CDU_1_PUMP_4_MODBUS_WATER_TEMPERATURE,
            ENV_CDU_1_PUMP_4_MODBUS_WINDING_1_TEMPERATURE,
            ENV_CDU_1_PUMP_4_MODBUS_WINDING_2_TEMPERATURE,
            ENV_CDU_1_PUMP_4_MODBUS_WINDING_3_TEMPERATURE,
            ENV_CDU_1_PUMP_4_MODBUS_POWER_MODULE_TEMPERATURE,
            ENV_CDU_1_PUMP_4_MODBUS_QUADRANT_CURRENT,
            ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_STATUS_IO,
            ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_1,
            ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ALARM_2,
            ENV_CDU_1_PUMP_4_MODBUS_BIT_FIELDS_ERROR,
            ENV_CDU_1_PUMP_4_MODBUS_ACTIVE_ERROR_CODE,
            ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_MIN_SETPOINT,
            ENV_CDU_1_PUMP_4_MODBUS_CONSTANT_CURVE_MAX_SETPOINT,
            ENV_CDU_1_PUMP_4_MODBUS_COMMUNICATIONS_PROTOCOL,
            ENV_CDU_1_PUMP_4_MODBUS_COMMUNICATIONS_BAUD_RATE,
            ENV_CDU_1_PUMP_4_MODBUS_LIFETIME_TIMER_LSW,
            ENV_CDU_1_PUMP_4_MODBUS_LIFETIME_TIMER_MSW,
            ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_0_25_LSW,
            ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_0_25_MSW,
            ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_25_50_LSW,
            ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_25_50_MSW,
            ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_50_75_LSW,
            ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_50_75_MSW,
            ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_75_100_LSW,
            ENV_CDU_1_PUMP_4_MODBUS_POWER_CONSUMPTION_75_100_MSW,
            ENV_CDU_1_PUMP_4_MODBUS_CURRENT_INDEX_LOG,

            MaxAuxCduModBusDataIndex

        };

        const timeout_t defaultAuxModBusTimeout;
        std::array<updateIfData_t, MaxAuxCduModBusDataIndex> AuxCdu1ModBusData;


        //----------------------------------------------------------------------
        // EXTERNAL DEVICE INFO SUPPORT
        //

        enum ExternalDeviceDataIndex
        {
//            MBRD = 0,
            PA_1 = 1,
            PA_2,
            PA_3,
            PA_4,
            PA_5,
            PA_6,
            PA_7,
            PA_8,
            DRIVER_1,
            DRIVER_2,
            DRIVER_3,
            MaxExtDevDataIndex
        };


        struct ExternalDeviceDataStruct {
            std::string     DevicePresenceAccessKey;
            bool            DevicePresent;
            std::string     DeviceTemperatureAccessKey;
            temp_t          DeviceTemp;
        };

        std::array<ExternalDeviceDataStruct, MaxExtDevDataIndex> ExtDeviceData;
        typedef std::pair<uint16_t, std::string> pairUInt16String;
//        const std::vector<pairUInt16String> deviceKeys;
        const std::vector<pairUInt16String> deviceKeysPMods;
//        const std::vector<pairUInt16String> deviceKeysAnalogSensors;


//        empower::moduleManagerController::devices     pmods;
//        empower::moduleManagerController::cmds        pmCmds;
//
//        using hw_map_t = std::map<moduleManagerController::devices, std::pair<bool, moduleManagerController::pmod_data_t> >;
//        hw_map_t hwConfig;
//        hw_map_t::iterator pmodDev;
//
//        std::map<moduleManagerController::devices, std::string> deviceKeys;
//
//        static constexpr std::size_t maxModuleDevices = static_cast<std::size_t>(moduleManagerController::devices::MAX);
//        static constexpr std::size_t firstModuleDevice = static_cast<std::size_t>(moduleManagerController::devices::PA_1);

        rf_bands_t bands;
        pallet_addrs_t allPmodAddrs;
        pallet_addrs_t::iterator curPmod;


//        static constexpr std::size_t maxModuleDevices;
//        static constexpr std::size_t firstModuleDevice;
//
//        std::array<ExternalDeviceDataStruct, maxModuleDevices> ModuleData;
//        std::vector<ExternalDeviceDataStruct> ModuleDevicesVect;


        //----------------------------------------------------------------------
        // DEBUG / MESSAGE VARS
        std::string msgHdr;
        std::string msgData;


        //----------------------------------------------------------------------
        // INITIALIZATION ROUTINES
        //
        bool EnviroMgrInitComplete;
        void initEnviroConfig();
        bool loadRackSlotModeInfo();
        bool loadEnviroConfig();
        void dumpEnviroConfig();

        bool initThermalBandData();

        bool initModuleDevicesVect();
        bool loadModuleDevicesVect();

        bool initAirCoolAlgo();


        // SYSTEM WORKER METHODS
        //

        //----------------------------------------------------------------------
        // Cooling Methods and Vars
        //

        // Returns string data for query
        std::tuple<std::string, bool> getCoolingMode();

        std::tuple<std::string, bool> getCoolingType();

        std::tuple<std::string, bool> getCoolingState();

        // checks gathered data for max temp / device id
        bool getMaxSystemTemp(temp_t& maxTemp);

        // Calculate  FPGA values for cooling level
        bool computeCoolPoints(mmData_t setpoint, pwm_t& hival, pwm_t& loval);

        // method to adjust cooling level
        bool setFanPWM(mmData_t setpoint);

        // method to adjust cooling level
        bool regulateCoolLevel();

        // check system cooling state - adjust if failure
        bool CoolingFailsafeActive;
        bool getFailsafeActive() { return CoolingFailsafeActive; }
        void setFailsafeActive(bool active) { CoolingFailsafeActive = active; }
        void coolingFailsafe(void);

        // method to collect, convert and store Fan tach data
        bool getFanTachs();

        // test for Hysteresis and Sample Polling Timeouts
        void setSamplePeriodStartPoint();
        void getSamplePeriodStartPoint(std::chrono::steady_clock::time_point& timePoint);

        bool isSamplePeriodTimeOutExpired();
        bool isTransitionUpTimeoutExpired();
        bool isTransitionDownTimeoutExpired();

        std::chrono::steady_clock::time_point tmrHysteresisStartPoint;
        std::chrono::steady_clock::time_point tmrSamplePeriodStartPoint;
        void setHysteresisStartPoint(void);
        void getHysteresisStartPoint(std::chrono::steady_clock::time_point& timePoint);

        // flags for setting status of Cooling loop
        bool CoolSetpointComplete;
        void setCoolSetpointComplete(bool complete);
        bool getCoolSetpointComplete();

        //----------------------------------------------------------------------
        // Process Status Methods and Vars
        //

        // Pulls requested data form supplied key list
        bool getStatusData();

        // Update Status Proc Report Data and Pushes to Status Proc
        bool pushEnviroStatus();
//        bool pushEnviroStatus(const EnvDataStruct& envData);

        bool pushAuxCduStatus();

        static std::string updateParm();

        unsigned int StatusNoDataAvailPollCnt;
        unsigned int ModBusNoDataAvailPollCnt;



    public:
        envMgrController(zmq::context_t& zmqCtx, configManagerConnection& config);

        bool processConfigChangeUpdate(std::string parmName);

        // MANUAL [FACTORY] Getters / Setters - FPGA Access
        bool getFanEnableState(bool& enable);
        bool setFanEnableState(const bool enable);
        bool loadPumpConfig();

        int  getFanDutyCycle(void);
        void setFanDutyCycle(mmData_t duty_cycle);

        // Status Data Requests - Status Proc access
        void getAuxCdu1Data();
        void getAuxCdu1Data(SYSTEM_REPORT_DATA_S & statusdata, bool freshData);

        void getAuxCdu1ModBusData();
        void getAuxCdu1ModBusData(MODBUS_REPORT_DATA_S & modbuspumpdata, bool freshData);


        // Ticker Tasks
        // - current list of Tick times available
        //
        void coolingAirCoolTickerTasks();
        void coolingLiquidCoolTickerTasks();

        void Ticker100ms_Tasks();
        void Ticker500ms_Tasks();
        void Ticker1000ms_Tasks();
        void Ticker5000ms_Tasks();
        void Ticker30000ms_Tasks();

        /// SSS : Utility Methods (may move to own class
        ///
        bool dataRangeCheck(SysParamVal_t type, float & value);
        std::string formatFloatToString(float & value, int precision);
        std::string dataValidateAndFormat(SysParamVal_t type, float & value, int precision);


    };
}

#endif
