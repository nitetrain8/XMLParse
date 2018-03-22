#pragma once

#ifndef MY_EXC_H
#define MY_EXC_H

#include <sstream>


#define EXC_BODY_BASE(cls, super) public: \
                                        template <typename T, typename Q, typename Z> \
                                         cls(T msg, Q func, Z file, int line) : super(msg, func, file, line) \
                                         { \
                                             init(); \
                                         }; \
                                        template <typename Q, typename Z> \
                                         cls(std::string &msg, Q func, Z file, int line) : super(msg, func, file, line) \
                                         { \
                                             init(); \
                                         }; \
                                         virtual ~cls() throw() {};

#define EXC_BODY(cls) EXC_BODY_BASE(cls, Exception)


/*For MSVC, the typeid.name() function returns "class XXX" instead
of just "XXX". Move the pointer forward 6 characters to cut off "class ".*/
#ifdef _MSC_VER
#define CLSNAME(type) (typeid(type).name()+6)
#else
#error Unsupported compiler
#endif

static void _strip_ext(std::string &s) {
    int i = 0;
    size_t n = s.length();
    for (auto it = s.crbegin(); it != s.crend(); ++it, --n, ++i) {
        if (*it == '.') {
            s.erase(n - 1, i + 1);
            return;
        }
    }
}

template<typename T>
static std::string strip_ext(T cstr) {
    std::string s(cstr);
    _strip_ext(s);
    return s;
}

class BaseException : public std::exception {
public:
    BaseException() {};
    virtual ~BaseException() {};
};

template <typename Sub>
class Exception : public BaseException {
public:
    std::string m_msg;
    std::string m_func;
    std::string m_file;
    int         m_lineno;
    template <typename T, typename Q, typename Z>
    Exception(T msg, Q func, Z file, int line) :
        BaseException(),
        m_msg(msg),
        m_func(func),
        m_file(strip_ext(file)),
        m_lineno(line)
    {
        init();
    };
    template <typename Q, typename Z>
    Exception(std::string &msg, Q func, Z file, int line) :
        BaseException(),
        m_msg(msg),
        m_func(func),
        m_file(strip_ext(file)),
        m_lineno(line)
    {
        init();
    };
    virtual ~Exception() throw() {};
    virtual const char *what() const throw() {
        return asdf_exc_helper("%s(%s::%s::%d): %s", CLSNAME(Sub), m_file.c_str(), m_func.c_str(), m_lineno, m_msg.c_str()).c_str();
    }
    void init(void) {};
};


class FileNotFoundError : public Exception<FileNotFoundError> {
    EXC_BODY(FileNotFoundError)
};


#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define Exc_Create(exc, msg) exc(msg, __func__, __FILENAME__, __LINE__)
#define Exc_Create2(exc, fmt, ...) Exc_Create(exc, asdf_exc_helper(fmt, __VA_ARGS__))

/* Funny name to help avoid name conflicts.... */
std::string asdf_exc_helper(const char *fmt, ...);

#endif // !MY_EXC_H
