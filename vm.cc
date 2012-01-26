#include "vm.h"

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
            Value *v1 = stack.pop(); \
            Value *v2 = stack.pop(); \
            Value *v = new Value; \
            v->set_int(v2->value.int_value op v1->value.int_value); \
            stack.push(v); \
            v1->release(); \
            v2->release(); \
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
                Value *v = stack.pop();
                Value *s = v->to_s();
                printf("%s", s->value.str_value);
                s->release();
            } else if (strcmp(funname->value.str_value, "p") == 0) {
                Value *v = stack.pop();
                v->dump();
                v->release();
            } else if (strcmp(funname->value.str_value, "say") == 0) {
                Value *v = stack.pop();
                Value *s = v->to_s();
                printf("%s\n", s->value.str_value);
                s->release();
            } else if (strcmp(funname->value.str_value, "exit") == 0) {
                Value *v = stack.pop();
                assert(v->value_type == VALUE_TYPE_INT);
                Value *s = v->to_i();
                exit(s->value.int_value);
                s->release();
                v->release();
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
            Value * v = stack.pop();

            Value * b = v->to_b();
            if (!b->value.bool_value) {
                pc = op->operand.int_value-1;
            }
            b->release();
            v->release();
            break;
        }
        case OP_JUMP: {
            pc = op->operand.int_value-1;
            break;
        }
        case OP_EQ: {
            Value * v1 = stack.back();
            stack.pop_back();
            Value * v2 = stack.back();
            stack.pop_back();

            switch (v1->value_type) {
            case VALUE_TYPE_INT: {
                Value * i2 = v2->to_i();
                Value *result = new Value();
                result->set_bool( v1->value.int_value == i2->value.int_value );
                stack.push(result);
                break;
            }
            default:
                abort();
            }

            v1->release();
            v2->release();

            break;
        }
        case OP_GT: {
            Value * v1 = stack.back();
            stack.pop_back();
            Value * v2 = stack.back();
            stack.pop_back();

            switch (v1->value_type) {
            case VALUE_TYPE_INT: {
                Value * i2 = v2->to_i();
                Value *result = new Value();
                result->set_bool( v1->value.int_value > i2->value.int_value );
                stack.push(result);
                break;
            }
            default:
                abort();
            }

            v1->release();
            v2->release();

            break;
        }
        case OP_LT: {
            Value * v1 = stack.back();
            stack.pop_back();
            Value * v2 = stack.back();
            stack.pop_back();

            switch (v1->value_type) {
            case VALUE_TYPE_INT: {
                Value * i2 = v2->to_i();
                Value *result = new Value();
                result->set_bool( v1->value.int_value < i2->value.int_value );
                stack.push(result);
                break;
            }
            default:
                abort();
            }

            v1->release();
            v2->release();

            break;
        }
        case OP_GE: {
            Value * v1 = stack.back();
            stack.pop_back();
            Value * v2 = stack.back();
            stack.pop_back();

            switch (v1->value_type) {
            case VALUE_TYPE_INT: {
                Value * i2 = v2->to_i();
                Value *result = new Value();
                result->set_bool( v1->value.int_value >= i2->value.int_value );
                stack.push(result);
                break;
            }
            default:
                abort();
            }

            v1->release();
            v2->release();

            break;
        }
        case OP_LE: {
            Value * v1 = stack.back();
            stack.pop_back();
            Value * v2 = stack.back();
            stack.pop_back();

            switch (v1->value_type) {
            case VALUE_TYPE_INT: {
                Value * i2 = v2->to_i();
                Value *result = new Value();
                result->set_bool( v1->value.int_value <= i2->value.int_value );
                stack.push(result);
                break;
            }
            default:
                abort();
            }

            v1->release();
            v2->release();

            break;
        }
        case OP_ASSIGN: {
            Value * v1 = stack.back();
            stack.pop_back();
            Value * v2 = stack.back(); // variable
            stack.pop_back();

            v2->value_type  = v1->value_type;
            v2->value = v1->value;

            v1->release();

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
