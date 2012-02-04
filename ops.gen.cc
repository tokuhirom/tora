#include "ops.gen.h"

const char*tora::opcode2name[] = {
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
    "OP_PUSH_VALUE",
    "OP_DUMP",
    "OP_JUMP_IF_FALSE",
    "OP_LT",
    "OP_GT",
    "OP_LE",
    "OP_GE",
    "OP_EQ",
    "OP_ASSIGN",
    "OP_JUMP",
    "OP_END",
    "OP_RETURN",
    "OP_ENTER",
    "OP_LEAVE",
    "OP_SETLOCAL",
    "OP_SETDYNAMIC",
    "OP_GETLOCAL",
    "OP_GETDYNAMIC",
    "OP_DEFINE_METHOD",
    "OP_MAKE_ARRAY",
    "OP_GET_ITEM",
    "OP_SET_ITEM",
    "OP_UNARY_NEGATIVE",
    "OP_METHOD_CALL",
};
