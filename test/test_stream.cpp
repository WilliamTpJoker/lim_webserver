#include "logstream.h"

#include <iostream>

void defaultOutput(const char* msg, int len)
{
  size_t n = fwrite(msg, 1, len, stdout);
  //FIXME check n
  (void)n;
}

void defaultFlush()
{
  fflush(stdout);
}

void test_stream()
{
    lim_webserver::LogStream ls;
    ls<<"dsfsadfasdfasdfasa\n";
    defaultOutput(ls.buffer().data(),ls.buffer().length());
    defaultFlush();
    ls<<"dfsdfsdfsdfsdfsd";
    std::cout<<ls.buffer().toString()<<std::endl;
}

int main(int argc, char* args[])
{
    test_stream();
    return 0;
}