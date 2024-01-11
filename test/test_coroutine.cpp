#include "Scheduler.h"

#include <iostream>
#include <unistd.h>

using namespace lim_webserver;

void run_in_co()
{
    std::cout<<"Hello world"<<std::endl;
}

void test_schduler()
{
    g_Scheduler->start(1);
    for(int i=0;i<10;++i)
    {
        g_Scheduler->createTask(&run_in_co);
    }
    sleep(1);
}

int main()
{   
    test_schduler();
    return 0;
}