#ifndef TORA_LEXER_H_
#define TORA_LEXER_H_

#include "node.h"

namespace tora {

typedef union {
    int int_value;
    double double_value;
    const char *str_value;
    class tora::Node* node;
    std::vector<tora::SharedPtr<tora::Node>> *node_list;
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
