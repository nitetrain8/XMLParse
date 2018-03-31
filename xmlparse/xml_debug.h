#pragma once

#ifndef XML_DEBUG_H
#define XML_DEBUG_H

#ifndef NDEBUG

template<typename T>
void xml_debug(msg) {
    std::cout << msg << std::endl;
}
#else 
#define xml_debug(msg)
#endif // XML_DEBUG_H
#endif //NDEBUG
