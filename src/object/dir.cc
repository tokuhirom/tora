#include <sys/types.h>
#include <dirent.h>

#include "dir.h"
#include "../shared_ptr.h"
#include "../vm.h"
#include "../package.h"
#include "../value/object.h"
#include "../value/pointer.h"

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

static SharedPtr<Value> dir___iter__(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    ObjectValue* v = new ObjectValue(vm->symbol_table->get_id("Dir::Iterator"), vm);
    v->set_value(vm->symbol_table->get_id("__dir"), self);
    return v;
}

static SharedPtr<Value> dir_Iterator___next__(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> dir = self->upcast<ObjectValue>()->get_value(vm->symbol_table->get_id("__dir"));
    if (dir->value_type != VALUE_TYPE_OBJECT) { return new ExceptionValue("[BUG] This is not a Directory value.: %s", dir->type_str()); }
    SharedPtr<Value> ret = dir_read(vm, dir.get());
    if (ret->value_type == VALUE_TYPE_UNDEF) { return new StopIterationExceptionValue(); }
    return ret;
}

void tora::Init_Dir(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Dir");
    pkg->add_method(vm->symbol_table->get_id("read"), new CallbackFunction(dir_read));
    pkg->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(dir_DESTROY));
    pkg->add_method(vm->symbol_table->get_id("__iter__"), new CallbackFunction(dir___iter__));

    SharedPtr<Package> iter = vm->find_package("Dir::Iterator");
    iter->add_method(vm->symbol_table->get_id("__next__"), new CallbackFunction(dir_Iterator___next__));
}

