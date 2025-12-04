#ifndef LEGACY_PROTOCOL_H_
#define LEGACY_PROTOCOL_H_

#include <controller/protocolInterface.h>

namespace empower
{
    /**
     * @brief      This class describes a legacy prototype command set for Pallet's and Driver's.
     */
    class legacyProto:
        public protocolInterface
    {
    public:
        using pmod_addr_t = protocolInterface::pmod_addr_t;
        using data_set_t = protocolInterface::data_set_t;
        using ser_msg_t = protocolInterface::ser_msg_t;
        using pmod_data_t = protocolInterface::pmod_data_t;
        using timeout_t = protocolInterface::timeout_t;

        enum class cmds: pmod_data_t
        {
            ping = 0x00,
            status = 0x02,
            reset = 0x03,
            setPwrOnCond = 0x04,
            disable = 0x05,
            enable = 0x06,
            getTemp = 0x07,
            getAlarms = 0x08,
            clearAlarms = 0x09,
            getCurrent = 0x0A,
            getInputVoltage = 0x0B,
            emergencyOverride = 0x0E,
            statusVariable = 0x30
        };

        enum class status: pmod_data_t
        {
            completed_ok = 0,
            status_tx_buffer_empty, status_rx_data_available, status_svc_msg_available, status_svc_msg_for_decode,
            status_svc_msg_readytosend, status_data_acquired, status_alarm_state_change, status_sys_state_change,
            status_enable_module, status_disable_module,
            error_status_start = 0x10,
            error_invalid_baud, error_msg_incomplete, error_chksum, error_tx_overrun, error_rx_overrun,
            error_rx_timeout, error_port_closed, error_warning_detected, error_alarm_detected, error_fault_detected,
            error_module_shutdown, error_temp_thrsh, error_vsupply_thrsh, error_vneg_thrsh, error_vgate_thrsh,
            error_iq1_thrsh, error_iq2_thrsh, error_iq3_thrsh, error_iq4_thrsh, error_iq5_thrsh, error_vdet_thrsh,
            error_vatt_thrsh, error_invalid_cmd_code, error_invalid_cmd_data, error_invalid_msg_data,
            error_access_denied, error_cmd_not_available, error_data_not_available, error_invalid_pa_mode,
            error_invalid_id, error_invalid_parm, error_no_status
        };

    private:
        static constexpr std::size_t updateIfRawIdx = 0;
        static constexpr std::size_t updateIfCurrCntIdx = 1;
        static constexpr std::size_t updateIfCurr1Idx = 2;
        static constexpr std::size_t updateIfCurr2Idx = 3;
        static constexpr std::size_t updateIfCurr3Idx = 4;
        static constexpr std::size_t updateIfCurr4Idx = 5;
        static constexpr std::size_t updateIfCurr5Idx = 6;
        static constexpr std::size_t updateIfCurr6Idx = 7;
        static constexpr std::size_t updateIfCurr7Idx = 8;
        static constexpr std::size_t updateIfCurr8Idx = 9;
        static constexpr std::size_t updateIfTotalCurrIdx = 10;
        static constexpr std::size_t updateIfInputVoltIdx = 11;
        static constexpr std::size_t updateIfTempIdx = 12;
        static constexpr std::size_t updateIfAlarmILimIdx = 13;
        static constexpr std::size_t updateIfAlarmSeqIdx = 14;
        static constexpr std::size_t updateIfAlarmGateIdx = 15;
        static constexpr std::size_t updateIfAlarmTempIdx = 16;
        static constexpr std::size_t updateIfAlarmAnalogIdx = 17;
        static constexpr std::size_t updateIfEnabledIdx = 18;
        static constexpr std::size_t updateIfAlarmHighIdx = 19;
        static constexpr std::size_t updateIfWarnHighIdx = 20;
        static constexpr std::size_t updateIfAlarmLowIdx = 21;
        static constexpr std::size_t updateIfWarnLowIdx = 22;
        static constexpr std::size_t updateIfPresentIdx = 23;

        static constexpr std::size_t sizeOfStatusMsgZero = 21;
        static constexpr std::size_t sizeOfStatusMsgEight = 37;

    public:
        legacyProto(statusUpdateInterface& upIf, serialInterfaceConnection& ser,
            const std::string& key, const pmod_addr_t addr, const timeout_t& time);

        void poll();
        void reset();
        void faultClear();

    private:
        void buildDataset();
        [[nodiscard]] ser_msg_t buildPmodMsg(const cmds command, const ser_msg_t& insertData = {});
        [[nodiscard]] bool validatePacket(ser_msg_t& data);
        void decodeStatus(const ser_msg_t& data);
    };
}

#endif
