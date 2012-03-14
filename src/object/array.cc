#include "array.h"
#include "../vm.h"
#include "../value/array.h"
#include "../value/code.h"
#include "../package.h"
#include "../peek.h"
#include "../frame.h"

using namespace tora;

/**
 * Array#size()
 * 
 * Get the number of elements in an array.
 */
static SharedPtr<Value> av_size(VM *vm, Value* self) {
    SharedPtr<IntValue> size = new IntValue(self->upcast<ArrayValue>()->size());
    return size;
}

/**
 * Array#sort()
 * 
 * Get a sorted array. This method is unstable sort(Perl5's sort function is stable sort).
 */
static SharedPtr<Value> av_sort(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_ARRAY);
    // copy and sort.
    SharedPtr<ArrayValue> av = new ArrayValue(*(self->upcast<ArrayValue>()));
    av->sort();
    return av;
}

/**
 * Array#push($elem)
 * 
 * Push a object to the array. $elem put at end of array.
 *
 * Perl5: push(@array, $elem);
 */
static SharedPtr<Value> av_push(VM * vm, Value* self, Value* v) {
    self->upcast<ArrayValue>()->push(v);
    return self;
}

/**
 * $array.pop() : Any
 *
 * Pop a object from array.
 */
static SharedPtr<Value> av_pop(VM * vm, Value* self) {
    return self->upcast<ArrayValue>()->pop();
}

/**
 * $array.unshift($v)
 *
 * push $v to front.
 */
static SharedPtr<Value> av_unshift(VM * vm, Value* self, Value *v) {
    self->upcast<ArrayValue>()->values->push_front(
        v
    );
    return UndefValue::instance();
}

/**
 * $array.shift() : Any
 *
 * pop value from front.
 */
static SharedPtr<Value> av_shift(VM * vm, Value* self) {
    if (self->upcast<ArrayValue>()->size() > 0) {
        SharedPtr<Value> ret = self->upcast<ArrayValue>()->values->at(0);
        self->upcast<ArrayValue>()->values->pop_front();
        return ret;
    } else {
        return UndefValue::instance();
    }
}

static SharedPtr<Value> av_reverse(VM * vm, Value* self) {
    ArrayValue * src = self->upcast<ArrayValue>();
    SharedPtr<ArrayValue> nav = new ArrayValue(*src);
    std::reverse(nav->values->begin(), nav->values->end());
    return nav.get();
}

/**
 * ['hoge', 'fuga'].join(' ')
 *
 * concatenete strings in array.
 */
static SharedPtr<Value> av_join(VM * vm, Value* self, Value *inner) {
    assert(self->value_type == VALUE_TYPE_ARRAY);
    SharedPtr<ArrayValue> src = self->upcast<ArrayValue>();
    SharedPtr<StrValue> str = inner->to_s();
    std::string ret;
    for (auto iter=src->begin(); iter!=src->end(); ++iter) {
        ret += (*iter)->to_s()->str_value;
        if (iter+1 != src->end()) {
            ret += str.get()->str_value;
        }
    }
    return new StrValue(ret);
}

/**
 * [1,2,3].map(-> $n { say($n) })
 */
static SharedPtr<Value> av_map(VM * vm, Value* self, Value *code_v) {
    assert(self->value_type == VALUE_TYPE_ARRAY);
    SharedPtr<ArrayValue> src = self->upcast<ArrayValue>();
    SharedPtr<ArrayValue> ret = new ArrayValue();
    if (code_v->value_type != VALUE_TYPE_CODE) {
        throw new ExceptionValue("Code is not a foo");
    }
    SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
    for (auto iter=src->begin(); iter!=src->end(); ++iter) {
        vm->stack.push_back(*iter);

            int argcnt = 1;
            size_t pc = vm->pc;
            // TODO: catch exceptions in destroy
            SharedPtr<OPArray> end_ops = new OPArray();
            end_ops->push_back(new OP(OP_END), -1);
            vm->function_call(argcnt, code, UndefValue::instance());
            vm->frame_stack->back()->upcast<FunctionFrame>()->return_address = -1;
            SharedPtr<OPArray> orig_ops = vm->frame_stack->back()->upcast<FunctionFrame>()->orig_ops;
            vm->frame_stack->back()->upcast<FunctionFrame>()->orig_ops = end_ops;
            vm->pc = 0;
            vm->execute();

            // restore
            // TODO: restore variables by RAII
            vm->pc = pc;
            vm->ops = orig_ops;

        ret->push(vm->stack.back());
        vm->stack.pop_back();
    }
    return ret;
}

/**
 * $array.capacity() : Int
 *
 * returns the number of elements that can be held in currently allocated storage
 */
/*
static SharedPtr<Value> av_capacity(VM * vm, Value* self) {
    return new IntValue(self->upcast<ArrayValue>()->values->capacity());
}
*/

/**
 * $array.reserve($n)
 *
 * reserves storage
 */
/*
static SharedPtr<Value> av_reserve(VM * vm, Value* self, Value * v) {
    self->upcast<ArrayValue>()->values->reserve(v->to_int());
    return UndefValue::instance();
}
*/

void tora::Init_Array(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Array");
    pkg->add_method(vm->symbol_table->get_id("size"), new CallbackFunction(av_size));
    pkg->add_method(vm->symbol_table->get_id("sort"), new CallbackFunction(av_sort));
    pkg->add_method(vm->symbol_table->get_id("push"), new CallbackFunction(av_push));
    pkg->add_method(vm->symbol_table->get_id("pop"), new CallbackFunction(av_pop));
    pkg->add_method(vm->symbol_table->get_id("unshift"), new CallbackFunction(av_unshift));
    pkg->add_method(vm->symbol_table->get_id("shift"), new CallbackFunction(av_shift));
    pkg->add_method(vm->symbol_table->get_id("reverse"), new CallbackFunction(av_reverse));
    pkg->add_method(vm->symbol_table->get_id("join"), new CallbackFunction(av_join));
    pkg->add_method(vm->symbol_table->get_id("map"), new CallbackFunction(av_map));
    // pkg->add_method(vm->symbol_table->get_id("capacity"), new CallbackFunction(av_capacity));
    // pkg->add_method(vm->symbol_table->get_id("reserve"), new CallbackFunction(av_reserve));
}

