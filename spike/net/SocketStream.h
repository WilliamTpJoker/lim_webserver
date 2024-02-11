#pragma once

#include "ByteArray.h"
#include "Socket.h"

namespace lim_webserver
{
    class SocketStream
    {
    public:
        using ptr = std::shared_ptr<SocketStream>;

        static ptr Create(Socket::ptr sock, bool owner = true) { return std::make_shared<SocketStream>(sock, owner); }

    public:
        /**
         * @brief 构造函数
         * @param[in] sock Socket类
         * @param[in] owner 是否完全控制
         */
        SocketStream(Socket::ptr sock, bool owner = true);

        /**
         * @brief 析构函数
         * @details 如果m_owner=true,则close
         */
        ~SocketStream();

        /**
         * @brief 读取数据
         * @param[out] buffer 待接收数据的内存
         * @param[in] length 待接收数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int resv(void *buffer, size_t length);

        /**
         * @brief 读取数据
         * @param[out] ba 接收数据的ByteArray
         * @param[in] length 待接收数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int resv(ByteArray::ptr ba, size_t length);

        /**
         * @brief 写入数据
         * @param[in] buffer 待发送数据的内存
         * @param[in] length 待发送数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int send(const void *buffer, size_t length);

        /**
         * @brief 写入数据
         * @param[in] ba 待发送数据的ByteArray
         * @param[in] length 待发送数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int send(ByteArray::ptr ba, size_t length);

        /**
         * @brief 关闭socket
         */
        virtual void close();

        /**
         * @brief 返回Socket类
         */
        Socket::ptr socket() const { return m_socket; }

        /**
         * @brief 返回是否连接
         */
        bool isConnected() const;

        Address::ptr peerAddress();
        Address::ptr localAddress();
        std::string peerAddressString();
        std::string localAddressString();

    protected:
        /// Socket类
        Socket::ptr m_socket;
        /// 是否主控
        bool m_owner;
    };
} // namespace lim_webserver
