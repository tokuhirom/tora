#include "../object.h"
#include "../vm.h"
#include "../value/array.h"
#include "../value/object.h"
#include "../value/code.h"
#include "../package.h"
#include "../peek.h"
#include "../frame.h"

using namespace tora;

/**
 * class MetaClass
 *
 * This is a metaclass. Meta class is class of class.
 *
 * You can get a meta class for $foo from $foo.meta().
 * 
 * Note. API is compatible with Moose in Perl5, preferably.
 */

static inline Package *GET_PACKAGE(VM *vm, Value *self) {
    ID pkgid = self->upcast<ObjectValue>()->data()->upcast<IntValue>()->int_value();
    Package * pkg = vm->find_package(pkgid);
    return pkg;
}

/**
 * $meta.has_method(Str $name) : Boolean
 *
 * Returns a boolean indicating whether or not the class defines the named method.
 * It does not include methods inherited from parent classes.
 */
static SharedPtr<Value> mc_has_method(VM * vm, Value* self, Value * methname_v) {
    SharedPtr<StrValue> methname = methname_v->to_s();
    ID pkgid = self->upcast<ObjectValue>()->data()->upcast<IntValue>()->int_value();
    Package * pkg = vm->find_package(pkgid);
    return BoolValue::instance(pkg->has_method(vm->symbol_table->get_id(methname->str_value())));
}

/**
 * $meta.get_method_list() : Array[Str]
 *
 * Get a method list defined in package.
 */
static SharedPtr<Value> mc_get_method_list(VM * vm, Value* self) {
    ID pkgid = self->upcast<ObjectValue>()->data()->upcast<IntValue>()->int_value();
    Package * pkg = vm->find_package(pkgid);
    SharedPtr<ArrayValue> av = new ArrayValue();
    for (auto iter=pkg->begin(); iter!=pkg->end(); ++iter) {
        av->push_back(new StrValue(vm->symbol_table->id2name(iter->first)));
    }
    return av;
}


/**
 * $meta.name() : String
 *
 * Get a name of class.
 */
static SharedPtr<Value> mc_name(VM * vm, Value* self) {
    ID pkgid = self->upcast<ObjectValue>()->data()->upcast<IntValue>()->int_value();
    return new StrValue(vm->symbol_table->id2name(pkgid));
}

/**
 * $meta.superclass() : String
 *
 * This method returns superclass in string.
 * If the class does not have a superclass, it returns undef.
 */
static SharedPtr<Value> mc_superclass(VM * vm, Value* self) {
    Package * pkg = GET_PACKAGE(vm, self);
    Package * super = pkg->superclass();
    if (super) {
        return new StrValue(vm->symbol_table->id2name(super->id()));
    } else {
        return UndefValue::instance();
    }
}

void tora::Init_MetaClass(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("MetaClass");
    pkg->add_method(vm->symbol_table->get_id("has_method"), new CallbackFunction(mc_has_method));
    pkg->add_method(vm->symbol_table->get_id("get_method_list"), new CallbackFunction(mc_get_method_list));
    pkg->add_method(vm->symbol_table->get_id("name"), new CallbackFunction(mc_name));
    pkg->add_method(vm->symbol_table->get_id("superclass"), new CallbackFunction(mc_superclass));
}

