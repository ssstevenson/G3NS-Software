#ifndef MAIN_HELPER_H_
#define MAIN_HELPER_H_

#include <cassert>
#include <functional>
#include <utility>
#include <vector>
#include <zmq.hpp>

namespace empower::helpers
{
    using socket_handle_t = int;
    using pollHandles = std::vector<socket_handle_t>;
    using pollItem = std::vector<zmq::pollitem_t>;
    using pollCall = std::vector<std::function<void()>>;
    using poller = std::tuple<pollHandles, pollItem, pollCall>;

    /**
     * @brief      Gets the socket handle.
     *
     * @return     The socket handle.
     */
    inline static socket_handle_t getSocketHandle()
    {
        static socket_handle_t handles{0};
        return ++handles;
    }

    /**
     * @brief      Infinite RUN loop
     *
     * @param      pollMe  The poller data structure
     */
    [[noreturn]] inline void run(poller& pollMe)
    {
        const pollHandles& pollHandlesRef{std::get<0>(pollMe)};
        pollItem& pollItemRef{std::get<1>(pollMe)};
        const pollCall& pollCallRef{std::get<2>(pollMe)};

        assert(((pollItemRef.size() == pollCallRef.size()) && (pollItemRef.size() == pollHandlesRef.size())) &&
            "Poller Vectors different sizes!!!");

        for(;;)
        {
            if(zmq::poll(pollItemRef) > 0)
            {
                for(const auto& pollIdx: pollItemRef)
                {
                    if(pollIdx.revents & ZMQ_POLLIN)
                    {
                        const auto callIdx{static_cast<std::size_t>(&pollIdx - &*pollItemRef.begin())};
                        if(pollCallRef.at(callIdx))
                        {
                            pollCallRef.at(callIdx)();
                        }
                    }
                }
            }
        }
    }
}

#endif
