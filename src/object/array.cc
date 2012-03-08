#include "../vm.h"
#include "../value/array.h"
#include "array.h"

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
    // pkg->add_method(vm->symbol_table->get_id("capacity"), new CallbackFunction(av_capacity));
    // pkg->add_method(vm->symbol_table->get_id("reserve"), new CallbackFunction(av_reserve));
}

