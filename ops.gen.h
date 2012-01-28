#ifndef OPS_GEN_H_
#define OPS_GEN_H_

typedef enum {
    OP_NOP,
    OP_ADD,
    OP_SUB,
    OP_DIV,
    OP_MUL,
    OP_PRINT,
    OP_PUSH_INT,
    OP_PUSH_TRUE,
    OP_PUSH_FALSE,
    OP_PUSH_IDENTIFIER,
    OP_FUNCALL,
    OP_PUSH_STRING,
    OP_DUMP,
    OP_JUMP_IF_FALSE,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_ASSIGN,
    OP_VARIABLE,
    OP_JUMP,
    OP_END,
    OP_RETURN,
    OP_ENTER,
    OP_LEAVE,
} OP_TYPE;

static const char*opcode2name[] = {
    "OP_NOP",
    "OP_ADD",
    "OP_SUB",
    "OP_DIV",
    "OP_MUL",
    "OP_PRINT",
    "OP_PUSH_INT",
    "OP_PUSH_TRUE",
    "OP_PUSH_FALSE",
    "OP_PUSH_IDENTIFIER",
    "OP_FUNCALL",
    "OP_PUSH_STRING",
    "OP_DUMP",
    "OP_JUMP_IF_FALSE",
    "OP_LT",
    "OP_GT",
    "OP_LE",
    "OP_GE",
    "OP_EQ",
    "OP_ASSIGN",
    "OP_VARIABLE",
    "OP_JUMP",
    "OP_END",
    "OP_RETURN",
    "OP_ENTER",
    "OP_LEAVE",
};

#endif // OPS_GEN_H_
