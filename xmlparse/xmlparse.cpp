// xmlparse.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "xmlparse.h"
#include "futil.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <inttypes.h>

#define PARSE_ERROR(msg) Exc_Create(XMLParseError, msg)
#define PARSE_ERROR2(msg, ...) Exc_Create2(XMLParseError, msg, __VA_ARGS__)
#define ParseError PARSE_ERROR
#define ParseError2 PARSE_ERROR2
#define ParseError3(msg) ParseError2("%s (Line %d Column %d)", msg, m_line, doc - m_row + 1)
#define ParseError4(msg, ...) ParseError3(asdf_exc_helper(msg, __VA_ARGS__).c_str())

/* I don't know why libxml used explicit index checks in macros to compare strings.
I assume it was for the 0.0001% performance increase. Oh well, I'm not going to
mess with a working formula, even though str_startl works just fine for string 
literals...

*/

/* 2nd arg must be string literal only!!! */
#define str_startl(str, s) (!strncmp(str, s, sizeof(s) - 1))

/* works but slow */
#define str_startp(str, s) (!strncmp(str, s, strlen(s) - 1))

/* Think of these as unrolling the strcmp loop... */
#define str_start1(s, _0) (s[0] == _0)
#define str_start2(s, _0, _1) (str_start1(s, _0) && s[1] == _1)
#define str_start3(s, _0, _1, _2) (str_start2(s, _0, _1) && s[2] == _2)
#define str_start4(s, _0, _1, _2, _3) (str_start3(s, _0, _1, _2) && s[3] == _3)
#define str_start5(s, _0, _1, _2, _3, _4) (str_start4(s, _0, _1, _2, _3) && s[4] == _4)
#define str_start6(s, _0, _1, _2, _3, _4, _5) (str_start5(s, _0, _1, _2, _3, _4) && s[5] == _5)
#define str_start7(s, _0, _1, _2, _3, _4, _5, _6) (str_start6(s, _0, _1, _2, _3, _4, _5) && s[6] == _6)
#define str_start8(s, _0, _1, _2, _3, _4, _5, _6, _7) (str_start7(s, _0, _1, _2, _3, _4, _5, _6) && s[7] == _7)
#define str_start9(s, _0, _1, _2, _3, _4, _5, _6, _7, _8) (str_start8(s, _0, _1, _2, _3, _4, _5, _6, _7) && s[8] == _8)
#define str_start10(s, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9) (str_start9(s, _0, _1, _2, _3, _4, _5, _6, _7, _8) && s[9] == _9)

#define CHECK_EOF(s) if (*s == '\0') {\
                        throw ParseError3("Unexpected EOF"); \
                       }
#define IS_WHITESPACE(c) (c == '\r' || c == '\n' || c == ' ' || c == '\t')

#define NAME_START_CHAR(c) (c == ':' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
#define NAME_CHAR(c) (NameStartChar(c) || c == '-' || c == '.' || c >= '0' && c <= '9')

#define LEN_CAST(l) (static_cast<const unsigned int>(l))

/* These macro patterns manipulate T* doc, T* start, and int len 
to point to values suitable for initializing an std::string = {start, len}.
*/
#define PARSE_VAL(doc, start, len) start = doc + 1; \
                                    parse_val(doc); \
                                    len = static_cast<unsigned int>(doc - start) - 1;

#define PARSE_NAME(doc, start, len) start = doc; \
                                    parse_name(doc); \
                                    len = static_cast<unsigned int>(doc - start);

#define PARSE_ATTVAL(doc, start, len) start = doc; \
                                    parse_attr_val(doc); \
                                    len = static_cast<unsigned int>(doc - start);

#define PARSE_CHARDATA(doc, start, len) start = doc; \
                                    parse_char_data(doc); \
                                    len = static_cast<unsigned int>(doc - start);

#define DMSG(msg) (std::cout << __FILENAME__ << ":" << __LINE__ << " " << msg << std::endl)

#define XML_DECL(s) (s[0] == '<' && s[1] == '?' && (s[2] == 'x' || s[2] == 'X') && \
                                                   (s[3] == 'm' || s[3] == 'M') && \
                                                   (s[4] == 'l' || s[4] == 'L'))


namespace XML
{

    template<typename T>
    static inline bool NameStartChar(T c) {
        return NAME_START_CHAR(c);
    }

    template<typename T>
    static inline bool NameChar(T c) {
        return NAME_CHAR(c);
    }

    static inline bool xml_isversion(char c) {
        return ((c >= '0' && c <= '9') || c == '.');
    }

