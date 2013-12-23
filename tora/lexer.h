#ifndef TORA_LEXER_H_
#define TORA_LEXER_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <queue>

#include "node.h"
#include "prim.h"

namespace tora {

struct ParserState {
    int errors;
    bool failure;
    SharedPtr<tora::Node> root_node;
    int lineno;
    std::string filename;

    ParserState(const std::string &filename_) : errors(0), failure(false), filename(filename_) { }
    ~ParserState() { }
};

class Scanner {
    PRIM_DECL()
private:
    // iostream sucks. very slow.
    std::istream *ifs;
 
    // buffer memory
    char* m_buffer;
    // current position
    char* m_cursor;
    char* m_limit;
    char* m_token;
    char* m_marker;
    int m_buffer_size;
    int m_lineno;
    bool divable;
    std::stringstream *string_buffer;
    unsigned char qw_mode;

    struct HereDoc {
        StrNode *val;
        std::string marker;
    };
    std::queue<HereDoc*> heredoc_queue;
    struct NodeQueueStuff {
        Node *lval;
        int type;
    };
    // queue to store the parsed stuffs.
    std::queue<NodeQueueStuff> node_queue;
    std::string filename_;
 
public:

    void increment_line_number() {
        m_lineno++;
    }


    /**
    * string buffer
    */
    void tora_open_string_literal() {
        if (string_buffer) {
            delete string_buffer;
            string_buffer = NULL;
        }
        string_buffer = new std::stringstream();
    }
    void tora_add_string_literal(const std::string &str) {
        assert(string_buffer);
        (*string_buffer) << str;
    }
    void tora_add_string_literal(char c) {
        assert(string_buffer);
        (*string_buffer) << c;
    }
    void tora_close_string_literal() {
        delete string_buffer;
        string_buffer = NULL;
    }
 
    Scanner( std::istream *ifs_, const std::string & filename, int init_size=1024 )
        : refcnt(0)
        , m_buffer(0)
        , m_cursor(0)
        , m_limit(0)
        , m_token(0)
        , m_marker(0)
        , m_buffer_size(init_size)
        , m_lineno(1)
        , divable(false)
        , string_buffer(NULL)
        , qw_mode('\0')
        , filename_(filename)
    {
        m_buffer = new char[m_buffer_size];
        assert(m_buffer);
        m_cursor = m_limit = m_token = m_marker = m_buffer;
        ifs = ifs_;
    }
 
    ~Scanner() {
        delete [] m_buffer;
        if (string_buffer) { delete string_buffer; }
    }
 
    bool fill(int n);
 
    std::string text() {
        return std::string( m_token, m_token+length() );
    }
    int length() {
        return (m_cursor-m_token);
    }
    int lineno() {
        return m_lineno;
    }

    bool in_heredoc() {
        return heredoc_queue.size() > 0;
    }
    int scan(Node **yylval);
};

};

#endif // TORA_LEXER_H_
