#ifndef SERIAL_HARDWARE_INTERFACE_PARSER_H_
#define SERIAL_HARDWARE_INTERFACE_PARSER_H_

#include <messageFactoryConnection/messageFactoryConnection.h>
#include <parser/parserInterface.h>
#include <serialInterface/serialInterface.h>
#include <string>
#include <utility>
#include <vector>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      This class describes a serial hardware interface parser.
     */
    class serialHardwareInterfaceParser: protected parserInterface
    {
    public:
        using ser_reg_t = serialInterface::hw_reg_t;
        using ser_data_t = helpers::types::ser_ext_data_t;

    private:
        serialInterface serIf;
        messageFactoryConnection msgIf;
        std::string serialResponseSuccessJsonStr, serialResponseErrorJsonStr;

        static constexpr ser_data_t FEND = 0xC0;
        static constexpr ser_data_t FESC = 0xDB;
        static constexpr ser_data_t TFEND = 0xDC;
        static constexpr ser_data_t TFESC = 0xDD;

    public:
        serialHardwareInterfaceParser(zmq::context_t& zmqCtx, helpers::poller& poller, configManagerConnection& conf,
            const std::string& sockNameStr, const ser_reg_t baseAddr);

    private:
        void jsonResponse(rapidjson::Document& jsonDoc);
        std::vector<ser_data_t> kissEncode(const std::vector<ser_data_t>& in);
        std::vector<ser_data_t> kissDecode(const std::vector<ser_data_t>& in);
    };
}

#endif
