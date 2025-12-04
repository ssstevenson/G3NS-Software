#ifndef JSON_UPDATE_HELPERS_H_
#define JSON_UPDATE_HELPERS_H_

#include <array>
#include <functional>
#include <helpers/types.h>
#include <helpers/zmqConnectionNames.h>
#include <iomanip>
#include <logger/logger.h>
#include <memory>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wnull-dereference"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#pragma GCC diagnostic pop
#include <sstream>
#include <string>
#include <vector>

namespace empower::helpers
{
    template<typename T>
    [[nodiscard]] inline bool isWholeNumber(double val)
    {
        return std::isfinite(val) && val == std::floor(val);
    }

    // Safe type conversion from RapidJSON value
    template<typename T>
    [[nodiscard]] inline std::optional<T> safeGetValue(const rapidjson::Value& val)
    {
        try {
            if constexpr(std::is_same_v<T, bool>)
            {
                if(val.IsBool()) return val.GetBool();
                if(val.IsInt()) return val.GetInt() != 0;
                if(val.IsUint()) return val.GetUint() != 0;
                if(val.IsString()) 
                {
                    std::string s = val.GetString();
                    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                    return (s == "true" || s == "1" || s == "yes" || s == "on");
                }
            }
            else if constexpr(std::is_same_v<T, std::string>)
            {
                if(val.IsString()) return std::string(val.GetString(), val.GetStringLength());
                if(val.IsInt()) return std::to_string(val.GetInt());
                if(val.IsInt64()) return std::to_string(val.GetInt64());
                if(val.IsUint()) return std::to_string(val.GetUint());
                if(val.IsUint64()) return std::to_string(val.GetUint64());
                if(val.IsDouble()) return std::to_string(val.GetDouble());
                if(val.IsBool()) return val.GetBool() ? "true" : "false";
            }
            else if constexpr(std::is_unsigned_v<T> && std::is_integral_v<T>)
            {
                // Check all numeric types for unsigned conversion
                if(val.IsUint())
                {
                    auto uval = val.GetUint();
                    if(uval <= std::numeric_limits<T>::max())
                        return static_cast<T>(uval);
                }
                else if(val.IsUint64())
                {
                    auto uval = val.GetUint64();
                    if(uval <= std::numeric_limits<T>::max())
                        return static_cast<T>(uval);
                }
                else if(val.IsInt())
                {
                    auto ival = val.GetInt();
                    if(ival >= 0 && static_cast<unsigned>(ival) <= std::numeric_limits<T>::max())
                        return static_cast<T>(ival);
                }
                else if(val.IsInt64())
                {
                    auto ival = val.GetInt64();
                    if(ival >= 0 && static_cast<uint64_t>(ival) <= std::numeric_limits<T>::max())
                        return static_cast<T>(ival);
                }
                else if(val.IsDouble())
                {
                    double dval = val.GetDouble();
                    if(dval >= 0 && isWholeNumber<T>(dval))
                    {
                        // Special handling for large unsigned types
                        if constexpr(sizeof(T) >= sizeof(uint64_t))
                        {
                            if(dval <= 9007199254740992.0) // 2^53
                            {
                                return static_cast<T>(dval);
                            }
                        }
                        else
                        {
                            if(dval <= static_cast<double>(std::numeric_limits<T>::max()))
                            {
                                return static_cast<T>(dval);
                            }
                        }
                    }
                }
                else if(val.IsString())
                {
                    try {
                        std::string str = val.GetString();
                        size_t idx = 0;
                        unsigned long long uval = std::stoull(str, &idx);
                        if(idx == str.length() && uval <= std::numeric_limits<T>::max())
                            return static_cast<T>(uval);
                    } catch(...) {}
                }
            }
            else if constexpr(std::is_signed_v<T> && std::is_integral_v<T>)
            {
                if(val.IsInt())
                {
                    auto ival = val.GetInt();
                    if(ival >= std::numeric_limits<T>::min() && ival <= std::numeric_limits<T>::max())
                        return static_cast<T>(ival);
                }
                else if(val.IsInt64())
                {
                    auto ival = val.GetInt64();
                    if(ival >= std::numeric_limits<T>::min() && ival <= std::numeric_limits<T>::max())
                        return static_cast<T>(ival);
                }
                else if(val.IsUint())
                {
                    auto uval = val.GetUint();
                    if(uval <= static_cast<unsigned>(std::numeric_limits<T>::max()))
                        return static_cast<T>(uval);
                }
                else if(val.IsUint64())
                {
                    auto uval = val.GetUint64();
                    if(uval <= static_cast<uint64_t>(std::numeric_limits<T>::max()))
                        return static_cast<T>(uval);
                }
                else if(val.IsDouble())
                {
                    double dval = val.GetDouble();
                    if(isWholeNumber<T>(dval))
                    {
                        // Special handling for large signed types
                        if constexpr(sizeof(T) >= sizeof(int64_t))
                        {
                            // For 64-bit signed, check if the double is in valid range
                            if(dval >= -9007199254740992.0 && dval <= 9007199254740992.0)
                            {
                                return static_cast<T>(dval);
                            }
                        }
                        else
                        {
                            // For smaller types, the comparison is safe
                            if(dval >= static_cast<double>(std::numeric_limits<T>::min()) && 
                            dval <= static_cast<double>(std::numeric_limits<T>::max()))
                            {
                                return static_cast<T>(dval);
                            }
                        }
                    }
                }
                else if(val.IsString())
                {
                    try {
                        std::string str = val.GetString();
                        size_t idx = 0;
                        long long ival = std::stoll(str, &idx);
                        if(idx == str.length() && 
                        ival >= std::numeric_limits<T>::min() && 
                        ival <= std::numeric_limits<T>::max())
                            return static_cast<T>(ival);
                    } catch(...) {}
                }
            }
            else if constexpr(std::is_floating_point_v<T>)
            {
                if(val.IsDouble()) 
                {
                    double dval = val.GetDouble();
                    if constexpr(!std::is_same_v<T, long double>)
                    {
                        if(std::isfinite(dval) &&
                        dval >= std::numeric_limits<T>::lowest() &&
                        dval <= std::numeric_limits<T>::max())
                            return static_cast<T>(dval);
                    }
                    else
                    {
                        if(std::isfinite(dval))
                            return static_cast<T>(dval);
                    }
                }
                else if(val.IsFloat())
                {
                    float fval = val.GetFloat();
                    if(std::isfinite(fval))
                        return static_cast<T>(fval);
                }
                else if(val.IsInt()) 
                    return static_cast<T>(val.GetInt());
                else if(val.IsInt64()) 
                    return static_cast<T>(val.GetInt64());
                else if(val.IsUint()) 
                    return static_cast<T>(val.GetUint());
                else if(val.IsUint64()) 
                {
                    // Be careful with uint64 to float conversion
                    auto uval = val.GetUint64();
                    if constexpr(std::is_same_v<T, float>)
                    {
                        // Float can only precisely represent integers up to 2^24
                        if(uval <= 16777216ULL)
                            return static_cast<T>(uval);
                    }
                    else
                    {
                        // Double can precisely represent integers up to 2^53
                        if(uval <= 9007199254740992ULL)
                            return static_cast<T>(uval);
                    }
                }
                else if(val.IsString())
                {
                    try {
                        std::string str = val.GetString();
                        size_t idx = 0;
                        long double ldval = std::stold(str, &idx);
                        if(idx == str.length() && std::isfinite(ldval))
                        {
                            if constexpr(!std::is_same_v<T, long double>)
                            {
                                if(ldval >= std::numeric_limits<T>::lowest() &&
                                ldval <= std::numeric_limits<T>::max())
                                    return static_cast<T>(ldval);
                            }
                            else
                            {
                                return static_cast<T>(ldval);
                            }
                        }
                    } catch(...) {}
                }
            }
        }
        catch(const std::exception& e) {
            // Log error if logger is available
            // logger::debug(__FILE__, __FUNCTION__, 
            //              std::string("JSON value conversion error: ") + e.what());
        }
        catch(...) {
            // Catch any other errors
        }
        
        return std::nullopt;
    }

