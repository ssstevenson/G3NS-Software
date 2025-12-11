#include "fpgaInterface.h"

#include <helpers/jsonUpdateHelpers.h>
#include <helpers/zmqConnectionNames.h>
#include <numeric>
#include <vector>
#include <syslog.h>
#include <chrono>
#include <ctime>

#ifdef syslog
#undef syslog
#endif
#define syslog(...) ((void)0)

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      ctx     The zmq context
 * @param      poller  The poller
 * @param      cfgIf   The configuration interface
 */
fpgaInterface::fpgaInterface(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& cfgIf):
    parserInterface{ctx, poller, cfgIf},
    configIf{cfgIf},
    msgIf{ctx},
    parserMap{},
    moduleMap{},
    memoryMap{},
    readResponseJsonStr{},
    readResponseErrJsonStr{},
    batchReadResponseJsonStr{},
    batchReadResponseErrJsonStr{},
    rangeReadResponseJsonStr{},
    rangeReadResponseErrJsonStr{},
    writeResponseJsonStr{},
    writeResponseErrJsonStr{},
    batchWriteResponseJsonStr{},
    batchWriteResponseErrJsonStr{},
    rangeWriteResponseJsonStr{},
    rangeWriteResponseErrJsonStr{},
    readModuleResponseJsonStr{},
    readModuleResponseErrJsonStr{},
    batchReadModuleResponseJsonStr{},
    batchReadModuleResponseErrJsonStr{},
    rangeReadModuleResponseJsonStr{},
    rangeReadModuleResponseErrJsonStr{},
    writeModuleResponseJsonStr{},
    writeModuleResponseErrJsonStr{},
    batchWriteModuleResponseJsonStr{},
    batchWriteModuleResponseErrJsonStr{},
    rangeWriteModuleResponseJsonStr{},
    rangeWriteModuleResponseErrJsonStr{}
{
    registerZmqSockets(zmq::socket_type::rep, enums::zmqConnections::getSocket("getFpgaHardwareInterfaceAPI"));

    registerParser("fpgaRequestRead", [this](rapidjson::Document& doc){ jsonRead(doc); });
    registerParser("fpgaRequestReadBatch", [this](rapidjson::Document& doc){ jsonReadBatch(doc); });
    registerParser("fpgaRequestReadRange", [this](rapidjson::Document& doc){ jsonReadRange(doc); });
    registerParser("fpgaRequestWrite", [this](rapidjson::Document& doc){ jsonWrite(doc); });
    registerParser("fpgaRequestWriteBatch", [this](rapidjson::Document& doc){ jsonWriteBatch(doc); });
    registerParser("fpgaRequestWriteRange", [this](rapidjson::Document& doc){ jsonWriteRange(doc); });
    registerParser("fpgaModuleRequestRead", [this](rapidjson::Document& doc){ jsonReadModule(doc); });
    registerParser("fpgaModuleRequestReadBatch", [this](rapidjson::Document& doc){ jsonReadModuleBatch(doc); });
    registerParser("fpgaModuleRequestReadRange", [this](rapidjson::Document& doc){ jsonReadModuleRange(doc); });
    registerParser("fpgaModuleRequestWrite", [this](rapidjson::Document& doc){ jsonWriteModule(doc); });
    registerParser("fpgaModuleRequestWriteBatch", [this](rapidjson::Document& doc){ jsonWriteModuleBatch(doc); });
    registerParser("fpgaModuleRequestWriteRange", [this](rapidjson::Document& doc){ jsonWriteModuleRange(doc); });

    registerConfigUpdate([this](){ initializeData(); });

    readResponseJsonStr = msgIf.getMessageStr("getFpgaResponseReadSuccess", true).value_or("");
    readResponseErrJsonStr = msgIf.getMessageStr("getFpgaResponseReadError", true).value_or("");
    batchReadResponseJsonStr = msgIf.getMessageStr("getFpgaResponseReadBatchSuccess", true).value_or("");
    batchReadResponseErrJsonStr = msgIf.getMessageStr("getFpgaResponseReadBatchError", true).value_or("");
    rangeReadResponseJsonStr = msgIf.getMessageStr("getFpgaResponseReadRangeSuccess", true).value_or("");
    rangeReadResponseErrJsonStr = msgIf.getMessageStr("getFpgaResponseReadRangeError", true).value_or("");
    writeResponseJsonStr = msgIf.getMessageStr("getFpgaResponseWriteSuccess", true).value_or("");
    writeResponseErrJsonStr = msgIf.getMessageStr("getFpgaResponseWriteError", true).value_or("");
    batchWriteResponseJsonStr = msgIf.getMessageStr("getFpgaResponseWriteBatchSuccess", true).value_or("");
    batchWriteResponseErrJsonStr = msgIf.getMessageStr("getFpgaResponseWriteBatchError", true).value_or("");
    rangeWriteResponseJsonStr = msgIf.getMessageStr("getFpgaResponseWriteRangeSuccess", true).value_or("");
    rangeWriteResponseErrJsonStr = msgIf.getMessageStr("getFpgaResponseWriteRangeError", true).value_or("");
    readModuleResponseJsonStr = msgIf.getMessageStr("getFpgaModuleResponseReadSuccess", true).value_or("");
    readModuleResponseErrJsonStr = msgIf.getMessageStr("getFpgaModuleResponseReadError", true).value_or("");
    batchReadModuleResponseJsonStr = msgIf.getMessageStr("getFpgaModuleResponseReadBatchSuccess", true).value_or("");
    batchReadModuleResponseErrJsonStr = msgIf.getMessageStr("getFpgaModuleResponseReadBatchError", true).value_or("");
    rangeReadModuleResponseJsonStr = msgIf.getMessageStr("getFpgaModuleResponseReadRangeSuccess", true).value_or("");
    rangeReadModuleResponseErrJsonStr = msgIf.getMessageStr("getFpgaModuleResponseReadRangeError", true).value_or("");
    writeModuleResponseJsonStr = msgIf.getMessageStr("getFpgaModuleResponseWriteSuccess", true).value_or("");
    writeModuleResponseErrJsonStr = msgIf.getMessageStr("getFpgaModuleResponseWriteError", true).value_or("");
    batchWriteModuleResponseJsonStr = msgIf.getMessageStr("getFpgaModuleResponseWriteBatchSuccess", true).value_or("");
    batchWriteModuleResponseErrJsonStr = msgIf.getMessageStr("getFpgaModuleResponseWriteBatchError", true).value_or("");
    rangeWriteModuleResponseJsonStr = msgIf.getMessageStr("getFpgaModuleResponseWriteRangeSuccess", true).value_or("");
    rangeWriteModuleResponseErrJsonStr = msgIf.getMessageStr("getFpgaModuleResponseWriteRangeError", true).value_or("");
}

