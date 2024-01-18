#include <iostream>
#include <chrono>
#include <sys/time.h>

#include "splog/splog.h"
#include "base/TimeStamp.h"

using namespace lim_webserver;

class Timestamp
{
public:
    explicit Timestamp(long long microseconds)
        : microseconds_(microseconds) {}

    long long microseconds() const { return microseconds_; }

    static Timestamp now_system_clock()
    {
        auto currentTimePoint = std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now());
        auto epoch = currentTimePoint.time_since_epoch();
        return Timestamp(epoch.count());
    }

    static Timestamp now_high_resolution_clock()
    {
        auto currentTimePoint = std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now());
        auto epoch = currentTimePoint.time_since_epoch();
        return Timestamp(epoch.count());
    }

    static Timestamp now_gettimeofday()
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t seconds = tv.tv_sec;
        return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
    }

    static Timestamp now_localtime()
    {
        struct tm time_struct;              // 定义存储时间的结构体
        time_t time_l = time(0);            // 获取时间
        localtime_r(&time_l, &time_struct); // 将时间数转换成当地时间格式
        return Timestamp(time_l);
    }

private:
    long long microseconds_;
    static const int64_t kMicroSecondsPerSecond = 1000000;
};

void measureTime(const char *method, Timestamp (*getTime)())
{
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100000; ++i)
    {
        getTime();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Method: " << method << " - Time taken: " << duration.count() << " microseconds" << std::endl;
}

void test_time()
{

    auto currentTimePoint = std::chrono::time_point_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now());
    auto epoch = currentTimePoint.time_since_epoch();
    std::cout << epoch.count() << std::endl;

    auto currentTimePoint2 = std::chrono::time_point_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now());
    auto epoch2 = currentTimePoint2.time_since_epoch().count();
    std::cout << epoch2 << std::endl;

    static const int64_t kMicroSecondsPerSecond = 1000000;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    std::cout << seconds * kMicroSecondsPerSecond + tv.tv_usec << std::endl;

    struct tm time_struct;              // 定义存储时间的结构体
    time_t time_l = time(0);            // 获取时间
    localtime_r(&time_l, &time_struct); // 将时间数转换成当地时间格式
    std::cout << time_l << std::endl;
}

void test_curTime()
{
    TimeStamp ts = TimeStamp::now();
    std::cout << ts.toString() << std::endl;
    std::cout << ts.toFormattedString(true) << std::endl;
}

void test_formatString()
{
    const int iterations = 100000; // 测试迭代次数

    std::string result;

    auto start1 = std::chrono::high_resolution_clock::now();

    // 测试直接调用构造的效率
    for (int i = 0; i < iterations; ++i)
    {
        lim_webserver::TimeStamp ts = lim_webserver::TimeStamp::now();
        result = ts.toFormattedString(false);
    }

    auto end1 = std::chrono::high_resolution_clock::now();

    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
    std::cout << result << std::endl;
    std::cout << "Direct construction duration: " << duration1.count() << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    // 测试经过 TimeManager 处理的效率
    for (int i = 0; i < iterations; ++i)
    {
        result = lim_webserver::TimeManager::GetInstance()->getTimeString(time(0));
    }

    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
    std::cout << result << std::endl;
    // 打印两种实现的耗时
    std::cout << "TimeManager duration: " << duration2.count() << " microseconds" << std::endl;
}

int main()
{
    measureTime("system_clock", Timestamp::now_system_clock);
    measureTime("high_resolution_clock", Timestamp::now_high_resolution_clock);
    measureTime("gettimeofday", Timestamp::now_gettimeofday);
    measureTime("localtime", Timestamp::now_localtime);

    test_time();

    test_curTime();

    test_formatString();

    return 0;
}