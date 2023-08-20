#include "config.h"
#include "log.h"
#include <yaml-cpp/yaml.h>

typename std::shared_ptr<lim_webserver::ConfigVar<int>> g_int_value_config = lim_webserver::Config::Lookup("system.port",(int)8080,"system port");

typename std::shared_ptr<lim_webserver::ConfigVar<std::vector<int>>> g_int_vec_value_config = lim_webserver::Config::Lookup("system.inc_vec",std::vector<int> {1,2},"system port");

void test_yaml()
{   
    YAML::Node r=YAML::LoadFile("./config/log.yaml");
    LIM_LOG_INFO(LIM_LOG_ROOT())<<r;
    
}

//配置替换测试
void test_config()
{
    LIM_LOG_INFO(LIM_LOG_ROOT())<<g_int_value_config->getValue();

    YAML::Node r=YAML::LoadFile("./config/log.yaml");
    lim_webserver::Config::LoadFromYaml(r);

    LIM_LOG_INFO(LIM_LOG_ROOT())<<g_int_value_config->getValue();
}

void test_lexical()
{
    auto v = g_int_vec_value_config->getValue();
    for(auto &i:v){
        LIM_LOG_INFO(LIM_LOG_ROOT())<<"int_vec: "<<i;
    }
}

int main(int argc, char** argv)
{
    // test_yaml();
    test_config();
    test_lexical();

    

    return 0;
}
