#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <memory>

namespace lim_webserver
{
    /**
     * @brief 单例包装类 返回原生类指针 
     */
    template <class T, int N = 0>
    class Singleton final
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
    class SingletonPtr final
    {
    public:
        static Shared_ptr<T> GetInstance()
        {
            static auto ins = MakeShared<T>();
            return ins;
        }
    };
}

#endif