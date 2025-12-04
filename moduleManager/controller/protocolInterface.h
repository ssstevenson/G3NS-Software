#ifndef PROTOCOL_INTERFACE_H_
#define PROTOCOL_INTERFACE_H_

#include <array>
#include <helpers/rfLayoutTypes.h>
#include <serialInterfaceConnection/serialInterfaceConnection.h>
#include <statusUpdateInterface/statusUpdateInterface.h>
#include <vector>

namespace empower
{
    /**
     * @brief      This class describes a protocol interface for a Pallet or Driver Command Set
     */
    class protocolInterface
    {
    public:
        using pmod_addr_t = helpers::types::pmod_addr_t;
        using updateIfData_t = helpers::types::updateIfData_t;
        using data_set_t = helpers::types::data_set_t;
        using ser_msg_t = serialInterfaceConnection::ser_msg_t;
        using pmod_data_t = helpers::types::ser_data_t;
        using timeout_t = helpers::types::timeout_t;
        using poll_err_cnt_t = std::size_t;

    protected:
        static constexpr auto serMsgStartTimeout = timeout_t{20};
        static constexpr auto serMsgCharTimeout = timeout_t{5};
        static constexpr poll_err_cnt_t pollingRetryCnt = 3;

        pmod_addr_t curAddr;
        std::string deviceKey;
        data_set_t dataset;
        serialInterfaceConnection& serIf;
        statusUpdateInterface& updateIf;
        timeout_t timeout;
        poll_err_cnt_t pollErrCnt;

    public:
        protocolInterface(statusUpdateInterface& u_, serialInterfaceConnection& s_,
                const std::string& k_, const pmod_addr_t a_, const timeout_t& t_):
            curAddr{a_},
            deviceKey{k_},
            dataset{},
            serIf{s_},
            updateIf{u_},
            timeout{t_},
            pollErrCnt{0}
        {}

        virtual ~protocolInterface(){}
        virtual void poll() = 0;
        virtual void reset() = 0;
        virtual void faultClear() = 0;
        operator pmod_addr_t() const { return curAddr; }
    };
}

#endif
