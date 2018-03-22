#pragma once


#ifndef A_SPRINTF_H
#define A_SPRINTF_H
#include <stdarg.h>
#include <string>
#include "except.h"

namespace asprintf
{
    class ASPrintfError : public Exception<ASPrintfError> {
    public:
        EXC_BODY(ASPrintfError)
    };

    class StringEncodingError : public ASPrintfError {
    public:
        EXC_BODY_BASE(StringEncodingError, ASPrintfError)
    };

    class StringFormattingError : public ASPrintfError {
    public:
        EXC_BODY_BASE(StringFormattingError, ASPrintfError)
    };
    char *vasprintf(const char *fmt, va_list ap);
    char *asprintf(const char *fmt, ...);
    char *asprintf(std::string *fmt, ...);
    std::string asprintf2(const char *fmt, ...);
    std::string asprintf2(std::string *fmt, ...);

    template<typename T>
    void asprintf3_inner(const char *&fmt, std::stringstream &ss, T t) {
        asprintf3_one_arg(fmt, ss, t);
        if (!*fmt) {
            return;
        }
        while (char c = *fmt++) {
            ss << c;
        }
        return;
    }

    template<typename T, typename ...Args>
    void asprintf3_inner(const char *&fmt, std::stringstream &ss, T t, Args...args) {
        asprintf3_one_arg(fmt, ss, t);
        return asprintf3_inner(fmt, ss, args...);
    }

    template<typename T>
    void asprintf3_one_arg(const char *&fmt, std::stringstream &ss, T t) {
        while (char c = *fmt++) {
            switch (c) {
                case '\0':
                    --fmt;
                    return;
                case '%':
                    switch (*fmt++) {
                        case '\0':
                            throw PyExc_Create(StringFormattingError, "Invalid format");
                        case '%':
                            ss << '%';
                            break;
                        case 's':
                        case 'r':
                        case 'o':
                            ss << t;
                            return;
                    }
                default:
                    ss << c;
            }
        }
    }

    template<typename ...Args>
    std::string asprintf3(const char *fmt, Args... args) {
        std::stringstream ss{};
        asprintf3_inner(fmt, ss, args...);
        return ss.str();
    }

    template<typename ...Args>
    std::string asprintf3(std::string &fmt, Args... args) {
        return asprintf3(fmt.c_str(), args...);
    }
}
#endif // A_SPRINTF_H