#include <controller/kissUpdateData.h>

using namespace empower::kiss;

/**
 * @brief      Constructs a new instance.
 *
 * @param      upIf  The Status Update Interface
 * @param[in]  dev   The Device Name
 * @param[in]  t     The Timeout
 */
updateData::updateData(statusUpdateInterface& upIf, const std::string& dev, const timeout_t& t):
    updateIf{upIf},
    deviceKey{dev},
    timeout{t},
    dataset{}
{
    buildDataset();
}

/**
 * @brief      Pushes an update.
 */
void updateData::pushUpdate() const
{
    updateIf.updateData(dataset, [](const data_map_t::iterator it){ return it->second; });
}

/**
 * @brief      Invalidates the dataset
 */
void updateData::invalidate()
{
    std::for_each(std::begin(dataset), std::end(dataset), [](auto& d){ d.second.valid = false; });
}

/**
 * @brief      Updates the dataset and pushes keys to Status Manager
 *
 * @param      msg      The Message
 * @param[in]  present  If the module is present
 */
void updateData::update(message& msg, const bool present)
{
    invalidate();

    dataset.at(cmd_id_t::ack).data = present;
    dataset.at(cmd_id_t::ack).valid = true;

    if(present)
    {
        if(auto raw{msg.getRawMessage()}; raw.has_value())
        {
            dataset.at(cmd_id_t::dummy).data = helpers::types::vecToDataStr(raw.value());
            dataset.at(cmd_id_t::dummy).valid = true;
        }

        std::for_each(msg.begin(), msg.end(), [&](const auto& chunk){
            const auto cmdId{chunk.getCommandId()};

            // if command in dataset and not dummy, set the data and valid flags
            if(auto dataIt{dataset.find(cmdId)}; ((dataIt != dataset.end()) && (cmdId != cmd_id_t::dummy) &&
                (cmdId != cmd_id_t::ack)))
            {
                auto data{chunk.getData()};

                if(data.has_value())
                {
                    dataIt->second.valid = std::visit([&dataIt](auto&& val) -> bool {
                        using T = std::decay_t<decltype(val)>;

                        if constexpr(std::is_same_v<T, bool> || std::is_same_v<T, std::size_t>)
                        {
                            dataIt->second.data = val;
                            return true;
                        }
                        else if constexpr(std::is_same_v<T, base_vec_t>)
                        {
                            dataIt->second.data = helpers::types::vecToDataStr(val);
                            return true;
                        }
                        else if constexpr(std::is_floating_point_v<T>)
                        {
                            dataIt->second.data = static_cast<double>(val);
                            return true;
                        }
                        else if constexpr(std::is_integral_v<T> && std::is_signed_v<T>)
                        {
                            dataIt->second.data = static_cast<int>(val);
                            return true;
                        }
                        else if constexpr(std::is_integral_v<T> && !std::is_signed_v<T>)
                        {
                            dataIt->second.data = static_cast<unsigned>(val);
                            return true;
                        }

                        dataIt->second.data = 0;
                        return false;
                    }, data.value());
                }
            }
        });
    }

    pushUpdate();
}

/**
 * @brief      Builds the dataset.
 */
void updateData::buildDataset()
{
    using std::string_literals::operator""s;

    dataset.clear();

    // This ID is used to fill in the RAW Serial Message
    dataset.emplace(cmd_id_t::dummy,
        updateIfData_t{deviceKey + "_RAW"s, deviceKey + " Raw Response"s, "",  timeout});

    // This ID is used to fill in if the module is Present or not
    dataset.emplace(cmd_id_t::ack,
        updateIfData_t{deviceKey + "_PRESENT"s, deviceKey + " Is Present"s, "", timeout});

    // Remainder of Command ID's are as expected
    dataset.emplace(cmd_id_t::dsa_rf_a,
        updateIfData_t{deviceKey + "_DSA_RF_A"s, deviceKey + " DSA RF A"s, "dB",  timeout});
    dataset.emplace(cmd_id_t::dsa_rf_b,
        updateIfData_t{deviceKey + "_DSA_RF_B"s, deviceKey + " DSA RF B"s, "dB",  timeout});
    dataset.emplace(cmd_id_t::current_rf_a,
        updateIfData_t{deviceKey + "_CURRENT_RF_A"s, deviceKey + " RF Current A"s, "A",  timeout});
    dataset.emplace(cmd_id_t::current_rf_b,
        updateIfData_t{deviceKey + "_CURRENT_RF_B"s, deviceKey + " RF Current B"s, "A",  timeout});
}
