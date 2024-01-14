#pragma once

#include <memory>

namespace lim_webserver
{
    /**
     * @brief 单例包装类 返回原生类指针 
     */
    template <class T, int N = 0>
    class Singleton
    {
    public:
        static T *GetInstance()
        {
            static T ins;
            return &ins;
        }
    };
    /**
     *  @brief 单例包装类 返回类智能指针
     */
    template <class T, int N = 0>
    class SingletonPtr
    {
    public:
        static std::shared_ptr<T> GetInstance()
        {
            static auto ins = std::make_shared<T>();
            return ins;
        }
    };
}