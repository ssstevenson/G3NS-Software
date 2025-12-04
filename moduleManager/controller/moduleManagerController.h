#ifndef MODULE_NAMAGER_CONTROLLER_H_
#define MODULE_NAMAGER_CONTROLLER_H_

#include <array>
#include <climits>
#include <configManagerConnection/configManagerConnection.h>
#include <controller/kissProto.h>
#include <controller/legacyProto.h>
#include <fpgaInterfaceConnection/fpgaInterfaceConnection.h>
#include <functional>
#include <helpers/rfLayoutTypes.h>
#include <serialInterfaceConnection/serialInterfaceConnection.h>
#include <statusUpdateInterface/statusUpdateInterface.h>
#include <string>
#include <unordered_map>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      Controls the data flow into a module manager object and updates the view whenever data changes.
     */
    class moduleManagerController
    {
    public:
        using updateIfData_t = helpers::types::updateIfData_t;
        using timeout_t = helpers::types::timeout_t;
        using rf_bands_t = helpers::types::rf_bands_t;
        using pallet_addrs_t = helpers::types::pallet_addrs_t;
        using pmod_device_keys_t = helpers::types::pmod_device_keys_t;
        using ser_msg_t = serialInterfaceConnection::ser_msg_t;
        using reg_t = fpgaInterfaceConnection::reg_t;
        using mmData_t = fpgaInterfaceConnection::mmData_t;
        using fault_bits_t = helpers::types::pmod_bits_t;
        using side_data_t = std::array<updateIfData_t, 1>;
        using configured_t = std::array<updateIfData_t, 11>;
        using pmod_addr_t = helpers::types::pmod_addr_t;
        using pmod_map_t = std::vector<std::unique_ptr<protocolInterface>>;

    private:
        configManagerConnection& configIf;
        serialInterfaceConnection serIf;
        fpgaInterfaceConnection fpgaIf;
        statusUpdateInterface updateIf;
        const std::string pmodManagerModName;
        const std::string errorManagerModName;
        pmod_device_keys_t deviceKeys;
        rf_bands_t bands;
        pallet_addrs_t allPmodAddrs;
        pmod_map_t::iterator curPmod;
        side_data_t sideDataMatrix;
        configured_t configuredModuleArr;
        pmod_map_t pmodMap;

        struct pmodManagerRegMap
        {
            static constexpr reg_t ctrlRegOff = 0x0000;
            static constexpr reg_t alarmPolRegOff = 0x0004;
            static constexpr reg_t alarmMaskRegOff = 0x0008;
            static constexpr reg_t alarmInputRegOff = 0x000C;
            static constexpr reg_t alarmForceRegOff = 0x0010;
            static constexpr reg_t alarmMap1RegOff = 0x0020;
            static constexpr reg_t alarmMap2RegOff = 0x0024;
            static constexpr reg_t alarmMap3RegOff = 0x0028;
            static constexpr reg_t alarmMap4RegOff = 0x002C;
            static constexpr reg_t alarmMap5RegOff = 0x0030;
            static constexpr reg_t alarmMap6RegOff = 0x0034;
            static constexpr reg_t alarmMap7RegOff = 0x0038;
            static constexpr reg_t alarmMap8RegOff = 0x003C;
            static constexpr reg_t alarmMap9RegOff = 0x0040;
            static constexpr reg_t alarmMap10RegOff = 0x0044;
            static constexpr reg_t paEnableState = 0x0048;
            static constexpr reg_t offsetStepSize = (alarmMap2RegOff - alarmMap1RegOff);
        };

        struct errorManagerRegMap
        {
            static constexpr reg_t paEnableStart = 0x1200;
        };

        constexpr static std::size_t pmodHwrAlarmMapCnt = 10;
        constexpr static std::uint32_t upTimePerDev = 1250; // milliseconds
        constexpr static auto defaultFpgaTimeout = timeout_t{10000};

    public:
        moduleManagerController(zmq::context_t& zmqCtx, configManagerConnection& config);
        void initializeData();

        void setupPmodHwrAlarmMap();
        void updateSideData();
        void pollPmod();
        void reset(const std::string& addrs);
        void faultClear(const std::string& addrs);

    private:
        [[nodiscard]] std::optional<mmData_t> getPaEnableState() noexcept;
    };
}

#endif
