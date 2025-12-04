/** @file */
/* =========================================================================*
* Copyright (C)Empower RF, 2015.  All rights reserved.                      *
*                                                                           *
*                                                                           *
* ==========================================================================*
*
*****************************************************************************
* Module Name: conf.h
*
* Functional Description:
*
* This file defines the config...Generic
*
* Version History:
*
* Version  Date        Author         Description
* -------  ----------  ----------     --------------------------------
* 0      03/12/16   Marc Obbad       Initial Release.
* 1      10/25/2016   M. Obbad       Read configuration parameters from a file
*
*****************************************************************************/
#ifndef __GEN_CONFIG_H__
#define __GEN_CONFIG_H__

#include <string>

class LoadConfig
{
public:
    enum class CONFIG_TYPE: std::size_t { NUMBER_T = 0, FLOAT_T = 1, STRING_T = 2, BOOLEAN_T = 3, NONE_T = 4 };

    typedef struct
    {
        const char *paramName;
        CONFIG_TYPE type;
        void *paramValue;
    } SosConfSet_T;

private:
    std::string fileName;

public:
    LoadConfig(const std::string& filename);
    bool getParamFromMsg(std::string msg, std::string info_req, CONFIG_TYPE type, int &vali, float &valf, std::string &vals);
    int load(SosConfSet_T *params, int paramLEn);
};

#endif /* __GEN_CONFIG_H__ */
