#pragma once
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <rfManagerController.h>
#include <unordered_map>
#include <syslog.h>
#include <helpers/debug.h>
#include <FpgaIO/fpgaHal.h>

#define M2M_TO_RFMANAGER  "ipc:///tmp/m2mToRfManager"
#define STATUS_TO_M2M     "ipc:///tmp/statusToM2m"

using namespace std;
using namespace empower;
//
//    Every Generic  Message is sent as simple JSON Command
//   {
//     "messageType": "setmgclevel",
//      "sequenceNumber": 0,
//      "value": 55.5
//    }

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
//      "result" :" anything or nothing "
//    }
//
////////// Some Command has a list of parameters, we put them in an array
//   {
//     "messageType": "saveConfig",
//      "sequenceNumber": 0,
//      "value": 9,  // number of parameters, used for validation
//      "commandParameters": ["1", "R", "S", "CW", "S", "A", "D", "22", "55.50"]
//    }

//
//   {
//     "messageType": "getConfig",
//      "sequenceNumber": 0,
//      "value": 9,  // used for validation as expect 9 paramet
//      "result": ["1", "R", "S", "CW", "S", "A", "D", "22", "55.50"]
//    }

//
//    Response from RF manager to SS  should be
//
//
//    {
//     "messageType": "status",
//      "sequenceNumber": 0,
//      "value": rack.slot or 0.0
//      "result" :"OFI XX xxxx "   M2M should not know Hardware interanl errorrs, that is RF manager
//    }
//    Response to user is depending on rack.slot
//     if rack.slot is 0 or 0.0          --->SS OFI XX xxxx
//     if rack.slot is 0.n (ex. 0.1)    ---->SS 1 OFI XX xxxx
//     if rack.slot is  r.n (ex. 1.3)   ---->SS 1.3 OFI XX xxxx

class M2MZeroMQ {
    public:
        M2MZeroMQ(std::shared_ptr<zmq::context_t> ctx):m2mZeroMQctx(ctx),m2mRepSock{},m2mPushSock{}{
            setZmqSockets();
        }
        M2MZeroMQ(const M2MZeroMQ &) = delete;
        M2MZeroMQ& operator=(const M2MZeroMQ&)= delete;

        void sendReply(const string msg)
        {
            sendMessage(m2mRepSock, msg);
        }

        void sendStatus(const string msg)
        {
            sendMessage(m2mPushSock, msg);
        }
        string getM2MCommand()
        {
            return  (readM2MCommand());
        }
        //
        ~M2MZeroMQ()
        {
            m2mRepSock.close();
            m2mPushSock.close();
        }

    private:

        std::shared_ptr<zmq::context_t> m2mZeroMQctx ;
        zmq::socket_t  m2mRepSock;
        zmq::socket_t  m2mPushSock;

        //
        void setZmqSockets()
        {
            m2mRepSock   =   zmq::socket_t (*m2mZeroMQctx, ZMQ_REP);
            m2mPushSock   = zmq::socket_t (*m2mZeroMQctx, ZMQ_PUSH);
            //m2mRepSock.setsockopt(ZMQ_RCVTIMEO, 500);
            sockBind(m2mPushSock,STATUS_TO_M2M);
            sockBind(m2mRepSock, M2M_TO_RFMANAGER);
        }
        //
        void sendMessage(zmq::socket_t  &sock, const string msg)
        {
            sock.send(zmq::buffer(msg), zmq::send_flags::none);
        }

        bool sockBind(zmq::socket_t &sock, const std::string &addr) {
            try {
                sock.bind(addr);
                return true;
            } catch (const zmq::error_t& ex) {
                 syslog(LOG_ERR, "Exception in sockBind(): %s", ex.what());
                return false;
            }
        }
        string readM2MCommand()
        {
            try {
                zmq::message_t request;
                string recv_str;
                if (m2mRepSock.recv(request, zmq::recv_flags::none) )
                {
                    recv_str= std::string(static_cast<char*>(request.data()), request.size());
                    if (!recv_str.empty() ) {
                        dbgprintf( "[%s]received message: %s \n", __FUNCTION__,recv_str.c_str());
                        syslog(LOG_INFO, "[%s]received message: %s", __FUNCTION__,recv_str.c_str());
                    }
                }
                return recv_str;
            } catch (const std::exception& ex) {
                syslog(LOG_ERR, "Exception in readM2MCommand(): %s", ex.what());
                return "";
            }
        }

};

