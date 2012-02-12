#include "disasm.h"

void tora::Disasm::disasm_op(OP* op) {
    printf("OP: %s", opcode2name[op->op_type]);
    switch (op->op_type) {
    case OP_SETLOCAL: {
        printf(" %d", op->operand.int_value);
        break;
    }
    case OP_GETLOCAL: {
        printf(" %d", op->operand.int_value);
        break;
    }
    case OP_GETDYNAMIC: {
        int level = (op->operand.int_value >> 16) & 0x0000FFFF;
        int no    = op->operand.int_value & 0x0000ffff;
        printf(" level: %d, no: %d", level, no);
        break;
    }
    case OP_PUSH_INT: {
        printf(" %d", op->operand.int_value);
        break;
    }
    default:
        break;
    }
    printf("\n");
}
