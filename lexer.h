#ifndef TORA_LEXER_H_
#define TORA_LEXER_H_

#include "node.h"

namespace tora {

// TODO: return StrNode, IntNode, DoubleNode in lexer!
typedef union {
    class tora::Node* node;
} YYSTYPE;

struct ParserState {
    int errors;
    bool failure;
    tora::Node *root_node;
    int lineno;
    ParserState() : errors(0), failure(false) { }
    ~ParserState() { }
};

};

#endif // TORA_LEXER_H_
