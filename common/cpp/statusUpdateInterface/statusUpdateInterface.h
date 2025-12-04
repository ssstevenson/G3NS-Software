#ifndef STATUS_UPDATE_INTERFACE_H_
#define STATUS_UPDATE_INTERFACE_H_

#include <chrono>
#include <cmath>
#include <helpers/jsonUpdateHelpers.h>
#include <helpers/types.h>
#include <helpers/zmqHelpers.h>
#include <iterator>
#include <logger/logger.h>
#include <memory>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <string>
#include <type_traits>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      This class describes a status update interface.
     */
    class statusUpdateInterface
    {
    public:
        using updateIfData_t = helpers::types::updateIfData_t;
        using timeout_t = helpers::types::timeout_t;

    private:
        zmq::context_t& ctx;
        messageFactoryConnection msgIf;
        std::unique_ptr<zmq::socket_t> sock;
        std::string statusUpdateJsonStr;

    public:
        statusUpdateInterface(zmq::context_t& zmqCtx);

        bool getStatusDoc(rapidjson::Document& response);
        bool sendStatusDoc(rapidjson::Document& jsonDoc);

        template<typename T>
        void sendStatus(const std::string& key, const std::string& label, const T& value, const timeout_t timeout);
        template<typename T>
        void sendStatus(const std::string& key, const std::string& label, const T& value, const std::string& units,
            const timeout_t timeout);

        template<typename T>
        void updateDoc(rapidjson::Document& jsonDoc, const std::string& key, const std::string& label,
            const T& value, const timeout_t timeout);
        template<typename T>
        void updateDoc(rapidjson::Document& jsonDoc, const std::string& key, const std::string& label,
            const T& value, const std::string& units, const timeout_t timeout);

        template<class T, class UnaryOperation = updateIfData_t (const typename T::iterator)>
        bool updateData(T datum, UnaryOperation func = [](const typename T::iterator it){ return *it; });

    private:
        [[nodiscard]] bool validate(rapidjson::Value::MemberIterator& it, const rapidjson::Document& jsonDoc);
    };

    /**
     * @brief      Sends a status.
     *
     * @param[in]  key      The key
     * @param[in]  label    The label
     * @param[in]  value    The value
     * @param[in]  timeout  The timeout
     *
     * @tparam     T        The type of the value being sent
     */
    template<typename T>
    void statusUpdateInterface::sendStatus(const std::string& key, const std::string& label, const T& value,
        const timeout_t timeout)
    {
        rapidjson::Document workingDoc;

        if(!getStatusDoc(workingDoc))
        {
            return;
        }

        updateDoc(workingDoc, key, label, value, timeout);
        sendStatusDoc(workingDoc);
    }

    /**
     * @brief      Sends a status.
     *
     * @param[in]  key      The key
     * @param[in]  label    The label
     * @param[in]  value    The value
     * @param[in]  units    The units
     * @param[in]  timeout  The timeout
     *
     * @tparam     T        The type of the value being sent
     */
    template<typename T>
    void statusUpdateInterface::sendStatus(const std::string& key, const std::string& label, const T& value,
        const std::string& units, const timeout_t timeout)
    {
        rapidjson::Document workingDoc;

        if(!getStatusDoc(workingDoc))
        {
            return;
        }

        updateDoc(workingDoc, key, label, value, units, timeout);
        sendStatusDoc(workingDoc);
    }

    /**
     * @brief      Updates a Json Document to include the status key
     *
     * @param      jsonDoc  The json document
     * @param[in]  key      The key
     * @param[in]  label    The label
     * @param[in]  value    The value
     * @param[in]  timeout  The timeout
     *
     * @tparam     T        The type of the value being updated
     */
    template<typename T>
    void statusUpdateInterface::updateDoc(rapidjson::Document& jsonDoc, const std::string& key,
        const std::string& label, const T& value, const timeout_t timeout)
    {
        rapidjson::Value statusObject(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType& alloc = jsonDoc.GetAllocator();
        rapidjson::Value::MemberIterator it = jsonDoc.FindMember("fields");

        if(!validate(it, jsonDoc))
        {
            return;
        }

        helpers::jsonSet(statusObject, alloc, key, "key");
        helpers::jsonSet(statusObject, alloc, label, "label");
        helpers::jsonSet(statusObject, alloc, value, "value");
        helpers::jsonSet(statusObject, alloc, timeout.count(), "timeoutMs");

        it->value.GetArray().PushBack(statusObject, alloc);
    }

    /**
     * @brief      Updates a Json Document to include the status key
     *
     * @param      jsonDoc  The json document
     * @param[in]  key      The key
     * @param[in]  label    The label
     * @param[in]  value    The value
     * @param[in]  units    The units
     * @param[in]  timeout  The timeout
     *
     * @tparam     T        The type of the value being updated
     */
    template<typename T>
    void statusUpdateInterface::updateDoc(rapidjson::Document& jsonDoc, const std::string& key,
        const std::string& label, const T& value, const std::string& units,
        const timeout_t timeout)
    {
        rapidjson::Value statusObject(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType& alloc = jsonDoc.GetAllocator();
        rapidjson::Value::MemberIterator it = jsonDoc.FindMember("fields");

        if(!validate(it, jsonDoc))
        {
            return;
        }

        helpers::jsonSet(statusObject, alloc, key, "key");
        helpers::jsonSet(statusObject, alloc, label, "label");
        helpers::jsonSet(statusObject, alloc, value, "value");
        helpers::jsonSet(statusObject, alloc, units, "units");
        helpers::jsonSet(statusObject, alloc, timeout.count(), "timeoutMs");

        it->value.GetArray().PushBack(statusObject, alloc);
    }

    /**
     * @brief      Updates data based on a container being passed in.
     *
     * @param[in]  datum           The datum
     * @param[in]  func            The function
     *
     * @tparam     T               The type of the container holding status data to be updated
     * @tparam     UnaryOperation  A callback function to extract data from the container to the update structs
     *
     * @return     True if the update is sucessful, False otherwise.
     */
    template<class T, class UnaryOperation>
    bool statusUpdateInterface::updateData(T datum, UnaryOperation func)
    {
        assert(
            (typeid(typename std::iterator_traits<typename T::iterator>::iterator_category) == typeid(std::forward_iterator_tag)) ||
            (typeid(typename std::iterator_traits<typename T::iterator>::iterator_category) == typeid(std::bidirectional_iterator_tag)) ||
            (typeid(typename std::iterator_traits<typename T::iterator>::iterator_category) == typeid(std::random_access_iterator_tag))
        );

        const auto sendRange = [&](const auto& firstCalc, const auto& lastCalc) -> bool{
            rapidjson::Document workingDoc;

            if(!getStatusDoc(workingDoc))
            {
                logger::warn(__FILE__, __FUNCTION__, "***ERROR - getStatusDoc() FAILED!!!");
                return false;
            }

            auto first{std::next(std::begin(datum), firstCalc)};
            const auto last{std::next(std::begin(datum), lastCalc)};

            for(;first != last; first++)
            {
                const updateIfData_t& upData{func(first)};
                if(upData.valid)
                {
                    std::visit(
                        [&](const auto& val){
                            if(upData.units.size() > 0)
                            {
                                updateDoc(workingDoc, upData.key, upData.label, val, upData.units, upData.timeout);
                            }
                            else
                            {
                                updateDoc(workingDoc, upData.key, upData.label, val, upData.timeout);
                            }
                        },
                        upData.data
                    );
                }
            }

            if(!sendStatusDoc(workingDoc))
            {
                logger::warn(__FILE__, __FUNCTION__, "***ERROR - Status Update FAILED!!!");
                return false;
            }

            return true;
        };

        constexpr auto transmitChunkSize = 10;
        const auto loopCnt{static_cast<decltype(datum.size())>(
            std::ceil(static_cast<double>(datum.size()) / static_cast<double>(transmitChunkSize)))};
        bool result{true};

        for(unsigned i = 0; i < loopCnt; ++i)
        {
            const auto firstCalc{(i * transmitChunkSize)};
            const auto lastCalcPre{((loopCnt * transmitChunkSize) - (((loopCnt - i - 1) * transmitChunkSize)))};
            const auto lastCalc{static_cast<typename T::difference_type>(std::min(datum.size(), lastCalcPre))};
            result &= sendRange(firstCalc, lastCalc);
        }

        return result;
    }
}

#endif
