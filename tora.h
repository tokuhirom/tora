#ifndef TORA_H_
#define TORA_H_
#define TORA_VERSION_STR "0.0.1"

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

typedef enum {
    VALUE_TYPE_NIL = 0,
    VALUE_TYPE_INT = 1,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_STR,
} value_type_t;

#endif // TORA_H_

