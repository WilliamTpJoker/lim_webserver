#include "Scheduler1.h"

#include <iostream>
#include <unistd.h>

using namespace lim_webserver;

void run_in_co()
{
    std::cout<<"Hello world"<<std::endl;
}

int main()
{   
    g_Scheduler->start(1);
    g_Scheduler->createTask(&run_in_co);
    sleep(10);
    return 0;
}