    // Existing helper functions remain the same
    template<typename RJT>
    [[nodiscard]] inline rapidjson::StringBuffer jsonToStrBuf(const RJT& jsonDoc)
    {
        static_assert(std::is_same_v<RJT, rapidjson::Document> || std::is_same_v<RJT, rapidjson::Value>);
        constexpr std::size_t jsonStrBufferSize = (4 * 1024);

        rapidjson::StringBuffer jsonStrBuf(0, jsonStrBufferSize);
        jsonStrBuf.Clear();
        rapidjson::Writer<rapidjson::StringBuffer, typename RJT::EncodingType, rapidjson::ASCII<> >
            jsonWriter(jsonStrBuf);
        jsonDoc.Accept(jsonWriter);
        return jsonStrBuf;
    }

    template<typename RJT>
    [[nodiscard]] inline std::string jsonToStr(const RJT& jsonDoc)
    {
        auto jsonStrBuf{jsonToStrBuf(jsonDoc)};
        return std::string(jsonStrBuf.GetString(), jsonStrBuf.GetSize());
    }

    template<typename RJT>
    [[nodiscard]] inline zmq::message_t jsonToMsg(const RJT& jsonDoc)
    {
        auto jsonStrBuf{jsonToStrBuf(jsonDoc)};
        return zmq::message_t(jsonStrBuf.GetString(), jsonStrBuf.GetSize());
    }

