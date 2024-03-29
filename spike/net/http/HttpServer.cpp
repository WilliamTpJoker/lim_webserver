#include "HttpServer.h"
#include "HttpSession.h"
#include "splog.h"

namespace lim_webserver
{
    namespace http
    {
        static Logger::ptr g_logger = LOG_SYS();

        HttpServer::HttpServer(bool keepalive, Scheduler *worker, Scheduler *accepter) : TcpServer(worker, accepter), m_isKeepalive(keepalive) {}

        void HttpServer::handleClient(Socket::ptr client)
        {
            HttpSession::ptr session(new HttpSession(client));
            LOG_TRACE(g_logger) << "handleClient " << session->peerAddressString();
            while (true)
            {
                auto req = session->recvRequest();
                if (!req)
                {
                    LOG_DEBUG(g_logger) << "recv http request fail, errno=" << errno << " errstr=" << strerror(errno)
                                        << " cliet:" << session->peerAddressString() << " keep_alive=" << m_isKeepalive;
                    break;
                }

                HttpResponse::ptr rsp(new HttpResponse(req->version(), req->isClose() || !m_isKeepalive));
                rsp->setHeader("Server", m_name);
                rsp->setBody("hello world, If you see this page, the lim web server is successfully installed and working. Further configuration is required.");
                session->sendResponse(rsp);

                LOG_TRACE(g_logger) << "cliet: " << session->peerAddressString() << ", request:\n" << req->toString();
                LOG_TRACE(g_logger) << "cliet: " << session->peerAddressString() << ", response:\n" << rsp->toString();

                if (!m_isKeepalive || req->isClose())
                {
                    break;
                }
            }
            session->close();
        }
    } // namespace http

} // namespace lim_webserver
