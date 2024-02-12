#pragma once

#include <stdint.h>
#include <endian.h>

namespace lim_webserver
{
    namespace endian
    {
        template<class T>
        typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
        hostToNetwork(T host)
        {
            return (T)htobe64((uint64_t)host);
        }

        template<class T>
        typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
        hostToNetwork(T host)
        {
            return (T)htobe32((uint32_t)host);
        }
        
        template<class T>
        typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
        hostToNetwork(T host)
        {
            return (T)htobe16((uint16_t)host);
        }

        template<class T>
        typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
        networkToHost(T net)
        {
            return (T)be64toh((uint64_t)net);
        }

        template<class T>
        typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
        networkToHost(T net)
        {
            return (T)be32toh((uint32_t)net);
        }

        template<class T>
        typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
        networkToHost(T net)
        {
            return (T)be16toh((uint16_t)net);
        }
    }
} // namespace lim_webserver
