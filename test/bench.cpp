#include "SpikeLog.h"

using namespace lim_webserver;

void bench(Logger::ptr logger, size_t thr_num, size_t msg_num, size_t msg_len)
{
    std::cout << "测试日志器:" << logger->getName() << std::endl;
    std::cout << "测试日志：" << msg_num << "条，总大小：" << (msg_num * msg_len) / 1024 << "KB" << std::endl;
    // 2.组织指定长度的日志消息
    std::string msg(msg_len - 1, 'w'); //-1是想要在每条日志消息最后填一个换行
    // 3.创建指定数量的线程
    std::vector<std::thread> threads;
    std::vector<double> cost_arry(thr_num); // 用来记录每条线程处理日志消息的所用时间
    size_t msg_per_thr = msg_num / thr_num; // 每个线程需要输出的日志数量=日志总量/线程总量,这里不准确，存在不能整除，这里只为观察现象，因此不作为重点处理，
    for (int i = 0; i < thr_num; i++)
    {
        // emplace_back是vector提供的操作，功能是在vector已有的空间基础上直接构造并尾插
        threads.emplace_back([&, i]()
                             {
     //线程函数内部开始计时
     auto start =std::chrono::high_resolution_clock::now();
     //开始循环写日志
    for(int j=0;j<msg_per_thr;j++){
        LOG_INFO(logger)<<msg;
    }
    //线程内部结束计时
    auto end=std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> cost=end-start;
    cost_arry[i]=cost.count();
    std::cout<<"线程"<<i<<":"<<"\t输出数量"<<msg_per_thr<<" 耗时:"<<cost.count()<<"s"<<std::endl; });
    }
    for (int i = 0; i < thr_num; i++)
    {
        threads[i].join();
    }
    // 4.计算总耗时：在多线程中，每个线程都会耗费时间，但是线程是并发处理的，因此耗时最高的那个就是总时间
    double max_cost = cost_arry[0];
    for (int i = 0; i < thr_num; i++)
    {
        max_cost = max_cost < cost_arry[i] ? cost_arry[i] : max_cost;
    }
    size_t msg_per_sec = msg_num / max_cost;                       // 每秒处理的日志消息数量
    size_t size_per_sec = (msg_num * msg_len) / (max_cost * 1024); // 每秒处理的日志总大小
    // 打印测试结果
    std::cout << "每秒输出日志数量：" << msg_per_sec << "条" << std::endl;
    std::cout << "每秒输出日志大小：" << size_per_sec << "KB" << std::endl;
    std::cout << std::endl;
}

void sync_bench()
{
    Logger::ptr logger = LOG_NAME("sync_logger");

    FileAppender::ptr fappender = AppenderFactory::Create<FileAppender>();
    fappender->setFile("/home/book/Webserver/log/stress_log.txt");
    fappender->setName("file_test");
    fappender->setFormatter("%m%n");
    fappender->setAppend(false);
    fappender->setLevel(LogLevel_DEBUG);
    fappender->start();

    logger->addAppender(fappender);

    bench(logger, 1, 1000000, 100);
    bench(logger, 3, 1000000, 100);
    // bench(logger, 10, 1000000, 100);
    fappender->stop();
}
void async_bench()
{
    Logger::ptr logger = LOG_NAME("async_logger");

    FileAppender::ptr fappender = AppenderFactory::Create<FileAppender>();
    fappender->setFile("/home/book/Webserver/log/stress_log.txt");
    fappender->setName("file_test");
    fappender->setFormatter("%m%n");
    fappender->setAppend(false);
    fappender->setLevel(LogLevel_DEBUG);
    fappender->start();

    AsyncAppender::ptr asy_appender = AppenderFactory::Create<AsyncAppender>();
    asy_appender->bindAppender(fappender);
    asy_appender->setInterval(2);
    asy_appender->start();

    logger->addAppender(asy_appender);
    bench(logger, 1, 1000000, 100);
    bench(logger, 3, 1000000, 100);
    bench(logger, 10, 1000000, 100);
    asy_appender->stop();
}

int main()
{
    sync_bench();
    async_bench();
    return 0;
}