    inline void setupJsonResponse(rapidjson::Document& jsonDoc, const std::string& newStr)
    {
        if(rapidjson::Document newDoc; !newDoc.Parse(newStr.data(), newStr.size()).HasParseError())
        {
            if(newDoc.FindMember("request") != newDoc.MemberEnd())
            {
                newDoc.RemoveMember("request");
            }

            newDoc.AddMember("request", rapidjson::Value().CopyFrom(jsonDoc, newDoc.GetAllocator()).Move(),
                newDoc.GetAllocator());
            jsonDoc.Swap(newDoc);
        }
    }

    /**
     * @brief      Sets a value in a Json Structure
     *
     * @param[in]  jsonIt     The json iterator
     * @param      alloc      The allocate
     * @param[in]  val        The value
     * @param[in]  curIt      The current iterator
     * @param[in]  endIt      The end iterator
     *
     * @tparam     T          The type of the value being set
     * @tparam     Iter       Iterator Type for navigating the Json Structure
     *
     * @return     True if the value was sucessfully set, False otherwise.
     */
    template<typename T, class Iter, typename = typename std::enable_if<!std::is_same_v<const char*, T>>::type,
        typename = typename std::enable_if<!std::is_same_v<char*, T>>::type>
    [[nodiscard]] inline bool jsonSetVal(rapidjson::Value::MemberIterator jsonIt,
        rapidjson::Document::AllocatorType& alloc, const T& val, Iter curIt, Iter endIt)
    {
        if(std::distance(curIt, endIt) == 0)
        {

            if constexpr(std::is_same_v<std::string, T>)
            {
                jsonIt->value.SetString(val.data(), static_cast<rapidjson::SizeType>(val.size()), alloc);
            }
            else
            {
                jsonIt->value.Set(val);
            }

            return true;
        }
        else if(!jsonIt->value.IsObject())
        {
            if constexpr(std::is_same_v<std::string, T>)
            {
                jsonIt->value = rapidjson::Value(rapidjson::kObjectType).AddMember(
                    rapidjson::Value(curIt->data(), alloc).Move(), rapidjson::Value(val.data(), alloc).Move(),
                    alloc);
            }
            else if constexpr(std::is_same_v<float, T>)
            {
                jsonIt->value = rapidjson::Value(rapidjson::kObjectType).AddMember(
                    rapidjson::Value(curIt->data(), alloc).Move(), static_cast<double>(val), alloc);
            }
            else
            {
                jsonIt->value = rapidjson::Value(rapidjson::kObjectType).AddMember(
                    rapidjson::Value(curIt->data(), alloc).Move(), val, alloc);
            }
        }

        rapidjson::Value::MemberIterator tmpJsonIt = jsonIt->value.FindMember(curIt->data());

        if(tmpJsonIt == jsonIt->value.MemberEnd())
        {
            rapidjson::Value keyVal(curIt->data(), alloc);
            jsonIt->value.AddMember(keyVal, rapidjson::Value(rapidjson::kObjectType).Move(), alloc);
            tmpJsonIt = jsonIt->value.FindMember(curIt->data());
        }

        return jsonSetVal(tmpJsonIt, alloc, val, std::next(curIt), endIt);
    }

