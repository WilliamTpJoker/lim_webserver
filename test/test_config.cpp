#include "SpikeLog.h"
#include "LogInitializer.h"

using namespace lim_webserver;

typename ConfigerVar<int>::ptr g_int_value_config = Configer::Lookup("system.port", (int)8080, "system port");

typename ConfigerVar<std::vector<int>>::ptr g_int_vec_value_config = Configer::Lookup("system.inc_vec", std::vector<int>{1, 2}, "system port");

// typename ConfigerVar<LogConfigerDefine>::ptr g_defines = Configer::Lookup("logconfig", LogConfigerDefine(), "logs config");


void test_yaml()
{
    YAML::Node r = YAML::LoadFile("home/Webserver/config/log.yaml");
    auto node = r["logconfig"][0]["loggers"][0]["appender-ref"];
    auto s = node.as<std::vector<std::string>>();
    std::cout<<node<<std::endl;
    for(auto &i:s)
    {
        std::cout<<i<<" ";
    }
    std::cout<<std::endl;
}

// 配置替换测试
void test_config()
{
    // LOG_INFO(LOG_ROOT()) << g_int_value_config->getValue();

    YAML::Node r = YAML::LoadFile("home/book/Webserver/config/log.yaml");
    Configer::LoadFromYaml(r);
    Configer::LoadFromYaml("./config/test.yaml");
    Configer::Visit([](ConfigerVarBase::ptr var){
        std::cout<< "name=" << var->getName()
                                     << " description=" << var->getDescription()
                                     << " value=" << var->toString()<<std::endl;
    });
}

void test_lexical()
{
    auto v = g_int_vec_value_config->getValue();
    for (auto &i : v)
    {
        LOG_INFO(LOG_ROOT()) << "int_vec: " << i;
    }
}

void test_change_callback()
{
    g_int_value_config->addListener([](const int &old_val, const int &new_val)
                                    { std::cout << "old value:" << old_val << ",new value:" << new_val << std::endl; });

    LOG_INFO(LOG_ROOT()) << g_int_value_config->getValue();

    YAML::Node r = YAML::LoadFile("./config/test.yaml");
    Configer::LoadFromYaml(r);

    LOG_INFO(LOG_ROOT()) << g_int_value_config->getValue();
}

void test_visit()
{
    Configer::LoadFromYaml("/home/book/Webserver/config/log.yaml");

    auto f = [](ConfigerVarBase::ptr var)
    {
        LOG_INFO(LOG_ROOT()) << "name=" << var->getName()
                                     << " description=" << var->getDescription()
                                     << " value=" << var->toString();
    };
    Configer::Visit(f);
}

int main(int argc, char **argv)
{
    // test_yaml();
    // test_config();
    // test_lexical();
    // test_change_callback();
    // test_log();
    test_visit();
    return 0;
}
