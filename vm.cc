#include "vm.h"
#include "value.h"

using namespace tora;

// run program
void VM::execute() {
    // TODO: move to vm.h
    this->lexical_vars_stack = new std::vector<LexicalVarsFrame *>();
    this->lexical_vars_stack->push_back(new LexicalVarsFrame());
    this->function_frames = new std::vector<FunctionFrame*>();

    for (;;) {
        // printf("[DEBUG] OP: %s\n", opcode2name[ops->at(pc)->op_type]);

        OP *op = ops->at(pc);
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
        case OP_PUSH_VALUE: {
            Value *v = op->operand.value;
            stack.push(v);
            break;
        }
        case OP_DEFINE_METHOD: {
            ValuePtr code(stack.pop()); // code object
            assert(code->value_type == VALUE_TYPE_CODE);
            const char *funcname = op->operand.str_value;
            this->add_function(funcname, &(*code));
            break;
        }
#define BINOP(optype, op) \
        case optype: { \
            ValuePtr v1(stack.pop()); \
            ValuePtr v2(stack.pop()); \
            if (!v1->is_numeric()) {  \
                ValuePtr s(v1->to_s()); \
                fprintf(stderr, "'%s' is not numeric.\n", s->value.str_value); \
                exit(1); /* TODO: die */ \
            } \
            Value *v = new Value; \
            v->set_int(v2->value.int_value op v1->value.int_value); \
            stack.push(v); \
            break; \
        }
        BINOP(OP_SUB, -);
        BINOP(OP_DIV, /);
        BINOP(OP_MUL, *);
#undef BINOP
        case OP_ADD: {
            ValuePtr v1(stack.pop());
            ValuePtr v2(stack.pop());
            if (v1->is_numeric()) {
                Value *v = new Value();
                ValuePtr i(v2->to_i());
                v->set_int(v2->value.int_value + v1->value.int_value);
                stack.push(v);
            } else if (v1->value_type == VALUE_TYPE_STR) {
                // TODO: support null terminated string
                Value *v = new Value();
                ValuePtr s(v2->to_s());
                // strdup
                v->set_str(strdup((std::string(s->value.str_value) + std::string(v1->value.str_value)).c_str()));
                stack.push(v);
            } else {
                ValuePtr s(v1->to_s());
                fprintf(stderr, "'%s' is not numeric or string.\n", s->value.str_value);
                exit(1); // TODO : die
            }
            break;
        }
        case OP_FUNCALL: {
            ValuePtr funname(stack.pop());
            const char *funname_c = funname->value.str_value;
            if (!(stack.size() >= (size_t) op->operand.int_value)) {
                printf("[BUG] bad argument: %s requires %d arguments but only %d items available on stack(OP_FUNCALL)\n", funname_c, op->operand.int_value, stack.size());
                dump_stack();
                abort();
            }
            assert(funname->value_type == VALUE_TYPE_STR);
            if (strcmp(funname->value.str_value, "print") == 0) {
                for (int i=0; i<op->operand.int_value; i++) {
                    ValuePtr v(stack.pop());
                    ValuePtr s(v->to_s());
                    printf("%s", s->value.str_value);
                }
            } else if (strcmp(funname->value.str_value, "p") == 0) {
                ValuePtr v(stack.pop());
                v->dump();
            } else if (strcmp(funname->value.str_value, "say") == 0) {
                for (int i=0; i<op->operand.int_value; i++) {
                    ValuePtr v(stack.pop());
                    ValuePtr s(v->to_s());
                    printf("%s\n", s->value.str_value);
                }
            } else if (strcmp(funname->value.str_value, "getenv") == 0) {
                assert(op->operand.int_value==1);
                ValuePtr v(stack.pop());
                ValuePtr s(v->to_s());
                Value *ret = new Value();
                ret->set_str(strdup(getenv(s->value.str_value)));
                stack.push(ret);
            } else if (strcmp(funname->value.str_value, "exit") == 0) {
                ValuePtr v(stack.pop());
                assert(v->value_type == VALUE_TYPE_INT);
                ValuePtr s(v->to_i());
                exit(s->value.int_value);
            } else {
                std::map<std::string, Value*>::iterator iter =  this->functions.find(funname->value.str_value);
                if (iter != this->functions.end()) {
                    Value *code = iter->second;
                    // printf("calling %s\n", funname->value.str_value);
                    {
                        FunctionFrame * fframe = new FunctionFrame();
                        fframe->return_address = pc;
                        fframe->orig_ops = ops;
                        fframe->top = stack.size() - op->operand.int_value;
                        function_frames->push_back(fframe);
                    }
                    pc = 0;
                    this->ops = code->value.code_value.opcodes;

                    LexicalVarsFrame *frame = new LexicalVarsFrame(lexical_vars_stack->back());
                    // TODO: vargs support
                    // TODO: kwargs support
                    assert(op->operand.int_value == code->value.code_value.params->size());
                    for (int i=0; i<op->operand.int_value; i++) {
                        Value* arg = stack.pop();
                        std::string *argname = code->value.code_value.params->at(i);
                        // printf("set lexical var: %d for %s\n", i, argname->c_str());
                        frame->setVar(argname, arg);
                    }
                    lexical_vars_stack->push_back(frame);

                    continue;
                } else {
                    fprintf(stderr, "Unknown function: %s\n", funname->value.str_value);
                }
            }
            break;
        }
        case OP_RETURN: {
            assert(function_frames->size() > 0);
            FunctionFrame *fframe = function_frames->back();
            function_frames->pop_back();
            pc = fframe->return_address+1;
            ops = fframe->orig_ops;
            // printf("RETURN :orig: %d, current: %d\n", fframe->top, stack.size());
            if (fframe->top == stack.size()) {
                Value * v = new Value();
                v->value_type = VALUE_TYPE_UNDEF;
                stack.push(v);
            }
            delete fframe;

            lexical_vars_stack->pop_back();

            continue;
        }
        case OP_ENTER: {
            LexicalVarsFrame *frame = new LexicalVarsFrame(lexical_vars_stack->back());
            lexical_vars_stack->push_back(frame);
            break;
        }
        case OP_LEAVE: {
            lexical_vars_stack->pop_back();
            break;
        }
        case OP_PUSH_IDENTIFIER: {
            // operand = the number of args
            Value *v = new Value;
            v->value.str_value = strdup(op->operand.str_value);
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
            // lexical vars
            Value *v = lexical_vars_stack->back()->find(op->operand.str_value);
            if (v) {
                // printf("found lexical var\n");
                v->retain();
                stack.push(v);
            } else { // TODO: remove this and use 'my' keyword
                v = new Value();
                v->value_type = VALUE_TYPE_UNDEF;
                lexical_vars_stack->back()->setVar(new std::string(op->operand.str_value), v);
                v->retain();
                stack.push(v);
            }
            break;
        }
        case OP_END: {
            goto END;
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
END:
    return;
}
