#include "vm.h"
#include "value.h"
#include "code.h"
#include <unistd.h>
#include <algorithm>

using namespace tora;

VM::VM(std::vector<SharedPtr<OP>>* ops_) {
    sp = 0;
    pc = 0;
    ops = new std::vector<SharedPtr<OP>>(*ops_);
    this->lexical_vars_stack = new std::vector<LexicalVarsFrame *>();
    this->lexical_vars_stack->push_back(new LexicalVarsFrame());
    this->function_frames = new std::vector<FunctionFrame*>();
}

VM::~VM() {
    delete this->ops;
    delete this->lexical_vars_stack;
    delete this->function_frames;
    while (this->stack.size() > 0) {
        this->stack.pop()->release();
    }
}

static void disasm_one(OP* op) {
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

// run program
void VM::execute() {
    DBG2("************** VM::execute\n");

    for (;;) {
#ifdef DEBUG
        DBG2("[DEBUG] ");
        disasm_one(ops->at(pc));
#endif

        SharedPtr<OP> op = ops->at(pc);
        switch (op->op_type) {
        case OP_PUSH_TRUE: {
            stack.push(BoolValue::true_instance());
            break;
        }
        case OP_PUSH_FALSE: {
            stack.push(BoolValue::false_instance());
            break;
        }
        case OP_PUSH_INT: {
            stack.push(new IntValue(op->operand.int_value));
            break;
        }
        case OP_PUSH_DOUBLE: {
            stack.push(new DoubleValue(op->operand.double_value));
            break;
        }
        case OP_PUSH_STRING: {
            Value *sv = ((ValueOP*)&(*(op)))->value;
            sv->retain();
            stack.push(sv);
            break;
        }
        case OP_PUSH_VALUE: {
            Value *v = ((ValueOP*)&(*(op)))->value;
            stack.push(v);
            break;
        }
        case OP_DEFINE_METHOD: {
            SharedPtr<Value> code = stack.pop(); // code object
            assert(code->value_type == VALUE_TYPE_CODE);
            const char *funcname = ((ValueOP*)&(*(op)))->value->to_str()->str_value;
            this->add_function(funcname, code);
            break;
        }
#define BINOP(optype, op) \
        case optype: { \
            ValuePtr v1(stack.pop()); /* rvalue */ \
            ValuePtr v2(stack.pop()); /* lvalue */ \
            if (v2->value_type == VALUE_TYPE_DOUBLE) { \
                if (v1->value_type == VALUE_TYPE_DOUBLE) { \
                    DoubleValue *v = new DoubleValue(v2->to_double()->double_value op v1->to_double()->double_value); \
                    stack.push(v); \
                } else if (v1->value_type == VALUE_TYPE_INT) {\
                    DoubleValue *v = new DoubleValue(v2->to_double()->double_value op (double)v1->to_int()->int_value); \
                    stack.push(v); \
                } \
            } else if (v2->value_type == VALUE_TYPE_INT) { \
                IntValue *v = new IntValue(v2->to_int()->int_value op v1->to_int()->int_value); \
                stack.push(v); \
            } else {  \
                ValuePtr s(v2->to_s()); \
                fprintf(stderr, "'%s' is not numeric.\n", s->to_str()->str_value); \
                exit(1); /* TODO: die */ \
            } \
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
                ValuePtr i(v2->to_i());
                IntValue *v = new IntValue(v2->to_int()->int_value + v1->to_int()->int_value);
                stack.push(v);
            } else if (v1->value_type == VALUE_TYPE_STR) {
                // TODO: support null terminated string
                StrValue *v = new StrValue();
                ValuePtr s(v2->to_s());
                // strdup
                v->set_str(strdup((std::string(s->to_str()->str_value) + std::string(v1->to_str()->str_value)).c_str()));
                stack.push(v);
            } else {
                ValuePtr s(v1->to_s());
                fprintf(stderr, "'%s' is not numeric or string.\n", s->to_str()->str_value);
                exit(1); // TODO : die
            }
            break;
        }
        case OP_FUNCALL: {
            ValuePtr funname(stack.pop());
            const char *funname_c = funname->to_str()->str_value;
            if (!(stack.size() >= (size_t) op->operand.int_value)) {
                printf("[BUG] bad argument: %s requires %d arguments but only %d items available on stack(OP_FUNCALL)\n", funname_c, op->operand.int_value, stack.size());
                dump_stack();
                abort();
            }
            assert(funname->value_type == VALUE_TYPE_STR);
            if (strcmp(funname->to_str()->str_value, "print") == 0) {
                for (int i=0; i<op->operand.int_value; i++) {
                    ValuePtr v(stack.pop());
                    ValuePtr s(v->to_s());
                    printf("%s", s->to_str()->str_value);
                }
            } else if (strcmp(funname->to_str()->str_value, "p") == 0) {
                ValuePtr v(stack.pop());
                v->dump();
            } else if (strcmp(funname->to_str()->str_value, "say") == 0) {
                for (int i=0; i<op->operand.int_value; i++) {
                    ValuePtr v(stack.pop());
                    ValuePtr s(v->to_s());
                    printf("%s\n", s->to_str()->str_value);
                }
            } else if (strcmp(funname->to_str()->str_value, "getenv") == 0) {
                assert(op->operand.int_value==1);
                ValuePtr v(stack.pop());
                ValuePtr s(v->to_s());
                char *env = getenv(s->to_str()->str_value);
                if (env) {
                    StrValue *ret = new StrValue();
                    ret->set_str(strdup(env));
                    stack.push(ret);
                } else {
                    stack.push(UndefValue::instance());
                }
            } else if (strcmp(funname->to_str()->str_value, "usleep") == 0) {
                // TODO: remove later
                ValuePtr v(stack.pop());
                assert(v->value_type == VALUE_TYPE_INT);
                ValuePtr s(v->to_i());
                usleep(s->to_int()->int_value);
            } else if (strcmp(funname->to_str()->str_value, "exit") == 0) {
                ValuePtr v(stack.pop());
                assert(v->value_type == VALUE_TYPE_INT);
                ValuePtr s(v->to_i());
                exit(s->to_int()->int_value);
            } else {
                auto iter =  this->functions.find(funname->to_str()->str_value);
                if (iter != this->functions.end()) {
                    CodeValue *code = iter->second->to_code();
                    // printf("calling %s\n", funname->to_str()->str_value);
                    {
                        FunctionFrame * fframe = new FunctionFrame();
                        fframe->return_address = pc;
                        fframe->orig_ops = ops;
                        fframe->top = stack.size() - op->operand.int_value;
                        function_frames->push_back(fframe);
                    }
                    pc = 0;
                    this->ops = code->code_opcodes;

                    LexicalVarsFrame *frame = new LexicalVarsFrame(lexical_vars_stack->back());
                    // TODO: vargs support
                    // TODO: kwargs support
                    assert(op->operand.int_value == (int)code->code_params->size());
                    for (int i=0; i<op->operand.int_value; i++) {
                        SharedPtr<Value> arg = stack.pop();
                        /*
                        std::string *argname = code->code_params->at(i);
                        // printf("set lexical var: %d for %s\n", i, argname->c_str());
                        frame->setVar(argname, arg);
                        */
                        frame->setVar(i, arg);
                    }
                    lexical_vars_stack->push_back(frame);

                    continue;
                } else {
                    fprintf(stderr, "Unknown function: %s\n", funname->to_str()->str_value);
                }
            }
            break;
        }
        case OP_METHOD_CALL: {
            ValuePtr object(stack.pop());
            ValuePtr funname(stack.pop());
            const char *funname_c = funname->to_str()->str_value;
            if (!(stack.size() >= (size_t) op->operand.int_value)) {
                printf("[BUG] bad argument: %s requires %d arguments but only %d items available on stack(OP_FUNCALL)\n", funname_c, op->operand.int_value, stack.size());
                dump_stack();
                abort();
            }
            assert(funname->value_type == VALUE_TYPE_STR);
            switch (object->value_type) {
            case VALUE_TYPE_ARRAY: {
                ArrayValue*av = object->to_array();
                if (strcmp(funname->to_str()->str_value, "size") == 0) {
                    IntValue *size = new IntValue(av->size());
                    stack.push(size);
                }
                break;
            }
            default:
                fprintf(stderr, "Unknown method %s for %s\n", funname->to_str()->str_value, object->type_str());
                // TODO throw exception
                break;
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
            if (fframe->top == (int)stack.size()) {
                stack.push(UndefValue::instance());
            }
            fframe->release();

            lexical_vars_stack->back()->release();
            lexical_vars_stack->pop_back();

            continue;
        }
        case OP_ENTER: {
            LexicalVarsFrame *frame = new LexicalVarsFrame(lexical_vars_stack->back());
            lexical_vars_stack->push_back(frame);
            break;
        }
        case OP_LEAVE: {
            lexical_vars_stack->back()->release();
            lexical_vars_stack->pop_back();
            break;
        }
        case OP_PUSH_IDENTIFIER: {
            Value *sv = ((ValueOP*)&(*(op)))->value;
            sv->retain();
            stack.push(sv);
            break;
        }
        case OP_DUMP: {
            this->dump_stack();
            break;
        }
        case OP_JUMP_IF_FALSE: {
            ValuePtr v(stack.pop());

            ValuePtr b(v->to_b());
            if (!b->to_bool()->bool_value) {
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
                ValuePtr i2(v2->to_i()); \
                BoolValue *result = BoolValue::instance(v1->to_int()->int_value op i2->to_int()->int_value); \
                result->retain(); \
                stack.push(result); \
                break; \
            } \
            default: \
                abort(); \
            } \
            break; \
        }
        CMPOP(OP_EQ, ==)
        CMPOP(OP_GT, >)
        CMPOP(OP_LT, <)
        CMPOP(OP_GE, >=)
        CMPOP(OP_LE, <=)

        // variable
        case OP_SETLOCAL: {
            SharedPtr<Value> rvalue = stack.pop();
            lexical_vars_stack->back()->setVar(
                op->operand.int_value,
                rvalue
            );
            stack.push(rvalue);
            break;
        }
        case OP_SETDYNAMIC: {
            // lexical_vars_stack->back()->dump_vars();
            SharedPtr<LexicalVarsFrame> frame = lexical_vars_stack->back();
            int level = (op->operand.int_value >> 16) & 0x0000FFFF;
            int no    = op->operand.int_value & 0x0000ffff;
            DBG("SETDYNAMIC %d, %d\n", level, no);
            for (int i=0; i<level; i++) {
                frame = frame->up;
            }
            SharedPtr<Value> rvalue = stack.pop();
            frame->setVar(
                no,
                rvalue
            );
            stack.push(rvalue);
            break;
        }
        case OP_GETDYNAMIC: {
            // lexical vars
            SharedPtr<LexicalVarsFrame> frame = lexical_vars_stack->back();
            int level = (op->operand.int_value >> 16) & 0x0000FFFF;
            int no    = op->operand.int_value & 0x0000ffff;
            for (int i=0; i<level; i++) {
                frame = frame->up;
            }
            Value *v = frame->find(no);
            if (v) {
                DBG2("found lexical var\n");
                v->retain();
                stack.push(v);
            } else { // TODO: remove this and use 'my' keyword?
                DBG2("There is no variable...\n");
                v = UndefValue::instance();
                lexical_vars_stack->back()->setVar(op->operand.int_value, v);
                stack.push(v);
            }
            break;
        }
        case OP_GETLOCAL: {
            // lexical vars
            Value *v = lexical_vars_stack->back()->find(op->operand.int_value);
            if (v) {
                // printf("found lexical var\n");
                v->retain();
                stack.push(v);
            } else { // TODO: remove this and use 'my' keyword?
                v = UndefValue::instance();
                lexical_vars_stack->back()->setVar(op->operand.int_value, v);
                v->retain();
                stack.push(v);
            }
            break;
        }

        case OP_GET_ITEM: {
            ValuePtr index(stack.pop());
            ValuePtr container(stack.pop());

            SharedPtr<Value> ret = container->get_item(&(*index));
            stack.push(ret);
            break;
        }
        case OP_SET_ITEM: {
            ValuePtr rvalue(stack.pop());
            ValuePtr index(stack.pop());
            ValuePtr container(stack.pop());

            rvalue->retain();
            container->set_item(&(*index), &(*rvalue));
            rvalue->retain();
            stack.push(&(*rvalue));
            break;
        }

        case OP_UNARY_NEGATIVE: {
            ValuePtr v(stack.pop());
            Value * result = v->tora__neg__();
            stack.push(result);
            break;
        }

        case OP_MAKE_ARRAY: {
            SharedPtr<ArrayValue> a = new ArrayValue();
            int array_size = op->operand.int_value;
            for (int i=0; i<array_size; i++) {
                SharedPtr<Value> v = stack.pop();
                a->push(v);
            }
            stack.push(a);
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
