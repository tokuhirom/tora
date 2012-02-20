#include "vm.h"
#include "value.h"
#include "value/hash.h"
#include "value/code.h"
#include "value/regexp.h"
#include "value/file.h"
#include <unistd.h>
#include <algorithm>
#include <functional>

using namespace tora;

VM::VM(std::vector<SharedPtr<OP>>* ops_, SharedPtr<SymbolTable> &symbol_table_) {
    sp = 0;
    pc = 0;
    symbol_table = symbol_table_;
    ops = new std::vector<SharedPtr<OP>>(*ops_);
    this->frame_stack = new std::vector<SharedPtr<LexicalVarsFrame>>();
    this->frame_stack->push_back(new LexicalVarsFrame(0));
    this->global_vars = new std::vector<SharedPtr<Value>>();
}

VM::~VM() {
    delete this->global_vars;
    delete this->ops;
    delete this->frame_stack;

    {
        auto iter = standard.begin();
        for (; iter!=standard.end(); iter++) {
            auto mp = iter->second;
            auto iter2 = mp->begin();
            for (; iter2!=mp->end(); iter2++) {
                delete iter2->second;
            }
            delete mp;
        }
    }

    {
        // std::map<ID, CallbackFunction*> builtin_functions;
        auto iter = builtin_functions.begin();
        for (;iter != builtin_functions.end(); iter++) {
            delete iter->second;
        }
    }
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

void VM::die(SharedPtr<Value> & exception) {
    while (1) {
        if (frame_stack->size() == 1) {
            fprintf(stderr, "died\n");
            exception->dump();
            break;
        }

        SharedPtr<LexicalVarsFrame> frame = frame_stack->back();
        if (frame->type == FRAME_TYPE_FUNCTION) {
            SharedPtr<FunctionFrame> fframe = frame->upcast<FunctionFrame>();
            pc = fframe->return_address;
            ops = fframe->orig_ops;

            while (stack.size() > frame->top) {
                stack.pop();
            }

            frame_stack->pop_back();
        } else if (frame->type == FRAME_TYPE_TRY) {
            SharedPtr<TryFrame> tframe = frame->upcast<TryFrame>();
            pc = tframe->return_address;

            while (stack.size() > frame->top) {
                stack.pop();
            }
            SharedPtr<TupleValue> t = new TupleValue();
            t->push(UndefValue::instance());
            t->push(exception);

            frame_stack->pop_back();

            stack.push(t);

            break;
        } else {
            // printf("THIS IS NOT A FUNCTION FRAME\n");
            while (stack.size() > frame->top) {
                stack.pop();
            }
            frame_stack->pop_back();
        }
    }
}

static SharedPtr<Value> av_size(SharedPtr<Value>& self) {
    SharedPtr<IntValue> size = new IntValue(self->upcast<ArrayValue>()->size());
    return size;
}

static SharedPtr<Value> str_length(SharedPtr<Value>& self) {
    return new IntValue(self->upcast<StrValue>()->length());
}

static SharedPtr<Value> str_match(SharedPtr<Value>&self, SharedPtr<Value>&arg1) {
    SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
    SharedPtr<BoolValue> b = new BoolValue(regex->match(self->upcast<StrValue>()->str_value));
    return b;
}

static SharedPtr<Value> builtin_p(SharedPtr<Value>&arg1) {
    arg1->dump();
    return UndefValue::instance();
}

static SharedPtr<Value> builtin_getenv(SharedPtr<Value> &v) {
    SharedPtr<Value> s(v->to_s());
    char *env = getenv(s->upcast<StrValue>()->str_value.c_str());
    if (env) {
        SharedPtr<StrValue> ret = new StrValue();
        ret->set_str(env);
        return ret;
    } else {
        return UndefValue::instance();
    }
}

static SharedPtr<Value> builtin_exit(SharedPtr<Value> &v) {
    assert(v->value_type == VALUE_TYPE_INT);
    SharedPtr<Value> s(v->to_i());
    exit(s->upcast<IntValue>()->int_value);
}

static SharedPtr<Value> builtin_open(const std::vector<SharedPtr<Value>> & args) {
    SharedPtr<Value> filename(args.at(1));
    std::string mode;
    if (args.size() >= 2) {
        mode = args.at(1)->upcast<StrValue>()->str_value.c_str();
    } else {
        mode = "rb";
    }
    // TODO: check \0
    SharedPtr<FileValue> file = new FileValue();
    if (file->open(
        filename->upcast<StrValue>()->str_value,
        mode
    )) {
        return file;
    } else {
        abort(); // todo: throw exception
    }
}


static SharedPtr<Value> builtin_print(const std::vector<SharedPtr<Value>> & args) {
    auto iter = args.begin();
    for (; iter!=args.end(); iter++) {
        SharedPtr<Value> v(*iter);
        SharedPtr<Value> s(v->to_s());
        printf("%s", s->upcast<StrValue>()->str_value.c_str());
    }
    return UndefValue::instance();
}

static SharedPtr<Value> builtin_say(const std::vector<SharedPtr<Value>> & args) {
    auto iter = args.begin();
    for (; iter!=args.end(); iter++) {
        SharedPtr<Value> v(*iter);
        SharedPtr<Value> s(v->to_s());
        printf("%s\n", s->upcast<StrValue>()->str_value.c_str());
    }
    return UndefValue::instance();
}

void VM::register_standard_methods() {
    {
        MetaClass meta(this, VALUE_TYPE_ARRAY);
        meta.add_method("size", av_size);
    }
    {
        MetaClass meta(this, VALUE_TYPE_STR);
        meta.add_method("length", str_length);
        meta.add_method("match", str_match);
    }
    // this->add_builtin_function("print");
    this->add_builtin_function("p", builtin_p);
    this->add_builtin_function("getenv", builtin_getenv);
    this->add_builtin_function("exit", builtin_exit);
    this->add_builtin_function("say", builtin_say);
    this->add_builtin_function("print", builtin_print);
}