    /**
     * @brief      Sets a field in a Json Document
     *
     * @param      jsonDoc  The json document
     * @param[in]  val      The value
     * @param[in]  args     The arguments
     *
     * @tparam     T        The type of the value being set
     *
     * @return     True if the value is sucessfully set, False otherwise.
     */
    template<typename T, typename ...Types>
    inline bool jsonSet(rapidjson::Document& jsonDoc, const T& val, const Types... args)
    {
        static_assert(sizeof...(args) > 0);

        std::array<const std::string_view, sizeof...(args)> data = {{args...}};
        typename std::array<const std::string_view, sizeof...(args)>::const_iterator dataIt{data.begin()};
        rapidjson::Value::MemberIterator jsonIt = jsonDoc.FindMember(dataIt->data());

        if(jsonIt == jsonDoc.MemberEnd())
        {
            rapidjson::Value keyVal(dataIt->data(), jsonDoc.GetAllocator());
            jsonDoc.AddMember(keyVal, rapidjson::Value(rapidjson::kNullType).Move(), jsonDoc.GetAllocator());
            jsonIt = jsonDoc.FindMember(dataIt->data());
        }

        if constexpr((std::is_same_v<char*, T>) || std::is_same_v<const char*, T>)
        {
            const std::string valStr{val};
            return jsonSetVal(jsonIt, jsonDoc.GetAllocator(), valStr, std::next(dataIt), data.end());
        }

        return jsonSetVal(jsonIt, jsonDoc.GetAllocator(), val, std::next(dataIt), data.end());
    }

    /**
     * @brief      Sets a field in a Json Value
     *
     * @param      jsonVal  The json value
     * @param      alloc    The allocator for the Json Structure
     * @param[in]  val      The value
     * @param[in]  args     The arguments
     *
     * @tparam     T        The type of the value being set
     *
     * @return     True if the value is sucessfully set, False otherwise.
     */
    template<typename T, typename ...Types>
    inline bool jsonSet(rapidjson::Value& jsonVal, rapidjson::Document::AllocatorType& alloc, const T& val,
        const Types... args)
    {
        static_assert(sizeof...(args) > 0);

        std::array<const std::string_view, sizeof...(args)> data = {{args...}};
        typename std::array<const std::string_view, sizeof...(args)>::const_iterator dataIt{data.begin()};
        rapidjson::Value::MemberIterator jsonIt = jsonVal.FindMember(dataIt->data());

        if(jsonIt == jsonVal.MemberEnd())
        {
            rapidjson::Value keyVal(dataIt->data(), alloc);
            jsonVal.AddMember(keyVal, rapidjson::Value(rapidjson::kNullType).Move(), alloc);
            jsonIt = jsonVal.FindMember(dataIt->data());
        }

        if constexpr((std::is_same_v<char*, T>) || std::is_same_v<const char*, T>)
        {
            const std::string valStr{val};
            return jsonSetVal(jsonIt, alloc, valStr, std::next(dataIt), data.end());
        }

        return jsonSetVal(jsonIt, alloc, val, std::next(dataIt), data.end());
    }

    /**
     * @brief      Get a value from a Json Structure
     *
     * @param[in]  jsonIt  The json iterator
     * @param[in]  curIt   The current iterator
     * @param[in]  endIt   The end iterator
     *
     * @tparam     T       The type of the value bein gretreived
     *
     * @return     The value if it can sucessfully be read, std::nullopt otherwise.
     */
    template<typename T, class Iter>
    [[nodiscard]] inline std::optional<T> jsonGetVal(rapidjson::Value::ConstMemberIterator jsonIt,
        Iter curIt, Iter endIt)
    {
        if(std::distance(curIt, endIt) == 0)
        {
            return safeGetValue<T>(jsonIt->value);
        }

        if(!jsonIt->value.IsObject())
        {
            return std::nullopt;
        }

        rapidjson::Value::ConstMemberIterator tmpJsonIt{jsonIt->value.FindMember(curIt->data())};
        if(tmpJsonIt != jsonIt->value.MemberEnd())
        {
            return jsonGetVal<T>(tmpJsonIt, std::next(curIt), endIt);
        }

        return std::nullopt;
    }

