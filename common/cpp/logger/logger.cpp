#include "logger.h"

#include <helpers/zmqConnectionNames.h>
#include <helpers/zmqHelpers.h>
#include <messageFactoryConnection/messageFactoryConnection.h>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 */
logger::logger():
    logPostJsonStr{},
    pName{""},
    severityStrMap{
        {severity_t::verbose, "verbose"},
        {severity_t::debug, "debug"},
        {severity_t::info, "info"},
        {severity_t::warn, "warn"},
        {severity_t::critical, "critical"}
    },
    setupDone{false},
    sock{}
{}

/**
 * @brief      Gets the instance.
 *
 * @return     The instance.
 */
logger& logger::getInst()
{
    static logger inst;
    return inst;
}

/**
 * @brief      This sets up the ZMQ socket connection to the logger
 *
 * @param      zmqCtx  The zmq context
 * @param[in]  name    The socket name
 */
void logger::setup(zmq::context_t& zmqCtx, const std::string& name)
{
    if(!setupDone)
    {
        sock = helpers::getSockConnect(zmqCtx, zmq::socket_type::push, enums::zmqConnections::getSocket("getLogger"));
        pName = name;
        messageFactoryConnection msgFactory(zmqCtx);
        logPostJsonStr = msgFactory.getMessageStr("getLogPostRequest", false).value_or("");
        setupDone = true;
    }

    genLog(__FILE__, __FUNCTION__, "Process Starting...", severity_t::info);
}

/**
 * @brief      Gennerates a log message
 *
 * @param[in]  file  Source File
 * @param[in]  func  Source Function
 * @param[in]  msg   Log Message
 * @param[in]  sev   Log Severity
 */
void logger::genLog(const std::string& file, const std::string& func, const std::string& msg, const severity_t sev)
{
    std::string fname{file.substr(file.find_last_of("/") + 1)};
    std::string sendMsg{fname + std::string(" [ ") + std::string(func) + std::string("() ]: ") + msg};

    getInst().logMessage(sendMsg, sev);
}

void logger::genLogRaw(const std::string& json)
{
    getInst().logMessageRaw(json);
}

/**
 * @brief      Generate a Verbose Log Message
 *
 * @param[in]  file  Source File
 * @param[in]  func  Source Function
 * @param[in]  msg   Log Message
 */
void logger::verbose(const std::string& file, const std::string& func, const std::string& msg)
{
    genLog(file, func, msg, severity_t::verbose);
}

/**
 * @brief      Generate a Debug Log Message
 *
 * @param[in]  file  Source File
 * @param[in]  func  Source Function
 * @param[in]  msg   Log Message
 */
void logger::debug(const std::string& file, const std::string& func, const std::string& msg)
{
    genLog(file, func, msg, severity_t::debug);
}

/**
 * @brief      Generate a Info Log Message
 *
 * @param[in]  file  Source File
 * @param[in]  func  Source Function
 * @param[in]  msg   Log Message
 */
void logger::info(const std::string& file, const std::string& func, const std::string& msg)
{
    genLog(file, func, msg, severity_t::info);
}

/**
 * @brief      Generate a Warn Log Message
 *
 * @param[in]  file  Source File
 * @param[in]  func  Source Function
 * @param[in]  msg   Log Message
 */
void logger::warn(const std::string& file, const std::string& func, const std::string& msg)
{
    genLog(file, func, msg, severity_t::warn);
}

/**
 * @brief      Generate a Critical Log Message
 *
 * @param[in]  file  Source File
 * @param[in]  func  Source Function
 * @param[in]  msg   Log Message
 */
void logger::critical(const std::string& file, const std::string& func, const std::string& msg)
{
    genLog(file, func, msg, severity_t::critical);
}

/**
 * @brief      Logs a message.
 *
 * @param[in]  msg   The message
 * @param[in]  sev   The severity
 *
 * @return     True if the message was successfully logged, False otherwise.
 */
bool logger::logMessage(const std::string& msg, const severity_t sev)
{
    if(setupDone)
    {
        if(rapidjson::Document workingDoc;
           !workingDoc.Parse(logPostJsonStr.c_str(), logPostJsonStr.size()).HasParseError())
        {
            if(helpers::jsonSet(workingDoc, severityStrMap.at(sev), "criticality") &&
                helpers::jsonSet(workingDoc, pName, "processName") &&
                helpers::jsonSet(workingDoc, msg, "message"))
            {
                return helpers::sendJsonDoc(workingDoc, sock.get());
            }
        }
    }

    return false;
}

/**
 * @brief      Logs a Raw Message JSON String.
 *
 * @param[in]  json  The json string
 *
 * @return     True if the message was sucessfully logged, False otherwise.
 */
bool logger::logMessageRaw(const std::string& json)
{
    return helpers::sendString(json, sock.get());
}
