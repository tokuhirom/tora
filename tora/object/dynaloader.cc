#include "../object.h"
#include "../shared_ptr.h"
#include "../value.h"
#include "../vm.h"
#include "../package.h"

using namespace tora;

/**
 * class DynaLoader
 * 
 * This is a dynamic extension library loader for tora.
 * This is a low level library.
 * You don't need to use this module directly in normal case. You can use XSLoader instead.
 */

/**
 * DynaLoader.load(Str $filename, Str $endpoint) : Undef
 *
 * Load a extension library from file $filename.
 * This method loads $filename by dlopen(3) and find symbol named $endpoint, and call it.
 */
static SharedPtr<Value> dynaloader_load(VM *vm, Value *self, Value *filename_v, Value *endpoint_v) {
    SharedPtr<StrValue> filename_s = filename_v->to_s();
    SharedPtr<StrValue> endpoint_s = endpoint_v->to_s();
    vm->load_dynamic_library(filename_s->str_value(), endpoint_s->str_value());
    return UndefValue::instance();
}

void tora::Init_DynaLoader(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("DynaLoader");
    pkg->add_method(vm->symbol_table->get_id("load"), new CallbackFunction(dynaloader_load));
}