class M2MProcessor : public M2MZeroMQ
{
    public:
        M2MProcessor(rfManagerController &rfController, std::shared_ptr<zmq::context_t> ctx, FpgaHal &fpgaHal):
            M2MZeroMQ(ctx),
            stop_m2mthread(false),
            m2m_thread(),
            rfCtrl(rfController),
            cmdAPI{},
            fpga(fpgaHal)
        {
            initCommand();
        }

        M2MProcessor(const M2MZeroMQ &) = delete;
        M2MProcessor& operator=(const M2MZeroMQ&)= delete;

        //
        void m2mRun();

        void stopM2M() { stop_m2mthread = true; }
        void startM2MThread()
        {
            stop_m2mthread=false;
            m2m_thread = std::thread(&M2MProcessor::m2mRun, this);
        };

        ~M2MProcessor() {
            // cleanup
            stop_m2mthread = true;
            if (m2m_thread.joinable()) {
                m2m_thread.join();
            }
        }

        class MSC
        {
            public:
                MSC():index(0),ctrl{},startup{},modulation{},detectionMode{},power{},unit{},par(0),outputLevel(0.0){};
                int index;
                std::string ctrl;
                std::string startup;
                std::string modulation;
                std::string detectionMode;
                std::string power;
                std::string unit;
                int par;
                float outputLevel;
        };

        std::thread &getM2MThread() { return m2m_thread; }

    private:
        bool stop_m2mthread;
        std::thread m2m_thread;
        rfManagerController &rfCtrl;
        using func = std::function< void ( float &, std::string &)>;
        std::unordered_map<std::string, func>  cmdAPI;
        FpgaHal &fpga;
        void initCommand();
        //
        // API to implment zerMQ commands from M2M
        //
        void setCentralFrequency(float &val, string &msg);
        void setBandwidth(float &val, string &msg);

        void getCentralFrequency(float &val, string &msg);
        void getBandwidth(float &val, string &msg);
        void disableAutoRF(float &val, string &msg);
        void enableAutoRF(float &val, string &msg);
        void getAutoRFState(float &val, string &msg);

        void setFastDetectionMode(float &val, string &msg);
        void setSlowDetectionMode(float &val, string &msg);
        void getDetectionModeState(float &val, string &msg);
        void faultClear(float &val, string &msg);

        void setAlcLevel(float &val, string &msg);
        void getAlcLevel(float &val, string &msg);
        void setAgcLevel(float &val, string &msg);
        void getAgcLevel(float &val, string &msg);
        void setMgcLevel(float &val, string &msg);
        void getMgcLevel(float &val, string &msg);




        void coldStandBy(float &val, string &msg);
        void online(float &val, string &msg);
        void hotStandBy(float &val, string &msg);
        void getModeState(float &val, string &msg);

        void getForwardPower(float &val, string &msg);
        void getInputPower(float &val, string &msg);
        void getReversePower(float &val, string &msg);
    //
        void getVswrState(float &val, string &msg);
        void switchAntenna(float &val, string &msg);
        void getCurrentAntenna(float &val, string &msg);
        void SystemStatus(float &val, string &msg);

        void setUnitsToDBm(float &val, string &msg);
        void setUnitsToWatts(float &val, string &msg);
        void getUnits(float &val, string &msg);
        void getSystemTemp(float &val, string &msg);
        void setTRSwitch(float &val, string &msg);
        void getTRSwitch(float &val, string &msg);

        void saveConfig(float &val, string &msg);
        void getConfig(float &val, string &msg);
        // Helpers functions, may move to RF Manager

        void setRfOnOff( bool on ) {
            fpga.setRfOnOff (on );
        }

        void setPsuOnOff( bool on ) {
            fpga.setPsuOnOff (on );
        }

        bool getRfState() {
            return (fpga.getRfState() );
        }

        bool getPsuState() {
            return (fpga.getPsuState() );
        }
        ///////////////////////////////////////
        ///////////// Helpers Functions ///////
        //////////////////////////////////////
        void  processMSC(MSC &msc);
        // Form System Status without SS in front i
        // example "ONL 00 0000"
        string formSystemStatus();
        // From MSC messge in response to MSC?
        // example  (1, R, S, CW, S, A, D, 255.0, 55.50)
        string formMSC();

};
