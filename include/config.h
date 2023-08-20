#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "log.h"
#include <regex>
#include <yaml-cpp/yaml.h>
#include <unordered_set>

namespace lim_webserver
{
    class ConfigVarBase
    {
    public:
        ConfigVarBase(const std::string &name, const std::string &description = "")
            : m_name(name), m_description(description) {}
        virtual ~ConfigVarBase() {}

        const std::string &getName() const { return m_name; }
        const std::string &getDescription() const { return m_description; }

        virtual std::string toString() = 0;
        virtual bool fromString(const std::string &val) = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };

    /**
     * @brief YAML格式字符串到其他类型的转换仿函数
     * boost::lexical_cast 的包装，
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
     * @brief YAML格式字符串到其他类型的转换仿函数
     * LexicalCast 的偏特化，针对 std::string 到 std::vector<T> 的转换，
     * 接受可被 YAML::Load() 解析的字符串
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
                ss.str("");
                // 利用 YAML::Node 实现的 operator<<() 将 node 转换为字符串
                ss << item;
                // 递归解析，直到 T 为基本类型
                config_list.push_back(LexicalCast<std::string, T>()(ss.str()));
            }

            return config_list;
        }
    };

    /**
     * @brief YAML格式字符串到其他类型的转换仿函数
     * LexicalCast 的偏特化，针对 std::vector<T> 到 std::string 的转换，
     */
    template <typename T>
    class LexicalCast<std::vector<T>, std::string>
    {
    public:
        std::string operator()(const std::vector<T> &source)
        {
            // 暴力解析，将 T 解析成字符串，在解析回 YAML::Node 插入 node 的尾部，
            YAML::Node node;
            // 最后通过 std::stringstream 与调用 yaml-cpp 库实现的 operator<<() 将 node 转换为字符串
            for (const auto &item : source)
            {
                // 调用 LexicalCast 递归解析，知道 T 为基本类型
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(item)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
     * @brief YAML格式字符串到其他类型的转换仿函数
     * LexicalCast 的偏特化，针对 std::string 到 std::list<T> 的转换，
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
     * @brief YAML格式字符串到其他类型的转换仿函数
     * LexicalCast 的偏特化，针对 std::list<T> 到 std::string 的转换，
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
     * @brief YAML格式字符串到其他类型的转换仿函数
     * LexicalCast 的偏特化，针对 std::string 到 std::unordered_map<std::string, T> 的转换，
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
     * @brief YAML格式字符串到其他类型的转换仿函数
     * LexicalCast 的偏特化，针对 std::unordered_map<std::string, T> 到 std::string 的转换，
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
     * @brief YAML格式字符串到其他类型的转换仿函数
     * LexicalCast 的偏特化，针对 std::string 到 std::unordered_set<T> 的转换，
     */
    template <typename T>
    class LexicalCast<std::string, std::unordered_set<T>>
    {
    public:
        std::unordered_set<T> operator()(const std::string &source)
        {
            YAML::Node node = YAML::Load(source);
            std::unordered_set<T> config_set;
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
     * @brief YAML格式字符串到其他类型的转换仿函数
     * LexicalCast 的偏特化，针对 std::unordered_set<T> 到 std::string 的转换，
     */
    template <typename T>
    class LexicalCast<std::unordered_set<T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_set<T> &source)
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
     * 模板参数:
     *      T               配置项的值的类型
     *      ToStringFN      {functor<std::string(T&)>} 将配置项的值转换为 std::string
     *      FromStringFN    {functor<T(const std::string&)>} 将 std::string 转换为配置项的值
     * */
    template <class T, class ToStringFN = LexicalCast<T, std::string>, class FromStringFN = LexicalCast<std::string, T>>
    class ConfigVar : public ConfigVarBase
    {
    public:
        ConfigVar(const std::string &name, const T &default_val, const std::string &description = "")
            : ConfigVarBase(name, description), m_val(default_val) {}

        T getValue() const { return m_val; }

        std::string toString() override
        {
            try
            {
                return ToStringFN()(m_val); // 将配置转换成字符串类型
            }
            catch (const std::exception &e)
            {
                LIM_LOG_ERROR(LIM_LOG_ROOT()) << "ConfigVar::toString exception" << e.what() << " convert:" << typeid(m_val).name() << "to string";
            }
            return "";
        }

        bool fromString(const std::string &val) override
        {
            try
            {
                m_val = FromStringFN()(val); // 将字符串转换成配置类型
            }
            catch (const std::exception &e)
            {
                LIM_LOG_ERROR(LIM_LOG_ROOT()) << "ConfigVar::fromString exception" << e.what() << " convert:string to" << typeid(m_val).name();
            }
            return false;
        }

    private:
        T m_val;
    };

    class Config
    {
    public:
        using ConfigVarMap = std::unordered_map<std::string, Shared_ptr<ConfigVarBase>>;

        template <class T>
        static typename lim_webserver::Shared_ptr<ConfigVar<T>> Lookup(const std::string &name, const T &default_value, const std::string &description = "")
        {

            auto tmp = Lookup<T>(name);
            if (tmp)
            {
                LIM_LOG_INFO(LIM_LOG_ROOT()) << "Lookup name=" << name << "exists";
                return tmp;
            }
            // 检验名字是否合法
            std::regex pattern("^[\\w.].*|");
            if (std::regex_match(name, pattern))
            {
                typename lim_webserver::Shared_ptr<ConfigVar<T>> v = lim_webserver::MakeShared<ConfigVar<T>>(name, default_value, description);
                GetConfigs()[name] = v;
                return v;
            }
            else
            {
                LIM_LOG_ERROR(LIM_LOG_ROOT()) << "Lookup name invalid" << name;
                throw std::invalid_argument(name);
            }
        }

        template <class T>
        static typename lim_webserver::Shared_ptr<ConfigVar<T>> Lookup(const std::string &name)
        {
            auto it = GetConfigs().find(name);
            if (it == GetConfigs().end())
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second); // 将基类指针转换
        }
        // 从Yaml读取配置
        static void LoadFromYaml(const YAML::Node &yaml_file);

        static Shared_ptr<ConfigVarBase> LookupBase(const std::string &name);

    private:
        static ConfigVarMap &GetConfigs()
        {
            static ConfigVarMap s_configs;
            return s_configs;
        }
        // 将Yaml配置文件解析
        static void analysisYaml(const std::string &prefix, const YAML::Node &node, std::list<std::pair<std::string, const YAML::Node>> &output);
        // 根据正则递归解析
        static void recursive_analysis(const std::regex &pattern, const std::string &prefix, const YAML::Node &node, std::list<std::pair<std::string, const YAML::Node>> &output);
    };

}

#endif