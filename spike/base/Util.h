#pragma once

#include <string>

namespace lim_webserver
{
    std::string Time2Str(time_t ts = time(0), const std::string &format = "%Y-%m-%d %H:%M:%S");

    class StringUtil
    {
    public:
        static std::string Format(const char *fmt, ...);
        static std::string Formatv(const char *fmt, va_list ap);

        static std::string UrlEncode(const std::string &str, bool space_as_plus = true);
        static std::string UrlDecode(const std::string &str, bool space_as_plus = true);

        static std::string Trim(const std::string &str, const std::string &delimit = " \t\r\n");
        static std::string TrimLeft(const std::string &str, const std::string &delimit = " \t\r\n");
        static std::string TrimRight(const std::string &str, const std::string &delimit = " \t\r\n");

        static std::string WStringToString(const std::wstring &ws);
        static std::wstring StringToWString(const std::string &s);
    };

} // namespace lim_webserver
