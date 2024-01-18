#include "net/ByteArray.h"
#include "splog/splog.h"


namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_NAME("system");

ByteArray::Node::Node(size_t s)
    :ptr(new char[s])
    ,next(nullptr)
    ,size(s) {
}

ByteArray::Node::Node()
    :ptr(nullptr)
    ,next(nullptr)
    ,size(0) {
}

ByteArray::Node::~Node() {
    if(ptr) {
        delete[] ptr;
    }
}
} // namespace lim_webserver
