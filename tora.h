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
    NODE_VARIABLE,
    NODE_ASSIGN,
    NODE_WHILE,
} NODE_TYPE;

typedef struct {
    int op_type;
    union {
        int  int_value;
        bool bool_value;
        const char *str_value;
    } operand;
} OP;

#endif // TORA_H_

