#pragma once

namespace lim_webserver
{
    /**
     * @brief 该类用于禁止对象的拷贝和赋值
     */
    class Noncopyable
    {
    public:
        /** 默认构造函数 */
        Noncopyable() = default;

        /** 默认析构函数 */
        ~Noncopyable() = default;

        /** 拷贝构造函数（禁用） */
        Noncopyable(const Noncopyable &) = delete;

        /** 赋值操作符（禁用）*/
        Noncopyable &operator=(const Noncopyable &) = delete;
    };
}