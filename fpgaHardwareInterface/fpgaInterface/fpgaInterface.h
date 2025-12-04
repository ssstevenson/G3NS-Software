#ifndef FPGA_INTERFACE_H_
#define FPGA_INTERFACE_H_

#include <configManagerConnection/configManagerConnection.h>
#include <functional>
#include <hardwareInterface/hardwareInterface.h>
#include <map>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <parser/parserInterface.h>
#include <string>
#include <unordered_map>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      This class describes an fpga interface.
     */
    class fpgaInterface: protected parserInterface
    {
    public:
        using parserMap_t = std::unordered_map<std::string, std::function<void(rapidjson::Document&)> >;
        using reg_t = hardwareInterface::reg_t;
        using mmData_t = hardwareInterface::mmData_t;
        using modAddrLut_t = std::unordered_map<std::string, reg_t>;
        using hardwareInterfaceMap_t = std::map<reg_t, std::unique_ptr<hardwareInterface> >;

    private:
        configManagerConnection& configIf;
        messageFactoryConnection msgIf;
        parserMap_t parserMap;
        modAddrLut_t moduleMap;
        hardwareInterfaceMap_t memoryMap;
        static constexpr reg_t defaultPageSize = 0x10000;
        static constexpr reg_t pageMask = ~(defaultPageSize - 1);
        std::string readResponseJsonStr;
        std::string readResponseErrJsonStr;
        std::string batchReadResponseJsonStr;
        std::string batchReadResponseErrJsonStr;
        std::string rangeReadResponseJsonStr;
        std::string rangeReadResponseErrJsonStr;
        std::string writeResponseJsonStr;
        std::string writeResponseErrJsonStr;
        std::string batchWriteResponseJsonStr;
        std::string batchWriteResponseErrJsonStr;
        std::string rangeWriteResponseJsonStr;
        std::string rangeWriteResponseErrJsonStr;
        std::string readModuleResponseJsonStr;
        std::string readModuleResponseErrJsonStr;
        std::string batchReadModuleResponseJsonStr;
        std::string batchReadModuleResponseErrJsonStr;
        std::string rangeReadModuleResponseJsonStr;
        std::string rangeReadModuleResponseErrJsonStr;
        std::string writeModuleResponseJsonStr;
        std::string writeModuleResponseErrJsonStr;
        std::string batchWriteModuleResponseJsonStr;
        std::string batchWriteModuleResponseErrJsonStr;
        std::string rangeWriteModuleResponseJsonStr;
        std::string rangeWriteModuleResponseErrJsonStr;

    public:
        fpgaInterface(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& cfgIf);

    private:
        void initializeData();
        void jsonRead(rapidjson::Document& jsonDoc);
        void jsonReadBatch(rapidjson::Document& jsonDoc);
        void jsonReadRange(rapidjson::Document& jsonDoc);
        void jsonWrite(rapidjson::Document& jsonDoc);
        void jsonWriteBatch(rapidjson::Document& jsonDoc);
        void jsonWriteRange(rapidjson::Document& jsonDoc);
        void jsonReadModule(rapidjson::Document& jsonDoc);
        void jsonReadModuleBatch(rapidjson::Document& jsonDoc);
        void jsonReadModuleRange(rapidjson::Document& jsonDoc);
        void jsonWriteModule(rapidjson::Document& jsonDoc);
        void jsonWriteModuleBatch(rapidjson::Document& jsonDoc);
        void jsonWriteModuleRange(rapidjson::Document& jsonDoc);

        hardwareInterface* getHwIf(const reg_t addr);
    };
}

#endif
