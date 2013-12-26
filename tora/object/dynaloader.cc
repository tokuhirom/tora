#include "../object.h"
#include "../shared_ptr.h"
#include "../value.h"
#include "../vm.h"
#include "../value/class.h"
#include "../symbols.gen.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class DynaLoader
 *
 * This is a dynamic extension library loader for tora.
 * This is a low level library.
 * You don't need to use this module directly in normal case. You can use
 *XSLoader instead.
 */

/**
 * DynaLoader.load(Str $filename, Str $endpoint) : Undef
 *
 * Load a extension library from file $filename.
 * This method loads $filename by dlopen(3) and find symbol named $endpoint, and
 *call it.
 */
static SharedPtr<Value> dynaloader_load(VM *vm, Value *self, Value *filename_v,
                                        Value *endpoint_v) {
  std::string filename_s = filename_v->to_s();
  std::string endpoint_s = endpoint_v->to_s();
  vm->load_dynamic_library(filename_s, endpoint_s);
  return new_undef_value();
}

void tora::Init_DynaLoader(VM *vm) {
  ClassBuilder builder(vm, SYMBOL_DYNALOADER_CLASS);
  builder.add_method("load", new CallbackFunction(dynaloader_load));
  vm->add_builtin_class(builder.value());
}
