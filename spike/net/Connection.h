#pragma once

#include "Buffer.h"
#include "Socket.h"

namespace lim_webserver
{
    class Connection
    {
    public:
    private:
    };

    class TcpConnection : public Connection
    {
    public:
    private:
        Buffer m_inputBuffer;
        Buffer m_outputBuffer;

        Address::ptr m_localAddress;
        Address::ptr m_peerAddress;
    };
} // namespace lim_webserver
