#pragma once
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <syslog.h>
#include <helpers/debug.h>
#include <map>
#include <configManagerConnection/configManagerConnection.h>
#include <helpers/mainHelper.h>
#include <helpers/zmqConnectionNames.h>

#define M2M_TO_RFMANAGER  "ipc:///tmp/m2mToRfManager"
#define M2M_TO_PSUMANAGER  "ipc:///tmp/m2mToPSUManager"
#define M2M_TO_ENVMANAGER  "ipc:///tmp/m2mToENVManager"
#define STATUS_TO_M2M      "ipc:///tmp/statusToM2m"

using namespace std;
using namespace empower;
using namespace empower::enums;
//
//    Every Message is sent as simple JSON Command
//////{
//     "messageType": "setmgclevel",
//      "sequenceNumber": 0,
//      "value": 55.5
//    }
//
//   or
//    {
//     "messageType": "getmgclevel",
//      "sequenceNumber": 0,
//      "value": 0.0
//   }
//
// response is something like
//    {
//     "messageType": "getmgclevel",
//      "sequenceNumber": 0,
//      "value": 55.5
//      "result" :" complete" "?" or nothing "
//    }
//
//
//    Response from RF manager to SS  should be
//
//
//    {
//     "messageType": "status",
//      "sequenceNumber": 0,
//      "value": rack.slot or 0.0
//      "result" :" OFI XX xxxx "    M2M should not know Hardware interanl errorrs, that is RF manager
//    }
//
class M2MZeroMQ {
    public:
        M2MZeroMQ():m2mZeroMQctx{},m2mMQReqRepRF{},m2mZeroMQPoll{},m2mMQReqRepPSU{},m2mMQReqRepENV{},msgIf{},Target{}
        {
        };

        ~M2MZeroMQ()
        {
            for (auto* sock : clientSocketArray) {
                if (sock) sock->close();
            }
            m2mZeroMQPoll.close();
        }

        enum class Module : std::size_t {
            RF_MANAGER,
            ENV_MANAGER,
            PSU_MANAGER,
            MODULE_MAX
        };

        static constexpr std::array<const char*, static_cast<std::size_t>(Module::MODULE_MAX)> ModuleS {
            {
           "RF Manager",
           "ENV Manager",
           "PSU Manager"
            }
        };

        static constexpr std::array<const char*, static_cast<std::size_t>(Module::MODULE_MAX)> SocketID
        {      {
                M2M_TO_RFMANAGER,
                M2M_TO_ENVMANAGER,
                M2M_TO_PSUMANAGER
            }
        };

        void setupNotificationSocket() noexcept
        {
            m2mZeroMQPoll.close();
            m2mZeroMQPoll = zmq::socket_t(*m2mZeroMQctx, ZMQ_PULL);

            if ( socketBind(m2mZeroMQPoll, STATUS_TO_M2M) )
            {
                int timeout = 500; // ms
                m2mZeroMQPoll.setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
            }
        }

        void setZmqSockets(const std::shared_ptr<zmq::context_t>& ctx)
        {
            m2mZeroMQctx  = ctx;
            setupNotificationSocket();
            // iterate safely with no int casting
            for (std::size_t i = 0; i <  static_cast<std::size_t>(Module::MODULE_MAX); ++i) {
                createSocket(static_cast<Module>(i));
            }
        }

        void setMsgFactoryConnection( std::shared_ptr<messageFactoryConnection > &msgIfConn)
        {
            msgIf = msgIfConn;
        }
        std::shared_ptr<zmq::context_t> getZerMQCtx() { return m2mZeroMQctx; }

        void sendReqToProcess(Module m, const string& msg, string& reply_str)
        {
            if (m >= Module::MODULE_MAX || clientSocketArray[static_cast<std::size_t>(m)] == nullptr)
            {
                syslog(LOG_ERR, "[%s] Invalid module or uninitialized socket", __FUNCTION__);
                return;
            }

            if (!sendRequest(*clientSocketArray[static_cast<std::size_t>(m)], msg, reply_str)) {
                createSocket(m);
            }
        }

