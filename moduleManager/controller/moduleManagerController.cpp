#include "moduleManagerController.h"

#include <algorithm>
#include <logger/logger.h>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      zmqCtx  The zmq context
 * @param      config  The configuration
 */
moduleManagerController::moduleManagerController(zmq::context_t& zmqCtx, configManagerConnection& config):
    configIf{config},
    serIf{zmqCtx, "getSerialPmodInterfaceAPI"},
    fpgaIf{zmqCtx},
    updateIf{zmqCtx},
    pmodManagerModName{"PMOD"},
    errorManagerModName{"ERROR"},
    deviceKeys{},
    bands{},
    allPmodAddrs{},
    curPmod{},
    sideDataMatrix{
        updateIfData_t{"PA_ENABLE_STATE", "Current state of the PA Enable Bits", "", defaultFpgaTimeout}
    },
    configuredModuleArr{},
    pmodMap{}
{}

/**
 * @brief      Initializes the data.
 */
void moduleManagerController::initializeData()
{
    using std::string_literals::operator""s;
    using namespace helpers::types;

    bands = getRfLayoutStructure(configIf);
    allPmodAddrs = getAllPmods(bands);
    deviceKeys = getDeviceKeys(configIf, allPmodAddrs);

    setupPmodHwrAlarmMap();

    const auto buildPmod =
        [&, defTimeout = timeout_t{allPmodAddrs.size() * upTimePerDev}](const auto& pmodDev)
    {
        const auto busAddr{pmodDevToBusAddr(pmodDev)};
        const auto keyHeader{deviceKeys.at(busAddr)};
        const auto thisProtocol{helpers::types::pmodDevToProtocol(pmodDev)};
        std::unique_ptr<protocolInterface> proto;

        switch(thisProtocol)
        {
            case pmod_protocol_t::KISS_1:
                proto = std::unique_ptr<protocolInterface>(
                    new kissProto{updateIf, serIf, keyHeader, busAddr, defTimeout});
                break;

            case pmod_protocol_t::XMEGA_256_1:
            case pmod_protocol_t::XMEGA_128_1:
            default:
                proto = std::unique_ptr<protocolInterface>(
                    new legacyProto{updateIf, serIf, keyHeader, busAddr, defTimeout});
                break;
        }

        return proto;
    };

    pmodMap.clear();
    std::transform(std::begin(allPmodAddrs), std::end(allPmodAddrs), std::back_inserter(pmodMap), buildPmod);
    curPmod = std::begin(pmodMap);

    for(std::size_t i{0}; i < configuredModuleArr.size(); ++i)
    {
        auto name{configIf.getParam<std::string>("INV_RF_PMOD_NAME_"s + std::to_string(i + 1)).value_or("")};
        configuredModuleArr.at(i) = updateIfData_t(name + "_CONFIGURED"s, name + " is Configured for the System"s,
            "",  timeout_t(deviceKeys.size() * upTimePerDev));

        configuredModuleArr.at(i).valid = true;
    }

    for(const auto& pmod: pmodMap)
    {
        configured_t::size_type add{static_cast<configured_t::size_type>(*pmod - 1)};
        configuredModuleArr.at(add).data = true;
    }
}

/**
 * @brief      Sets up the PMOD Hardware Alarm Mapping
 */
