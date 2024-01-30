#pragma once

#include "net/Server.h"

namespace lim_webserver
{
    namespace http
    {
        /**
         * @brief HTTP服务器类
         */
        class HttpServer : public TcpServer
        {
        public:
            /// 智能指针类型
            using ptr = std::shared_ptr<HttpServer>;

        public:
            /**
             * @brief 构造函数
             * @param[in] keepalive 是否长连接
             * @param[in] accept_worker 接收连接调度器
             */
            HttpServer(const std::string &name, bool keepalive = false);

        protected:
            virtual void handleClient(Socket::ptr client) override;

        private:
            bool m_isKeepalive; // 是否支持长连接
        };
    } // namespace http

} // namespace lim_webserver
