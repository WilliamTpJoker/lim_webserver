#include "TriggeringPolicy.h"

namespace lim_webserver
{
    bool SizeBasedTriggeringPolicy::isTriggeringMessage(FileSink::ptr file, LogMessage::ptr message)
    {
        return file->getFileSize()>=m_maxFileSize->getSize();
    }

    void SizeBasedTriggeringPolicy::setMaxFileSize(long size)
    {
        m_maxFileSize = FileSize::Create(size);
    }

    const FileSize::ptr &SizeBasedTriggeringPolicy::getMaxFileSize()
    {
        return m_maxFileSize;
    }
} // namespace lim_webserver
