#ifndef NODES_GEN_H_
#define NODES_GEN_H_

namespace tora {

typedef enum {
    NODE_INT,
    NODE_DOUBLE,
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
    NODE_ASSIGN,
    NODE_WHILE,
    NODE_ROOT,
    NODE_VOID,
    NODE_FUNCDEF,
    NODE_RETURN,
    NODE_BLOCK,
    NODE_MY,
    NODE_MAKE_ARRAY,
    NODE_GET_ITEM,
    NODE_FOR,
    NODE_UNARY_NEGATIVE,
    NODE_METHOD_CALL,
} node_type_t;

extern const char*node_type2name[];

};

#endif // NODES_GEN_H_