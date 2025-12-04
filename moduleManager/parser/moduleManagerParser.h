#ifndef MODULE_MANAGER_PARSER_H_
#define MODULE_MANAGER_PARSER_H_

#include <helpers/mainHelper.h>
#include <parser/parserInterface.h>
#include <controller/moduleManagerController.h>

namespace empower
{
    /**
     * @brief      This class describes a module manager parser.
     */
    class moduleManagerParser:
        protected parserInterface
    {
    private:
        moduleManagerController ctrl;

    public:
        moduleManagerParser(zmq::context_t& ctx, helpers::poller& poller, configManagerConnection& config);
    };
}

#endif
