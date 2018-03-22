
#include "except.h"
#include "asprintf.h"

std::string asdf_exc_helper(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char *b = asprintf::vasprintf(fmt, ap);
    va_end(ap);
    std::string s(b);
    delete[] b;
    return s;
}
