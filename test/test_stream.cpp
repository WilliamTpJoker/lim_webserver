#include "SpikeLog.h"

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

void test_buffer()
{
  lim_webserver::LogFile output("test.txt");

  std::string longStr(4000, 'X');

  lim_webserver::LogStream stream;
  std::cout<<stream.buffer().avail()<<std::endl;
  stream << longStr;

  lim_webserver::AsyncLog::BufferPtr buf(new lim_webserver::AsyncLog::Buffer);
  buf->bzero();
  for(int i=0;i<1000;++i)
  {
    buf->append(stream.buffer().data(),stream.buffer().length());
  }

  clock_t start = clock();

  std::cout << " length:" << buf->length() << std::endl;
  output.append(buf->data(), buf->length());
  clock_t end = clock();
  printf("%lf\n", (float)(end - start) * 1000 / CLOCKS_PER_SEC);
}

int main(int argc, char *args[])
{
  // test_stream();
  test_buffer();
  return 0;
}