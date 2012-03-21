#include "../object.h"
#include "../shared_ptr.h"
#include "../inspector.h"
#include "../value.h"
#include "../value/object.h"
#include "../value/symbol.h"
#include "../package.h"
#include "../vm.h"
#include "../peek.h"
#include "../symbols.gen.h"

using namespace tora;

/**
 * convert object to tora source.
 */
static SharedPtr<Value> object_tora(VM *vm, Value *self) {
    Inspector ins(vm);
    return new StrValue(ins.inspect(self));
}

/**
 * $object.meta();
 *
 * get a meta class.
 */
static SharedPtr<Value> object_meta(VM *vm, Value *self) {
    ID pkgid;
    if (self->value_type == VALUE_TYPE_SYMBOL) {
        pkgid = self->upcast<SymbolValue>()->id;
    } else if (self->value_type == VALUE_TYPE_OBJECT) {
        pkgid = self->upcast<ObjectValue>()->package_id();
    } else {
        pkgid = vm->symbol_table->get_id(self->type_str());
    }
    return new ObjectValue(vm, SYMBOL_METACLASS_CLASS, new IntValue(pkgid));
}

static bool check_isa(VM *vm, ID pkgid, ID target_id) {
    if (pkgid == target_id) {
        return true;
    } else {
        Package * parent = vm->find_package(pkgid);
        if (parent->superclass()) {
            return check_isa(vm, parent->superclass()->id(), target_id);
        } else {
            return false;
        }
    }
}

static SharedPtr<Value> object_isa(VM *vm, Value *self, Value *target_v) {
    ID pkgid = vm->symbol_table->get_id(self->type_str());
    SharedPtr<StrValue> target = target_v->to_s();
    ID target_id = vm->symbol_table->get_id(target->str_value());
    return BoolValue::instance(check_isa(vm, pkgid, target_id));
}

void tora::Init_Object(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Object");
    pkg->add_method(vm->symbol_table->get_id("tora"), new CallbackFunction(object_tora));
    pkg->add_method(vm->symbol_table->get_id("meta"), new CallbackFunction(object_meta));
    pkg->add_method(vm->symbol_table->get_id("isa"), new CallbackFunction(object_isa));
}

