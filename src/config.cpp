#include "config.h"

namespace lim_webserver
{

    void Config::analysisYaml(const std::string &prefix, const YAML::Node &node, std::list<std::pair<std::string, const YAML::Node>> &output)
    {
        std::regex pattern("^[\\w.].*|");
        recursive_analysis(pattern, prefix, node, output);
    }

    void Config::recursive_analysis(const std::regex &pattern,
                                    const std::string &prefix,
                                    const YAML::Node &node,
                                    std::list<std::pair<std::string, const YAML::Node>> &output)
    {

        if (std::regex_match(prefix, pattern))
        {
            output.emplace_back(std::make_pair(prefix, node));
        }
        else
        {
            LIM_LOG_ERROR(LIM_LOG_ROOT()) << "Lookup name invalid" << prefix;
            throw std::invalid_argument(prefix);
        }
        // 为映射则后续还有配置
        if (node.IsMap())
        {
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                recursive_analysis(pattern, prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
            }
        }
    }

    void Config::LoadFromYaml(const YAML::Node &root)
    {
        std::list<std::pair<std::string, const YAML::Node>> all_nodes;
        analysisYaml("", root, all_nodes);

        for (auto &i : all_nodes)
        {
            std::string &key = i.first; // config参数
            if (key.empty())
            {
                continue;
            }

            ConfigVarBase::ptr var = LookupBase(key);
            if (var)
            {
                // 判定为标量则转化
                if (i.second.IsScalar())
                {
                    var->fromString(i.second.Scalar());
                }
                else
                {
                    std::stringstream ss;
                    ss << i.second;
                    var->fromString(ss.str());
                }
            }
        }
    }

    void Config::LoadFromYaml(const std::string &file)
    {
        std::cout << "Load yaml config from " << file << std::endl;
        YAML::Node r = YAML::LoadFile(file);
        Config::LoadFromYaml(r);
    }

    ConfigVarBase::ptr Config::LookupBase(const std::string &name)
    {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetConfigs().find(name);
        return it == GetConfigs().end() ? nullptr : it->second;
    }

    void Config::Visit(std::function<void(ConfigVarBase::ptr)> callback)
    {
        RWMutexType::ReadLock lock(GetMutex());
        ConfigVarMap &m = GetConfigs();
        for (auto it = m.begin(); it != m.end(); ++it)
        {
            callback(it->second);
        }
    }

}