#ifndef SERIAL_INTERFACE_CONNECTION_H_
#define SERIAL_INTERFACE_CONNECTION_H_

#include <climits>
#include <helpers/types.h>
#include <messageFactoryConnection/messageFactoryConnection.h>
#include <string>
#include <vector>
#include <zmq.hpp>

namespace empower
{
    /**
     * @brief      This class describes a serial interface connection.
     */
    class serialInterfaceConnection
    {
    public:
        using ser_data_t = helpers::types::ser_data_t;
        using timeout_t = helpers::types::timeout_t;
        typedef std::vector<ser_data_t> ser_msg_t;

    private:
        zmq::context_t& ctx;
        std::string connectionName;
        std::unique_ptr<zmq::socket_t> reqRepSock;
        messageFactoryConnection msgIf;
        std::string serialRequestJsonStr;

    public:
        serialInterfaceConnection(zmq::context_t& zmqCtx, const std::string& connName);

        bool write(const ser_msg_t& datum, const bool kissEncoding);
        [[nodiscard]] std::optional<ser_msg_t> read(const ser_msg_t& datum, timeout_t startTimeout,
            timeout_t charTimeout, const bool kissEncoding);

    private:
        [[nodiscard]] std::optional<std::string> setupJson(const ser_msg_t& datum, timeout_t startTimeout,
            timeout_t charTimeout, const bool kissEncoding);
        void setupSocket();
    };

    namespace helpers::serConn
    {
        using ser_msg_t = serialInterfaceConnection::ser_msg_t;

        /**
         * @brief      Gets a multi-byte value from a Serial Message
         *
         * @param      it     The iterator to where the byte string starts
         *
         * @tparam     T      Type being returned
         * @tparam     bytes  Number of Bytes to be read, defaulted to the size of T
         *
         * @return     The value
         */
        template<typename T, std::size_t bytes = sizeof(T)>
        [[nodiscard]] static auto getVal(ser_msg_t::const_iterator& it)
        {
            using data_array_t = std::array<std::uint8_t, bytes>;

            static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");
            static_assert(sizeof(T) >= bytes, "Specified Bytes more than can fit in data type!");
            static_assert(bytes > 0, "Specified Bytes must be greater than 0!");

            union
            {
                data_array_t ch;
                T u;
            } dest;
            dest.u = T{};

            for(typename data_array_t::size_type idx = bytes; idx > 0; --idx)
            {
                dest.ch[(idx - 1)] = static_cast<typename data_array_t::value_type>(*it++);
            }

            if constexpr(std::numeric_limits<T>::is_signed && std::numeric_limits<T>::is_integer)
            {
                if((bytes < sizeof(T)) && ((dest.ch[(bytes - 1)] & 0x80) != 0))
                {
                    for(typename data_array_t::size_type idx = bytes; idx < sizeof(T); ++idx)
                    {
                        dest.ch[idx] = 0xFF;
                    }
                }
            }

            return dest.u;
        }

        /**
         * @brief      Gets a Multi-Byte value from a Serial Message at a given Index offset
         *
         * @param[in]  data   The Serial Message
         * @param[in]  idx    The Index
         *
         * @tparam     T      The type being returned
         * @tparam     bytes  Number of Bytes to be read, defaulted to the size of T
         *
         * @return     The value
         */
        template<typename T, std::size_t bytes = sizeof(T)>
        [[nodiscard]] static auto getValIndex(const ser_msg_t& data, const ser_msg_t::size_type idx)
        {
            ser_msg_t::const_iterator it{std::next(std::begin(data), static_cast<ser_msg_t::difference_type>(idx))};
            return getVal<T, bytes>(it);
        }

        /**
         * @brief      Pushes a multi-byte value onto a ser_msg_t
         *
         * @param      it     The Back Inserter Iterator
         * @param[in]  data   The data
         *
         * @tparam     T      Type of the Data being pushed
         * @tparam     bytes  Number of bytes from the data that should be pushed
         */
        template<typename T, std::size_t bytes = sizeof(T)>
        static void putVal(std::back_insert_iterator<ser_msg_t>& it, const T& data)
        {
            using data_array_t = std::array<std::uint8_t, bytes>;

            static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");
            static_assert(sizeof(T) >= bytes, "Specified Bytes more than can fit in data type!");
            static_assert(bytes > 0, "Specified Bytes must be greater than 0!");

            union
            {
                data_array_t ch;
                T u;
            } dest;
            dest.u = data;

            for(typename data_array_t::size_type idx = bytes; idx > 0; --idx)
            {
                it = dest.ch[(idx - 1)];
            }
        }

        /**
         * @brief      Pushes a multi-byte value onto a ser_msg_t
         *
         * @param      it     The Back Inserter Iterator
         * @param[in]  data   The data
         *
         * @tparam     T      Type of the Data being pushed
         * @tparam     bytes  Number of bytes from the data that should be pushed
         */
        template<typename T, std::size_t bytes = sizeof(T)>
        static void putVal(std::back_insert_iterator<ser_msg_t>&& it, const T& data)
        {
            putVal<T, bytes>(it, data);
        }
    }
}

#endif
