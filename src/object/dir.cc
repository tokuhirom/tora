#include "../shared_ptr.h"
#include "../vm.h"
#include "../value/object.h"
#include "../value/pointer.h"
#include "dir.h"
#include <sys/types.h>
#include <dirent.h>

using namespace tora;

static SharedPtr<Value> dir_read(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> v = self->upcast<ObjectValue>()->get_value(vm->symbol_table->get_id("__d"));
    assert(v->value_type = VALUE_TYPE_POINTER);
    DIR * dp = (DIR*)v->upcast<PointerValue>()->ptr();
    assert(dp);
    // TODO: support readdir_r
    struct dirent * ent = readdir(dp);
    if (ent) {
        return new StrValue(ent->d_name);
    } else {
        return UndefValue::instance();
    }
}

static SharedPtr<Value> dir_DESTROY(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> v = self->upcast<ObjectValue>()->get_value(vm->symbol_table->get_id("__d"));
    assert(v->value_type = VALUE_TYPE_POINTER);
    DIR * dp = (DIR*)v->upcast<PointerValue>()->ptr();
    assert(dp);
    closedir(dp);
#ifndef NDEBUG
    self->upcast<ObjectValue>()->set_value(vm->symbol_table->get_id("__d"), UndefValue::instance());
#endif
    return UndefValue::instance();
}

void tora::Init_Dir(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Dir");
    pkg->add_method(vm->symbol_table->get_id("read"), new CallbackFunction(dir_read));
    pkg->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(dir_DESTROY));
}
