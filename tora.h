#ifndef TORA_H_
#define TORA_H_
#define TORA_VERSION_STR "0.0.1"

#include <vector>
#include "value.h"

typedef struct TNode {
    int type;
    union {
        struct TNode *node;
        int int_value;
        const char*str_value;
        struct {
            struct TNode *left;
            struct TNode *right;
        } binary;
        struct {
            struct TNode *name;
            std::vector<struct TNode*> *args;
        } funcall;
        struct {
            struct TNode *name;
            std::vector<struct TNode*> *params;
            struct TNode *block;
        } funcdef;
        struct {
            struct TNode *cond;
            struct TNode *if_body;
            struct TNode *else_body;
        } if_stmt;
        struct {
            struct TNode *lvalue;
            struct TNode *rvalue;
        } set_value;
    };
} NODE;

typedef enum {
    NODE_INT = 0,
    NODE_TRUE,
    NODE_FALSE,
    NODE_IDENTIFIER,
    NODE_FUNCALL,
    NODE_STRING,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_STMTS,
    NODE_NEWLINE,
    NODE_IF,
    NODE_LT,
    NODE_GT,
    NODE_LE,
    NODE_GE,
    NODE_EQ,
    NODE_GETVARIABLE,
    NODE_SETVARIABLE,
    NODE_SETVARIABLE_MY,
    NODE_ASSIGN,
    NODE_WHILE,
    NODE_ROOT,
    NODE_VOID,
    NODE_FUNCDEF,
    NODE_RETURN,
    NODE_BLOCK,
    NODE_MY,
} NODE_TYPE;

#include "op.h"

#endif // TORA_H_

