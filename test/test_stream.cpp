#include "splog.h"

#include <iostream>

void defaultOutput(const char *msg, int len)
{
  size_t n = fwrite(msg, 1, len, stdout);
  // FIXME check n
  (void)n;
}

void defaultFlush()
{
  fflush(stdout);
}

void test_stream()
{
  lim_webserver::LogStream ls;
  ls << "this is a test stream\n";
  defaultOutput(ls.buffer().data(), ls.buffer().length());
  lim_webserver::LogStream ls2;
  ls2 << ls.buffer().toString();
  defaultOutput(ls2.buffer().data(), ls2.buffer().length());
}

int main(int argc, char *args[])
{
  test_stream();
  return 0;
}