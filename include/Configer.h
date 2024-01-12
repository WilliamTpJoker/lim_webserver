#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <regex>
#include <yaml-cpp/yaml.h>
#include <unordered_set>
#include <iostream>

#include "Mutex.h"

namespace lim_webserver
{
    class ConfigerVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigerVarBase>;

    public:
        /**
         * @brief 构造函数
         * @param name 配置参数名称
         * @param description 配置参数描述
         */
        ConfigerVarBase(const std::string &name, const std::string &description = "")
            : m_name(name), m_description(description) {}
        virtual ~ConfigerVarBase() {}

        /**
         * @brief 获得配置参数名称
         */
        const std::string &getName() const { return m_name; }
        /**
         * @brief 获得配置参数描述
         */
        const std::string &getDescription() const { return m_description; }

        /**
         * @brief 转成字符串
         */
        virtual std::string toString() = 0;
        /**
         * @brief 从字符串初始化值
         */
        virtual bool fromString(const std::string &val) = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };

    /**
     * @brief 类型转换模板类(Source 源类型, Target 目标类型)
     *
     * @note boost::lexical_cast 的包装，
     * 因为 boost::lexical_cast 是使用 std::stringstream 实现的类型转换，
     * 所以仅支持实现了 ostream::operator<< 与 istream::operator>> 的类型,
     * 可以说默认情况下仅支持 std::string 与各类 Number 类型的双向转换。
     * 需要转换自定义的类型，可以选择实现对应类型的流操作符，或者将该模板类进行偏特化
     */
    template <typename Source, typename Target>
    class LexicalCast
    {
    public:
        Target operator()(const Source &source)
        {
            return boost::lexical_cast<Target>(source);
        }
    };

    /**
     * @brief 类型转换模板类偏特化(YAML String 转换成 std::vector<T>)
     */
    template <typename T>
    class LexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &source)
        {
            // 调用 YAML::Load 解析传入的字符串，解析失败会抛出异常
            YAML::Node node = YAML::Load(source);
            std::vector<T> config_list;
            std::stringstream ss;
            for (const auto &item : node)
            {
                // 利用字符流将 YAML::Node 转换为字符串
                ss.str("");
                ss << item;
                // 递归解析，直到解析到的YAML::Node 为基础数据类型
                config_list.push_back(LexicalCast<std::string, T>()(ss.str()));
            }

            return config_list;
        }
    };

    /**
     * @brief 类型转换模板类偏特化(std::vector<T> 转换成 YAML String)
     */
    template <typename T>
    class LexicalCast<std::vector<T>, std::string>
    {
    public:
        std::string operator()(const std::vector<T> &source)
        {
            YAML::Node node;
            for (const auto &item : source)
            {
                // 递归调用 LexicalCast 解析
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(item)));
            }
            // 利用字符流将 YAML::Node 转换为字符串
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
     * @brief 类型转换模板类偏特化(YAML String 转换成 std::list<T>)
     */
    template <typename T>
    class LexicalCast<std::string, std::list<T>>
    {
    public:
        std::list<T> operator()(const std::string &source)
        {
            YAML::Node node = YAML::Load(source);
            std::list<T> config_list;

            std::stringstream ss;
            for (const auto &item : node)
            {
                ss.str("");
                ss << item;
                config_list.push_back(LexicalCast<std::string, T>()(ss.str()));
            }

            return config_list;
        }
    };

    /**
     * @brief 类型转换模板类偏特化(std::list<T> 转换成 YAML String)
     */
    template <typename T>
    class LexicalCast<std::list<T>, std::string>
    {
    public:
        std::string operator()(const std::list<T> &source)
        {
            YAML::Node node;
            for (const auto &item : source)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(item)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
     * @brief 类型转换模板类偏特化(YAML String 转换成 std::unordered_set<T>)
     */
    template <typename T>
    class LexicalCast<std::string, std::unordered_map<std::string, T>>
    {
    public:
        std::unordered_map<std::string, T> operator()(const std::string &source)
        {
            YAML::Node node = YAML::Load(source);
            std::unordered_map<std::string, T> config_map;
            if (node.IsMap())
            {
                std::stringstream ss;
                for (const auto &item : node)
                {
                    ss.str("");
                    ss << item.second;
                    config_map.insert(std::make_pair(
                        item.first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
                }
            }
            return config_map;
        }
    };

    /**
     * @brief 类型转换模板类偏特化(std::unordered_set<T> 转换成 YAML String)
     */
    template <typename T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_map<std::string, T> &source)
        {
            YAML::Node node;
            for (const auto &item : source)
            {
                node[item.first] = YAML::Load(LexicalCast<T, std::string>()(item.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
     * @brief 类型转换模板类偏特化(YAML String 转换成 std::set<T>)
     */
    template <typename T>
    class LexicalCast<std::string, std::set<T>>
    {
    public:
        std::set<T> operator()(const std::string &source)
        {
            YAML::Node node = YAML::Load(source);
            std::set<T> config_set;
            if (node.IsSequence())
            {
                std::stringstream ss;
                for (const auto &item : node)
                {
                    ss.str("");
                    ss << item;
                    config_set.insert(LexicalCast<std::string, T>()(ss.str()));
                }
            }
            return config_set;
        }
    };

    /**
     * @brief 类型转换模板类偏特化(std::set<T> 转换成 YAML String)
     */
    template <typename T>
    class LexicalCast<std::set<T>, std::string>
    {
    public:
        std::string operator()(const std::set<T> &source)
        {
            YAML::Node node;
            for (const auto &item : source)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(item)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
     * @brief 通用型配置项的模板类
     * @details 模板参数:
     *      T               配置项的值的类型
     *      ToStringFN      {functor<std::string(T&)>} 将配置项的值转换为 std::string
     *      FromStringFN    {functor<T(const std::string&)>} 将 std::string 转换为配置项的值
     */
    template <class T, class ToStringFN = LexicalCast<T, std::string>, class FromStringFN = LexicalCast<std::string, T>>
    class ConfigerVar : public ConfigerVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigerVar>;
        using RWMutexType = RWMutex;
        using onChangeCallBack = std::function<void(const T &old_val, const T &new_val)>;
        static ptr Create(const std::string &name, const T &default_val, const std::string &description = "")
        {
            return std::make_shared<ConfigerVar>(name, default_val, description);
        }

    public:
        /**
         * @brief 通过参数名,参数值,描述构造ConfigerVar
         * @param name 参数名称
         * @param default_value 参数的默认值
         * @param description 参数的描述
         */
        ConfigerVar(const std::string &name, const T &default_val, const std::string &description = "")
            : ConfigerVarBase(name, description), m_val(default_val) {}

        /**
         * @brief 获取当前参数的值
         */
        T getValue()
        {
            RWMutexType::ReadLock lock(m_mutex);
            return m_val;
        }

        /**
         * @brief 设置当前参数的值
         * @details 如果参数的值有发生变化,则通知对应的注册回调函数
         */
        void setValue(const T v)
        {
            {
                RWMutexType::ReadLock lock(m_mutex);
                if (v == m_val)
                {
                    return;
                }
                for (auto &i : m_callback_map)
                {

                    i.second(m_val, v);
                }
            }
            RWMutexType::WriteLock lock(m_mutex);
            m_val = v;
        }

        /**
         * @brief 将参数值转换成YAML String
         * @exception 当转换失败抛出异常
         */
        std::string toString() override
        {
            try
            {
                return ToStringFN()(getValue()); // 将配置转换成字符串类型
            }
            catch (const std::exception &e)
            {
                std::cerr << "ConfigerVar::toString exception" << e.what() << " convert:" << typeid(getValue()).name() << "to string"<<std::endl;
            }
            return "";
        }

        /**
         * @brief 从YAML String 转成参数的值
         * @exception 当转换失败抛出异常
         */
        bool fromString(const std::string &val) override
        {
            try
            {
                setValue(FromStringFN()(val)); // 将字符串转换成配置类型
                return true;
            }
            catch (const std::exception &e)
            {
                std::cerr << "ConfigerVar::fromString exception" << e.what() << " convert:string to" << typeid(getValue()).name()<<std::endl;
            }
            return false;
        }

        /**
         * @brief 添加变化回调函数
         * @return 返回该回调函数对应的唯一id,用于删除回调
         */
        uint64_t addListener(onChangeCallBack callback)
        {
            static uint64_t s_func_id = 0;
            RWMutexType::WriteLock lock(m_mutex);
            ++s_func_id;
            m_callback_map[s_func_id] = callback;
            return s_func_id;
        }
        /**
         * @brief 删除回调函数
         * @param key 回调函数的唯一id
         */
        void delListener(uint64_t key)
        {
            RWMutexType::WriteLock lock(m_mutex);
            m_callback_map.erase(key);
        }
        /**
         * @brief 清理所有的回调函数
         */
        void clearListener()
        {
            RWMutexType::WriteLock lock(m_mutex);
            m_callback_map.clear();
        }
        /**
         * @brief 获取回调函数
         * @param key 回调函数的唯一id
         * @return 如果存在返回对应的回调函数,否则返回nullptr
         */
        onChangeCallBack getListener(uint64_t key)
        {
            RWMutexType::ReadLock lock(m_mutex);
            auto it = m_callback_map.find(key);
            return it == m_callback_map.end() ? nullptr : it->second;
        }

    private:
        T m_val;
        std::unordered_map<uint64_t, onChangeCallBack> m_callback_map; // 变更回调函数
        RWMutexType m_mutex;
    };

    /**
     * @brief ConfigerVar的管理类
     * @details 提供便捷的方法创建/访问ConfigerVar
     */
    class Configer
    {
    public:
        using ConfigerVarMap = std::unordered_map<std::string, ConfigerVarBase::ptr>;
        using RWMutexType = RWMutex;

        /**
         * @brief 获取/创建对应参数名的配置参数
         * @param name 配置参数名称
         * @param default_value 参数默认值
         * @param description 参数描述
         * @details 获取参数名为name的配置参数,如果存在直接返回
         *          如果不存在,创建参数配置并用default_value赋值
         * @return 返回对应的配置参数,如果参数名存在但是类型不匹配则返回nullptr
         * @exception 如果参数名包含非法字符 抛出异常 std::invalid_argument
         */
        template <class T>
        static typename ConfigerVar<T>::ptr Lookup(const std::string &name, const T &default_value, const std::string &description = "")
        {
            auto tmp = Lookup<T>(name);
            if (tmp)
            {
                std::cout << "Lookup name=" << name << "exists"<<std::endl;
                return tmp;
            }
            // 检验名字是否合法
            std::regex pattern("^[\\w.].*|");
            if (std::regex_match(name, pattern))
            {
                typename ConfigerVar<T>::ptr v = ConfigerVar<T>::Create(name, default_value, description);
                AddConfigerVar<T>(name, v);
                return v;
            }
            else
            {
                std::cerr << "Lookup name invalid" << name<<std::endl;
                throw std::invalid_argument(name);
            }
        }

        /**
         * @brief 查找配置参数
         * @param name 配置参数名称
         * @return 返回配置参数名为name的配置参数
         */
        template <class T>
        static typename ConfigerVar<T>::ptr Lookup(const std::string &name)
        {
            RWMutexType::ReadLock lock(GetMutex());
            auto it = GetConfigers().find(name);
            if (it == GetConfigers().end())
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigerVar<T>>(it->second); // 将基类指针转换
        }

        /**
         * @brief 使用YAML::Node初始化配置模块
         */
        static void LoadFromYaml(const YAML::Node &yaml_file);
        /**
         * @brief 使用文件路径初始化配置模块
         */
        static void LoadFromYaml(const std::string &file);

        /**
         * @brief 查找配置参数,返回配置参数的基类
         * @param name 配置参数名称
         */
        static ConfigerVarBase::ptr LookupBase(const std::string &name);

        /**
         * @brief 遍历配置模块里面所有配置项
         * @param callback 配置项回调函数
         */
        static void Visit(std::function<void(ConfigerVarBase::ptr)> callback);

    private:
        /**
         * @brief 添加配置
         */
        template <class T>
        static void AddConfigerVar(const std::string &name, typename ConfigerVar<T>::ptr &v)
        {
            RWMutexType::WriteLock lock(GetMutex());
            GetConfigers()[name] = v;
        }

        /**
         * @brief 返回所有的配置项
         */
        static ConfigerVarMap &GetConfigers()
        {
            static ConfigerVarMap s_configs;
            return s_configs;
        }

        /**
         * @brief 配置项的RWMutex
         */
        static RWMutexType &GetMutex()
        {
            static RWMutexType s_mutex;
            return s_mutex;
        }

        // 将Yaml配置文件解析
        static void analysisYaml(const std::string &prefix, const YAML::Node &node, std::list<std::pair<std::string, const YAML::Node>> &output);
        // 根据正则递归解析
        static void recursiveAnalysis(const std::regex &pattern, const std::string &prefix, const YAML::Node &node, std::list<std::pair<std::string, const YAML::Node>> &output);
    };

}
