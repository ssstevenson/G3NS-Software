#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <syslog.h>
#include <algorithm>
#include <cctype>
#include <cmath>


using namespace std;
using namespace rapidjson;

class JsonFormater {

public:
    JsonFormater() {};
    ~JsonFormater() {};

    // For  messages like LD<nn> ....
    // Create JSON like this
    // {"messageType":"setAlcLevel","sequenceNumber":0, "value": 55.5 }
    string createSimpleMessage(const string& message, int sequence, const float value = 0.0, const string& reply = "")
    {
        string jsonString = "";
        if (!message.empty() ) {
            Document jsonDoc;
            jsonDoc.SetObject();


            Document::AllocatorType& allocator = jsonDoc.GetAllocator();

            jsonDoc.AddMember("messageType", Value().SetString(message.c_str(), allocator), allocator);
            jsonDoc.AddMember("sequenceNumber", sequence, allocator);
            float roundedValue = std::round(value * 100.0f) / 100.0f;
            jsonDoc.AddMember("value", roundedValue, allocator);
            if (!reply.empty()) {
                jsonDoc.AddMember("result", Value().SetString(reply.c_str(), allocator), allocator);
            }


            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            jsonDoc.Accept(writer);

            jsonString =  buffer.GetString();
        }
        return jsonString;
    }

    // Create JSON like this
    // {"messageType":"saveConfig","sequenceNumber":0, "value": 9,"commandParameters":["1","R","S","CW","S","A","D","0","55.50"]}
    string createJsonWithParams(const string& message, int sequence, std::vector<string> params)
    {
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        // Add command and sequenceNumber
        document.AddMember("messageType", Value().SetString(message.c_str(), allocator), allocator);
        document.AddMember("sequenceNumber", sequence, allocator);

        document.AddMember("value", params.size(), allocator);
        // Create commandParameters array
        Value commandParams(kArrayType);
        for ( string str: params)
        {
            commandParams.PushBack(Value().SetString(str.c_str(), allocator), allocator);
        }

        // Add the array to the document
        document.AddMember("commandParameters", commandParams, allocator);

        // Stringify the document
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        document.Accept(writer);

        return buffer.GetString();
    }


    bool parseMessage(const string& jsonStr, string &command, int &sequenceNumber, float &value, string &vals)
    {
        Document doc;

        // Parse the JSON string
        if (doc.Parse(jsonStr.c_str()).HasParseError())
        {
            return false;
        }
        // Extract values into local variables
        if (doc.HasMember("messageType") && doc["messageType"].IsString())
        {
            command = doc["messageType"].GetString();
            trim(command);
        }

        if (doc.HasMember("sequenceNumber") && doc["sequenceNumber"].IsInt())
        {
            sequenceNumber = doc["sequenceNumber"].GetInt();
        }

        if (doc.HasMember("value") && doc["value"].IsNumber())
        {
            value = doc["value"].GetFloat();
        }

        if (doc.HasMember("result") && doc["result"].IsString())
        {
            vals = doc["result"].GetString();
        }
        if (doc.HasMember("commandParameters") && doc["commandParameters"].IsArray()) {
            const Value& commandParams = doc["commandParameters"];

            for (SizeType i = 0; i < commandParams.Size(); ++i) {
                if (i > 0) {
                    vals += ",";
                }
                vals += commandParams[i].GetString();
            }
        }
        return true;
    }

    void trim(std::string& s) {
        // Trim from start
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));

        // Trim from end
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

};

