#include "vm.h"
#include "value.h"

// run program
void VM::execute() {
    while (pc < ops.size()) {
        // printf("[DEBUG] OP: %s\n", opcode2name[ops[pc]->op_type]);

        OP *op = ops[pc];
        switch (op->op_type) {
        case OP_PUSH_TRUE: {
            Value *v = new Value;
            v->set_bool(true);
            stack.push(v);
            break;
        }
        case OP_PUSH_FALSE: {
            Value *v = new Value;
            v->set_bool(false);
            stack.push(v);
            break;
        }
        case OP_PUSH_INT: {
            Value *v = new Value;
            v->set_int(op->operand.int_value);
            stack.push(v);
            break;
        }
        case OP_PUSH_STRING: {
            Value *v = new Value;
            v->set_str(op->operand.str_value);
            stack.push(v);
            break;
        }
#define BINOP(optype, op) \
        case optype: { \
            ValuePtr v1(stack.pop()); \
            ValuePtr v2(stack.pop()); \
            Value *v = new Value; \
            v->set_int(v2->value.int_value op v1->value.int_value); \
            stack.push(v); \
            break; \
        }
        BINOP(OP_ADD, +);
        BINOP(OP_SUB, -);
        BINOP(OP_DIV, /);
        BINOP(OP_MUL, *);
#undef BINOP
        case OP_FUNCALL: {
            assert(stack.size() >= op->operand.int_value + 1);
            Value *funname = stack.at(stack.size()-op->operand.int_value-1);
            assert(funname->value_type == VALUE_TYPE_STR);
            if (strcmp(funname->value.str_value, "print") == 0) {
                ValuePtr v(stack.pop());
                ValuePtr s(v->to_s());
                printf("%s", s->value.str_value);
            } else if (strcmp(funname->value.str_value, "p") == 0) {
                ValuePtr v(stack.pop());
                v->dump();
            } else if (strcmp(funname->value.str_value, "say") == 0) {
                ValuePtr v(stack.pop());
                Value *s = v->to_s();
                printf("%s\n", s->value.str_value);
            } else if (strcmp(funname->value.str_value, "exit") == 0) {
                ValuePtr v(stack.pop());
                assert(v->value_type == VALUE_TYPE_INT);
                ValuePtr s(v->to_i());
                exit(s->value.int_value);
            } else {
                fprintf(stderr, "Unknown function: %s\n", funname->value.str_value);
            }
            break;
        }
        case OP_PUSH_IDENTIFIER: {
            // operand = the number of args
            Value *v = new Value;
            v->value.str_value = op->operand.str_value;
            v->value_type = VALUE_TYPE_STR;
            stack.push(v);
            break;
        }
        case OP_DUMP: {
            this->dump_stack();
            break;
        }
        case OP_JUMP_IF_FALSE: {
            ValuePtr v(stack.pop());

            ValuePtr b(v->to_b());
            if (!b->value.bool_value) {
                pc = op->operand.int_value-1;
            }
            break;
        }
        case OP_JUMP: {
            pc = op->operand.int_value-1;
            break;
        }
#define CMPOP(type, op) \
        case type: { \
            ValuePtr v1(stack.pop()); \
            ValuePtr v2(stack.pop()); \
            \
            switch (v1->value_type) { \
            case VALUE_TYPE_INT: { \
                Value * i2 = v2->to_i(); \
                Value *result = new Value(); \
                result->set_bool( v1->value.int_value op i2->value.int_value ); \
                stack.push(result); \
                break; \
            } \
            default: \
                abort(); \
            } \
            v1->release(); \
            v2->release(); \
            break; \
        }
        CMPOP(OP_EQ, ==)
        CMPOP(OP_GT, >)
        CMPOP(OP_LT, <)
        CMPOP(OP_GE, >=)
        CMPOP(OP_LE, <=)
        case OP_ASSIGN: {
            ValuePtr v1(stack.pop());
            Value * v2 = stack.pop(); // variable

            v2->value_type  = v1->value_type;
            v2->value = v1->value;

            stack.push(v2);

            break;
        }
        case OP_VARIABLE: {
            Value *v = global_vars[std::string(op->operand.str_value)];
            if (!v) {
                v = new Value();
                global_vars[std::string(op->operand.str_value)] = v;
            }
            v->retain();
            stack.push(v);
            break;
        }
        default: {
            fprintf(stderr, "[BUG] OOPS. unknown op code: %d(%s)\n", op->op_type, opcode2name[op->op_type]);
            abort();
            break;
        }
        }
        // dump_stack();
        pc++;
    }
}
