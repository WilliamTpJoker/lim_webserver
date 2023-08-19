#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "log.h"
#include <regex>

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

    template <class T>
    class ConfigVar : public ConfigVarBase
    {
    public:
        ConfigVar(const std::string &name, T &default_val, const std::string &description = "")
            : ConfigVarBase(name, description), m_val(default_val) {}

        std::string toString() override
        {
            try
            {
                boost::lexical_cast<std::string>(m_val); // 将配置转换成字符串类型
            }
            catch (const std::exception &e)
            {
                // LIM_LOG_ERROR(LIM_LOG_ROOT()) << "ConfigVar::toString exception" << e.what()
                //                               << " convert:" << typeid(m_val).name() << "to string";
            }
            return "";
        }

        bool fromString(const std::string &val) override
        {
            try
            {
                m_val = boost::lexical_cast<T>(val); // 将字符串转换成配置类型
            }
            catch (const std::exception &e)
            {
                // LIM_LOG_ERROR(LIM_LOG_ROOT()) << "ConfigVar::fromString exception" << e.what()
                //                               << " convert:string to" << typeid(m_val).name();
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
                // LIM_LOG_INFO(LIM_LOG_ROOT()) << "Lookup name=" << name << "exists";
                return tmp;
            }
            //检验名字是否合法
            std::regex pattern("^[\\w.]+");
            if (std::regex_match(name, pattern))
            {
                typename lim_webserver::Shared_ptr<ConfigVar<T>> v = lim_webserver::MakeShared<ConfigVar<T>>(name, default_value, description);
                s_configs[name] = v;
            }
            else
            {
                // LIM_LOG_ERROR(LIM_LOG_ROOT())<<"Lookup name invalid"<< name;
                throw std::invalid_argument(name);
            }
        }

        template <class T>
        static typename lim_webserver::Shared_ptr<ConfigVar<T>> Lookup(const std::string &name)
        {
            auto it = s_configs.find(name);
            if (it == s_configs.end())
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second); // 将基类指针转换
        }

    private:
        static ConfigVarMap s_configs;
    };

}

#endif