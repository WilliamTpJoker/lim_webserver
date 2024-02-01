#include "base/LFQueue.h"
#include <iostream>
#include <thread>

using namespace lim_webserver;

LFQueue<int> newQueue;
LFQueue<int> runableQueue;

void producer()
{
    for (int i = 0; i < 100; ++i)
    {
        if (i == 50)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        newQueue.enqueue(i);
    }
}

void consumer()
{
    for (int i = 0; i < 100; ++i)
    {
        int value;
        while (!newQueue.dequeue(value))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // runableQueue.concatenate(newQueue);
        }
        std::cout << "Dequeued: " << value << std::endl;
    }
}

int main()
{
    std::thread producerThread(producer);
    std::thread consumerThread(consumer);

    producerThread.join();
    consumerThread.join();

    return 0;
}