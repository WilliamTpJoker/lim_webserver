#include "Accepter.h"

namespace lim_webserver
{
    Accepter::Accepter(EventLoop* loop, Address::ptr address)
    : m_loop(loop), m_socket(Socket::CreateTCP(address)),m_channel(loop, m_socket->fd())
    {

    }

    void Accepter::listen()
    {
        m_listening=true;
        m_socket->listen();
    }

    void Accepter::HandleRead()
    {
        
    }

} // namespace lim_webserver
