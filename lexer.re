#ifndef TORA_SCANNER_H_
#define TORA_SCANNER_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include "node.h"
#include "nodes.gen.h"
#include "parser.h"
#include <iostream>
#include <fstream>

namespace tora {

class Scanner {
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
    void tora_add_string_literal(char c) {
        (*string_buffer) << c;
    }
    const char *tora_close_string_literal() {
        const char * ret = strdup(string_buffer->str().c_str());
        delete string_buffer;
        string_buffer = NULL;
        return ret;
    }
 
    Scanner( std::istream *ifs_, int init_size=1024 )
        : m_buffer(0)
        , m_cursor(0)
        , m_limit(0)
        , m_token(0)
        , m_marker(0)
        , m_buffer_size(init_size)
        , m_lineno(1)
        , divable(false)
        , string_buffer(NULL)
    {
        m_buffer = new char[m_buffer_size];
        m_cursor = m_limit = m_token = m_marker = m_buffer;
        ifs = ifs_;
    }
 
    ~Scanner() {
        delete ifs;
        delete [] m_buffer;
        if (string_buffer) { delete string_buffer; }
    }
 
    bool fill(int n) {
 
        // is eof?
        if (ifs->eof()) {
            if ((m_limit-m_cursor) <= 0) {
                return false;
            }
        }
 
        int restSize = m_limit-m_token;
        if (restSize+n >= m_buffer_size) {
            // extend buffer
            m_buffer_size *= 2;
            char* newBuffer = new char[m_buffer_size];
            for (int i=0; i<restSize; ++i) { // memcpy
                *(newBuffer+i) = *(m_token+i);
            }
            m_cursor = newBuffer + (m_cursor-m_token);
            m_token = newBuffer;
            m_limit = newBuffer + restSize;
 
            delete [] m_buffer;
            m_buffer = newBuffer;
        } else {
            // move remained data to head.
            for (int i=0; i<restSize; ++i) { //memmove( m_buffer, m_token, (restSize)*sizeof(char) );
                *(m_buffer+i) = *(m_token+i);
            }
            m_cursor = m_buffer + (m_cursor-m_token);
            m_token = m_buffer;
            m_limit = m_buffer+restSize;
        }
 
        // fill to buffer
        int read_size = m_buffer_size - restSize;
        ifs->read( m_limit, read_size );
        m_limit += ifs->gcount();
 
        return true;
    }
 
 
    std::string text() {
        return std::string( m_token, m_token+length() );
    }
    int length() {
        return (m_cursor-m_token);
    }
    int lineno() {
        return m_lineno;
    }
 
