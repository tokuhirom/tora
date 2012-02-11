#ifndef TORA_LEXER_H_
#define TORA_LEXER_H_

#include "node.h"

namespace tora {

struct ParserState {
    int errors;
    bool failure;
    SharedPtr<tora::Node> root_node;
    int lineno;
    ParserState() : errors(0), failure(false) { }
    ~ParserState() { }
};

};

#endif // TORA_LEXER_H_
