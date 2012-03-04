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

void tora::Init_Array(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Array");
    pkg->add_method(vm->symbol_table->get_id("size"), new CallbackFunction(av_size));
    pkg->add_method(vm->symbol_table->get_id("sort"), new CallbackFunction(av_sort));
    pkg->add_method(vm->symbol_table->get_id("push"), new CallbackFunction(av_push));
}