    template<typename T, typename RJT, typename ...Types>
    [[nodiscard]] inline std::optional<T> jsonGet(const RJT& jsonDoc, const Types... args)
    {
        static_assert(std::is_same_v<RJT, rapidjson::Document> || std::is_same_v<RJT, rapidjson::Value>);
        static_assert(sizeof...(args) > 0);

        try {
            std::array<const std::string_view, sizeof...(args)> data = {{args...}};
            typename std::array<const std::string_view, sizeof...(args)>::const_iterator dataIt{data.begin()};

            if(!jsonDoc.IsObject())
            {
                return std::nullopt;
            }

            rapidjson::Value::ConstMemberIterator jsonIt{jsonDoc.FindMember(dataIt->data())};
            if(jsonIt != jsonDoc.MemberEnd())
            {
                return jsonGetVal<T>(jsonIt, std::next(dataIt), data.end());
            }
        }
        catch(const std::exception& e) {
            logger::debug(__FILE__, __FUNCTION__, 
                         std::string("JSON get error: ") + e.what());
        }
        catch(...) {
            logger::debug(__FILE__, __FUNCTION__, "Unknown JSON get error");
        }

        return std::nullopt;
    }

    /**
     * @brief      Json Set Vector data helper
     *
     * @param[in]  jsonIt  The json iterator
     * @param      alloc   The allocator
     * @param[in]  val     The value
     * @param[in]  curIt   The current iterator
     * @param[in]  endIt   The end iterator
     *
     * @tparam     T       The type of the data in the array
     *
     * @return     True if the write was sucessful, False otherwise.
     */
    template<typename T, class Iter>
    [[nodiscard]] inline bool jsonSetVecVal(rapidjson::Value::MemberIterator jsonIt,
        rapidjson::Document::AllocatorType& alloc, const std::vector<T>& val, Iter curIt, Iter endIt)
    {
        if(std::distance(curIt, endIt) == 0)
        {
            if(!jsonIt->value.IsArray())
            {
                jsonIt->value = rapidjson::Value(rapidjson::kArrayType);
            }

            for(auto& valIt: val)
            {
                if constexpr(std::is_same_v<std::string, T>)
                {
                    jsonIt->value.PushBack(rapidjson::Value().SetString(valIt.data(),
                        static_cast<rapidjson::SizeType>(valIt.size()), alloc), alloc);
                }
                else if constexpr(std::is_same_v<char*, T>)
                {
                    const std::string valStr{valIt};
                    jsonIt->value.PushBack(rapidjson::Value().SetString(valStr.data(),
                        static_cast<rapidjson::SizeType>(valStr.size()), alloc), alloc);
                }
                else if constexpr(std::is_same_v<float, T>)
                {
                    jsonIt->value.PushBack(static_cast<double>(valIt), alloc);
                }
                else
                {
                    jsonIt->value.PushBack(valIt, alloc);
                }
            }

            return true;
        }
        else if(!jsonIt->value.IsObject())
        {
            jsonIt->value = rapidjson::Value(rapidjson::kObjectType).AddMember(
                rapidjson::Value(curIt->data(), alloc).Move(),
                rapidjson::Value(rapidjson::kArrayType).Move(), alloc);
        }

        rapidjson::Value::MemberIterator tmpJsonIt = jsonIt->value.FindMember(curIt->data());

        if(tmpJsonIt == jsonIt->value.MemberEnd())
        {
            rapidjson::Value keyVal(curIt->data(), alloc);
            jsonIt->value.AddMember(keyVal, rapidjson::Value(rapidjson::kObjectType).Move(), alloc);
            tmpJsonIt = jsonIt->value.FindMember(curIt->data());
        }

        return jsonSetVecVal(tmpJsonIt, alloc, val, std::next(curIt), endIt);
    }

    /**
     * @brief      Json Write Vector element to a document
     *
     * @param      jsonDoc  The json document
     * @param[in]  val      The value
     * @param[in]  args     The arguments
     *
     * @tparam     T        The type of data in the vector
     *
     * @return     True if the write was sucessful, False otherwise.
     */
    template<typename T, typename ...Types>
    inline bool jsonSetVec(rapidjson::Document& jsonDoc, const std::vector<T>& val, const Types... args)
    {
        static_assert(sizeof...(args) > 0);

        std::array<const std::string_view, sizeof...(args)> data = {{args...}};
        typename std::array<const std::string_view, sizeof...(args)>::const_iterator dataIt{data.begin()};
        rapidjson::Value::MemberIterator jsonIt = jsonDoc.FindMember(dataIt->data());

        if(jsonIt == jsonDoc.MemberEnd())
        {
            rapidjson::Value keyVal(dataIt->data(), jsonDoc.GetAllocator());
            jsonDoc.AddMember(keyVal, rapidjson::Value(rapidjson::kObjectType).Move(), jsonDoc.GetAllocator());
            jsonIt = jsonDoc.FindMember(dataIt->data());
        }

        return jsonSetVecVal(jsonIt, jsonDoc.GetAllocator(), val, std::next(dataIt), data.end());
    }

