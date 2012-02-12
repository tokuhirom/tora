#ifndef TORA_PARSER_CLASS_H_
#define TORA_PARSER_CLASS_H_

#include "node.h"
#include "lexer.h"

namespace tora {

class Parser {
public:
    static void ParseTrace(FILE *TraceFILE, char *zTracePrompt);

    static void Parse(
        void *yyp,                   /* The parser */
        int yymajor,                 /* The major token code number */
        Node* yyminor       /* The value for the token */
        , ParserState *state
    );

    static void ParseFree(
        void *p,                    /* The parser to be deleted */
        void (*freeProc)(void*)     /* Function used to reclaim memory */
    );

    static void *ParseAlloc(void *(*mallocProc)(size_t));
};

};

#endif // TORA_PARSER_CLASS_H_

