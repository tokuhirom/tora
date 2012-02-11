#ifndef TORA_LEXER_H_
#define TORA_LEXER_H_

#include "node.h"

typedef union {
    int int_value;
    double double_value;
    const char *str_value;
    class tora::Node* node;
    std::vector<tora::SharedPtr<tora::Node>> *node_list;
} YYSTYPE;

#endif // TORA_LEXER_H_
