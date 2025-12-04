/*
*  Created on: January 23, 2016
*      Author: Marc Obbad
*/
#include <cstring>
#include <iostream>
#include <stdint.h>
#include <sys/stat.h>

#include "conf.h"

LoadConfig::LoadConfig(const std::string& filename):
    fileName{filename}
{}

int LoadConfig::load(SosConfSet_T *params, int paramLEn)
{
    FILE *fd{NULL};
    std::string alertmsg;
    int i{0};
    char *paramInfo;
    std::size_t fileSize{0};
    int ret{0};

    fd = fopen(fileName.c_str(), "rb");
    if(fd != NULL)
    {
        printf("[%s]  Using SOS Config file %s \n", __FUNCTION__, fileName.c_str());
    }
    else
    {
        alertmsg = "ERROR:  Configuration FILE MISSING";
        printf("\n\n %s() ==> %s (file=%s) \n", __FUNCTION__, alertmsg.c_str(), fileName.c_str());
        printf("\n[%s] ==> Config file %s \n", __FUNCTION__, fileName.c_str());
        ret = -2;
    }

    if (ret == 0 )
    {
        struct stat st;
        if ( stat(fileName.c_str(), &st) == 0 )
        {
            fileSize = static_cast<std::size_t>(st.st_size);
        }

        paramInfo = new char [fileSize +1];
        memset(paramInfo, 0, fileSize + 1);

        // Read File in buffer
        auto len = fread(paramInfo, 1, fileSize, fd);
        if (len <= 0)
        {
            alertmsg = "ERROR: sos Configuration FILE Invalid";
            printf("\n\n %s() ==> %s ",  __FUNCTION__, alertmsg.c_str());
            delete [] paramInfo;
            fclose (fd);
            return -3;
        }

        for (i=0 ; i < paramLEn ; i++)
        {
            if (params[i].paramName[0] == '\0')
            {
                break;
            }

            float valf;
            int vali;
            std::string str;
            if (getParamFromMsg(paramInfo, params[i].paramName, params[i].type, vali, valf, str))
            {
                switch(params[i].type)
                {
                case CONFIG_TYPE::NUMBER_T:
                    *reinterpret_cast<int*>(params[i].paramValue) = vali;
                    std::cout << " Parameter: " << params[i].paramName << "------>" <<  vali << std::endl;
                    break;

                case CONFIG_TYPE::FLOAT_T:
                    *reinterpret_cast<float*>(params[i].paramValue) = valf;
                    break;

                case CONFIG_TYPE::STRING_T:
                    *reinterpret_cast<std::string*>(params[i].paramValue) = str;
                    break;

                case CONFIG_TYPE::NONE_T:
                    break;

                case CONFIG_TYPE::BOOLEAN_T:
                    bool val = (str.compare("true") == 0);
                    *reinterpret_cast<bool*>(params[i].paramValue) = val;
                    std::cout << " Parameter: " << params[i].paramName << "------>" << std::boolalpha << val << std::endl;
                    break;
                }
            }
        }

        delete [] paramInfo;
        fclose(fd);
    }

    return ret;
}

bool LoadConfig::getParamFromMsg(std::string msg, std::string info_req, CONFIG_TYPE type, int &vali, float &valf, std::string &vals)
{
    std::string ss1;      // sub str tmp storage
    std::string tag;
    std::size_t p1, p2, p3, p4;

    bool valid{true};
    tag = info_req;
    info_req += "=";

    if((msg.find(info_req) != std::string::npos))
    {
        p1 = msg.find(info_req);
        p2 = msg.find("=", p1);
        p3 = msg.find(";", p2);
        p4 = p3 - (p2+1);
        ss1 = msg.substr(p2+1, p4);
        vali = atoi(ss1.c_str());
        if ( (CONFIG_TYPE::FLOAT_T == type ) || (CONFIG_TYPE::NUMBER_T == type))
            valf =  std::stof(ss1);
        vals = ss1;
    }
    else
    {
        printf("\n\n***ERROR: Tag %s NOT FOUND \n", tag.c_str());
        valid = false;
    }

    return (valid);
}
