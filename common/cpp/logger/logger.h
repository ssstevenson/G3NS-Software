#ifndef LOGGER_H_
#define LOGGER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      This class describes a logger.
     */
    class logger
    {
    public:
        /**
         * @brief      This class describes logging severity.
         */
        enum class severity_t
        {
            verbose, debug, info, warn, critical
        };

    private:
        std::string logPostJsonStr;
        std::string pName;
        const std::unordered_map<severity_t, std::string> severityStrMap;
        bool setupDone;
        std::unique_ptr<zmq::socket_t> sock;

    public:
        logger();
        static logger& getInst();
        void setup(zmq::context_t& zmqCtx, const std::string& name);
        static void genLog(const std::string& file, const std::string& func, const std::string& msg,
            const severity_t sev);
        static void genLogRaw(const std::string& json);
        static void verbose(const std::string& file, const std::string& func, const std::string& msg);
        static void debug(const std::string& file, const std::string& func, const std::string& msg);
        static void info(const std::string& file, const std::string& func, const std::string& msg);
        static void warn(const std::string& file, const std::string& func, const std::string& msg);
        static void critical(const std::string& file, const std::string& func, const std::string& msg);

    private:
        logger(const logger& other) = delete;
        logger& operator=(const logger& other) = delete;
        logger(logger&& other) = delete;
        logger& operator=(logger&& other) = delete;

        bool logMessage(const std::string& msg, const severity_t sev);
        bool logMessageRaw(const std::string& json);
    };
}

#endif
