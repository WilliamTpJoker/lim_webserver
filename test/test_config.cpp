#include "config.h"
#include "log.h"
#include <yaml-cpp/yaml.h>

using namespace lim_webserver;

typename std::shared_ptr<ConfigVar<int>> g_int_value_config = Config::Lookup("system.port", (int)8080, "system port");

typename std::shared_ptr<ConfigVar<std::vector<int>>> g_int_vec_value_config = Config::Lookup("system.inc_vec", std::vector<int>{1, 2}, "system port");

void test_yaml()
{
    YAML::Node r = YAML::LoadFile("./config/log.yaml");
    LIM_LOG_INFO(LIM_LOG_ROOT()) << r;
}

// 配置替换测试
void test_config()
{
    LIM_LOG_INFO(LIM_LOG_ROOT()) << g_int_value_config->getValue();

    YAML::Node r = YAML::LoadFile("./config/log.yaml");
    Config::LoadFromYaml(r);

    LIM_LOG_INFO(LIM_LOG_ROOT()) << g_int_value_config->getValue();
}

void test_lexical()
{
    auto v = g_int_vec_value_config->getValue();
    for (auto &i : v)
    {
        LIM_LOG_INFO(LIM_LOG_ROOT()) << "int_vec: " << i;
    }
}

void test_change_callback()
{
    g_int_value_config->addListener(10, [](const int &old_val, const int &new_val)
                                    { std::cout << "old value:" << old_val << ",new value:" << new_val << std::endl; });

    LIM_LOG_INFO(LIM_LOG_ROOT()) << g_int_value_config->getValue();

    YAML::Node r = YAML::LoadFile("./config/log.yaml");
    Config::LoadFromYaml(r);

    LIM_LOG_INFO(LIM_LOG_ROOT()) << g_int_value_config->getValue();
}

void test_log()
{
    static Shared_ptr<Logger> sys_logger = LIM_LOG_NAME("system");
    LIM_LOG_INFO(sys_logger) << "hello system";

    std::cout << LoggerMgr::GetInstance()->toYamlString() << std::endl;

    YAML::Node r = YAML::LoadFile("./config/log.yaml");
    Config::LoadFromYaml(r);
    std::cout << "=====================================" << std::endl;
    std::cout << LoggerMgr::GetInstance()->toYamlString() << std::endl;

    LIM_LOG_INFO(sys_logger) << "hello system";
    sys_logger->setFormatter("%d -%m- %n");
    LIM_LOG_INFO(sys_logger) << "hello system";

    LIM_LOG_DEBUG(LIM_LOG_ROOT()) << "hello root";
}

int main(int argc, char **argv)
{
    // test_yaml();
    // test_config();
    // test_lexical();
    // test_change_callback();
    test_log();
    return 0;
}
