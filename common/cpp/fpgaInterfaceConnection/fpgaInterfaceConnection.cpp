#include "fpgaInterfaceConnection.h"

#include <chrono>
#include <helpers/jsonUpdateHelpers.h>
#include <helpers/types.h>
#include <helpers/zmqHelpers.h>
#include <logger/logger.h>
#include <thread>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      zmqCtx  The zmq context
 */
fpgaInterfaceConnection::fpgaInterfaceConnection(zmq::context_t& zmqCtx):
    ctx{zmqCtx},
    reqRepSock{helpers::getSockConnect(ctx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getFpgaHardwareInterfaceAPI"))},
    msgIf{zmqCtx},
    fpgaRequestModuleWriteJsonStr{},
    fpgaRequestModuleReadJsonStr{},
    fpgaRequestModuleReadBatchJsonStr{},
    fpgaRequestModuleReadRangeJsonStr{}
{
    fpgaRequestModuleWriteJsonStr = msgIf.getMessageStr("getFpgaModuleRequestWrite", true).value_or("");
    fpgaRequestModuleReadJsonStr = msgIf.getMessageStr("getFpgaModuleRequestRead", true).value_or("");
    fpgaRequestModuleReadBatchJsonStr = msgIf.getMessageStr("getFpgaModuleRequestReadBatch", true).value_or("");
    fpgaRequestModuleReadRangeJsonStr = msgIf.getMessageStr("getFpgaModuleRequestReadRange", true).value_or("");
}

/**
 * @brief      Writes data to the FPGA
 *
 * @param[in]  module_name  The module name
 * @param[in]  reg_offset   The register offset
 * @param[in]  reg_data     The register data
 *
 * @return     True if the write was sucessful, False otherwise.
 */
bool fpgaInterfaceConnection::write(const std::string& module_name, reg_t reg_offset,
    const mmData_t& reg_data) noexcept
{
    try
    {
        if(rapidjson::Document workingDoc; !workingDoc.Parse(fpgaRequestModuleWriteJsonStr.c_str(),
            fpgaRequestModuleWriteJsonStr.size()).HasParseError())
        {
            if(helpers::jsonSet(workingDoc, module_name, "module") &&
                helpers::jsonSet(workingDoc, helpers::types::toHexString(reg_offset), "hexOffset") &&
                helpers::jsonSet(workingDoc, helpers::types::toHexString(reg_data), "data"))
            {
                if(helpers::sendReqRep(workingDoc, reqRepSock.get(), [this](){ setupSocket(); }))
                {
                    if(auto result{helpers::jsonGet<std::string>(workingDoc, "response", "data")}; result.has_value())
                    {
                        if(auto val{helpers::types::strToNum<mmData_t>(result.value(), 16)}; val.has_value())
                        {
                            return val.value() == reg_data;
                        }
                    }
                }
            }
        }
    }
    catch(...)
    {}

    logger::warn(__FILE__, __FUNCTION__, std::string(" ***ERROR - Failed to set FPGA Reg State to ") +
        helpers::types::toHexString(reg_data));

    return false;
}

/**
 * @brief      Read's data from the FPGA
 *
 * @param[in]  module_name  The module name
 * @param[in]  reg_offset   The register offset
 *
 * @return     The read data if the read was sucessful, std::nulopt otherwise.
 */
std::optional<fpgaInterfaceConnection::mmData_t> fpgaInterfaceConnection::read(
    const std::string& module_name, reg_t reg_offset) noexcept
{
    try
    {
        if(rapidjson::Document workingDoc; !workingDoc.Parse(fpgaRequestModuleReadJsonStr.c_str(),
            fpgaRequestModuleReadJsonStr.size()).HasParseError())
        {
            if(helpers::jsonSet(workingDoc, module_name, "module") &&
                helpers::jsonSet(workingDoc, helpers::types::toHexString(reg_offset), "hexOffset") )
            {
                if(helpers::sendReqRep(workingDoc, reqRepSock.get(), [this](){ setupSocket(); }))
                {
                    if(auto result{helpers::jsonGet<std::string>(workingDoc, "response", "data")}; result.has_value())
                    {
                        return helpers::types::strToNum<mmData_t>(result.value(), 16);
                    }
                }
            }
        }
    }
    catch(...)
    {}

    logger::warn(__FILE__, __FUNCTION__, "***ERROR - Failed to get FPGA Module Register State!!!");

    return std::nullopt;
}

/**
 * @brief      Reads a vector of random addresses from the FPGA
 *
 * @param[in]  module  The module
 * @param[in]  addr    The address
 *
 * @return     The read data if the read was sucessful, std::nullopt otherwise.
 */
std::optional<std::vector<fpgaInterfaceConnection::mmData_t>> fpgaInterfaceConnection::read(
    const std::string& module, const std::vector<reg_t>& addr) noexcept
{
    try
    {
        if(rapidjson::Document workingDoc; !workingDoc.Parse(fpgaRequestModuleReadBatchJsonStr.c_str(),
            fpgaRequestModuleReadBatchJsonStr.size()).HasParseError())
        {
            std::vector<std::string> addrStr;

            std::transform(addr.begin(), addr.end(), std::back_inserter(addrStr),
                [&](const auto& d){ return helpers::types::toHexString(d); });

            if(helpers::jsonSet(workingDoc, module, "module")
                && helpers::jsonSetVec(workingDoc, addrStr, "hexOffsets"))
            {
                if(helpers::sendReqRep(workingDoc, reqRepSock.get(), [this](){ setupSocket(); }))
                {
                    if(auto result{helpers::jsonGetVec<std::string>(workingDoc, "response", "data")};
                        result.has_value())
                    {
                        std::vector<std::string>& resultVec{result.value()};

                        if(addr.size() == resultVec.size())
                        {
                            std::vector<mmData_t> data;
                            std::transform(resultVec.begin(), resultVec.end(), std::back_inserter(data),
                                [&](const auto& str){
                                    return std::move(helpers::types::strToNum<mmData_t>(str, 16).value_or(0));
                                });

                            return data;
                        }
                    }
                }
            }
        }
    }
    catch(...)
    {}

    logger::warn(__FILE__, __FUNCTION__, "***ERROR - Failed to get FPGA Module Register State!!!");

    return std::nullopt;
}

/**
 * @brief      Reads a contiguous range from the FPGA
 *
 * @param[in]  module  The module
 * @param[in]  start   The start address
 * @param[in]  stop    The stop address
 *
 * @return     The read data if the read was sucessful, std::nullopt otherwise.
 */
std::optional<std::vector<fpgaInterfaceConnection::mmData_t>>  fpgaInterfaceConnection::read(
    const std::string& module, const reg_t& start, const reg_t& stop) noexcept
{
    try
    {
        if(rapidjson::Document workingDoc; !workingDoc.Parse(fpgaRequestModuleReadRangeJsonStr.c_str(),
            fpgaRequestModuleReadRangeJsonStr.size()).HasParseError())
        {
            if(helpers::jsonSet(workingDoc, module, "module") &&
                helpers::jsonSet(workingDoc, helpers::types::toHexString(start), "startHexOffset") &&
                helpers::jsonSet(workingDoc, helpers::types::toHexString(stop), "endHexOffset"))
            {
                if(helpers::sendReqRep(workingDoc, reqRepSock.get(), [this](){ setupSocket(); }))
                {
                    if(auto result{helpers::jsonGetVec<std::string>(workingDoc, "response", "data")};
                        result.has_value())
                    {
                        std::vector<mmData_t> data;

                        std::transform(result.value().begin(), result.value().end(), std::back_inserter(data),
                            [&](const auto& str){ return helpers::types::strToNum<mmData_t>(str, 16).value_or(0); });

                        if(data.size() == ((stop - start) >> 2))
                        {
                            return data;
                        }
                    }
                }
            }
        }
    }
    catch(...)
    {}

    logger::warn(__FILE__, __FUNCTION__, "***ERROR - Failed to get FPGA Module Register State!!!");

    return std::nullopt;
}

/**
 * @brief      Set's up the ZMQ Socket
 */
void fpgaInterfaceConnection::setupSocket()
{
    if(reqRepSock)
    {
        reqRepSock.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    reqRepSock = helpers::getSockConnect(ctx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getFpgaHardwareInterfaceAPI"));
}
