#pragma once

#include "Address.h"

#include <string>

namespace lim_webserver
{
    class Client
    {
    public:
        Client(Address::ptr address, std::string name);

        void send();

        void read();

        void connect();

    private:
        
    };
} // namespace lim_webserver