    int scan(YYSTYPE *yylval) {
std:
        m_token = m_cursor;
 
    /*!re2c
        re2c:define:YYCTYPE = "char";
        re2c:define:YYCURSOR = m_cursor;
        re2c:define:YYMARKER = m_marker;
        re2c:define:YYLIMIT = m_limit;
        re2c:define:YYFILL:naked = 1;
        re2c:define:YYFILL@len = #;
        re2c:define:YYFILL = "if (!fill(#)) { return 0; }";
        re2c:yyfill:enable = 1;
        re2c:indent:top = 2;
        re2c:indent:string="    ";

        ANY_CHARACTER          = [^];
        VARNAME                = [$][A-Za-z_][A-Za-z0-9_]*;
        IDENTIFIER             = [A-Za-z_][A-Za-z0-9_]*;
        DOUBLE                 = [1-9][0-9]*[.][0-9]+;

    */

/*!re2c
    "+" { return ADD; }
    "-" { return SUB; }
    "*" { return MUL; }
    "/" {
        if (divable) {
            return DIV;
        } else {
            tora_open_string_literal();
            goto regexp_literal;
        }
    }
    "//" {
        goto cc_comment;
    }
    "\n" {
        increment_line_number();
        goto std;
    }
    ";" { return SEMICOLON; }
    "sub" {return FUNCSUB; }
    "for" { return FOR; }
    "," { return COMMA; }
    ".." { return DOTDOT; }
    DOUBLE {
        double tmp = 0;
        char *head;
        for (head=m_token; head<m_cursor; head++) {
            if (*head == '.') {
                head++;
                break;
            } else {
                tmp = tmp*10 + (*head - '0');
            }
        }
        for (double n = 0.1; head<m_cursor; head++, n*=0.1) {
            tmp = tmp + (*head-'0')*n;
        }
        yylval->node = new DoubleNode(NODE_DOUBLE, tmp);
        divable = true;
        return DOUBLE_LITERAL;
    }
    "." { return DOT; }
    "0" {
        yylval->node = new IntNode(NODE_INT, 0);
        divable = true;
        return INT_LITERAL;
    }
    [1-9][0-9]* {
        int tmp = 0;
        for (char *head=m_token; head<m_cursor; head++) {
            tmp = tmp*10 + (*head - '0');
        }
        yylval->node = new IntNode(NODE_INT, tmp);
        divable = true;
        return INT_LITERAL;
    }
    "if" { return IF; }
    "<" { return GT; }
    ">" { return LT; }
    "<=" { return GE; }
    ">=" { return LE; }
    "==" { return EQ; }
    "++" { return PLUSPLUS; }
    "/=" { return DIV_ASSIGN; }
    "(" { 
        divable = false;
        return L_PAREN;
    }
    ")" {
        divable = true;
        return R_PAREN;
    }
    "{" { return L_BRACE; }
    "}" { return R_BRACE; }
    "[" { return L_BRACKET; }
    "]" { divable = true; return R_BRACKET; }
    "=" { return ASSIGN; }
    "my" { return MY; }
    "true" { return TRUE; }
    "false" { return FALSE; }
    "while" { return WHILE; }
    "return" { return RETURN; }
    "else" { return ELSE; }
    [ \t\r] {
        // skip white space
        goto std;
    }
    "\n__END__\n" {
        increment_line_number();
        increment_line_number();
        goto end;
    }
    "\n" {
        increment_line_number();
        goto std;
    }
    IDENTIFIER {
        std::string token(m_token, m_cursor-m_token);
        yylval->node = new StrNode(NODE_IDENTIFIER, token);
        divable = true;
        return IDENTIFIER;
    }
    VARNAME {
        std::string token(m_token, m_cursor-m_token);
        yylval->node = new StrNode(NODE_GETVARIABLE, token);
        divable = true;
        return VARIABLE;
    }
    "\"" {
        tora_open_string_literal();
        goto string_literal;
    }
    "/*" {
        goto c_comment;
    }
    "#" {
        goto perl_comment;
    }
    "\000" {
        goto std;
    }
    ANY_CHARACTER {
        printf("unexpected character: '%c(%d)'\n", *m_token, *m_token);
        goto std;
    }

    */

c_comment:
/*!re2c
    "\n" { increment_line_number(); }
    "*/" {
        goto std;
    }
    ANY_CHARACTER { goto c_comment; }
    */

perl_comment:
cc_comment:
/*!re2c
    "\n" { 
        increment_line_number();
        goto std;
    }
    ANY_CHARACTER { goto cc_comment; }
    */

end:
/*!re2c
    ANY_CHARACTER { goto end; }
    */

string_literal:
/*!re2c
    "\"" {
        yylval->node = new StrNode(NODE_STRING, tora_close_string_literal());
        return STRING_LITERAL;
    }
    "\n" {
        tora_add_string_literal('\n');
        increment_line_number();
        goto string_literal;
    }
    "\\\"" {
        tora_add_string_literal('"');
        goto string_literal;
    }
    "\\t" {
        tora_add_string_literal('\t');
        goto string_literal;
    }
    "\\\\" {
        tora_add_string_literal('\\');
        goto string_literal;
    }
    ANY_CHARACTER {
        tora_add_string_literal(*(m_cursor-1));
        goto string_literal;
    }

    */

regexp_literal:
/*!re2c
    "/" {
        // # <REGEXP>"\\/" tora_add_string_literal('/');
        /* TODO: options like /xsmi */
        yylval->node = new RegexpNode(NODE_REGEXP, tora_close_string_literal());
        return REGEXP_LITERAL;
    }
    ANY_CHARACTER {
        tora_add_string_literal(*(m_cursor-1));
        goto regexp_literal;
    }
*/

    abort();
}

};

};

#endif
