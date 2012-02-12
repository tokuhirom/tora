#include "vm.h"
#include "value.h"
#include "value/hash.h"
#include "value/code.h"
#include "regexp.h"
#include <unistd.h>
#include <algorithm>
#include <functional>

using namespace tora;

VM::VM(std::vector<SharedPtr<OP>>* ops_) {
    sp = 0;
    pc = 0;
    ops = new std::vector<SharedPtr<OP>>(*ops_);
    this->lexical_vars_stack = new std::vector<SharedPtr<LexicalVarsFrame>>();
    this->lexical_vars_stack->push_back(new LexicalVarsFrame());
    this->function_frames = new std::vector<SharedPtr<FunctionFrame>>();
    this->global_vars = new std::vector<SharedPtr<Value>>();
}

VM::~VM() {
    delete this->global_vars;
    delete this->ops;
    delete this->lexical_vars_stack;
    delete this->function_frames;
}

void VM::init_globals(int argc, char**argv) {
    // $ARGV
    SharedPtr<ArrayValue> avalue = new ArrayValue();
    for (int i=0; i<argc; i++) {
        avalue->push(new StrValue(argv[i]));
    }
    this->global_vars->push_back(avalue);
}

template <class operationI, class operationD>
void VM::binop(operationI operation_i, operationD operation_d) {
    SharedPtr<Value> v1(stack.pop()); /* rvalue */
    SharedPtr<Value> v2(stack.pop()); /* lvalue */

    if (v2->value_type == VALUE_TYPE_DOUBLE) {
        if (v1->value_type == VALUE_TYPE_DOUBLE) {
            SharedPtr<DoubleValue>v = new DoubleValue(operation_d(v2->upcast<DoubleValue>()->double_value, v1->upcast<DoubleValue>()->double_value));
            stack.push(v);
        } else if (v1->value_type == VALUE_TYPE_INT) {
            SharedPtr<DoubleValue>v = new DoubleValue(operation_d(v2->upcast<DoubleValue>()->double_value, (double)v1->upcast<IntValue>()->int_value));
            stack.push(v);
        }
    } else if (v2->value_type == VALUE_TYPE_INT) {
        SharedPtr<IntValue> v = new IntValue(operation_i(v2->upcast<IntValue>()->int_value, v1->upcast<IntValue>()->int_value));
        stack.push(v);
    } else { 
        SharedPtr<Value> s(v2->to_s());
        fprintf(stderr, "'%s' is not numeric.\n", s->upcast<StrValue>()->str_value.c_str());
        exit(1); /* TODO: die */
    }
}

