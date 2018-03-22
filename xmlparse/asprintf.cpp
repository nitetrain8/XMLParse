
#include "asprintf.h"
#include <string>
#include <stdarg.h>
#include <iostream>

namespace asprintf
{

#define StringError(msg) Exc_Create(StringEncodingError, msg)
#define FormattingError(msg) Exc_Create(StringFormattingError, msg)

    char *vasprintf(const char *fmt, va_list ap) {
        char *p;
        int sz = 4096;
        int n = sz;
        va_list ap2;
        while (1) {
            va_copy(ap2, ap);
            p = new char[n];
            n = vsnprintf(p, sz, fmt, ap);
            va_end(ap2);
            if (n < 0) {
                delete[] p;
                throw Exc_Create(StringEncodingError, "encoding error");
            }
            else if (n <= sz) {
                return p;
            }
            delete[] p;
            sz = n;
        }
    }

    char *asprintf(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        char *rv = vasprintf(fmt, ap);
        va_end(ap);
        return rv;
    }

    char *asprintf(std::string *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        char *rv = vasprintf(fmt->c_str(), ap);
        va_end(ap);
        return rv;
    }

    std::string asprintf2(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        char *rv = vasprintf(fmt, ap);
        va_end(ap);
        std::string s(rv);
        //std::cout << rv << std::endl;
        delete[] rv;
        return s;
    }

    std::string asprintf2(std::string *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        char *rv = vasprintf(fmt->c_str(), ap);
        va_end(ap);
        return rv;
    }


} // namespace asprintf