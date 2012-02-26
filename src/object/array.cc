#include "../vm.h"
#include "array.h"

using namespace tora;

static SharedPtr<Value> av_size(SharedPtr<Value>& self) {
    SharedPtr<IntValue> size = new IntValue(self->upcast<ArrayValue>()->size());
    return size;
}

void tora::Init_Array(VM* vm) {
    MetaClass meta(vm, VALUE_TYPE_ARRAY);
    meta.add_method("size", av_size);
}

