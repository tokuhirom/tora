#include "metaclass.h"
#include "../vm.h"
#include "../value/array.h"
#include "../value/code.h"
#include "../package.h"
#include "../peek.h"
#include "../frame.h"

using namespace tora;

/**
 * $meta.has_method('foo');
 *
 * Returns a boolean indicating whether or not the class defines the named method.
 * It does not include methods inherited from parent classes.
 */
static SharedPtr<Value> mc_has_method(VM * vm, Value* self, Value * methname_v) {
    SharedPtr<StrValue> methname = methname_v->to_s();
    ID pkgid = self->upcast<ObjectValue>()->data()->package_id();
    Package * pkg = vm->find_package(pkgid);
    return BoolValue::instance(pkg->has_method(methname->str_value));
}

void tora::Init_MetaClass(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("MetaClass");
    pkg->add_method(vm->symbol_table->get_id("has_method"), new CallbackFunction(mc_has_method));
}

