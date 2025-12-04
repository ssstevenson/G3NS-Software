#ifndef KISS_UPDATE_DATA_H_
#define KISS_UPDATE_DATA_H_

#include <controller/kissMsg.h>
#include <helpers/types.h>
#include <statusUpdateInterface/statusUpdateInterface.h>

namespace empower::kiss
{
    /**
     * @brief      This class describes a KISS Data Update
     */
    class updateData
    {
    public:
        using data_set_t = helpers::types::data_set_t;
        using updateIfData_t = helpers::types::updateIfData_t;
        using timeout_t = helpers::types::timeout_t;
        using cmd_id_t = chunk::cmd_id_t;
        using data_map_t = std::unordered_map<cmd_id_t, updateIfData_t>;
        using base_vec_t = chunk::base_vec_t;

    private:
        statusUpdateInterface& updateIf;
        const std::string deviceKey;
        const timeout_t timeout;
        data_map_t dataset;

    public:
        updateData(statusUpdateInterface& upIf, const std::string& dev, const timeout_t& t);

        void pushUpdate() const;
        void invalidate();
        void update(message& msg, const bool present);

    private:
        void buildDataset();
    };
}

#endif
