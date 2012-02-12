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