    /**
     * @brief      Json Write Vector element to a Value
     *
     * @param      jsonVal  The json value
     * @param      alloc    The allocator
     * @param[in]  val      The value
     * @param[in]  args     The arguments
     *
     * @tparam     T        The type of data in the vector
     *
     * @return     True if the write was sucessful, False otherwise.
     */
    template<typename T, typename ...Types>
    inline bool jsonSetVec(rapidjson::Value& jsonVal, rapidjson::Document::AllocatorType& alloc,
        const std::vector<T>& val, const Types... args)
    {
        static_assert(sizeof...(args) > 0);

        std::array<const std::string_view, sizeof...(args)> data = {{args...}};
        typename std::array<const std::string_view, sizeof...(args)>::const_iterator dataIt{data.begin()};
        rapidjson::Value::MemberIterator jsonIt = jsonVal.FindMember(dataIt->data());

        if(jsonIt == jsonVal.MemberEnd())
        {
            rapidjson::Value keyVal(dataIt->data(), alloc);
            jsonVal.AddMember(keyVal, rapidjson::Value(rapidjson::kObjectType).Move(), alloc);
            jsonIt = jsonVal.FindMember(dataIt->data());
        }

        return jsonSetVecVal(jsonIt, alloc, val, std::next(dataIt), data.end());
    }

    /**
     * @brief      Get a vector element from a Json structure
     *
     * @param[in]  jsonIt  The json iterator
     * @param[in]  curIt   The current iterator
     * @param[in]  endIt   The end iterator
     *
     * @tparam     T       The type of data in the vector
     *
     * @return     The vector of elements if sucessful, std::nullopt otherwise.
     */
    template<typename T, class Iter>
    [[nodiscard]] inline std::optional<std::vector<T>> jsonGetVecVal(
        rapidjson::Value::ConstMemberIterator jsonIt, Iter curIt, Iter endIt)
    {
        if(std::distance(curIt, endIt) == 0)
        {
            if(!jsonIt->value.IsArray())
            {
                return std::nullopt;
            }

            std::vector<T> val;
            val.reserve(jsonIt->value.Size());

            for(const auto& arrayIt: jsonIt->value.GetArray())
            {
                auto convertedVal = safeGetValue<T>(arrayIt);
                if(!convertedVal.has_value())
                {
                    logger::debug(__FILE__, __FUNCTION__, 
                                 "Failed to convert array element to requested type");
                    return std::nullopt;
                }
                val.emplace_back(convertedVal.value());
            }

            return val;
        }

        if(!jsonIt->value.IsObject())
        {
            return std::nullopt;
        }

        rapidjson::Value::ConstMemberIterator tmpJsonIt{jsonIt->value.FindMember(curIt->data())};
        if(tmpJsonIt != jsonIt->value.MemberEnd())
        {
            return jsonGetVecVal<T>(tmpJsonIt, std::next(curIt), endIt);
        }

        return std::nullopt;
    }

    // Updated jsonGetVec with additional safety
    template<typename T, typename RJT, typename ...Types>
    [[nodiscard]] inline std::optional<std::vector<T>> jsonGetVec(const RJT& jsonDoc, const Types... args)
    {
        static_assert(std::is_same_v<RJT, rapidjson::Document> || std::is_same_v<RJT, rapidjson::Value>);
        static_assert(sizeof...(args) > 0);

        try {
            std::array<const std::string_view, sizeof...(args)> data = {{args...}};
            typename std::array<const std::string_view, sizeof...(args)>::const_iterator dataIt{data.begin()};

            if(!jsonDoc.IsObject())
            {
                return std::nullopt;
            }

            rapidjson::Value::ConstMemberIterator jsonIt{jsonDoc.FindMember(dataIt->data())};
            if(jsonIt != jsonDoc.MemberEnd())
            {
                return jsonGetVecVal<T>(jsonIt, std::next(dataIt), data.end());
            }
        }
        catch(const std::exception& e) {
            logger::debug(__FILE__, __FUNCTION__, 
                         std::string("JSON get vector error: ") + e.what());
        }
        catch(...) {
            logger::debug(__FILE__, __FUNCTION__, "Unknown JSON get vector error");
        }

        return std::nullopt;
    }
}

#endif