/**
 * @brief      Initializes the data.
 */
void fpgaInterface::initializeData()
{
    using std::string_literals::operator""s;
    try
    {
        auto moduleList = helpers::types::dataTokenizer<std::string>(
            configIf.getParam<std::string>("FHI_MODULE_LIST").value_or(""));

        for(auto& core: moduleList)
        {
            logger::verbose(__FILE__, __FUNCTION__, "Getting Config Data for Module: "s + core);

            auto coreName{configIf.getParam<std::string>(core).value_or("")};
            if(auto coreBaseAddr{helpers::types::strToNum<reg_t>(
                configIf.getParam<std::string>(core + "_BASE_HEX_ADDRESS"s).value_or(""), 16)};
                coreBaseAddr.has_value())
            {
                moduleMap.insert(std::make_pair(coreName, coreBaseAddr.value()));
                memoryMap.insert(std::make_pair(coreBaseAddr.value(),
                    std::make_unique<hardwareInterface>(coreBaseAddr.value(), defaultPageSize)
                ));
            }
        }
    }
    catch(const std::exception& e)
    {
        logger::critical(__FILE__, __FUNCTION__, "Failed to get parameter from Configuration Manager - "s +
            std::string(e.what()));
        std::terminate();
    }

    logger::info(__FILE__, __FUNCTION__, "Data Initialization Done!!!");
}

