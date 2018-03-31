#ifndef XML_PARSE_H
#define XML_PARSE_H

#include <vector>
#include "except.h"
#include <memory>
#include <unordered_map>
#include <utility>

/* max length for attribute values, else error */
#define MAX_ATTVAL_LEN 1024 

namespace XML
{

    class XMLNode;
    typedef std::vector<XMLNode *> XMLNodeList;
    typedef std::shared_ptr<XMLNodeList> SPXMLNodeList;
    typedef std::unordered_map<std::string, std::string> AttrList;
    typedef std::shared_ptr<AttrList> SPAttrList;
 

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
        SPXMLNodeList m_children;
        SPAttrList    m_attrs;

        XMLNode() : m_tag(""),
                    m_text(""),
                    m_tail(""),
                    m_children(new XMLNodeList()),
                  m_attrs(new AttrList())
        {};

        template<typename A, typename B, typename C>
        XMLNode(A ptag, B ptext, C ptail) :
            m_tag(ptag),
            m_text(ptext),
            m_tail(ptail),
            m_children(new XMLNodeList()),
            m_attrs(new AttrList())
        {
        }
#define PAIR(typ) const char *p##typ, int l##typ
        XMLNode(PAIR(tag), PAIR(text), PAIR(tail)) :
            m_tag(ptag, ltag),
            m_text(ptext, ltext),
            m_tail(ptail, ltail),
            m_children(new XMLNodeList()),
            m_attrs(new AttrList())
        {
        }
        XMLNode(PAIR(tag), PAIR(text)) :
            m_tag(ptag, ltag),
            m_text(ptext, ltext),
            m_tail(""),
            m_children(new XMLNodeList()),
            m_attrs(new AttrList())
        {
        }
        XMLNode(PAIR(tag), PAIR(text), AttrList *attrs) :
            m_tag(ptag, ltag),
            m_text(ptext, ltext),
            m_tail(""),
            m_children(new XMLNodeList()),
            m_attrs(attrs) 
        {

        }

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
        XMLNode *new_child(Args && ... args) {
            XMLNode *n = new XMLNode(std::forward<Args>(args)...);
            m_children->push_back(n);
            return n;
        }

        inline auto begin(void) {
            return m_children->begin();
        }
        inline auto end(void) {
            return m_children->end();
        }
    };

    class XMLRoot : public XMLNode {
    public:
        std::string m_version;
        std::string m_encoding;
        int         m_standalone;
        bool        m_has_decl;

        XMLRoot(void) : XMLNode(), 
                        m_version(""),
                        m_encoding(""),
                        m_standalone(-1),
                        m_has_decl(0)
        {
        };
    };
    
    template<typename T>
    class XMLParser {
        XMLRoot *m_root;
        const T *m_start;
        const T *m_row;
        int      m_line;
        inline void parse_eq(const T *&doc);
        void parse_elem(XMLNode &parent, const T *&doc);
        void parse_char_data(const T *&doc);
        void parse_attribute(AttrList *attrs, const T *&doc);
        int parse_attr_val(const T *&doc, T *buf, int maxlen);
        void translate_charref(const T *&doc, T *&buf, int &maxlen);
        void parse_name(const T *&doc);
        void parse_decl(const T *&doc, XMLRoot *root);
        void parse_standalone(const T *&doc, XMLRoot *root);
        void parse_encoding(const T *&doc, XMLRoot *root);
        void parse_version(const T *&doc, XMLRoot *root);
        void parse_val(const T *&doc);
        inline void require_space(const T *&s, int eof_ok = 0, const char *errmsg = "Expected blank space");
        inline void consume_space(const T *&s, int eof_ok = 0);
        inline void consume_whitespace(const T *&s, int eof_ok = 0);
    public:
        XMLParser();
        XMLRoot *parse_doc(const T *doc);
    };

    /* standard public API */
    XMLRoot *fromstring(const char *s);
    XMLRoot *fromfn(const char *fn);
    void dump(XMLRoot *r);
    void dump(XMLNode *n);
}

#endif // !XML_PARSE_H
