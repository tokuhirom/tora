#include "../object.h"
#include "../vm.h"
#include "../symbols.gen.h"
#include "../value/class.h"
#include "../class_builder.h"

void tora::Init_Int(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_INT_CLASS);
  vm->add_builtin_class(builder.value());
}
