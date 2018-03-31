
#include "except.h"
#include "asprintf.h"

std::string asdf_exc_helper(const char *fmt, ...) {
    /* it turned out that this function is identical to 
    asprintf2, but has to be defined here because asprintf.h
    is required by except.h. The result is a three-way dependency
    but avoids the circular import issue by delaying definition
    of this function to compile time rather than preprocessing. 
    */
    va_list ap;
    va_start(ap, fmt);
    char *b = asprintf::vasprintf(fmt, ap);
    va_end(ap);
    std::string s(b);
    delete[] b;
    return s;
}
