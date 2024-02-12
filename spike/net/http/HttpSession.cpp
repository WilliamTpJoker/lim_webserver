#include "HttpSession.h"
#include "splog.h"

namespace lim_webserver
{
    namespace http
    {
        HttpSession::HttpSession(Socket::ptr sock, bool owner) : SocketStream(sock, owner) {}

        HttpRequest::ptr HttpSession::recvRequest()
        {
            HttpRequestParser::ptr parser(new HttpRequestParser);
            uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
            std::shared_ptr<char> buffer(new char[buff_size], [](char *ptr) { delete[] ptr; });
            char *data = buffer.get();
            int offset = 0;
            while (true)
            {
                int len = resv(data + offset, buff_size - offset);
                if (len <= 0)
                {
                    close();
                    return nullptr;
                }
                len += offset;
                size_t nparse = parser->execute(data, len);
                if (parser->hasError())
                {
                    close();
                    return nullptr;
                }
                offset = len - nparse;
                if (offset == (int)buff_size)
                {
                    close();
                    return nullptr;
                }
                if (parser->isFinished())
                {
                    break;
                }
            }
            int64_t length = parser->getContentLength();
            if (length > 0)
            {
                std::string body;
                body.resize(length);

                int len = 0;
                if (length >= offset)
                {
                    memcpy(&body[0], data, offset);
                    len = offset;
                }
                else
                {
                    memcpy(&body[0], data, length);
                    len = length;
                }
                length -= offset;
                if (length > 0)
                {
                    if (resv(&body[len], length) <= 0)
                    {
                        close();
                        return nullptr;
                    }
                }
                parser->data()->setBody(body);
            }

            parser->data()->init();
            return parser->data();
        }

        int HttpSession::sendResponse(HttpResponse::ptr rsp)
        {
            std::string data = rsp->toString();
            return send(data.c_str(), data.size());
        }

    } // namespace http

} // namespace lim_webserver
