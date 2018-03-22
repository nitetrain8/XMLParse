
#include "futil.h"
namespace futil
{

    // http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
    std::string read(const char *filename)
    {
        std::ifstream in(filename, std::ios::in | std::ios::binary);
        if (in)
        {
            std::string contents;
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
            return(contents);
        }
        throw(Exc_Create(FileNotFoundError, filename));
    }

    std::string read(std::string &s)
    {
        return read(s.c_str());
    }

    char *readc(const char *fn) {
        std::ifstream in(fn, std::ios::in | std::ios::binary);
        if (in) {
            in.seekg(0, std::ios::end);
            int64_t sz = in.tellg();
            in.seekg(0, std::ios::beg);
            char *buf = new char[sz + 1];
            in.read(buf, sz);
            buf[sz] = '\0';
            return buf;
        }
        throw (Exc_Create(FileNotFoundError, fn));
    }

    char *readc(std::string &s) {
        return readc(s.c_str());
    }
}