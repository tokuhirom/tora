/* vim: set filetype=cpp: */
#include "lexer.h"
#include "nodes.gen.h"
#include "parser.h"

using namespace tora;

bool Scanner::fill(int n) {

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
 
int Scanner::scan(Node **yylval) {
std:
    m_token = m_cursor;

    if (next_token) {
        int n = next_token;
        next_token = 0;
        return n;
    }

    char string_close_char = '\0';

    if (qw_mode) {
        goto qw_literal;
    }


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
        PACKAGE_VARNAME        = [$] ( IDENTIFIER "::" )* IDENTIFIER;
        DOUBLE                 = ([1-9][0-9]*[.][0-9]+) | ([0][.][0-9]+);
        LF                     = "\\n";
        HEREDOC_MARKER         = [A-Za-z_][A-Za-z0-9_]*;
        HEREDOC_START          = "<<" HEREDOC_MARKER;
        CLASS_NAME             = IDENTIFIER ( "::" IDENTIFIER )*;
    */

/*!re2c
    "qq{" {
        tora_open_string_literal();
        string_close_char = '}';
        goto single_string_literal;
    }
    "qq[" {
        tora_open_string_literal();
        string_close_char = ']';
        goto single_string_literal;
    }
    "qq(" {
        tora_open_string_literal();
        string_close_char = ')';
        goto single_string_literal;
    }
    "qq!" {
        tora_open_string_literal();
        string_close_char = '!';
        goto single_string_literal;
    }
    "q{" {
        tora_open_string_literal();
        string_close_char = '}';
        goto single_string_literal;
    }
    "q(" {
        tora_open_string_literal();
        string_close_char = ')';
        goto single_string_literal;
    }
    "q[" {
        tora_open_string_literal();
        string_close_char = ']';
        goto single_string_literal;
    }
    "q!" {
        tora_open_string_literal();
        string_close_char = '!';
        goto single_string_literal;
    }
    "self" {
        return SELF;
    }
    "${" {
        return DEREF;
    }
    "qw{" {
        qw_mode = '{';
        return QW_START;
    }
    "qw!" {
        qw_mode = '!';
        return QW_START;
    }
    "qw(" {
        qw_mode = '(';
        return QW_START;
    }
    "qw[" {
        qw_mode = '[';
        return QW_START;
    }
    "+=" { return ADD_ASSIGN; }
    "-=" { return SUB_ASSIGN; }
    "*=" { return MUL_ASSIGN; }
    "/=" { return DIV_ASSIGN; }
    "&=" { return AND_ASSIGN; }
    "|=" { return OR_ASSIGN; }
    "^=" { return XOR_ASSIGN; }
    "%=" { return MOD_ASSIGN; }
    "+" { return ADD; }
    "!=" { return NE; }
    "-" { return SUB; }
    "**" { return POW; }
    "*" { return MUL; }
    "%" { return MOD; }
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
        if (heredoc_queue.size() > 0) {
            tora_open_string_literal();
            ++m_token;
            goto heredoc_literal_start;
        } else {
            goto std;
        }
    }
    "?" { return QUESTION; }
    ":" { return COLON; }
    ";" { return SEMICOLON; }
    "sub" {return FUNCSUB; }
    "try" {return TRY; }
    "die" {return DIE; }
    "for" { return FOR; }
    "next" { return NEXT; }
    "last" { return LAST; }
    "redo" { return REDO; }
    "," { return COMMA; }
    "=>" { return FAT_COMMA; }
    ".." { return DOTDOT; }
    "..." { return DOTDOTDOT; }
    HEREDOC_START {
        HereDoc * here = new HereDoc();
        *yylval = here->val = new StrNode(NODE_STRING, "");
        here->marker = this->text().substr(2, this->text().length()-2);
        heredoc_queue.push(here);
        return HEREDOC_START;
    }
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
        *yylval = new DoubleNode(NODE_DOUBLE, tmp);
        divable = true;
        return DOUBLE_LITERAL;
    }
    "." { return DOT; }
    "0" {
        *yylval = new IntNode(NODE_INT, 0);
        divable = true;
        return INT_LITERAL;
    }
    [1-9][0-9]* {
        int tmp = 0;
        for (char *head=m_token; head<m_cursor; head++) {
            tmp = tmp*10 + (*head - '0');
        }
        *yylval = new IntNode(NODE_INT, tmp);
        divable = true;
        return INT_LITERAL;
    }
    "->" { return LAMBDA; }
    "if" { return IF; }
    "unless" { return UNLESS; }
    "undef" { return UNDEF; }
    "in" { return IN; }
    "class" { return CLASS; }
    "<" { return GT; }
    ">" { return LT; }
    "<=" { return GE; }
    ">=" { return LE; }
    "==" { return EQ; }
    "++" { return PLUSPLUS; }
    "--" { return MINUSMINUS; }
    "-f" { *yylval = new IntNode(NODE_INT, 'f'); return FILE_TEST; }
    "-d" { *yylval = new IntNode(NODE_INT, 'd'); return FILE_TEST; }
    "-e" { *yylval = new IntNode(NODE_INT, 'e'); return FILE_TEST; }
    "-s" { *yylval = new IntNode(NODE_INT, 's'); return FILE_TEST; }
    "-x" { *yylval = new IntNode(NODE_INT, 'x'); return FILE_TEST; }
    "!" { return NOT; }
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
    "||" { return OROR; }
    "&&" { return ANDAND; }
    "|" { return BITOR; }
    "&" { return BITAND; }
    "^" { return BITXOR; }
    "<<" { return BITLSHIFT; }
    ">>" { return BITRSHIFT; }
    "=" { return ASSIGN; }
    "my" { return MY; }
    "local" { return LOCAL; }
    "true" { return TRUE; }
    "false" { return FALSE; }
    "while" { return WHILE; }
    "return" { return RETURN; }
    "else" { return ELSE; }
    "elsif" { return ELSIF; }
    "use" { return USE; }
    [ \t\r] {
        // skip white space
        goto std;
    }
    "__PACKAGE__" {
        return PACKAGE_LITERAL;
    }
    "__LINE__" {
        *yylval = new IntNode(NODE_INT, this->lineno());
        divable = true;
        return INT_LITERAL;
    }
    "__FILE__" {
        *yylval = new StrNode(NODE_STRING, this->filename_);
        return STRING_LITERAL;
    }
    "\n__END__\n" {
        increment_line_number();
        increment_line_number();
        goto end;
    }
    IDENTIFIER {
        std::string token(m_token, m_cursor-m_token);
        *yylval = new StrNode(NODE_IDENTIFIER, token);
        divable = true;
        return IDENTIFIER;
    }
    CLASS_NAME {
        std::string token(m_token, m_cursor-m_token);
        *yylval = new StrNode(NODE_IDENTIFIER, token);
        divable = true;
        return IDENTIFIER;
    }
    PACKAGE_VARNAME {
        std::string token(m_token, m_cursor-m_token);
        *yylval = new StrNode(NODE_GET_PACKAGE_VARIABLE, token);
        divable = true;
        return VARIABLE;
    }
    VARNAME {
        std::string token(m_token, m_cursor-m_token);
        *yylval = new StrNode(NODE_GETVARIABLE, token);
        divable = true;
        return VARIABLE;
    }
    "\"" {
        tora_open_string_literal();
        string_close_char = '"';
        goto string_literal;
    }
    "'" {
        tora_open_string_literal();
        string_close_char = '\'';
        goto single_string_literal;
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

single_string_literal:
/*!re2c
    [')!}\]] {
        if (string_close_char == *(m_cursor-1)) {
            *yylval = new StrNode(NODE_STRING, string_buffer->str());
            delete string_buffer; string_buffer = NULL;
            return STRING_LITERAL;
        } else {
            tora_add_string_literal(*(m_cursor-1));
            goto single_string_literal;
        }
    }
    "\\\\" {
        tora_add_string_literal('\\');
        goto single_string_literal;
    }
    "\\'" {
        tora_add_string_literal('\'');
        goto single_string_literal;
    }
    ANY_CHARACTER {
        tora_add_string_literal(*(m_cursor-1));
        goto single_string_literal;
    }
*/

string_literal:
/*!re2c
    [")!}\]] {
        if (string_close_char == *(m_cursor-1)) {
            *yylval = new StrNode(NODE_STRING, string_buffer->str());
            delete string_buffer; string_buffer = NULL;
            return STRING_LITERAL;
        } else {
            tora_add_string_literal(*(m_cursor-1));
            goto string_literal;
        }
    }
    "\\r" {
        tora_add_string_literal('\r');
        goto string_literal;
    }
    LF {
        tora_add_string_literal('\n');
        goto string_literal;
    }
    "\\\"" {
        tora_add_string_literal('"');
        goto string_literal;
    }
    "\\0" {
        tora_add_string_literal('\0');
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
        // TODO: options like /xsmi
        *yylval = new RegexpNode(NODE_REGEXP, string_buffer->str());
        delete string_buffer; string_buffer = NULL;
        return REGEXP_LITERAL;
    }
    ANY_CHARACTER {
        tora_add_string_literal(*(m_cursor-1));
        goto regexp_literal;
    }
*/

qw_literal:
/*!re2c
    [)!}\]] {
        char open_char;
        switch (*(m_cursor-1)) {
        case ')': open_char='('; break;
        case '!': open_char='!'; break;
        case '}': open_char='{'; break;
        case ']': open_char='['; break;
        default:  abort();
        }
        if (open_char == qw_mode) {
            qw_mode = '\0';

            if (string_buffer) {
                this->next_token = QW_END;

                *yylval = new StrNode(NODE_STRING, string_buffer->str());
                delete string_buffer; string_buffer = NULL;
                return QW_WORD;
            } else {
                return QW_END;
            }
        } else {
            if (!string_buffer) {
                tora_open_string_literal();
            }
            tora_add_string_literal(*(m_cursor-1));
            goto regexp_literal;
        }
    }
    [ \t\f\b\n] {
        if (string_buffer) {
            *yylval = new StrNode(NODE_STRING, string_buffer->str());
            delete string_buffer; string_buffer = NULL;
            return QW_WORD;
        }
        goto qw_literal; // skip
    }
    ANY_CHARACTER {
        if (!string_buffer) {
            tora_open_string_literal();
        }
        tora_add_string_literal(*(m_cursor-1));
        goto qw_literal;
    }
*/

heredoc_literal_start:
    char * cc_marker = m_cursor;
/*!re2c
    [A-Z0-9a-z_]+ "\n" {
        std::string &marker = heredoc_queue.front()->marker;
        if (this->text().length() >= marker.length()+1) {
            std::string target_marker = this->text().substr(this->text().length()-marker.length()-1, marker.length());
            if (target_marker == marker) {
                HereDoc *here = heredoc_queue.front();
                heredoc_queue.pop();
                here->val->str_value = string_buffer->str();
                delete string_buffer; string_buffer = NULL;
                delete here;
                if (heredoc_queue.size() > 0) {
                    tora_open_string_literal();
                    goto heredoc_literal_start;
                } else {
                    goto std;
                }
            } else {
                // printf("TARGET: '%s' FOO : '%s'\n", target_marker.c_str(), marker.c_str());
                goto FAIL_TARGET;
            }
        } else {
    FAIL_TARGET:
            std::string token(cc_marker, m_cursor-cc_marker);
            tora_add_string_literal(token);
            goto heredoc_literal_start;
        }
    }
    "\n" {
        increment_line_number();
        tora_add_string_literal(*(m_cursor-1));
        goto heredoc_literal_start;
    }
    ANY_CHARACTER {
        tora_add_string_literal(*(m_cursor-1));
        goto heredoc_literal_inner;
    }
*/

heredoc_literal_inner:
/*!re2c
    "\n" {
        increment_line_number();
        tora_add_string_literal(*(m_cursor-1));
        goto heredoc_literal_start;
    }
    ANY_CHARACTER {
        tora_add_string_literal(*(m_cursor-1));
        goto heredoc_literal_inner;
    }
*/

    abort();
}