template <class operationI, class operationD>
void VM::cmpop(operationI operation_i, operationD operation_d) {
    SharedPtr<Value> v1(stack.pop());
    SharedPtr<Value> v2(stack.pop());
 
    switch (v1->value_type) {
    case VALUE_TYPE_INT: {
        SharedPtr<Value> i2(v2->to_i());
        SharedPtr<BoolValue> result = BoolValue::instance(operation_i(v1->upcast<IntValue>()->int_value, i2->upcast<IntValue>()->int_value));
        stack.push(result);
        break;
    }
    default:
        printf("UNKNOWN MATCHING PATTERN:: %s\n", opcode2name[ops->at(pc)->op_type]);
        v1->dump();
        v2->dump();
        abort();
    }
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
            stack.push(new BoolValue(true));
            break;
        }
        case OP_PUSH_FALSE: {
            stack.push(new BoolValue(false));
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
            SharedPtr<Value> sv = ((ValueOP*)&(*(op)))->value;
            stack.push(sv);
            break;
        }
        case OP_PUSH_VALUE: {
            SharedPtr<Value> v = ((ValueOP*)&(*(op)))->value;
            stack.push(v);
            break;
        }
        case OP_NEW_RANGE: {
            SharedPtr<Value> l = stack.pop();
            SharedPtr<Value> r = stack.pop();
            stack.push(new RangeValue(l->upcast<IntValue>(), r->upcast<IntValue>()));
            break;
        }
        case OP_DEFINE_METHOD: {
            SharedPtr<Value> code = stack.pop(); // code object
            assert(code->value_type == VALUE_TYPE_CODE);
            const char *funcname = ((ValueOP*)&(*(op)))->value->upcast<StrValue>()->str_value.c_str();
            this->add_function(funcname, code);
            break;
        }
        case OP_SUB: {
            std::minus<int>    i;
            std::minus<double> d;
            this->binop(i, d);
            break;
        }
        case OP_DIV: {
            std::divides<int>    i;
            std::divides<double> d;
            this->binop(i, d);
            break;
        }
        case OP_MUL: {
            std::multiplies<int>    i;
            std::multiplies<double> d;
            this->binop(i, d);
            break;
        }
        case OP_ADD: {
            SharedPtr<Value> v1(stack.pop());
            SharedPtr<Value> v2(stack.pop());
            if (v1->is_numeric()) {
                SharedPtr<Value> i(v2->to_i());
                SharedPtr<IntValue>v = new IntValue(v2->upcast<IntValue>()->int_value + v1->upcast<IntValue>()->int_value);
                stack.push(v);
            } else if (v1->value_type == VALUE_TYPE_STR) {
                // TODO: support null terminated string
                SharedPtr<StrValue>v = new StrValue();
                SharedPtr<Value> s(v2->to_s());
                v->set_str(std::string(s->upcast<StrValue>()->str_value.c_str()) + std::string(v1->upcast<StrValue>()->str_value.c_str()));
                stack.push(v);
            } else {
                SharedPtr<Value> s(v1->to_s());
                fprintf(stderr, "'%s' is not numeric or string.\n", s->upcast<StrValue>()->str_value.c_str());
                exit(1); // TODO : die
            }
            break;
        }
        case OP_FUNCALL: {
            SharedPtr<Value> funname(stack.pop());
            const char *funname_c = funname->upcast<StrValue>()->str_value.c_str();
            if (!(stack.size() >= (size_t) op->operand.int_value)) {
                printf("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", funname_c, op->operand.int_value, stack.size());
                dump_stack();
                abort();
            }
            assert(funname->value_type == VALUE_TYPE_STR);
            if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "print") == 0) {
                for (int i=0; i<op->operand.int_value; i++) {
                    SharedPtr<Value> v(stack.pop());
                    SharedPtr<Value> s(v->to_s());
                    printf("%s", s->upcast<StrValue>()->str_value.c_str());
                }
            } else if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "p") == 0) {
                SharedPtr<Value> v(stack.pop());
                v->dump();
            } else if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "say") == 0) {
                for (int i=0; i<op->operand.int_value; i++) {
                    SharedPtr<Value> v(stack.pop());
                    SharedPtr<Value> s(v->to_s());
                    printf("%s\n", s->upcast<StrValue>()->str_value.c_str());
                }
            } else if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "getenv") == 0) {
                assert(op->operand.int_value==1);
                SharedPtr<Value> v(stack.pop());
                SharedPtr<Value> s(v->to_s());
                char *env = getenv(s->upcast<StrValue>()->str_value.c_str());
                if (env) {
                    SharedPtr<StrValue> ret = new StrValue();
                    ret->set_str(env);
                    stack.push(ret);
                } else {
                    stack.push(UndefValue::instance());
                }
            } else if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "usleep") == 0) {
                // TODO: remove later
                SharedPtr<Value> v(stack.pop());
                assert(v->value_type == VALUE_TYPE_INT);
                SharedPtr<Value> s(v->to_i());
                usleep(s->upcast<IntValue>()->int_value);
            } else if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "exit") == 0) {
                SharedPtr<Value> v(stack.pop());
                assert(v->value_type == VALUE_TYPE_INT);
                SharedPtr<Value> s(v->to_i());
                exit(s->upcast<IntValue>()->int_value);
            } else {
                auto iter =  this->functions.find(funname->upcast<StrValue>()->str_value.c_str());
                if (iter != this->functions.end()) {
                    SharedPtr<CodeValue> code = iter->second->to_code();
                    // printf("calling %s\n", funname->upcast<StrValue>()->str_value.c_str());
                    {
                        SharedPtr<FunctionFrame> fframe = new FunctionFrame();
                        fframe->return_address = pc;
                        fframe->orig_ops = ops;
                        fframe->top = stack.size() - op->operand.int_value;
                        function_frames->push_back(fframe);
                    }
                    pc = 0;
                    this->ops = code->code_opcodes;

                    SharedPtr<LexicalVarsFrame> frame = new LexicalVarsFrame(lexical_vars_stack->back());
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
                    fprintf(stderr, "Unknown function: %s\n", funname->upcast<StrValue>()->str_value.c_str());
                }
            }
            break;
        }
        case OP_METHOD_CALL: {
            SharedPtr<Value> object(stack.pop());
            SharedPtr<Value> funname(stack.pop());
            const char *funname_c = funname->upcast<StrValue>()->str_value.c_str();
            if (!(stack.size() >= (size_t) op->operand.int_value)) {
                printf("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", funname_c, op->operand.int_value, stack.size());
                dump_stack();
                abort();
            }
            assert(funname->value_type == VALUE_TYPE_STR);
            switch (object->value_type) {
            case VALUE_TYPE_ARRAY: {
                SharedPtr<ArrayValue>av = object->upcast<ArrayValue>();
                if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "size") == 0) {
                    SharedPtr<IntValue> size = new IntValue(av->size());
                    stack.push(size);
                }
                break;
            }
            case VALUE_TYPE_STR: {
                SharedPtr<StrValue>sv = object->upcast<StrValue>();
                if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "match") == 0) {
                    if (op->operand.int_value == 1) {
                        SharedPtr<AbstractRegexpValue> regex = stack.pop()->upcast<AbstractRegexpValue>();
                        SharedPtr<BoolValue> b = new BoolValue(regex->match(std::string(sv->str_value.c_str())));
                        stack.push(b);
                    } else {
                        fprintf(stderr, "Missing args for 'match'\n");
                        abort();
                    }
                } else if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "length") == 0) {
                    if (op->operand.int_value == 0) {
                        stack.push(new IntValue(sv->length()));
                    } else {
                        fprintf(stderr, "length needs 0 argument\n");
                        abort();
                    }
                }
                break;
            }
            default:
                fprintf(stderr, "Unknown method %s for %s\n", funname->upcast<StrValue>()->str_value.c_str(), object->type_str());
                // TODO throw exception
                break;
            }
            break;
        }
        case OP_RETURN: {
            assert(function_frames->size() > 0);
            SharedPtr<FunctionFrame> fframe = function_frames->back();
            function_frames->pop_back();
            pc = fframe->return_address+1;
            ops = fframe->orig_ops;
            // printf("RETURN :orig: %d, current: %d\n", fframe->top, stack.size());
            if (fframe->top == (int)stack.size()) {
                stack.push(UndefValue::instance());
            }

            lexical_vars_stack->pop_back();

            continue;
        }
        case OP_ENTER: {
            SharedPtr<LexicalVarsFrame> frame = new LexicalVarsFrame(lexical_vars_stack->back());
            lexical_vars_stack->push_back(frame);
            break;
        }
        case OP_LEAVE: {
            lexical_vars_stack->pop_back();
            break;
        }
        case OP_PUSH_IDENTIFIER: {
            SharedPtr<Value> sv = ((ValueOP*)&(*(op)))->value;
            stack.push(sv);
            break;
        }
        case OP_DUMP: {
            this->dump_stack();
            break;
        }
        case OP_JUMP_IF_FALSE: {
            SharedPtr<Value> v(stack.pop());

            SharedPtr<Value> b(v->to_b());
            if (!b->upcast<BoolValue>()->bool_value) {
                pc = op->operand.int_value-1;
            }
            break;
        }
        case OP_JUMP: {
            pc = op->operand.int_value-1;
            break;
        }
        case OP_EQ: {
            std::equal_to<int> i;
            std::equal_to<double> d;
            this->cmpop(i, d);
            break;
        }
        case OP_GT: {
            std::greater<int> i;
            std::greater<double> d;
            this->cmpop(i, d);
            break;
        }
        case OP_LT: {
            std::less<int> i;
            std::less<double> d;
            this->cmpop(i, d);
            break;
        }
        case OP_GE: {
            std::greater_equal<int> i;
            std::greater_equal<double> d;
            this->cmpop(i, d);
            break;
        }
        case OP_LE: {
            std::less_equal<int> i;
            std::less_equal<double> d;
            this->cmpop(i, d);
            break;
        }

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
            SharedPtr<Value>v = frame->find(no);
            if (v) {
                DBG2("found lexical var\n");
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
            SharedPtr<Value>v = lexical_vars_stack->back()->find(op->operand.int_value);
            if (v) {
                // printf("found lexical var\n");
                stack.push(v);
            } else { // TODO: remove this and use 'my' keyword?
                v = UndefValue::instance();
                lexical_vars_stack->back()->setVar(op->operand.int_value, v);
                stack.push(v);
            }
            break;
        }

        case OP_GET_ITEM: {
            SharedPtr<Value> index(stack.pop());
            SharedPtr<Value> container(stack.pop());

            SharedPtr<Value> ret = container->get_item(&(*index));
            stack.push(ret);
            break;
        }
        case OP_SET_ITEM: {
            SharedPtr<Value> rvalue(stack.pop());
            SharedPtr<Value> index(stack.pop());
            SharedPtr<Value> container(stack.pop());

            container->set_item(index, rvalue);
            stack.push(&(*rvalue));
            break;
        }

        case OP_UNARY_NEGATIVE: {
            SharedPtr<Value> v(stack.pop());
            SharedPtr<Value> result = v->tora__neg__();
            stack.push(result);
            break;
        }

        case OP_MAKE_HASH: {
            SharedPtr<HashValue> h = new HashValue();
            int size = op->operand.int_value;
            for (int i=0; i<size; i+=2) {
                SharedPtr<StrValue> k = stack.pop()->to_s();
                SharedPtr<Value> v = stack.pop();
                h->set(k->str_value, v);
            }
            stack.push(h);
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

        case OP_UNARY_INCREMENT: {
            SharedPtr<IntValue> i = stack.pop()->upcast<IntValue>();
            if (i->value_type == VALUE_TYPE_INT) {
                i->tora__incr__();
                stack.push(i);
            } else {
                abort(); // TODO: throw exception
            }
            break;
        }
        case OP_GETGLOBAL: {
            int globalvarno = op->operand.int_value;
            stack.push(this->global_vars->at(globalvarno));
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
