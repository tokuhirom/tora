#include "../vm.h"
#include "array.h"

using namespace tora;

static SharedPtr<Value> av_size(SharedPtr<Value>& self) {
    SharedPtr<IntValue> size = new IntValue(self->upcast<ArrayValue>()->size());
    return size;
}

static SharedPtr<Value> av_sort(SharedPtr<Value>& self) {
    assert(self->value_type == VALUE_TYPE_ARRAY);
    // copy and sort.
    SharedPtr<ArrayValue> av = new ArrayValue(*(self->upcast<ArrayValue>()));
    av->sort();
    return av;
}

void tora::Init_Array(VM* vm) {
    MetaClass meta(vm, VALUE_TYPE_ARRAY);
    meta.add_method("size", av_size);
    meta.add_method("sort", av_sort);
}

