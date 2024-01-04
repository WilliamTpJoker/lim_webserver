#pragma once

#include <memory>
#include <string>

namespace lim_webserver
{
    class FileSize
    {
    public:
        using ptr = std::unique_ptr<FileSize>;

        static ptr Create(long size=0)
        {
            return std::make_unique<FileSize>(size);
        }
        
    public:
        FileSize(long size) : m_size(size) {}

        long getSize(){return m_size;}

        void addSize(long size){m_size+=size;}

        std::string toString()
        {
            long inKB = m_size / KB_COEFFICIENT;
            if (inKB == 0)
            {
                return m_size + " Bytes";
            }

            long inMB = m_size / MB_COEFFICIENT;
            if (inMB == 0)
            {
                return inKB + " KB";
            }

            long inGB = m_size / GB_COEFFICIENT;
            if (inGB == 0)
            {
                return inMB + " MB";
            }

            return inGB + " GB";
        }

    private:
        const long KB_COEFFICIENT = 1024;
        const long MB_COEFFICIENT = 1024 * KB_COEFFICIENT;
        const long GB_COEFFICIENT = 1024 * MB_COEFFICIENT;

        long m_size=0;
    };
} // namespace lim_webserver