/**
 * @brief      Json command to Read from the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonRead(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto strAddr{helpers::jsonGet<std::string>(jsonDoc, "hexAddress")}; strAddr.has_value())
        {
            if(const auto address{helpers::types::strToNum<reg_t>(strAddr.value(), 16)}; address.has_value())
            {
                const reg_t pageAddr = address.value() & pageMask;
                const reg_t offset = address.value() & ~pageMask;

                if(auto data{getHwIf(pageAddr)->read(offset)}; data.has_value())
                {
                    helpers::setupJsonResponse(jsonDoc, readResponseJsonStr);
                    helpers::jsonSet(jsonDoc, helpers::types::toHexString(data.value()), "response", "data");
                    return;
                }

                helpers::setupJsonResponse(jsonDoc, readResponseErrJsonStr);
                helpers::jsonSet<std::string>(jsonDoc, "AXI Bus Error", "response", "error");
                return;
            }
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, readResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, readResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to Read a batch of addresses in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonReadBatch(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto workData{helpers::jsonGetVec<std::string>(jsonDoc, "hexAddresses")}; workData.has_value())
        {
            std::vector<std::string>& workDataVec{workData.value()};
            std::transform(workDataVec.begin(), workDataVec.end(), workDataVec.begin(),
                [this](const std::string& addr) -> std::string {
                    if(const auto address{helpers::types::strToNum<reg_t>(addr, 16)}; address.has_value())
                    {
                        const reg_t pageAddr = address.value() & pageMask;
                        const reg_t offset = address.value() & ~pageMask;

                        if(auto data{getHwIf(pageAddr)->read(offset)}; data.has_value())
                        {
                            return helpers::types::toHexString(data.value());
                        }
                    }

                    return std::string("");
                }
            );

            helpers::setupJsonResponse(jsonDoc, batchReadResponseJsonStr);
            helpers::jsonSetVec(jsonDoc, workDataVec, "response", "data");
            return;
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, batchReadResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, batchReadResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to read a Range of addresses from the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonReadRange(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto startStr{helpers::jsonGet<std::string>(jsonDoc, "startHexAddress")},
            endStr{helpers::jsonGet<std::string>(jsonDoc, "endHexAddress")};
            startStr.has_value() && endStr.has_value())
        {
            const auto start = helpers::types::strToNum<reg_t>(startStr.value(), 16);
            const auto end = helpers::types::strToNum<reg_t>(endStr.value(), 16);

            if(start.has_value() && end.has_value())
            {
                const reg_t startPage = start.value() & pageMask;
                const reg_t endPage = end.value() & pageMask;

                const std::size_t numPages = (((startPage / defaultPageSize) - (endPage / defaultPageSize)) + 1);
                const std::size_t mapSize = defaultPageSize * numPages;

                const reg_t startOffset = start.value() & (mapSize - 1);
                const reg_t endOffset = end.value() & (mapSize - 1);

                if(auto data{getHwIf(startPage)->readRange(startOffset, endOffset)}; data.has_value())
                {
                    std::vector<std::string> dataStrVec;
                    std::vector<mmData_t>& dataVec{data.value()};
                    std::transform(dataVec.begin(), dataVec.end(), std::back_inserter(dataStrVec),
                        helpers::types::toHexString<mmData_t>);
                    helpers::setupJsonResponse(jsonDoc, rangeReadResponseJsonStr);
                    helpers::jsonSetVec(jsonDoc, dataStrVec, "response", "data");
                    return;
                }

                helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
                helpers::jsonSet<std::string>(jsonDoc, "AXI Bus Error", "response", "error");
                return;
            }
        }

        const std::string errMsg{"Error with Start Hex Address or Stop Hex Address:"};
        helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to write to an address in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonWrite(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto hexData{helpers::jsonGet<std::string>(jsonDoc, "data")},
            hexAddr{helpers::jsonGet<std::string>(jsonDoc, "hexAddress")};
            hexData.has_value() && hexAddr.has_value())
        {
            const auto writeVal = helpers::types::strToNum<mmData_t>(hexData.value(), 16);
            const auto address{helpers::types::strToNum<reg_t>(hexAddr.value(), 16)};

            if(writeVal.has_value() && address.has_value())
            {
                const reg_t pageAddr = address.value() & pageMask;
                const reg_t offset = address.value() & ~pageMask;
                auto hwIfPtr{getHwIf(pageAddr)};

                // Log write operation with timestamp
                auto now = std::chrono::system_clock::now();
                auto time_t_now = std::chrono::system_clock::to_time_t(now);
                syslog(LOG_DEBUG, "FPGA Write - Address: 0x%lx, Time: %s, value: %u", address.value(), std::ctime(&time_t_now),  writeVal.value());

                if(hwIfPtr->write(offset, writeVal.value()))
                {
                    if(auto readData{hwIfPtr->read(offset)}; readData.has_value())
                    {
                        helpers::setupJsonResponse(jsonDoc, writeResponseJsonStr);
                        helpers::jsonSet(jsonDoc, helpers::types::toHexString(readData.value()), "response", "data");
                        return;
                    }
                }

                const std::string errMsg{"AXI Bus Error"};
                helpers::setupJsonResponse(jsonDoc, writeModuleResponseErrJsonStr);
                helpers::jsonSet(jsonDoc, errMsg, "response", "error");
                logger::warn(__FILE__, __FUNCTION__, errMsg);
                return;
            }
        }

        const std::string errMsg{"Error with Hex Address or Data:"};
        helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, writeModuleResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, writeModuleResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to write to a batch of addresses in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonWriteBatch(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto hexAddrs{helpers::jsonGetVec<std::string>(jsonDoc, "hexAddresses")}; hexAddrs.has_value())
        {
            if(auto hexData{helpers::jsonGetVec<std::string>(jsonDoc, "data")}; hexData.has_value())
            {
                if(hexAddrs.value().size() == hexData.value().size())
                {
                    bool writtenAll{true};
                    std::vector<std::string> readBack(hexAddrs.value().size());

                    for(std::size_t idx = 0; idx < hexAddrs.value().size(); idx++)
                    {
                        bool writtenThisVal{false};
                        const auto writeVal{helpers::types::strToNum<mmData_t>(hexData.value().at(idx), 16)};
                        const auto address{helpers::types::strToNum<reg_t>(hexAddrs.value().at(idx), 16)};

                        if(writeVal.has_value() && address.has_value())
                        {
                            const reg_t pageAddr = address.value() & pageMask;
                            const reg_t offset = address.value() & ~pageMask;
                            auto hwIfPtr{getHwIf(pageAddr)};

                            // Log write operation with timestamp
                            auto now = std::chrono::system_clock::now();
                            auto time_t_now = std::chrono::system_clock::to_time_t(now);
                            syslog(LOG_DEBUG, "FPGA Write - Address: 0x%lx, Time: %s, value: %u", address.value(), std::ctime(&time_t_now),  writeVal.value());

                            if(hwIfPtr->write(offset, writeVal.value()))
                            {
                                if(auto readData{hwIfPtr->read(offset)}; readData.has_value())
                                {
                                    writtenThisVal = true;
                                    readBack.at(idx) = helpers::types::toHexString(readData.value());
                                }
                            }
                        }

                        writtenAll &= writtenThisVal;
                    }

                    if(writtenAll)
                    {
                        helpers::setupJsonResponse(jsonDoc, batchWriteResponseJsonStr);
                        helpers::jsonSetVec(jsonDoc, readBack, "response", "data");
                        return;
                    }
                }

                const std::string errMsg{"AXI Bus Error"};
                helpers::setupJsonResponse(jsonDoc, batchWriteResponseErrJsonStr);
                helpers::jsonSet(jsonDoc, errMsg, "response", "error");
                logger::warn(__FILE__, __FUNCTION__, errMsg);
                return;
            }
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, batchWriteResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, batchWriteResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to write a range of addresses in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonWriteRange(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto startStr{helpers::jsonGet<std::string>(jsonDoc, "startHexAddress")}; startStr.has_value())
        {
            if(auto dataVecOpt{helpers::jsonGetVec<std::string>(jsonDoc, "data")}; dataVecOpt.has_value())
            {
                if(const auto start = helpers::types::strToNum<reg_t>(startStr.value(), 16); start.has_value())
                {
                    std::vector<std::string>& dataVecStr{dataVecOpt.value()};
                    std::vector<std::optional<mmData_t>> mmDataOptionalVec;

                    const reg_t end = start.value() + (dataVecOpt.value().size() << 2);

                    const reg_t startPage = start.value() & pageMask;
                    const reg_t endPage = end & pageMask;

                    const std::size_t numPages = (((startPage / defaultPageSize) - (endPage / defaultPageSize)) + 1);
                    const std::size_t mapSize = defaultPageSize * numPages;

                    const reg_t startOffset = start.value() & (mapSize - 1);
                    const reg_t endOffset = end & (mapSize - 1);

                    auto hwIfPtr{getHwIf(startPage)};

                    std::transform(dataVecStr.begin(), dataVecStr.end(), std::back_inserter(mmDataOptionalVec),
                        [&](auto const& str){
                            return helpers::types::strToNum<mmData_t>(str, 16);
                        }
                    );

                    if(std::accumulate(std::begin(mmDataOptionalVec), std::end(mmDataOptionalVec), true,
                        [](const bool acc, const std::optional<mmData_t>& val){ return acc && val.has_value(); }))
                    {
                        std::vector<mmData_t> mmDataVec;

                        std::transform(std::begin(mmDataOptionalVec), std::end(mmDataOptionalVec),
                            std::back_inserter(mmDataVec), [](const auto& d){ return d.value(); });

                        // Log range write operation with timestamp
                        auto now = std::chrono::system_clock::now();
                        auto time_t_now = std::chrono::system_clock::to_time_t(now);
                        syslog(LOG_DEBUG, "FPGA Range Write - Start Address: 0x%lx, End Address: 0x%lx, Time: %s", 
                            start.value(), end, std::ctime(&time_t_now));

                        if(hwIfPtr->writeRange(startOffset, mmDataVec))
                        {
                            if(auto data{hwIfPtr->readRange(startOffset, endOffset)}; data.has_value())
                            {
                                std::vector<std::string> dataStrVec;
                                std::vector<mmData_t>& dataVec{data.value()};
                                std::transform(dataVec.begin(), dataVec.end(), std::back_inserter(dataStrVec),
                                    helpers::types::toHexString<mmData_t>);
                                helpers::setupJsonResponse(jsonDoc, rangeReadResponseJsonStr);
                                helpers::jsonSetVec(jsonDoc, dataStrVec, "response", "data");
                                return;
                            }
                        }

                        const std::string errMsg{"AXI Bus Error"};
                        helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
                        helpers::jsonSet<std::string>(jsonDoc, errMsg, "response", "error");
                        logger::warn(__FILE__, __FUNCTION__, errMsg);
                        return;
                    }
                }

                const std::string errMsg{"Error with Starting Address:"};
                helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
                helpers::jsonSet(jsonDoc, errMsg, "response", "error");
                logger::warn(__FILE__, __FUNCTION__, errMsg);
                return;
            }
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to read an offset address from a Module in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonReadModule(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto modName{helpers::jsonGet<std::string>(jsonDoc, "module")},
            offsetStr{helpers::jsonGet<std::string>(jsonDoc, "hexOffset")};
            modName.has_value() && offsetStr.has_value())
        {
            if(modAddrLut_t::iterator modIt = moduleMap.find(modName.value()); modIt != moduleMap.end())
            {
                if(auto data{getHwIf(modIt->second)->read(
                    helpers::types::strToNum<reg_t>(offsetStr.value(), 16).value_or(0))}; data.has_value())
                {
                    helpers::setupJsonResponse(jsonDoc, readModuleResponseJsonStr);
                    helpers::jsonSet(jsonDoc, helpers::types::toHexString(data.value()), "response", "data");
                    return;
                }

                const std::string errMsg{"AXI Bus Error"};
                helpers::setupJsonResponse(jsonDoc, readModuleResponseErrJsonStr);
                helpers::jsonSet(jsonDoc, errMsg, "response", "error");
                logger::warn(__FILE__, __FUNCTION__, errMsg);
                return;
            }

            const std::string errMsg{"Undefined Module"};
            helpers::setupJsonResponse(jsonDoc, readModuleResponseErrJsonStr);
            helpers::jsonSet(jsonDoc, errMsg, "response", "error");
            logger::warn(__FILE__, __FUNCTION__, errMsg);
            return;
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, readModuleResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, readModuleResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to read a batch of addresses from a Module in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonReadModuleBatch(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto modName{helpers::jsonGet<std::string>(jsonDoc, "module")}; modName.has_value())
        {
            if(auto workData{helpers::jsonGetVec<std::string>(jsonDoc, "hexOffsets")}; workData.has_value())
            {
                if(modAddrLut_t::iterator modIt = moduleMap.find(modName.value()); modIt != moduleMap.end())
                {
                    std::vector<std::string>& workDataVec{workData.value()};
                    std::transform(workDataVec.begin(), workDataVec.end(), workDataVec.begin(),
                        [&](const std::string& addr) -> std::string {
                            return helpers::types::toHexString(getHwIf(modIt->second)->read(
                                helpers::types::strToNum<reg_t>(addr, 16).value_or(0)).value_or(0)
                            );
                        }
                    );

                    helpers::setupJsonResponse(jsonDoc, batchReadModuleResponseJsonStr);
                    helpers::jsonSetVec(jsonDoc, workDataVec, "response", "data");
                    return;
                }

                const std::string errMsg{"AXI Bus Error"};
                helpers::setupJsonResponse(jsonDoc, batchReadModuleResponseErrJsonStr);
                helpers::jsonSet(jsonDoc, errMsg, "response", "error");
                logger::warn(__FILE__, __FUNCTION__, errMsg);
                return;
            }

            const std::string errMsg{"Undefined Module"};
            helpers::setupJsonResponse(jsonDoc, batchReadModuleResponseErrJsonStr);
            helpers::jsonSet(jsonDoc, errMsg, "response", "error");
            logger::warn(__FILE__, __FUNCTION__, errMsg);
            return;
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, batchReadModuleResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, batchReadModuleResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to read a range of addresses from a Module in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonReadModuleRange(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto modName{helpers::jsonGet<std::string>(jsonDoc, "module")},
            startOffset{helpers::jsonGet<std::string>(jsonDoc, "startHexOffset")},
            stopOffset{helpers::jsonGet<std::string>(jsonDoc, "endHexOffset")};
            modName.has_value() && startOffset.has_value() && stopOffset.has_value())
        {
            if(modAddrLut_t::iterator modIt = moduleMap.find(modName.value()); modIt != moduleMap.end())
            {
                if(auto data{
                    getHwIf(modIt->second)->readRange(
                        helpers::types::strToNum<reg_t>(startOffset.value(), 16).value_or(0),
                        helpers::types::strToNum<reg_t>(stopOffset.value(), 16).value_or(0)
                    )};
                    data.has_value())
                {
                    std::vector<mmData_t>& dataVec{data.value()};
                    std::vector<std::string> dataStr;
                    std::transform(dataVec.begin(), dataVec.end(), std::back_inserter(dataStr),
                        [&](const mmData_t val) -> std::string {
                            return helpers::types::toHexString(val);
                        }
                    );

                    helpers::setupJsonResponse(jsonDoc, rangeReadModuleResponseJsonStr);
                    helpers::jsonSetVec(jsonDoc, dataStr, "response", "data");
                    return;
                }

                const std::string errMsg{"Failed to Read Data Range"};
                helpers::setupJsonResponse(jsonDoc, rangeReadModuleResponseErrJsonStr);
                helpers::jsonSet(jsonDoc, errMsg, "response", "error");
                logger::warn(__FILE__, __FUNCTION__, errMsg);
                return;
            }

            const std::string errMsg{"Undefined Module"};
            helpers::setupJsonResponse(jsonDoc, rangeReadModuleResponseErrJsonStr);
            helpers::jsonSet(jsonDoc, errMsg, "response", "error");
            logger::warn(__FILE__, __FUNCTION__, errMsg);
            return;
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, rangeReadModuleResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, rangeReadModuleResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to write to a Module in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonWriteModule(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto modName{helpers::jsonGet<std::string>(jsonDoc, "module")},
            offset{helpers::jsonGet<std::string>(jsonDoc, "hexOffset")},
            data{helpers::jsonGet<std::string>(jsonDoc, "data")};
            modName.has_value() && offset.has_value() && data.has_value())
        {
            if(modAddrLut_t::iterator modIt = moduleMap.find(modName.value()); modIt != moduleMap.end())
            {
                if(getHwIf(modIt->second)->write(helpers::types::strToNum<reg_t>(offset.value(), 16).value_or(0),
                    helpers::types::strToNum<mmData_t>(data.value(), 16).value_or(0)))
                {
                    if(auto regVal{getHwIf(modIt->second)->read(
                        helpers::types::strToNum<reg_t>(offset.value(), 16).value_or(0))}; regVal.has_value())
                    {
                        helpers::setupJsonResponse(jsonDoc, writeModuleResponseJsonStr);
                        helpers::jsonSet(jsonDoc, helpers::types::toHexString(regVal.value()), "response", "data");
                        return;
                    }
                }

                const std::string errMsg{"AXI Bus Error"};
                helpers::setupJsonResponse(jsonDoc, writeModuleResponseErrJsonStr);
                helpers::jsonSet(jsonDoc, errMsg, "response", "error");
                logger::warn(__FILE__, __FUNCTION__, errMsg);
                return;
            }

            const std::string errMsg{"Undefined Module"};
            helpers::setupJsonResponse(jsonDoc, writeModuleResponseErrJsonStr);
            helpers::jsonSet(jsonDoc, errMsg, "response", "error");
            logger::warn(__FILE__, __FUNCTION__, errMsg);
            return;
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, writeModuleResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, writeModuleResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to write to a batch of addresses in a Module in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonWriteModuleBatch(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto modName{helpers::jsonGet<std::string>(jsonDoc, "module")}; modName.has_value())
        {
            if(modAddrLut_t::iterator modIt = moduleMap.find(modName.value()); modIt != moduleMap.end())
            {
                if(auto hexOffsets{helpers::jsonGetVec<std::string>(jsonDoc, "hexOffsets")}; hexOffsets.has_value())
                {
                    if(auto hexData{helpers::jsonGetVec<std::string>(jsonDoc, "data")}; hexData.has_value())
                    {
                        if(hexOffsets.value().size() == hexData.value().size())
                        {
                            bool writtenAll{true};
                            std::vector<std::string> readBack(hexOffsets.value().size());

                            for(std::size_t idx = 0; idx < hexOffsets.value().size(); idx++)
                            {
                                bool writtenThisVal{false};
                                const auto writeVal{helpers::types::strToNum<mmData_t>(hexData.value().at(idx), 16)};
                                const auto offset{helpers::types::strToNum<reg_t>(hexOffsets.value().at(idx), 16)};
                                auto hwIfPtr{getHwIf(modIt->second)};

                                if(writeVal.has_value() && offset.has_value())
                                {
                                    if(hwIfPtr->write(offset.value(), writeVal.value()))
                                    {
                                        if(auto readData{hwIfPtr->read(offset.value())}; readData.has_value())
                                        {
                                            writtenThisVal = true;
                                            readBack.at(idx) = helpers::types::toHexString(readData.value());
                                        }
                                    }
                                }

                                writtenAll &= writtenThisVal;
                            }

                            if(writtenAll)
                            {
                                helpers::setupJsonResponse(jsonDoc, batchWriteResponseJsonStr);
                                helpers::jsonSetVec(jsonDoc, readBack, "response", "data");
                                return;
                            }
                        }

                        const std::string errMsg{"AXI Bus Error"};
                        helpers::setupJsonResponse(jsonDoc, batchWriteResponseErrJsonStr);
                        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
                        logger::warn(__FILE__, __FUNCTION__, errMsg);
                        return;
                    }
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, batchWriteResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, batchWriteResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Json command to write to a Module in the FPGA
 *
 * @param      jsonDoc  The json document
 */
