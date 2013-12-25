#include "disasm.h"
#include "shared_ptr.h"
#include "op.h"
#include "value.h"

using namespace tora;

void Disasm::disasm_op(OP* op) {
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

void Disasm::disasm(const SharedPtr<OPArray>& ops) {
    printf("-- OP DUMP    --\n");
    for (size_t i=0; i<ops->size(); i++) {
        const OP* op = ops->at(i);
        printf("[%03zu] %s", i, opcode2name[op->op_type]);
        switch (op->op_type) {
        case OP_GETDYNAMIC: {
            int level = (op->operand.int_value >> 16) & 0x0000FFFF;
            int no    = op->operand.int_value & 0x0000ffff;
            printf(" level: %d, no: %d", level, no);
            break;
        }
        case OP_JUMP: {
            printf("\t%d", op->operand.int_value);
            break;
        }
        case OP_LAST: {
            printf("\t%d", op->operand.int_value);
            break;
        }
        case OP_SETDYNAMIC: {
            int level = (op->operand.int_value >> 16) & 0x0000FFFF;
            int no    = op->operand.int_value & 0x0000ffff;
            printf(" level: %d, no: %d", level, no);
            break;
        }
        case OP_PUSH_INT:
            printf("\t%d", op->operand.int_value);
            break;
        case OP_PUSH_STRING:
            printf("\t'%s'", get_str_value(static_cast<const ValueOP*>(op)->value)->c_str());
            break;
        default:
            // nop.
            break;
        }
        printf("\n");
    }
    printf("----------------\n");
}

