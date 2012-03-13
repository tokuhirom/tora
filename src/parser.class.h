#ifndef TORA_PARSER_CLASS_H_
#define TORA_PARSER_CLASS_H_

#include "node.h"
#include "lexer.h"
#include "stdlib.h"

namespace tora {

class Parser {
    void *yyp;
    static void *ParseAlloc(void *(*mallocProc)(size_t));
    static void ParseFree(
        void *p,                    /* The parser to be deleted */
        void (*freeProc)(void*)     /* Function used to reclaim memory */
    );
    static void Parse(
        void *yyp,                   /* The parser */
        int yymajor,                 /* The major token code number */
        Node* yyminor       /* The value for the token */
        , ParserState *state
    );
    ParserState state_;
public:
    Parser(const std::string & filename) :state_(filename) {
        yyp = ParseAlloc(malloc);
    }
    ~Parser() {
        ParseFree(yyp, free);
    }
    static void ParseTrace(FILE *TraceFILE, char *zTracePrompt);
    void parse(int yymajor, Node *yyminor) {
        Parser::Parse(yyp, yymajor, yyminor, &state_);
    }
    bool is_failure() {
        return state_.failure || state_.errors > 0;
    }
    void set_lineno(int n) {
        state_.lineno = n;
    }
    SharedPtr<Node> root_node() {
        return this->state_.root_node;
    }
};

};

#endif // TORA_PARSER_CLASS_H_

