#include "compiler.h"

void tora_compile(TNode *node, VM &vm) {
    switch (node->type) {
    case NODE_STRING: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_STRING;
        tmp->operand.str_value = strdup(node->str_value);
        vm.ops.push_back(tmp);
        break;
    }
    case NODE_INT: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_INT;
        tmp->operand.int_value = node->int_value;
        vm.ops.push_back(tmp);
        break;
    }
    case NODE_TRUE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_TRUE;
        vm.ops.push_back(tmp);
        break;
    }
    case NODE_FALSE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_FALSE;
        vm.ops.push_back(tmp);
        break;
    }
    case NODE_IDENTIFIER: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_IDENTIFIER;
        tmp->operand.str_value = strdup(node->str_value);
        vm.ops.push_back(tmp);
        break;
    }

    case NODE_FUNCALL: {
        tora_compile(node->binary.left, vm);
        tora_compile(node->binary.right, vm);

        OP * tmp = new OP;
        tmp->op_type = OP_FUNCALL;
        tmp->operand.int_value = 1; // the number of args
        vm.ops.push_back(tmp);
        break;
    }
#define C_OP_BINARY(type) \
    { \
        tora_compile(node->binary.left, vm); \
        tora_compile(node->binary.right, vm); \
        OP * tmp = new OP; \
        tmp->op_type = (type); \
        vm.ops.push_back(tmp); \
        break; \
    }

    case NODE_ADD: C_OP_BINARY(OP_ADD);
    case NODE_SUB: C_OP_BINARY(OP_SUB);
    case NODE_MUL: C_OP_BINARY(OP_MUL);
    case NODE_DIV: C_OP_BINARY(OP_DIV);
    case NODE_LT:  C_OP_BINARY(OP_LT);
    case NODE_GT:  C_OP_BINARY(OP_GT);
    case NODE_LE:  C_OP_BINARY(OP_LE);
    case NODE_GE:  C_OP_BINARY(OP_GE);
    case NODE_EQ:  C_OP_BINARY(OP_EQ);
    case NODE_ASSIGN:  C_OP_BINARY(OP_ASSIGN);
#undef C_OP_BINARY

    case NODE_STMTS: {
        tora_compile(node->binary.left, vm);
        tora_compile(node->binary.right, vm);
        break;
    }
    case NODE_IF: {
        /*
            run_jouken_code
            jump_if_false END_LABEL
            run_block_code
        END_LABEL:
            ...
        */
        tora_compile(node->binary.left, vm); // jouken

        OP * tmp = new OP;
        tmp->op_type = OP_JUMP_IF_FALSE;
        vm.ops.push_back(tmp);

        tora_compile(node->binary.right, vm); // block

        tmp->operand.int_value = vm.ops.size();

        break;
    }
    case NODE_NEWLINE:
        // nop
        break;
    case NODE_VARIABLE: {
        OP * tmp = new OP;
        tmp->op_type = OP_VARIABLE;
        tmp->operand.str_value = node->str_value;
        vm.ops.push_back(tmp);
        break;
    }

    default:
        printf("Unknown node: %d\n", node->type);
        abort();
        break;
    }
}