    template<typename T>
    static inline bool str_startswith(const T *s, const T *what) {
        while (*s == *what) {
            if (!*s) return true;  // strings are equal
            ++s;
            ++what;
        }
        if (*what == 0) return true;  // end of string what, but not end of string s
        return false;
    }

    template<typename T>
    static inline bool str_startswith(const T *s, const T what) {
        return s[0] == what;
    }

    template<typename T>
    static inline int xml_htoi(T c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            return c - 'a' - 10;
        }
        else if (c >= 'A' && c <= 'F') {
            return c - 'A' - 10;
        }
        else {
            return -1;
        }
    }

    template<typename T>
    static inline int xml_htoi(T *s, int base) {
        int32_t rv = 0;
        if (base > 16) {
            throw ParseError("Can't support bases higher than 16");
        }
        while (*s) {
            auto i = xml_htoi(*s);
            if (i == -1) {
                throw ParseError2("bad Hex characer '%c'", *s);
            }
            rv = rv * base + i;
        }
        return rv;
    }

    template<typename T>
    static inline bool xml_ischardata(T c) {
        return c != '<' && c != '&';
    }

    template<typename T>
    inline void XMLParser<T>::parse_eq(const T *&doc) {
        consume_space(doc);
        if (*doc++ != '=') {
            throw ParseError3("Expected '='");
        }
        consume_space(doc);
    }   

    template<typename T>
    XMLParser<T>::XMLParser() : m_line(1),
                    m_root(nullptr),
                    m_start(nullptr),
                    m_row(nullptr)
    {};

    template<typename T>
    XMLRoot *XMLParser<T>::parse_doc(const T *doc) {
        if (m_root) {
            throw ParseError3("Can't parse multiple files with the same parser instance.");
        }
        m_start = doc;
        m_row = doc;
        m_root = new XMLRoot{};

        if (XML_DECL(doc)) {
            parse_decl(doc, m_root);
            consume_whitespace(doc);
        }
        if (*doc++ != '<') {
            throw ParseError2("Expected '<', found '%c'.\n", *(doc - 1));
        }
        parse_elem(*m_root, doc);
        consume_whitespace(doc, 1);
        if (*doc) {
            throw ParseError4("Unexpected junk at end of file: '%c'", *doc);
        }
        return m_root;
    }

    template<typename T>
    void XMLParser<T>::parse_elem(XMLNode &parent, const T *&doc) {

        /* string name = {nstart, nlen} */
        const T *nstart; unsigned int nlen;
        PARSE_NAME(doc, nstart, nlen);
        if (*doc != ' ' && *doc != '>') {
            throw ParseError2("Illegal character found: Asc(%d).", *doc);
        }
        consume_space(doc);

        /* Attrlist attrs */
        auto attrs = new AttrList();
        while (NameStartChar(*doc)) {
            parse_attribute(attrs, doc);
            consume_space(doc);
        }

        /* end of tag and strip whitespace */
        if (*doc++ != '>') {
            throw ParseError3("Expected closing tag.");
        }
        consume_whitespace(doc);
        /* text */
        const T *ts; int tl;
        PARSE_CHARDATA(doc, ts, tl);
        auto n = parent.new_child(nstart, nlen, ts, tl, attrs);
        while (doc[0] == '<' && doc[1] != '/') {
            ++doc;
            parse_elem(*n, doc);
            consume_whitespace(doc);
        }
        if (!str_start2(doc, '<', '/')) {
            throw ParseError3("Expected closing tag");
        }
        doc +=2;
        const T *s = doc;
        while (*++doc != '>'){};
        std::string close_tag{s, (unsigned int) (doc - s)};
        if (n->m_tag != close_tag) {
            throw ParseError4("Invalid closing tag: (expected '%s', got '%s'", n->m_tag.c_str(), close_tag.c_str());
        }
        ++doc;
    }

    template<typename T>
    void XMLParser<T>::parse_char_data(const T*&doc) {
        while (xml_ischardata(*doc)) {doc++;};
        if (!*doc) {
            throw ParseError3("Unexpected EOF.");
        }
    }

    template<typename T>
    void XMLParser<T>::parse_attribute(AttrList *attrs, const T *&doc){
        AttrList &at = *attrs;
        const T *ns = doc; unsigned int nl;
        PARSE_NAME(doc, ns, nl);
        parse_eq(doc);
            
        unsigned int vl;
        T buf[MAX_ATTVAL_LEN];
        vl = parse_attr_val(doc, buf, sizeof(buf) - 1);
        at[{ns, nl}] = {buf, vl};
        ++doc;
    };

    template<typename T>
    int XMLParser<T>::parse_attr_val(const T *&doc, T *buf, int maxlen) {
        const T c = *doc++;
        int nmax = maxlen;
        if (c != '\'' && c != '"') {
            throw ParseError3("Expected ''' or '\"'");
        }
        while (*doc != c && maxlen) {
            switch (*doc) {
                std::cout << __LINE__ << "::" << doc - m_start << " " << *doc << std::endl;
                case '\0':
                    throw ParseError3("EOF parsing attribute value.");
                case '<':
                    throw ParseError3("Unexpected left bracket '<' parsing attribute value.");
                case '&':
                    if (doc[1] == '#') {
                        translate_charref(doc, buf, maxlen);
                    }
                    else if (str_start4(doc, '&', 'l', 't', ';')) {
                        *buf++ = '<'; doc += 4;
                        --maxlen;
                    }
                    else if (str_start4(doc, '&', 'g', 't', ';')) {
                        *buf++ = '>'; doc += 4;
                        --maxlen;
                    }
                    else if (str_start5(doc, '&', 'a', 'm', 'p', ';')) {
                        *buf++ = '&'; doc += 5;
                        --maxlen;
                    }
                    else {
                        /* entity reference */
                        throw ParseError3("Unsupported Entity");
                    }
                    break;
                default:
                    *buf++ = *doc++;
                    --maxlen;
            }
        }
        if (!maxlen) {
            throw ParseError3("Attr value too long.");
        }
        *buf = '\0';
        return nmax - maxlen;
    }

    template<typename T>
    void XMLParser<T>::translate_charref(const T *&doc, T *&buf, int &maxlen) {
        int code = 0;
        int base = 0;
        if (doc[1] == 'x') {
            /* hex */
            doc += 2;
            base = 16;
        }
        else {
            doc += 1;
            base = 10;
        }
        for (;;) {
            int c = xml_htoi(*doc);
            if (c == -1) {
                if (c == ';') {
                    break;
                }
                else if (c == '\0') {
                    throw ParseError3("Unexpected EOF");
                }
                else {
                    throw ParseError4("Unexpected character '%c'", *doc);
                }
            }
            code = code * base + c;
            ++doc;
        }

        if (code < 256) {
            *buf++ = code;
            --maxlen;
        } else {
            /* todo: translate code points :) */
            throw ParseError4("Don't know how to translate code point %d.", code);
        }
    }

    template<typename T>    
    void XMLParser<T>::parse_name(const T *&doc) {
        if (!NameStartChar(*doc)) {
            throw ParseError4("Invalid name start character: '%c'.", *(doc-1));
        }
        while (NameChar(*++doc)){};
    }

    template<typename T>
    void XMLParser<T>::parse_decl(const T *&doc, XMLRoot *root) {
        doc += 5;
        root->m_has_decl = true;
        require_space(doc, 0, "Expected space after '<?xml'.");

        /* next field must be version */
        if (!str_start7(doc, 'v', 'e', 'r', 's', 'i', 'o', 'n')) {
            throw ParseError("Failed to find version\n");
        }
        else {
            doc += 7;
            parse_version(doc, root);
        }
        consume_space(doc);

        if (str_start8(doc, 'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g')) {
            parse_encoding(doc, root);
            consume_space(doc);
        }

        if (str_start10(doc, 's', 't', 'a', 'n', 'd', 'a', 'l', 'o', 'n', 'e')) {
            parse_standalone(doc, root);
            consume_space(doc);
        }

        if (!str_start2(doc, '?', '>')) {
            throw ParseError3("Expected declaration tag to end in '?>')");
        }
        doc += 2;
    }

    template<typename T>
    void XMLParser<T>::parse_standalone(const T *&doc, XMLRoot *root) {
        doc += 10;
        parse_eq(doc);
        const T *start; int len;
        PARSE_VAL(doc, start, len);

        if (len == 3 && ((doc[0] == 'Y' || doc[0] == 'y') &&
                            (doc[1] == 'E' || doc[1] == 'e') &&
                            (doc[2] == 'S' || doc[2] == 's'))) {
            root->m_standalone = 1;
        }
        else if (len == 2 && ((doc[0] == 'N' || doc[0] == 'n') &&
                                (doc[1] == 'O' || doc[1] == 'o'))) {
            root->m_standalone = 0;
        }
        else {
            throw ParseError3("Value for 'standalone' must be 'yes' or 'no'.");
        }
    }

    template<typename T>
    void XMLParser<T>::parse_encoding(const T *&doc, XMLRoot *root) {
        doc += 8;
        const T *start; int len;
        parse_eq(doc);
        PARSE_VAL(doc, start, len);
        root->m_encoding={start, LEN_CAST(len)};            
    }

    template<typename T>
    void XMLParser<T>::parse_version(const T *&doc, XMLRoot *root) {
        consume_space(doc);
        if (*doc++ != '=') {
            throw ParseError("Expected '='");
        }

        consume_space(doc);

        char sc = *doc;
        if (sc != '"' && sc != '\'') {
            throw ParseError("Expected value for version");
        }

        const T *vs = ++doc;
        while (xml_isversion(*doc)) {++doc;};
        if (*doc != sc) {
            throw ParseError("Mismatching quote tags");
        }

        root->m_version = {vs, LEN_CAST(doc - vs)};
        ++doc;
    }
    
    template<typename T>
    void XMLParser<T>::parse_val(const T *&doc) {
        T c = *doc;
        if (c != '"' && c != '\'') {
            throw ParseError("Expected quotes surrounding attribute value");
        }
        while (*++doc != c) {
            if (!*doc) {
                throw ParseError3("Unexpected EOF");
            }
        }
        ++doc; // move past space
    }
    
    template<typename T>
    inline void XMLParser<T>::require_space(const T *&s, int eof_ok, const char *errmsg) {
        if (*s != ' ') {
            throw ParseError(errmsg);
        }
        else {
            consume_space(s, eof_ok);
        }
    }
    
    template<typename T>
    inline void XMLParser<T>::consume_space(const T *&s, int eof_ok) {
        while (*s == ' '){s++;};
        if (*s == '\0' && !eof_ok) {
            throw ParseError("Unexpected EOF");
        }
    }
    
    template<typename T>
    inline void XMLParser<T>::consume_whitespace(const T *&s, int eof_ok) {
        for (;;) {
            switch (*s) {
                case '\r':
                    if (s[1] == '\n') {
                        ++s;
                    } 
                    /* fallthrough */
                case '\n':
                    m_line += 1;
                    m_row = s + 1;
                case ' ':
                case '\t':
                    break;
                case '\0':
                    if (eof_ok) {
                        return;
                    }
                    else {
                        throw ParseError("Unexpected EOF");
                    }
                default:
                    return;
            }
            ++s;
        }
    }

    XMLRoot *fromstring(const char *s){
        XMLParser<char> p{};
        return p.parse_doc(s);
    }
    

    XMLRoot *fromfn(const char *fn) {
        int64_t len;
        const char *s = futil::readc(fn, &len);
        auto rv = fromstring(s);
        delete[] s;
        return rv;
    }

    /* Debugging functions */
    void dump(XMLNode *n, int level = 0) {
        for (auto e : *n) {
            std::string space(level * 4, ' ');
            std::cout << space << '<' << e->m_tag;
            for (auto a : *e->m_attrs) {
                std::cout << ' ' << a.first << '=' << '"' << a.second << '"';
            }
            std::cout << '>';
            std::cout << e->m_text;
            if (e->has_children()) {
                std::cout << '\n';
                dump(e, level + 1);
                std::cout << space;
            }
            std::cout << "</" << e->m_tag << ">\n";
        }
    }

    void dump(XMLRoot *r) {
        if (r->m_has_decl) {
            std::cout << "<?xml version=\"" << r->m_version << '"';
            if (!r->m_encoding.empty())
                std::cout << " encoding=\"" << r->m_encoding << "\"";
            if (r->m_standalone != -1)
                std::cout << " standalone=\"" << (r->m_standalone ? "yes" : "no") << "\"";
            std::cout << "?>\n";
        }
        dump(r, 0);
    }
}


using namespace XML;
int _test(char *fn) {

    XMLRoot *n = nullptr;
    try {
        n = fromfn(fn);
    }
    catch (XMLError &e) {
        std::cout << e.what();
        return -2;
    }
    catch (BaseException &e) {
        std::cout << e.what();
        return -3;
    }
    dump(n);
    delete n;
    return 0;
}

int test(int argc, char **argv) {

    if (argc != 2) {
        std::cout << "Usage: xmlparse.exe xmlfile" << std::endl;
        return -1;
    }

    try {
        return _test(argv[1]);
    }
    catch (BaseException &e) {
        std::cout << e.what();
    }
    return -5;
}