void moduleManagerController::setupPmodHwrAlarmMap()
{
    logger::info(__FILE__, __FUNCTION__, "Setting up the PMOD Hardware Polling...");

    try
    {
        using namespace helpers::types;

        fault_bits_t alarmMask{0};
        fault_bits_t alarmPolarity{0};

        // Setup Hardware Fault Line mapping...
        for(const auto& dev: allPmodAddrs)
        {
            auto devCfg{pmodDevToHwrConfig(dev)};
            reg_t errLutIdx{hwrConfigToErrLutIdx(devCfg)};
            reg_t offset = pmodManagerRegMap::alarmMap1RegOff + (pmodManagerRegMap::offsetStepSize * errLutIdx);

            if(errLutIdx >= pmodHwrAlarmMapCnt)
            {
                logger::warn(__FILE__, __FUNCTION__, std::string("Error LUT Index for ") +
                    deviceKeys[pmodDevToBusAddr(dev)] + std::string(" is invalid (0-9): ") +
                    std::to_string(errLutIdx));
                break;
            }

            alarmMask.set(errLutIdx);
            if(hwrConfigToInverted(devCfg))
            {
                alarmPolarity.set(errLutIdx);
            }

            fpgaIf.write(pmodManagerModName, offset, hwrConfigToFaultLine(devCfg));
        }

        // Setup Alarm Mask
        fpgaIf.write(pmodManagerModName, pmodManagerRegMap::alarmMaskRegOff,
            static_cast<mmData_t>(alarmMask.to_ulong()));

        // Setup Alarm Polarity
        fpgaIf.write(pmodManagerModName, pmodManagerRegMap::alarmPolRegOff,
            static_cast<mmData_t>(alarmPolarity.to_ulong()));

        // Setup Error LUT Table Entries
        for(const auto& [paEn, bits]: getAllPaEnPmodBits(bands))
        {
            fpgaIf.write(errorManagerModName, errorManagerRegMap::paEnableStart + (paEnToRegVal(paEn) << 2),
                static_cast<mmData_t>(bits.to_ulong()));
        }
    }
    catch(const std::exception& e)
    {
        logger::warn(__FILE__, __FUNCTION__, std::string("Serial Polling Exception - ") + std::string(e.what()));
    }
}

/**
 * @brief      Periodically updates polled data from the FPGA about the PMODs
 */
void moduleManagerController::updateSideData()
{
    using namespace helpers::types;

    constexpr std::size_t paEnStateIdx = 0;

    const auto getPaEnState = [&](){
        auto resp{getPaEnableState()};
        return std::make_pair(resp.value_or(0), resp.has_value());
    };

    std::tie(sideDataMatrix.at(paEnStateIdx).data, sideDataMatrix.at(paEnStateIdx).valid) = getPaEnState();

    updateIf.updateData(sideDataMatrix);
}

/**
 * @brief      Periodically updates polled data from the PMODs
 */
void moduleManagerController::pollPmod()
{
    using std::string_literals::operator""s;
    const auto pmodAddr{static_cast<pmod_addr_t>(*(curPmod->get()))};

    logger::verbose(__FILE__, __FUNCTION__, "PMOD Polling for "s + deviceKeys[pmodAddr] + " (Bus Address: "s +
        std::to_string(pmodAddr) + ")"s);

    (*curPmod)->poll();

    std::advance(curPmod, 1);
    if(std::end(pmodMap) == curPmod)
    {
        curPmod = std::begin(pmodMap);
    }

    updateIf.updateData(configuredModuleArr);
}

/**
 * @brief      Resets the given addresses.
 *
 * @param[in]  addrs  The addresses
 */
void moduleManagerController::reset(const std::string& addrs)
{
    if((addrs.size() == 0) || (addrs.front() == '*'))
    {
        for_each(std::begin(pmodMap), std::end(pmodMap), [&](auto& dev){ dev->reset(); });
    }
    else
    {
        for(const auto& a: helpers::types::dataTokenizer<pmod_addr_t>(addrs, ",", 10))
        {
            for(const auto& p: pmodMap)
            {
                if(a == *p)
                {
                    p->reset();
                }
            }
        }
    }
}

/**
 * @brief      Clears the faults for the given addresses
 *
 * @param[in]  addrs  The addresses
 */
void moduleManagerController::faultClear(const std::string& addrs)
{
    if((addrs.size() == 0) || (addrs.front() == '*'))
    {
        for_each(std::begin(pmodMap), std::end(pmodMap), [&](auto& dev){ dev->faultClear(); });
    }
    else
    {
        for(const auto& a: helpers::types::dataTokenizer<pmod_addr_t>(addrs, ",", 10))
        {
            for(const auto& p: pmodMap)
            {
                if(a == *p)
                {
                    p->faultClear();
                }
            }
        }
    }
}

/**
 * @brief      Gets the pa enable state from the FPGA.
 *
 * @return     The pa enable state if sucessful, std::nullopt otherwise.
 */
[[nodiscard]] std::optional<moduleManagerController::mmData_t> moduleManagerController::getPaEnableState() noexcept
{
    return fpgaIf.read(pmodManagerModName, pmodManagerRegMap::paEnableState);
}
