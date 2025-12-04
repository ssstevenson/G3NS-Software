/*
*  Created on: January 23, 2016
*      Author: Marc Obbad
* 
*       edits: Steve Stevenson
*/
#include <iostream>
#include <sys/resource.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#include "conf.h"

using namespace std;

LoadConfig::LoadConfig( string filename):
        fileName()
{
   fileName = filename;
}

LoadConfig::~LoadConfig()
{
}

int  LoadConfig::loadConfig(SosConfSet_T  *params, int  paramLEn)
{

    FILE                *fd = NULL;
    string              alertmsg;
    int i = 0;
    char   *paramInfo;
    size_t  fileSize = 0;
    int ret = 0;


    fd = fopen(fileName.c_str(), "rb");
    if(fd != NULL)
    {
        printf("[%s]  Using SOS Config file %s \n", __FUNCTION__, fileName.c_str());
    }
    else
    {
        alertmsg = "ERROR:  Configuration FILE MISSING";
        printf("\n\n %s() ==> %s (file=%s) \n",  __FUNCTION__, alertmsg.c_str(), fileName.c_str());
        printf("\n[%s] ==> Config file %s \n", __FUNCTION__, fileName.c_str());
        ret = -2;
    }

    if (ret == 0 )
    {
        struct stat st;
        if ( stat(fileName.c_str(), &st) == 0 )
        {
            fileSize = static_cast<size_t>(st.st_size );
        }

        paramInfo = new char [fileSize +1];
        memset(paramInfo, 0, fileSize + 1);

        // Read File in buffer
        int len = static_cast<int>( fread ( paramInfo, 1,fileSize , fd ) );
        if (len <= 0)
        {
            alertmsg = "ERROR: sos Configuration FILE Invalid";
            printf("\n\n %s() ==> %s ",  __FUNCTION__, alertmsg.c_str());
            delete [] paramInfo;
            fclose (fd);
            return -3;
        }


        int paramSize = paramLEn;

        bool valid;

        for (i=0 ; i < paramSize ; i++)
        {
            if (params[i].paramName[0] == '\0')
                break;

            float valf;
            int vali;
            string str;
            valid = getParamFromMsg(paramInfo, params[i].paramName, params[i].type,vali,  valf, str);
            if (valid )
            {
                switch ( params[i].type)
                {
                    case TYPE_NUMBER:
                        * ( static_cast<int*>(params[i].paramValue))  = vali;
                        cout << " Parameter: " << params[i].paramName << "------>" <<  vali << endl;
                        break;

                    case TYPE_FLOAT:
                        * ( static_cast<float*>(params[i].paramValue))  = valf;
                        break;
                    case TYPE_STRING:
                        * (static_cast<string*>(params[i].paramValue))  = str;
                        break;
                    case TYPE_BOOLEAN:
                        string strue = "true";
                        string  sfalse ="false";
                        unsigned int val = 1;
                        if (str == sfalse )
                        {
                            val = 0;
                           
                        }
                        * (static_cast<unsigned int*>(params[i].paramValue))  = val;
                       
                        cout << " Parameter: " << params[i].paramName << "------>" <<  val<< endl;
                       break;
                }
            }
        }

        delete [] paramInfo;
        fclose(fd);
    }

    return ret;

}


bool LoadConfig::getParamFromMsg(string msg, string info_req, int type , int &vali, float &valf, string &vals)
{

    string              ss1;      // sub str tmp storage
    string              tag;
    size_t              p1, p2, p3, p4;


    bool valid = true;
    tag = info_req;
    info_req += "=";


    if((msg.find(info_req) != string::npos))
    {
        p1 = msg.find(info_req);
        p2 = msg.find("=", p1);
        p3 = msg.find(";", p2);
        p4 = p3 - (p2+1);
        ss1 = msg.substr(p2+1, p4);
        vali = atoi(ss1.c_str());
        if ( (TYPE_FLOAT == type ) || (TYPE_NUMBER == type))
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

