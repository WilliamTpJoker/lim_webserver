#include "net/Socket.h"
#include "net/EventLoop.h"
#include "net/Server.h"
#include "net/Client.h"
#include "net/ByteArray.h"

#define g_net lim_webserver::EventLoop::GetInstance()