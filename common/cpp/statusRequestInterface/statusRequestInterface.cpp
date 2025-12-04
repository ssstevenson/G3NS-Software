/*!
* \file
* \brief Status Processor Data Request Support Class
*
-------------------------------------------------------------------------------

 Name        : statusRequestInterface.cpp

 Author      : Steve Stevenson

 Version     : v 00.01.00 [ Initial Pre Build ]

 Description : Status Processor Data Request Support Class Definitions

-------------------------------------------------------------------------------

Copyright   : (C) Copyright 2018 - 2019 Empower RF Systems

-------------------------------------------------------------------------------
*
*
*
*
*/
#include "statusRequestInterface.h"

#include <thread>

using namespace empower;

/**
 * @brief      Constructs a new instance.
 *
 * @param      zmqCtx  The zmq context
 */
statusRequestInterface::statusRequestInterface(zmq::context_t& zmqCtx):
    ctx{zmqCtx},
    reqRepSock{helpers::getSockConnect(ctx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getStatusProcessorAPI"))},
    msgIf{zmqCtx},
    statusRequestJsonStr{}
{
    statusRequestJsonStr = msgIf.getMessageStr("getStatusItem", true).value_or("");
}

/**
 * @brief      Sets up the ZMQ Socket
 */
void statusRequestInterface::setupSocket()
{
    if(reqRepSock)
    {
        reqRepSock.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    reqRepSock = helpers::getSockConnect(ctx, zmq::socket_type::req,
        enums::zmqConnections::getSocket("getStatusProcessorAPI"));
}
