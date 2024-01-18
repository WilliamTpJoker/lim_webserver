#include "base/Configer.h"

namespace lim_webserver
{
    void Configer::analysisYaml(const std::string &prefix, const YAML::Node &node, std::list<std::pair<std::string, const YAML::Node>> &output)
    {
        std::regex pattern("^[\\w.].*|");
        recursiveAnalysis(pattern, prefix, node, output);
    }

    void Configer::recursiveAnalysis(const std::regex &pattern,
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
            std::cerr << "Lookup name invalid" << prefix<<std::endl;
            throw std::invalid_argument(prefix);
        }
        // 为映射则后续还有配置
        if (node.IsMap())
        {
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                recursiveAnalysis(pattern, prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
            }
        }
    }

    void Configer::LoadFromYaml(const YAML::Node &root)
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

            ConfigerVarBase::ptr var = LookupBase(key);
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

    void Configer::LoadFromYaml(const std::string &file)
    {
        std::cout << "Load yaml config from " << file << std::endl;
        YAML::Node r = YAML::LoadFile(file);
        Configer::LoadFromYaml(r);
    }

    ConfigerVarBase::ptr Configer::LookupBase(const std::string &name)
    {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetConfigers().find(name);
        return it == GetConfigers().end() ? nullptr : it->second;
    }

    void Configer::Visit(std::function<void(ConfigerVarBase::ptr)> callback)
    {
        RWMutexType::ReadLock lock(GetMutex());
        ConfigerVarMap &m = GetConfigers();
        for (auto it = m.begin(); it != m.end(); ++it)
        {
            callback(it->second);
        }
    }

}