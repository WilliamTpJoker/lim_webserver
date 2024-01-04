#include <iostream>
#include <chrono>
#include <sys/time.h>

#include "SpikeLog.h"

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

private:
    long long microseconds_;
    static const int64_t kMicroSecondsPerSecond = 1000000;
};

void measureTime(const char *method, Timestamp (*getTime)())
{
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000000; ++i)
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
}

void test_curTime()
{
    TimeStamp ts = TimeStamp::now();
    std::cout<<ts.toString()<<std::endl;
    std::cout<<ts.toFormattedString(true)<<std::endl;
}

int main()
{
    measureTime("system_clock", Timestamp::now_system_clock);
    measureTime("high_resolution_clock", Timestamp::now_high_resolution_clock);
    measureTime("gettimeofday", Timestamp::now_gettimeofday);

    test_time();

    test_curTime();

    return 0;
}