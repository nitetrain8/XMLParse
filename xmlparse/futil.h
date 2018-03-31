#pragma once

#ifndef FUTIL_H
#define FUTIL_H
#include <string>
#include <fstream>
#include "except.h"

namespace futil
{
    std::string read(const char *filename);
    std::string read(std::string &filename);
    char *      readc(const char *filename, int64_t *len=nullptr);
    char *      readc(std::string &filename, int64_t *len=nullptr);
}
#endif // !FUTIL_H
