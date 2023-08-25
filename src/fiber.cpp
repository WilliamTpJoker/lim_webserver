#include "fiber.h"

namespace lim_webserver
{
    Fiber::Fiber(std::function<void()> callback, size_t stacksize)
    {
    }

    Fiber::~Fiber()
    {
    }

    void Fiber::reset(std::function<void()> callback)
    {
    }

    void Fiber::swapIn()
    {
    }

    void Fiber::swapOut()
    {
    }

    Shared_ptr<Fiber> Fiber::GetThis()
    {
        return Shared_ptr<Fiber>();
    }

    void Fiber::YieldToReady()
    {
    }
    void Fiber::YieldToHold()
    {
    }

    uint64_t Fiber::TotalFibers()
    {
        return 0;
    }
    
    void Fiber::MainFunc()
    {
    }
}