        void setupSocket(std::unique_ptr<zmq::socket_t> &cmdProcessorSock) {
            if(cmdProcessorSock)
            {
                cmdProcessorSock.reset();
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
             std::string connectionName{enums::zmqConnections::getSocket("getCommandProcessorM2MTcpAPI")};
            cmdProcessorSock = helpers::getSockConnect(*m2mZeroMQctx, zmq::socket_type::req, connectionName);
        }

        //
        std::string  getSysInfo( std::string sysinfo)
        {
            std::string result;


            std::string connectionName{enums::zmqConnections::getSocket("getCommandProcessorM2MTcpAPI")};
            std::unique_ptr<zmq::socket_t> cmdProcessorSock = helpers::getSockConnect(*m2mZeroMQctx, zmq::socket_type::req, connectionName);
            if(auto jsonDocOpt{msgIf->getMessageJson(sysinfo, false)}; jsonDocOpt.has_value())
            {
                rapidjson::Document& jsonDoc{jsonDocOpt.value()};

                if(helpers::sendReqRep(jsonDoc, cmdProcessorSock.get(), [&](){ setupSocket(cmdProcessorSock); }))
                {
                    result += helpers::jsonGet<std::string>(jsonDoc, "resultPayload").value_or("");
                }

            }
            return result;
        }
        //
        std::string  getSystemModel()
        {
            std::string result;
            std::string sysinfo{"getModel"};

            return ( getSysInfo( sysinfo));
        }

       std::string  getSystemSN()
        {
            std::string result;
            std::string sysinfo{"getSerial"};

            return ( getSysInfo( sysinfo));
        }

        std::string getSystemID()
        {
            std::string result;


            std::string connectionName{enums::zmqConnections::getSocket("getCommandProcessorM2MTcpAPI")};
            std::unique_ptr<zmq::socket_t> cmdProcessorSock = helpers::getSockConnect(*m2mZeroMQctx, zmq::socket_type::req, connectionName);
            if(auto jsonDocOpt{msgIf->getMessageJson("getSystemDescription", false)}; jsonDocOpt.has_value())
            {
                rapidjson::Document& jsonDoc{jsonDocOpt.value()};

                if(helpers::sendReqRep(jsonDoc, cmdProcessorSock.get(), [&](){ setupSocket(cmdProcessorSock); }))
                {
                    result += helpers::jsonGet<std::string>(jsonDoc, "resultPayload", "modelNumber").value_or("");
                    result += " ";
                    result += helpers::jsonGet<std::string>(jsonDoc, "resultPayload", "partNumber").value_or("");
                    result += " ";
                    result += helpers::jsonGet<std::string>(jsonDoc, "resultPayload", "description").value_or("");
                }

                dbgprintf ("[%s]---> : %s\n", __FUNCTION__, result.c_str());
            }
            return result;
        }
        bool  receiveNotification( string &status) noexcept
        {
            bool  statusOk = false;
            status.clear();

            zmq_pollitem_t items[] =
            {
                { static_cast<void*>(m2mZeroMQPoll), 0, ZMQ_POLLIN, 0 }
            };

            int timeout = 500;
            int rc = zmq::poll(items, 1, timeout);

            if (rc == -1)
            {
                // close and restart socket
                //syslog(LOG_INFO, "[%s]---> Poll Failed Closing socket  \n", __FUNCTION__ );
                setupNotificationSocket();
            }
            else
            {
                if (rc > 0)
                {

                    // Message is available to be received
                    zmq::message_t message;
                    zmq::detail::recv_result_t received = m2mZeroMQPoll.recv(message, zmq::recv_flags::none);

                    if (received > 0)
                    {
                        // Successfully received a message
                        status = std::string(static_cast<char*>(message.data()), message.size());
                        statusOk = true;
                    }
                    else
                    {
                        //syslog(LOG_INFO, "[%s]---> m2mZeroMQPoll.recv  Timed Out or Error  \n", __FUNCTION__ );
                        setupNotificationSocket();
                    }
                }

            }

            return statusOk;
        }
   private:

        std::shared_ptr<zmq::context_t> m2mZeroMQctx ;
        zmq::socket_t  m2mMQReqRepRF;
        zmq::socket_t  m2mZeroMQPoll;
        zmq::socket_t m2mMQReqRepPSU;
        zmq::socket_t m2mMQReqRepENV;
        // messageFactoryConnection &msgIf;
        std::shared_ptr<messageFactoryConnection > msgIf;
        std::map<Module, zmq::socket_t * > Target;

        //
        //
        //
        std::array<zmq::socket_t*, static_cast<std::size_t>(Module::MODULE_MAX)> clientSocketArray
        {
            {
            &m2mMQReqRepRF,
            &m2mMQReqRepPSU,
            &m2mMQReqRepENV
            }
        };
        //
        //
        //
        void createSocket( Module m )
        {
            if (m < Module::MODULE_MAX)
            {
                auto it = Target.find(m);
                if (it != Target.end() ) {
                    //socket is open ---close it
                    (*it->second).close();
                }
                zmq::socket_t &socket = *clientSocketArray[static_cast<std::size_t> (m)];

                socket   =   zmq::socket_t (*m2mZeroMQctx, ZMQ_REQ);
                //socket.setsockopt(ZMQ_RCVTIMEO, 1000);
                int timeout = 1000; // ms
                socket.setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
                socket.setsockopt(ZMQ_SNDTIMEO, &timeout, sizeof(timeout));
                Target[m] = &socket;
                socket.connect(SocketID[static_cast<std::size_t> (m)]);
            }
        }

        //
        //
        //
        void addTarget(Module m, zmq::socket_t& s) {
            Target[m] = &s;
        }
        //
        //
        //
        //
        bool sendRequest(zmq::socket_t& sock, const string& msg, string& reply_str)
        {
            zmq::message_t request(msg.size());
            memcpy(request.data(), msg.data(), msg.size());

            auto sent = sock.send(request, zmq::send_flags::none);
            if (!sent) {
                syslog(LOG_ERR,"[%s] Error sending: %s", __FUNCTION__, msg.c_str());
                return false;
            }

            syslog(LOG_INFO,"[%s] Sent: %s", __FUNCTION__, msg.c_str());
            dbgprintf("[%s] Sent: %s\n", __FUNCTION__, msg.c_str());

            zmq::message_t reply;
            auto recv_ok = sock.recv(reply, zmq::recv_flags::none);
            if (!recv_ok) {
                syslog(LOG_ERR,"[%s] No reply to: %s", __FUNCTION__, msg.c_str());
                dbgprintf("[%s] No reply to: %s\n", __FUNCTION__, msg.c_str());
                return false;
            }

            reply_str = std::string(static_cast<char*>(reply.data()), reply.size());
            syslog(LOG_INFO,"[%s] Received: %s", __FUNCTION__, reply_str.c_str());
            dbgprintf("[%s] Received: %s\n", __FUNCTION__, reply_str.c_str());

            return true;
        }

        //
        //
        //

        bool socketBind(zmq::socket_t &sock, const std::string &addr) {
            try {
                sock.bind(addr);
                return true;
            } catch (const zmq::error_t& e) {
                std::cerr << "Bind failed: " << e.what() << std::endl;
                return false;
            }
        }


};