void fpgaInterface::jsonWriteModuleRange(rapidjson::Document& jsonDoc)
{
    try
    {
        if(auto modName{helpers::jsonGet<std::string>(jsonDoc, "module")}; modName.has_value())
        {
            if(modAddrLut_t::iterator modIt = moduleMap.find(modName.value()); modIt != moduleMap.end())
            {
                if(auto startStr{helpers::jsonGet<std::string>(jsonDoc, "hexOffsetStart")}; startStr.has_value())
                {
                    if(auto dataVecOpt{helpers::jsonGetVec<std::string>(jsonDoc, "data")}; dataVecOpt.has_value())
                    {
                        std::vector<std::string>& dataVecStr{dataVecOpt.value()};
                        std::vector<std::optional<mmData_t>> mmDataOptionalVec;

                        if(const auto start{helpers::types::strToNum<reg_t>(startStr.value(), 16)}; start.has_value())
                        {
                            const reg_t end = start.value() + (dataVecOpt.value().size() << 2);

                            const reg_t startPage = start.value() & pageMask;
                            const reg_t endPage = end & pageMask;

                            const std::size_t numPages = (((startPage / defaultPageSize) -
                                (endPage / defaultPageSize)) + 1);
                            const std::size_t mapSize = defaultPageSize * numPages;

                            const reg_t startOffset = start.value() & (mapSize - 1);
                            const reg_t endOffset = end & (mapSize - 1);

                            auto hwIfPtr{getHwIf(modIt->second)};

                            std::transform(dataVecStr.begin(), dataVecStr.end(), std::back_inserter(mmDataOptionalVec),
                                [&](auto const& str){
                                    return helpers::types::strToNum<mmData_t>(str, 16);
                                }
                            );

                            if(std::accumulate(std::begin(mmDataOptionalVec), std::end(mmDataOptionalVec), true,
                                [](const bool acc, const std::optional<mmData_t>& val){ return acc && val.has_value(); }))
                            {
                                std::vector<mmData_t> mmDataVec;

                                std::transform(std::begin(mmDataOptionalVec), std::end(mmDataOptionalVec),
                                    std::back_inserter(mmDataVec), [](const auto& d){ return d.value(); });

                                if(hwIfPtr->writeRange(startOffset, mmDataVec))
                                {
                                    if(auto data{hwIfPtr->readRange(startOffset, endOffset)}; data.has_value())
                                    {
                                        std::vector<std::string> dataStrVec;
                                        std::vector<mmData_t>& dataVec{data.value()};
                                        std::transform(dataVec.begin(), dataVec.end(), std::back_inserter(dataStrVec),
                                            helpers::types::toHexString<mmData_t>);
                                        helpers::setupJsonResponse(jsonDoc, rangeReadResponseJsonStr);
                                        helpers::jsonSetVec(jsonDoc, dataStrVec, "response", "data");
                                        return;
                                    }
                                }

                                const std::string errMsg{"AXI Bus Error"};
                                helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
                                helpers::jsonSet<std::string>(jsonDoc, errMsg, "response", "error");
                                logger::warn(__FILE__, __FUNCTION__, errMsg);
                                return;
                            }
                        }

                        const std::string errMsg{"Error with Starting Address:"};
                        helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
                        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
                        logger::warn(__FILE__, __FUNCTION__, errMsg);
                        return;
                    }
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        const std::string errMsg{"Failed to setup read JSON object correctly - " + std::string(e.what())};
        helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
        helpers::jsonSet(jsonDoc, errMsg, "response", "error");
        logger::warn(__FILE__, __FUNCTION__, errMsg);
        return;
    }

    const std::string errMsg{"JSON Error"};
    helpers::setupJsonResponse(jsonDoc, rangeReadResponseErrJsonStr);
    helpers::jsonSet(jsonDoc, errMsg, "response", "error");
    logger::warn(__FILE__, __FUNCTION__, errMsg);
}

/**
 * @brief      Gets the hardware interface.
 *
 * @param[in]  addr  The address
 *
 * @return     The hardware interface.
 */
hardwareInterface* fpgaInterface::getHwIf(const reg_t addr)
{
    const auto baseAddr{addr & ~pageMask};
    hardwareInterfaceMap_t::iterator pos{memoryMap.find(addr)};

    if(memoryMap.end() == pos)
    {
        std::tie(pos, std::ignore) = memoryMap.insert(std::make_pair(baseAddr,
            std::make_unique<hardwareInterface>(baseAddr, defaultPageSize)));
    }

    return pos->second.get();
}
