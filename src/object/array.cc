#include "../vm.h"
#include "array.h"

using namespace tora;

static SharedPtr<Value> av_size(VM *vm, Value* self) {
    SharedPtr<IntValue> size = new IntValue(self->upcast<ArrayValue>()->size());
    return size;
}

static SharedPtr<Value> av_sort(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_ARRAY);
    // copy and sort.
    SharedPtr<ArrayValue> av = new ArrayValue(*(self->upcast<ArrayValue>()));
    av->sort();
    return av;
}

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

