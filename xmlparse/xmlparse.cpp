// xmlparse.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "xmlparse.h"
#include "futil.h"
#include <iostream>
#include <memory>

#define PARSE_ERROR(msg) Exc_Create(XMLParseError, msg)
#define PARSE_ERROR2(msg, ...) Exc_Create2(XMLParseError, msg, __VA_ARGS__)
#define ParseError PARSE_ERROR

#define CMP5(a,b,c,d,e)

namespace XML
{
    class XMLParser {
    public:
        XMLParser(){
        };
        template<typename T>
        XMLRoot *parse_doc(T *doc) {
            if (*doc++ != '<') {
                throw ParseError2("Expected '<', found '%s'.\n", *(doc-1));
            }

        }
    };

    void parse_node(XMLNode *parent, const char *&text) {
        for (;;) {
            char c = *text++;
            switch (c) {
                case '\0':
                    throw ParseError("Unexpected EOF");
            }
        }
    }

    template<typename T>
    void xml_dump2(XMLNode *node, T &out, int l = 0) {
        auto &n = *node;
        std::string space(l * 2, ' ');
        if (n.has_children()) {
            out << space << '<' << n.tag() << ">\n";
            for (auto c : n) {
                xml_dump2(c, out, l + 1);
            }
            out << space << "</" << n.tag() << '>';
        }
        else {
            out << asprintf::asprintf3("%s<%s>%s</%s>", space, n.tag(), n.text(), n.tag());
        }
        out << std::endl;
    }

    XMLRoot *fromstring(const char *s){
        XMLParser p{};        
    }

    XMLRoot *fromfn(const char *fn) {
        const char *s = futil::readc(fn);
        return fromstring(s);
    }


    int _main(int argc, char **argv) {
        if (argc != 2) {
            std::cout << "Usage: xmlparse.exe xmlfile" << std::endl;
            return -1;
        }
        XMLNode *n = nullptr;
        try {
            n = fromfn(argv[1]);
        }
        catch (XMLError &e) {
            std::cout << e.what();
            return -2;
        }
        catch (BaseException &e) {
            std::cout << e.what();
            return -3;
        }

        return 0;
    }

}

#undef PARSE_ERROR
#undef PARSE_ERROR2

int main(int argc, char **argv) {
    try {
        return XML::_main(argc, argv);
    }
    catch (BaseException &e) {
        std::cout << e.what();
    }
    return -5;
}