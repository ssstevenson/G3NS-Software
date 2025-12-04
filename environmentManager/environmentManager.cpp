/*!
* \file
*
* \brief Environmental Manager Main()
*
*
-------------------------------------------------------------------------------

 Name        : environmentManager.cpp

 Author      : Steve Stevenson

 Version     : v 00.90.00 [ Initial Dev Build ]

 Description : Environmental Manager Main()

-------------------------------------------------------------------------------

Copyright   : (C) Copyright 2018 - 2020 Empower RF Systems

-------------------------------------------------------------------------------
*
*
*
*
*/

#include <heartbeatResponse/heartbeatResponse.h>
#include <helpers/mainHelper.h>
#include <logger/logger.h>
#include "parser/envMgrParser.h"

int main()
{
    zmq::context_t zmqCtx{1};
    empower::configManagerConnection configIf{zmqCtx};
    empower::logger::getInst().setup(zmqCtx, PROCESS_NAME);
    empower::helpers::poller poller;
    empower::envMgrParser parser{zmqCtx, poller, configIf};
    empower::heartbeatResponse hb{zmqCtx, poller, configIf, PROCESS_NAME};
    empower::helpers::run(poller);

    return EXIT_FAILURE;
}
