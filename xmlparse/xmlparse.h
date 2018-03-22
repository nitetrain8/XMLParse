#ifndef XML_PARSE_H
#define XML_PARSE_H

#include <vector>
#include "except.h"
#include <memory>

namespace XML
{

    class XMLNode;
    typedef std::shared_ptr<std::vector<XMLNode *>> XMLNodeList;

    class XMLError : public Exception<XMLError> {
        EXC_BODY(XMLError)
    };

    class XMLParseError : public XMLError {
        EXC_BODY_BASE(XMLParseError, XMLError)
    };

    class XMLNode {
    public:
        std::string m_tag;
        std::string m_text;
        std::string m_tail;
        XMLNodeList m_children;

        XMLNode(void) : m_tag(""),
            m_text(""),
            m_tail("")
        {
        }
        template<typename A, typename B, typename C>
        XMLNode(A ptag, B ptext, C ptail) :
            m_tag(ptag),
            m_text(ptext),
            m_tail(ptail)
        {
        }
#define PAIR(typ) const char *p##typ, int l##typ
        XMLNode(PAIR(tag), PAIR(text), PAIR(tail)) :
            m_tag(ptag, ltag),
            m_text(ptext, ltext),
            m_tail(ptail, ltail)

        {
        }
        XMLNode(PAIR(tag), PAIR(text)) :
            m_tag(ptag, ltag),
            m_text(ptext, ltext),
            m_tail("")
        {
        };
#undef PAIR
        std::string &tag(void) {
            return m_tag;
        }
        std::string &text(void) {
            return m_text;
        }
        std::string &tail(void) {
            return m_tail;
        }
        bool has_children(void) {
            return !m_children->empty();
        }

        template<typename ...Args>
        void append(Args && ... args) {
            m_children->emplace(std::forward<Args>...);
        }

        void append(XMLNode *n) {
            m_children->push_back(n);
        }
        auto begin(void) {
            return m_children->begin();
        }
        auto end(void) {
            return m_children->end();
        }
    };

    class XMLRoot : public XMLNode {
    public:
        std::string m_filename;
        XMLNodeList m_children;
        template<typename T>
        XMLRoot(T filename) : XMLNode(),
                                m_filename(filename) 
        {
        };
    };

    XMLRoot *fromstring(const char *s);
}

#endif // !XML_PARSE_H
