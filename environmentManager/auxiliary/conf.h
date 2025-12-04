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

#include <iostream>
#include <iostream>
#include <string>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>


using namespace std;

enum CONFIG_TYPE{ TYPE_NUMBER, TYPE_FLOAT, TYPE_STRING, TYPE_BOOLEAN };

typedef struct
{
    const char  *paramName;
    int   type;  //0= int  //1= float, 2=std::string, 3 boolaen
    void  *paramValue;
} SosConfSet_T;

class LoadConfig
{
    public:
        LoadConfig( string filename);
        ~LoadConfig();
        bool getParamFromMsg(std::string msg, std::string info_req, int type , int &vali, float &valf, std::string &vals);
        int  loadConfig(SosConfSet_T  *params, int  paramLEn);
        string fileName;
};

#endif /* __GEN_CONFIG_H__ */
